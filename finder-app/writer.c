/**
* @File writer.c
* @author Tanvi Sharma
*/

#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <syslog.h>

int main(int argc, char ** argv)
{
	char *filename = NULL, *filestr = NULL;
	
	openlog("assignment2", LOG_CONS, LOG_USER);
	
	if(argc < 2)
	{
		syslog(LOG_ERR, "Not enough arguments %s", strerror(errno));
		return 1;
	
	}
	else
	{
		
		filename = argv[1];
		filestr = argv[2];
		int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0744);

		 if (fd == -1) 
		 {
		 	syslog(LOG_ERR, "Error encountered: %s File open failed", strerror(errno));
			return 1;
		 }
		 
		 syslog(LOG_DEBUG,"Writing %s to %s", filestr,filename);
		 
		 ssize_t numbytes = write(fd, filestr, strlen(filestr));
		 if (numbytes == -1) 
		 {
			syslog(LOG_ERR, "Error encountered: %s File write failed", strerror(errno));
			close(fd); // Close the file even if writing failed
			return 1;
    		 }
    		 
	}
	
	syslog(LOG_INFO,"File writing successful");
	closelog();
	
	return 0;
}
