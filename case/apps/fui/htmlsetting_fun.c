#include <stdio.h>  
#include <errno.h>  
#include <stddef.h>  
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "htmlsetting.h"
#include "system_info.h"
#include "lcm_op.h"
#include "sys_cfg.h"
#include "apps_vram.h"
#include "display.h"

#ifdef	HTMLSETTING_ENABLE
#if 1
#define ediddbg_info(fmt,arg...) printf("MINF[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)
#else
#define ediddbg_info(fmt, arg...)
#endif
#if 1
#define sysdbg_info(fmt,arg...) printf("MINF[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)
#else
#define sysdbg_info(fmt, arg...)
#endif
#define sysdbg_err(fmt,arg...) printf("MERR[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)

#define SNAPSHOT_PATH		"/mnt/user1/thttpd/html/tmp/snapshot.jpg"
#define SNAPSHOT_TMP_PATH		"/mnt/user1/thttpd/html/tmp/snapshot_tmp.jpg"
#define CVB_DISABLE_PATH	"/mnt/user1/thttpd/html/img/cbv_disable.jpg"

extern int output_prev_mode;
extern char output_prev_enable;
extern int output_mode_now;
extern char output_enable_now;
extern char cgi_factorytest_result[256];
extern int Dongle_response;
#if TEST_ENABLE
extern int sysinfo_refreshdata_factorytest();
#endif
extern int sys_info_factory_test(char *);
extern int card_stat;
extern int usb_stat;

pthread_t airview_tid=-1;
unsigned int keep_airviewing=0; //Mos: workaround to avoid overflow...
#define AIRVIEW_PERIOD 3
extern void *deinst;

static void *airview_thread(void * arg)
{
	unsigned long framebuf_addr=0;		
	DE_config ds_conf;
	int filesize=0;
	int w=0,h=0;
	char cmd[256];

	framebuf_addr = _fui_get_cur_framebuf();
	
	
	while(keep_airviewing>0){
		//if(jpeg_encode((void*)framebuf_addr,filename,3,&filesize,w,h,4,1)!=0){
		if(ezPlayerOnVideoStreaming()){
			//puts("video playing...show cbv_disable.jpg 2!!");
			snprintf(cmd, sizeof(cmd), "cp -f %s %s", CVB_DISABLE_PATH, SNAPSHOT_PATH);
			system(cmd);
			//keep_airviewing=0;
			sleep(AIRVIEW_PERIOD*2);			
			continue;
		}
		//puts("going to do snapshot!!");			
		de_get_config(deinst,&ds_conf,DE_CFG_ALL);
		w = ds_conf.input.width;//gui_get_image_width();
		h = ds_conf.input.height;//gui_get_image_height();
		//printf("Save Snapshot file=%s,bufaddr=0x%x,w=%d,h=%d\n",SNAPSHOT_PATH,framebuf_addr,w,h);	
		framebuf_addr = fui_get_virtual_address((int)ds_conf.input.bus_addr);//_fui_get_cur_framebuf();

        if (ezCustomerOnAirplayMirror()){
            /* Mos: when Airplay Mirror, framebuf format will be NV12/YUV_420_SEMIPLANAR
            So we need config jpeg encode with different argument */
            while(jpeg_encode(SNAPSHOT_TMP_PATH,(void*)framebuf_addr,(void*)(framebuf_addr+(w*h)),NULL,w,2,0,0,w,h,3,&filesize,1)!=0){
                //printf("%s,%d:Save Snapshot Error! wait 1 sec to re-do...\n",__FILE__,__LINE__);
                sleep(1);
                framebuf_addr = _fui_get_cur_framebuf();
            }
        }
        else{
    		while(jpeg_encode(SNAPSHOT_TMP_PATH,(void*)framebuf_addr,NULL,NULL,w*2,4,0,0,w,h,3,&filesize,1)!=0){
	    		//printf("%s,%d:Save Snapshot Error! wait 1 sec to re-do...\n",__FILE__,__LINE__);		
		    	sleep(1);
			    framebuf_addr = _fui_get_cur_framebuf();				
    		}
        }
		//EZCASTLOG("w: %d, h: %d, gui_get_image_width: %d, gui_get_image_height: %d\n", w, h, gui_get_image_width(), gui_get_image_height());
		snprintf(cmd, sizeof(cmd), "chmod 666 %s", SNAPSHOT_TMP_PATH);
		system(cmd);
		if(rename(SNAPSHOT_TMP_PATH, SNAPSHOT_PATH) != 0){
			EZCASTLOG("rename %s to %s fail\n", SNAPSHOT_TMP_PATH, SNAPSHOT_PATH);
			perror("ERROR");
		}
		//printf("%s,%d:Save Snapshot OK Size=0x%x!\n",__FILE__,__LINE__,filesize);
		//keep_airviewing=0;
		//puts("airview thread go to sleep...");		
		sleep(AIRVIEW_PERIOD);
		//printf("airview thread wake up, keep==%d\n",keep_airviewing);
	}
	airview_tid=-1;
	//printf("%s,%s,%d,airview_thread ending......\n",__FILE__,__func__,__LINE__);
	pthread_exit(NULL);
	return NULL;
}

int snapshot_save()
{

	char cmd[256];
	DE_config ds_conf;
	rtc_date_t rtc_date;
	rtc_time_t rtc_time;
	int ret=0;
	
	if(tm_get_rtc(&rtc_date,&rtc_time)!=0){
		printf("%s,%d:Get rtc Date Error!",__FILE__,__LINE__);
		return -1;
	}
//	sprintf(filename,"%ssnapshot.jpg","/mnt/user1/thttpd/html/");
	if(access(SNAPSHOT_PATH,F_OK)!=0){
		int filesize=0;
		puts("mount tmpfs on mnt/user1/thttpd/html/tmp");
		system("mount -t tmpfs -o size=2m tmpfs /mnt/user1/thttpd/html/tmp/");
		if(ezPlayerOnVideoStreaming()){
			//puts("video playing...show cbv_disable.jpg 1!!");
			snprintf(cmd, sizeof(cmd), "cp -f %s %s", CVB_DISABLE_PATH, SNAPSHOT_PATH);
			system(cmd);
		}else{//do snapshot here since to prevent showing no-img on cbv.html

			de_get_config(deinst,&ds_conf,DE_CFG_ALL);
			int w = ds_conf.input.width;//gui_get_image_width();
			int h = ds_conf.input.height;//gui_get_image_height();
			unsigned long framebuf_addr = fui_get_virtual_address((int)ds_conf.input.bus_addr);//_fui_get_cur_framebuf();
			//printf("Save Snapshot file=%s,bufaddr=0x%x,w=%d,h=%d\n",SNAPSHOT_PATH,framebuf_addr,w,h);	

            if (ezCustomerOnAirplayMirror()){
                /* Mos: when Airplay Mirror, framebuf format will be NV12/YUV_420_SEMIPLANAR
                 So we need config jpeg encode with different argument */
                jpeg_encode(SNAPSHOT_TMP_PATH,(void*)framebuf_addr,(void*)(framebuf_addr+(w*h)),NULL,
                    w,2,0,0,w,h,3,&filesize,1);
            }
            else{
                jpeg_encode(SNAPSHOT_TMP_PATH,(void*)framebuf_addr,NULL,NULL,
                    w*2,4,0,0,w,h,3,&filesize,1);
        }

			snprintf(cmd, sizeof(cmd), "chmod 666 %s", SNAPSHOT_TMP_PATH);
			system(cmd);
			if(rename(SNAPSHOT_TMP_PATH, SNAPSHOT_PATH) != 0){
				EZCASTLOG("rename %s to %s fail\n", SNAPSHOT_TMP_PATH, SNAPSHOT_PATH);
				perror("ERROR");
			}
		}		
	}else{
//		puts("already mount tmpfs on html/tmp");	
	}
	snprintf(cmd, sizeof(cmd), "chmod 666 %s", SNAPSHOT_PATH);
	system(cmd);

	if(airview_tid==-1){
		keep_airviewing=1;
		do{
			printf("create airview_thread!!\n");
			ret=pthread_create(&airview_tid,NULL,airview_thread,NULL);
			printf(" pthread_create rtn==%d ",ret);
			printf(" tid==%x\n ",airview_tid);
		}while(ret!=0);
	}else{
		keep_airviewing++;
		//printf("%s,%s,%d,tid==%d,keep++==%d\n",__FILE__,__FUNCTION__,__LINE__,airview_tid,keep_airviewing);
	}
	return 0;
}

static int html_snapshot_save(void *val, void *handle){
	printf("\t%s,Snapshot!!!\n",__FILE__);
	snapshot_save();
	function_end(handle);
	return 0;
}

static int html_test(void *val, void *handle){
	printf("val is: %s\n", (char *)val);
	return_val("12345678", strlen("12345678"), handle);
	function_end(handle);
	return 0;
}

struct test_s{
	int num;
	char ch[15];
	int n;
	char c;
};

static int html_test2(void *val, void *handle){
	struct test_s *t;
	t = (struct test_s *)val;
	printf("num: %d, ch: %s, n: %d, c: %c\n", t->num, t->ch, t->n, t->c);
	return_val(t, sizeof(struct test_s), handle);
	function_end(handle);
	return 0;
}

static int html_setpassword(void *val, void *handle){

	return 0;
}

static int html_creat_edid_bin(void *val, void *handle)
{	
	char edid_buf[64]={0};
	char head_info[48]={0};
	FILE *fp;
	int fd_write;
	int len=0;
	int i=0;
	unsigned char ch;
	int err;
	edid_config_info *edid_config_info_temp = NULL;
	
	if(val){
		edid_config_info_temp = (edid_config_info *)val;
	}else{
		goto ___sysinfo_creat_edid_bin_end__;
	}

	ediddbg_info("edid_config_info_temp.vga_valid====%d\n",edid_config_info_temp->vga_valid);
	ediddbg_info("edid_config_info_temp.hdmi_valid====%d\n",edid_config_info_temp->hdmi_valid);
	ediddbg_info("edid_config_info_temp.vga_format====%d\n",edid_config_info_temp->vga_format);
	ediddbg_info("edid_config_info_temp.hdmi_format====%d\n",edid_config_info_temp->hdmi_format);
	
	ediddbg_info();
	del_edid_binfile();
	ediddbg_info();

	fd_write = open(EDID_CONFIG_PATH_IN_CASE, O_RDWR|O_CREAT, 0664);
	if(fd_write < 0){
		printf("open file error\n");
		goto ___sysinfo_creat_edid_bin_end__;
	}

	ediddbg_info();

	if( write(fd_write, head_info, 48) != 48 ) {
		printf(" write edid error 1\n");
		close(fd_write);
		goto ___sysinfo_creat_edid_bin_end__;
	}

	if( write(fd_write, edid_config_info_temp, sizeof(edid_config_info)) != sizeof(edid_config_info) ) {
		printf(" write edid error 2\n");
		close(fd_write);
		goto ___sysinfo_creat_edid_bin_end__;
	}
	
	//write(head_info,1,48,fp);
	//fwrite(edid_config_info_temp,1,sizeof(edid_config_info),fp);
	
	ediddbg_info();
	//fflush(fp);
	
	ediddbg_info();
	// = fileno(fp);
	
	ediddbg_info();
	fsync(fd_write);
	ediddbg_info();
	//fclose(fp);
	close(fd_write);
	ediddbg_info();
	
___sysinfo_creat_edid_bin_end__:
	function_end(handle);

	return 0;

}

static int html_read_edid_bin(void *val, void *handle)
{
	
	int read_index = -1;
	int read_reslut=-1;
/*	read_reslut=find_result_form_edid_binfile(read_index);
*/
	DE_INFO	de_info;

	if(val){
		read_index= *(int*)val;
	}else{
		goto __READ_EDID_END__;
	}
	ediddbg_info("read_index==============%d\n",read_index);
	int fd = open("/dev/lcm",O_RDWR);
	if(fd<0){
		sysdbg_err("Sorry Open lcm cfg Error\n");
		return -1;
	}
	if(ioctl(fd,GET_DE_CONFIG,&de_info)<0){
		sysdbg_err("de get config error\n");
		close(fd);
		return -1;
	}
	switch(read_index){
		case 0:
			switch(de_info.screen_type){
				case 0:
					read_reslut=LCD_ENABLE;
					break;
				case 1:
					read_reslut=HDMI_ENABLE;
					break;
				case 2:
					read_reslut=VGA_ENABLE;
					break;
				case 3:
					read_reslut=CVBS_ENABLE;
					break;
				case 4:
					read_reslut=YPBPR_ENABLE;
					break;
				default:
					
					read_reslut=LCD_ENABLE;
					break;
					
			}
					
			break;
		case 1:
			read_reslut=de_info.screen_output_format;
			printf("read_reslut == %d\n", read_reslut);
			break;
	}
	printf("de_info.screen_type= %d %d\n ",de_info.screen_type,de_info.screen_output_format);

__READ_EDID_END__:	
	printf("read_reslut = %d\n", read_reslut);
	return_val((void *)&read_reslut, sizeof(read_reslut), handle);
	function_end(handle);

	return 0;
}

static int html_set_HDMI_mode(void *val, void * handle)
{
	int hmode = 0;
	
	if(val){
		hmode = *(int*)val;
	}else{
		goto sysinfo_set_HDMI_mode_out;
	}

	sysdbg_info("output enable is 0x%x\n",output_enable_now);
	output_prev_enable = output_enable_now;
	if((output_enable_now & HDMI_ENABLE)!=0)
	{
		sysdbg_info("HDMI Output is enable\n");
		output_prev_mode = output_mode_now;
		if(_exec_hdmi_mode(hmode)!=0)
		{
			sysdbg_err("Sorry _exec_hdmi_mode Error!");
		}
	}
	else if((output_enable_now & CVBS_ENABLE)!=0)
	{
		sysdbg_info("CVBS Output is enable\n");
		_do_action(CMD_SET_CVBS_MODE,&hmode);
		screen_output_data.screen_output_true= 1;
		screen_output_data.screen_output_mode = hmode;
		output_prev_mode = output_mode_now;
		output_mode_now = hmode;
	}
	else if((output_enable_now & YPBPR_ENABLE)!=0)
	{
		sysdbg_info("YPbPr Output is enable\n");
		_do_action(CMD_SET_YPBPR_MODE,&hmode);
		screen_output_data.screen_output_true= 1;
		screen_output_data.screen_output_mode = hmode;
		output_prev_mode = output_mode_now;
		output_mode_now = hmode;
	}
	else if((output_enable_now & VGA_ENABLE)!=0)
	{
		sysdbg_info("VGA Output is enable\n");
		_do_action(CMD_SET_VGA_MODE,&hmode);
		screen_output_data.screen_output_true= 1;
		screen_output_data.screen_output_mode = hmode;
		output_prev_mode = output_mode_now;
		output_mode_now = hmode;
	}
	sysdbg_info("set previous output enable is 0x%x, mode is %d\n",output_prev_enable, output_prev_mode);
	_get_screen_size();
sysinfo_set_HDMI_mode_out:
	function_end(handle);
	return 0;
}
#if TEST_ENABLE
static int factory_test(void *val, void * handle)
{
	char *factory_test_cmd;
	//int rcv_flag=-1;
	Dongle_response=0;
	if(val){
		factory_test_cmd = (char *)val;
		if(!strcmp(factory_test_cmd,"testmode=true"))
		{
			Dongle_response=1;

		}
		if(!strcmp(factory_test_cmd,"testresult=true"))
		{

			
			int ret=sysinfo_refreshdata_factorytest();
			if(ret==-1){
				//memset(cgi_factorytest_result,0,256);
				printf("---FactoryTest Testing cgi get result  later---\n");
				memset(cgi_factorytest_result,0,256);
			}
			else
			{
				printf("--- cgi get FactoryTest result---\n");

			}
			return_val((void *)&cgi_factorytest_result, sizeof(cgi_factorytest_result), handle);
			printf("Factory Test Result:%s\n",cgi_factorytest_result);
			//memset(cgi_factorytest_result,0,256);
			function_end(handle);
			return 0;
		}
		//rcv_flag=1;
	}else{
		goto sysinfo_factory_test_cmd_end;
		Dongle_response=0;
	}
	sys_info_factory_test(factory_test_cmd);
	
	///memset(factory_test_cmd,0,1024);

sysinfo_factory_test_cmd_end:
	return_val((void *)&Dongle_response, sizeof(Dongle_response), handle);
	function_end(handle);
	return 0;



}
#endif
/*
system()  not excute in CGI  add a api for excute system cmd in cgi
*/
extern int udisk1_in;
extern int udisk2_in;
static int airdisk_cmd(void *val, void * handle)
{

	char *disk_cmd;
	int card_status=card_stat;
	int usb_status=usb_stat;//
	int rn=0;
	if(val){
		disk_cmd = (char *)val;
	}else{
		goto system_cmd_end;
	}
	printf("Airdisk cmd:%s \n",disk_cmd);
	if(!strcmp(disk_cmd,"card_sta"))
	{
		printf("card stat:%d \n",card_status);	
		return_val((void *)&card_status, sizeof(card_status), handle);
	}
	else if(!strcmp(disk_cmd,"usb_sta")){
		printf("usb_status:%d \n",usb_status);
		if(udisk1_in==0&&udisk2_in==0) //no usb  insert
			usb_status=2;
		return_val((void *)&usb_status, sizeof(usb_status), handle);

	}
	
system_cmd_end:
	function_end(handle);
	return 0;

}
static int system_cmd(void *val, void * handle)
{
	char *sys_cmd;
	int rn=0;
	if(val){
		sys_cmd = (char *)val;
		//rcv_flag=1;
	}else{
		goto system_cmd_end;
	}
	printf("cgi system cmd:%s \n",sys_cmd);
	if((rn=system(sys_cmd))<0)
		printf("cgi system(%s) Fail\n",sys_cmd);
system_cmd_end:
	function_end(handle);
	return 0;

}

static int html_app_command(void *val, void * handle){
	char *cmdstr = val;
	char *retStr = NULL;
	int retlen;
	int cmd = atoi(cmdstr);
	int ret = -1;

	switch(cmd){
		case CMDNUM_HELPVIEW:
			//ezCastAddKey(0x1BF);
			ezRemoteSendKey(0x1BF);
			break;
		case CMDNUM_APPINFO:
			//ezCastAddKey(0x1C0);
			ezRemoteSendKey(0x1C0);
			break;
		case CMDNUM_GETDONGLEINFO:
			ezPushDongleinfo(&retStr, &retlen);
			if(retStr != NULL){
				return_val((void *)retStr, retlen, handle);
				ezPushDongleinfoFree(&retStr);
			}
			break;
#ifdef AM8252TO8251KEY_ENABLE
		case CMDNUM_KEY:
			ezCastKeyHandle();
			break;
		case CMDNUM_TRIALKEY:
			ezCastTrialKeyHandle();
			break;
#endif
		default:
			break;
	}
	
	function_end(handle);
	return 0;
}

static int html_confCtrl(void *val, void * handle)
{
	char *cmd=(char *)val;
	int ret=0;
	static int adbDisplayStarted = 0;
	if(cmd==NULL){
		EZCASTWARN("cmd is NULL!!!\n");
		ret = -1;
		goto __END__;
	}
	if(strlen(cmd) <= 0){
		EZCASTWARN("cmd error!!!\n");
		ret = -1;
		goto __END__;
	}
	EZCASTLOG("conference control: %s\n", cmd);
	ezRemoteConfCtrl(cmd, NULL, 0);

	ret = 0;

__END__:
	function_end(handle);
	return ret;
}

static int html_confCtrlAndReturn(void *val, void * handle)
{
	char *cmd=(char *)val;
	char res[20480];
	int ret=0;
	static int adbDisplayStarted = 0;
	if(cmd==NULL){
		EZCASTWARN("cmd is NULL!!!\n");
		ret = -1;
		goto __END__;
	}
	if(strlen(cmd) <= 0){
		EZCASTWARN("cmd error!!!\n");
		ret = -1;
		goto __END__;
	}
	EZCASTLOG("conference control: %s\n", cmd);
	ezRemoteConfCtrl(cmd, res, sizeof(res));
    //Mos: Disable log
	//EZCASTLOG("return: %s\n", res);
	return_val((void *)res, strlen(res)+1, handle);

	ret = 0;

__END__:
	function_end(handle);
	return ret;
}

// ***************************************************  Dividing line *************************************************

const struct funLink_s funLink[] = {
		{"htmlsetting_creat_edid_bin", html_creat_edid_bin},
		{"htmlsetting_read_edid_bin", html_read_edid_bin},
		{"htmlsetting_set_HDMI_mode", html_set_HDMI_mode},
		{"htmlsetting_snapshot_save", html_snapshot_save},
		{"htmlsetting_app_command", html_app_command},
		{"htmlsetting_confCtrl", html_confCtrl},
		{"htmlsetting_confCtrlAndReturn", html_confCtrlAndReturn},
		{"htmlsetting_test", html_test},
		{"htmlsetting_test2", html_test2},
#if TEST_ENABLE
		{"factory_test_cmd",factory_test},
#endif
		{"system_cmd",system_cmd},
		{"airdisk_cmd",airdisk_cmd},
};

int do_function(char *cmd, void *val, struct select_ctrl_s *handle){
	int i;

	if(cmd == NULL){
		printf("Argument error!!!\n");
		return -1;
	}
	for(i=0; i<(sizeof(funLink)/sizeof(struct funLink_s)); i++){
		if(!strcmp(cmd, funLink[i].name)){
			funLink[i].func(val, (void *)handle);
			return 0;
		}
	}

	printf("[%s] [%d] -- function \"%s\" not exist!!!\n", __func__, __LINE__, cmd);
	return 1;
}

#endif

