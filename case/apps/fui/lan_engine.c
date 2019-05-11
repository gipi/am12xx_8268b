#if MODULE_CONFIG_LAN

#include "swf_ext.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/sockios.h>
#include <linux/ethtool.h>
#include <linux/types.h>
#include <dirent.h>
#include <sys_mac.h>

#include <sys_at24c02.h>
#include "wifi_engine.h"
#include "net_config_info.h"
#include "customization_info.h"
#include "ezcast_public.h"

#define AM_SYS_LIB_DIR 		"/lib/modules/2.6.27.29"

#if WEBSETTING_ENABLE
extern void create_websetting_server();
#endif

char lanEnableFlag = 0;
char lanIfupFlag = 0;

static char *pre_gateway;
char ip_address_for_display[50]={0};
char mask_for_display[50]={0};
char gateway_for_display[50]={0};

char gateway_data_from_cul[50]={0};
char first_dns_for_display[50]={0};
char second_dns_for_display[50]={0};
char mac_address[50]={0};
char inter_face[50]={"eth0"};	
int lan_access_mode=-1;
int exec_atop_flag = 1;
extern int wpa24g_connected;
extern int wpa5g_connected;

/* Check network adapter is up or down */
// Note: one or two adapter drivers may be not support this method
// exemple: based virtual machine

//#ifdef 0
int netlink_status;
int first_test=1;

const char* VERSION = "0.6.0.48 --maintain by isnowran";

struct mii_data
{
        __u16 phy_id;
        __u16 reg_num;
        __u16 val_in;
        __u16 val_out;
};
void my_sleep(int time_interval){
	int i=0;
	int j=0;
	for(i=0;i<time_interval;i++)
		for(j=0;j<time_interval;j++){
			int temp=0;
			temp++;
			temp--;
		}

}
int check_netlink_status_common(){
		int skfd = 0;
		int fd;
		char callbuf[50]={0};
		int err;
		char cmd[256];
		struct eeprom_data_t test_eeprom;
	#if AM_CHIP_ID==1211
      
		
		if(access("/sys/module/ax88796c_net",F_OK)==-1){
			sprintf(callbuf,"insmod /lib/modules/2.6.27.29/ax88796c_net.ko");
			system(callbuf);
			return -1;
		}
			
	
		else{
		
      	  if( ( skfd = socket( AF_INET, SOCK_DGRAM, 0 ) ) < 0 )
       	 {
                perror( "socket" );
				
				close( skfd );
               return -1;
       	 }

      	  struct ifreq ifr;
      	  bzero( &ifr, sizeof( ifr ) );
       	  strncpy( ifr.ifr_name, "eth0", IFNAMSIZ - 1 );
      	  ifr.ifr_name[IFNAMSIZ - 1] = 0;
		  
     	  if( ioctl( skfd, SIOCGMIIPHY, &ifr ) < 0 )
      	  {
      	  		
				close( skfd );
                perror( "ioctl" );
                return -1;
      	  }

      	  struct mii_data* mii = NULL;
      	  mii = (struct mii_data*)&ifr.ifr_data;
      	  mii->reg_num = 0x01;
      	  if( ioctl( skfd, SIOCGMIIREG, &ifr ) < 0 )
       	 {		
                perror( "ioctl2" );
				
				close( skfd );
                return -2;
       	 }
       	 if( mii->val_out & 0x0004&&mii->val_out!=65535 ){
				close( skfd );
				return 1;
      	  }
      	  else{
               			
				close( skfd );
				return 0;
			}
        	close( skfd );
		}
	#endif
#if AM_CHIP_ID == 1213	
		if(access("/sys/module/am7x_net",F_OK)==-1){
#if EEPROM_TYPE == 1
			sleep(3);
			if(access("/sys/module/i2c_am7x",F_OK)==-1){
				sprintf(callbuf,"%s%s/%s","insmod  ",AM_SYS_LIB_DIR,"i2c-am7x.ko");
				system(callbuf);
			}
			sleep(3);
			if(access("/sys/module/at24_i2c",F_OK)==-1){
				sprintf(callbuf,"%s%s/%s","insmod  ",AM_SYS_LIB_DIR,"at24_i2c.ko");
				system(callbuf);
			}
#elif EEPROM_TYPE == 2
			sleep(3);
			if(access("/sys/module/am7x_spi",F_OK)==-1){
				sprintf(callbuf,"%s%s/%s","insmod  ",AM_SYS_LIB_DIR,"am7x_spi.ko");
				system(callbuf);
			}
			sleep(3);
			if(access("/sys/module/at93_spi",F_OK)==-1){
				sprintf(callbuf,"%s%s/%s","insmod  ",AM_SYS_LIB_DIR,"at93_spi.ko");
				system(callbuf);
			}
#endif
			sprintf(callbuf,"insmod /lib/modules/2.6.27.29/am7x_net.ko");
			system(callbuf);
			system("ifconfig eth0 up");
		}
	
		else{		
	      	  if( ( skfd = socket( AF_INET, SOCK_DGRAM, 0 ) ) < 0 ) {
	                perror( "socket" );
					
					close( skfd );
	               return -1;
	       	 }
	      	  struct ifreq ifr;
	      	  bzero( &ifr, sizeof( ifr ) );
	       	  strncpy( ifr.ifr_name, "eth0", IFNAMSIZ - 1 );
	      	  ifr.ifr_name[IFNAMSIZ - 1] = 0;		  
	     	  if( ioctl( skfd, SIOCGMIIPHY, &ifr ) < 0 )
	      	  {
	      	  		
					close( skfd );
	                perror( "ioctl" );
	                return -1;
	      	  }
	      	  struct mii_data* mii = NULL;
	      	  mii = (struct mii_data*)&ifr.ifr_data;
	      	  mii->reg_num = 0x01;
	      	  if( ioctl( skfd, SIOCGMIIREG, &ifr ) < 0 )
	       	 {		
	                perror( "ioctl2" );
					
					close( skfd );
	                return -2;
	       	 }
	       	 if( mii->val_out & 0x0004&&mii->val_out!=65535 ){
					close( skfd );
					return 1;
	      	  }
	      	  else{	               			
					close( skfd );
					return 0;
			  }
	          close( skfd );
		}

	#endif
	return -1;
}
int get_netlink_status()
{

	
#if AM_CHIP_ID==1211

	int netlink_status_1211=check_netlink_status_common();
	return netlink_status_1211;
#endif

#if AM_CHIP_ID==1213
		int ret=0;
		int fd;
		char buf[8]={0};
		char callbuf[128];
		if(access("/sys/module/am7x_net",F_OK)==-1){
#if EEPROM_TYPE == 1
			sleep(3);
			if(access("/sys/module/i2c_am7x",F_OK)==-1){
				sprintf(callbuf,"%s%s/%s","insmod  ",AM_SYS_LIB_DIR,"i2c-am7x.ko");
				system(callbuf);
			}
			sleep(3);
			if(access("/sys/module/at24_i2c",F_OK)==-1){
				sprintf(callbuf,"%s%s/%s","insmod  ",AM_SYS_LIB_DIR,"at24_i2c.ko");
				system(callbuf);
			}
#elif EEPROM_TYPE == 2
			sleep(3);
			if(access("/sys/module/am7x_spi",F_OK)==-1){
				sprintf(callbuf,"%s%s/%s","insmod  ",AM_SYS_LIB_DIR,"am7x_spi.ko");
				system(callbuf);
			}
			sleep(3);
			if(access("/sys/module/at93_spi",F_OK)==-1){
				sprintf(callbuf,"%s%s/%s","insmod  ",AM_SYS_LIB_DIR,"at93_spi.ko");
				system(callbuf);
			}
#endif	
			system("insmod /lib/modules/2.6.27.29/am7x_net.ko");
		}
		if(lanIfupFlag == 0){
			printf("ifconfig eth0 up!\n");
			system("ifconfig eth0 up");
			lanIfupFlag = 1;
		}
		if(access("/proc/driver/am7x_net_phy_stat",F_OK)==-1){
			return 0;
		}	
		fd = open("/proc/driver/am7x_net_phy_stat",O_RDONLY);
	
		memset(buf,0,8);
		read(fd,buf,8);
		ret=close(fd);
		if(strstr(buf,"1"))
			return 1;
		if(strstr(buf,"0"))
			return 0;
#endif
	return -1;
}




void find_gateway(char * ip_data){
	
	char gateway_data_part[50];
	char *locate1=NULL;
	char *locate2=NULL;
	char *locate3=NULL;
	char *ip_data_tmp=ip_data;
	memset(gateway_data_from_cul,0,50);
	printf("ip_data=======%s\n",ip_data);
	locate1=strstr(ip_data,".");
	printf("locate1=====%s\n",locate1);
	if(locate1==NULL)
		return ;
	locate1++;
	locate2=strstr(locate1,".");
	if(locate2==NULL)
		return ;
	printf("locate2=====%s\n",locate2);
	locate2++;
	locate3=strstr(locate2,".");
	if(locate3==NULL)
		return ;

	
	printf("locate3=====%s\n",locate3);
	memset(gateway_data_part,0,50);
	memcpy(gateway_data_part,ip_data_tmp,strlen(ip_data_tmp)-strlen(locate3));
	strcat(gateway_data_part,".1");
	printf("gateway_data_part======%s\n",gateway_data_part);
	memcpy(gateway_data_from_cul,gateway_data_part,strlen(gateway_data_part));
	//printf("*gateway_data=======%s\n",*gateway_data);
/*	char *gateway_data_tmp=NULL;
	gateway_data_tmp=*gateway_data;
	
	memset(gateway_data_tmp,0,strlen(gateway_data_tmp));
	memcpy(gateway_data_tmp,gateway_data_part,strlen(gateway_data_part));
*/	
}
void dns_get_from_resolv_file(){
	FILE *fp;
	char callbuf[50]={0};
	char buf[256]={0};
	int ret=-1;
	
	if(access("/mnt/vram/wifi/resolv.conf",F_OK)==-1){
		printf("cann't access to the resolv.conf");
		return ;
	}
	fp = fopen("/mnt/vram/wifi/resolv.conf","r");

	if(fp == NULL){
			printf("read resolv.conf open error!");
			return ;
		}
	ret=fread(buf, 1, 256, fp);

	ret=fclose(fp);
	fprintf(stderr,"function:%s line:%d ret=%d\n",__FUNCTION__,__LINE__,ret);	
	
	
	printf("%s\n",buf);
	char *locate1=strstr(buf,"nameserver ");
	if(locate1==NULL)
		return ;
	locate1+=strlen("nameserver ");
	char *locate1_begin_point=locate1;
	char *locate2=strstr(locate1,"\n");
	if(locate2==NULL)
		return ;
	memset(first_dns_for_display,0,50);
	memcpy(first_dns_for_display,locate1_begin_point,strlen(locate1_begin_point)-strlen(locate2));
	fprintf(stderr,"function:%s line:%d first_dns_for_display=%s\n",__FUNCTION__,__LINE__,first_dns_for_display);	
	char *locate3=strstr(locate2,"nameserver ");
	if(locate3==NULL)
		return ;
	locate3+=strlen("nameserver ");
	char *locate3_begin_point=locate3;
	char *locate4=strstr(locate3,"\n");
	if(locate4==NULL)
		return ;
	memset(second_dns_for_display,0,50);
	memcpy(second_dns_for_display,locate3_begin_point,strlen(locate3_begin_point)-strlen(locate4));
	
	fprintf(stderr,"function:%s line:%d second_dns_for_display=%s\n",__FUNCTION__,__LINE__,second_dns_for_display);	
}
static int lan_firstdns_get_for_landisplay(void *handle)
{
		SWFEXT_FUNC_BEGIN(handle);
		
		dns_get_from_resolv_file();
		Swfext_PutString(first_dns_for_display);
	
		SWFEXT_FUNC_END();	
}
static int lan_seconddns_get_for_landisplay(void *handle)
{
		SWFEXT_FUNC_BEGIN(handle);
		
		Swfext_PutString(second_dns_for_display);
		SWFEXT_FUNC_END();	
}

char *getLanIp(){
	char ipTemp[16 + 1] = {0};
	
	memset(ipTemp, 0, 16 + 1);
#if !EZWILAN_ENABLE	
	getIfconfigInfo("br0",IFCONFIG_IP,ipTemp,16 + 1);
	
	if( strlen(ipTemp) > strlen("0.0.0.0") )
		getIfconfigInfo("br0",IFCONFIG_IP,ip_address_for_display,50);
	else
#endif
		getIfconfigInfo("eth0",IFCONFIG_IP,ip_address_for_display,50);
	
	return ip_address_for_display;
}

static int lan_ip_get_for_landisplay(void *handle)
{
		SWFEXT_FUNC_BEGIN(handle);
		
		Swfext_PutString(getLanIp());
	
		SWFEXT_FUNC_END();	
}

static char *getLanMask(){
	char ipTemp[16 + 1] = {0};

	memset(ipTemp, 0, 16 + 1);
#if !EZWILAN_ENABLE	
	getIfconfigInfo("br0",IFCONFIG_IP,ipTemp,16 + 1);

	if( strlen(ipTemp) > strlen("0.0.0.0") )
		getIfconfigInfo("br0",IFCONFIG_MASK,mask_for_display,50);
	else
#endif
		getIfconfigInfo("eth0",IFCONFIG_MASK,mask_for_display,50);
	
	return mask_for_display;
}

static int lan_mask_get_for_landisplay(void *handle)
{
		SWFEXT_FUNC_BEGIN(handle);
		
		Swfext_PutString(getLanMask());
		SWFEXT_FUNC_END();	
}

static char *getLanGateway(){
	char ipTemp[16 + 1] = {0};

	memset(ipTemp, 0, 16 + 1);

	getIfconfigInfo("br0",IFCONFIG_IP,ipTemp,16 + 1);
	memset(gateway_for_display,0,50);
#if !EZWILAN_ENABLE	

	if( strlen(ipTemp) > strlen("0.0.0.0") )
		__get_kernel_gateway_info(gateway_for_display,"br0");
	else
#endif

		__get_kernel_gateway_info(gateway_for_display,"eth0");

	return gateway_for_display;
}

static int lan_gateway_get_for_landisplay(void *handle)
{
		SWFEXT_FUNC_BEGIN(handle);

		Swfext_PutString(getLanGateway());
	
		SWFEXT_FUNC_END();	
}

void clear_lan_error(){
	char callbuf[50];
	sprintf(callbuf,"killall udhcpc");
	system(callbuf);
	sprintf(callbuf,"ifconfig eth0 down");
	system(callbuf);
#if AM_CHIP_ID == 1211
	sprintf(callbuf,"rmmod ax88796c_net");
	
	system(callbuf);
#endif

#if AM_CHIP_ID == 1213
	sprintf(callbuf,"rmmod am7x_net");
	system(callbuf);

#endif

}


/**
* open the dir and if fail then creat the directory.
*/
static int lan_try_dir(const char *path)
{
	DIR *dir;

	dir = opendir(path);
	if(!dir){
		if(mkdir(path,0664)){
			printf("%s:%d: mkdir errno=%d\n",__func__,__LINE__,errno);
			return -errno;
		}
	}else{
		closedir(dir);
	}
	
	return 0;
}

static int lan_try_copy(const char *dest,const char *src)
{
	FILE *d,*s;
	unsigned char buf[128];
	int size;

	s = fopen(src,"rb");
	if(s == NULL){
		return -1;
	}

	d = fopen(dest,"wb");
	if(d == NULL){
		fclose(s);
		return -1;
	}
	
	while(!feof(s)){
		size = fread(buf,sizeof(unsigned char),sizeof(buf),s);
		if(size > 0){
			fwrite(buf, sizeof(unsigned char), size, d);
		}
	}

	fclose(s);
	fsync(fileno(d));
	fclose(d);
	
	return 0;
}

static int lan_open_fun(){
	int fd;
	char cmd[256];	
	char callbuf[50];
	int ret=0;		
	lanEnableFlag=0;
	char *pub_interface=NULL;
	char *private_interface=NULL;
	char *private_subnet=NULL;

	char ipTemp[16 + 1] = {0}; 
	memset(ipTemp,0,16 + 1);	
	if(access("/mnt/vram/wifi/resolv.conf",F_OK)==-1){
		/** create it */
		if(lan_try_dir("/mnt/vram/wifi/")==0){
			lan_try_copy("/mnt/vram/wifi/resolv.conf","/mnt/user1/dns/resolv.conf");
		}
		
	}

#if EZCAST_ENABLE
	set3G4GMode(0);
#endif
	
#if MODULE_CONFIG_EZCASTPRO_MODE!=8075
	custom_pipe_set_lan_in_flag(1);
#endif
	printf("---------------init cgi------------\n");
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE!=0
#if WEBSETTING_ENABLE
	printf("===========create web server=========\n");
	create_websetting_server();
#endif
#if EZCAST_ENABLE
	printf("===========start htmlsetting....=========\n");
	htmlSettingStart();
#endif
#endif
	/*test am7x pipe*/
	
#if AM_CHIP_ID == 1213	
		if(access("/sys/module/am7x_net",F_OK)==-1){
#if EEPROM_TYPE == 1
			sleep(3);
			if(access("/sys/module/i2c_am7x",F_OK)==-1){
				sprintf(callbuf,"%s%s/%s","insmod  ",AM_SYS_LIB_DIR,"i2c-am7x.ko");
				system(callbuf);
			}
			sleep(3);
			if(access("/sys/module/at24_i2c",F_OK)==-1){
				sprintf(callbuf,"%s%s/%s","insmod  ",AM_SYS_LIB_DIR,"at24_i2c.ko");
				system(callbuf);
			}
#elif EEPROM_TYPE == 2
			sleep(3);
			if(access("/sys/module/am7x_spi",F_OK)==-1){
				sprintf(callbuf,"%s%s/%s","insmod  ",AM_SYS_LIB_DIR,"am7x_spi.ko");
				system(callbuf);
			}
			sleep(3);
			if(access("/sys/module/at93_spi",F_OK)==-1){
				sprintf(callbuf,"%s%s/%s","insmod  ",AM_SYS_LIB_DIR,"at93_spi.ko");
				system(callbuf);
			}
#endif
		sprintf(callbuf,"insmod lib/modules/2.6.27.29/am7x_net.ko");
		system(callbuf);
	}	
#endif

#if AM_CHIP_ID == 1211
	if(access("/sys/module/ax88796c_net",F_OK)==-1){
		sprintf(callbuf,"insmod lib/modules/2.6.27.29/ax88796c_net.ko");
		system(callbuf);
	}
#endif


#ifdef MODULE_CONFIG_WIFI_BRIDGE_ENABLE	

#ifdef MODULE_CONFIG_WIFI_CONCURRENT_MODE

	getIfconfigInfo("wlan1",IFCONFIG_IP,ipTemp,16 + 1);

	if( strlen(ipTemp) > strlen("0.0.0.0") ){
		wifiBridgeProcess(WIFI_BRIDGE_ADD_ETH0_AND_WLAN1);
		
		goto ___enable_lan_end__;
	}
	else
		goto ____enable_lan_no_bridge__;

#else
	getIfconfigInfo("wlan0",IFCONFIG_IP,ipTemp,16 + 1);

	if( strlen(ipTemp) > strlen("0.0.0.0") ){
		wifiBridgeProcess(WIFI_BRIDGE_ADD_ETH0_AND_WLAN0);
		goto ___enable_lan_end__;
	}
	else{
		goto ____enable_lan_no_bridge__;

	}
#endif

#endif
____enable_lan_no_bridge__:

	udhcpd_dns_enable();
	sleep(3);
	sprintf(callbuf,"ifconfig eth0 up");
	ret=system(callbuf);
	
	sprintf(callbuf,"ifconfig eth0 0.0.0.0");
	system(callbuf);
	
#if AM_CHIP_ID == 1213
	sleep(3);
#endif

#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8074
	printf("ezcastpro projector LAN udhcpc will be done by ATOP... \n");
#else

	printf("run ezcast lan \n");
	sprintf(callbuf,"udhcpc -i eth0 -t 10 -n");
	ret=system(callbuf);

	sprintf(callbuf,"killall udhcpc");
	system(callbuf);
#endif

#if ROUTER_ONLY_ENABLE
	if(getRouterOnlyStatus() != 0){
		if(get_ap_close_flag() == 0){
			char *lanIP = getLanIp();
			if(strlen(lanIP) > strlen("0.0.0.0")){
				wifi_softap_close();
			}
		}
	}else
#endif
	{
		pub_interface="eth0";
		private_interface="wlan1";
#if EZCAST_ENABLE
		#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE!=0
			private_subnet="192.168.168.1/24";		// EZCastpro
		#else
			private_subnet="192.168.203.1/24";		// EZCast/EZMusic/EZWilan
		#endif
#else
		private_subnet="192.168.111.1/24";
#endif

#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075
	//probox route etho to wlan1,wlan3  with dif ip 
	//enable_route_function(pub_interface,"wlan3","192.168.203.1/24",LAN_ROUTING);
	//
	enable_route_function(pub_interface,"br0","192.168.168.1/24",LAN_ROUTING);	
	Probox_set_led(LEDLAN_ON);		
	wpa24g_connected=0;
	wpa5g_connected=0;
#else
		enable_route_function(pub_interface,private_interface,private_subnet,LAN_ROUTING);
#endif
	}
	start_thttpd_process();
	
___enable_lan_end__:
	lanEnableFlag=1;
	lan_access_mode=1;
	lanIfupFlag = 1;
	
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE!=8075	
/**chavi 120814**/
	if(exec_atop_flag){
		system("mount -o noatime -o nodiratime /dev/partitions/atop /mnt/atop");
		system("/etc/user.sh");
		exec_atop_flag = 0;
		printf("======exec user.sh======\n");
	}	
#endif
	return ret;
}

#if EZWILAN_ENABLE
static void *pthread_lan_open(void *arg){
	lan_open_fun();
	pthread_exit(NULL);
	return NULL;
}

static int lan_open(void *handle)
{
	int ret = -1;
	pthread_t tidp;
	SWFEXT_FUNC_BEGIN(handle);
	ret = pthread_create(&tidp, NULL, pthread_lan_open, NULL);
	if (ret != 0)
	{
		printf("%s/%d: pthread_create error(%d), FIXME!\n", __FUNCTION__, __LINE__, ret);
		ret = -1;
	}else{
		pthread_detach(tidp);
	}
		

	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();	
}
#else
static int lan_open(void *handle)
{
	int ret = -1;
	SWFEXT_FUNC_BEGIN(handle);
	ret = lan_open_fun();
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();	
}
#endif

static int lan_close(void *handle)
{
	char callbuf[50];
	char ipTemp[16 + 1];

	SWFEXT_FUNC_BEGIN(handle);
	
	memset(ipTemp, 0, 16 + 1);
	
	/*int pipe_fd = 0;
	pipe_fd  = open("/dev/am7x_pipe",O_RDWR);
	if(pipe_fd<0){
		printf("Sorry Open am7x Error\n");
	}else{
		ioctl(pipe_fd,15,0);
		printf("open am7x pipe!!!!\n");
		close(pipe_fd);	
	}*/
	custom_pipe_set_lan_in_flag(0);
#ifdef MODULE_CONFIG_WIFI_BRIDGE_ENABLE	
	
	getIfconfigInfo("br0",IFCONFIG_IP,ipTemp,16 + 1);
	if( strlen(ipTemp) > strlen("0.0.0.0") ){

#ifdef MODULE_CONFIG_WIFI_CONCURRENT_MODE
	wifiBridgeProcess(WIFI_BRIDGE_DEL_ETH0_AND_WLAN1);
#else
	wifiBridgeProcess(WIFI_BRIDGE_DEL_ETH0_AND_WLAN0);
#endif
	goto ___lan_close_end___;
}else{
	goto ____lan_close_no_bridge___;
}
#endif


____lan_close_no_bridge___:
#if AM_CHIP_ID == 1213
	sprintf(callbuf,"killall udhcpc");
	system(callbuf);
	
	sleep(3);
	sprintf(callbuf,"ifconfig eth0 0.0.0.0");
	system(callbuf);
	sprintf(callbuf,"ifconfig eth0 down");
	system(callbuf);
#if  MODULE_CONFIG_EZCASTPRO_MODE==8075
	getIfconfigInfo("br0",IFCONFIG_IP,ipTemp,16 + 1);
	//printf("lan_close  ipTemp=%s\n",ipTemp);
	if( strlen(ipTemp) >strlen("0.0.0.0") )
	{
	}
	else
	{
	}
	
#endif
#endif

#if AM_CHIP_ID == 1211
	sprintf(callbuf,"killall udhcpc");
	system(callbuf);
	sprintf(callbuf,"ifconfig eth0 down");
	system(callbuf);
	
	
	disable_route_function(LAN_ROUTING);

#endif

___lan_close_end___:
		
#if EZCAST_ENABLE
	set3G4GDefault();
#endif
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075	
	Probox_set_led(LEDLAN_OFF);		
#endif

#if ROUTER_ONLY_ENABLE
	if(getRouterOnlyStatus() != 0 && get_ap_close_flag() != 0){
		wifi_softap_start();
	}
#endif

	lanEnableFlag=0;
	lanIfupFlag = 0;
	SWFEXT_FUNC_END();	
}
static int lan_stop(void *handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	char callbuf[50];
#if AM_CHIP_ID == 1213
	sprintf(callbuf,"rmmod am7x_net");
	system(callbuf);
	
	
#endif

#if AM_CHIP_ID == 1211
	sprintf(callbuf,"rmmod ax88796c_net");
	system(callbuf);

	
	
#endif
		
#if EZCAST_ENABLE
	set3G4GDefault();
#endif
	lanEnableFlag=0;
	lanIfupFlag = 0;


	SWFEXT_FUNC_END();	
}

static int lan_open_manual(void *handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	char callbuf[128];
	int ret=-1;
	FILE *fp;
	FILE *fpin;
	char *ip=Swfext_GetString();
	char *mask=Swfext_GetString();
	char *gateway=Swfext_GetString();
	char *first_dns=Swfext_GetString();
	char *second_dns=Swfext_GetString();
    char buf[256];
	int rtn;
	char *pub_interface=NULL;
	char *private_interface=NULL;
	char *private_subnet=NULL;
	
	lanEnableFlag=0;
#if AM_CHIP_ID == 1213
	if(access("/sys/module/am7x_net",F_OK)==-1){
#if EEPROM_TYPE == 1
				sleep(3);
				if(access("/sys/module/i2c_am7x",F_OK)==-1){
					sprintf(callbuf,"%s%s/%s","insmod  ",AM_SYS_LIB_DIR,"i2c-am7x.ko");
					system(callbuf);
				}
				sleep(3);
				if(access("/sys/module/at24_i2c",F_OK)==-1){
					sprintf(callbuf,"%s%s/%s","insmod  ",AM_SYS_LIB_DIR,"at24_i2c.ko");
					system(callbuf);
				}
#elif EEPROM_TYPE == 2
				sleep(3);
				if(access("/sys/module/am7x_spi",F_OK)==-1){
					sprintf(callbuf,"%s%s/%s","insmod  ",AM_SYS_LIB_DIR,"am7x_spi.ko");
					system(callbuf);
				}
				sleep(3);
				if(access("/sys/module/at93_spi",F_OK)==-1){
					sprintf(callbuf,"%s%s/%s","insmod  ",AM_SYS_LIB_DIR,"at93_spi.ko");
					system(callbuf);
				}
#endif
		sprintf(callbuf,"insmod lib/modules/2.6.27.29/am7x_net.ko");
		system(callbuf);
	}	
#endif
#if AM_CHIP_ID == 1211
	if(access("/sys/module/ax88796c_net",F_OK)==-1){
			sprintf(callbuf,"insmod lib/modules/2.6.27.29/ax88796c_net.ko");
			system(callbuf);
	}
#endif
	udhcpd_dns_enable();
	sleep(3);
	sprintf(callbuf,"ifconfig eth0 %s netmask %s up",ip,mask) ;
	lanIfupFlag = 1;
	printf("the callbuf is %s\n",callbuf);
	
	fpin = popen(callbuf,"r");
	fprintf(stderr,"function:%s line:%d fpin:%p\n",__FUNCTION__,__LINE__,fpin);
	
	if(fpin == NULL){
			sprintf(callbuf,"ifconfig eth0 down") ;
			ret=-1;
			system(callbuf);
			goto __lan_open_manual___;
		}

	pclose(fpin);

	sprintf(callbuf,"route del default gw %s",pre_gateway);
	ret=system(callbuf);
	
	sprintf(callbuf,"route add default gw %s",gateway) ;
	ret=system(callbuf);
	if(ret!=0){
			sprintf(callbuf,"route del default gw %s",gateway);
			ret=system(callbuf);
			find_gateway(ip);
			memcpy(gateway,gateway_data_from_cul,50);
			sprintf(callbuf,"route add default gw %s",gateway) ;
			ret=system(callbuf);
		}	
	pre_gateway=gateway;

	if(access("/mnt/vram/wifi/resolv.conf",F_OK)==-1){
			sprintf(callbuf,"mkdir /mnt/vram/wifi/");
			system(callbuf);
			memset(callbuf,0,128);
			sprintf(callbuf,"cp /mnt/user1/dns/resolv.conf /mnt/vram/wifi/resolv.conf");
			system(callbuf);
			memset(callbuf,0,128);
		}
	fp = fopen("/mnt/vram/wifi/resolv.conf","r");

	if(fp == NULL){
			printf("read resolv.conf open error!");
			ret=-1;
			
			goto __lan_open_manual___;
		}
	ret=fread(buf, 1, 256, fp);

	ret=fclose(fp);
	
#if EZCAST_ENABLE
	if(access("/tmp/dnsmasq.conf", F_OK)==0){
		unlink("/tmp/dnsmasq.conf");
	}
	system("cp /etc/dnsmasq.conf /tmp/dnsmasq.conf");
	memset(buf, 0, sizeof(buf));
	snprintf(buf, sizeof(buf), "echo server=%s >> /tmp/dnsmasq.conf", first_dns);
	system(buf);
	memset(buf, 0, sizeof(buf));
	snprintf(buf, sizeof(buf), "echo server=%s >> /tmp/dnsmasq.conf", second_dns);
	system(buf);

    /* Mos: When set Lan manual IP, should set resolv.conf with same configuration */
    memset(buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf), "echo nameserver %s > /mnt/vram/wifi/resolv.conf", first_dns);
    system(buf);
    memset(buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf), "echo nameserver %s >> /mnt/vram/wifi/resolv.conf", second_dns);
    system(buf);
#else
	memset(buf,0,256);
	sprintf(buf,"search actions.com.cn\nnameserver %s\nnameserver %s\n",first_dns,second_dns);
	if(buf!=NULL)
			fp = fopen("/mnt/vram/wifi/resolv.conf","wb+");
		
	if(fp == NULL){
			fprintf(stderr,"function:%s line:%d errno:%d\n",__FUNCTION__,__LINE__,errno);
			ret=-1;
			
			goto __lan_open_manual___;
		}
	ret=fwrite(buf, 1, 256, fp);
	ret=fflush(fp);
	int fd_write = fileno(fp);
	ret=fsync(fd_write);
	ret=fclose(fp);
	
	if(ret!=0){
		goto __lan_open_manual___;
	}
#endif
	
	sleep(5);
	check_ip_arping(ip);
	ret=read_arping_reply();
	printf("ret======%d\n",ret);
	if(ret!=0){
		ret=-2;
		//clear_lan_error();
		
		lanEnableFlag=0;
		goto __lan_open_manual___;
	}
	lan_access_mode=2;
	printf("ret======%d\n",ret);
	
#if EZCAST_ENABLE
	set3G4GMode(0);
#endif
	lanEnableFlag = 1;

#if ROUTER_ONLY_ENABLE
	if(getRouterOnlyStatus() != 0){
		if(get_ap_close_flag() == 0){
			char *lanIP = getLanIp();
			if(strlen(lanIP) > strlen("0.0.0.0")){
				wifi_softap_close();
			}
		}
	}else
#endif
	{
		pub_interface="eth0";
		private_interface="wlan1";
#if EZCAST_ENABLE
		#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE!=0
			private_subnet="192.168.168.1/24";		// EZCastpro
		#else
			private_subnet="192.168.203.1/24";		// EZCast/EZMusic/EZWilan
		#endif
#else
		private_subnet="192.168.111.1/24";
#endif

#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075
			//probox route etho to wlan1,wlan3	with dif ip 
			//enable_route_function(pub_interface,"wlan3","192.168.203.1/24",LAN_ROUTING);
			enable_route_function(pub_interface,"br0","192.168.168.1/24",LAN_ROUTING);
			printf("[%s][%d]wifiBridgeProcess\n",__func__,__LINE__);
			sleep(1);
			//wifiBridgeProcess(WIFI_BRIDGE_ADD_BR0_AND_WLAN1_WLAN3);
#else
		
		enable_route_function(pub_interface,private_interface,private_subnet,LAN_ROUTING);
		
#endif	
	}
	start_thttpd_process();
	
__lan_open_manual___:		

	Swfext_PutNumber(ret);

	SWFEXT_FUNC_END();	
}

static int ip_mask_match_judge(void *handle)
{
		SWFEXT_FUNC_BEGIN(handle);
		int ret=-1;
		
		unsigned short int ip_1=Swfext_GetNumber();
		
		unsigned short int ip_2=Swfext_GetNumber();
		
		unsigned short int ip_3=Swfext_GetNumber();
		unsigned short int ip_4=Swfext_GetNumber();
		
		unsigned short int mask_1=Swfext_GetNumber();
		
		unsigned short int mask_2=Swfext_GetNumber();
		
		unsigned short int mask_3=Swfext_GetNumber();
		
		unsigned short int mask_4=Swfext_GetNumber();
		printf("ip1==%d\n,ip2==%d\n,ip3==%d\n,ip4==%d\n",ip_1,ip_2,ip_3,ip_4);
		
		printf("mask_1==%d\n,mask_2==%d\n,mask_3==%d\n,mask_4==%d\n",mask_1,mask_2,mask_3,mask_4);
		ret=mask_match_mask_judging(ip_1,ip_2,ip_3,ip_4,mask_1,mask_2,mask_2,mask_4);
		
		Swfext_PutNumber(ret);
		SWFEXT_FUNC_END();	
}

static int ip_gateway_match(void *handle){
		SWFEXT_FUNC_BEGIN(handle);
		int ret=-1;
		
		unsigned short int ip_1=Swfext_GetNumber();
		
		unsigned short int ip_2=Swfext_GetNumber();
		
		unsigned short int ip_3=Swfext_GetNumber();
		unsigned short int ip_4=Swfext_GetNumber();
		
		unsigned short int gateway_1=Swfext_GetNumber();
		
		unsigned short int gateway_2=Swfext_GetNumber();
		
		unsigned short int gateway_3=Swfext_GetNumber();
		
		unsigned short int gateway_4=Swfext_GetNumber();
		//printf("ip1==%d\n,ip2==%d\n,ip3==%d\n,ip4==%d\n",ip_1,ip_2,ip_3,ip_4);
		
	//	printf("gateway_1==%d\n,gateway_2==%d\n,gateway_3==%d\n,gateway_4==%d\n",gateway_1,gateway_2,gateway_3,gateway_4);
		ret=GatewayMatchJudging(ip_1,ip_2,ip_3,ip_4,gateway_1,gateway_2,gateway_3,gateway_4);

	//	printf("ip_gateway_match----------%d",ret);

		Swfext_PutNumber(ret);
		SWFEXT_FUNC_END();	
}

static int get_lan_acess_mode(void *handle)
{
		SWFEXT_FUNC_BEGIN(handle);
		
		Swfext_PutNumber(lan_access_mode);
		
		SWFEXT_FUNC_END();	
}

#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE!=0
char net_manual_buf[NET_MANUAL_CONFIG_BUFFER_LENGTH]={0};
static int lan_mac_get_for_landisplay(void *handle)
{
		char macTemp[17 + 1] = {0};
		
		SWFEXT_FUNC_BEGIN(handle);
		printf("%s called\n",__func__);
		memset(macTemp, 0, 17 + 1);
		
		getIfconfigInfo("br0",IFCONFIG_HWADDR,macTemp,17 + 1);

		if(	strlen(macTemp) > strlen("0.0.0.0") )
			getIfconfigInfo("br0",IFCONFIG_HWADDR,mac_address,50);
		else
			getIfconfigInfo("eth0",IFCONFIG_HWADDR,mac_address,50);
		printf("got %s\n",mac_address);			
		Swfext_PutString(mac_address);
	
		SWFEXT_FUNC_END();	
}
static int ezcastpro_net_config_reload(void *handle)
{
		SWFEXT_FUNC_BEGIN(handle);
		int ret=Swfext_GetNumber();
		memset(net_manual_buf,0,NET_MANUAL_CONFIG_BUFFER_LENGTH);		
		switch(ret){
			case 0://wifi client
				ret=ezcastpro_config_readwrite(net_manual_buf,CLIENT_MANUAL_CONFIG_PATH);				
				break;
			case 1://LAN
				ret=ezcastpro_config_readwrite(net_manual_buf,LAN_MANUAL_CONFIG_PATH);			
				break;				
		}
		printf("%s ret==%d\n",__func__,ret);
		if(ret!=0)memset(net_manual_buf,0,NET_MANUAL_CONFIG_BUFFER_LENGTH);
		Swfext_PutString(net_manual_buf);
		SWFEXT_FUNC_END();	
}
static int ezcastpro_net_config_save(void *handle)
{
		SWFEXT_FUNC_BEGIN(handle);
		int ret=Swfext_GetNumber();		
		char* input_str=Swfext_GetString();
		switch(ret){
			case 0://wifi client
				ret=ezcastpro_config_readwrite(input_str,CLIENT_MANUAL_CONFIG_PATH);				
				break;
			case 1://LAN
				ret=ezcastpro_config_readwrite(input_str,LAN_MANUAL_CONFIG_PATH);
				system("cat /mnt/user1/lan/manual.conf");				
				break;				
		}
		Swfext_PutNumber(ret);		
		SWFEXT_FUNC_END();	
}
#endif

#if EZWILAN_ENABLE
typedef struct lanStore_s{
	char ip[16];
	char mask[16];
	char gateway[16];
	char dns1[16];
	char dns2[16];
}lanStore_t;

#define LANINFO_STORE_PATH			"/mnt/vram/ezcast/laninfo.bin"
#define LAN_AUTOMATIC_DISABLE		"/mnt/vram/ezcast/LANAUTOMATICDISABLE"

enum{
	LANSETTING_IP = 0,
	LANSETTING_GATEWAY,
	LANSETTING_MASK,
	LANSETTING_DNS1,
	LANSETTING_DNS2
};
int lan_getLanAutomaticEnable(void *handle){
	int isAutomaticEnable = 0;
	SWFEXT_FUNC_BEGIN(handle);
	if(access(LAN_AUTOMATIC_DISABLE, F_OK) != 0){
		isAutomaticEnable = 1;
	}
	Swfext_PutNumber(isAutomaticEnable);
	SWFEXT_FUNC_END();
}
int lan_setLanAutomaticEnable(void *handle){
	int isAutomaticEnable = 0;
	SWFEXT_FUNC_BEGIN(handle);
	isAutomaticEnable = !!Swfext_GetNumber();
	EZCASTLOG("isAutomaticEnable: %d\n", isAutomaticEnable);
	if(isAutomaticEnable == 0){
		if(access(LANINFO_STORE_PATH, F_OK) == 0 && access(LAN_AUTOMATIC_DISABLE, F_OK) != 0){
			FILE *fp = NULL;
			fp = fopen(LAN_AUTOMATIC_DISABLE, "w");
			if(fp != NULL)
				fclose(fp);
		}
	}else{
		if(access(LAN_AUTOMATIC_DISABLE, F_OK) == 0){
			unlink(LAN_AUTOMATIC_DISABLE);
		}
	}
	sync();
	SWFEXT_FUNC_END();
}

#if WEBSETTING_ENABLE
int webset_lan_getLanAutomaticEnable(){
	int isAutomaticEnable = 0;
	if(access(LAN_AUTOMATIC_DISABLE, F_OK) != 0){  //文件不存在
		isAutomaticEnable = 1;  //on
	}
	return isAutomaticEnable;
}

int webset_lan_setLanAutomatic_on(){
	
	int isAutomaticEnable;
		if(access(LAN_AUTOMATIC_DISABLE, F_OK) == 0){
			unlink(LAN_AUTOMATIC_DISABLE);
		//printf("=====off  file===22222222222222222222222====\n");

		}
	sync();
	
	isAutomaticEnable=webset_lan_getLanAutomaticEnable();
	//printf("=====webset_lan_setLanAutomaticEnable==on===%d\n",isAutomaticEnable);
	
	return isAutomaticEnable;
}

int webset_lan_setLanAutomatic_off(){
	
	int isAutomaticEnable;
	
	if(access(LANINFO_STORE_PATH, F_OK) == 0 && access(LAN_AUTOMATIC_DISABLE, F_OK) != 0){
		FILE *fp = NULL;
		fp = fopen(LAN_AUTOMATIC_DISABLE, "w");
		//printf("=====creat file===111111111111111111====\n");
		if(fp != NULL)
			fclose(fp);
		}
	sync();
	
	isAutomaticEnable=webset_lan_getLanAutomaticEnable();
	//printf("=====webset_lan_setLanAutomaticEnable===off====%d\n",isAutomaticEnable);
	
	return isAutomaticEnable;
}
int webset_getLanSettingautoVal(int flag){
	char *val = "Auto";
	if(get_netlink_status() != 0)
	{
		switch(flag){
			case LANSETTING_IP:
					val = getLanIp();
				break;
			case LANSETTING_GATEWAY:
					val = getLanGateway();
				break;
			case LANSETTING_MASK:
					val = getLanMask();
				break;
			case LANSETTING_DNS1:
					dns_get_from_resolv_file();
					val = first_dns_for_display;
				break;
			case LANSETTING_DNS2:
					dns_get_from_resolv_file();
					val = second_dns_for_display;
				break;
			default:
				break;
		}
	}
	return val;
}


#endif
int getLanSettingVal(int flag){
	int ret = -1;
	char *val = NULL;
	static lanStore_t lanStoreData;
	int isInfoStore = 0;
	if(access(LANINFO_STORE_PATH, F_OK) == 0){
		FILE *fp = fopen(LANINFO_STORE_PATH, "r");
		if(fp != NULL){
			ret = fread(&lanStoreData, sizeof(lanStore_t), 1, fp);
			if(ret > 0){
				isInfoStore = 1;
			}
			fclose(fp);
		}
	}
	switch(flag){
		case LANSETTING_IP:
			if(isInfoStore == 1)
				val = lanStoreData.ip;
			else
				val = getLanIp();
			break;
		case LANSETTING_GATEWAY:
			if(isInfoStore == 1)
				val = lanStoreData.gateway;
			else
				val = getLanGateway();
			break;
		case LANSETTING_MASK:
			if(isInfoStore == 1)
				val = lanStoreData.mask;
			else
				val = getLanMask();
			break;
		case LANSETTING_DNS1:
			if(isInfoStore == 1)
				val = lanStoreData.dns1;
			else{
				dns_get_from_resolv_file();
				val = first_dns_for_display;
			}
			break;
		case LANSETTING_DNS2:
			if(isInfoStore == 1)
				val = lanStoreData.dns2;
			else{
				dns_get_from_resolv_file();
				val = second_dns_for_display;
			}
			break;
		default:
			break;
	}

	return val;
}
int lan_getLanSettingVal(void *handle){
	int flag = 0;
	char *val = NULL;
	SWFEXT_FUNC_BEGIN(handle);
	flag = Swfext_GetNumber();
	val = getLanSettingVal(flag);
	if(val != NULL){
		Swfext_PutString(val);
	}else{
		EZCASTWARN("Get val error[flag:%d]!!!\n", flag);
		Swfext_PutString("error");
	}
	SWFEXT_FUNC_END();
}

int lan_storeLanManualInfo(void *handle){
	char *ip = NULL;
	char *mask = NULL;
	char *gateway = NULL;
	char *dns1 = NULL;
	char *dns2 = NULL;
	lanStore_t lanStoreData;
	FILE *fp = NULL;
	
	SWFEXT_FUNC_BEGIN(handle);
	ip = Swfext_GetString();
	mask = Swfext_GetString();
	gateway = Swfext_GetString();
	dns1 = Swfext_GetString();
	dns2 = Swfext_GetString();
	
	memset(lanStoreData.ip, 0, sizeof(lanStoreData.ip));
	memset(lanStoreData.mask, 0, sizeof(lanStoreData.mask));
	memset(lanStoreData.gateway, 0, sizeof(lanStoreData.gateway));
	memset(lanStoreData.dns1, 0, sizeof(lanStoreData.dns1));
	memset(lanStoreData.dns2, 0, sizeof(lanStoreData.dns2));
	
	snprintf(lanStoreData.ip, 16, "%s", ip);
	snprintf(lanStoreData.mask, 16, "%s", mask);
	snprintf(lanStoreData.gateway, 16, "%s", gateway);
	snprintf(lanStoreData.dns1, 16, "%s", dns1);
	snprintf(lanStoreData.dns2, 16, "%s", dns2);
	
	fp = fopen(LANINFO_STORE_PATH, "w");
	if(fp != NULL){
		fwrite(&lanStoreData, sizeof(lanStore_t), 1, fp);
		fclose(fp);
		sync();
	}else{
		EZCASTWARN("open [%s] error!!!\n", LANINFO_STORE_PATH);
		perror("error");
	}
	SWFEXT_FUNC_END();
}

#if WEBSETTING_ENABLE

int webset_lan_storeLanManualInfo(char *val){

	char setcmd_tmp[256];
	char *ip = NULL;
	char *mask = NULL;
	char *gateway = NULL;
	char *dns1 = NULL;
	char *dns2 = NULL;

	if(val!=NULL)
		sprintf(setcmd_tmp,"%s",val);
	else
		return -1;
	
	//printf("=====0000===webset_lan_storeLanManualInfo====%s\n",setcmd_tmp);
	
	ip=strtok(setcmd_tmp,"\n");
	mask=strtok(NULL,"\n");
	gateway=strtok(NULL,"\n");
	dns1=strtok(NULL,"\n");
	dns2=strtok(NULL,"\n");
	
//printf("=====0000===webset_lan_storeLanManualInfo====%d\n",webset_ip_gateway_match(ip,gateway));
//printf("=====1111===webset_lan_storeLanManualInfo====%d\n",webset_ip_mask_match_judge(ip,mask));
    if(webset_ip_gw_subnet_check(ip,gateway,mask)!=0)
        return -1;
	if(webDnsCheck(dns1)!=0)
		dns1 = " ";
	if(webDnsCheck(dns2)!=0)
		dns2 = " ";

	lanStore_t lanStoreData;
	FILE *fp = NULL;

	memset(lanStoreData.ip, 0, sizeof(lanStoreData.ip));
	memset(lanStoreData.mask, 0, sizeof(lanStoreData.mask));
	memset(lanStoreData.gateway, 0, sizeof(lanStoreData.gateway));
	memset(lanStoreData.dns1, 0, sizeof(lanStoreData.dns1));
	memset(lanStoreData.dns2, 0, sizeof(lanStoreData.dns2));
	
	snprintf(lanStoreData.ip, 16, "%s", ip);
	snprintf(lanStoreData.mask, 16, "%s", mask);
	snprintf(lanStoreData.gateway, 16, "%s", gateway);
	snprintf(lanStoreData.dns1, 16, "%s", dns1);
	snprintf(lanStoreData.dns2, 16, "%s", dns2);

	//printf("=====manual save ok!===webset_lan_storeLanManualInfo====%s\n",lanStoreData.ip);

	fp = fopen(LANINFO_STORE_PATH, "w");
	if(fp != NULL){
		fwrite(&lanStoreData, sizeof(lanStore_t), 1, fp);
		fclose(fp);
		sync();
		return 1;

	}else{
		EZCASTWARN("open [%s] error!!!\n", LANINFO_STORE_PATH);
		perror("error");
		return 0;

	}
}

char * webset_lan_getMacAddress()
{
	static char mac[32];
	if(getLanMacAddress(mac) == 0){
		//printf("=====webset_lan_getMacAddress====%s\n",mac);
		return mac;

	}else{
		return 0;
	}
}

#endif
int getLanMacAddress(char *mac_address_info){
	char callbuf[50];
	int ret=-1;
	FILE *fp;
	char buf[512];
	char mask_get_from_ifconfig[50]={0};
	
	memset(callbuf,0,50);
#ifdef MODULE_CONFIG_WIFI_BRIDGE_ENABLE	
	sprintf(callbuf,"ifconfig br0 > /tmp/ifconfig.info");
#else
	sprintf(callbuf,"ifconfig eth0 > /tmp/ifconfig.info");
#endif
	printf("the call is %s\n",callbuf); 
	system(callbuf);
	fp = fopen("/tmp/ifconfig.info","r");
	if(NULL == fp)
	{
			fprintf(stderr,"function:%s line:%d \n",__FUNCTION__,__LINE__);	
			printf("fopen error:%s\n",strerror(errno));
			return -1;
	}		
	ret=fread(buf, 1, 512, fp);
	fclose(fp);

	if(ret <= 0 ){
		fprintf(stderr,"function:%s line:%d ret:%d\n",__FUNCTION__,__LINE__,ret);
		return -1;
	}
	
	char *locate3=strstr(buf,"HWaddr ");
	if(locate3==NULL)
		return -1;
	
	locate3+=strlen("HWaddr ");
	char *locate3_tmp=locate3;
	char *locate4=strstr(locate3,"\n");
	if(locate4==NULL)
		return -1;
	
	memset(mask_get_from_ifconfig,0,50);
	memcpy(mask_get_from_ifconfig,locate3_tmp,strlen(locate3_tmp)-strlen(locate4));

	memset(mac_address_info,0,18);
	strncpy(mac_address_info,mask_get_from_ifconfig,17);
	
	printf("mac_address_info============%s\n",mac_address_info);
	return 0;
}
int lan_getMacAddress(void *handle){
	char mac[32];
	SWFEXT_FUNC_BEGIN(handle);
	if(getLanMacAddress(mac) == 0){
		Swfext_PutString(mac);
	}else{
		Swfext_PutString("error");
	}
	SWFEXT_FUNC_END();
}
#endif

int swfext_lan_register(void)
{
	SWFEXT_REGISTER("lan_Open", lan_open);
	SWFEXT_REGISTER("lan_Stop", lan_stop);
	
	SWFEXT_REGISTER("lan_Close", lan_close);
	SWFEXT_REGISTER("lan_openmanual", lan_open_manual);
	SWFEXT_REGISTER("lan_ip_display", lan_ip_get_for_landisplay);
	SWFEXT_REGISTER("lan_mask_display", lan_mask_get_for_landisplay);
	SWFEXT_REGISTER("lan_gateway_display", lan_gateway_get_for_landisplay);
	SWFEXT_REGISTER("lan_firstdns_display", lan_firstdns_get_for_landisplay);
	SWFEXT_REGISTER("lan_seconddns_display", lan_seconddns_get_for_landisplay);
	SWFEXT_REGISTER("ip_maskmatchjudge", ip_mask_match_judge);
	SWFEXT_REGISTER("lan_acess_mode", get_lan_acess_mode);
	SWFEXT_REGISTER("ip_gatewaymatch", ip_gateway_match);
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE!=0
	SWFEXT_REGISTER("lan_mac_display", lan_mac_get_for_landisplay);
	SWFEXT_REGISTER("ezcastpro_net_config_reload", ezcastpro_net_config_reload);
	SWFEXT_REGISTER("ezcastpro_net_config_save", ezcastpro_net_config_save);
#endif		
#if EZWILAN_ENABLE
	SWFEXT_REGISTER("lan_getLanAutomaticEnable", lan_getLanAutomaticEnable);
	SWFEXT_REGISTER("lan_setLanAutomaticEnable", lan_setLanAutomaticEnable);
	SWFEXT_REGISTER("lan_getLanSettingVal", lan_getLanSettingVal);
	SWFEXT_REGISTER("lan_storeLanManualInfo", lan_storeLanManualInfo);
	SWFEXT_REGISTER("lan_getMacAddress", lan_getMacAddress);
#endif
	return 0;
}
#else

int swfext_lan_register(void)
{
	return 0;
}

#endif

