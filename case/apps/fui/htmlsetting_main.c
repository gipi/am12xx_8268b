#include <stdio.h>  
#include <sys/stat.h>  
#include <sys/socket.h>  
#include <sys/un.h>  
#include <errno.h>  
#include <stddef.h>  
#include <string.h>
#include <stdlib.h>

#include "htmlsetting.h"
  

#ifdef	HTMLSETTING_ENABLE
struct htmlsetting_priv_s{
	int exit;
	int running;
	pthread_t looper;
};

static int htmlsetting_fd = -1;
struct htmlsetting_priv_s htmlsetting_priv;

/* * Create a server endpoint of a connection. * Returns fd if all OK, <0 on error. */  
static int unix_socket_listen(const char *servername)  
{   
	int fd;  
	struct sockaddr_un un;   
	if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)  
	{  
		return(-1);   
	}  
	int len, rval;   
	unlink(servername);               /* in case it already exists */   
	memset(&un, 0, sizeof(un));   
	un.sun_family = AF_UNIX;   
	strcpy(un.sun_path, servername);   
	len = offsetof(struct sockaddr_un, sun_path) + strlen(servername);   
	/* bind the name to the descriptor */   
	if (bind(fd, (struct sockaddr *)&un, len) < 0)  
	{   
		rval = -2;   
	}   
	else  
	{  
		if (listen(fd, MAX_CONNECTION_NUMBER) < 0)      
		{   
			rval =  -3;   
		}  
		else  
		{  
			return fd;  
		}  
	}  
	int err;  
	err = errno;  
	close(fd);   
	errno = err;  
	return rval;    
} 

static int unix_socket_accept(int listenfd, uid_t *uidptr)  
{   
	int clifd, len, rval;   
	time_t staletime;   
	struct sockaddr_un un;  
	struct stat statbuf;   
	len = sizeof(un);   
	if ((clifd = accept(listenfd, (struct sockaddr *)&un, (socklen_t *)&len)) < 0)   
	{  
		return(-1);       
	}  
	/* obtain the client's uid from its calling address */   
	len -= offsetof(struct sockaddr_un, sun_path);  /* len of pathname */  
	un.sun_path[len] = 0; /* null terminate */   
	if (stat(un.sun_path, &statbuf) < 0)   
	{  
		rval = -2;  
	}   
	else  
	{  
		if (S_ISSOCK(statbuf.st_mode) )   
		{   
			if (uidptr != NULL) *uidptr = statbuf.st_uid;    /* return uid of caller */   
			unlink(un.sun_path);       /* we're done with pathname now */   
			return clifd;        
		}   
		else  
		{  
			rval = -3;     /* not a socket */   
		}  
	}  
	int err;  
	err = errno;   
	close(clifd);   
	errno = err;  
	return(rval);  
}  
   
static void unix_socket_close(int fd)  
{  
	printf("[%s] [%d] -- FUI SOCKET CLOSE!!!\n", __func__, __LINE__);
	close(fd);       
} 

int return_val(void *val, int len, void *handle){
	int ret = -1;
	struct select_ctrl_s *clinfo = (struct select_ctrl_s *)handle;

	if(val != NULL && len != 0){
		void *send_buf = (void *)malloc(len+1);
		if(send_buf != NULL){
			struct msglen_s msglen;

			memset(send_buf, 0, len+1);
			msglen.len = len;
			memcpy(msglen.flag, SENDLEN_MARK, 2);
			memcpy(send_buf, val, len);
			printf("return len: %d\n", len);
			ret = send(clinfo->fd, (void *)&msglen, (sizeof(struct msglen_s)), 0);
			ret = send(clinfo->fd, send_buf, len, 0);
			printf("[%s] [%d] -- SEND RETURN VALUE!!!\n", __func__, __LINE__);
			free(send_buf);
		}
	}

	return ret;
}

int function_end(void *handle){
	int ret;
	struct select_ctrl_s *clinfo = (struct select_ctrl_s *)handle;
	struct msglen_s msglen;

	msglen.len = 0;
	memcpy(msglen.flag, END_MARK, 2);

	ret = send(clinfo->fd, (void *)&msglen, sizeof(struct msglen_s), 0);
	if(ret < 0){
		if(errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN){
			ret = send(clinfo->fd, (void *)&msglen, sizeof(struct msglen_s), 0);
		}
	}
	printf("[%s] [%d] -- FUI FUNCTION END!!!\n", __func__, __LINE__);
	usleep(2000);
	return ret;
}

static int receive_val(int fd, void **buf, char *mark){
	int size = -1;
	struct msglen_s msglen;
	void *tmp = NULL;

	size = recv(fd, (void *)&msglen, sizeof(msglen), MSG_DONTWAIT);
	if(size < 0){
		perror("unix socket rcv");
		return -1;
	}else if(size == 0){
		printf("socket closed!!!");
		return 0;
	}
	printf("msglen: %s\n", (char *)&msglen);
	if(strncmp(msglen.flag, mark, 2) || msglen.len > 20*1024 || msglen.len < 0){
		printf("[%s] [%d] -- unix socket receive lenght error!! flag: %s, len: %d\n", __func__, __LINE__, msglen.flag, msglen.len);
		return -1;
	}
	printf("len: %d\n", msglen.len);

	if(msglen.len == 0){
		return 1;
	}

	tmp = (void *)malloc(msglen.len+1);
	if(tmp == NULL){
		perror("cmd malloc error");
		return -1;
	}
	memset(tmp, 0, msglen.len+1);

	usleep(2000);
	size = recv(fd, (void *)tmp, msglen.len, MSG_DONTWAIT);
	if(size < 0){
		perror("unix socket rcv");
		free(tmp);
		return -1;
	}else if(size == 0){
		printf("socket closed!!!");
		free(tmp);
		return 0;
	}

	*buf = tmp;

	return size;
}

static int rcv_function(struct select_ctrl_s *handle){
	char *cmd = NULL;
	void *val = NULL;
	int ret = -1;

	ret = receive_val(handle->fd, (void **)&cmd, CMDLEN_MARK);
	if(ret > 0){
		printf("cmd: %s\n", cmd);
		ret = receive_val(handle->fd, (void **)&val, VALLEN_MARK);
		if(ret > 0){
			if(val) printf("val: %s\n", (char *)val);
			do_function(cmd, val, handle);
		}
	}

	if(cmd) free(cmd);
	if(val)	free(val);
	return ret;
}

static void *htmlsettingCmdLooper(void *arg){
	int ret = -1, listen_fd = htmlsetting_fd;
	uid_t uid;
	struct select_ctrl_s select_ctrl;

	htmlsetting_priv.running = 1;
	select_ctrl.fd = listen_fd;
	select_ctrl.priv_time = time(NULL);

	while(select_ctrl.fd >= 0){
		int maxFd = -1;
		fd_set readFds;
		struct timeval timeout;

		pthread_testcancel();
		if (htmlsetting_priv.exit)
		{
			if(select_ctrl.fd != listen_fd){
				unix_socket_close(select_ctrl.fd);
				unix_socket_close(listen_fd);
			}else{
				unix_socket_close(select_ctrl.fd);
			}
			htmlsetting_priv.running = 0;
			pthread_exit(NULL);
			break;
		}
		
		FD_ZERO(&readFds);
		if (select_ctrl.fd >= 0)
		{
			FD_SET(select_ctrl.fd, &readFds);
			if (maxFd < select_ctrl.fd)
			{
				maxFd = select_ctrl.fd;
			}
		}
		
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;

		ret = select(maxFd + 1, &readFds, NULL, NULL, &timeout);
		if (ret == 0) //Check select timeout.
		{
			if(select_ctrl.fd != listen_fd){
				time_t nowT = time(NULL);
				if((nowT-select_ctrl.priv_time) > CLI_TIME_OUT){
					printf("[%s] [%d] -- SOCKET[%d] CLOSED!!\n", __func__, __LINE__, select_ctrl.fd);
					unix_socket_close(select_ctrl.fd);
					select_ctrl.fd = listen_fd;
					continue;
				}
			}
			
		}else if(ret > 0){
			if (FD_ISSET(listen_fd, &readFds))
			{
				select_ctrl.fd = unix_socket_accept(listen_fd, &uid);
				if(select_ctrl.fd < 0){
					perror("unix socket accept");
					select_ctrl.fd = listen_fd;
					continue;
				}
				select_ctrl.priv_time = time(NULL);
			}else if(FD_ISSET(select_ctrl.fd, &readFds)){
				ret = rcv_function(&select_ctrl);
				if(ret < 0){
					printf("[%s] [%d] -- Get cmd error!!\n", __func__, __LINE__);
				}else if(ret == 0){
					printf("[%s] [%d] -- SOCKET[%d] CLOSED!!\n", __func__, __LINE__, select_ctrl.fd);
					unix_socket_close(select_ctrl.fd);
					select_ctrl.fd = listen_fd;
				}
				select_ctrl.priv_time = time(NULL);
				continue;
			}
		}

	}

	htmlsetting_priv.running = 0;
	return NULL;
}

int htmlSettingStart(){
	int ret = -1;

	EZCASTLOG("-- htmlSettingStart\n");
	if(htmlsetting_fd < 0){
		htmlsetting_fd = unix_socket_listen(UNIX_SOCKET_PATCH);
	}

	if(htmlsetting_priv.running == 0){
		EZCASTLOG("-- Create pthread htmlsettingCmdLooper\n");
		htmlsetting_priv.exit = 0;
		ret = pthread_create(&htmlsetting_priv.looper, NULL, htmlsettingCmdLooper, NULL);
		if (ret != 0)
		{
			printf("%s/%d: pthread_create error(%d), FIXME!\n", __FUNCTION__, __LINE__, ret);
			ret = -1;
		}
	}

	return ret;
}

int htmlSettingStop(){
	int ret = -1;
	unix_socket_close(htmlsetting_fd);
	htmlsetting_fd = -1;
	htmlsetting_priv.exit = 0;

	
	ret = pthread_join(htmlsetting_priv.looper, NULL);
	if (ret != 0)
	{
		printf("%s/%d: pthread_join error(%d)\n", __FUNCTION__, __LINE__, ret);
	}
	return 0;
}

#endif

