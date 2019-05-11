#ifndef __WIRE_CONFIG_H
#define __WIRE_CONFIG_H
#include <stdbool.h>

#define WIRE_IPHONE_PLUG_START	0x200
#define WIRE_IPHONE_PLUG_STOP	0x201
#define WIRE_IPHONE_SETUP_START	0x202
#define WIRE_IPHONE_SETUP_STOP	0x203

#define WIRE_ANDROID_PLUG_START	0x204
#define WIRE_ANDROID_PLUG_STOP	0x205
#define WIRE_ANDROID_SETUP_START	0x206
#define WIRE_ANDROID_SETUP_STOP	0x207

/********para must base on 1080P(pic para will be converted when it loads)***/

#define WIRE_UI_BG					"/am7x/case/data/bg.jpg"
#define WIRE_UI_BG_WIDTH 			(1920)
#define WIRE_UI_BG_HEIGHT 			(1080)
#define WIRE_UI_BG_X 				(0)
#define WIRE_UI_BG_Y 				(0)

#define WIRE_UI_DONGLE				"/am7x/case/data/miralink_03.jpg"
#define WIRE_UI_DONGLE_WIDTH 		(39)
#define WIRE_UI_DONGLE_HEIGHT 		(87)
#define WIRE_UI_DONGLE_X 			(1614)
#define WIRE_UI_DONGLE_Y 			(48)

#define WIRE_UI_ARROW_DISCONNECT	"/am7x/case/data/miralink_11.jpg"
#define WIRE_UI_ARROW_CONNECT		"/am7x/case/data/miralink_07.jpg"
#define WIRE_UI_ARROW_CONNECTING	"/am7x/case/data/miralink_13.jpg"
#define WIRE_UI_ARROW_PLUG			"/am7x/case/data/miralink_09.jpg"
#define WIRE_UI_ARROW_WIDTH 		(116)
#define WIRE_UI_ARROW_HEIGHT 		(87)
#define WIRE_UI_ARROW_X 			(1653)
#define WIRE_UI_ARROW_Y 			(48)

#define WIRE_UI_PHONE				"/am7x/case/data/miralink_05.jpg"
#define WIRE_UI_PHONE_WIDTH 		(39)
#define WIRE_UI_PHONE_HEIGHT 		(87)
#define WIRE_UI_PHONE_X 			(1771)
#define WIRE_UI_PHONE_Y 			(48)

#define WIRE_UI_SETUP				"/am7x/case/data/miralink_18.jpg"
#define WIRE_UI_PLUG				"/am7x/case/data/miralink_15.jpg"
#define WIRE_UI_PLUG_WIDTH 			(856)
#define WIRE_UI_PLUG_HEIGHT 		(147)
#define WIRE_UI_PLUG_X 				(534)
#define WIRE_UI_PLUG_Y 				(645)

#define WIRE_UI_MIRAPLUG_BG		"/am7x/case/data/miraplug_bg.jpg"
#define WIRE_UI_PLUG_ON			"/am7x/case/data/plugOn.jpg"
#define WIRE_UI_SETUP_ON			"/am7x/case/data/airplayOn.jpg"
#define WIRE_UI_PLUG_OFF		       "/am7x/case/data/plugOff.jpg"
#define WIRE_UI_SETUP_OFF			"/am7x/case/data/airplayOff.jpg"


#define WIRE_UI_PLUGMODE_WIDTH 			(200)
#define WIRE_UI_PLUGMODE_HEIGHT 		       (46)
#define WIRE_UI_PLUGMODE_X 				(398)
#define WIRE_UI_PLUGMODE_Y 			       (470)

#define WIRE_UI_SETUPMODE_WIDTH 			(200)
#define WIRE_UI_SETUPMODE_HEIGHT 		       (46)
#define WIRE_UI_SETUPMODE_X 				(398)
#define WIRE_UI_SETUPMODE_Y 			       (795)

#define WIRE_UI_VERSION				"/am7x/case/data/"
#define WIRE_UI_VERSION_WIDTH 		(20)
#define WIRE_UI_VERSION_HEIGHT 		(30)

#define MIRALINE                                   (8)
#define MIRAPLUG                                  (10)
#define WIRE_UI_WARN_ADB				"/am7x/case/data/warn_adb.jpg"
#define WIRE_UI_WARN_AOA				"/am7x/case/data/warn_aoa.jpg"
#define WIRE_UI_WARN_EMPTY				"/am7x/case/data/warn_empty.jpg"
#define WIRE_UI_WARN_WIDTH 		(350)
#define WIRE_UI_WARN_HEIGHT 		(100)
#define WIRE_UI_WARN_X 			(810)
#define WIRE_UI_WARN_Y 			(810)

#if (defined(MODULE_CONFIG_AD_UI_ENABLE) && MODULE_CONFIG_AD_UI_ENABLE == 1)
	#if (defined(MODULE_CONFIG_EZWIRE_TYPE) && MODULE_CONFIG_EZWIRE_TYPE == 10)
		#define WIRE_UI_VERSION_X			(1820)
	#else
		#define WIRE_UI_VERSION_X			(200)
	#endif
	#define WIRE_UI_VERSION_Y 			(1000)
	
	#define WIRE_UI_REDDOT_X 	              (1188)
	#define WIRE_UI_REDDOT_Y	              (333)
#else
	#if EZWIRE_TYPE==MIRALINE 
		#if (defined(MODULE_CONFIG_8258D_ENABLE) && MODULE_CONFIG_8258D_ENABLE == 1)  //8258d
			#define WIRE_UI_VERSION_X			(200)
			#define WIRE_UI_VERSION_Y			(900)
		#else
			#define WIRE_UI_VERSION_X			(1820)
			#define WIRE_UI_VERSION_Y			(900)
		#endif
		#define WIRE_UI_REDDOT_X 	              (1330)
		#define WIRE_UI_REDDOT_Y	              (345)
		
	#else
		#define WIRE_UI_VERSION_X 			(1170)
		#define WIRE_UI_VERSION_Y 			(1005)
		#define WIRE_UI_REDDOT_X 	              (1360)
		#define WIRE_UI_REDDOT_Y	              (605)
	#endif
#endif


#define WIRE_UI_ADQR			"/tmp/AD_QR.jpg"
#define WIRE_UI_ADQR_WIDTH 		(324)
#define WIRE_UI_ADQR_HEIGHT		(324)
#define WIRE_UI_ADQR_X 			(876)
#define WIRE_UI_ADQR_Y 			(345)

#define WIRE_UI_TETHERING				"/am7x/case/data/tethering_mode.jpg"
#define WIRE_UI_PNP						"/am7x/case/data/pnp_mode.jpg"
#define WIRE_UI_TETHERING_WIDTH 		(54)
#define WIRE_UI_TETHERING_HEIGHT		(54)
#define WIRE_UI_TETHERING_X 			(666)
#define WIRE_UI_TETHERING_Y 			(333)

/***************************************************************/


int isAM8251W();
int isQCMode();
void QCKeyCheck();
void ezwireInit();
void productInit();
int ezwireUiInit();
void ezwireUiRelease();
int isWirePlugMode();
int ezwireMainUiStart();
int ezwireDrawDefaultBg();
int ezwireDrawWindow(char *jpgPath, int pos_x, int pos_y, int pic_width, int pic_height);
int ezwireDrawFlip();
int ezwireMainUiHide();
void ezwireUiDeRelease();
void ezwireMainUiStop();
bool getWirePlugFlag();
void setWirePlugFlag(bool Flag);
int getWireMirrorStatus();
void setWireMirrorStatus(int stat);
int getWireSetupStatus();
void setWireSetupStatus(int stat);
int setWirePlugMode(int on_off);
int getAndroidSetupStatus();
void setAndroidSetupStatus(int stat);
int getAndroidAdbStatus();
void setAndroidAdbStatus(int stat);
#endif
