/**
* @File aesdsocket.c
* @author Tanvi Sharma
*/
#include "aesdsocket.h"

//******************************************************
// Global Variables
//******************************************************
int sockfd,newfd,outfd;
struct addrinfo hints;
struct addrinfo *servinfo;
struct sockaddr_in clientaddr;
socklen_t clientaddrsize;
struct sigaction newaction;
char *readBuffer = NULL;
volatile sig_atomic_t caughtSignal = 0;

//******************************************************
// Function Declarations
//******************************************************
static void signalHandler( int signalNum);
static void processData( char *readBuffer, int length);
static void resendData(int outfd,int newfd);
static void gracefulShutdown(void);
static void daemonizeProcess(void);

//******************************************************
// Daemonize the process `(o_o)'
//******************************************************
static void daemonizeProcess(void)
{
    pid_t pid = fork();
    int fd;
    
    /* An error occurred */
    if (pid < 0)
    {
        syslog(LOG_ERR,"Error in fork - 1st child, %d, (%s)",errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    /* Success: Let the parent terminate */
    if (pid > 0)
        exit(EXIT_SUCCESS);

    /* On success: The child process becomes session leader */
    if (setsid() < 0)
    {
        syslog(LOG_ERR,"Error in setsid, %d, (%s)",errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    pid = fork();

    /* An error occurred */
    if (pid < 0)
    {
        syslog(LOG_ERR,"Error in fork - 1st child, %d, (%s)",errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    /* Success: Let the parent terminate */
    if (pid > 0)
        exit(EXIT_SUCCESS);

    /* Change working directory to root */
    if (chdir("/") < 0) 
    {
        exit(EXIT_FAILURE);
    }

    /* Reset file permissions */
    umask(0);

    /* Close standard file descriptors */
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    if((fd = open("/dev/null", O_RDWR)) == -1)
    {
        syslog(LOG_ERR,"Error in opening /dev/null, %d, (%s)",errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
    else
    {
        dup2(fd, STDIN_FILENO);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);

    }

}

//******************************************************
// This API Ensures Graceful Shutdown
//******************************************************
static void gracefulShutdown(void)
{
    if(sockfd > 0)
    {
        if(shutdown(sockfd,SHUT_RDWR) == -1)
        {
            syslog(LOG_ERR,"Error in closing sockfd, %d, (%s)",errno, strerror(errno));
        }
    }

    if(newfd > 0)
    {
        if(shutdown(newfd,SHUT_RDWR) == -1)
        {
            syslog(LOG_ERR,"Error in closing newfd, %d, (%s)",errno, strerror(errno));
        }
    }

    if(outfd > 0)
    {
        if(close(outfd) == -1)
        {
            syslog(LOG_ERR,"Error in closing outfd, %d, (%s)",errno, strerror(errno));
        }
    }

    if(unlink(FILE_PATH) != 0)
    {
        /* Logs the error in file path unlinking */
        syslog(LOG_ERR,"Unlinking error for file path, %d, (%s)",errno, strerror(errno));
    }

    if(readBuffer != NULL)
    {
        free(readBuffer);
    }
}

//******************************************************
// Echo Data back to Client for TCP Bytestream
//******************************************************
static void resendData(int outfd,int newfd)
{
    struct stat st;
    char *sendBuffer = NULL;
    int totalIterations = 0;
    int readBytes = 0 ,totalReadBytes = 0,totalSendBytes = 0,sendBytes = 0;

    /* Move the file pointer to the start of file */
    if(lseek(outfd,0,SEEK_SET) == -1)
    {
        syslog(LOG_ERR,"LSEEK API failure, %d, (%s)",errno, strerror(errno));
    }

    /* Get the file size */
    if (stat(FILE_PATH, &st) == -1)
    {
        syslog(LOG_ERR,"Stat API failure, %d, (%s)",errno, strerror(errno));
    }

    if(st.st_size < 0)
    {
        syslog(LOG_ERR,"FileSize is negative, %d, (%s)",errno, strerror(errno));
    }
    else
    {
        syslog(LOG_INFO,"File Size, %ld",(intmax_t)st.st_size);
        totalIterations = (int)st.st_size/SEND_BUFFER_SIZE;

        /* Allocate memory for sending data */
        sendBuffer = (char *)malloc(SEND_BUFFER_SIZE);

        for(int i = 0; i <= totalIterations; i++)
        {
            if((readBytes = read(outfd,sendBuffer,SEND_BUFFER_SIZE)) < 0)
            {
                syslog(LOG_ERR,"Read file API failure, %d, (%s)",errno, strerror(errno));
            }
            else
            {
                totalReadBytes += readBytes;
                //syslog(LOG_INFO,"totalIterations, %d",totalIterations);
                syslog(LOG_INFO,"Current Read Bytes, %d",readBytes);
                //yslog(LOG_INFO,"Total Read Bytes, %d",totalReadBytes);

                /* Send the data that was read from file until the entire number of bytes read are sent */
                while(sendBytes < readBytes)
                {
                    /* Send the bytes read to the client , echoing the file data back */
                    if((sendBytes = send(newfd,sendBuffer,readBytes,0)) == -1)
                    {
                        syslog(LOG_ERR,"Send API failure, %d, (%s)",errno, strerror(errno));
                    }
                    else
                    {
                        totalSendBytes += sendBytes;
                        syslog(LOG_INFO,"Current Send Bytes, %d",sendBytes);
                        //syslog(LOG_INFO,"Total Send Bytes, %d",totalSendBytes);
                    }
                }

                sendBytes = 0;

                if(((intmax_t)totalReadBytes) >= ((intmax_t)st.st_size))
                {
                    break;
                }
            }

        }

        /* Freeing the send buffer memory */
        free(sendBuffer);
    }
}

//******************************************************
// Data parser for TCP Bytestream
//******************************************************
static void processData( char *readBuffer, int length)
{
    int bytesWritten;

    if(readBuffer != NULL && length > 0)
    {
        if((bytesWritten = write(outfd, readBuffer , length)) == -1)
        {
            syslog(LOG_ERR,"Packet write error, %d, (%s)",errno, strerror(errno));
        }
        else
        {
            //syslog(LOG_INFO,"Packet Size written %d",bytesWritten);
        }

        for(int i = 0; i< length; i++)
        {
            if(readBuffer[i] == '\n')
            {
                /* Resend Data back to client */
                resendData(outfd,newfd);
            }
        }
    }
}

//******************************************************
// Signal Handler
//******************************************************
static void signalHandler( int signalNum)
{
    if(signalNum == SIGINT || signalNum == SIGTERM )
    {
        syslog(LOG_INFO,"Caught signal, exiting");
        caughtSignal = 1;
    }
    else
    {
        syslog(LOG_ERR,"Invalid signal interruption occurred, %d, (%s)",errno, strerror(errno));
        caughtSignal = 1;
    }

}

//******************************************************
// main function
//******************************************************
int main(int argc, char**argv)
{
    int status,receivedbytes,output;
    char ip_address_str[INET_ADDRSTRLEN];

    /* Open log for syslog logging */
    openlog("assignment5part1", LOG_PERROR | LOG_CONS, LOG_USER);

    /* Setting up signal handling using sigaction*/
    memset(&newaction, 0, sizeof(newaction));
    newaction.sa_handler = signalHandler;

    /* Open the file with O_APPEND, O_CREAT (create if non-existent), and O_RDWR (read-write) flags */
    outfd = open(FILE_PATH,  O_RDWR | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR);

    if(sigaction(SIGTERM,&newaction,NULL) != 0)
    {
        /* Logs the error in signal handler registration */
        syslog(LOG_ERR,"Signal Handler registration failure, %d, (%s)",errno, strerror(errno));
		exit(-1);
    }

    if(sigaction(SIGINT,&newaction,NULL) != 0)
    {
        /* Logs the error in signal handler registration */
        syslog(LOG_ERR,"Signal Handler registration failure, %d, (%s)",errno, strerror(errno));
		exit(-1);
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;   // Allow IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP stream socket
    hints.ai_flags = AI_PASSIVE;   // Fill in my IP for me (INADDR_ANY)

    /* Populate the socketaddr structure using getaddrinfo API */

    if((status = getaddrinfo(NULL, PORT_ADDRESS, &hints, &servinfo)) != 0)
    {
        syslog(LOG_ERR, "getaddrinfo: %s\n", gai_strerror(status));
        exit(-1);
    }


    /* Create socket file descriptor */
    sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);

    if(sockfd < 0)
    {
        /* Logs the error in creation of socket file descriptor */
        syslog(LOG_ERR,"Socket creation API failure %d, (%s)",errno, strerror(errno));
		exit(-1);
    }

    int optval = 1; // Non-zero value to enable the option

    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
    {
        syslog(LOG_ERR,"Sockopt API failure %d, (%s)",errno, strerror(errno));
        close(sockfd);
    }

    /* Bind socket file descriptor to specific IP address */
    if(bind(sockfd,servinfo->ai_addr,servinfo->ai_addrlen) != 0)
    {
        syslog(LOG_ERR, "Bind API failure %d, (%s)",errno, strerror(errno));
        gracefulShutdown();
        exit(-1);
    }

    if (servinfo == NULL) {
        syslog(LOG_ERR, "Binding failure %d, (%s)",errno, strerror(errno));
        gracefulShutdown();
        exit(-1);
    }

    /* Free the servinfo memory as it was malloc'd inside getaddrinfo*/

    freeaddrinfo(servinfo);

    if (listen(sockfd, BACKLOG) == -1) 
    {
        syslog(LOG_ERR, "Listen API failure %d, (%s)",errno, strerror(errno));
        exit(-1);
    }

    readBuffer = (char *)malloc(RECV_BUFFER_SIZE);

    if(readBuffer == NULL)
    {
        /* Logs the error in Malloc failure */
        syslog(LOG_ERR,"Malloc failure, %d, (%s)",errno, strerror(errno));
		exit(-1);
    }

    /* Check if the second argument requires the program to run in daemon mode */
    if ((argc == 2) && (strcmp(argv[1], "-d") == 0))
    {
        daemonizeProcess();
    }

    while(caughtSignal != 1)
    {
        clientaddrsize = sizeof(clientaddr);
        /* Accept client connection and use newfd for receiving data */
        newfd = accept(sockfd, (struct sockaddr *)&clientaddr,&clientaddrsize);

        if(newfd == -1)
        {
            syslog(LOG_ERR, "Accept API failure %d, (%s)",errno, strerror(errno));
            gracefulShutdown();
            exit(-1);
        }
        else
        {
            if (inet_ntop(AF_INET, &(clientaddr.sin_addr), ip_address_str, INET_ADDRSTRLEN) == NULL) 
            {
                syslog(LOG_ERR, "inet_ntop, gathering IPv4 Address API failure %d, (%s)",errno, strerror(errno));
                gracefulShutdown();
                exit(-1);
            }
            else
            {
                syslog(LOG_INFO,"Accepted connection from %s\n",ip_address_str);
            }
        }

        while((output = recv(newfd, readBuffer, RECV_BUFFER_SIZE - 1, 0)) > 0)
        {
            receivedbytes +=output;
            syslog(LOG_INFO, "Total Received Bytes %d",receivedbytes);
            processData(readBuffer, output);
        }

        if(output == 0)
        {
            /* Client has closed the connection, no more data to be received on this file descriptor */
            syslog(LOG_INFO, "Closed connection from %s",ip_address_str);
            /* Close the current file descriptor */
            close(newfd);
        }
        else
        {
            syslog(LOG_ERR, "recv API failure %d, (%s)",errno, strerror(errno));
            close(newfd);
            gracefulShutdown();
            exit(-1);
        }

    }

    gracefulShutdown();

    return 0;
}
