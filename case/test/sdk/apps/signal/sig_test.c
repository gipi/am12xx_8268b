#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys_msg.h>
#include <sys_timer.h>
#include <fcntl.h>
#include <signal.h>


int mainpid=0;
int t1pid=0;
int t2pid=0;
void sig_receive(int signum,siginfo_t *info,void *myact)
{
	printf("receive signal %d,pid=%d\n", signum,getpid());
	//sleep(5);
}


static void timer_isr1(int *pdata)
{
	kill(t1pid,SIGUSR1);
}

static void timer_isr2(int *pdata)
{
	kill(t2pid,SIGUSR2);
}

static void *thread1_routine(void * param)
{
	struct sigaction act;	
	int sig;
	sig=SIGUSR1;
	sigemptyset(&act.sa_mask);
	act.sa_flags=SA_SIGINFO;
	act.sa_sigaction=sig_receive;
	
	if(sigaction(sig,&act,NULL) < 0)
	{
		printf("install sigal 1 error\n");
	}

	t1pid  = getpid();
	printf("thread1 pid == %d\n",t1pid);
	while(1)
	{
		sleep(10);
	}
	pthread_exit(NULL);
}

static void *thread2_routine(void *param)
{
	struct sigaction act;	
	int sig;
	sig=SIGUSR2;
	sigemptyset(&act.sa_mask);
	act.sa_flags=SA_SIGINFO;
	act.sa_sigaction=sig_receive;
	
	if(sigaction(sig,&act,NULL) < 0)
	{
		printf("install sigal 1 error\n");
	}
	
	t2pid = getpid();

	printf("thread2 pid == %d\n",t2pid);
	while(1)
	{
		sleep(10);
	}
	pthread_exit(NULL);
}


int main()
{
	int fd1,fd2;
	volatile int count = 0;
	pthread_t thread1,thread2;

	fd1 = am_timer_create(5000, (void(*)(void*))timer_isr1, &count);
	fd2 = am_timer_create(6000, (void(*)(void*))timer_isr2, &count);
	printf("main thread pid == %d\n",getpid());
	
	pthread_create (&thread1,NULL,thread1_routine,NULL) ;
	pthread_create (&thread2,NULL,thread2_routine,NULL) ;
	
	while(1)
	{
		sleep(20);
	}

	pthread_join(thread1,NULL);
	pthread_join(thread2,NULL);
	if(fd1>0){
		am_timer_del(fd1);
	}

	if(fd2>0){
		am_timer_del(fd2);
	}
	exit(0);
}


