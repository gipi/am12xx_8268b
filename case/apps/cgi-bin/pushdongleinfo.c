#include <stdio.h>
#include <stdlib.h>
#include "cgic.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "am_cgi.h"
#include "htmlsetting_cgi.h"

#if 0
static int init_send_socket(struct sockaddr_un *addr, char *path){
	int sockfd;  
	sockfd=socket(AF_UNIX,SOCK_DGRAM,0);  
	if(sockfd<0)  
	{  
		return -1;  
	}  
	bzero(addr,sizeof(struct sockaddr_un));  
	addr->sun_family=AF_UNIX;  
	strcpy(addr->sun_path,path);  
	return sockfd;
}

static int send_to_socket(int sockfd, char msg[], const struct sockaddr_un * addr)  
{  
	int len;   
	len = strlen(addr->sun_path)+sizeof(addr->sun_family);  
	sendto(sockfd, msg, strlen(msg), 0, (struct sockaddr*)addr,len);   
	return 1;  
}

int init_recv_socket(char * path)  
{  
		int sockfd,len;   
		struct sockaddr_un addr;   
		sockfd=socket(AF_UNIX,SOCK_DGRAM,0);   
		if(sockfd<0)   
		{
        	perror("socket");
			return -1;  
		}   
		bzero(&addr,sizeof(struct sockaddr_un));   
		addr.sun_family = AF_UNIX;   
		strcpy(addr.sun_path, path);  
		unlink(path);  
		len = strlen(addr.sun_path) + sizeof(addr.sun_family);  
		if(bind(sockfd,(struct sockaddr *)&addr,len)<0)   
		{
			perror("bind");
			return -1;  
		}   
		return sockfd;  

}  
int receive_from_socket(int sockfd)  
{  
	int n; 
	char *msg;
	int msg_size = 0;
	//printf("sockfd = %d\n", sockfd);
	if((n=recvfrom(sockfd, &msg_size, sizeof(int), 0, NULL, NULL))>0){
		//printf("msg_size = %d\n", msg_size);
		msg = (char *)malloc(msg_size);
		if(msg != NULL){
			memset(msg, 0, msg_size);  
			n=recvfrom(sockfd, msg, msg_size, 0, NULL, NULL);   
			if(n<=0)  
			{  
				printf("recv error\n");
				return -1;  
			}  
			msg[n]=0;  
			fprintf(cgiOut, "%s\n", msg);
			//printf("msg: %s\n", msg);
			free(msg);
			return n; 
		}
	}
	return -1;
} 
#endif

int cgiMain(int argc, char *argv[]){
	char *msg = NULL;
	int msg_size = 0;
	//*recv_buffer = NULL;
    cgiHeaderCacheControl("no-cache, no-store, must-revalidate");
    cgiHeaderPragma("no-cache");
    cgiHeaderExpires(0);
	cgiHeaderContentType("text/html");

#if 0
    if( (fd_send = init_send_socket(&addr, SEND_PATH)) > 0 && (fd_recv = init_recv_socket(RECV_PATH))> 0){
		send_to_socket(fd_send, CMD_GETDONGLEINFO, &addr); 
		//memset(recv_buffer, 0, MSG_SIZE);  
		if(receive_from_socket(fd_recv))  
		{  
			//printf("cgi: %s\n", recv_buffer);
		}
		close(fd_send);
		close(fd_recv);
		return 0;
	}

	if(fd_send > 0)
		close(fd_send);
	if(fd_recv > 0)
		close(fd_recv);
#else
	htmlsetting_func("htmlsetting_app_command", CMD_GETDONGLEINFO, strlen(CMD_GETDONGLEINFO), (void **)&msg, &msg_size);
	if(msg != NULL){
		fprintf(cgiOut, "%s\n", msg);
		del_return((void **)&msg);
	}
#endif

	return -1;
}

