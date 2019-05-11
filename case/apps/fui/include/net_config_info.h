#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include "ezcastpro.h"
#define NET_CONFIG_INFO_DBG(fmt, arg...) printf("NET_CONFIG_INFO_DBG[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)


typedef enum {
	IFCONFIG_IP,
	IFCONFIG_MASK,
	IFCONFIG_BRDADDR,
	IFCONFIG_HWADDR,
	
}ifconfigCmd_s;

int getIfconfigInfo(char *ifFace,ifconfigCmd_s ifconfigCmd,void *resultInfo,int resultInfoLength);
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE!=0
#include <fcntl.h>
#define NET_MANUAL_CONFIG_BUFFER_LENGTH 83
#define NET_MANUAL_CONFIG_MINIMUM_LENGTH 43
#define LAN_MANUAL_CONFIG_PATH "/mnt/user1/lan/manual.conf"
#define CLIENT_MANUAL_CONFIG_PATH "/mnt/user1/clientap/manual.conf"
int ezcastpro_config_readwrite(char* str,char* filepath);
#endif

