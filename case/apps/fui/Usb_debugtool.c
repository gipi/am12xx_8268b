#ifdef MODULE_CONFIG_USB_DEBUG
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
#include <sys/wait.h>

#include "media_hotplug.h"
#include "sys_buf.h"
#include "display.h"
#include "sys_cfg.h"
#include "usb_debug.h"
#include "lcm_op.h"


extern int usb_stat;

#define USB_DEBUG_DEV	"/dev/usb_debug"
#define LCM_DEV			"/dev/lcm"
#define MEM_DEV			"/dev/mem"

#define act_writel(val,reg)  (*(volatile unsigned int *)(reg) = (val))
#define act_readl(port)  (*(volatile unsigned int *)(port))

#define     DE_Con              0xB0040000
#define     D_Color             0xB0040004
#define     DB_WR               0xB00400E4

#if 0
	#define tooldbg_info(fmt, arg...) printf("RECINF[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)
	#define tooldbg_err(fmt,arg...) printf("RECERR[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)
#else
	#define tooldbg_info(fmt, arg...)
	#define tooldbg_err(fmt,arg...)
#endif

int fd_usb = -1;
unsigned int vir_ori,off_size;
pid_t wait_pid;
struct _bulk_head return_bdata;

int PhyToLogic(int add,int fd)
{
	int count = 40;
	unsigned int vir_add,offset;
	tooldbg_info("add=0x%x",add);
	offset=add-(add&0xfffff000);
	off_size=offset+count*4;
	vir_ori=(int)mmap(0, off_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, add&0x1ffff000);
	vir_add=vir_ori+offset;
	return vir_add;
}

int unmap_mem()
{
	munmap((void*)vir_ori,off_size);
	vir_ori = 0;
	off_size = 0;
	return 0;
}

int check_cmd_info(struct _cmd_info *cmd)
{
	int ndata=0;
	int isrw=0;
	int ret=-1;
	int i=0;
	int info_size = sizeof(struct _bulk_head);
	unsigned char *buf = NULL;
	DE_INFO	de_info;
	int fd_lcm = -1;
	int fd_mem = -1;
	
	struct _bulk_head bdata = cmd->head;
	ndata = cmd->datalen;
	isrw = cmd->datadir;
	if(cmd->bulk_type==BULK_MULTI_DATA){
		buf = (unsigned char *)SWF_Malloc(ndata);
		if(buf==NULL){
			tooldbg_err("malloc data len=%d fail",ndata);
			return -1;
		}
		read(fd_usb,buf,ndata);
	}
	else if(cmd->bulk_type==BULK_SINGLE_DATA){
		ndata = 0;
	}
	else
		return 0;

	tooldbg_info("bulk_type is %d, ndata is %d, isrw is %d",cmd->bulk_type,ndata,isrw);

	tooldbg_info("info len is %d",bdata.infolen);

	#if 0
	for(i=0;i<bdata.infolen;i++){
		printf("%02x ",bdata.info[i]);
	}
	printf("\n");
	#endif

	tooldbg_info("cmd is %d",bdata.cmd_type);
	memset(&return_bdata,0,sizeof(struct _bulk_head));
	if(bdata.cmd_type==DEBUG_CMD_GAMMA || bdata.cmd_type==DEBUG_CMD_CBSH || bdata.cmd_type==DEBUG_CMD_SHARPNESS){
		fd_lcm = open(LCM_DEV,O_RDWR);
		if(fd_lcm<0){
			tooldbg_err("Sorry Open lcm cfg Error");
			return -1;
		}
		if(ioctl(fd_lcm,GET_DE_CONFIG,&de_info)<0){
			tooldbg_err("DE get config error");
			close(fd_lcm);
			return -1;
		}
	}
	else if(bdata.cmd_type==DEBUG_CMD_PATTERN || bdata.cmd_type==DEBUG_CMD_DFCOLOR || bdata.cmd_type==DEBUG_CMD_READREG || bdata.cmd_type==DEBUG_CMD_WRITEREG){
		fd_mem = open(MEM_DEV, O_RDWR | O_SYNC);
		if(fd_mem<0){
			tooldbg_err("Sorry Open /dev/mem Error");
			return -1;
		}
	}
	
	if(isrw==DATA_DIR_TO_HOST){
		/*for(i=0;i<ndata;i++){
			buf[i] = 0x55;
		}
		write(fd_usb,buf,ndata);*/
		ret =0;
	}
	else if(isrw==DATA_DIR_FROM_HOST){
		switch(bdata.cmd_type){
			case DEBUG_CMD_GAMMA:
			{
				de_info.GammaMode = 1;
				de_info.Gamma = (INT32U*)buf;

				if(ioctl(fd_lcm,GAMMA_ADJUST,&de_info)<0){
					tooldbg_err("DE set error");
					close(fd_lcm);
					return -1;
				}
				break;
			}
			case DEBUG_CMD_CBSH:
			{
				int *pCbsh = (int*)(buf);
				
				de_info.contrast = *pCbsh++;
				de_info.brightness = *pCbsh++;
				de_info.saturation = *pCbsh++;
				de_info.hue = *pCbsh;

				tooldbg_info("contrast is %d",de_info.contrast);
				tooldbg_info("brightness is %d",de_info.brightness);
				tooldbg_info("saturation is %d",de_info.saturation);
				tooldbg_info("hue is %d",de_info.hue);
				
				if(ioctl(fd_lcm,COLOR_ADJUST,&de_info)<0){
					tooldbg_err("DE set error");
					close(fd_lcm);
					return -1;
				}
				break;
			}
			case DEBUG_CMD_SHARPNESS:
			{
				int *pSharp = (int*)(buf);
				
				de_info.sharpness= *pSharp;
				tooldbg_info("sharpness is %d",de_info.sharpness);
				if(ioctl(fd_lcm,SHARPNESS_ADJUST,&de_info)<0){
					tooldbg_err("DE set error");
					close(fd_lcm);
					return -1;
				}
				break;
			}
			case DEBUG_CMD_PATTERN:
			{
				int *pPattern = (int*)(buf);
				int logic_addr = PhyToLogic(DE_Con,fd_mem);
				if(1==pPattern[0])
				{
					act_writel(act_readl(logic_addr)|(1<<12),logic_addr);			
				}
				else
				{
					//¹Ø±Õtest pattern£¬default color mode disable
					act_writel(act_readl(logic_addr)&(~(1<<12)),logic_addr);
				}
				unmap_mem();
				break;
			}
			case DEBUG_CMD_DFCOLOR:
			{
				int *pDecolor = (int*)(buf);
				act_writel(pDecolor[0],PhyToLogic(D_Color,fd_mem));
				unmap_mem();
				act_writel(1, PhyToLogic(DB_WR,fd_mem));
				unmap_mem();
				tooldbg_info("DefalultColor=0x%x",pDecolor[0]);
				break;
			}
		}
		
		#if 0
		for(i=0;i<ndata;i++){
			if((i%16)==0)
				printf("\n");
			printf("%02x ",buf[i]);
		}
		printf("\n");
		#endif
		
		ret = 0;
	}
	else if(isrw==DATA_DIR_NONE && ndata==0){
		switch(bdata.cmd_type){
			case DEBUG_CMD_READREG:
			{
				int *pReadreg = (int*)(bdata.info);
				int *pReturn_value = (int*)(return_bdata.info);
				int logic_addr = 0;
				for(i=0;i<bdata.infolen/4;i++){
					logic_addr = PhyToLogic(pReadreg[i],fd_mem);
					tooldbg_info("physical address is 0x%x",pReadreg[i]);
					pReturn_value[i] = act_readl(logic_addr);
					unmap_mem();
					tooldbg_info("read reg addr=0x%x, value is %d",pReadreg[i],pReturn_value[i]);
				}
				return_bdata.infolen = bdata.infolen;
				break;
			}
			case DEBUG_CMD_WRITEREG:
			{
				int *pWritereg = (int*)(bdata.info);
				for(i=0;i<bdata.infolen/4;i=i+2){
					tooldbg_info("write reg addr=0x%x, value is %d",pWritereg[i],pWritereg[i+1]);
					act_writel(pWritereg[i+1], PhyToLogic(pWritereg[i],fd_mem));
					unmap_mem();
				}
				break;
			}
			default:
			{
				tooldbg_info("parse bug");
				break;
			}
		}
		ret = 0;
	}
	if(buf){
		SWF_Free(buf);
		buf=NULL;
	}
	if(fd_lcm!=-1){
		close(fd_lcm);
		fd_lcm = -1;
	}
	if(fd_mem!=-1){
		close(fd_mem);
		fd_mem = -1;
	}
	
	return ret;
	
}

int create_usb_communication()
{
	pid_t pid1,pid2;
	int status;
	int fd[2];

	if(pipe(fd) < 0){
		tooldbg_err("pipe error");
		return -1;
	}
	pid1 = fork();
	if(pid1 == -1){
		tooldbg_err("task fork error");
		return -1;
	}
	else if(pid1 == 0){
		// first child process
		tooldbg_info("in child process first pid child=%d",getpid());
		pid2 = fork();
		if(pid2 == -1){
			tooldbg_err("task fork error");
			return -1;
		}
		else if(pid2 == 0){
			// second child process
			int ret;
			int byterw;
			pid_t pid_child;
			struct _cmd_info	cmd;
			tooldbg_info("in child process second pid child=%d",getpid());

			fd_usb = open(USB_DEBUG_DEV,O_RDWR);
			if(fd_usb<0){
				tooldbg_err("Sorry Open USB debug Error");
				return -1;
			}
			tooldbg_info("start usb debug protocol----");
			while(1){
				tooldbg_info("get cbwinfo---");
				byterw = ioctl(fd_usb,IOCTL_USB_DEBUG_GET_CMD,&cmd);
				if(byterw!=0){
					tooldbg_err("Sorry IOCTL_USB_DEBUG_GET_CMD Error");
					continue;
				}
				tooldbg_info("check cbwinfo----");
				if(check_cmd_info(&cmd)){
					tooldbg_err("Sorry check_cmd_info Error");
					continue;
				}
				tooldbg_info("sync csw to driver----");
				byterw = ioctl(fd_usb,IOCTL_USB_DEBUG_SET_CSW,&return_bdata);
				if(byterw!=0){
					tooldbg_err("Sorry IOCTL_USB_DEBUG_SET_CSW Error");
					continue;
				}
			}
			return 0;
		}
		else{
			wait_pid = pid2;
			close(fd[0]);
			write(fd[1],&wait_pid,sizeof(pid_t));
			exit(0);
		}
		
	}
	else{
		int wait_child = -1;
		int status;
		wait_child = waitpid(pid1,&status,0);
		if(wait_child == -1){
			tooldbg_err("wait for child error");
			return -1;
		}
		if(WIFEXITED(status)){
			tooldbg_err("normal termination");
		}	
		if(WIFSIGNALED(status)){
			tooldbg_err("signal exit");
		}
		if(WIFSTOPPED(status)){
			tooldbg_err("stopped");
		}
		close(fd[1]);
		read(fd[0],&wait_pid,sizeof(pid_t));
		tooldbg_info("wait pid =%d",wait_pid);
		return 0;
	}
	
	return -1;
}

int kill_usb_communication()
{
	char app_path[50];

	close(fd_usb);

	sprintf(app_path,"kill %d",wait_pid);
	tooldbg_info("app path is %s",app_path);
	system(app_path);
	
	return 0;
}

int mass_switchto_debug()
{
	char app_path[50]={0};
	int rc=0;
	sprintf(app_path,"%s/%s %s",AM_CASE_SC_DIR,"usb_process.sh","dev_mass_2_debug");
	rc = system(app_path);
	if(rc==0){
		usb_stat = HOTPLUG_STAT_OUT;
		_inform_application_device_out();
		create_usb_communication();
	}
	return rc;
}

int mass_backfrom_debug(unsigned int dataload)
{
	char app_path[50]={0};
	int rc=0;

	kill_usb_communication();
	
	if(dataload==DEVICE_DEBUG_2_MASS){
		sprintf(app_path,"%s/%s %s",AM_CASE_SC_DIR,"usb_process.sh","dev_debug_2_mass");
		rc = system(app_path);
		if(rc==0){
			usb_stat = HOTPLUG_STAT_IN;
			_inform_application_device_in();
		}
	}
	else if(dataload==DEVICE_DEBUG_QUIT){
		sprintf(app_path,"%s/%s %s",AM_CASE_SC_DIR,"usb_process.sh","dev_debug_out");
		rc = system(app_path);
		if(rc==0){
			usb_stat = HOTPLUG_STAT_OUT;
		}
	}
	
	return rc;
}
#else
int mass_switchto_debug()
{
	return 0;
}

int mass_backfrom_debug(unsigned int dataload)
{
	return 0;
}
#endif	/** MODULE_CONFIG_USB_DEBUG */