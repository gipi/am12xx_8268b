#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include "rs232_main.h"
#include "uartcom.h"
#include "pthread.h"
#include "sys_msg.h"


int read_fd=-1;
int write_fd=-1;

pthread_t pid=-1;

char timer_en=1;

extern struct s_query *g_query_list_uartcmd; //hash table, define in config_loader.c
extern struct s_query *g_query_list_webcmd; //hash table, define in config_loader.c
struct s_query *g_query_list_index = NULL;

void uartNotify(char *cmd){
	int ret = benq_result_parser(cmd,write_fd);
	printf("ret is %d\n",ret);
}

int sendCmdUart(char *data,int size){
	int fd=-1;
	fd = open("/dev/uartcom",O_RDWR);
	if(fd!=-1){
		int ret = write(fd,data,size);
		close(fd);
		return ret;
	}else{
		printf("uartcom open error (%s)\n",strerror(errno));
	}
	return -1;
}

int sendUart(char *input_cmd){
	char cmd[CMD_MAX_LEN];
	int uart_cmd_len;
	int *head=NULL;
	int *tail=NULL;
	int ret;

	uart_cmd_len = strlen(input_cmd);
	head=(int *)get_head();
	tail=(int *)get_tail();
	cmd[0]=head[0];
	cmd[1]=head[1];
	memcpy(&cmd[2],input_cmd,uart_cmd_len);
	cmd[2+uart_cmd_len]=tail[0];
	cmd[2+uart_cmd_len+1]=tail[1];
	cmd[4+uart_cmd_len]=0;
	ret = sendCmdUart(cmd,uart_cmd_len+4);
	if(ret!=-1){
		printf("write %d bytes\n",ret);
	}else{
		printf("send cmd to scaler error\n");
	}
	return ret;
}

void selectConfig(int i){
	load_config("/mnt/user1/rs232/1.cfg");

}


/*lemon add for fix init value NULL  bug*/


#define CMD_START_GETVAL 23
#define CMD_GET_INITVAL  24

char cmd_table[][20]={"pow=?","vol=?","mute=?","sour=?","appmod=?","ct=?","asp=?","pp=?","lampm=?","3d=?","rr=?","con=?","bri=?","color=?","sharp=?"};




int send_getvalcmd()
{
	int i=0;
	char buf[64]="";
	 int timeout_times=0;
	unsigned char head[2]={0x0d,0x2a};
	unsigned char tail[2]={0x23,0x0d};
	 
	for(i=0;i<15;i++)
	{	
		timeout_times=0;
		memset(buf,0,64);
		buf[0]=0x0d;
		buf[1]=0x2a;
		sprintf(&buf[2],"%s",cmd_table[i]);
		buf[2+strlen(cmd_table[i])]=0x23;
		buf[3+strlen(cmd_table[i])]=0x0d;
		buf[4+strlen(cmd_table[i])]='\0';
		printf("cmd[%d]%s\n",i,buf);
		sendCmdUart(buf,strlen(buf));
		
	}
	
	return 0;

}


int  send_start_getvalcmd()
{
	int uart=-1;
	char buff[64]="";
	uart = open("/dev/uartcom",O_RDWR);
	if(uart!=-1){
    	ioctl(uart,CMD_START_GETVAL,buff);
		close(uart);
		return 0;
	}
	return -1;

}

void *wait_initdata()
{
	int uart=-1;
	char buff[1024]="";
	while(1){
	uart = open("/dev/uartcom",O_RDWR);
	if(uart!=-1){
    	ioctl(uart,CMD_GET_INITVAL,buff);
		if(buff!=NULL && strstr(buff,"pow=?")){
			struct msg_t tmsg;
			tmsg.type=110;
			memcpy(tmsg.buf,buff,1024);
			int n_write = msgsnd(write_fd,&tmsg,sizeof(struct msg_t),0);
			printf("initdata=%s n_write=%d\n",buff,n_write);
			break;
		}
		
		close(uart);
		
	}
	usleep(40000);
	}

}
void *msg_sender(void* arg){
	int read_fd = *((int *)arg);
	//printf("read_fd=%d\n",read_fd);
	struct msg_t tmsg;
	struct hash_data *data = NULL;
	struct s_query *s;
	int ret=-1;
	int nread;
	struct msg_t res;
	int n_write=0;
	
	int *head=NULL;
	int *tail=NULL;
	int cmd_len=0;
	char cmd[CMD_MAX_LEN];
	
	load_config("/mnt/user1/rs232/1.cfg");
	printf("%s, %s, %d\n",__FILE__, __func__, __LINE__);
	//printf("msg_sender main loop start read_fd=%d\n",read_fd);
	pthread_t thread_id;
	while(1){
		nread=0;
		memset(&tmsg,0,sizeof(struct msg_t));
		nread = msgrcv(read_fd,&tmsg,sizeof(struct msg_t),0,0);
		if(nread==-1){
			printf("msg recv error %d(%s)\n",read_fd,strerror(errno));
			break;
		}
		printf("msg.buf=%s",tmsg.buf);
		if(strstr(tmsg.buf,"BENQINITVAL")){
			send_start_getvalcmd();
			send_getvalcmd();
			int rtn=pthread_create(&thread_id, NULL, wait_initdata, NULL);
			tmsg.NonCached=0;

		}
	
		
		//Mos: if cgi request command in query cache table, check flag to decide reply cache value or query directly
		printf("%s, %d, tmsg.buf: %s, tmsg.NonCached=%02x\n",__FILE__,__LINE__,tmsg.buf, tmsg.NonCached);
		HASH_FIND(hh2, g_query_list_webcmd, tmsg.buf, strlen(tmsg.buf), s);
		if (s && !tmsg.NonCached){
			/*
			printf("%s, %d, s->uart_cmd:%s\n",__FILE__,__LINE__,s->uart_cmd);
			printf("%s, %d, s->web_cmd:%s\n",__FILE__,__LINE__,s->web_cmd);
			printf("%s, %d, s->query_result:%s\n",__FILE__,__LINE__,s->query_result);
			memset(&res,0,sizeof(struct msg_t));
			res.type=10;
	        sprintf(res.buf,"%s",s->query_result);
			printf("%s, %d, res.type:%d\n",__FILE__,__LINE__,res.type);
			printf("%s, %d, res.buf:%s\n",__FILE__,__LINE__,res.buf);
	        n_write = msgsnd(write_fd,&res,sizeof(struct msg_t),0);
			if(n_write==-1){
				printf("error : %s(%d)\n",strerror(errno),errno);
			}
			*/
		}
		else{
			printf("1. %s, %s, %d, tmsg.buf=%s\n",__FILE__, __func__, __LINE__,tmsg.buf);
			data = (struct hash_data *)getHashByStrCmd(tmsg.buf);
			
			if(data!=NULL){
				cmd_len=strlen(data->cmd);
				printf("2. cate=0x%x , item=0x%x , cmd=%s , len=%d\n",data->catelog,data->item,data->cmd,cmd_len);
			}else{
				printf("get hash data error=%s\n",tmsg.buf);
				continue;
			}
			
			head=(int *)get_head();
			tail=(int *)get_tail();
			cmd[0]=head[0];
			cmd[1]=head[1];
			memcpy(&cmd[2],data->cmd,cmd_len);
			cmd[2+cmd_len]=tail[0];
			cmd[2+cmd_len+1]=tail[1];
			cmd[4+cmd_len]=0;
			
			//set 5sec timeout 
			put_msg("timeout",5000,data->catelog,data->item,data->value);

			ret = sendCmdUart(cmd,cmd_len+4);
			if(ret!=-1){
				printf("write %d bytes\n",ret);
			}else{
				printf("send cmd to scaler error\n");
			}
		}
		cmd_len=0;
		memset(cmd,0,CMD_MAX_LEN);
	}

	msgctl(read_fd,IPC_RMID,0);
	return 0;
}

//Mos: Signal user1 handler, list cache hash table, for debug use!,
//type console command "kill -16 $pid" to send signal
void sigusr1_handler(int signum){
	struct s_query *query;
	for(query=g_query_list_uartcmd; query != NULL; query=query->hh1.next) {
        printf("uartcmd: %s, webcmd:%s, result: %s\n", query->uart_cmd, query->web_cmd, query->query_result);
    }
}

//Mos: Signal user2 handler, switch alarm signal enable or not, for debug use!
//type console command "kill -17 $pid" to send signal
void sigusr2_handler(int signum){
struct itimerval timer;
	if (timer_en){
		memset(&timer, 0, sizeof(timer));
		timer.it_value.tv_sec = 0;  // sec
		timer.it_value.tv_usec = 0; // micro sec.
		timer.it_interval.tv_sec = 0;
		timer.it_interval.tv_usec = 0;
		setitimer(ITIMER_REAL, &timer, NULL);	
		timer_en = 0;
	}
	else{
		memset(&timer, 0, sizeof(timer));
		timer.it_value.tv_sec = 1;  // sec
		timer.it_value.tv_usec = 0; // micro sec.
		timer.it_interval.tv_sec = 1;
		timer.it_interval.tv_usec = 0;
		setitimer(ITIMER_REAL, &timer, NULL);	
		timer_en = 1;
	}
}

//Mos: Signal Alarm handler, loop to query cmd, and update cache hash table
static int sigalrm_handler_count = 0;
void sigalrm_handler(int signum)
{
	printf("sigalrm!");
	if (g_query_list_index == NULL)
		g_query_list_index = g_query_list_uartcmd;
	
	sendUart(g_query_list_index->uart_cmd);
	
	//Mos: Move to next item
	g_query_list_index = g_query_list_index->hh1.next;
	if (g_query_list_index == NULL)
		g_query_list_index = g_query_list_uartcmd;
}


int main(int argc,char *argv[]){

    int keyPipe;
    int msgq_t;
	int rPipe;
	int flags;
	
	struct itimerval timer;

    if(argc > 1){
        if(argc != 4){
            printf("error parameters %d\n",argc);
            return -1;
        }
        msgq_t = (int)strtol(argv[1],NULL,10);   //we don't need this
        rPipe = (int)strtol(argv[2],NULL,10);   //we don't need this
        keyPipe = (int)strtol(argv[3],NULL,10);
    }
    else{
        printf("rs232_main arg err\n");
        return -1;
    }
	/*
	flags = fcntl(keyPipe, F_GETFL,0);
	flags &= (~O_NONBLOCK);
    if (fcntl(keyPipe, F_SETFL, flags) < 0)
    {
        printf("set key pipe block failed!\n");
    }

    flags = fcntl(rPipe, F_GETFL,0);
    flags &= (~O_NONBLOCK);
   	if (fcntl(rPipe, F_SETFL, flags) < 0)
	{
    	printf("set r pipe block failed!\n");
	}
	*/

    /* create message queue */
    read_fd = msgget(READ_MSG_KEY, 0666|IPC_CREAT|IPC_EXCL);
    if(read_fd==-1){
        printf("read msg queue create error (%s)\n",strerror(errno));
        return -1;
    }
    write_fd = msgget(WRITE_MSG_KEY, 0666|IPC_CREAT|IPC_EXCL);
    if(write_fd==-1){
        printf("write msg queue create error (%s)\n",strerror(errno));
        msgctl(read_fd,IPC_RMID,0);
        return -1;
    }
	
    char buff[CMD_MAX_LEN];
    memset(buff,0,CMD_MAX_LEN);
    int max_select_fd = 0;

    if(rPipe > max_select_fd){
        max_select_fd = rPipe;
    }

    if(keyPipe > max_select_fd){
        max_select_fd = keyPipe;
    }

    fd_set event_set;
    FD_ZERO(&event_set);
    FD_SET(keyPipe,&event_set);
	FD_SET(keyPipe,&event_set);

	init_queue();
	
	int ret = pthread_create(&pid,NULL,msg_sender,&read_fd);

	//Mos: set timer to register sigalrm_handler
	//Disable by default
    memset(&timer, 0, sizeof(timer));
    // Timeout to run function first time
    timer.it_value.tv_sec = 30;  // sec
    timer.it_value.tv_usec = 0; // micro sec.
    // Interval time to run function
    timer.it_interval.tv_sec = 10;
    timer.it_interval.tv_usec = 0;
	/* Register Signal handler
	* And register for periodic timer with Kernel*/
	signal(SIGALRM, &sigalrm_handler);
	signal(SIGUSR1, &sigusr1_handler);
	signal(SIGUSR2, &sigusr2_handler);
	//setitimer(ITIMER_REAL, &timer, NULL);	
	
	
	struct am_sys_msg msg;
	int rdnr;
    while(1){
        ret = select(max_select_fd+1,&event_set,NULL,NULL,NULL);
        if(ret==0){
            printf("recive timeout receive=%d\n",keyPipe);
        }else if(ret > 0){
            if(FD_ISSET(keyPipe,&event_set)){
				rdnr = read(keyPipe,&msg,sizeof(struct am_sys_msg));
				int uart=-1;
				uart = open("/dev/uartcom",O_RDWR);
				if(uart!=-1){
                	ioctl(uart,CMD_GET_RESULT,buff);
					put_msg(buff,0,0,0,0);
					//uartNotify(buff,strlen(buff));
					close(uart);
					memset(buff,0,CMD_MAX_LEN);
				}else{
					printf("uartcom open failed , %s\n",strerror(errno));
				}
				memset(&msg,0,sizeof(struct am_sys_msg));
			}else if(FD_ISSET(keyPipe,&event_set)){
				printf("is r set");
            }else{
                printf("is not set");
            }
        }else{
            printf("select error %d\n",strerror(errno));
        }
    }
    msgctl(read_fd,IPC_RMID,0);
    msgctl(write_fd,IPC_RMID,0);
    close(keyPipe);
	close(msgq_t);
	close(rPipe);
}
