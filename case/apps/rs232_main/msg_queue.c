#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <rs232_main.h>
#include <time.h>
#include <semaphore.h>

#define QUEUE_MAX_LEN 30

typedef struct{
	int catelog;
	int item;
	int value;
	struct timespec ts;
	char* buff[CMD_MAX_LEN];
}rs232_data;

typedef struct rs232_tok_t{
	rs232_data *data;
	struct rs232_tok_t *next;
}rs232_tok;

rs232_tok *rs232_msgq=NULL;
int rs232_msgq_size =0;

sem_t lock_get;
//pthread_mutex_t msgq_mutex;
pid_t get_thread;

void queue_sem_wait(sem_t *sem){
    int err;

__PEND_REWAIT:
    err = sem_wait(sem);
    if(err == -1){
        int errsv = errno;
        if(errsv == EINTR){
            //Restart if interrupted by handler
            printf("EINTR ????\n");
			goto __PEND_REWAIT;
		}else{
			printf("work_sem_pend: errno:%d\n",errsv);
		}
	}else{
		printf("err=%d %s\n",err,strerror(errno));
	}
}

void queue_sem_post(sem_t *sem){
	int err;
	err=sem_post(sem);
}

int cancel_msg(int catelog,int item,int value){
	rs232_tok **tok = &rs232_msgq;
	if(*tok==NULL){
		printf("no msg in queue\n");
		return -1;
	}
	rs232_tok **pre = tok;
//	pthread_mutex_lock(&msgq_mutex);
	while(*tok!=NULL){
		if((*tok)->data->catelog==catelog && (*tok)->data->item==item && (*tok)->data->value==value){
			printf("find msg and cancel it catelog=%x , item=%x , value=%x",catelog,item,value);
			if(*tok!=rs232_msgq){
				(*pre)->next = (*tok)->next;
			}
			free((*tok)->data);
			free(*tok);
			*tok=NULL;
			break;
		}
    	pre=tok;
        tok=&((*tok)->next);
	}
//	pthread_mutex_unlock(&msgq_mutex);
	return 0;
}

int cancel_msg_by_buff(char *buff){
	int ret = 0;
	rs232_tok **tok = &rs232_msgq;
	if(*tok==NULL){
		printf("no msg in queue\n");
		return 0;//-1;
	}
	rs232_tok **pre = tok;
//	pthread_mutex_lock(&msgq_mutex);
	while(*tok!=NULL){
		if(strcmp((*tok)->data->buff,buff) == 0){
			printf("find msg and cancel it catelog=%x , item=%x , value=%x",(*tok)->data->catelog,(*tok)->data->item,(*tok)->data->value);
			if(*tok!=rs232_msgq){
				(*pre)->next = (*tok)->next;
			}
			free((*tok)->data);
			free(*tok);
			*tok=NULL;
			ret = 1;
			break;
		}
    	pre=tok;
        tok=&((*tok)->next);
	}
//	pthread_mutex_unlock(&msgq_mutex);
	return ret;
}

int put_msg(char *buff,int ms,int catelog,int item,int value){
	if(rs232_msgq_size==QUEUE_MAX_LEN){
		printf("msg queue is full , drop this message\n");
		return -1;
	}
	printf("put msg catelog=%x item=%x value=%x len=%d\n",catelog,item,value,strlen(buff));
	int idx;
	for(idx=0;idx<CMD_MAX_LEN;idx++){
		printf("[%x]",buff[idx]);
	}
	printf("\n");
	rs232_data* data = (rs232_data *)malloc(sizeof(rs232_data));
	memset(data->buff,0,CMD_MAX_LEN);
	memcpy(data->buff,buff,strlen(buff));
	data->catelog=catelog;
	data->item=item;
	data->value=value;
	struct timeval tv;
	gettimeofday(&tv);
	data->ts.tv_nsec=((ms*1000+tv.tv_usec)%1000000)*1000;
	data->ts.tv_sec=tv.tv_sec+(ms*1000+tv.tv_usec)/1000000;
	rs232_tok **tok = &rs232_msgq;
	rs232_tok *new_tok = (rs232_tok *)malloc(sizeof(rs232_tok));
	new_tok->data=data;
	new_tok->next=NULL;
	printf("%s %d\n",__func__,__LINE__);
//	pthread_mutex_lock(&msgq_mutex);
	if(*tok==NULL){
		//printf("no element");
		printf("put msg in empty %d.%d\n",data->ts.tv_sec,data->ts.tv_nsec);
		*tok=new_tok;
	}else{
		rs232_tok **pre = tok;
		while(*tok!=NULL){
			if((*tok)->data->ts.tv_sec>data->ts.tv_sec || ((*tok)->data->ts.tv_sec=data->ts.tv_sec && (*tok)->data->ts.tv_nsec>data->ts.tv_nsec)){
				new_tok->next=*tok;
				if(*pre==rs232_msgq){
					rs232_msgq=new_tok;
					printf("put msg at head %d.%d\n",data->ts.tv_sec,data->ts.tv_nsec);
				}else{
					new_tok->next = (*pre)->next;
					(*pre)->next=new_tok;
					printf("put msg at %d.%d\n",data->ts.tv_sec,data->ts.tv_nsec);
				}
				break;
			}
			pre=tok;
			tok=&((*tok)->next);
		}
		if((*tok)==NULL){
			(*pre)->next=new_tok;
			printf("put msg at tail %d.%d\n",data->ts.tv_sec,data->ts.tv_nsec);
		}
	}
//	pthread_mutex_unlock(&msgq_mutex);
	queue_sem_post(&lock_get);
	return 0;
} 

int replace_msg(char *buff,int ms,int catelog,int item,int value){
	int ret;
	cancel_msg(catelog,item,value);
	ret = put_msg(buff,ms,catelog,item,value);
	return ret;
}

void *get_msg(void *arg){
	//printf("msg thread create\n");
	rs232_data *data=NULL;
	rs232_tok *tok=NULL;
	struct timespec tv;
	while(1){
		//printf("thread wait %d\n",__LINE__);
		if(rs232_msgq==NULL){
			printf("thread wait %d\n",__LINE__);
			queue_sem_wait(&lock_get);
		}
		GET_NEW_CMD:
		if(rs232_msgq==NULL){
            continue;
        }
		printf("%s %d\n",__func__,__LINE__);
//		pthread_mutex_lock(&msgq_mutex);
		tok = rs232_msgq;
		data = tok->data;
		//gettimeofday(&tv);
		clock_gettime(CLOCK_REALTIME, &tv);
		//printf("msg time=%d.%ld real time=%d.%ld\n",data->ts.tv_sec,data->ts.tv_nsec,tv.tv_sec,tv.tv_nsec);
		if(tv.tv_sec==data->ts.tv_sec && (tv.tv_nsec>=data->ts.tv_nsec-50000000 && tv.tv_nsec<=data->ts.tv_nsec+50000000)){
			printf("msg get %s\n",data->buff);
        	rs232_msgq = rs232_msgq->next;
			//pthread_mutex_unlock(&msgq_mutex);
			uartNotify(data->buff);
			free(tok->data);
            free(tok);
		}else{
			if(tv.tv_sec > data->ts.tv_sec || ( tv.tv_sec == data->ts.tv_sec && tv.tv_nsec > data->ts.tv_nsec+50000000) ){
				rs232_msgq = rs232_msgq->next;
				free(tok->data);
				free(tok);
				printf("drop expired\n");
				//pthread_mutex_unlock(&msgq_mutex);
			}else{
				struct timespec ts;
				ts.tv_sec = data->ts.tv_sec;
				ts.tv_nsec = data->ts.tv_nsec;
				printf("wait time out %d,%ld\n",ts.tv_sec,ts.tv_nsec);
//				pthread_mutex_unlock(&msgq_mutex);
				int ret = sem_timedwait(&lock_get,&ts);
				if(ret==-1){
					printf("timedwait error = %s\n",strerror(errno));
				}else{
					printf("sem_timedwait finish %s(%d)\n",ret,strerror(errno));
				}
			}
		}
	}
	return 0;
}

int init_queue(int size){
    if(sem_init(&lock_get,0,0)==-1){
        printf("lock_get init error\n");
        return -1;
    }else{
		printf("sem init %s\n",strerror(errno));
	}
/*	
	if(pthread_mutex_init(&msgq_mutex,NULL)!=0){
		printf("msgq_mutext init error\n");
		return -1;
	}
*/
    pthread_create(&get_thread,NULL,get_msg,NULL);
	printf("%s\n",__func__);
    return 0;
}

