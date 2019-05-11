#ifndef _EXT_SCAN_H_
#define _EXT_SCAN_H_

#ifdef __cplusplus
extern "C"{
#endif

#define SCANNING_MAC_LEN 17	//AA:BB:CC:DD:EE:FF

typedef struct scanning_info{
	char sc_addr[SCANNING_MAC_LEN+1];
	char sc_essid[IW_ESSID_MAX_SIZE+1];
	int channel;
}sc_info;

int extension_scanning(char *ifnane,char *essid, sc_info *ap_info);

#ifdef __cplusplus
}
#endif

#endif