#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ftp_socket.h"

ssize_t Writen(int fd,const void *vptr,size_t n)
{
	size_t nleft;
	ssize_t nwritten;
	const char *ptr;
	ptr = vptr;
	nleft = n;

	while(nleft>0)
	{
		if((nwritten=write(fd,ptr,nleft))<=0)
		{
			if(nwritten<0 && errno==EINTR)
				nwritten=0;
			else
				return -1;
		}
		nleft -=nwritten;
		ptr +=nwritten;
	}
	return n;
}

void Listen (int fd,int backlog)
{
	//char *ptr;
	if(listen(fd,backlog)<0)
		printf("listen error\n");
}

int ftp_socket_connect(int socketHandle, const char *ipAddress, const int port)
{    
	struct sockaddr_in servAddr;    
	return_val_if_fail(socketHandle != INVALID_SOCKET, -1);   
	return_val_if_fail(ipAddress != NULL, -1);    
	
	//bzero(&servAddr, sizeof(servAddr));    
	memset(&servAddr,0,sizeof(servAddr));
	servAddr.sin_family = AF_INET;    
	servAddr.sin_port = htons(port);    
	inet_pton(AF_INET, ipAddress, &servAddr.sin_addr);    
	return connect(socketHandle, (struct sockaddr *)&servAddr, sizeof(servAddr));//(SA *)
}

FTP_Ret ftp_socket_bind_and_listen(SOCKET_HANDLE socketHandle, const char *ipAddress, const int port)
{    
	struct sockaddr_in servAddr;    
	return_val_if_fail(socketHandle != INVALID_SOCKET, FTP_RET_INVALID_PARAMS);    
	return_val_if_fail(ipAddress != NULL, FTP_RET_INVALID_PARAMS);    
	
	//bzero(&servAddr, sizeof(servAddr));  
	memset(&servAddr,0,sizeof(servAddr));
	servAddr.sin_family = AF_INET;    
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);    
	servAddr.sin_port = htons(port);    
	bind(socketHandle, (struct sockaddr *)&servAddr, sizeof(servAddr)); //(SA *)   
	Listen(socketHandle, 1);   
	return FTP_RET_OK;
}

FTP_Ret ftp_socket_listen(SOCKET_HANDLE socketHandle, int maxListen)
{    
	return_val_if_fail(socketHandle != INVALID_SOCKET, FTP_RET_INVALID_PARAMS);   
	Listen(socketHandle, maxListen);    
	return FTP_RET_OK;
}
int ftp_socket_accept(SOCKET_HANDLE socketHandle)
{    
	return_val_if_fail(socketHandle != INVALID_SOCKET, FTP_RET_INVALID_PARAMS);    
	return accept(socketHandle, NULL, NULL);
}
SOCKET_HANDLE ftp_socket_create(void)
{    
	return socket(AF_INET, SOCK_STREAM, 0);
}
int ftp_socket_select(SOCKET_HANDLE socketHandle, int event, int secTime)
{    
	struct timeval timeValue;    
	fd_set readSet, writeSet;    
	return_val_if_fail(socketHandle != INVALID_SOCKET, FTP_RET_INVALID_PARAMS);    
	
	FD_ZERO(&readSet);    
	if ( (event & FD_ACCEPT) or (event & FD_READ) or (event & FD_CLOSE) )    
	{        
		FD_SET(socketHandle, &readSet);    
	}    
	FD_ZERO(&writeSet);   
	if ( (event & FD_CONNECT) or (event & FD_WRITE) )    
	{        
		FD_SET(socketHandle, &writeSet);    
	}   
	timeValue.tv_sec = secTime;    
	timeValue.tv_usec = secTime * 1000;    
	return select(socketHandle + 1, &readSet, &writeSet, NULL, &timeValue);
}

FTP_Ret ftp_socket_close(SOCKET_HANDLE socketHandle)
{    
	return_val_if_fail(socketHandle != INVALID_SOCKET, FTP_RET_INVALID_PARAMS);    
	close(socketHandle);   
	socketHandle = INVALID_SOCKET;    
	return FTP_RET_OK;
}


