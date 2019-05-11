#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include "shell_opt.h"

static int rpipe, wpipe;

static void *do_system_pthread(void *arg){
	char retVal[8];
	int *ret = (int *)&retVal[1];
	
	if(arg == NULL){
		EZCASTERR("arg NULL\n");
		return NULL;
	}

	EZCASTLOG("do command: %s\n", arg);
	*ret = system(arg);
	retVal[0] = 'r';

	if(wpipe >= 0){
		write(wpipe, retVal, sizeof(retVal));
	}
	
	return NULL;
}

static int do_system(char *cmd){
	int ret;
	pthread_t pid;
	
	if(cmd == NULL){
		EZCASTERR("cmd is NULL\n");
		return -1;
	}

	ret = pthread_create(&pid, NULL, do_system_pthread, cmd);
	if(ret != 0){
		EZCASTERR("Create pthread fail\n");
		return -1;
	}
	pthread_detach(pid);
	
	return 0;
}

int main(int argc, char **argv)
{
	char buff[CMD_MAX_LEN+5];
	int ret;
	
	if(argc != 3){
		EZCASTERR("parameters error\n");
		exit(1);
	}

	rpipe = (int)strtol(argv[1],NULL,10);
	wpipe = (int)strtol(argv[2],NULL,10);
	EZCASTLOG("read pipe: %d, write pipe: %d\n", rpipe, wpipe);

	while(1){
		ret = read(rpipe, buff, sizeof(buff));
		if(ret <= 0){
			EZCASTERR("Read buff fail\n");
			continue;
		}

		if(buff[0] != 'l'){
			EZCASTERR("buff error\n");
			continue;
		}

		int len = (int)buff[1];
		EZCASTLOG("command length: %d\n", len);
		char *cmd = (char *)malloc(len+1);
		if(cmd == NULL){
			EZCASTERR("malloc fail\n");
			perror("ERROR");
			continue;
		}
		strncpy(cmd, &buff[5], len);
		cmd[len] = '\0';
		do_system(cmd);
	}
}
