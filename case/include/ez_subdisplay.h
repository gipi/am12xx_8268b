#ifndef _EZ_SUBDISPLAY_H_
#define _EZ_SUBDISPLAY_H_

#include <stdint.h>

#define DEBUG 0
#if (DEBUG)
  #define dbg_out(fmt,arg...) printf("[file:%s function: %s line:%d ] " fmt"\n",__FILE__,__func__,__LINE__,##arg);
#else
  #define dbg_out(fmt,arg...)
#endif
#define war_out(fmt,arg...) printf("Warning : [file:%s line:%d ] " fmt"\n",__FILE__,__LINE__,##arg);
#define err_out(fmt,arg...) printf("Error : [file:%s line:%d ] " fmt"\n",__FILE__,__LINE__,##arg);


typedef struct subdisp_env_info_s
{
	int width;
	int height;
	int chipid;
}subdisp_env_info_t;


int ezFuiWifiStart(uint32_t logBase, uint32_t phyBase, void *fui_get_bus_address, void *sysBufInitFn, void *sysBufDestroyFn);
int ezFuiWifiStop(void);

int ezFuiUsbStart(uint32_t logBase, uint32_t phyBase, uint32_t chipId, void *fui_get_bus_address);
int ezFuiUsbStop(void);

#endif //_EZ_SUBDISPLAY_H_

