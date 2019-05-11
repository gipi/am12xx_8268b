#ifndef __EZCAST_H_SDK__		// This file created by scripts/.sh
#define __EZCAST_H_SDK__

#ifdef MODULE_CONFIG_EZWIRE_TYPE
#undef MODULE_CONFIG_EZWIRE_TYPE
#endif

#ifdef MODULE_CONFIG_FLASH_TYPE
#undef MODULE_CONFIG_FLASH_TYPE
#endif
#ifdef MODULE_CONFIG_CHIP_TYPE
#undef MODULE_CONFIG_CHIP_TYPE
#endif
#ifdef MODULE_CONFIG_QR_URL
#undef MODULE_CONFIG_QR_URL
#endif
#ifdef MODULE_CONFIG_QR_APP_VENDOR
#undef MODULE_CONFIG_QR_APP_VENDOR
#endif

#define EZWILAN_ENABLE 0
#define		MODULE_CONFIG_EZWIRE_TYPE		0	//EZWIRE type, 0: EZWire with 8251/8251W; 1:MiraWire with 8252B; 2:MiraWireDuo with 8252C; 3: MiraWirePlus with 8252B/8252W; 4: EZDocking with 8251W; 5:SNOR flash with 8252N; 6: MiraWire with CDC/EDM; 7: MiraWire with CDC/EDM at snor;8:SNOR 8M flash with 8252N;9:SNOR 8M flash with 8256;10:MiraPlug with 8252N
#define		MODULE_CONFIG_FLASH_TYPE		1	//Flash type, 0: Nand flash, 1: 16M snor flash, 2: 8M snor flash
#define		MODULE_CONFIG_CHIP_TYPE		32	//Chip type, 0: AM8250; 10: AM8251/AM8252; 20: AM8258; 30: AM8268
#define		MODULE_CONFIG_QR_URL		"https://www.ezcast.com/upgrade/download.php"	//The URL to set QRCode
#define		MODULE_CONFIG_QR_APP_VENDOR		"ezmira"	//app_vendor of QRCode

#endif
