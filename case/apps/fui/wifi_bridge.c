#include <unistd.h>
#include "file_list.h"
#include "wifi_engine.h"
#include "filelist_engine.h"
#include <sys/wait.h>
#include <linux/errno.h>

#if 1
	#define wifi_bridge_dbg(fmt, arg...) printf("WIFI_BRIDGE_DBG[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)
	
#else
	#define wifi_bridge_dbg(fmt, arg...)
#endif

#define WIFI_BRIDGE_SCRIPTS_PATH         	"/am7x/case/scripts/wifi_bridge_process.sh"



int wifiBridgeProcess(wifi_bridge_process_t processChoose){

	char cmd[128] = {0};
	int ret = -1;
	memset(cmd,0,128);
	switch(processChoose){
		case WIFI_BRIDGE_ADD_BR0:
			snprintf(cmd,128,"sh %s %s",WIFI_BRIDGE_SCRIPTS_PATH,"wifi_bridge_add_br0");
			break;
			
		case WIFI_BRIDGE_ENABLE_BR0:
			snprintf(cmd,128,"sh %s %s",WIFI_BRIDGE_SCRIPTS_PATH,"wifi_bridge_enable_br0");
			break;
			
		case WIFI_BRIDGE_DEL_WLAN1:
			snprintf(cmd,128,"sh %s %s",WIFI_BRIDGE_SCRIPTS_PATH,"wifi_bridge_del_wlan1");
			break;
			
		case WIFI_BRIDGE_ADD_WLAN1:
			snprintf(cmd,128,"sh %s %s",WIFI_BRIDGE_SCRIPTS_PATH,"wifi_bridge_add_wlan1");
			break;	
			
		case WIFI_BRIDGE_DEL_BR0:
			snprintf(cmd,128,"sh %s %s",WIFI_BRIDGE_SCRIPTS_PATH,"wifi_bridge_del_br0");
			break;

			
		case WIFI_BRIDGE_ADD_ETH0_AND_WLAN0:
			snprintf(cmd,128,"sh %s %s",WIFI_BRIDGE_SCRIPTS_PATH,"wifi_bridge_add_eth0_and_wlan0");
			break;	
			
		case WIFI_BRIDGE_ADD_ETH0_AND_WLAN1:
			snprintf(cmd,128,"sh %s %s",WIFI_BRIDGE_SCRIPTS_PATH,"wifi_bridge_add_eth0_and_wlan1");
			break;
			
		case WIFI_BRIDGE_DEL_ETH0_AND_WLAN0:
			snprintf(cmd,128,"sh %s %s",WIFI_BRIDGE_SCRIPTS_PATH,"wifi_bridge_del_eth0_and_wlan0");
			break;	
			
		case WIFI_BRIDGE_DEL_ETH0_AND_WLAN1:
			snprintf(cmd,128,"sh %s %s",WIFI_BRIDGE_SCRIPTS_PATH,"wifi_bridge_del_eth0_and_wlan1");
			break;
		case WIFI_BRIDGE_ADD_BR0_AND_WLAN1_WLAN3:
			printf("[%s][%d]WIFI_BRIDGE_ADD_BR1_AND_WLAN1_WLAN3\n",__func__,__LINE__);
			snprintf(cmd,128,"sh %s %s",WIFI_BRIDGE_SCRIPTS_PATH,"wifi_bridge_add_br0_and_wlan1_wlan3");
			break;
		//case WIFI_BRIDGE_DEL_WLAN3:
			//snprintf(cmd,128,"sh %s %s",WIFI_BRIDGE_SCRIPTS_PATH,"wifi_bridge_del_wlan3");
			//break;
		//case WIFI_BRIDGE_ADD_WLAN3:
			//snprintf(cmd,128,"sh %s %s",WIFI_BRIDGE_SCRIPTS_PATH,"wifi_bridge_add_wlan3");
			//break;

		default:
			printf("[%s-%d]error,processChoose invaild",__FILE__,__LINE__);
			break;
			
	
	}
	
	wifi_bridge_dbg("cmd =============== %s\n",cmd);
	
	ret = system(cmd);
	
	wifi_bridge_dbg("ret =============== %d\n",ret);
	//system("pkill  thttpd");
	//system("thttpd -C /etc/thttpd.conf -u root");
	return ret;
}




