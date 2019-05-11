/**
*@brief this file is used to deal with system message
*@system message mainly includes storage hotplug such as card, usb, etc.
*
*@author: yekai
*@date: 2010-05-11
*@version: 0.1
*/
#include <stdio.h>
#include <sys_msg.h>
#include <sys_cfg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


int deal_card_msg(struct am_sys_msg *msg)
{
	char app_path[50];
	
	sprintf(app_path,"%s/%s",AM_CASE_SC_DIR,"card_process.sh");
	switch(msg->subtype){
	case SD:
	case MMC:
		if(msg->dataload==IN)
			sprintf(app_path,"%s %s",app_path,"sdin");
		else
			sprintf(app_path,"%s %s",app_path,"sdout");
		break;
	case XD:
		if(msg->dataload==IN)
			sprintf(app_path,"%s %s",app_path,"xdin");
		else
			sprintf(app_path,"%s %s",app_path,"xdout");
		break;
	case CF:
		if(msg->dataload==IN)
			sprintf(app_path,"%s %s",app_path,"cfin");
		else
			sprintf(app_path,"%s %s",app_path,"cfout");
		break;
	default:
		app_path[0]='\0';
		break;
	}

	return system(app_path);
}


int check_am7x_hcd_exisit(){

	char callbuf[50];
	FILE *fp=NULL;
	char buf[512] = {0};
	char * tmpbuf=NULL;
	int ret=-1;
	char *locate=NULL;

	
	sprintf(callbuf,"lsmod");
	system(callbuf);
	printf("the call is %s\n",callbuf);	
	fp = popen(callbuf, "r");
	
    if(NULL == fp)
    {
    	fprintf(stderr,"function:%s line:%d \n",__FUNCTION__,__LINE__);

        printf("popen error:%s\n",strerror(errno));
        return -1;
    }
	
	ret=fread(buf, 1, 512, fp);
	
	fprintf(stderr,"function:%s line:%d ret:%d\n",__FUNCTION__,__LINE__,ret);
  	ret = pclose(fp);
	fprintf(stderr,"function:%s line:%d ret:%d\n",__FUNCTION__,__LINE__,ret);

	locate=strstr(buf,"am7x_hcd");
	if(locate!=NULL)
		return 0;
	else 
		return -1;


}

int deal_usb_msg(struct am_sys_msg *msg)
{
	char app_path[50];
	sprintf(app_path,"%s/%s",AM_CASE_SC_DIR,"usb_process.sh");
	switch(msg->subtype){
		case DEVICE_USB_IN:
			switch(msg->dataload){
				case DEVICE_RAW:
					printf("%s:host raw in\n",__FUNCTION__);
					sprintf(app_path,"%s %s",app_path,"dev_raw_in");
					break;
				case DEVICE_MASS_STORAGE:
					printf("%s:host mass out\n",__FUNCTION__);
					sprintf(app_path,"%s %s",app_path,"dev_mass_in");
					break;
				case DEVICE_SUBDISPLAY:
					sprintf(app_path,"%s %s",app_path,"subdisp_in");
				case DEVICE_PICTBRIDGE:
					sprintf(app_path,"%s %s",app_path,"picbri_in");
					break;
			}
			break;
		case DEVICE_USB_OUT:
			switch(msg->dataload){
				case DEVICE_RAW:
					printf("%s:device raw out\n",__FUNCTION__);
					sprintf(app_path,"%s %s",app_path,"device_raw_out");
					break;
				case DEVICE_MASS_STORAGE:
					printf("%s:device mass out\n",__FUNCTION__);
					sprintf(app_path,"%s %s",app_path,"dev_mass_out");
					break;
				case DEVICE_SUBDISPLAY:
					sprintf(app_path,"%s %s",app_path,"subdisp_out");
					break;
				case DEVICE_PICTBRIDGE:
					sprintf(app_path,"%s %s",app_path,"subdisp_out");
					break;
			}
			break;
		case HOST_USB_IN:
			switch(msg->dataload){
				case HOST_RAW:
					printf("%s:host raw in\n",__FUNCTION__);
					sprintf(app_path,"%s %s",app_path,"host_raw_in");
					int hcd_exsit=check_am7x_hcd_exisit();
					if(hcd_exsit==0){
						sleep(2);
						printf("the am7x_hcd hasn't rmmod,please wait!");
					}
					break;
				case HOST_MASS_STORAGE:
					printf("%s:host mass in\n",__FUNCTION__);
					sprintf(app_path,"%s %s",app_path,"host_mass_in");
					break;
				case HOST_MASS_STOARGE_SCANOK:
					printf("%s:host mass scan ok\n",__FUNCTION__);
					sprintf(app_path,"%s %s",app_path,"host_mass_scanok");
					break;
				case HOST_USB_HID:
					sprintf(app_path,"%s %s",app_path,"host_hid_in");
					break;
				/* others*/
			}
			break;
		case HOST_USB_OUT:
				switch(msg->dataload){
				case HOST_RAW:
					printf("%s:host raw out\n",__FUNCTION__);
					sprintf(app_path,"%s %s",app_path,"host_raw_out");
					int hcd_exsit=check_am7x_hcd_exisit();
					if(hcd_exsit==-1){
						
						printf("the am7x_hcd has rmmod!");
						return 0;
						
					}
					break;
				case HOST_MASS_STORAGE:
					printf("%s:host mass out\n",__FUNCTION__);
					sprintf(app_path,"%s %s",app_path,"host_mass_out");
					break;
				case HOST_USB_HID:
					sprintf(app_path,"%s %s",app_path,"host_hid_out");
					break;
				/* others*/
			}
			break;
		default:
			return 0; 
	}
	
	return system(app_path);
}

