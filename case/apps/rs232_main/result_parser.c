#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "rs232_main.h"
char temp[CMD_MAX_LEN];

extern struct s_query *g_query_list_uartcmd; //hash table, define in config_loader.c
extern struct s_query *g_query_list_webcmd; //hash table, define in config_loader.c

int benq_result_parser(char *buf,int msg_fd){
	struct msg_t res;
	struct s_query *s, *item;
	memset(&res,0,sizeof(struct msg_t));
	int n_write=0;
	char *str_ptr;
	if(strcmp(buf,"timeout")==0){
		printf("timeout!!!\n");
		res.type=10;
		sprintf(res.buf,"timeout");
		n_write = msgsnd(msg_fd,&res,sizeof(struct msg_t),0);
		return n_write;
	}
	if(strcmp(buf,"fail")==0){
		printf("fail!!!\n");
        res.type=10;
        sprintf(res.buf,"fail");
        n_write = msgsnd(msg_fd,&res,sizeof(struct msg_t),0);
        return n_write;
	}
    struct hash_data *data=NULL;
	if(buf==NULL){
		printf("cmd is null\n");
		return -1;
	}

	int idx;
	printf("cmd get : ");
	for(idx=0;idx<CMD_MAX_LEN;idx++){
		printf("[%x]",buf[idx]);
	}
	printf("\n");

	if(buf[0]==0x3e && buf[1]==0x2a){	//this is cmd echo
		printf("%s, %s, %d\n",__FILE__, __func__, __LINE__);
    	data = (struct hash_data *)getHashFromDehash(&buf[2]);
    	if(data!=NULL){
        	printf("cate=0x%x , item=0x%x , cmd=%s\n",data->catelog,data->item,data->cmd);
			//reset 0.5 sec for fail
			replace_msg("fail",500,data->catelog,data->item,data->value);
    	}else{
        	printf("cannot find hash data %s\n",buf);
    	}
		//Mos: keep cmd echo to lookup query cache table
		memset(temp, '\0', CMD_MAX_LEN);
		str_ptr = strstr(&buf[2], "#");
		if (str_ptr != NULL){
			strncpy(temp, &buf[2], str_ptr-&buf[2]);
			printf("temp: %s\n",temp);		
			printf("strlen(temp):%d\n",strlen(temp));
		}
	}else if(buf[0]==0x2a){		//this is result
		printf("%s, %s, %d\n",__FILE__, __func__, __LINE__);
		printf("result is %s\n",&buf[1]);
		int len=strlen(&buf[1]);
		int i;
		char buff[CMD_MAX_LEN];
		memset(buff,0,CMD_MAX_LEN);
		for(i=0;i<len;i++){	//switch to lower case
			if(buf[i+1]>=0x41 && buf[i+1]<=0x5a){
				buff[i]=buf[i+1]+0x20;
			}else{
				buff[i]=buf[i+1];
			}
		}
		//Mos: get uart reply, if CMD echo exist in table, mean it's query command
		//Then we need update the table result, update twice cuz the hash table actually has 2 table to keep 2 hash key.
		//data = (struct hash_data *)getHashFromDehash(buff);
		printf("%s, %d, %s, size:%d\n",__FILE__,__LINE__,temp, strlen(temp));
		item = (struct s_query*)malloc(sizeof(struct s_query));
		memset(item , '\0', sizeof(struct s_query));

		printf("temp: %s\n",temp);

		HASH_FIND(hh1, g_query_list_uartcmd, temp, strlen(temp), s);
		if (s){
		printf("%s, %d, key found!\n",__FILE__,__LINE__);
		memcpy(item, s, sizeof(struct s_query));
		strncpy(item->query_result, buff, strlen(buff));
		s = NULL;
		HASH_REPLACE(hh1, g_query_list_uartcmd, uart_cmd, strlen(item->uart_cmd), item, s);
		if (s == NULL) printf("%s, %d, hh1 not replaced!\n",__FILE__,__LINE__);
		s = NULL;
		HASH_REPLACE(hh2, g_query_list_webcmd, web_cmd, strlen(item->web_cmd), item, s);
		if (s == NULL) printf("%s, %d, hh2 not replaced!\n",__FILE__,__LINE__);
		}
		printf("%d ",__LINE__);
		memset(temp, '\0', CMD_MAX_LEN);
		printf("%d ",__LINE__);

		printf("%d ",__LINE__);
		if(cancel_msg_by_buff("timeout")){
			//printf("%s\n",buf);
			//printf("5. cate=0x%x , item=0x%x , cmd=%s\n",data->catelog,data->item,data->cmd);
			//cancel_msg(data->catelog,data->item,data->value);
	        res.type=10;
	        sprintf(res.buf,"%s",buff);
	        n_write = msgsnd(msg_fd,&res,sizeof(struct msg_t),0);
			if(n_write==-1){
				printf("error : %s(%d)\n",strerror(errno),errno);
			}
	        return n_write;
		}
/*		if(data!=NULL){
			printf("%s\n",buf);
			printf("5. cate=0x%x , item=0x%x , cmd=%s\n",data->catelog,data->item,data->cmd);
			cancel_msg(data->catelog,data->item,data->value);
	        res.type=10;
	        sprintf(res.buf,"%s",data->cmd);
	        n_write = msgsnd(msg_fd,&res,sizeof(struct msg_t),0);
			if(n_write==-1){
				printf("error : %s(%d)\n",strerror(errno),errno);
			}
	        return n_write;
		}*/
	}
	return 0;
}
