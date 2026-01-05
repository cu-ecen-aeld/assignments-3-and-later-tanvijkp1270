/**
* @File aesdsocket.h
* @author Tanvi Sharma
*/
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <syslog.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/un.h>
#include <signal.h>
#include <inttypes.h>
#include <stdio.h>

#define PORT_ADDRESS "9000"
#define BACKLOG 10
#define FILE_PATH "/var/tmp/aesdsocketdata"
#define RECV_BUFFER_SIZE 4096
#define SEND_BUFFER_SIZE 4096
#define DELIMITER "\n"