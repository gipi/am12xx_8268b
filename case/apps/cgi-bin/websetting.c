#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "cgic.h"
#include "am_cgi.h"
#include "am_socket.h"
#include "htmlsetting_cgi.h"
#ifndef __ACT_MODULE_CONFIG_H__ //For LinuxTool bug!! (i.e.; #if !define xxx)
#define __ACT_MODULE_CONFIG_H__
#include "../../../scripts/mconfig.h"
#endif
#include "../../include/ezcast_public.h"

struct process_list_s{
	char name[64];
	int (*cgiMain)(int argc, char *argv[]);
};

#define F_FILENAME      "fileName"
#define FILENAME_LEN    1024
#ifdef BUF_LEN
	#undef BUF_LEN
#endif
#define BUF_LEN         10240
#define CMD_LEN			64
#define PATH "/tmp/domainpath"
#define LOG_PATH	"/tmp/upload_log"

#define FILE_HELPVIEW	"helpview.jpg"
#define FILE_APPINFO	"appInfo.json"
#define FILE_KEY		"key"
#define FILE_TRIALKEY	"trialkey"
#define FILE_SUBTITLE1	"ezsubtitle.srt"
#define FILE_SUBTITLE2	"ezsubtitle.smi"
#define FILE_SUBTITLE3	"ezsubtitle.ssa"
#define FILE_SUBTITLE4	"ezsubtitle.cdg"
#define FILE_SUBTITLE5	"time.txt"

#define FILE_SUBTITLE	"ezsubtitle"

#define SUPPORT_FILES	{\
							FILE_HELPVIEW,\
							FILE_APPINFO,\
							FILE_KEY,\
							FILE_TRIALKEY,\
							FILE_SUBTITLE1,\
							FILE_SUBTITLE2,\
							FILE_SUBTITLE3,\
							FILE_SUBTITLE4,\
							FILE_SUBTITLE5 }

#define PATH_TMP		"/tmp"
#define PATH_EZDATA		"/tmp/ezdata"

#define WRITE_LOG	0

#if WRITE_LOG
#define EZCAST_WRITE_LOG(fmt, args...) do { struct timeval tv; gettimeofday(&tv,NULL); char msg[1024]; snprintf(msg, 1024, "-%06lu.%06lu- "fmt,tv.tv_sec,tv.tv_usec,##args); write_log(msg); } while (0)
#else
#define EZCAST_WRITE_LOG(fmt, args...)
#endif

#if 0
static int init_send_socket(struct sockaddr_un *addr, char *path){
	int sockfd;  
	sockfd=socket(AF_UNIX,SOCK_DGRAM,0);  
	if(sockfd<0)  
	{  
		return -1;  
	}  
	bzero(addr,sizeof(struct sockaddr_un));  
	addr->sun_family=AF_UNIX;  
	strcpy(addr->sun_path,path);  
	return sockfd;
}

static int send_to_socket(int sockfd, char msg[], const struct sockaddr_un * addr)  
{  
	int len;   
	len = strlen(addr->sun_path)+sizeof(addr->sun_family);  
	sendto(sockfd, msg, strlen(msg), 0, (struct sockaddr*)addr,len);   
	return 1;  
}

static int upload_send_command(char *msg){
	int fd;  
    struct sockaddr_un addr;  

    if( (fd = init_send_socket(&addr, PATH)) > 0){
		send_to_socket(fd, msg, &addr); 
		close(fd);
		return 0;
	}

	return -1;
}
#endif

int isFileInvalid(char *fileName){
	if(fileName == NULL){
		return -1;
	}

	int i = 0;
	char files[][64] = SUPPORT_FILES;

	for(i=0; i<sizeof(files)/sizeof(files[0]); i++){
		if(strcmp(fileName, files[i]) == 0)
			return 0;
		else{
			char *p = strrchr(fileName, '.');
			if(p != NULL && strcmp(p, ".pem") == 0)
				return 0;
		}
	}

	return 1;
}

static void write_log(char *str){
	if(access(LOG_PATH, F_OK) == 0){
		struct stat info;
		stat(LOG_PATH, &info);
		if(info.st_size >= 102400)
			unlink(LOG_PATH);
	}

	FILE *fp = fopen(LOG_PATH, "a");
	if(fp != NULL){
		fwrite(str, 1, strlen(str), fp);
		fclose(fp);
	}
}

static int isSubtitle(char *file_name){
	char *tmp;

	tmp = strrchr(file_name, '.');
	if((tmp-file_name)!=strlen(FILE_SUBTITLE)){
		//printf("the length of \"%s\" is: %d, the length of \"%s\" is: %d\n", file_name, (tmp-file_name), FILE_SUBTITLE, strlen(FILE_SUBTITLE));
		return -1;
	}
	if(!strncmp(file_name, FILE_SUBTITLE, strlen(FILE_SUBTITLE))){
		return 0;
	}

	return -1;
}
static int send_command(char *cmd){
	htmlsetting_func("htmlsetting_app_command", cmd, strlen(cmd), NULL, NULL);
	return 0;
}

static int file_to_command(char *file_name, char *command){
	//char cmd[256];
	
	if(!strcmp(file_name, FILE_HELPVIEW)){
		strncpy(command, CMD_HELPVIEW, strlen(CMD_HELPVIEW));
		return 0;
	}
	if(!strcmp(file_name, FILE_APPINFO)){
		strncpy(command, CMD_APPINFO, strlen(CMD_APPINFO));
		return 0;
	}
	//if(!strcmp(file_name, FILE_SUBTITLE1) || !strcmp(file_name, FILE_SUBTITLE2) || !strcmp(file_name, FILE_SUBTITLE3)){
	//if(!isSubtitle(file_name)){
	//	snprintf(cmd, sizeof(cmd), "mv %s/%s %s/%s", PATH_TMP, file_name, PATH_EZDATA, file_name);
	//	system(cmd);
	//}
	if(!strcmp(file_name, FILE_KEY)){
		strncpy(command, CMD_KEY, strlen(CMD_KEY));
		return 0;
	}
	if(!strcmp(file_name, FILE_TRIALKEY)){
		strncpy(command, CMD_TRIALKEY, strlen(CMD_TRIALKEY));
		return 0;
	}

	return 1;
}

static int  Ck_uploadfileExist(char *filepath){
	
	int cert_exist= access(filepath,F_OK);
	char Vrambak_pem[64]="cp -f /etc/cert/*.pem /mnt/vram/cert/ ";
	htmlsetting_func("system_cmd", (void*)Vrambak_pem, strlen(Vrambak_pem), NULL, NULL);
	return cert_exist;
}

static int Is_CApemfile(char *filepath){
	if(strstr(filepath,".pem"))
		return 1;
	else
		return 0;
}
static int Is_Timesyncfile(char *filepath){
	if(strcmp(filepath,"time.txt"))
		return 0;
	else
		return 1;
}
int wifi_create_dir(char * dir_path)
{
	int rtn=0;
	rtn = access(dir_path,W_OK);
	if(rtn!=0){
		rtn = mkdir(dir_path,0777);
		if(rtn!=0){
			printf("Make Dir Failed=%s\n",dir_path);
		}
		//system("sync");
	}
	return rtn;	
}

int upload(int argc, char *argv[])
{
    cgiFilePtr cfp;
    char rfile[FILENAME_LEN] /*remote file */ ,
        lfile[FILENAME_LEN] /* local file */ , buf[BUF_LEN], 
        cmd[CMD_LEN];
    int ret, fd, fsize, got;
	int rn=0;
    if ((ret = cgiFormFileName(F_FILENAME, rfile, sizeof(rfile))) !=
        cgiFormSuccess) {
		cgiHeaderContentType("text/html");
        fprintf(cgiOut, "<p>No file was uploaded.(%d)</p>", ret);
        EZCAST_WRITE_LOG("No file was uploaded.(%d)", ret);
		fprintf(cgiOut,"<script>window.location.href='./../upload.html'</script>");
		return -1;
    }
	 cgiHeaderContentType("text/html");
	fprintf(cgiOut,"<body>");
	fprintf(cgiOut,"<script> $('body').css({'background-color':'#000000'});</script>");
    cgiFormFileSize(F_FILENAME, &fsize);
	fprintf(cgiOut,"<p>---- filename=%sfilesize: %d<p>",F_FILENAME, fsize);
	EZCAST_WRITE_LOG("---- filesize: %d\n", fsize);
   
	if(fsize > 2*1024*1024){
		printf("--fail-- The file is too large!!!\n");
        EZCAST_WRITE_LOG("--fail-- The file is too large!!!\n");
		fprintf(cgiOut,"<script>window.location.href='./../upload.html'</script>");
		return -1;
	}

	if(isFileInvalid(rfile) != 0){
		printf("--fail-- The file[%s] is invalid!!!\n", rfile);
        EZCAST_WRITE_LOG("--fail-- The file[%s] is invalid!!!\n", rfile);
		//fprintf(cgiOut,"<script>window.location.href='./../upload.html'</script>");
		return -1;
	}

    if ((ret = cgiFormFileOpen(F_FILENAME, &cfp)) != cgiFormSuccess) {
        fprintf(cgiOut, "<p>open cgi file failed.(%d)</p>", ret);
        EZCAST_WRITE_LOG("open cgi file failed.(%d)", ret);
		fprintf(cgiOut,"<script>window.location.href='./../upload.html'</script>");
        return -1;
    }
	if(!isSubtitle(rfile)){
    	snprintf(lfile, sizeof(lfile), "/tmp/ezdata/%s", rfile);
	}else{
    	snprintf(lfile, sizeof(lfile), "/tmp/%s", rfile);
	}
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE!=0
	if(Is_CApemfile(rfile)){
		wifi_create_dir("/etc/cert/");
		//sprintf(cmd,￡?"%s","rm /etc/ssl/ca/*.pem");
		//htmlsetting_func("system_cmd", (void*)cmd, strlen(cmd), NULL, NULL);
		memset(lfile,0,FILENAME_LEN);
		snprintf(lfile, sizeof(lfile), "/etc/cert/Certificate.pem");	
	}
	if(Is_Timesyncfile(rfile)){
		memset(lfile,0,FILENAME_LEN);
		snprintf(lfile, sizeof(lfile), "/mnt/vram/%s",rfile);	
	}
	
#endif
    fprintf(cgiOut, "write to %s\n", lfile);
	EZCAST_WRITE_LOG("write to %s\n", lfile);
    if ((fd = open(lfile, O_CREAT | O_WRONLY,
              S_IRWXU | S_IRWXG | S_IRWXO)) < 0) {
        fprintf(cgiOut, "<p>create file on server failed.(%s)</p>",
                strerror(errno));
		EZCAST_WRITE_LOG("create file on server failed.(%s)", strerror(errno));
		fprintf(cgiOut,"<script>window.location.href='./../upload.html'</script>");
        return -1;
    }
    while (cgiFormFileRead(cfp, buf, sizeof(buf), &got) == cgiFormSuccess) {
        if ((ret = write(fd, buf, got)) < 0) {
            fprintf(cgiOut, "<p> write failed(%d/%s)</p>\n", errno,
                    strerror(errno));
			EZCAST_WRITE_LOG("write failed(%d/%s)\n", errno, strerror(errno));
        }
        fprintf(cgiOut, "<p>got(%d)</p>\n", got);
		EZCAST_WRITE_LOG("got(%d)\n", got);
    }
    close(fd);
    cgiFormFileClose(cfp);
	memset(cmd, 0, sizeof(cmd));
	if(!file_to_command(rfile, cmd))
		send_command(cmd);
	rn=Ck_uploadfileExist(lfile);
	fprintf(cgiOut, "---- filesize: %d\n", fsize);
	EZCAST_WRITE_LOG("---- filesize: %d\n", fsize);
	sleep(3);
	
    fprintf(cgiOut, "<p> end </p>\n");
	fprintf(cgiOut,"</body>");
	fprintf(cgiOut,"<script>window.location.href='./../upload.html?type=%d'</script>",rn);

	//fprintf(cgiOut,"<script>window.location.href='./upload.html'</script>");
    return 0;
}

int wifi_info_GET(int argc, char *argv[])
{
	int ret;
	char cmd_string[512]={0};
	int probox_enable=-1;
    cgiHeaderCacheControl("no-cache, no-store, must-revalidate");
    cgiHeaderPragma("no-cache");
    cgiHeaderExpires(0);
	cgiHeaderContentType("text/html");

	cgiFormString("type", cmd_string, 512);
     
	if(strstr(cmd_string,"get_AM_TYPE")){
#if (MODULE_CONFIG_EZCASTPRO_MODE==8075)
		probox_enable=1;
#else
		probox_enable=0;
#endif
		fprintf(cgiOut,"%d\n",probox_enable);		

	}
	
	else if(!strcmp(cmd_string,"EZMUSIC_ENABLE"))
	{
	
#if MODULE_CONFIG_EZCAST_ENABLE
	#if MODULE_CONFIG_EZCASTPRO_MODE 
		#if (MODULE_CONFIG_EZCASTPRO_MODE==8075)
			#if MODULE_CONFIG_LAN_ONLY
				fprintf(cgiOut,"%s","9");//pro box lan only
			#else
				fprintf(cgiOut,"%s","6");//pro box
			#endif
		#else
			#if MODULE_CONFIG_EZWILAN_ENABLE
				fprintf(cgiOut,"%s","5");//pro lan
			#else
				fprintf(cgiOut,"%s","4");//pro dongle
			#endif
		#endif
	#elif MODULE_CONFIG_EZWILAN_ENABLE
		fprintf(cgiOut,"%s","3");//ezcat lan
	#elif MODULE_CONFIG_EZMUSIC_ENABLE
		fprintf(cgiOut,"%s","1");
	#elif MODULE_CONFIG_EZWIRE_ENABLE
		#if (MODULE_CONFIG_EZWIRE_TYPE==1 || MODULE_CONFIG_EZWIRE_TYPE>=5)
			fprintf(cgiOut,"%s","8"); //not audio:  1:MiraWire with 8252B;  5:SNOR flash with 8252N; 6: MiraWire with CDC/EDM; 7: MiraWire with CDC/EDM at snor
		#else
			fprintf(cgiOut,"%s","7");//EZWIRE type, 0: EZWire with 8251/8251W; 2:MiraWireDuo with 8252C; 3: MiraWirePlus with 8252B/8252W; 4: EZDocking with 8251W;
		#endif
	#else
		fprintf(cgiOut,"%s","2");
	#endif
#endif
		
	}
	else if(!strcmp(cmd_string,"OTA_SERVER_ENABLE"))
	{
#if (MODULE_CONFIG_EZCASTPRO_MODE==8075)
	#if OTA_SERVER_SET_ENABLE
		fprintf(cgiOut,"%s","1");
	#else
		fprintf(cgiOut,"%s","0");
	#endif
#else
	
	#if OTA_SERVER_SET_ENABLE
		fprintf(cgiOut,"%s","1");
	#else
		fprintf(cgiOut,"%s","0");
	#endif
#endif
	}
	else if(!strcmp(cmd_string,"CONFIG_BUSINESS_CUSTOMER"))
	{
	#if MODULE_CONFIG_BUSINESS_CUSTOMER
		fprintf(cgiOut,"%s","1");
	#else
		fprintf(cgiOut,"%s","0");
	#endif
	}
	else if(!strcmp(cmd_string,"ROUTER_ONLY_ENABLE"))
	{
	#if ROUTER_ONLY_ENABLE
		fprintf(cgiOut,"%s","1");
	#else
		fprintf(cgiOut,"%s","0");
	#endif
	}
	else if(!strcmp(cmd_string,"CONFIG_3TO1"))
	{
	#ifdef MODULE_CONFIG_3TO1
		fprintf(cgiOut,"%d",MODULE_CONFIG_3TO1);
	#else
		fprintf(cgiOut,"%s","null");
	#endif
	}
	else
	{
		char receiveline[1000]={0};
		ret=create_websetting_client(cmd_string);
		if(ret<0)
			return -1;
		websetting_client_write(cmd_string);
		ret=websetting_client_read(receiveline);
		fprintf(cgiOut,"%s",receiveline);
	}
	return ret;
}

int password_POST(int argc, char *argv[])
{
	int ret;
	char cmd_string[100]={0};
	cgiHeaderContentType("text/html");
	cgiFormStringNoNewlines("appsk", cmd_string, 40);
	ret=create_websetting_client("cgi_setting_server");
	if(ret<0)
		return -1;
	strcat(cmd_string,"set_softap_psk");
	websetting_client_write(cmd_string);
	close_websetting_socket_cli_fd();
	return 0;
}

int add_network_POST(int argc, char *argv[])
{
	int ret;
	char cmd_string[100]={0};
	char ssid_result[40] = {0};
	char psk_result[40] = {0};
	char AuthenType[5]={0};
	cgiHeaderContentType("text/html");
	cgiFormStringNoNewlines("ssid", ssid_result, 40);
	cgiFormStringNoNewlines("psk", psk_result, 40);
	cgiFormStringNoNewlines("AuthenType", AuthenType, 5);
	ret=create_websetting_client("cgi_setting_server");
	if(ret<0)
		return -1;
	strncpy(cmd_string,ssid_result,strlen(ssid_result));
	strcat(cmd_string,"\n");
	strcat(cmd_string,psk_result);
	strcat(cmd_string,"\n");
	strcat(cmd_string,AuthenType);
	strcat(cmd_string,"\n");
	strcat(cmd_string,"add_network");//add network
	websetting_client_write(cmd_string);
	close_websetting_socket_cli_fd();
	return 0;
}

int set_wifi_POST(int argc, char *argv[])
{
	int ret;
	char cmd_string[150]={0};
	char ssid_result[40] = {0};
	char psk_result[65] = {0};
	char ssid_index[5]={0};
	cgiHeaderContentType("text/html");
	cgiFormStringNoNewlines("ssid", ssid_result, 40);
	cgiFormStringNoNewlines("psk", psk_result, 65);
	cgiFormStringNoNewlines("ssid_index", ssid_index, 5);
	ret=create_websetting_client("cgi_setting_server");
	if(ret<0)
		return -1;
	strncpy(cmd_string,ssid_result,strlen(ssid_result));
	strcat(cmd_string,"\n");
	strcat(cmd_string,psk_result);
	strcat(cmd_string,"\n");
	strcat(cmd_string,ssid_index);
	strcat(cmd_string,"\n");
	strcat(cmd_string,"set_wifi");
	websetting_client_write(cmd_string);
	close_websetting_socket_cli_fd();
	return 0;
}

int delete_password_POST()
{
	int ret;
	char cmd_string[100]={0};
	char ssid_result[40] = {0};
	char ssid_index[5]={0};
	cgiHeaderContentType("text/html");
	cgiFormStringNoNewlines("ssid_index", ssid_index, 5);
	cgiFormStringNoNewlines("ssid", ssid_result, 40);
	ret=create_websetting_client("cgi_setting_server");
	if(ret<0)
		return -1;
	strncpy(cmd_string,ssid_index,strlen(ssid_index));
	strcat(cmd_string,"\n");
	strcat(cmd_string,ssid_result);
	strcat(cmd_string,"\n");
	strcat(cmd_string,"delete_password");
	websetting_client_write(cmd_string);
	close_websetting_socket_cli_fd();
	return 0;
}

int set_defaultmode_POST(int argc, char *argv[])
{
	int ret;
	char cmd_string[100]={0};
	char default_mode_value[5]={0};
	cgiHeaderContentType("text/html");
	cgiFormStringNoNewlines("Default_mode_value", default_mode_value, 5);
	ret=create_websetting_client("cgi_setting_server");
	if(ret<0)
		return -1;
	strcat(cmd_string,"set_default_mode");	
	strcat(cmd_string,"\n");
	strcat(cmd_string,default_mode_value);
	strcat(cmd_string,"\n");		
	websetting_client_write(cmd_string);
	close_websetting_socket_cli_fd();
	return 0;
}

int pushdongleinfo(int argc, char *argv[]){
	char *msg = NULL;
	int msg_size = 0;
	//*recv_buffer = NULL;
    cgiHeaderCacheControl("no-cache, no-store, must-revalidate");
    cgiHeaderPragma("no-cache");
    cgiHeaderExpires(0);
	cgiHeaderContentType("text/html");

#if 0
    if( (fd_send = init_send_socket(&addr, SEND_PATH)) > 0 && (fd_recv = init_recv_socket(RECV_PATH))> 0){
		send_to_socket(fd_send, CMD_GETDONGLEINFO, &addr); 
		//memset(recv_buffer, 0, MSG_SIZE);  
		if(receive_from_socket(fd_recv))  
		{  
			//printf("cgi: %s\n", recv_buffer);
		}
		close(fd_send);
		close(fd_recv);
		return 0;
	}

	if(fd_send > 0)
		close(fd_send);
	if(fd_recv > 0)
		close(fd_recv);
#else
	htmlsetting_func("htmlsetting_app_command", CMD_GETDONGLEINFO, strlen(CMD_GETDONGLEINFO), (void **)&msg, &msg_size);
	if(msg != NULL){
		fprintf(cgiOut, "%s\n", msg);
		del_return((void **)&msg);
	}
#endif

	return -1;
}

#define ARP_PATH	"/proc/net/arp"
#define BUFF_LEN	2048

int get_my_mac(int argc, char *argv[])
{
    char *ip_address;
	int arp_fd;
	int buff_len;
	char buff[BUFF_LEN];
	char *ip_p;
	char ipaddress[32];
	int hw_type;
	int flags;
	char hw_address[32];
	char mask[8];
	char device[8];
    cgiHeaderCacheControl("no-cache, no-store, must-revalidate");
    cgiHeaderPragma("no-cache");
    cgiHeaderExpires(0);
    cgiHeaderContentType("text/html");

	ip_address = getenv("REMOTE_ADDR");
	if(ip_address == NULL){
		//fprintf(cgiOut, "get ip address error!!\n");
		return -1;
	}
	//fprintf(cgiOut, "My ip address is %s\n", ip_address);
	arp_fd = open(ARP_PATH, O_RDONLY);
	if(arp_fd < 0){
		//fprintf(cgiOut, "open arp file error!!!\n");
		return -1;
	}
	//fprintf(cgiOut, "arp_fd = %d\n", arp_fd);
	memset(buff, 0, 2048);
	//fprintf(cgiOut, "buff cleaned!!!\n");
	buff_len = read(arp_fd, buff, BUFF_LEN);
	if(buff_len < 0){
		//fprintf(cgiOut, "read arp file error!!!\n");
		close(arp_fd);
		return -1;
	}
	//fprintf(cgiOut, "%s\n", buff);
	close(arp_fd);
	//fprintf(cgiOut, "arp_fd closed!!!\n");
	ip_p = strstr(buff, ip_address);
	if(ip_p == NULL){
		//fprintf(cgiOut, "Could not find the ip %s\n", ip_address);
		return 1;
	}
	//fprintf(cgiOut, "\n%s\n", ip_p);
	sscanf(ip_p, "%s%x%x%s%s%s", ipaddress, &hw_type, &flags, hw_address, mask, device);
	fprintf(cgiOut, "%s", hw_address);
	
    return 0;
}

int set_resolution_POST(int argc, char *argv[])
{
	int ret;
	char cmd_string[100]={0};
	char res_value[10]={0};
	char current_res_value[5]={0};
	cgiHeaderContentType("text/html");
	cgiFormStringNoNewlines("res_value", res_value, 10);
	cgiFormStringNoNewlines("current_res_value", current_res_value, 5);
	ret=create_websetting_client("cgi_setting_server");
	if(ret<0)
		return -1;
	if(strstr(res_value,"OK")!= 0)
	{
		char temp[8];
		strncpy(temp,res_value,(strlen(res_value)-2));
		strncpy(cmd_string,"set_resolution",strlen("set_resolution"));
		strcat(cmd_string,"\n");
		strcat(cmd_string,temp);
		strcat(cmd_string,"\n");
		strcat(cmd_string,"OK");
	}
	else
	{
		strncpy(cmd_string,"set_resolution",strlen("set_resolution"));	
		strcat(cmd_string,"\n");
		strcat(cmd_string,res_value);
		strcat(cmd_string,"\n");	
		strcat(cmd_string,current_res_value);
		strcat(cmd_string,"\n");	
	}
	websetting_client_write(cmd_string);
	close_websetting_socket_cli_fd();
	return 0;
}

/*
**	start of "factory_test"
*/
static int get_keybuf_value(char *buf,char *key,char *value)
{
	char * locate1 = NULL;
	int key_len=0;
	locate1 = strstr(buf,key);
	if(locate1!=NULL)
	{
		char * locate2 = NULL;
		//if((locate2=strstr(locate1,"&"))==NULL)
		locate2 = strchr(locate1,',');
	
		if(locate2!=NULL)
		{
			key_len=strlen(key);
			
			memcpy(value,(locate1+key_len),locate2-locate1-key_len);

			//printf("key value=%s\n",value);
		}
		else
			return -1;
		//value=tmp;
	}
	else
		return -1;
	return 0;
}

static int show_result(char *buf)
{

	char value[64]="";
	if(!get_keybuf_value(buf,"1=",value)){
		fprintf(cgiOut,"Wifi Throuthput:%s\n<br>",value);
		memset(value,0,64);
	}
	if(!get_keybuf_value(buf,"2=",value)){
		fprintf(cgiOut,"Wifi Channel_1:%s\n<br>",value);
		memset(value,0,64);
	}
	if(!get_keybuf_value(buf,"3=",value)){
		fprintf(cgiOut,"Wifi Channel_6::%s\n<br>",value);
		memset(value,0,64);
	}
	if(!get_keybuf_value(buf,"4=",value)){
		fprintf(cgiOut,"Wifi Channel_11:%s\n<br>",value);
		memset(value,0,64);
	}
	if(!get_keybuf_value(buf,"5=",value)){
		fprintf(cgiOut,"Edid Resolution:%s\n<br>",value);
		memset(value,0,64);
	}
	if(!get_keybuf_value(buf,"6=",value)){
		fprintf(cgiOut,"Version Test:%s\n<br>",value);
		memset(value,0,64);
	}
	if(!get_keybuf_value(buf,"7=",value)){
		fprintf(cgiOut,"Language Config:%s\n<br>",value);
		memset(value,0,64);
	}

	return 1;
}
int get_url_inputs()
{
	//char method[10];
	int post_len;
	char *inputstring=NULL;
	//char *index;
	int *Dongle_res = NULL,*Test_result=NULL,ret = -1;	
	/*
	strcpy(method,getenv("REQUEST_METHOD"));
	if(!strcmp(method,"POST"))
	{
		fprintf(cgiOut,"post!!!\n");	
		post_len=atoi(getenv("CONTENT_LENGHT"));
		if(post_len!=0)
		{
			inputstring=malloc(sizeof(char)*post_len+1);
			fread(inputstring,sizeof(char),post_len,stdin); 
		}
	}
	*/
	
		//inputstring=malloc(1024);
	inputstring=getenv("QUERY_STRING");
	post_len=strlen(inputstring);
	if(strlen(inputstring)>0)
	{
		//cmd  执行时间长 cgi会超时
		if(!strcmp(inputstring,"testresult=true")){
			htmlsetting_func("factory_test_cmd", (void*)inputstring, post_len, (void **)&Test_result, NULL);
			char *factory_test_result = (char *)Test_result;
			show_result(factory_test_result);
			del_return((void **)&Test_result);
		}
		else{
			htmlsetting_func("factory_test_cmd", (void*)inputstring, post_len, (void **)&Dongle_res, NULL);
			if(Dongle_res){
				ret = *Dongle_res;
				fprintf(cgiOut,"%d\n",ret); 
			}else{
				ret = -1;
				fprintf(cgiOut,"Fail to get Dongle Response!!!\n"); 
			}
			del_return((void **)&Dongle_res);
		}
		
	}
	else 
	{
		return 0;
	}
	return 1;
}

int factory_test(int argc,char*argv[])
{
	cgiHeaderCacheControl("no-cache, no-store, must-revalidate");
	cgiHeaderPragma("no-cache");
	cgiHeaderExpires(0);
	cgiHeaderContentType("text/html");

	int inputs=0;
	if((inputs=get_url_inputs())!=1)
		fprintf(cgiOut,"cgi Get inputs Error");
	return 0;
}

/*
**	end of "factory_test"
*/
int set_devicename(int argc, char *argv[])
{
	int ret;
	char cmd_string[100]={0};
	cgiHeaderContentType("text/html");
	cgiFormStringNoNewlines("devicename", cmd_string, 40);
	ret=create_websetting_client("cgi_setting_server");
	if(ret<0)
		return -1;
	strcat(cmd_string,"set_device_name");
	websetting_client_write(cmd_string);
	close_websetting_socket_cli_fd();
	return 0;
}

int set_devicename_reboot(int argc, char *argv[])
{
	int ret;
	char cmd_string[100]={0};
	cgiHeaderContentType("text/html");
	cgiFormStringNoNewlines("devicename", cmd_string, 40);
	ret=create_websetting_client("cgi_setting_server");
	if(ret<0)
		return -1;
	strcat(cmd_string,"set_devicename_reboot");
	websetting_client_write(cmd_string);
	close_websetting_socket_cli_fd();
	return 0;
}

int set_lan_POST(int argc, char *argv[])
{
	int ret;
	char cmd_string[100]={0};
	char lan_index[5]={0};
	cgiHeaderContentType("text/html");
	cgiFormStringNoNewlines("lan_index", lan_index, 5);
	ret=create_websetting_client("cgi_setting_server");
	if(ret<0)
		return -1;
	strncpy(cmd_string,"set_language",strlen("set_language"));
	strcat(cmd_string,"\n");
	strcat(cmd_string,lan_index);
	strcat(cmd_string,"\n");
	websetting_client_write(cmd_string);
	close_websetting_socket_cli_fd();
	return 0;
}

int cgiMain(int argc, char *argv[])
{
	struct process_list_s process_list[]={
		{"wifi_info_GET.cgi", wifi_info_GET},
		{"password_POST.cgi", password_POST},
		{"add_network_POST.cgi", add_network_POST},
		{"set_wifi_POST.cgi", set_wifi_POST},
		{"delete_password_POST.cgi", delete_password_POST},
		{"set_defaultmode_POST.cgi", set_defaultmode_POST},
		{"get_my_mac.cgi", get_my_mac},
		{"upload.cgi", upload},
		{"set_resolution_POST.cgi", set_resolution_POST},
		{"factory_test.cgi", factory_test},
		{"set_devicename.cgi", set_devicename},
		{"set_devicename_reboot.cgi", set_devicename_reboot},
		{"pushdongleinfo.cgi", pushdongleinfo},
		{"set_lan_POST.cgi", set_lan_POST},
		{"dongleInfo.json", pushdongleinfo},
	};

	if(argc > 0)
	{
		char *pn = getenv("SCRIPT_NAME");
		char *process = NULL;
		int i = 0;

		process = strrchr(pn, '/');
		if(process == NULL)
			process = pn;
		else
			process++;

		if(process == NULL)
			return -1;

		for(i=0; i<sizeof(process_list)/sizeof(struct process_list_s); i++)
		{
			if(strcmp(process, process_list[i].name) == 0)
			{
				return process_list[i].cgiMain(argc, argv);
			}
		}
	}

	return -1;
}

