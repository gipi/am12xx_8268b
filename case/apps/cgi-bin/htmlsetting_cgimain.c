#include <stdio.h>  
#include <stddef.h>  
#include <sys/stat.h>  
#include <sys/socket.h>  
#include <sys/un.h>  
#include <errno.h>  
#include <string.h> 
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "htmlsetting_cgi.h"

#ifdef	HTMLSETTING_CGI_ENABLE

/* Create a client endpoint and connect to a server.   Returns fd if all OK, <0 on error. */  
static int unix_socket_conn(const char *servername)  
{   
	int fd;   
	if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)    /* create a UNIX domain stream socket */   
	{  
		return(-1);  
	}  
	int len, rval;  
	struct sockaddr_un un;            
	memset(&un, 0, sizeof(un));            /* fill socket address structure with our address */  
	un.sun_family = AF_UNIX;   
	sprintf(un.sun_path, "/tmp/scktmp%05d", getpid());   
	len = offsetof(struct sockaddr_un, sun_path) + strlen(un.sun_path);  
	unlink(un.sun_path);               /* in case it already exists */   
	if (bind(fd, (struct sockaddr *)&un, len) < 0)  
	{   
		rval=  -2;   
	}   
	else  
	{  
		/* fill socket address structure with server's address */  
		memset(&un, 0, sizeof(un));   
		un.sun_family = AF_UNIX;   
		strcpy(un.sun_path, servername);   
		len = offsetof(struct sockaddr_un, sun_path) + strlen(servername);   
		if (connect(fd, (struct sockaddr *)&un, len) < 0)   
		{  
			rval= -4;   
		}   
		else  
		{  
			return (fd);  
		}  
	}  
	int err;  
	err = errno;  
	close(fd);   
	errno = err;  
	return rval;      
}  
   
static void unix_socket_close(int fd)  
{  
	cgidbg_info("CGI SOCKET CLOSE!!!\n");
	close(fd);       
}

int del_return(void **return_val){
	
	if(*return_val) free(*return_val);
	*return_val = NULL;

	return 0;
}

int htmlsetting_func(char *cmd, void *val, int len, void **return_val, int *return_len){
	int connfd;
	struct msglen_s msglen;
	int cmdlen = strlen(cmd), msgsize;
	void *msg;
	int ret;
	
	connfd = unix_socket_conn(UNIX_SOCKET_PATCH);  
	if(connfd<0)	
	{  
		//printf("Error[%d] when connecting...",errno);
		return -1;
	}

	if(val == NULL || len == 0){
		msgsize = cmdlen+sizeof(struct msglen_s);
	}else{
		msgsize = len+(sizeof(struct msglen_s))*2+cmdlen;
	}
	msglen.len = cmdlen;
	memcpy(msglen.flag, CMDLEN_MARK, 2);
	ret = send(connfd, (void *)&msglen, sizeof(struct msglen_s), 0);
	ret = send(connfd, cmd, msglen.len, 0);

	if(val != NULL && len != 0){
		msglen.len = len;
	}else{
		msglen.len = 0;
	}
	memcpy(msglen.flag, VALLEN_MARK, 2);
	ret = send(connfd, (void *)&msglen, sizeof(struct msglen_s), 0);
	ret = send(connfd, val, msglen.len, 0);
	
	while(1){
		ret = recv(connfd, &msglen, sizeof(msglen), 0);
		if(ret>0) 
		{ 
			 if(!strncmp(msglen.flag, END_MARK, 2)){
			 	cgidbg_info("function end!!!!\n");
				unix_socket_close(connfd);
				return 0;
			 }else if(!strncmp(msglen.flag, SENDLEN_MARK, 2)){
			 	cgidbg_info("return!!!\n");
			 	if(msglen.len < 20*1024){
					if(msglen.len <= 0){
						cgidbg_info("msglen.len == 0!!!\n");
						continue;
					}
					msg = (void *)malloc(msglen.len+1);
					if(msg != NULL){
						memset(msg, 0, msglen.len+1);
						ret = recv(connfd, msg, msglen.len, 0);
						if(ret>=0) 
						{
							if(return_val != NULL){
								cgidbg_info("val point!!! return len: %d, msg: %s\n", msglen.len, (char *)msg);
								*return_val = msg;
								if(return_len != NULL){
									*return_len = msglen.len;
									cgidbg_info("return len = %d!!!\n", *return_len);
								}
							}else{
								cgidbg_info("return_val is NULL!!!\n");
								free(msg);
								msg = NULL;
							}
						}else{
							free(msg);
							msg = NULL;
						}
					}else{
						perror("msg malloc");
					}
				}else{
					cgidbg_info("msglen.len = %d\n", msglen.len);
				}
				cgidbg_info("continue!!\n");
				continue;
			 }else{
				cgidbg_info("RECEIVE ERROR MSG[%s]!!!\n", msglen.flag);
				continue;
			 }
		}
		cgidbg_info("break!! ret = %d\n", ret);
		break;      
	}

	unix_socket_close(connfd);
	return -1;
}

#endif

