///< if you want to format the udisk, using udisk_format_d() or udisk_format_d()
///< if you want to send some message to the key driver, use send_msg_to_keydriver()
#if 0     


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/statfs.h>
#include <linux/input.h>
#define CONTENT_BYTES (4*1024)
#define uformat_info(fmt,arg...) printf("MINF[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)
#define uformat_err(fmt,arg...)  printf("MERR[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)



static int udisk_check_busy()
{
	int rtn=0;
	char str_cmd_format[]="umount /mnt/udisk";
	rtn = system(str_cmd_format);
	uformat_info("Udisk umount Return value=%d\n",rtn);
	return rtn;
}

static int udisk_format_with_mkfs()
{
	int rtn=0;
	char str_cmd_format[]="mkfs.vfat /dev/partitions/udisk";
	rtn = system(str_cmd_format);
	uformat_info("Udisk vfat Return value=%d\n",rtn);
	return rtn;
}

static int mount_udisk_again()
{
	int rtn=0;
	char str_cmd_format[]="mount /dev/partitions/udisk /mnt/udisk";
	rtn = system(str_cmd_format);
	uformat_info("Udisk mount Return value=%d\n",rtn);
	return rtn;
}

static long _get_udisk_space(char * path, int type)
{
	struct statfs fsstatus;
	long space;
	long blocks;

	if(statfs(path, &fsstatus)==0){
		
		if(type == 0){
			blocks = fsstatus.f_blocks;
		}
		else{
			blocks = fsstatus.f_bfree;
		}
		printf("Bsize=0x%x,blocks=0x%x\n",fsstatus.f_bsize,blocks);
		return blocks*fsstatus.f_bsize;
	}
	else{
		return -1;
	}
}


///< format the udisk by writing the 00 to the sectors, make sure that the udisk isn't used
int udisk_format_d()
{
	char udisk_dev_path[]="/dev/partitions/udisk";
	int fd_udisk=0;
	int rtn=0,bytes_write=0;
	unsigned int total_write=0;
	unsigned int udisk_size=0;
	struct stat udisk_info;
	char *fill_content=(char*)SWF_Malloc(CONTENT_BYTES);
	if(fill_content)
		memset(fill_content,0,CONTENT_BYTES);
	else{
		uformat_info("Malloc Failed!");
		rtn = -1;
		goto UDISK_FORMAT_D_END;
	}
	if(udisk_check_busy()!=0){
		rtn = -1;
		goto UDISK_FORMAT_D_END;
	}
	fd_udisk = open(udisk_dev_path,O_RDWR|O_NONBLOCK);
	if(fd_udisk==-1){
		uformat_info("Open File Error name=%s\n",udisk_dev_path);
		rtn = -1;
		goto UDISK_FORMAT_D_END;
	}
	else{
		udisk_size=(unsigned int)_get_udisk_space("/mnt/udisk",0);
		uformat_info("UdiskSize=0x%x",udisk_size);
		while(total_write<=udisk_size){
			bytes_write = write(fd_udisk,fill_content,CONTENT_BYTES);
			if(bytes_write==-1){
				uformat_info("Write End Or OK!\n");
				break;
			}
			else{
				total_write += bytes_write;
				uformat_info("Total Write==0x%x",total_write);
				continue;
			}
		}
		uformat_info("Format End!!!!!");
		close(fd_udisk);
		udisk_format_with_mkfs();
		mount_udisk_again();
		rtn = 0;
	}
UDISK_FORMAT_D_END:
	if(fill_content)
		SWF_Free(fill_content);
	return rtn;
}

//**format the udisk quickly, only the fat table be reset**/
int udisk_format_fast()
{
	int rtn=0;
	char str_cmd_format[]="/am7x/case/scripts/udisk_format.sh";
	rtn = system(str_cmd_format);
	uformat_info("Udisk Format Fast Return value=%d\n",rtn);
	return rtn;
}


int send_msg_to_keydriver()
{
	char key_dev_path[]="/dev/event0";
	struct input_event key_event;
	key_event.type = EV_PWR;
	key_event.code = 0;	///used your own code
	key_event.value = 1;	///< used your own value
	int bytes_write=0;
	int rtn = 0;
	int fd_key=0;
	fd_key = open(key_dev_path,O_RDWR|O_NONBLOCK);
	if(fd_key==-1){
		uformat_info("Open File Error name=%s\n",key_dev_path);
		rtn = -1;
		goto SEND_KEY_MSG_END;
	}
	else{
		bytes_write = write(fd_key,&key_event,sizeof(struct input_event));
		uformat_info("Bytes Write==%d",bytes_write);
		close(fd_key);
	}
		
SEND_KEY_MSG_END:
	return rtn;
}
#endif
