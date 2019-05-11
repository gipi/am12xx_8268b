/**
 *	@author   simon
 *	@date: 2010/12/16
 *  @breif The SWF key class.
 */

dynamic class Actions.CSwfKey {

	/**
	*@addtogroup CSwfKey_as
	*@{
	*/

	/**
	 *@brief	standard key board key definitions
	 **/
	public static var SWF_MSG_KEY_LBUTTON = 0x1;  
	public static var SWF_MSG_KEY_RBUTTON = 0x2;  
	public static var SWF_MSG_KEY_CANCEL	= 0x3;  
	public static var SWF_MSG_KEY_MBUTTON	 = 0x4;  
	   
	public static var SWF_MSG_KEY_BACK 	 = 0x8;  
	public static var SWF_MSG_KEY_TAB		 = 0x9;  
	   
	public static var SWF_MSG_KEY_CLEAR	 = 0x0C;  
	public static var SWF_MSG_KEY_RETURN	 = 0x0D;   
	public static var SWF_MSG_ENTER = 0x0D;
	public static var SWF_MSG_KEY_SHIFT	 = 0x10;  
	public static var SWF_MSG_KEY_CONTROL	  = 0x11;  
	public static var SWF_MSG_KEY_MENU 	 = 0x12;  
	public static var SWF_MSG_KEY_PAUSE		  = 0x13;  
	public static var SWF_MSG_KEY_CAPITAL	  = 0x14;  
	   
	public static var SWF_MSG_KEY_JUNJA  = 0x17; 
	public static var SWF_MSG_KEY_FINAL	 = 0x18;  
	public static var SWF_MSG_KEY_HANJA	 = 0x19;  
	   
	public static var SWF_MSG_KEY_ESCAPE		  = 0x1B;   
	public static var SWF_MSG_KEY_CONVERT		 = 0x1C;  
	public static var SWF_MSG_KEY_NONCONVERT	 = 0x1D;   
	public static var SWF_MSG_KEY_ACCEPT		  = 0x1E;   
	public static var SWF_MSG_KEY_MODECHANGE	 = 0x1F;   
	   
	public static var SWF_MSG_KEY_SPACE		  = 0x20;  
	public static var SWF_MSG_KEY_PRIOR	 = 0x21;  
	public static var SWF_MSG_KEY_NEXT 	 = 0x22;  
	public static var SWF_MSG_KEY_END		 = 0x23;  
	public static var SWF_MSG_KEY_HOME 	 = 0x24;  
	public static var SWF_MSG_KEY_LEFT 	 = 0x25;  
	public static var SWF_MSG_KEY_UP		  = 0x26;   
	public static var SWF_MSG_KEY_RIGHT	 = 0x27;  
	public static var SWF_MSG_KEY_DOWN 		  = 0x28;  
	public static var SWF_MSG_KEY_SELECT	 = 0x29;   
	public static var SWF_MSG_KEY_PRINT	 = 0x2A;  
	public static var SWF_MSG_KEY_EXECUTE		 = 0x2B;  
	public static var SWF_MSG_KEY_SNAPSHOT 	 = 0x2C;  
	public static var SWF_MSG_KEY_INSERT	 = 0x2D;   
	public static var SWF_MSG_KEY_DELETE	 = 0x2E;   
	public static var SWF_MSG_KEY_HELP 	 = 0x2F;  
	   
	public static var SWF_MSG_KEY_0  = 0x30; 
	public static var SWF_MSG_KEY_1  = 0x31; 
	public static var SWF_MSG_KEY_2  = 0x32; 
	public static var SWF_MSG_KEY_3  = 0x33; 
	public static var SWF_MSG_KEY_4  = 0x34; 
	public static var SWF_MSG_KEY_5  = 0x35; 
	public static var SWF_MSG_KEY_6  = 0x36; 
	public static var SWF_MSG_KEY_7  = 0x37; 
	public static var SWF_MSG_KEY_8  = 0x38; 
	public static var SWF_MSG_KEY_9  = 0x39; 
	   
	public static var SWF_MSG_KEY_A  = 0x41; 
	public static var SWF_MSG_KEY_B  = 0x42; 
	public static var SWF_MSG_KEY_C  = 0x43; 
	public static var SWF_MSG_KEY_D  = 0x44; 
	public static var SWF_MSG_KEY_E  = 0x45; 
	public static var SWF_MSG_KEY_F  = 0x46; 
	public static var SWF_MSG_KEY_G  = 0x47; 
	public static var SWF_MSG_KEY_H  = 0x48; 
	public static var SWF_MSG_KEY_I  = 0x49; 
	public static var SWF_MSG_KEY_J  = 0x4A; 
	public static var SWF_MSG_KEY_K  = 0x4B; 
	public static var SWF_MSG_KEY_L  = 0x4C; 
	public static var SWF_MSG_KEY_M  = 0x4D; 
	public static var SWF_MSG_KEY_N  = 0x4E; 
	public static var SWF_MSG_KEY_O  = 0x4F; 
	public static var SWF_MSG_KEY_P  = 0x50; 
	public static var SWF_MSG_KEY_Q  = 0x51; 
	public static var SWF_MSG_KEY_R  = 0x52; 
	public static var SWF_MSG_KEY_S  = 0x53; 
	public static var SWF_MSG_KEY_T  = 0x54; 
	public static var SWF_MSG_KEY_U  = 0x55; 
	public static var SWF_MSG_KEY_V  = 0x56; 
	public static var SWF_MSG_KEY_W  = 0x57; 
	public static var SWF_MSG_KEY_X  = 0x58; 
	public static var SWF_MSG_KEY_Y  = 0x59; 
	public static var SWF_MSG_KEY_Z  = 0x5A; 
	   
	public static var SWF_MSG_KEY_LWIN = 0x5B;  
	public static var SWF_MSG_KEY_RWIN = 0x5C;  
	public static var SWF_MSG_KEY_APPS = 0x5D;  
	   
	public static var SWF_MSG_KEY_NUMPAD0	= 0x60;  
	public static var SWF_MSG_KEY_NUMPAD1	= 0x61;  
	public static var SWF_MSG_KEY_NUMPAD2	= 0x62;  
	public static var SWF_MSG_KEY_NUMPAD3	= 0x63;  
	public static var SWF_MSG_KEY_NUMPAD4	= 0x64;  
	public static var SWF_MSG_KEY_NUMPAD5	= 0x65;  
	public static var SWF_MSG_KEY_NUMPAD6	= 0x66;  
	public static var SWF_MSG_KEY_NUMPAD7	= 0x67;  
	public static var SWF_MSG_KEY_NUMPAD8	= 0x68;  
	public static var SWF_MSG_KEY_NUMPAD9	= 0x69;  
	public static var SWF_MSG_KEY_MULTIPLY = 0x6A; 
	public static var SWF_MSG_KEY_ADD	= 0x6B;  
	public static var SWF_MSG_KEY_SEPARATOR  = 0x6C; 
	public static var SWF_MSG_KEY_SUBTRACT = 0x6D; 
	public static var SWF_MSG_KEY_DECIMAL	= 0x6E;  
	public static var SWF_MSG_KEY_DIVIDE = 0x6F;   
	   
	public static var SWF_MSG_KEY_F1 = 0x70;   
	public static var SWF_MSG_KEY_F2 = 0x71;   
	public static var SWF_MSG_KEY_F3 = 0x72;   
	public static var SWF_MSG_KEY_F4 = 0x73;   
	public static var SWF_MSG_KEY_F5 = 0x74;   
	public static var SWF_MSG_KEY_F6 = 0x75;   
	public static var SWF_MSG_KEY_F7 = 0x76;   
	public static var SWF_MSG_KEY_F8 = 0x77;   
	public static var SWF_MSG_KEY_F9 = 0x78;   
	public static var SWF_MSG_KEY_F10 = 0x79;  
	public static var SWF_MSG_KEY_F11 = 0x7A;  
	public static var SWF_MSG_KEY_F12	= 0x7B;  
	public static var SWF_MSG_KEY_F13	= 0x7C;  
	public static var SWF_MSG_KEY_F14	= 0x7D;  
	public static var SWF_MSG_KEY_F15	= 0x7E;  
	public static var SWF_MSG_KEY_F16	= 0x7F;  
	public static var SWF_MSG_KEY_F17	= 0x80;  
	public static var SWF_MSG_KEY_F18	= 0x81;  
	public static var SWF_MSG_KEY_F19	= 0x82;  
	public static var SWF_MSG_KEY_F20	= 0x83;  
	public static var SWF_MSG_KEY_F21	= 0x84;  
	public static var SWF_MSG_KEY_F22	= 0x85;  
	public static var SWF_MSG_KEY_F23	= 0x86;  
	public static var SWF_MSG_KEY_F24	= 0x87;  
	   
	public static var SWF_MSG_KEY_NUMLOCK	= 0x90;  
	public static var SWF_MSG_KEY_SCROLL = 0x91;   
	   
	public static var SWF_MSG_KEY_LSHIFT = 0xA0;   
	public static var SWF_MSG_KEY_RSHIFT = 0xA1;   
	public static var SWF_MSG_KEY_LCONTROL = 0xA2; 
	public static var SWF_MSG_KEY_RCONTROL = 0xA3; 
	public static var SWF_MSG_KEY_LMENU	 = 0xA4;  
	public static var SWF_MSG_KEY_RMENU	 = 0xA5;  

	/**
	*@brief  ez cast command key  ----  Bink
	**/
	public static var SWF_MSG_KEY_EZCAST_SW = 0xB0;			// switch to EZ Cast
	public static var SWF_MSG_KEY_EZCAST_ON = 0xB1;			// turn on EZ Cast
	public static var SWF_MSG_KEY_EZCAST_OFF = 0xB2;		// turn off EZ Cast
	public static var SWF_MSG_KEY_DLNA_SW = 0xB3;			// switch to DLNA
	public static var SWF_MSG_KEY_DLNA_ON = 0xB4;			// turn on DLNA
	public static var SWF_MSG_KEY_DLNA_OFF = 0xB5;			// turn off DLNA
	public static var SWF_MSG_KEY_MIRACAST_SW = 0xB6;		// switch to Miracast
	public static var SWF_MSG_KEY_MIRACAST_ON = 0xB7;		// turn on Miracast
	public static var SWF_MSG_KEY_MIRACAST_OFF = 0xB8;		// turn off Miracast
	public static var SWF_MSG_KEY_EZAIR_SW = 0xB9;			// switch to EZ Air
	public static var SWF_MSG_KEY_EZAIR_ON = 0xBA;			// turn on EZ Air
	public static var SWF_MSG_KEY_EZAIR_OFF = 0xBB;			// turn off EZ Air
	public static var SWF_MSG_KEY_QUERY_STATUS = 0xBC;		// query on/off status of ez cast / dlna / miracast / ez air
	public static var SWF_MSG_KEY_MAIN_SW = 0xBD;			// restore to the main page when no any app is using the device.
	public static var SWF_MSG_KEY_SETTING_SW = 0xBE;		// switch to settings page
	public static var SWF_MSG_KEY_VIEW = 0xBF;				// show help view
	public static var SWF_MSG_KEY_APPINFO = 0xC0;			// set language or other info from app
	public static var SWF_MSG_KEY_LOAD_LANGUAGE = 0xC4;		// load language for websetting
	public static var SWF_MSG_KEY_PAYUPGRADE = 0xC5			// upgrade to 8251/8252 firmware when the 8252to8251key is efficient/lose efficient
	public static var SWF_MSG_KEY_STREAMSTART = 0xC6;		// net display stream is start
	public static var SWF_MSG_KEY_STREAMSTOP = 0xC7;		// net display stream is stop
	public static var SWF_MSG_KEY_SETPSK = 0xC8;	// load PSK for websetting
	public static var SWF_MSG_KEY_OTA_DOWNLOAD = 0xC9;	// OTA download for websetting
	public static var SWF_MSG_KEY_WIFI_CHANGE = 0xCA;	// Command for websetting, to change wifi mode to client & softap.
	public static var SWF_MSG_KEY_MODE_CHANGE = 0xCB;	// Command for websetting, when default mode is changed by websetting.
	public static var SWF_MSG_KEY_OTA_FROM_URL = 0xCD;	// OTA download from url.
	public static var SWF_MSG_KEY_PLUG_PLAY = 0xCE;	// To start Plug&play mirroring.
	public static var SWF_MSG_KEY_PLUG_STOP = 0xCF;	// To stop Plug&play mirroring.
	public static var SWF_MSG_KEY_WIFI_FAILED = 0xC2;//client connected failed
	/**
	 *@brief	media event key. e.g. card in/out etc.
	 **/
	public static var SWF_MSG_KEY_MEDIA_CHANGE  = 0xD0;
	
	/**
	 *@brief	usb plug in.
	 **/
	public static var SWF_MSG_KEY_USB_IN  = 0xD1;
	
	/**
	 *@brief	usb plug out.
	 **/
	public static var SWF_MSG_KEY_USB_OUT  = 0xD2;
	
	/**
	 *@brief	alarm ring event.
	 **/ 
	public static var SWF_MSG_KEY_RING = 0xD3;
	
	/*it is for mouse use*/
	public static var SWF_MSG_KEY_USB_HID_IN = 0xD4;
	public static var SWF_MSG_KEY_USB_HID_OUT = 0xD5;
	 
	/*ezlauncher v2*/
	public static var SWF_MSG_KEY_EZLAUNCHER_IN = 0xD8;
	public static var SWF_MSG_KEY_EZLAUNCHER_OUT = 0xD9;
	
	/**
	 *@brief	remote controller key.
	 **/  
	public static var SWF_MSG_KEY_EXT_PHOTO = 0xE0; 
	public static var SWF_MSG_KEY_EXT_MUSIC = 0xE1; 
	public static var SWF_MSG_KEY_EXT_VIDEO	  = 0xE2; 
	public static var SWF_MSG_KEY_EXT_CALENDAR	  = 0xE3; 
	public static var SWF_MSG_KEY_EXT_SETUP	  = 0xE4;
	public static var SWF_MSG_KEY_EXT_FILE	  = 0xE5; 
	public static var SWF_MSG_KEY_EXT_TEXT	  = 0xE6; 
	public static var SWF_MSG_KEY_EXT_VVD	 = 0xE7; 
	public static var SWF_MSG_KEY_EXT_IRMENU	  = 0xE8; 
	public static var SWF_MSG_KEY_EXT_PLAYER	  = 0xE9; 
	public static var SWF_MSG_KEY_EXT_VIEW	  = 0xEA; 
	public static var SWF_MSG_KEY_EXT_PREV	  = 0xEB; 
	public static var SWF_MSG_KEY_EXT_NEXT	  = 0xEC; 
	public static var SWF_MSG_KEY_EXT_COPY	  = 0xED; 
	public static var SWF_MSG_KEY_EXT_PLAY	  = 0xEE; 
	public static var SWF_MSG_KEY_EXT_COMBO	  = 0xEF; 
	public static var SWF_MSG_KEY_EXT_ZOOM	  = 0xF0; 
	public static var SWF_MSG_KEY_EXT_ROTATE	  = 0xF1;
	public static var SWF_MSG_KEY_EXT_MUTE	  = 0xF2;
	public static var SWF_MSG_KEY_EXT_VOL_PLUS	  = 0xF3;
	public static var SWF_MSG_KEY_EXT_VOL_MINUS	  = 0xF4; 
	public static var SWF_MSG_KEY_EXT_POWER	  = 0xF5; 

	/**
	*@brief user key
	**/
	public static var SWF_MSG_KEY_CARDUPGRADE	= 0xF6; 
	public static var SWF_MSG_KEY_NOT_SUPPORT = 0xF7;
	public static var SWF_MSG_KEY_RIGHT_PASSWORD_CONNECT = 0xF8;
	

/**
 *@}
 */
}
