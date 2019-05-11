#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/msg.h>
#include <semaphore.h>
#include <errno.h>
#include <pthread.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <linux/string.h>

#include "sys_buf.h"
#include "display.h"
#include "sys_cfg.h"
#include "usb_debug.h"

#define DEBUG_MSG

#define USB_DEBUG_DEV	"/dev/usb_debug"

int check_cmd_info(struct _cmd_info *cmd,int fd)
{
	int ndata;
	int trans_type;
	int isrw;
	int ret;
	int i,j;
	int info_size;
	unsigned char *buf=NULL;
	
	struct _bulk_head	*bdata;
	if(cmd->bulk_type==BULK_MULTI_DATA){
		ndata = cmd->datalen;
		isrw = cmd->datadir;
	}else if(cmd->bulk_type==BULK_SINGLE_DATA){
		ndata = 0;
	}
	else
		return 0;

	if(ndata){
		info_size = sizeof(struct _bulk_head);
		buf = malloc(ndata+info_size);
		if(buf==NULL){
			printf("malloc data len=%d fail\n",ndata);
			return -1;
		}
		if(isrw==DATA_DIR_TO_HOST){
			for(i=0;i<ndata;i++){
				buf[i] = 0x55;
			}
			write(fd,buf,ndata);
			ret =0;
		}
		else if(isrw==DATA_DIR_FROM_HOST){
			read(fd,buf,ndata+20);
			bdata = (struct _bulk_head*)buf;
			for(i=0;i<ndata;i++){
				if((i%16)==0)
					printf("\n");
				printf("%02x ",buf[i+info_size]);
			}
			printf("\n");
			
			ret = 0;
		}
	}
	else
		ret =0;
	return ret;
	
}

int main(int argc,char* argv[])
{
	int ret;
	int byterw;
	int fd;
	struct _cmd_info	cmd;
	fd = open(USB_DEBUG_DEV,O_RDWR);
	if(fd<0)
		return -1;
	printf("start usb debug protocol----\n");
	while(1){
		printf("get cbwinfo---\n");
		byterw = ioctl(fd,IOCTL_USB_DEBUG_GET_CMD,&cmd);
		if(byterw!=0)
			continue;
		printf("check cbwinfo---\n");
		if(check_cmd_info(&cmd,fd))
			continue;
		printf("sync csw to driver---\n");
		byterw = ioctl(fd,IOCTL_USB_DEBUG_SET_CSW,NULL);
	}
	return 0;
}



