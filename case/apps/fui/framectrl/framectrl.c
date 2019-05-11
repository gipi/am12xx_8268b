#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/poll.h>
#include <sys/time.h>



int main(int argc, char *argv[])
{
	fd_set wfds,rfds;
	struct timeval tv;
	int retval;
	int filedes[2];
	char dummy=0;
	int maxfd;
	int framerate=30;
	int rdpipe,wrpipe;

	if(argc > 1){
		if(argc != 3){
			printf("error parameters\n");
			exit(1);
		}
		rdpipe = (int)strtol(argv[1],NULL,10);
		wrpipe = (int)strtol(argv[2],NULL,10);
	}
	else{
		printf("fui arg err\n");
		exit(1);
	}

	if(pipe(filedes) < 0){
		printf("framerate child process pipe error\n");
		exit(1);
	}

	close(filedes[1]);

	maxfd = filedes[0];

	if(rdpipe > maxfd){
		maxfd = rdpipe;
	}

	while(1){

		FD_ZERO(&wfds);
		FD_ZERO(&rfds);
		FD_SET(filedes[0], &wfds);
		FD_SET(rdpipe, &rfds);

		tv.tv_sec = 0;
		tv.tv_usec = 1000*(1000/framerate);

		retval = select(maxfd+1,&rfds,&wfds, NULL, &tv);

		if(retval==0){
			/** timeout happens*/
			write(wrpipe,(void *)&dummy,sizeof(char));
		}
		else if(retval > 0){
			if(FD_ISSET(rdpipe,&rfds)){
				int nread;
				int rate;
				/**
				* adjust the framerate.
				*/
				nread = read(rdpipe,&rate,sizeof(int));
				if(nread == sizeof(int)){
					framerate = rate;
					///printf("do auto framerate adjustment,%d\n",framerate);
				}
			}
		}
	}

	return 0;
}

