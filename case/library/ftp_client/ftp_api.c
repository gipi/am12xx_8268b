#include "ftp.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/msg.h>
#include <dlfcn.h>
#include "ipc_key.h"
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "am_types.h"
#include "sys_cfg.h"

#define DEBUG    0
#define PORT    21
#define FTP_SHM_KEY	(APP_KEY_BASE + 0x20)

char* ip_addr=NULL;
char* usr=NULL;
char* usr_psd=NULL;
sem_t v_ftp;
sem_t v_ls;
char msg_content[128];
char *rbuf=NULL;

void _ftp_sem_wait(sem_t * sem)
{
	int err;

__PEND_REWAIT:
	err = sem_wait(sem);
	if(err == -1){
		int errsv = errno;
		if(errsv == EINTR){
			//Restart if interrupted by handler
			goto __PEND_REWAIT;	
		}
		else{
			printf("work_sem_pend: errno:%d\n",errsv);
			return;
		}
	}

	return;
	
}
void _ftp_sem_post(sem_t * sem)
{
	int err;
	err = sem_post(sem);
}

void *ftp_shmget(int size,int key)
{
	int shmid = -1;
	void *shbuf;

	if(size)
		shmid = shmget(key, size, IPC_CREAT |IPC_EXCL| 0666);
	if (!size || shmid == -1)
	{
		printf("shmget error,size:%d shmid:%d",size,shmid);
		return NULL;
	}

	shbuf = (void *)shmat(shmid, NULL, 0);
	if (shbuf == (void*) -1L) 
	{
		printf("shmget shbuf error,size:%d shmid:%d",size,shmid);
		return NULL;
	}
	memset(shbuf, 0,size);
	return shbuf;
}

int ftp_shmrm(int key)
{
	void *shbuf;
	int shmid = -1; 

	shmid = shmget(key, 0, 0);
	if (shmid == -1)
	{
		printf("shmget error,shmid:%d",shmid);
		return -1;
	}

	shbuf = (void *)shmat(shmid, NULL, 0);
	if (shmdt(shbuf) == -1) 
	{
		printf("shmdt failed");
		return -1;
	}
	if (shmctl(shmid, IPC_RMID, 0) == -1)
	{	
		printf("IPC_RMID error,shmid:%d",shmid);
		return -2;
	}
	
	return 0;
}

int analy_path_info(ftp_ls_info *lsinfo,char* recvBuf)
{
	int offset=0;
	int i=0;
	int j=0;
	if(recvBuf[0]==0)
		return -1;
	while(recvBuf[offset]!='\0')
	{
		if(recvBuf[offset]==0x0D&&recvBuf[offset+1]==0x0A)
			lsinfo->itemnum++;
		offset++;
	}
	p_printf("analy_path_info %d   %d\n",__LINE__,lsinfo->itemnum);
	lsinfo->pinfo = (path_info*)malloc(lsinfo->itemnum*sizeof(path_info));
	memset(lsinfo->pinfo,0x00,sizeof(path_info)*lsinfo->itemnum);
	offset = 0;
	while(recvBuf[offset]!='\0')
	{
		p_printf("analy_path_info %d\n",__LINE__);
		j=0;
		memcpy(lsinfo->pinfo[i].rwmode,recvBuf+offset,10);
		offset +=10;
		if(lsinfo->pinfo[i].rwmode[0]=='d')
			lsinfo->pinfo[i].pathtype = 1;
		else
			lsinfo->pinfo[i].pathtype = 0;

		while(recvBuf[offset]==0x20)
			offset++;
		if(recvBuf[offset]=='2'||recvBuf[offset]=='1')
			offset++;
		if(recvBuf[offset]==0x20)
			offset++;
		if(recvBuf[offset]=='0')
			offset++;
		while(recvBuf[offset]==0x20)
			offset++;
		if(recvBuf[offset]=='0')
			offset++;
		while(recvBuf[offset]==0x20)
			offset++;
		while(recvBuf[offset]!=0x20)
		{
			lsinfo->pinfo[i].filesize[j] = recvBuf[offset];
			offset++;
			j++;
		}
		while(recvBuf[offset]==0x20)
			offset++;
		memcpy(lsinfo->pinfo[i].createdate,recvBuf+offset,12);
		offset +=12;

		while(recvBuf[offset]==0x20)
			offset++;
		j=0;

		while(recvBuf[offset]!=0x0D)
		{
			lsinfo->pinfo[i].filename[j] = recvBuf[offset];
			offset++;
			j++;
		}
		p_printf("filebame==%s\n",lsinfo->pinfo[i].filename);	
		while(recvBuf[offset]==0x0D||recvBuf[offset]==0x0A)
			offset++;

		i++;
		p_printf("analy_path_info %d\n",__LINE__);
		
	}
	
	
	return 0;
}



EXPORT_SYMBOL
int ftpclient_connect(char * ipaddr, char* usrname,char* psd)
{
	char app_path[50];
	pthread_t thread_id;
	int err;

	p_printf("ftpclient_connect \n");
	sprintf(app_path,"%s/%s","sh ./am7x/case/scripts","config_wifi.sh");

	memset(rbuf,0x00,100*1024);
	system(app_path);
	err = sem_init(&v_ftp,0,0);
	err = sem_init(&v_ls,0,0);
	if(err<0)
		printf("sem init error\n");
	
	ip_addr = ipaddr;
	usr = usrname;
	usr_psd = psd;

	err = pthread_create (&thread_id,NULL,(void*)ftp_entry,NULL);
	if(err<0)
	{
		p_printf("create ftp thread error\n");
		return -1;
	}
	
	return 0;
	
}

EXPORT_SYMBOL
ftp_ls_info* ftpclient_get_list(void)
{
	ftp_ls_info *lsinfo;
	char content[128];
	p_printf("ftpclient_get_list\n");
	memset(content,0x00,128);
	memset(msg_content,0x00,128);
	sprintf(content,"%s","ls");

	if(rbuf==NULL)
		rbuf = (char*)malloc(100*1024);
	memcpy(msg_content,content,sizeof(content));

	_ftp_sem_post(&v_ftp);
	_ftp_sem_wait(&v_ls);

	lsinfo = (ftp_ls_info*)malloc(sizeof(ftp_ls_info));
	memset(lsinfo,0x00,sizeof(ftp_ls_info));
	if(analy_path_info(lsinfo, rbuf)<0)
		return 0;

	free(rbuf);
	rbuf = NULL;
	return lsinfo;
	
}
EXPORT_SYMBOL
int ftpclient_enter_dir(char* dir)
{
	char content[128];
	p_printf("ftpclient_enter_dir\n");
	
	memset(content,0x00,128);
	sprintf(content,"%s","cd ");
	strcat(content,dir);
	memcpy(msg_content,content,sizeof(content));
	_ftp_sem_post(&v_ftp);

	_ftp_sem_wait(&v_ls);
	return 0;
	
}
EXPORT_SYMBOL
int ftpclient_exit_dir(void)
{
	char content[128];
	p_printf("ftpclient_exit_dir\n");

	memset(content,0x00,128);
	sprintf(content,"%s","cd ..");
	memcpy(msg_content,content,sizeof(content));
	_ftp_sem_post(&v_ftp);
	
	_ftp_sem_wait(&v_ls);
	return 0;
	
}
EXPORT_SYMBOL
int ftpclient_get_file(char* filename)
{
	char content[128];
	p_printf("ftpclient_get_file\n");
	
	memset(content,0x00,128);
	sprintf(content,"%s","get ");
	strcat(content,filename);
	memcpy(msg_content,content,sizeof(content));
	_ftp_sem_post(&v_ftp);
	
	_ftp_sem_wait(&v_ls);
	return 0;
	
}
EXPORT_SYMBOL
int ftpclient_close(void)
{
	char content[128];
	p_printf("ftpclient_close\n");

	memset(content,0x00,128);
	sprintf(content,"%s","bye");
	memcpy(msg_content,content,sizeof(content));
	_ftp_sem_post(&v_ftp);
	
	_ftp_sem_wait(&v_ls);
	return 0;
}

