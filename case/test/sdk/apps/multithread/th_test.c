#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

static void *thread1_routine(void * param)
{
	pid_t pid;

	pid = getpid();
	printf("thread1 pid == %d\n",pid);
	sleep(6);
	pthread_exit(NULL);
}

static void *thread2_routine(void *param)
{
	pid_t pid;

	pid = getpid();

	printf("thread2 pid == %d\n",pid);
	sleep(5);
	pthread_exit(NULL);
}

int main()
{
	int ret=0;
	pthread_t thread1,thread2;
	
	printf("main process pid == %d\n",getpid());
	ret = pthread_create (&thread1,NULL,thread1_routine,NULL) ;
	if(ret){
		printf("thread create fail %d\n",ret);
		goto EXIT;
	}
	ret = pthread_create (&thread2,NULL,thread2_routine,NULL) ;
	if(ret){
		printf("thread create fail %d\n",ret);
		goto EXIT;
	}
	
	ret = pthread_join(thread1,NULL);
	if(ret){
		printf("thread join fail %d\n",ret);
		goto EXIT;
	}
	ret = pthread_join(thread2,NULL);
	if(ret){
		printf("thread join fail %d\n",ret);
		goto EXIT;
	}

EXIT:
	exit(ret);
}


