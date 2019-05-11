#include <unistd.h>
#include "file_list.h"
#include "wifi_engine.h"
#include "filelist_engine.h"
#include <sys/wait.h>
#include <linux/errno.h>
#include "ezcast_public.h"
#if 0
	#define wifi_route_dbg(fmt, arg...) printf("WIFIINF[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)
	
#else
	#define wifi_route_dbg(fmt, arg...)
#endif

void disable_route_function(int disable_type);


static int route_has_enbale=0;
route_type_emun route_type=-1;
int get_dns_form_resolve_conf(char *dns1,char *dns2){
	FILE *fp;
	int ret;
	char tmp_buf[128];
	int dns1_hs_copy=0;
	int dns2_hs_copy=0;
	char *locate_namesever1=NULL;
	char *locate_enter1=NULL;
	char *locate_namesever2=NULL;
	char *locate_enter2=NULL;
	
	if(access("/etc/resolv.conf",F_OK)!=0)	{
		ret=system("ln -s /mnt/vram/wifi/resolv.conf /etc/resolv.conf");
		if( ret == 0)
			printf("[%s]link resolv.conf ok!!\n",__FUNCTION__);
		else{
			printf("[%s]link resolv.conf error!!errno is %d\n",__FUNCTION__,errno);
			return -1;
		}
		system("sync");
	}
	fp = fopen("/etc/resolv.conf","r");
	if(fp==NULL)
		return -1;
	while(!feof(fp)){
		memset(tmp_buf,0x00,128);
		fgets(tmp_buf,128,fp);
		wifi_route_dbg("tmp_buf=========%s\n",tmp_buf);
		if((locate_namesever1=strstr(tmp_buf,"nameserver "))&&dns1_hs_copy==0){
			locate_enter1=strstr(locate_namesever1,"\n");
			wifi_route_dbg("strlen(locate_namesever1)-strlen(locate_enter1)=====%d\n",strlen(locate_namesever1)-strlen(locate_enter1)-strlen("nameserver "));
			strncpy(dns1,locate_namesever1+strlen("nameserver "),strlen(locate_namesever1)-strlen(locate_enter1)-strlen("nameserver "));
			dns1_hs_copy=1;
			continue;
		}
		if((locate_namesever2=strstr(tmp_buf,"nameserver "))&&dns2_hs_copy==0){
			locate_enter2=strstr(locate_namesever2,"\n");
			wifi_route_dbg("strlen(locate_namesever2)-strlen(locate_enter2)=====%d\n",strlen(locate_namesever2)-strlen(locate_enter2)-strlen("nameserver "));
			strncpy(dns2,locate_namesever2+strlen("nameserver "),strlen(locate_namesever2)-strlen(locate_enter2)-strlen("nameserver "));
			dns2_hs_copy=1;
			
			continue;
		}
		if(dns1_hs_copy&&dns2_hs_copy){
			
			wifi_route_dbg("dns1=========%s\n",dns1);
			wifi_route_dbg("dns2=========%s\n",dns2);
			break;
		}
	}
	fclose(fp);
	return 0;	
}

/*
*check the named.conf file 
*/
void checkAndRecoverForwardFile(){
	int ret = 0;
	char cmd[128] = {0};
	struct stat buf;

	if(access("/mnt/user1/named/named.conf",F_OK)!=0){
		memset(cmd,0,sizeof(cmd));
		sprintf(cmd,"cp /mnt/bak/named.conf  /mnt/user1/named/named.conf");
		ret=system(cmd);
		if(ret == 0)
			printf("[%s,%d]recover named.conf ok!!\n",__FUNCTION__,__LINE__);
		else
			printf("[%s,%d]recover named.conf error!!\n",__FUNCTION__,__LINE__);
		system("sync");
	}

	if(access("/etc/named.conf",F_OK)!=0){
		memset(cmd,0,sizeof(cmd));
		sprintf(cmd,"ln -s  /mnt/user1/named/named.conf /etc/named.conf");
		ret=system(cmd);
		if(ret == 0)
			printf("[%s,%d]link named.conf ok!!\n",__FUNCTION__,__LINE__);
		else
			printf("[%s,%d]link named.conf error!!\n",__FUNCTION__,__LINE__);
		system("sync");
	}
	else{
		lstat("/etc/named.conf",&buf);						//check the symbolic link.if unexist ,rebuild it
		if(!S_ISLNK(buf.st_mode)){
			memset(cmd,0,sizeof(cmd));
			sprintf(cmd,"ln -sf /mnt/user1/named/named.conf /etc/named.conf");
			ret=system(cmd);
			if(ret == 0)
				printf("[%s,%d]link named.conf ok!!\n",__FUNCTION__,__LINE__);
			else
				printf("[%s,%d]link named.conf error!!\n",__FUNCTION__,__LINE__);
			system("sync");

		}
	}

}

int write_dns_forward_file(char *dns1,char *dns2){
	FILE *fp;
	int ret;
	char buf[2048]={0};
	char tail_buf[1024]={0};
	char *locate1=NULL;
	char *buf_init=NULL;
	memset(buf,0,sizeof(buf));
	memset(tail_buf,0,sizeof(tail_buf));
	
	checkAndRecoverForwardFile();
	
	fp = fopen("/mnt/user1/named/named.conf","r");
	if(fp == NULL){
		fprintf(stderr,"function:%s line:%d errno:%d\n",__FUNCTION__,__LINE__,errno);
		return -1;
	}

	ret=fread(buf, 1, sizeof(buf), fp);	
	ret=fclose(fp);
	
	buf_init=buf;
	wifi_route_dbg("*****buf_init****\n%s\n",buf_init);
	locate1=strstr(buf,"forwarders {");
	if(locate1==NULL){
		
		wifi_route_dbg("cannot find forwarders {");
		return -1;
	}
	#if 1
	if(strlen(dns2)>0)
		sprintf(tail_buf,"\n                %s;\n                %s;\n                168.95.1.1;\n        };\n};",dns1,dns2);
	else
		sprintf(tail_buf,"\n                %s;\n                168.95.1.1;\n        };\n};",dns1);
	#else
	sprintf(tail_buf,"\n                %s;\n                %s;\n        };\n}%c",dns1,dns2,0x3B);	
	#endif
	wifi_route_dbg("tail_buf=========%s\n",tail_buf);
		
	memset(locate1+strlen("forwarders {"),0,strlen(locate1+strlen("forwarders {")));
	
	strncpy(locate1+strlen("forwarders {"),tail_buf,strlen(tail_buf));
	
	wifi_route_dbg("buf_init=========%s\n",buf_init);
	
	fp = fopen("/mnt/user1/named/named.conf","w+");
	if(fp == NULL){
		fprintf(stderr,"function:%s line:%d errno:%d\n",__FUNCTION__,__LINE__,errno);
		return -1;
	}
	
	ret=fwrite(buf_init, 1, strlen(buf_init), fp);

	ret=fflush(fp);

	int fd_write = fileno(fp);
	ret=fsync(fd_write);

	ret=fclose(fp);
	
	return 0;

	
}

void change_DNS_forward_server(){
	char dns1[16]={0};
	char dns2[16]={0};;
	int ret=get_dns_form_resolve_conf(dns1,dns2);
	wifi_route_dbg("dns1=========%s\n",dns1);
	wifi_route_dbg("dns2=========%s\n",dns2);
	wifi_route_dbg("ret=========%d\n",ret);
	#if 0//EZMUSIC_ENABLE
		printf("\n--------------EZMusic:checkAndRecoverForwardFile----------\n");
		checkAndRecoverForwardFile();
	#else
	
	if(ret==0&&(strlen(dns1)>0||strlen(dns2)>0))
		if(write_dns_forward_file(dns1,dns2)!=0)			//if named.conf empty or other error , restore the named.conf
			system("cp /mnt/bak/named.conf /mnt/user1/named/named.conf");
	#endif
}


void enable_route_function(char *pub_interface,char *private_interface,char *private_subnet,int routing_type){
	
	char callbuf[128]={0};

	wifi_route_dbg("pub_interface===%s,private_interface===%s,private_subnet======%s,routing_type===%d\n",pub_interface,private_interface,private_subnet,routing_type);

	switch(routing_type){
		case LAN_ROUTING:
				disable_route_function(WIFI_ROUTING);
				break;
		case WIFI_ROUTING:
				disable_route_function(LAN_ROUTING);
				break;
				
	}
	
	wifi_route_dbg("change_DNS_forward_server begin!");
	change_DNS_forward_server();
	
	wifi_route_dbg("change_DNS_forward_server OK!");
	memset(callbuf,0,128);
	sprintf(callbuf,"sh %s%s %s %s %s",AM_CASE_SCRIPTS_DIR,"iptalbes_nat.sh",pub_interface,private_interface,private_subnet);
	pac_system(callbuf);

	memset(callbuf,0,128);
	sprintf(callbuf,"sh %s%s",AM_CASE_SCRIPTS_DIR,"dns_forward.sh");
	pac_system(callbuf);
	route_has_enbale=1;
	route_type=routing_type;
}

void disable_route_function(int disable_type){
		if(route_has_enbale==0||disable_type!=route_type)
			return;
		char callbuf[128]={0};
		memset(callbuf,0,128);
		sprintf(callbuf,"sh %s%s",AM_CASE_SCRIPTS_DIR,"route_exit.sh");
		system(callbuf);
		route_has_enbale=0;
}

