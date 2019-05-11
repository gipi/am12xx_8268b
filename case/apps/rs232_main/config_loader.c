#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rs232_main.h"

#define MAX_HASH_SIZE 200

int hash_calc(char *str)
{
    unsigned int hash = 438474637;	//1315423911; // nearly a prime - 1315423911 = 3 * 438474637
    int c;

    if(str==NULL){
        return -1;
    }

	while ((c = *str))
	{
		hash ^= ((hash << 5) + c + (hash >> 2));
        str++;
    }
    return (hash & 0x7FFFFFFF);
}

struct hash_data *g_hash_table[MAX_HASH_SIZE];
struct dehash_data *g_dehash_table[MAX_HASH_SIZE];
struct s_query *g_query_list_webcmd = NULL;
struct s_query *g_query_list_uartcmd = NULL;

int g_head[2];
int g_tail[2];

int *get_head(){
	int *head=g_head;
	return head;
}

int *get_tail(){
	int *tail=g_tail;
	return tail;
}

void print_hash(){
	int i=0;
	for(i=0;i<MAX_HASH_SIZE;i++){
		if(g_hash_table[i]!=NULL){
			printf("hash[%d] catelog=%x , item=%x , cmd=%s\n",i,g_hash_table[i]->catelog,g_hash_table[i]->item,g_hash_table[i]->cmd);
		}
	}
}

void print_dehash(){
	int i=0;
    for(i=0;i<MAX_HASH_SIZE;i++){
        if(g_dehash_table[i]!=NULL){
            printf("dehash[%d] catelog=%x , item=%x , hash=%i\n",i,g_dehash_table[i]->catelog,g_dehash_table[i]->item,g_dehash_table[i]->hash);
        }
    }
}

int getCmdValue(char* cmd,int *catelog,int *item){
	char *start = cmd;
	char *sep = NULL;

	if(cmd==NULL){
		printf("command is NULL");
		return -1;
	}else if(catelog==NULL){
		printf("catelog is NULL");
		return -1;
	}else if(item==NULL){
		printf("item is NULL");
		return -1;
	}
	char cate[15];
	char item_s[15];
	memset(cate,0,sizeof(cate));
	memset(item_s,0,sizeof(item_s));
	sep=strstr(cmd,"_");
	if(sep==NULL){
		printf("this is not a correct cmd format (cate-item) =%s\n",cmd);
		return -1;
	}
	memcpy(cate,cmd,sep-cmd);
	memcpy(item_s,sep+1,15);
	*catelog = hash_calc(cate);
	*item= hash_calc(item_s);
	return 0;
}
/*
struct hash_data *getHashByIndex(int index){
	struct hash_data *data=NULL;
	if(index<0 || index>=MAX_HASH_SIZE){
		printf("invalid index %d\n",index);
		return data;
	}
	data = g_hash_table[index];
	return data;
}

struct dehash_data *getDehashByIndex(int index){
	struct dehash_data *data=NULL;
    if(index<0 || index>=MAX_HASH_SIZE){
        printf("invalid index %d\n",index);
        return data;
    }
    data = g_dehash_table[index];
	return data;
}
*/

struct hash_data *getHashByStrCmd(char *cmd){
	struct hash_data *data=NULL;
	int cate;
	int item;
	int ret = getCmdValue(cmd,&cate,&item);
	if(ret==-1){
		return data;
	}
	int hash = hash_calc(cmd)%MAX_HASH_SIZE;
	printf("%s, %s, %d\n",__FILE__, __func__, __LINE__);
	printf("cmd=%s,hash=%d cate=%x,item=%x\n",cmd,hash,cate,item);
	data = g_hash_table[hash];
	int step;
	for(step=0;step<MAX_HASH_SIZE;step++){
		if(data!=NULL && data->catelog==cate && data->item==item){
			break;
		}
		hash = (hash+1)%MAX_HASH_SIZE;
		data = g_hash_table[hash];
	}
	return data;
}

struct hash_data *getHashFromDehash(char *buf){
	printf("%s cmd:%s\n",__func__,buf);
	int i;
	for (i=0; i < strlen(buf); i++)
		printf("%02x ",buf[i]);
	printf("\n");
	int	value = hash_calc(buf);
	int dehash = value%MAX_HASH_SIZE;
	int step;
	struct hash_data *data=NULL;
	for(step=0;step<MAX_HASH_SIZE;step++){
		if( g_dehash_table[dehash]!=NULL && g_hash_table[g_dehash_table[dehash]->hash]!=NULL &&
			g_dehash_table[dehash]->catelog==g_hash_table[g_dehash_table[dehash]->hash]->catelog &&
			g_dehash_table[dehash]->item==g_hash_table[g_dehash_table[dehash]->hash]->item &&
			g_dehash_table[dehash]->value==value)
		{
			data = g_hash_table[g_dehash_table[dehash]->hash];
			break;
		}
		dehash = (dehash+1)%MAX_HASH_SIZE;
	}
	return data;
}



int load_config(char *path){
	FILE *fd=NULL;
	struct s_query *query, *tmp;
	int query_count = 0;
	
	fd = fopen(path,"r");
	if(fd==NULL){
		printf("file open error\n");
		return -1;
	}

	fseek(fd,0,SEEK_END);
	long size = ftell(fd);
	printf("size=%ld\n",size);
	fseek(fd,0,SEEK_SET);
	if(size==0){
		printf("file open error\n");
		close(fd);
		return -1;
	}
	char *buf=NULL;
	buf=(char *)malloc(sizeof(char)*size);
	fread(buf,sizeof(char),size,fd);
	close(fd);

	char v1[15]={0};
	char v2[15]={0};
	char cmd[30]={0};
	char value[30]={0};
	
	char *endline=NULL,*comment=NULL,*seperate=NULL,*cmd_end=NULL;
	endline = strtok(buf,"\n");
	int ret=-1;
	printf("%s, %s, %d\n",__FILE__, __func__, __LINE__);
	while(endline!=NULL){
		
		seperate = strstr(endline,"&");
		cmd_end = strstr(endline,";");
		comment = strstr(endline,"#");
		if(seperate==NULL || cmd_end==NULL || (comment!=NULL && (comment < seperate || comment < cmd_end))){
			printf("this is not a correct command = %s\n",endline);
			goto GET_NEXT_CMD;
		}
		if(seperate > cmd_end){
            printf("this is not a correct command = %s\n",endline);
            goto GET_NEXT_CMD;
		}
		strncpy(cmd,endline,seperate-endline);
		strncpy(value,seperate+1,cmd_end-seperate-1);
		
		if(strcmp(cmd,"head")==0){
			char *comma;
			comma = strstr(value,",");
			strncpy(v1,value,comma-value);
			strcpy(v2,comma+1);
			g_head[0] = atoi(v1);
			g_head[1] = atoi(v2);
			//printf("head = %x %x\n",g_head[0],g_head[1]);
			memset(v1,0,15);
			memset(v2,0,15);
		}else if(strcmp(cmd,"tail")==0){
            char *comma;
            comma = strstr(value,",");
            strncpy(v1,value,comma-value);
           	strcpy(v2,comma+1);
           	g_tail[0] = atoi(v1);
           	g_tail[1] = atoi(v2);
           	//printf("tail = %x %x\n",g_tail[0],g_tail[1]);
            memset(v1,0,15);
            memset(v2,0,15);
        }else {
			//Mos: Create g_query_list hash table
			//the table has 2 key to query, the 1st is uart command, when we get reply from projector, lookup table to get correct field
			//the 2nd key is web command, when we get query from cgi, lookup table to get correct response to reply
			if (strstr(value, "?") != NULL){
			query = (struct s_query*)malloc(sizeof(struct s_query));
			strncpy(query->uart_cmd, value, strlen(value));
			strncpy(query->web_cmd, cmd, strlen(cmd));
			//HASH_ADD_STR(g_query_list, uart_cmd, query);
			HASH_ADD(hh1, g_query_list_uartcmd, uart_cmd, strlen(query->uart_cmd), query);
			HASH_ADD(hh2, g_query_list_webcmd, web_cmd, strlen(query->web_cmd), query);
			}
			//printf("hash=0x%x cmd=%s , value=%s\n",hash_calc(cmd)%MAX_HASH_SIZE,cmd,value);
			//int hash = hash_calc(cmd)%MAX_HASH_SIZE;
			int hash = hash_calc(cmd);
			//printf("Mos: CMD, %s, hash, %d\n",cmd,hash);
			hash = hash % MAX_HASH_SIZE;
			//printf("cmd=%s, hash1=%d ",cmd,hash);
			
			int step;
			for(step=0;step<MAX_HASH_SIZE;step++){
				if(g_hash_table[hash]==NULL){
					break;
				}
				hash = (hash+1)%MAX_HASH_SIZE;
				
			}
			if(step==MAX_HASH_SIZE){
				printf("hash table is full\n");
				break;
			}
			ret=-1;
			g_hash_table[hash] = (struct hash_data *)malloc(sizeof(struct hash_data));
			memset(g_hash_table[hash],0,sizeof(struct hash_data));
			ret = getCmdValue(cmd,&g_hash_table[hash]->catelog,&g_hash_table[hash]->item);
			if(ret==-1){
				free(g_hash_table[hash]);
				goto GET_NEXT_CMD;
			}
			int cmdValue = hash_calc(value);
			g_hash_table[hash]->value = cmdValue;
			memcpy(g_hash_table[hash]->cmd,value,sizeof(value));
			//printf("hash=%d cate=%x item=%x value=%x cmd=%s\n",hash,g_hash_table[hash]->catelog,g_hash_table[hash]->item,g_hash_table[hash]->value,g_hash_table[hash]->cmd);
			if (strstr(g_hash_table[hash]->cmd, "hdmi") != NULL){
			int i;
			for (i=0; i < strlen(buf); i++)
				printf("%02x ",buf[i]);
			printf("\n");
			}
			int dehash = hash_calc(value)%MAX_HASH_SIZE;
			int destep;
			for(destep=0;destep<MAX_HASH_SIZE;destep++){
                if(g_dehash_table[dehash]==NULL){
					break;
				}
         		dehash=(dehash+1)%MAX_HASH_SIZE;
                
            }
            if(destep==MAX_HASH_SIZE){
                printf("hash table is full");
                break;
            }
            g_dehash_table[dehash] = (struct dehash_data *)malloc(sizeof(struct dehash_data));
			memset(g_dehash_table[dehash],0,sizeof(struct dehash_data));
			g_dehash_table[dehash]->catelog = g_hash_table[hash]->catelog;
			g_dehash_table[dehash]->item = g_hash_table[hash]->item;
			g_dehash_table[dehash]->hash = hash;
			g_dehash_table[dehash]->value = cmdValue;
		}

		GET_NEXT_CMD:
	    memset(cmd,0,30);
	    memset(value,0,30);
	
		endline=strtok(NULL,"\n");
	}
	//print_hash();
	//print_dehash();
	for(query=g_query_list_uartcmd; query != NULL; query=query->hh1.next) {
      // printf("uartcmd: %s, webcmd: %s, result: %s\n", query->uart_cmd, query->web_cmd, query->query_result);
    }
	
	free(buf);
	free(query);
	
	return 0;
}

