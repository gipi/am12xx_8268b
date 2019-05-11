#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/vfs.h>
#include <errno.h>
#include <sys/time.h>
#include "swf_ext.h"
#include "system_info.h"
#include "sys_rtc.h"
#include "fileselector.h"
#include "media_hotplug.h"
#include "sys_msg.h"
#include "am7x_dac.h"
#include "sys_vram.h"
#include "apps_vram.h"
#include "lcm_op.h"
#include "sys_cfg.h"
#include "am7x_dac.h"
#include "display.h"
#include "cardupgrade.h"
#include "image_decode.h"
#include "sys_pmu.h"
#include "file_list.h"
#include "wifi_engine.h"
#include "sys_gpio.h"
#include "ezcast_public.h"
#include "websetting.h"
#include "customization_info.h"

#if MODULE_CONFIG_LAN
extern char lanEnableFlag;
#endif

static int custom_ioctl_operation(const char *device, int op, void *addr_of_arg){
	
	int fd;
	fd = open(device, O_RDWR);
	printf("open device: %s\n", device);
	
	if(fd < 0){
		printf("cant open device: %s\n", device);
		return -1;
	}
	else{
		ioctl(fd, op, addr_of_arg);
		close(fd);
	}
	return 0;
}

static int custom_send_wifi_status(void *handle){
	//int fd = open("/dev/uartcom",O_RDWR);
	int wifi_status;
	int ret;
	
	SWFEXT_FUNC_BEGIN(handle);
	wifi_status = Swfext_GetNumber();
	/*if(fd < 0){
		printf("cant open uartcom device\n");
	}
	else{
		wifi_status = Swfext_GetNumber();
		printf("%s wifi_status=%d\n", __func__, wifi_status);
		ioctl(fd, IOCTL_GET_WIFI_STATUS, &wifi_status);
		close(fd);
	}*/
	
	if((ret = custom_ioctl_operation("/dev/uartcom", IOCTL_GET_WIFI_STATUS, &wifi_status)) == 0){
		printf("%s wifi_status=%d\n", __func__, wifi_status);
	}
	else{
		printf("/dev/uartcom ioctl fail\n");
	}
	SWFEXT_FUNC_END();
}

static void slice_ip(char *ip_addr, int *client_ip){
	int i = 0;
	char *delim = ".";
	char *pch;
	
	printf("%s ip_addr=%s\n", __func__, ip_addr);
	pch = strtok(ip_addr, delim);
	while(pch != NULL){
		//printf("client ip[%d] = %d\n", i, atoi(pch));
		client_ip[i] = atoi(pch);
		i++;
		pch = strtok(NULL, delim);
	}
}

#if 0
static int custom_send_ap_ip(void *handle){
	int fd = open("/dev/uartcom",O_RDWR);
	int ret = -1;
	int client_ip[4];
	char ip_addr[17];
	
	SWFEXT_FUNC_BEGIN(handle);
	ret = getIP(1, ip_addr);
	if(ret < 0){
		printf("[%s][%d] -- Get IP error!!!\n", __func__, __LINE__);
		goto IP_ERROR;
	}

	/*if(fd < 0){
		printf("cant open uartcom device\n");
	}
	else{
		ioctl(fd, 10, ip_addr);
		close(fd);
	}*/
	slice_ip(ip_addr, client_ip);
	if((ret = custom_ioctl_operation("/dev/uartcom", IOCTL_GET_CLIENT_IP, client_ip)) == 0){
		printf("%s - send IP\n", __func__);
	}
	else{
		printf("/dev/uartcom ioctl fail\n");
	}
IP_ERROR:
	
	SWFEXT_FUNC_END();
}
#endif

static int custom_set_vol(void *handle){
	//int fd = open("/dev/uartcom",O_RDWR);
	int vol_diff;
	int ret;
	SWFEXT_FUNC_BEGIN(handle);
	
	/*if(fd < 0){
		printf("cant open uartcom device\n");
	}
	else{
		vol_diff = Swfext_GetNumber();
		printf("%s vol=%d\n", __func__, vol_diff);
		ioctl(fd, 22, &vol_diff);
		close(fd);
	}
	*/
	vol_diff = Swfext_GetNumber();
	if((ret = custom_ioctl_operation("/dev/uartcom", IOCTL_SET_ATOP_VOLUME, &vol_diff)) == 0){
		printf("%s vol=%d\n", __func__, vol_diff);
	}
	else{
		printf("/dev/uartcom ioctl fail\n");
	}
	SWFEXT_FUNC_END();
}

static int custom_get_power_on_message(void *handle){
	int ret;
	char power_on_message[UART_MAX_STRING];
	char info[UART_MAX_STRING];
	SWFEXT_FUNC_BEGIN(handle);
	memset(power_on_message, 0, UART_MAX_STRING);
	memset(info, 0, UART_MAX_STRING);
	if((ret = custom_ioctl_operation("/dev/uartcom", IOCTL_GET_POWER_ON_MESSAGE, power_on_message)) == 0){
		sprintf(info, "%d%d%d%d%d%d%d", power_on_message[0]);
		printf("%s poweron_message=%s\n", __func__, power_on_message);
		Swfext_PutString(power_on_message);
	}
	else{
		printf("/dev/uartcom ioctl fail\n");
	}
	SWFEXT_FUNC_END();
}

static int custom_get_start_source(void *handle){
	int start_source;
	int ret;
	
	SWFEXT_FUNC_BEGIN(handle);
	if((ret = custom_ioctl_operation("/dev/uartcom", IOCTL_GET_SOURCE, &start_source)) == 0){
		if(start_source == -1){
			start_source = 224;
		}
		else{
			start_source += 176;
		}
		printf("%s start_source=%d\n", __func__, start_source);
	}
	else{
		start_source = 224;
		printf("/dev/uartcom ioctl fail\n");
	}
	Swfext_PutNumber(start_source);
	SWFEXT_FUNC_END();
}

static int custom_get_language(void *handle){
	int lang_index = -1;
	int ret;
	
	SWFEXT_FUNC_BEGIN(handle);
	if((ret = custom_ioctl_operation("/dev/uartcom", IOCTL_GET_LANGUAGE, &lang_index)) == 0){
		printf("%s get lang index = %d\n", __func__, lang_index);
	}
	else{
		printf("/dev/uartcom ioctl fail\n");
	}
	Swfext_PutNumber(lang_index);
	SWFEXT_FUNC_END();
}

static int custom_pipe_get_lan_in_flag(void *handle){//am_pipe 14
	int ret = -1;
	int lan_in_status = 0;
	
	SWFEXT_FUNC_BEGIN(handle);
	if((ret = custom_ioctl_operation("/dev/am7x_pipe", CMD_GET_LAN_PORT_FLAG, &lan_in_status)) == 0){
		printf("%s lan_in_flag = %d\n", __func__, lan_in_status);
	}
	else{
		printf("/dev/am7x_pipe ioctl get fail\n");
	}
	Swfext_PutNumber(lan_in_status);
	SWFEXT_FUNC_END();
}

void custom_pipe_set_lan_in_flag(int lan_status){//am_pipe 15
	int ret = -1;
	
	if((ret = custom_ioctl_operation("/dev/am7x_pipe", CMD_SET_LAN_IN_FLAG, &lan_status)) == 0){
		printf("%s lan_in_flag = %d\n", __func__, lan_status);
	}
	else{
		printf("/dev/am7x_pipe ioctl set fail\n");
	}
}

int swfext_customization_register(void)
{
	SWFEXT_REGISTER("custom_send_wifistatus", custom_send_wifi_status);
	//SWFEXT_REGISTER("custom_send_apip", custom_send_ap_ip);
	SWFEXT_REGISTER("custom_set_volume", custom_set_vol);
	SWFEXT_REGISTER("custom_get_poweron_message", custom_get_power_on_message);
	SWFEXT_REGISTER("custom_get_startsource", custom_get_start_source);
	SWFEXT_REGISTER("custom_get_scaler_language", custom_get_language);
	
	SWFEXT_REGISTER("custom_get_lan_in_flag", custom_pipe_get_lan_in_flag);
	return 0;
}
