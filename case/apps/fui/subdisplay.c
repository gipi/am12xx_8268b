#ifdef MODULE_CONFIG_USB_SUBDISPLAY
#include "subdisplay.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/msg.h>
#include <errno.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <linux/string.h>
#include <linux/unistd.h>
#include <hantro_misc.h>
#include "sys_buf.h"
#include "display.h"
#include "sys_cfg.h"
#include "usb_subdisp.h"
#include "fui_common.h"
#include "swf_types.h"
#include "sys_msg.h"
#include "media_hotplug.h"

#if 0 
#define subdisp_info(fmt,arg...) 	printf("MINF[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)
#define subdisp_err(fmt,arg...) 		printf("MERR[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)
#else
#define subdisp_info(fmt,arg...) do{}while(0);
#define subdisp_err(fmt,arg...) do{}while(0);
#endif

static const char subdisp_name[] = "/dev/usb_subdisp";

extern unsigned long swf_heap_logic_start;
extern unsigned long swf_heap_physic_start;
static DE_config subdisp_de_env_saved;
static char env_saved_ok=0;
extern int usb_stat;
extern int usb_stat_next;

#if 1

/**
*@brief 	save de configure
*
*@param[in] NULL
*@retval 0	:success
*@retval -1	:failed
*/
int usb_display_save_env()
{
	void *deinst = NULL;
	de_init(&deinst);
	memset(&subdisp_de_env_saved, 0, sizeof(DE_config));
	if (deinst != NULL)
	{
		if (de_get_config(deinst, &subdisp_de_env_saved, DE_CFG_ALL) != DE_OK)
		{
			env_saved_ok = 0;
			return -1;
		}
		else
		{
			env_saved_ok = 1;
			return 0;
		}
	}
}

/**
*@brief 	restore de configure
*
*@param[in] NULL
*@return NULL
*/
void  usb_display_restore_env()
{
	void *deinst = NULL;
	de_init(&deinst);
	if (deinst != NULL)
	{
		if (env_saved_ok)
		{
			de_set_Config(deinst, &subdisp_de_env_saved, DE_CFG_ALL);
		}
	}
}

#endif

/**
*@brief 	start the subdisplay
*
*@param[in] env_info: the pointer to the structure of subdisp_env_info_t
*@retval 0	:success
*@retval -1	:failed
*@see the structure of subdisp_env_info_t
*/
int subdisplay_work_start(subdisp_env_info_t *env_info)
{
	int ret = 0;

	ret = ezFuiUsbStart(swf_heap_logic_start, swf_heap_physic_start, env_info->chipid, &fui_get_bus_address);
	if (ret < 0)
	{
		subdisp_err("init global info failed, ret:%d",ret);
	}

	return ret;
}


/**
*@brief 	end of  the subdisplay
*
*@param[in] NULL
*@retval 0	:success
*@retval -1	:failed
*/
int subdisplay_work_end()
{
	int ret;

	ret = ezFuiUsbStop();
	
	return ret;
}

int subdisplay_switchto_subdisplay()
{
	char app_path[50];
	subdisp_env_info_t env_info;
	int i=0;
	
	sprintf(app_path,"%s/%s %s",AM_CASE_SC_DIR,"usb_process.sh","dev_mass_2_subdisp");
	system(app_path);

	usb_display_save_env();
	if(SWF_GetState(SWF_GetActiveInst()) & SWF_STATE_ACTIVE){
		/** first set swf to sleep, and buffer will be released */
		SWF_SetState(SWF_GetActiveInst(), SWF_STATE_SLEEP);
	}
	return 0;
}
//bq :add for cdrom 
int identify_dev_loop()
{
	char call_buf[100];
	FILE *fp = NULL;
	static int lines_sector = 0;
	sprintf(call_buf,"fdisk -l /dev/loop0");
	if ((fp = popen(call_buf, "r")) != NULL)
	{
		while (fgets(call_buf, sizeof(call_buf), fp) != NULL) {
			++lines_sector;
		}
		pclose(fp);
		fp = NULL;
	}
	printf("<%s %d> lines_sector=%d\n", __FILE__, __LINE__, lines_sector);
	if (lines_sector == 0)
	{
		return 0;
	}
	else if (lines_sector == 6)
	{
		return 1;
	}
	return -1;
}
#define DEV_LOOP  1  //dev/loop0 vlid
int subdisplay_backfrom_subdispaly(unsigned int dataload)
{
	char app_path[50];
	int i=0;
	//int number;
	subdisp_info("Back From SubDisplay");
	subdisplay_work_end();
	
	//printf("debug---%s %d dataload=%02x\n",__FILE__,__LINE__,dataload);
	if(dataload==DEVICE_SUBDISP_2_MASS){
		//number=identify_dev_loop();
		#if 0
		//printf("<%s %d> number=%d\n",__FILE__,__LINE__,number);
		//if(number==DEV_LOOP){//bq
			char call_buf[150];
			sprintf(call_buf,"rmmod g_subdisplay.ko");
			system(call_buf);
			usleep(50);
			memset(call_buf,0,150);
			sprintf(call_buf,"insmod  /lib/modules/2.6.27.29/g_file_storage.ko file=/dev/partitions/udisk,/dev/sd_card_block,/dev/cf_card_block,/dev/loop0  cdrom=1  msg_identify=0");
			system(call_buf);
	//	}else{//bq
		#else
			memset(app_path,0,50);
			sprintf(app_path,"%s/%s %s",AM_CASE_SC_DIR,"usb_process.sh","dev_subdisp_2_mass");
			system(app_path);
		#endif
	//	}
		if(access("/sys/module/am7x_udc",F_OK)==0){
			usb_stat = HOTPLUG_STAT_IN;
		}else if(access("/sys/module/am7x_udc_next",F_OK)==0){
			usb_stat_next = HOTPLUG_STAT_IN;
		}
		//usb_stat = HOTPLUG_STAT_IN;
	}
	else if(dataload==DEVICE_SUBDISP_QUIT){
		if(access("/sys/module/am7x_udc",F_OK)==0){
			usb_stat = HOTPLUG_STAT_OUT;
		}else if(access("/sys/module/am7x_udc_next",F_OK)==0){
			usb_stat_next = HOTPLUG_STAT_OUT;
		}
		memset(app_path,0,50);
		sprintf(app_path,"%s/%s %s",AM_CASE_SC_DIR,"usb_process.sh","dev_subdisp_out");
		system(app_path);
		
		//_inform_application_device_out();
	}
	
	usb_display_restore_env();
	if((SWF_GetState(SWF_GetActiveInst()) & SWF_STATE_ACTIVE) == 0){
			SWF_SetState(SWF_GetActiveInst(), SWF_STATE_ACTIVE);
	}
	if(dataload==DEVICE_SUBDISP_QUIT){
		printf("reset state machine\n");
		_inform_application_device_out();
	}
	return 0;
}
#else
#include "subdisplay.h"
int subdisplay_work_start(subdisp_env_info_t *env_info)
{
	return 0;
}

int subdisplay_switchto_subdisplay()
{
	return 0;
}

int subdisplay_backfrom_subdispaly(unsigned int dataload)
{
	return 0;
}
#endif	/** MODULE_CONFIG_USB_SUBDISPLAY */
