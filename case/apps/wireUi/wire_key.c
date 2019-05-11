#include <stdio.h>
#include "wire_log.h"
#include "wire_key.h"
#include "wire_config.h"
#include "sys_msg.h"

#ifndef __ACT_MODULE_CONFIG_H__ //For LinuxTool bug!! (i.e.; #if !define xxx)
#define __ACT_MODULE_CONFIG_H__
#include "../../../scripts/mconfig.h"
#endif

bool is_wire_key_event(int fd, fd_set *fds)
{
	if(fd < 0)
		return false;
	if(FD_ISSET(fd, fds)){
		WLOGI("select keyevent!\n");
		return true;
	}
	return false;
}

int wire_key_handler(int fd)
{
	WLOGI();
	int rdnr;
	int already_read_nr = 0;
	static int key_count=0;
	struct am_sys_msg  msg;
	
	do{
		rdnr = read(fd,&msg,sizeof(struct am_sys_msg));
		already_read_nr++;
		if(rdnr == sizeof(struct am_sys_msg)){
			WLOGI("Msgtype=0x%x,subtype=0x%x,dataload=0x%x\n",msg.type,msg.subtype,msg.dataload);
			switch(msg.type){
				case SYSMSG_KEY:
					{
						if(msg.dataload == WIRE_SWITCHKEY_UP){
							WLOGI("SWITCHKEY UP! key_count=%d\n",key_count);
							if(key_count>=130)
							{
								 key_count=0;
								 break;
							}
							else
							{
								key_count=0;	
							}
							if( getWireMirrorStatus() == WIRE_IPHONE_PLUG_START || getWireSetupStatus() == WIRE_IPHONE_SETUP_START  || getAndroidAdbStatus()==WIRE_ANDROID_PLUG_START || getAndroidSetupStatus() == WIRE_ANDROID_SETUP_START){
								WLOGI("mirroring || Setup can't switch!\n");
								break;
							}
							bool flag = getWirePlugFlag();
							if(flag){
								setWirePlugFlag(false);
								setWirePlugMode(0);
								
								#if MODULE_CONFIG_ADB_MIRROR_ONLY==1	
								setAoaMode(1);
								#endif
								#if EZWIRE_TYPE==MIRAPLUG//iOS & Android with 8M snor
								setAoaMode(1);
								ezwireDrawWindow(WIRE_UI_PLUG_OFF, WIRE_UI_PLUGMODE_X, WIRE_UI_PLUGMODE_Y, WIRE_UI_PLUGMODE_WIDTH, WIRE_UI_PLUGMODE_HEIGHT);
								ezwireDrawWindow(WIRE_UI_SETUP_ON, WIRE_UI_SETUPMODE_X, WIRE_UI_SETUPMODE_Y, WIRE_UI_SETUPMODE_WIDTH, WIRE_UI_SETUPMODE_HEIGHT);
								#else
								#if !defined(MODULE_CONFIG_AD_UI_ENABLE) || MODULE_CONFIG_AD_UI_ENABLE==0
								ezwireDrawWindow(WIRE_UI_SETUP, WIRE_UI_PLUG_X, WIRE_UI_PLUG_Y, WIRE_UI_PLUG_WIDTH, WIRE_UI_PLUG_HEIGHT);
								#endif
								#endif

								#if defined(MODULE_CONFIG_AD_UI_ENABLE) && MODULE_CONFIG_AD_UI_ENABLE!=0
								showADQRcode();
								ezwireDrawWindow(WIRE_UI_TETHERING, WIRE_UI_TETHERING_X, WIRE_UI_TETHERING_Y, WIRE_UI_TETHERING_WIDTH, WIRE_UI_TETHERING_HEIGHT);
								#else
								ezwireDrawWindow(WIRE_UI_ARROW_DISCONNECT, WIRE_UI_ARROW_X, WIRE_UI_ARROW_Y, WIRE_UI_ARROW_WIDTH, WIRE_UI_ARROW_HEIGHT);
								#endif
								ezwireDrawFlip();

							}else{
								setWirePlugFlag(true);
								setWirePlugMode(1);
								#if MODULE_CONFIG_ADB_MIRROR_ONLY==1	
								setAoaMode(0);
								#endif
								#if EZWIRE_TYPE==MIRAPLUG//iOS & Android with 8M snor
								setAoaMode(0);
								ezwireDrawWindow(WIRE_UI_PLUG_ON, WIRE_UI_PLUGMODE_X, WIRE_UI_PLUGMODE_Y, WIRE_UI_PLUGMODE_WIDTH, WIRE_UI_PLUGMODE_HEIGHT);
								ezwireDrawWindow(WIRE_UI_SETUP_OFF, WIRE_UI_SETUPMODE_X, WIRE_UI_SETUPMODE_Y, WIRE_UI_SETUPMODE_WIDTH, WIRE_UI_SETUPMODE_HEIGHT);
								#else
								#if !defined(MODULE_CONFIG_AD_UI_ENABLE) || MODULE_CONFIG_AD_UI_ENABLE==0
								ezwireDrawWindow(WIRE_UI_PLUG, WIRE_UI_PLUG_X, WIRE_UI_PLUG_Y, WIRE_UI_PLUG_WIDTH, WIRE_UI_PLUG_HEIGHT);
								#endif
								#endif
								
								#if defined(MODULE_CONFIG_AD_UI_ENABLE) && MODULE_CONFIG_AD_UI_ENABLE!=0
								showADQRcode();
								ezwireDrawWindow(WIRE_UI_PNP, WIRE_UI_TETHERING_X, WIRE_UI_TETHERING_Y, WIRE_UI_TETHERING_WIDTH, WIRE_UI_TETHERING_HEIGHT);
								#else
								ezwireDrawWindow(WIRE_UI_ARROW_PLUG, WIRE_UI_ARROW_X, WIRE_UI_ARROW_Y, WIRE_UI_ARROW_WIDTH, WIRE_UI_ARROW_HEIGHT);
								#endif
								ezwireDrawFlip();
							}
						}
						else if(msg.dataload == WIRE_SWITCHKEY_HOLD){
							key_count++;
							if(key_count>=130)
							{
								system("echo 1 > /tmp/reset.txt");
								system("rmmod  am7x_keys");
								system("sync");
								//sleep(1);
								//system("reboot");
								break;
								
							}
							//WLOGI("SWITCHKEY HOLD!key_count=%d \n",key_count);
						}
					}
					break;
				default:								
					break;
			}
		}
		else{
			break;
		}
	}while(already_read_nr < 10);
	return 0;
}

