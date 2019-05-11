#ifndef __WIFI_COB_H
#define __WIFI_COB_H


#define Debug 1
#if Debug
#define ALOGV(fmt, args...)   printf("V:<%s,%d> "fmt"\n",__FUNCTION__,__LINE__, ##args)
#define ALOGD(fmt, args...)   printf("D:<%s,%d> "fmt"\n", __FUNCTION__,__LINE__,##args)
#define ALOGI(fmt, args...)   printf("I:<%s,%d> "fmt"\n", __FUNCTION__,__LINE__,##args)
#define ALOGW(fmt, args...)   printf("W:<%s,%d> "fmt"\n", __FUNCTION__,__LINE__,##args)
#define ALOGE(fmt, args...)   printf("E:<%s,%d> "fmt"\n", __FUNCTION__,__LINE__,##args)
#else
#define ALOGV(fmt, args...)   
#define ALOGD(fmt, args...)  
#define ALOGI(fmt, args...)   
#define ALOGW(fmt, args...)  
#define ALOGE(fmt, args...)  
#endif

#define WIFI_COB_MP_MAP_FILE				"/etc/AM_8188FTV.map"
#define WIFI_COB_MP_MAC_STATISTIC_FILE		"/etc/wifi_mac_addr_mp.conf"
#define WIFI_COB_EFUSE_MAP_FILE				"/tmp/efuse.map"
//#define WIFI_COB_EFUSE_GET_TMP_FILE			"/tmp/efuse_get.map"
//#define WIFI_COB_EFUSE_GET_COMPARE_FILE		"/tmp/efuse_compare.map"

#define MAC_START_LINE						"Start Addr:"
#define MAC_TOTAL							"Total:"
#define MAC_LEFT							"Left:"

enum{
	WIFI_DONGLE_8188FU = 0,
	WIFI_DONGLE_8811AU,
};

#ifndef EXPORT_SYMBOL
#define EXPORT_SYMBOL   __attribute__ ((visibility("default")))
#endif

int wifi_cob_start();
int getDriverName();
int get_mac_total_left(int *total, int *left);
void get_left_mac_for_ui(int *num);


#endif
