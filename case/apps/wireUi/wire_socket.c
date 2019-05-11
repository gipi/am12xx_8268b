#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/un.h>
#include <stddef.h>

#define BUFFSIZE 1024*16
#define DEFAULT_HOST_NAME	"127.0.0.1"
int tcp_socket_flag=0;
int socket_cmd_fd=0;
static pthread_t tcp_id;
static pthread_t rev_id;

int createAdbSocket(int port)
{
	int hSocket;                 /* handle to socket */
    struct sockaddr_in Address;  /* Internet socket address stuct */

    /* make a socket */
    hSocket=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(hSocket < 0)
    {
		printf("socket error\n");
		perror("ERROR");
        return -1;
    }

	/* fill address struct */
	Address.sin_addr.s_addr=inet_addr(DEFAULT_HOST_NAME);
	Address.sin_port=htons(port);
	Address.sin_family=AF_INET;

	/* connect to host */
	system("ifconfig lo up");
	if(connect(hSocket,(struct sockaddr*)&Address,sizeof(Address)) < 0)
	{
		printf("connect error\n");
		perror("ERROR");
		return -1;
	}

	return hSocket;
}

static void *start_adb_tcprev(void *arg)
{
	int port = 0;
	char buff[BUFFSIZE];
	int nRead = -1;
	int adbFd = -1;
	tcp_socket_flag=1;
	port = 55555;
	printf("test start_adb_tcprev.\n");
	adbFd = createAdbSocket(port);
	if(adbFd < 0)
	{
		printf("Create ADB socket fail.\n");
		goto __ERR__;
	}

	while (tcp_socket_flag)
	{
		memset(buff,0,sizeof(buff));
		nRead = read(adbFd, buff, sizeof(buff));
		if (nRead > 0)
		{
			ezMirrorDataHandle(buff, nRead);
		
		}
		else if(nRead < 0)
	      {
			perror("ERROR");
		
	    }
			
	}

__ERR__:
	if(adbFd >= 0)
	{
		close(adbFd);
	}
	pthread_exit(NULL);
	return 0;
}

int start_adb_tcprev_pthread()
{
	if (pthread_create(&tcp_id ,NULL, start_adb_tcprev, NULL) != 0)
	{
		printf("[%s]:start_adb_tcprev error\n",__FUNCTION__);
		exit(EXIT_FAILURE);
	}
	else
		printf("[%s]:start_adb_tcprev ok\n",__FUNCTION__);
	return 0;
}
int stop_adb_tcprev()
{
      printf("stop_adb_tcprev.\n");
      tcp_socket_flag=0;
	return 0;
}


static void *receive_cmd_loop(void *arg)
{
	fd_set rfd_set;
	int receivesize,nready;
	char receiveline[512];
	int i,max_fd,sockfd;
	socklen_t csize;
	int socket_back_cli_fd;
	 int dec_flag=0; 
	struct sockaddr_un un1;
	struct timeval timeout;
	max_fd=socket_cmd_fd;
	while(1)
	{   
	       FD_ZERO(&rfd_set);
		FD_SET(socket_cmd_fd, &rfd_set);	
		/** timeout each second for get the chance to check loop_exit */
		timeout.tv_sec = 3;
		timeout.tv_usec = 0;
		nready = select(max_fd+1,&rfd_set,NULL,NULL,&timeout);
		if(nready==0)//time out
			continue;
		csize=sizeof(un1);  
              socket_back_cli_fd=accept(socket_cmd_fd,(struct sockaddr*)&un1,&csize);  
		if(socket_back_cli_fd<0)
		{
			perror("ERROR");
			continue;
		}
		FD_SET(socket_back_cli_fd, &rfd_set);	
		if(socket_back_cli_fd>max_fd)
			max_fd=socket_back_cli_fd;
		 if (socket_back_cli_fd > 0)
		{
			if (FD_ISSET(socket_back_cli_fd, &rfd_set))
			{
				memset(receiveline,0,sizeof(receiveline));
				receivesize = read(socket_back_cli_fd, receiveline, sizeof(receiveline));
				if (receivesize > 0)
				{
					printf("cmd receiveline=%s\n", receiveline);
					if(strstr(receiveline,"startAdbSocket"))	
					{
					     int ret = wire_HantroOpen();
						if(ret < 0){
							printf("open player failed!\n");
						}
						else
							dec_flag=1;
					       start_adb_tcprev_pthread();
					}
					if(strstr(receiveline,"stopAdbSocket"))
					{
					      printf("dec_flag=%d!\n",dec_flag);
						if(dec_flag==1)
						{
							dec_flag=0;
							ezMirrorDataExit();
							printf("wire_HantroClose!\n");
							wire_HantroClose();
						}
						stop_adb_tcprev();
					}
					close(socket_back_cli_fd);
					FD_CLR(socket_back_cli_fd,&rfd_set);
					socket_back_cli_fd=-1;
				}
			}
		} 
		 else
		 	{
				perror("ERROR");
		 	}
		 
	}
	printf("pthread_exit start!!!!!!!!!!!!!!!!!!!!!%s,%d\n",__FUNCTION__,__LINE__);
	pthread_exit(NULL);
	return NULL;
}
//#include "wire_ota.h"
void DocmdSocketUnix(void)
{
	struct sockaddr_un uns;
	int size;
	uns.sun_family = AF_UNIX;
    	strcpy(uns.sun_path, "/tmp/cgi_setting_server");
	if ((socket_cmd_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
	{
		printf("[%s]: socket create error\n",__FUNCTION__);
		exit(EXIT_FAILURE);
	}
	/** before bind, must make sure the path file not exist */
	unlink(uns.sun_path);
	
	size = offsetof(struct sockaddr_un, sun_path) + strlen(uns.sun_path);
	if (bind(socket_cmd_fd, (struct sockaddr *)&uns, size) < 0)
	{
		printf("[%s]: bind error\n",__FUNCTION__);
		close(socket_cmd_fd);
		socket_cmd_fd = -1;
		exit(EXIT_FAILURE);
	}
	if (listen(socket_cmd_fd, 5) < 0)
	{
		printf("[%s]: listen error\n",__FUNCTION__);
		close(socket_cmd_fd);
		socket_cmd_fd = -1;
		exit(EXIT_FAILURE);
	}
	if (pthread_create(&rev_id ,NULL, receive_cmd_loop, NULL) != 0)
	{
		printf("[%s]:receive_cmd_loop create error\n",__FUNCTION__);
		close(socket_cmd_fd);
		socket_cmd_fd = -1;
		exit(EXIT_FAILURE);
	} 

}





