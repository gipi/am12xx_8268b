
trace("*********** Load background **********");

MainSWF.LOGO_DISPLAY 		= true;
MainSWF.PSK_DISPLAY 		= true;
MainSWF.SSID_DISPLAY 		= true;	// if SSID_DISPLAY is false, then the value of PSK_DISPLAY will become invalid
MainSWF.ICON_DISPLAY 		= true;
MainSWF.TOPBAR_BG_DISPLAY 	= true;
MainSWF.NET_CONNECT_CHECK 	= true;

MainSWF.USER_MANUAL_DISPLAY	= true;

// CastCode & MiraCode settings is invalid while USER_MANUAL_DISPLAY is true
MainSWF.CASTCODE_X			= 1441;
MainSWF.CASTCODE_Y			= 205;
MainSWF.CASTCODE_W			= 347;
MainSWF.CASTCODE_H			= 58;
MainSWF.CASTCODE_SIZE		= 45;
MainSWF.CASTCODE_COLOR		= 0x00CCFF;

MainSWF.MIRACODE_X			= 1441;
MainSWF.MIRACODE_Y			= 269;
MainSWF.MIRACODE_W			= 347;
MainSWF.MIRACODE_H			= 58;
MainSWF.MIRACODE_SIZE		= 45;
MainSWF.MIRACODE_COLOR		= 0x0066FF;

MainSWF.bgLoadCallback();

trace("*********** Load background end**********");