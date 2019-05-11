
#include <stdio.h>
#include "cgic.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

struct msg_t{
	long int type;
	char buf[1024];
	char NonCached[2];
};

int cgiMain(){
	
	char cmdStr[20];
	char value[20];
	char nc[2];
	memset(cmdStr,0,sizeof(cmdStr));
	memset(value,0,sizeof(value));
	cgiFormString("cmd",cmdStr,sizeof(cmdStr));
	cgiFormString("value",value,sizeof(value));
	cgiFormString("nc",nc,sizeof(nc));

	 cgiHeaderContentType("text/plain");

	int read_fd=-1;
	int write_fd=-1;
	write_fd = msgget(500,IPC_CREAT);
	if(write_fd==-1){
		fprintf(cgiOut,"<result>write msg get error</result>");
		return 0;
	}
	
	struct msg_t tmsg;
	memset(&tmsg,0,sizeof(struct msg_t));
	tmsg.type=110;
	sprintf(tmsg.buf,"%s_%s",cmdStr,value);
	sprintf(tmsg.NonCached,"%s",nc);
	
	int wret = msgsnd(write_fd,&tmsg,sizeof(struct msg_t),0);
	if(wret==-1){
		fprintf(cgiOut,"<result>send msg error = %s(%d)</result>",strerror(errno),errno);
		return 0;
	}

	read_fd = msgget(501,IPC_CREAT);
	if(read_fd==-1){
		fprintf(cgiOut,"<result>read msg get error</result>");
		return 0;
	}

	memset(&tmsg,0,sizeof(struct msg_t));
	
	int rret = msgrcv(read_fd,&tmsg,sizeof(struct msg_t),0,0);
	
	if(rret==-1){
		return -1;
	}else{
		fprintf(cgiOut,"<result>%s</result>",tmsg.buf);
	}

    return 0;
}

