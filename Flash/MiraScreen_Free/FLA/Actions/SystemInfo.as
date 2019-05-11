
/**
 *	@author   wuxiaowen
 *	@version 1.0
 *  @date: 2009/06/08
 *	<pre>  
 *  SystemInfo Class 
 		system related control
 *  </pre>
 *	<pre>
 	Example:
 *  	sd_in = SystemInfo.checkCardStatus();
 *		cf_in = SystemInfo.checkCFStatus();
 *		udisk_in = SystemInfo.checkUsbStatus();	
 *	</pre>
 */

dynamic class Actions.SystemInfo {
	 
/**
*@addtogroup SystemInfo_as
*@{
*/

	public static var SYS_MEDIA_INTERNAL = 0;	///< store media:internal memory
	public static var SYS_MEDIA_SD = 1;			///< store media:SD/MMC/MS/XD card
	public static var SYS_MEDIA_CF = 2;			///< store media:CF card
	public static var SYS_MEDIA_UDISK = 3;		///< store media:Usb mass storage media
	public static var SYS_MEDIA_UDISK1 = 4;		///< store media:Usb mass storage media1
	public static var SYS_MEDIA_UDISK2 = 5;		///< store media:Usb mass storage media2
	
	public static var CARDSTATUS_INDEX_FOR_WIFI = 6;		///cardstatus index for set_wifi.swf
	public static var CARDSTATUS_INDEX_FOR_EZ_LAN = 7;		///cardstatus index for lanDisplay.swf
	public static var CARDSTATUS_INDEX_FOR_EZ_WIFI = 8;		///cardstatus index for wifiDisplay.swf
	public static var CARDSTATUS_INDEX_FOR_LAN = 9;		///cardstatus index for set_lan.swf
	public static var CARDSTATUS_INDEX_FOR_DLNA = 10;		///cardstatus index for dlna.swf
	public static var CARDSTATUS_INDEX_FOR_MIRCAST = 11;		///cardstatus index for mircast.swf




	
	public static var SYS_SET_MUTE = 100;		///< CMD  to SET MUTE/Cancel mute
	public static var MT_DONGLE =10;			///dongle media type, used in acceptHotplugInfo() function
	
	///< Func  to SET color
	public static var COLOR_Hue = 1;			///< Hue func
	public static var COLOR_Saturation = 2;		///< Saturation func
	public static var COLOR_Brightness = 3;		///< Brightness func
	public static var COLOR_Contrast = 4;		///< Contrast func
	public static var COLOR_Sharpness = 5;		///< Sharpness func

	///< Mode Read or Write  to SET color
	public static var Mode_Read = 0;				///< read mode
	public static var Mode_Write = 1;			///< write mode


	///< Card Upgrade State
	public static var CARD_STATE_INIT = 0; 		///< initing
	public static var CARD_STATE_WAIT = 1; 	///< waiting
	public static var CARD_STATE_RUNUING = 2; 	///< running
	public static var CARD_STATE_FINISHED = 3; 		///< finished
	public static var CARD_STATE_ERROR = 4; 		///< error

	///< backlight
	public static var BACKLIGHT_DEF_VALUE = 31; 	///< backlight max value


	///< ddr pll for change_pll()
	public static var DDR_LOW_PLL= 196; ///< clock rate 196M
	public static var DDR_HIGH_PLL=392; ///< clock rate 392M


	///< get global value cmd
	public static var CMD_GET_GLONUM=0; ///< get num
	public static var CMD_GET_GLOSTR=1;///< get string


	///< card type
	public static var CARD_TYPE_SD =1;///< sd card
	public static var CARD_TYPE_MMC =2;///< mmc card
	public static var CARD_TYPE_MS =3;///< ms card
	public static var CARD_TYPE_MS_PRO =4;///< ms pro card
	public static var CARD_TYPE_XD =5;///< xd card
	
	///< access mode, these may be OR'd together
	public static var MODE_R_OK=4 ; ///< test for read permission
	public static var MODE_W_OK=2; ///< test for write permission
	public static var MODE_X_OK =1; ///< test for execute permission
	public static var MODE_F_OK = 0; ///< test for existence

    /*add by chenshouhui for set output_mode*/

	/*hdmi mode*/
	public static var FMT_2200x1125_1920x1080_60I=0;
	public static var FMT_2640x1125_1920x1080_50I=1;
	public static var FMT_2200x1125_1920x1080_30P=2;
	public static var FMT_2640x1125_1920x1080_25P=3;
	public static var FMT_2750x1125_1920x1080_24P=4;
		/// SMPTE 296M
	public static var FMT_1650x750_1280x720_60P=5;
	public static var FMT_3300x750_1280x720_30P=6;
	public static var FMT_1980x750_1280x720_50P=7;
	public static var FMT_3960x750_1280x720_25P=8;
	public static var FMT_4125x750_1280x720_24P=9;
	
	public static var FMT_2304x1250_1920x1152_50I=10;
	
		/// ITU-R-656
	public static var FMT_858x525_720x480_60I=11;
	public static var FMT_864x625_720x576_50I=12;
	
		/// SMPTE 293M
	public static var FMT_858x525_720x480_60P=13;
	public static var FMT_864x625_720x576_50P=14;
	
	public static var FMT_1250x810_1024x768_60P=15;
	public static var FMT_1280x800_60P=16;
	
	public static var FMT_1920x1080_60P=17;
	public static var DEFAULT_OUTPUT_RESOLUTION=64;

	/*vga mode*/
	public static var VGA_1280_1024=0;
	public static var VGA_1280_768=1;
	public static var VGA_1024_768=2;
	public static var VGA_800_600=3;
	public static var VGA_640_480=4;
	public static var VGA_1280_800=5;
	public static var VGA_1920_1080=6;
	public static var VGA_1366_768=7;//richardyu 052714
	public static var VGA_1280_720=8;
	public static var VGA_1920_1080_30P=9;
	/*cvbs mode*/
	
	public static var CVBS_PAL=0;
	public static var CVBS_NTSC=1;

	
	public static var SUBTITLE_FONT_SIZE=32;      //the size of subtitle font,setting by user

	/*add by bink for get chip id*/
	public static var CHIP_ID_AM8250	=0;
	public static var CHIP_ID_AM8251	=1;
	public static var CHIP_ID_AM8252	=2;
	public static var CHIP_ID_ERROR		=3;
	
     /*add by chenshouhui for set output_mode*/

	public static var DISPLAY_EADA_ALARM_INFO_INIT_TIMES=20; //user for eada show alarm info seconds
    /**
	 * @brief	set system volume
	 * @param[in] newVolume	: range is 0~10,SYS_SET_MUTE 
	  	as 0~10 id normal volume, the SYS_SET_MUTE will mute the volume
	 * @return 
	 * 	- -1	: failed
	 *	- 0	: succeed
	 */
	public static function setVolume(newVolume:Number):Number
	{
		return ExternalInterface.call("sys_setVolume", newVolume);
	}
    
    /**
	 * @brief	get system volume
	 * @param[in] NULL
	 * @return 
	 *		- volume (0~10)
	 *		- SYS_SET_MUTE the volume is mute
	 */
	public static function getVolume():Number
	{
		return ExternalInterface.call("sys_getVolume");
	}
    
	/**
	 * @brief	set system volume mute, but doesn't save to the vram
	 * @param[in] NULL 
	 * @return 
	 * 	- -1	: failed
	 *	- 0	: succeed
	*/
	public static function setVolumeMute():Number
	{
		return ExternalInterface.call("sys_setVolumeMute");
	}
	
    /**
	 * @brief	get firmware version
	 * @param[in] NULL
	 * @return 
	 *		version string
	*/
	public static function getFwVersion(search_begin:String):String
	{
		return ExternalInterface.call("sys_getFwVersion",search_begin);
	} 
	
	  /**
	 * @brief	get firmware version
	 * @param[in] NULL
	 * @return 
	 *		version string
	 */
	public static function getVersion(search_begin:String):String
	{
		return ExternalInterface.call("sys_getVersion",search_begin);
	}
	
	/**
	 * @brief	get internal storage total capacity(kbytes)
	 * @param[in] NULL
	 * @return 
	 *		total capacity if success or <0 if fail
	 */
	public static function getDiskSpace():Number
	{
		return ExternalInterface.call("sys_getDiskSpace");
	}
	
	/**
	 * @brief	get internal storage free capacity(kbytes)
	 * @param[in] NULL
	 * @return 
	 *		free capacity if success or <0 if fail
	 */
	public static function getDiskSpaceLeft():Number
	{
		return ExternalInterface.call("sys_getDiskSpaceLeft");
	}
	
	/**
	 * @brief	get card storage total capacity(kbytes)
	 * @param[in] NULL
	 * @return 
	 *		total capacity if success or <0 if fail
	 */
	public static function getCardSpace():Number
	{
		return ExternalInterface.call("sys_getCardSpace");
	}
	
	/**
	 * @brief	get card storage free capacity(kbytes)
	 * @param[in] NULL
	 * @return 
	 *		free capacity if success or <0 if fail
	 */
	public static function getCardSpaceLeft():Number
	{
		return ExternalInterface.call("sys_getCardSpaceLeft");
	}
	
	/**
	 * @brief	get CF card storage total capacity(kbytes)
	 * @param[in] NULL
	 * @return 
	 *		total capacity if success or <0 if fail
	 */
	public static function getCFSpace():Number
	{
		return ExternalInterface.call("sys_getCFSpace");
	}
	
	/**
	 * @brief	get CF card storage free capacity(kbytes)
	 * @param[in] NULL
	 * @return 
	 *		free capacity if success or <0 if fail
	 */
	public static function getCFSpaceLeft():Number
	{
		return ExternalInterface.call("sys_getCFSpaceLeft");
	}
	
	/**
	 * @brief	get usb mass storage media total capacity(kbytes)
	 * @param[in] NULL
	 * @return 
	 *		total capacity if success or <0 if fail
	 */
	public static function getUSBSpace():Number
	{
		return ExternalInterface.call("sys_getUSBSpace");
	}
	
	/**
	 * @brief	get usb mass storage media free capacity(kbytes)
	 * @param[in] NULL
	 * @return 
	 *		free capacity if success or <0  if fail
	 */
	public static function getUSBSpaceLeft():Number
	{
		return ExternalInterface.call("sys_getUSBSpaceLeft");
	}	
	
	/**
	 * @brief	set backlight strength
	 * @param[in] strength	: strength to be set
	 * @return 
	 *		true if success or false if fail
	 */
	public static function setBackLightStrength(strength:Number):Boolean
	{
		return ExternalInterface.call("sys_setBackLightStrength", strength);
	}
	
	/**
	 * @brief	get backlight strength
	 * @param[in] NULL
	 * @return 
	 *		back light strengh if success of <0 if fail
	 */
	public static function getBackLightStrength():Number
	{
		return ExternalInterface.call("sys_getBackLightStrength");
	}

	/**
	 * @brief	Interface to Adjust color
	 * @param[in] Func	: Which func to select
	 *				- COLOR_Hue			:	1
	 *				- COLOR_Saturation	:	2
	 *				- COLOR_Brightness	:	3
	 *				- COLOR_Contrast	:	4
	 *				- COLOR_Sharpness	:	5
	 * @param[in] Mode	: Read or Write
	 *				- Mode_Read		:	0
	 *				- Mode_Write	:	1
	 * @param[in] Valuein	: value to trans in,max&min listed below
	 *				- COLOR_Hue     	-60 ---- 60
	 *				- COLOR_Saturation	-64 ---- 128
	 *				- COLOR_Brightness	-128 ---- 127
	 *				- COLOR_Contrast	-64 ---- 64
	 *				- COLOR_Sharpness	-31 ---- 31
	 * @return the selected func
	 *				- COLOR_Hue			:	1
	 *				- COLOR_Saturation	:	2
	 *				- COLOR_Brightness	:	3
	 *				- COLOR_Contrast	:	4
	 *				- COLOR_Sharpness	:	5
	 */
	public static function ColorAdjustIO(func:Number,mode:Number,valuein:Number):Number
	{
		return ExternalInterface.call("sys_ColorAdjustIO",func,mode,valuein);
	}
	
	/**
	 * @brief	get current time: year
	 * @param[in] NULL
	 * @return
	 *		year if success or <0 if fail
	 */
	public static function getYear():Number
	{
		return ExternalInterface.call("sys_getYear");
	}
	
	/**
	 * @brief	set current: year
	 * @param[in] newVal	: year to be set
	 * @return 
	 *		true if success or false if fail
	 */
	public static function setYear(newVal:Number):Boolean
	{
		return ExternalInterface.call("sys_setYear", newVal);
	}
	
	/**
	 * @brief	get current time: month
	 * @param[in] NULL
	 * @return
	 *		month if success or <0 if fail
	 */
	public static function getMonth():Number
	{
		return ExternalInterface.call("sys_getMonth");
	}
	
	/**
	 * @brief	set current: month
	 * @param[in] newVal	: month to be set
	 * @return 
	 *		true if success or false if fail
	 */
	public static function setMonth(newVal:Number):Boolean
	{
		 return ExternalInterface.call("sys_setMonth", newVal);
	}
	
	
	/**
	 * @brief	get current time: day
	 * @param[in] NULL
	 * @return
	 *		day if success or <0 if fail
	 */
	public static function getDay():Number
	{
		 return ExternalInterface.call("sys_getDay");
	}
	/**
	 * @brief	set current: day
	 * @param[in] newVal	: day to be set
	 * @return 
	 *		true if success or false if fail
	 */
	public static function setDay(newVal:Number):Boolean
	{
		 return ExternalInterface.call("sys_setDay", newVal);
	}

	/**
	 * @brief	get current time: wday
	 * @param[in] NULL
	 * @return
	 *		wday if success or <0 if fail(0:Sunday;1-6:Monday-Saterday)
	 */
	public static function getWday():Number
	{
		 return ExternalInterface.call("sys_getWday");
	}

	
	/**
	 * @brief	get current time: hour
	 * @param[in] NULL
	 * @return
	 *		hour if success or <0 if fail
	 */
	public static function getHour():Number
	{
		 return ExternalInterface.call("sys_getHour");
	}
	/**
	 * @brief	set current: hour
	 * @param[in] newVal	: hour to be set
	 * @return 
	 *		true if success or false if fail
	 */
	public static function setHour(newVal:Number):Boolean
	{
		 return ExternalInterface.call("sys_setHour", newVal);
	}	
	
	/**
	 * @brief	get current time: minute
	 * @param[in] NULL
	 * @return
	 *		minute if success or <0 if fail
	 */
	public static function getMin():Number
	{
		 return ExternalInterface.call("sys_getMin");
	}
	/**
	 * @brief	set current: minute
	 * @param[in] newVal	: minute to be set
	 * @return 
	 *		true if success or false if fail
	 */
	public static function setMin(newVal:Number):Boolean
	{
		 return ExternalInterface.call("sys_setMin", newVal);
	}	
	
	/**
	 * @brief	get current time: second
	 * @param[in] NULL
	 * @return
	 *		second if success or <0 if fail
	 */
	public static function getSecond():Number
	{
		 return ExternalInterface.call("sys_getSecond");
	}
	/**
	 * @brief	set current: second
	 * @param[in] newVal	: second to be set
	 * @return 
	 *		true if success or false if fail
	 */
	public static function setSecond(newVal:Number):Boolean
	{
		 return ExternalInterface.call("sys_setSecond", newVal);
	}
	
	
	/**
	 * @brief	get current time mode
	 * @param[in] NULL
	 * @return  	 
	 * - 0	:	SYS_CLOCKMODE_12
	 * - 1	:	SYS_CLOCKMODE_24
	 */
	public static function getClockMode():Number
	{
		 return ExternalInterface.call("sys_getClockMode");
	}
	/**
	 * @brief	set time mode
	 * @param[in] newVal	: new time mode
	 * 			- SYS_CLOCKMODE_12
	 * 			- SYS_CLOCKMODE_24	 
	 * @return 
	 *		true if success or false if fail
	 */
	public static function setClockMode(newVal:Number):Boolean
	{
		 return ExternalInterface.call("sys_setClockMode", newVal);
	}			
	
	/**
	 * @brief	get alarm time: hour
	 * @param[in] NULL
	 * @return
	 *		alarm time hour if success or <0 if fail
	 */
	public static function getAlarmHour():Number
	{
		 return ExternalInterface.call("sys_getAlarmHour");
	}
	/**
	 * @brief	set alarm time: houre
	 * @param[in] newVal	: new hour
	 * @return 
	 *		true if success or false if fail
	 */
	public static function setAlarmHour(newVal:Number):Boolean
	{
		 return ExternalInterface.call("sys_setAlarmHour", newVal);
	}		
	
	
	/**
	 * @brief	get alarm time: minute
	 * @param[in] NULL
	 * @return
	 *		alarm time minute if success or <0 if fail
	 */
	public static function getAlarmMin():Number
	{
		 return ExternalInterface.call("sys_getAlarmMin");
	}
	/**
	 * @brief	set alarm time: minute
	 * @param[in] newVal	: new minute
	 * @return 
	 *		true if success or false if fail
	 */
	public static function setAlarmMin(newVal:Number):Boolean
	{
		 return ExternalInterface.call("sys_setAlarmMin", newVal);
	}
	
	/**
	 * @brief	get alarm enable status
	 * @param[in] NULL
	 * @return
	 * - 0	:	disable
	 * - 1	:	enable
	 */
	public static function getAlarmEnable():Number
	{
		 return ExternalInterface.call("sys_getAlarmEnable");
	}
	/**
	 * @brief	set alarm enable 
	 * @param[in] newVal	: new alarm number
	 * @return 
	 *		true if success or false if fail
	 */
	public static function setAlarmEnable(newVal:Number):Boolean
	{
		 return ExternalInterface.call("sys_setAlarmEnable", newVal);
	}				
	
	
	/**
	 * @brief	get alarm ring
	 * @param[in] NULL
	 * @return
	 * - 0	:	SYS_ALARMRING_1
	 * - 1	:	SYS_ALARMRING_2
	 * - 2	:	SYS_ALARMRING_3 	 
	 */
	public static function getAlarmRing():Number
	{
		 return ExternalInterface.call("sys_getAlarmRing");
	}
	/**
	 * @brief	set alarm ring
	 * @param[in] newVal	: new alarm ring
	 * 			- SYS_ALARMRING_1
	 * 			- SYS_ALARMRING_2
	 * 			- SYS_ALARMRING_3 	 
	 * @return
	 *		true if success or false if fail
	 */
	public static function setAlarmRing(newVal:Number):Boolean
	{
		 return ExternalInterface.call("sys_setAlarmRing", newVal);
	}

	/**
	 * @brief	store your config change to memory\n
	 *  As you use any Function with App_Vram_write(...) \n
	 *  this function is required . 
	 * @param[in] NULL
	 * @return NULL
	 */
	public static function storeConfig():Boolean
	{
		return ExternalInterface.call("sys_storeConfig");
	}


	
	/**
	 * @brief	get current language identification
	 * @param[in] NULL
	 * @return 
	 *		current language id
	 */
	public static function getCurLanguage():Number
	{
	    return ExternalInterface.call("sys_getCurLanguage");
	}
	
	/**
	 * @brief	set current language id
	 * @param[in] newId	: the new language id
	 * @return 
	 *		true if success or false if fail
	 */
	public static function setCurLanguage(newId:Number):Boolean
	{
	    return ExternalInterface.call("sys_setCurLanguage", newId);
	}

	/**
	 * @brief	get number of language that the system support
	 * @param[in] NULL
	 * @return 
	 *		the number of language can be support
	 */
	public static function getLanguageTotalNum():Number
	{
	    return ExternalInterface.call("sys_getLanguageTotalNum");
	}

	/**
	 * @brief	get which language matched with the index
	 * @param[in] idx	: the range of this value is from 0 to the returned value of getLanguageTotalNum()
	 * @return 
	 *		language string, such as "en"
	 */
	public static function getLanguageIdx2Str(idx:Number):String
	{
	    return ExternalInterface.call("sys_getLanguageIdx2Str",idx);
	}
	
	/**
	 * @brief	check if card inserted
	 * @param[in] NULL
	 * @return 
	 * - 0	: cardup pthread create success
	 * - !0	: failed
	 */
	public static function systUpgrade(idx:Number):Number //richardyu 0509
	{
		return ExternalInterface.call("sys_systUpgrade",idx);
	}
	

	
	/**
	 * @brief	check if card inserted
	 * @param[in] NULL
	 * @return 
	 * - true	: already insert
	 * - false	: not inserted
	 */
	public static function checkCardStatus():Boolean
	{
		return ExternalInterface.call("sys_checkCardStatus");
	}
		
	/**
	 * @brief	check 4in1 card inserted type
	 * @param[in] NULL
	 * @return 
	 *		cardtype	:	sd/mmc/ms/xd see CARD_TYPE_SD etc
	 */
	public static function checkCardType():Number
	{
		return ExternalInterface.call("sys_checkCardType");
	}
	/**
	 * @brief	check if CF card inserted
	 * @param[in] NULL
	 * @return 
	 * - true	:	already insert
	 * - false	:	not inserted
	 */
	public static function checkCFStatus():Boolean
	{
		return ExternalInterface.call("sys_checkCFStatus");
	}
	
	/**
	 * @brief	check if CF card inserted
	 * @param[in] NULL
	 * @return 
	 * - true	:	already insert
	 * - false	:	not inserted
	 */
	public static function setVideoAutoPlayStatus(newId:Number):Number
	{
		return ExternalInterface.call("sys_setVideoAutoPlayStatus",newId);
	}

	/**
	 * @brief	check if CF card inserted
	 * @param[in] NULL
	 * @return 
	 * - true	:	already insert
	 * - false	:	not inserted
	 */
	public static function getVideoAutoPlayStatus():Number
	{
		return ExternalInterface.call("sys_getVideoAutoPlayStatus");
	}

/*----------------------richardyu 0523--------------------*/

	/**
	 * @brief	set the Audio setting value of Autoplay
	 * @param[in] NULL
	 * @return 
	 * - true	:	already insert
	 * - false	:	not inserted
	 */
	public static function setAudioAutoPlayStatus(newId:Number):Number
	{
		return ExternalInterface.call("sys_setAudioAutoPlayStatus",newId);
	}

	/**
	 * @brief	get the Audio setting value of Autoplay
	 * @param[in] NULL
	 * @return 
	 * - true	:	already insert
	 * - false	:	not inserted
	 */
	public static function getAudioAutoPlayStatus():Number
	{
		return ExternalInterface.call("sys_getAudioAutoPlayStatus");
	}

/*----------------------richardyu 0523--------------------*/


	/**
	 * @brief	check if USB1 inserted
	 * @param[in] NULL
	 * @return 
	 * - true	:	already insert
	 * - false	:	not inserted
	 */
	public static function checkUsb1Status():Boolean
	{
		return ExternalInterface.call("sys_checkUsb_one_Status");
	}
		/**
	 * @brief	check if USB2 inserted
	 * @param[in] NULL
	 * @return 
	 * - true	:	already insert
	 * - false	:	not inserted
	 */
	public static function checkUsb2Status():Boolean
	{
		return ExternalInterface.call("sys_checkUsb_two_Status");
	}

	/**
	 * @brief	get AutoPower Start able
	 * @param[in] NULL
	 * @return
	 * - 0	:	disable
	 * - 1	:	enable
	 */
	public static function getAutoPowerOnFlag():Number
	{
		 return ExternalInterface.call("sys_getAutoPowerOnFlag");
	}
	/**
	 * @brief	set  AutoPower Start able
	 * @param[in] newVal	:	0-disable;1-enable	 
	 * @return 
	 *		true if succsee or false if fail
	 */
	public static function setAutoPowerOnFlag(newVal:Number):Boolean
	{
		 return ExternalInterface.call("sys_setAutoPowerOnFlag", newVal);
	}					
	

   /**
	 * @brief	get AutoPower Start Hour
	 * @param[in] NULL
	 * @return
	 *		Number	:	hour---24 mode   
	 */
	public static function getAutoPowerOnHour():Number
	{
		 return ExternalInterface.call("sys_getAutoPowerOnHour");
	}

	   
	/**
	 * @brief	set  AutoPower Start Hour
	 * @param[in] newVal	: hour---24 mode 
	 * @return 
	 *		true if succsee or false if fail
	 */
	public static function setAutoPowerOnHour(newVal:Number):Boolean
	{
		 return ExternalInterface.call("sys_setAutoPowerOnHour", newVal);
	}				

	/**
	 * @brief	get AutoPower Start Minute
	 * @param[in] NULL
	 * @return
	 * - Number	:	Minute---0~59
	 */
	public static function getAutoPowerOnMin():Number
	{
		 return ExternalInterface.call("sys_getAutoPowerOnMin");
	}

	   
	/**
	 * @brief	set  AutoPower Start Minute
	 * @param[in] newVal	: Minute---0~59
	 * @return 
	 *		true if succsee or false if fail
	 */
	public static function setAutoPowerOnMin(newVal:Number):Boolean
	{
		 return ExternalInterface.call("sys_setAutoPowerOnMIn", newVal);
	}		

	
	/**
	 * @brief	get AutoPower close able
	 * @param[in] NULL
	 * @return
	 * - 0	:	disable
	 * - 1	:	enable    
	 */
	public static function getAutoPowerOffFlag():Number
	{
		 return ExternalInterface.call("sys_getAutoPowerOffFlag");
	}

	   
	/**
	 * @brief	set  AutoPower  close able
	 * @param[in] newVal	: 0-disable;1-enable
	 * @return 
	 *		true if succsee or false if fail
	 */
	public static function setAutoPowerOffFlag(newVal:Number):Boolean
	{
		 return ExternalInterface.call("sys_setAutoPowerOffFlag", newVal);
	}		
	

	/**
	 * @brief	get AutoPower close Hour
	 * @param[in] NULL
	 * @return
     *		number : hour---24 mode   
	 */
	public static function getAutoPowerOffHour():Number
	{
		 return ExternalInterface.call("sys_getAutoPowerOffHour");
	}

	   
	/**
	 * @brief	set  AutoPower  close Hour
	 * @param[in] newVal	: hour---24 mode
	 * @return 
	 *		true if succsee or false if fail
	 */
	public static function setAutoPowerOffHour(newVal:Number):Boolean
	{
		 return ExternalInterface.call("sys_setAutoPowerOffHour", newVal);
	}		
	

	/**
	 * @brief	get AutoPower close minute
	 * @param[in] NULL
	 * @return
     *		number	:	minute---0~59 
	 */
	public static function getAutoPowerOffMin():Number
	{
		 return ExternalInterface.call("sys_getAutoPowerOffMin");
	}

	   
	/**
	 * @brief	set  AutoPower  close minute
	 * @param[in] newVal	: minute---0~59
	 * @return 
	 *		true if succsee or false if fail
	 */
	public static function setAutoPowerOffMin(newVal:Number):Boolean
	{
		 return ExternalInterface.call("sys_setAutoPowerOffMin", newVal);
	}		


	/**
	 * @brief	get AutoPower Freq
	 * @param[in] NULL
	 * @return
	 * - 0	:	disable
	 * - 1	:	enable 
	 */
	public static function getAutoPowerFreq():Number
	{
		 return ExternalInterface.call("sys_getAutoPowerFreq");
	}

	   
	/**
	 * @brief	set  AutoPower  Freq
	 * @param[in] newVal	: autopower frequency 
	 * @return 
	 *		true if succsee or false if fail
	 */
	public static function setAutoPowerFreq(newVal:Number):Boolean
	{
		 return ExternalInterface.call("sys_setAutoPowerFreq", newVal);
	}		

	/**
	 * @brief	get media type from media number
	 * @param[in] newVal	: media type
	 *				- SYS_MEDIA_INTERNAL
	 *				- SYS_MEDIA_SD
	 *				- SYS_MEDIA_CF
	 *				- SYS_MEDIA_UDISK
	 * @return  
	 *		"C""D""E""F"......
	 */
	public static function media2Disk(newVal:Number):String
	{
		 return ExternalInterface.call("sys_media2Disk", newVal);
	}	
	
	/**
	 * @brief	set the media to be active
	 * @param[in] media	: media to set
	 *				- SYS_MEDIA_INTERNAL
	 *				- SYS_MEDIA_SD
	 *				- SYS_MEDIA_CF
	 *				- SYS_MEDIA_UDISK
	 * @return 
	 *		true if succsee or false if fail
	 */
	public static function setActiveMedia(media:Number):Number
	{
		 return ExternalInterface.call("sys_setActiveMedia", media);
	}

	/**
	 * @brief	reset system to default setting
	 * @param[in] NULL
	 * @return 
	 *		true if succsee or false if fail
	 */
	public static function resetDefaultSetting():Boolean
	{
	    return ExternalInterface.call("sys_resotreSysDefaultConfig");
	}
	
	/**
	 * @brief	get the working path, e.g. c:\ d:\ ...
	 * @param[in] NULL
	 * @return 
	 *		the working path
	 */
	public static function getActiveWorkPath():String
	{
		return ExternalInterface.call("sys_getActiveWorkPath");
	}
	
	/**
	 * @brief	clear all key message remained in flash playing task
	 * @param[in] NULL
	 * @return NULL
	 */
	public static function clearKeyMessage():Void
	{
		ExternalInterface.call("sys_clearKeyMessage");
	}	

	/**
	 * @brief	enable the printer
	 * @param[in] isenable	: enable or disable the printer
	 * @return
	 *		true if succsee or false if fail
	 */
	public static function enablePrinter(isenable:Number):Number
	{
		return ExternalInterface.call("sys_enablePrinter",isenable);
	}
	
	/**
	 * @brief	get the disk symbol for private disk
	 * @param[in] NULL
	 * @return
	 *		the disk symbol or 'C' for default
	 */
	public static function getPrivateDiskSymbol():String
	{
		return ExternalInterface.call("sys_getPrivateDiskSymbol");
	}
	
	/**
	 * @brief	tell player that the hotplub information has been accepted
	 * @param[in] mediaType	: card or cf or usb
	 * @param[in] plugType	: 0 for plugout and 1 for plugin
	 * @return NULL
	 */
	public static function acceptHotplugInfo(mediaType:Number,plugType:Number):Void
	{
		ExternalInterface.call("sys_acceptHotplugInfo",mediaType,plugType);
	}
	/**
	* @brief	tell player that the usb device information has been accepted
	* @param[in] plugType	: 0 for plugout and 1 for plugin
	* @return NULL
	*/
	public static function acceptPCconInfo(plugType:Number):Void
	{
		ExternalInterface.call("sys_acceptPCconInfo",plugType);
	}

	/**
	 * @brief	check usb device status when machine startup
	 * @param NULL
	 * @return 
	 *		connect status:
	 *			1:	connected
	 *			0:	disconnected
	 */
	public static function checkStartupUstatus():Number
	{
		return ExternalInterface.call("sys_checkStartupUstatus");
	}
	/**
	 * @brief	set HDMI Mode
	 * @param[in] ModeType	: HDMI mode to set
	 * @return 
	 *		true if succsee or false if fail
	 * 
	 */
	public static function setHDMIMode(ModeType:Number):Boolean
	{
		 return ExternalInterface.call("sys_setHDMIMode", ModeType);
	}

	/**
	 * @brief	get HDMI Mode
	 * @param[in] NULL
	 * @return 
	 *		ModeType	: HDMI mode 
	 */
	public static function getHDMIMode():Number
	{
		 return ExternalInterface.call("sys_getHDMIMode");
	}

	/**
	 * @brief	set Output previous Mode when this mode is error
	 * @param[in] NULL
	 * @return 
	 *		true if succsee or false if fail
	 */
	public static function setOutputPrevMode():Boolean
	{
		 return ExternalInterface.call("sys_setOutputPrevMode");
	}

	/**
	 * @brief	set Output Mode
	 * @param[in] OutputMode	:  Output mode to set(1-HDMI;	2-CVBS;	4-YPbPr)
	 * @return 
	 *		true if succsee or false if fail
	 */
	public static function setOutputMode(OutputMode:Number):Boolean
	{
		 return ExternalInterface.call("sys_setOutputMode",OutputMode);
	}
	
	/**
	 * @brief	get Output Mode
	 * @param[in] NULL
	 * @return 
	 *		OutputMode
	 *		- 1	:	HDMI
	 *		- 2	:	CVBS
	 *		- 4	:	YPbPr
	 */
	public static function getOutputMode():Number
	{
		 return ExternalInterface.call("sys_getOutputMode");
	}

	/**
	 * @brief	get Output Mode
	 * @param[in] param index	:
	 *				- 1		get screen width
	 *				- 2		get screen height
	 * @return 
	 *		true if succsee or false if fail
	 */
	public static function getCurscreenParam(paramIndex:Number):Number
	{
		 return ExternalInterface.call("sys_getCurscreenparam",paramIndex);
	}
	/**
	 * @brief	get the number of sectors when the mass storage is multi secotr
	 * @param[in] NULL
	 * @return
	 *		the number of the sectors
	 */
	public static function getMultiSectorNum():Number
	{
		return ExternalInterface.call("sys_getMultiSectorNum");
	}
	
	/**
	@brief get the status of upgrading 
	@param[in] NULL
	@return the status which the lower 8 bits is the processing and the next 8 bits is the state
	**/
	public static function getCardUpgradeStatus():Number
	{
		return ExternalInterface.call("sys_getCardUpgradeStatus");
	}
	/**
	@brief get the status of ota upgrading 
	@param[in] NULL
	@return the status which the lower 8 bits is the processing and the next 8 bits is the state
	**/
	public static function getOtaUpgradeStatus():Number
	{
		return ExternalInterface.call("sys_getOtaUpgradeStatus");
	}
	public static function getOtaLocalVersion():String
	{
		return ExternalInterface.call("sys_getOtaLocalVersion");
	}
	public static function getOtaServerVersion():String
	{
		return ExternalInterface.call("sys_getOtaServerVersion");
	}
	public static function getOtaCheckStatus():Number
	{

		return ExternalInterface.call("sys_getOtaCheckStatus");
	}

	/**
	 * @brief	get Auto Screen off able
	 * @param[in] NULL
	 * @return
	 * - 0	:	disable
	 * - 1	:	enable
	 */
	public static function getAutoScreenOffFlag():Number
	{
		 return ExternalInterface.call("sys_getAutoScreenOffFlag");
	}

	/**
	 * @brief	set  Auto Screen off able
	 * @param[in] newVal	:	0-disable;1-enable	 
	 * @return 
	 *		0 if succsee or -1 if fail
	 */
	public static function setAutoScreenOffFlag(newVal:Number):Boolean
	{
		 return ExternalInterface.call("sys_setAutoScreenOffFlag", newVal);
	}					
	
   /**
	 * @brief	get Auto Screen off time
	 * @param[in] NULL
	 * @return
	 *		Number	:	Auto Screen off time
	 */
	public static function getAutoScreenOffTime():Number
	{
		 return ExternalInterface.call("sys_getAutoScreenOffTime");
	}
 
	/**
	 * @brief	set  Auto Screen off time
	 * @param[in] newVal	: Auto Screen off time
	 * @return 
	 *		0 if succsee or -1 if fail
	 */
	public static function setAutoScreenOffTime(newVal:Number):Boolean
	{
		 return ExternalInterface.call("sys_setAutoScreenOffTime", newVal);
	}		
	
  /**
   * @brief Get battery charging status.
   *
   * @return The charging status:
      - UNKNOWN
      - CHARGING
      - DISCHARGING
      - NOT_CHARGING
      - FULL
   */
	public static function getBatteryChargeStatus():String
	{
		 return ExternalInterface.call("sys_getBatteryChargeStatus");
	}	
	
	

  /**
   * @brief Get battery current voltage.
   *
   * @return A string reprent the current voltage.
   */
	public static function getBatteryVoltage():String
	{
		 return ExternalInterface.call("sys_getBatteryVoltage");
	}			

	/**
	 * @brief	change ddr pll
	 * @param[in] clock	: cloce rate see DDR_LOW_PLL etc..
	 * @return 
	 *		0 if succsee or -1 if fail
	 */
	public static function changePll(clock:Number):Number
	{
		 return ExternalInterface.call("sys_change_pll", clock);
	}	

	/**
	@brief get the global value 
	@param[in] cmd: get num or get str, see CMD_GET_GLONUM
	@param[in] index: the index of the num or str
	@return 
		if the cmd==CMD_GET_GLONUM, you should convert the string which returnd to the num by yourself
		else 
			get the str
		if string returned is NULL, it means get value failed
	**/
	public static function getGlobalValue(cmd:Number,index:Number):String
	{
		 return ExternalInterface.call("sys_getGlobalValue", cmd,index);
	}	
	
	/**
	 * @brief	execute standby
	 * @param[in] NULL
	 * @return 
	 *		0 if succsee or -1 if fail
	 */
	public static function Standby():Number
	{
		 return ExternalInterface.call("sys_Standby");
	}


	/**
	 * @brief	access the path where had the specified mode
	 * @param[in] path : it can be a dir or a file
	 * @param[in] mode : see MODE_R_OK etc
	 * @return 
	 *		0 if success or -1 if fail
	 */
	public static function access(path:String,mode:Number):Number
	{
		return ExternalInterface.call("sys_Access");
	}

	/**
	 * @brief	execute softap
	 * @param[in] NULL
	 * @return 
	 *		0 if succsee or -1 if fail
	 */
	public static function Softap():Number
	{
		return ExternalInterface.call("sys_Softap");
	}
		/**
	 * @brief	stop softap
	 * @param[in] NULL
	 * @return 
	 *		0 if succsee or -1 if fail
	 */
	public static function StopSoftap():Number
	{
		 return ExternalInterface.call("sys_StopSoftap");
	}

	/**
	 * @brief     Start Net Display
	 *
	 */
	public static function StartNetDisplay():Number
	{
		return ExternalInterface.call("sys_StartNetDisplay");
	}        

	/**
	 * @brief  Stop Net Display
	 *
	 */
	public static function StopNetDisplay():Void
	{
		ExternalInterface.call("sys_StopNetDisplay");
	}

	public static function checkMediaStatus(media_type:Number):Boolean
	{
		return ExternalInterface.call("sys_checkMediaStatus",media_type);
	}
	
      /**
        * @brief        for LAN prepare.
        *
        */
	public static function LANPrepare():Void
	{
		ExternalInterface.call("sys_LanStart");
	}
        /**
         * @brief    Set for USB Display
         * @param[in] sel: 0,for storage use, 1: for usb display use.
         * @return
         *            0 if succsee or -1 if fail
         */
	public static function SetFouUSBDisplay(sel:Number):Void
	{
		ExternalInterface.call("sys_SetForUSBDisplay",sel);
	}

	/**
	 * @brief	Get Lan Status
	 * @param[in] NULL
	 * @return  Lan connect status
	 *		0 for no connect, 1 for connect
	 */
	public static function GetLanStatus():Number
	{
		return ExternalInterface.call("sys_GetLanStatus");
	}
	/**
	* @brief        Set for USB Mux
	* @param[in] sel: 1,switch to USB TYPB-A, 0: switch to usb TYPE-B.
	* @return
	*            0 if succsee or -1 if fail
	*/
	public static function setUsbMux(sel:Number):Void
	{
		ExternalInterface.call("sys_setUsbMux",sel);
	}
	/**
	* @brief        Set for USB TYPE-A Power ON/OFF
	* @param[in] sel: 0,for power OFF, 1: for power ON.
	* @return
	*            0 if succsee or -1 if fail
	*/
	public static function setUsbPower(sel:Number):Void
	{
		ExternalInterface.call("sys_setUsbPower",sel);
	}
		

	/**
	 * @brief      check memory used, add by richard
	 *
	 */
	public static function MemCheck():Number
	{
		return ExternalInterface.call("sys_checkRam");
	}
	/**
	 * @brief      get the real width of prefer character with specific fontsize, add by richard 0503
	 *
	 */
	public static function GetCharWidth(charcode:Number,fontsize:Number):Number
	{
		return ExternalInterface.call("sys_getcharwidth", charcode, fontsize);
	}
	public static function Getwifimode():Number
	{
		return ExternalInterface.call("sys_getwifimode");
	}
	public static function Putwifimode(wifi_mode:Number)
	{
		ExternalInterface.call("sys_putwifimode",wifi_mode);
	}
	public static function getlanstatus():Number
	{
		return ExternalInterface.call("sysinfo_getlanstatus");
	}
	public static function getnetlinkstatus():Number
	{
		return ExternalInterface.call("sysinfo_getnetlinkstatus");
	}
	public static function getCastCodeOnOff()
	{
		 return ExternalInterface.call("sys_get_passcode_onoff");
	}
	public static function getUserDefCastCode()
	{
		 return ExternalInterface.call("sys_get_user_define_passcode");
	}
	public static function setdisplaypassword(password:String)
	{
		 ExternalInterface.call("sysinfo_getpasswordfromDPF",password);
	}
	public static function getwifiautochannel():Number
	{
		return ExternalInterface.call("wifi_getautochannelchoose");
	}
	public static function putwifiautochannel(autochannelflag:Number)
	{
		 ExternalInterface.call("wifi_putautochannelchoose",autochannelflag);
	}
	public static function checkbinfilesexsit(storage_index:Number):Number
	{
		 return ExternalInterface.call("sysinfo_checkbinfilesexsit",storage_index);
	}
	public static function startcdromfunc()
	{
		  ExternalInterface.call("sysinfo_startcdromfunc");
	}
	/**
	 * @brief      get the hostname of DPF, add by chenshouhui
	 *
	 */	
	public static function gethostname():String
	{
		 return ExternalInterface.call("sysinfo_gethostname");
	}
	/**
	 * @brief      set the hostname of DPF, add by chenshouhui
	 *
	 */	
	public static function sethostname(hostname:String)
	{
		  ExternalInterface.call("sysinfo_sethostname",hostname);
	}
	/**
	 * @brief      set the value of gpio82 and gpio83 for usbswitch, add by chenshouhui
	 *
	 */	
	public static function setgpioforusbswitch(dirction:Number)
	{
		 return ExternalInterface.call("sysinfo_setgpioforusbswitch",dirction);
	}
	/**
	 * @brief    get the netlink status for into set_lan.SWF, add by chenshouhui
	 *
	 */	
	public static function get_netlink_status_for_intoSWF():Number
	{
		 return ExternalInterface.call("sysinfo_netlink_for_intoSWF");
	}
	;		
	/**
	 * @brief	Get usbcompsite device total
	 * @param[in] NULL
	 * @return  device total
	 *		
	 */
	public static function get_usbcompsite_device_total():Number
	{
		 return ExternalInterface.call("sysinfo_getusbcompsitedevicetotal");
	}
	;		
	/**
	 * @brief	Get usbcompsite interface total
	 * @param[in] NULL
	 * @return  interface total
	 *		
	 */
	public static function get_usbcompsite_interface_total(device_index:Number):Number
	{
		 return ExternalInterface.call("sysinfo_getusbcompsitedeviceintfacetotal",device_index);
	}
	;	
	/**
	 * @brief	Get usbcompsite interface infomation
	 * @param[in] index:the index of device 
	 * @return  device name infomation
	 *		
	 */
	public static function get_usbcompsite_device_name(index:Number):String
	{
		 return ExternalInterface.call("sysinfo_getusbcompsitedevicenameinformation",index);
	}
	;	
	/**
	 * @brief	Get usbcompsite interface infomation
	 * @param[in] dev_index:the index of device 
	 			   interface_index:the index of interface   
	 * @return  device class infomation
	 *		
	 */
	public static function get_usbcompsite_device_class_infomation(dev_index:Number,interface_index:Number):String
	{
		 return ExternalInterface.call("sysinfo_getusbcompsitedeviceclassinformation",dev_index,interface_index);
	}
	;	
		/**
	 * @brief	decide to insmod KO by the  dev_index and interface_index
	 * @param[in] dev_name:the name of device 
	 			   interface_name:the name of interface
	 * @return  device class infomation
	 *		
	 */
	public static function usbcompsite_choose_function(dev_index:Number,intf_idex:Number,class_type:Number)
	{
		 ExternalInterface.call("sysinfo_chooseusbcompsitefunction",dev_index,intf_idex,class_type);
	}
		/**
	 * @brief	get the type of connectPC
	 * @param[NULL] 
	 * @return 
			 0:	 g_file_storage
			 1:  EZ USB
	 *		
	 */
	public static function get_connectPC_flag():Number
	{
		 return ExternalInterface.call("sysinfo_getconnectpcflag");
	}
		/**
	 * @brief	creat edid bin file:/am7x/case/data/edid.bin
	 * @param[in] 
	 		hdmi_valid : 0 disable 1 enable
	 		hdmi_format:hdmi resolution
	 		vga_valid : 0 disable 1 enable
	 		vga_format:vga resolution
				
	 		
	 * @return
	 		0:	creat successfully
			 -1: creat file
	 *		
	 */
	public static function creat_edid_bin_file(hdmi_valid:Number,hdmi_format:Number,vga_valid:Number,vga_format:Number):Number
	{
		 return ExternalInterface.call("sysinfo_createdidbin",hdmi_valid,hdmi_format,vga_valid,vga_format);
	}
	/**
	 * @brief	delete edid bin file:/am7x/case/data/edid.bin
	 * @param[NULL] 
	 * @return 
			 0:	delete successfully
			 -1: delete file
	 *		
	 */
	public static function delete_edid_bin_file():Number
	{
		 return ExternalInterface.call("sysinfo_deleteedidbin");
	}
	/**
	 * @brief	delete edid bin file:/mnt/vram/edid.bin
	 * @param[in] choose_index
	 				0:hdmi enbale or disable
	 				1:hdmi format
	 				2:vga format
	 * @return result
			
	 *		
	 */
	public static function read_edid_bin_file(choose_index:Number):Number
	{
		 return ExternalInterface.call("sysinfo_readedidbin",choose_index);
	}

	/**
	 * @brief	get the total of mass storage total
	 * @param[in] NULL
	 				
	 * @return result
			
	 *		
	 */
	public static function get_mass_storage_total():Number
	{
		 return ExternalInterface.call("sysinfo_getstoragetotal");
	}

	/**
	 * @brief	get the mutisector infomation such as host controller index and mutisector number
	 * @param[in] 
	 		device_index:
	  				0:usb port1
	 				1:usb port2
	 		info_index:
	  				0:mutisector number
	 				1:host controller index	
	 * @return result
			
	 *		
	 */
	public static function get_udisk_mutisector_info(device_index:Number,info_index:Number):Number
	{
		 return ExternalInterface.call("sysinfo_getmutisectorinfo",device_index,info_index);
	}

	/*
	*
	*
	*/
	public static function get_udisk_devname_info(device_index:Number,info_index:Number):Number
	{
		 return ExternalInterface.call("sysinfo_getusbmass_devname",device_index,info_index);
	}
	
		/**
	 * @brief	get the flag of mtp device inserting in or no
	 * @param[in] NULL
	 				
	 * @return the flag
	 			0:mtp device out
	 			1:mtp device in
			
	 *		
	 */
	public static function get_mtp_device_in_flag():Number
	{
		 return ExternalInterface.call("sysinfo_getmtpdeviceinflag");
	}
	;	

	/**
	 * @brief	write the search type to /tmp/mtpfs_search_type
	 * @param[in] search_type:1 photo;2 video 3 audio 4 office
	 				
 	 * @return NULL
			
	 *		
	 */
	public static function write_mtpfs_search_type(search_type:Number)
	{
		  ExternalInterface.call("sysinfo_writemtpfssearchtype",search_type);
	}
	;	
	/**
	 * @brief	upgrade with ota
	 * @param[in] mode:
	 	param[in]   url  :the url of firmware	
 	 * @return NULL
			
	 *		
	 */

	public static function ota_upgrade(mode:Number,url:String):Number
	{
		 	return  ExternalInterface.call("sysinfo_otaupgrade",mode,url);
		 
	}
	/**
	 * @brief	upgrade with ota from APP
	 * @param[in] NULL
 	 * @return NULL
			
	 *		
	 */

	public static function ota_upgradeFromApp():Number
	{
		 	return  ExternalInterface.call("sysinfo_otaupgrade_from_app");
		 
	}
	/**
	 * @brief	set schema to json file
	 * @param[in] op:		JSON_SET_EZMIRROR_START	0
						JSON_SET_EZMIRROR_STOP		1
						JSON_SET_INTE_SSID			4
						JSON_SET_LANGUAGE			5
				val:		only when set language need this param, other operation would ignore this param.
 	 * @return: on sucess, return 0;
			
	 *		
	 */
	public static function json_set_value(op:Number, val:String):Number
	{
		 	return  ExternalInterface.call("sys_json_set_value",op, val);
		 
	}
	/**
	 * @brief	get the mask of enable ezcast
	 * @param[in] op:		NULL
 	 * @return: 			0: Disable EZCast    1: Enable EZCast
	 */
	public static function ezcast_mask():Number
	{
		 	return  ExternalInterface.call("sys_ezcast_mask");
		 
	}
	/**
	 * @brief	get language auto set status
	 * @param[in] op:		NULL
 	 * @return: 			0: enable auto set    1: disable auto set
	 */
	public static function getlangdisautostatus():Number
	{
		 	return  ExternalInterface.call("sys_get_disauto_status");
		 
	}
	/**
	 * @brief	set language auto set status
	 * @param[in] op:		0: enable auto set    1: disable auto set
 	 * @return: 			NULL
	 */
	public static function setlangdisautostatus(val:Number)
	{
		 	ExternalInterface.call("sys_set_disauto_status", val);
		 
	}
	/**
	 * @brief	get language from appInfo.json
	 * @param[in] op:		NULL
 	 * @return: 			language country code
	 */
	public static function langautoget():String
	{
		 	return  ExternalInterface.call("sys_langautoget");
		 
	}
	/**
	 * @brief	Get current WiFi display status(DLNA/Airplay/display).
	 * @param[in] op:		NULL
 	 * @return: 			the current status, 	enum{
	 *											EZCAST_STATUS_NULL = 0,
	 *											EZCAST_STATUS_DISPLAY,
	 *											EZCAST_STATUS_AIRPLAY,
	 *											EZCAST_STATUS_DLNA
	 *										}
	 */
	public static function wifidisplay_gets_tatus():Number
	{
		 	return  ExternalInterface.call("sys_get_display_status");
		 
	}
	/**
	 * @brief	Get EZCast OTA vendor.
	 * @param[in] op:		NULL
 	 * @return: 			the EZCast OTA vendor
	 */
	public static function get_ezcastvendor():String
	{
		 	return  ExternalInterface.call("sys_get_ezcast_vendor");
		 
	}
	/**
	 * @brief	Get the status that is it need 8252 to 8251 pay upgrade.
	 * @param[in] op:		NULL
 	 * @return: 			0: Not need;   1: Need;
	 */
	public static function isPayUpgrade():Number
	{
		 	return  ExternalInterface.call("sys_isPayUpgrade");
		 
	}
	/**
	 * @brief	reset the socket of wifi display, only reset the socket when the functions(wifidisplay, dlna and airplay) is not running.
	 * @param[in] op:		NULL
 	 * @return: 			NULL
	 */
	public static function ezsocket_reset():Number
	{
		 	return  ExternalInterface.call("sys_ez_socket_reset");
		 
	}
	/**
	 * @brief	get the UI when last power down
	 * @param[in] op:		NULL
 	 * @return: 			0: init value, load help UI; 1: DLNA/EZAir UI; 2: EZMirra UI;
	 */
	public static function get_lastui():Number
	{
		 	return  ExternalInterface.call("sys_get_last_ui");
		 
	}
	/**
	 * @brief	Get language index
	 * @param[in] op:		NULL
 	 * @return: 			language country code
	 */
	public static function Get_language_index():Number
	{
		 	return  ExternalInterface.call("sys_Get_language_index");
		 
	}

	/**
	 * @brief	Set language index
	 * @param[in] op:		NULL
 	 * @return: 			language country code
	 */
	public static function Set_language_index(lan_index:Number)
	{
		 	return  ExternalInterface.call("sys_Set_language_index");
		 
	}

	/**
	 * @brief	check ezmusic flag
	 * @param[in] op:		NULL
 	 * @return: 			ezmusic flag
	 */
	public static function Check_ezmusic_flag():Number
	{
		 	return  ExternalInterface.call("sys_check_ezmusic_flag");
		 
	}
	/**
	 * @brief	set schema to json file
	 * @param[in] op:		last_ui:	0 ->  init value, load help UI; 	1 ->  DLNA/EZAir/Setting UI; 	2 ->  EZMirra UI;
	 *					changed_by_user:	This set is system set or user set, this op is for schema.	0 -> system set;	1 -> user set;	
 	 * @return: 			NULL
	 */
	public static function set_lastui(last_ui:Number, set_by_user:Number)
	{
		 	ExternalInterface.call("sys_set_last_ui", last_ui, set_by_user);
		 
	}
	/**
	 * @brief	set hostname by user, and take effect after reboot.
	 * @param[in] op:		hostname:	hostname
 	 * @return: 			NULL
	 */
	public static function setDevName(hostname:String)
	{
		 	ExternalInterface.call("sys_set_device_name", hostname);
		 
	}
	/**
	 * @brief	get hostname which user setted, and not care is it take effect.
	 * @param[in] op:		NULL
 	 * @return: 			hostname
	 */
	public static function getDevName():String
	{
		 	return ExternalInterface.call("sys_get_device_name");
		 
	}
	/**
	 * @brief	Get how many people download this firmware
	 * @param[in] op:		NULL
 	 * @return: 			Download number
	 */
	public static function get_ota_num():Number
	{
		 	return ExternalInterface.call("sys_get_ota_download_number");
		 
	}
	/**
	 * @brief	Get OTA enforce flag 
	 * @param[in] op:		NULL
 	 * @return: 			0: Do not force to OTA upgrade; 1: Force to OTA upgrade
	 */
	public static function get_ota_enforce():Number
	{
		 	return ExternalInterface.call("sys_get_ota_enforce");
		 
	}
	/**
	 * @brief	Get CPU frequency
	 * @param[in] op:		NULL
 	 * @return: 			CPU frequency
	 */
	public static function get_cpu_frequency():Number
	{
		 	return ExternalInterface.call("sys_get_cpu_frequency");
		 
	}
	/**
	 * @brief	Get system time
	 * @param[in] op:		NULL
 	 * @return: 			System time
	 */
	public static function time():Number
	{
		 	return ExternalInterface.call("sysinfo_get_system_time");
		 
	}
	/**
	 * @brief	Get IP address in WLAN0/WLAN1
	 * @param[in] op:		port: 0: WLAN0; else: WLAN1
 	 * @return: 			IP address
	 */
	public static function getIP(port:Number):String
	{
		 	return ExternalInterface.call("sysinfo_get_ip", port);
		 
	}
	/**
	 * @brief	Get Router control enable set value
	 * @param[in] op:		NULL
 	 * @return: 			set value, 0: disable, 1: enable;
	 */
	public static function getRouterEnable():Number
	{
		 	return ExternalInterface.call("sysinfo_get_router_ctl");
		 
	}
	/**
	 * @brief	Get connect mode
 	 * @return: 			0-->Direct Only
	 *					1-->Through Router Allow
	 *					2-->Router Only
	 */
	public static function getConnectMode()
	{
		 	return ExternalInterface.call("sysinfo_get_connect_mode");
		 
	}
	/**
	 * @brief	Set connect mode
	 * @param[in] op:		val: Set val:
	 *							0-->Direct Only
	 *							1-->Through Router Allow
	 *							2-->Router Only
	 					toSave:
	 							true-->Save the value to config
	 							false-->Do not save the value to config
 	 * @return: 			NULL
	 */
	public static var DIRECT_ONLY	= 0;
	public static var ROUTER_ALLOW	= 1;
	public static var ROUTER_ONLY	= 2;
	public static function setConnectMode(val:Number)
	{
		 	ExternalInterface.call("sysinfo_set_connect_mode", val);
		 
	}
	/**
	 * @brief	get the config of test mode, the config file is "test_config.txt"
	 * @param[in] flag:	Operation
 	 * @return: 			Return value, if the value is number, it will turn to string.
	 */
	public static var GET_TEST_USB_PATH:String = "get_usb_path";
	public static var GET_TEST_WIFI_ENABLE:String = "wifi_test";
	public static var GET_TEST_VIDEO_ENABLE:String = "video_test";
	public static var GET_TEST_VERSION_ENABLE:String = "version_test";
	public static var GET_TEST_LAG_CONF_ENABLE:String = "language_config";
	public static var GET_TEST_EDID_CONF_ENABLE:String = "edid_config";
	public static var GET_TEST_ENABLE:String = "get_test_enable";
	public static var GET_TEST_SSID:String = "ssid";
	public static var GET_TEST_PSK:String = "psk";
	public static var GET_TEST_SIGNAL_STANDARD:String = "signal_standard";
	public static var GET_TEST_VIDEO_FILE:String = "v_file";
	public static var GET_TEST_LOOP_NUM:String = "v_num";
	public static var GET_TEST_VERSION:String = "version";
	public static var GET_TEST_CONFIG_LANGUAGE:String = "language";
	public static var RETURN_VALUE_ENABLE:String = "enable";
	public static var RETURN_VALUE_DISABLE:String = "disable";
	public static var RETURN_VALUE_TRUE:String = "true";
	public static var RETURN_VALUE_FALSE:String = "false";

	public static function get_test_config(flag:String):String
	{
		 	return  ExternalInterface.call("sys_get_test_config", flag);
		 
	}
	/**
	 * @brief	get the macro definition number value.
	 * @param[in] opt:		Operation;
 	 * @return: 			The macro definition number value.
	 */
	public static var INC_EZCAST_ENABLE:Number 			= 0;
	public static var INC_EZCAST_LITE_ENABLE:Number 	= 1;
	public static var INC_EZWILAN_ENABLE:Number 		= 2;
	public static var INC_AUTOPLAY_SET_ENABLE:Number 	= 3;
	public static var INC_TEST_ENABLE:Number 			= 4;
	public static var INC_EZCAST5G_ENABLE:Number 		= 5;
	public static var INC_ROUTER_ONLY_ENABLE:Number		= 6;
	public static var INC_EZWIRE_ENABLE:Number 			= 7;
	public static var INC_EZWIRE_TYPE:Number			= 8;
	public static var INC_LAN_ONLY:Number				= 9;
	public static var INC_EZWIRE_CAPTURE:Number			= 10;
	public static var INC_EZWIRE_ANDROID_ENABLE:Number	= 11;
	public static var INC_IS_SNOR_FLASH:Number			= 12;
	public static var INC_PNP_USE_WIFIDISPLAY:Number	= 13;
	public static function getIncNumVal(opt:Number):Number
	{
		 	return  ExternalInterface.call("sys_getIncludeNumberValue", opt);
		 
	}
	public static function set_factory_test_video_end(flag:Number)
	{
		 	return  ExternalInterface.call("sys_set_factory_test_video_end", flag);
		 
	}
	 /**
         * @brief    set stream play status
         * @param[in] sel: 0,stream start, 1: stream stop.
         * @return NULL
         */
	public static function set_stream_play_status(flag:Number)
	{
		 	return  ExternalInterface.call("sys_set_stream_play_status", flag);
		 
	}		
	/**
	 * @brief	reboot system
	 * @param[in] NULL:
 	 * @return NULL
			
	 *		
	 */

	public static function reboot_system()
	{
		 	 ExternalInterface.call("sysinfo_rebootsystem");
		 
	}
	public static function mainswf_skip_get_keycode():Number
	{
		return ExternalInterface.call("sysinfo_mainswf_skip_get_keycode");
	}
	public static function deleteWifiConf():Number
	{
		return ExternalInterface.call("sys_deleteWifiConf");
	}
	public static function deleteEZCastConf():Number
	{
		return ExternalInterface.call("sys_deleteEZCastConf");
	}
	public static function deleteEZmusicConf():Number
	{
		return ExternalInterface.call("sys_deleteEZmusicConf");
	}
	public static function sys_volume_ctrl_get()
	{
		return ExternalInterface.call("sys_sysinfo_sys_volume_ctrl_get");
	}

	public static function ezcastpro_read_mode():Number
	{
		return ExternalInterface.call("sys_ezcastpro_read_mode");
	}
	
	 public static function mount_usb_tohttp()
   	{
	   return ExternalInterface.call("mount_usb_tohttp");
   	}
	 
  	 //umount usb
	public static function umount_usb()
   	{
	   return ExternalInterface.call("umount_usb");
   	}
  

 	public static function mount_card_tohttp()
   	{
	   return ExternalInterface.call("mount_card_tohttp");
   	}
  	 //umount usb
	public static function umount_card()
   	{
	   return ExternalInterface.call("umount_card");
   	}
	public static function sysinfo_version_factorytest():String
	{
	   return ExternalInterface.call("sysinfo_version_factorytest");
   	}
		
	public static function sysinfo_language_factorytest():String
	{
	   return ExternalInterface.call("sysinfo_language_factorytest");
   	}
	public static function sysinfo_get_lan_factorytest():String
	{
	   return ExternalInterface.call("sysinfo_get_lan_factorytest");
   	}
	public static function sysinfo_get_uart_factorytest():String
	{
	   return ExternalInterface.call("sysinfo_get_uart_factorytest");
   	}
	public static function sysinfo_factorytest():Number
	{
	   return ExternalInterface.call("sysinfo_factorytest");
   	}
	public static function sysinfo_init_factorytest():Number
   	{
	   return ExternalInterface.call("sysinfo_init_factorytest");
   	}
	public static function sysinfo_get_Throughput():String
   	{
	   return ExternalInterface.call("sysinfo_get_Throughput");
   	}
	
	public static function sysinfo_get_edid_factorytest():String
	{
	   return ExternalInterface.call("sysinfo_get_edid_factorytest");
   	}
	public static function sysinfo_get_channel_signal():Number
	{
	   return ExternalInterface.call("sysinfo_get_channel_signal");
   	}
	public static function sysinfo_itemlist_factorytest():Number
	{
	   return ExternalInterface.call("sysinfo_itemlist_factorytest");
   	}
	
	public static function sysinfo_version_result_factorytest(val:String)
	{
	 	return ExternalInterface.call("sysinfo_version_result_factorytest", val);
   	}
	public static function sysinfo_language_result_factorytest(val:String)
	{
		return ExternalInterface.call("sysinfo_language_result_factorytest", val);
   	}
	public static function sysinfo_test_edid():String
	{
		return ExternalInterface.call("sysinfo_test_edid");
   	}

	
	public static function getAutoplayEnable():Number
	{
	   return ExternalInterface.call("sys_getAutoplayEnable");
   	}
	public static function setAutoplayEnable(val:Number)
	{
	   ExternalInterface.call("sys_setAutoplayEnable", val);
   	}
	public static function getAutoplayHostAp():Number
	{
	   return ExternalInterface.call("sys_getAutoplayHostAp");
   	}
	public static function setAutoplayHostAp(val:Number)
	{
	   ExternalInterface.call("sys_setAutoplayHostAp", val);
   	}
	public static function getAutoplayProgressive():Number
	{
	   return ExternalInterface.call("sys_getAutoplayProgressive");
   	}
	public static function setAutoplayProgressive(val:Number)
	{
	   ExternalInterface.call("sys_setAutoplayProgressive", val);
   	}
	public static function getAutoplayPlaylist():Number
	{
	   return ExternalInterface.call("sys_getAutoplayPlaylist");
   	}
	public static function setAutoplayPlaylist(val:Number)
	{
	   ExternalInterface.call("sys_setAutoplayPlaylist", val);
   	}
	public static function getAutoplayWaitime():Number
	{
	   return ExternalInterface.call("sys_getAutoplayWaitime");
   	}
	public static function setAutoplayWaitime(val:Number)
	{
	   ExternalInterface.call("sys_setAutoplayWaitime", val);
   	}
	public static function getAutoplayVolume():Number
	{
	   return ExternalInterface.call("sys_getAutoplayVolume");
   	}
	public static function setAutoplayVolume(val:Number)
	{
	   ExternalInterface.call("sys_setAutoplayVolume", val);
   	}
	
	public static function getStandbyTime():Number
	{
	   return ExternalInterface.call("sys_get_config", "standby time");
   	}
	public static function setStandbyTime(val:Number)
	{
	   ExternalInterface.call("sys_set_config", "standby time", val);
   	}
	
	// This function is only storage the config, and do not set the real function;
	// The real function is setted at NetDisplay start function.
	// val is boolean, 0 is false, else is true.
	public static function setNeighbour(val:Number)
	{
	   ExternalInterface.call("sys_set_config", "Neighbour", !!val);
   	}
	public static function getNeighbour():Number
	{
	   return ExternalInterface.call("sys_get_config", "Neighbour");
   	}
	/**
	 * @brief	set password hide status, and it will set schema.
	 * @param[in] op:		bool:	boolean value, 0/1 -> show/hide
 	 * @return: 			NULL
	 */
	public static function setPskHideStatus(bool:Number)
	{
		 	ExternalInterface.call("sys_set_config", "PSK_hide", !!bool);
		 
	}
	public static function getPskHideStatus():Number
	{
	   return ExternalInterface.call("sys_get_config", "PSK_hide");
   	}
	public static function ezwireGetStatus():Number
	{
	   return ExternalInterface.call("sys_get_ezwire_status");
   	}
	/**
	* @brief Get the status that forbid setting function.
	* @return:	true: forbid	false: allow
	*/
	public static function isSettingForbid():Boolean
	{
	   return (ExternalInterface.call("sys_get_config", "airoptions")==0)?true:false;
   	}
	
	public static function getWifidisplayStatus():Number
	{
	   return ExternalInterface.call("sys_get_wifidisplay_status");
   	}
	// WARN: The Blue LED is controlled by GPIO35, and it can not be set if function of MAC is used.
	public static function setLEDBlueOff()
	{
	   ExternalInterface.call("sys_set_LED", 0, 1);
   	}
	public static function setLEDGreenOff()
	{
	   ExternalInterface.call("sys_set_LED", 1, 1);
   	}
	// WARN: The Blue LED is controlled by GPIO35, and it can not be set if function of MAC is used.
	public static function setLEDBlueOn()
	{
	   ExternalInterface.call("sys_set_LED", 0, 0);
   	}
	public static function setLEDGreenOn()
	{
	   ExternalInterface.call("sys_set_LED", 1, 0);
   	}
//for WiFi Enterprise
	public static function wifi_set_802XEPA_conf(buf:String,flag:Number):Number
	{
		return  ExternalInterface.call("wifi_set_802XEPA_conf", buf,flag);
	}
	public static function wifi_full_802XEPA_conf(val:Number):Number
	{
		return  ExternalInterface.call("wifi_full_802XEPA_conf",val);
	}
//for WiFi Enterprise end   
	public static function save_outputmode(outputmode:Number,outputformat:Number)
	{
		 	 ExternalInterface.call("sysinfo_save_outputmode",outputmode,outputformat);
		 
	}
	public static function get_outputmode(outputflag:Number):Number
	{
		 	return  ExternalInterface.call("sysinfo_get_outputmode",outputflag);
		 
	}
   public static function reset_userpassword():Number
	{
		 	return  ExternalInterface.call("sys_reset_userpassword");
		 
	}
	 public static function get_miracode(code:String):String
	{
		 	return  ExternalInterface.call("sys_get_miracode",code);
		 
	}
	 public static function get_miracode_enable():Number
	{
		 	return  ExternalInterface.call("sys_get_miracode_enable");
		 
	}
	 public static function get_24g_or_5g():Number
	{
		 	return  ExternalInterface.call("sys_get_24g_or_5g");
		 
	}
	public static function checkSwfFileExist(name:String):Number
	{
		return  ExternalInterface.call("sys_swf_file_exist",name);
	  
	}
	public static function getEZWireIp4Mobile():String
	{
		return	ExternalInterface.call("sys_get_ezwire_ip_mobile");
	}
	public static function setEzairMode(val:Number)
	{
		ExternalInterface.call("sys_set_ezair_mode", val);
	}
	public static function getEzairMode():Number
	{
		return ExternalInterface.call("sys_get_ezair_mode", val);
	}
	public static function customerswf_file_exist(name:String):Number
	{
		return  ExternalInterface.call("sys_customerswf_file_exist",name);
	  
	}
	public static function aoaDisable(val:Number):Number
	{
		return ExternalInterface.call("sys_aoa_disable", val);
	}
	public static function isQCMode():Boolean
	{
		return (ExternalInterface.call("sys_isQCMode") == 0)?false:true;
	}

	public static function proBox_wifi_subdisplay_end()
	{
		return ExternalInterface.call("sys_proBox_wifi_subdisplay_end");
	}

	public static function getWirePlugMode():Boolean
	{
		return (ExternalInterface.call("sys_getWirePlugMode") == 0)?false:true;
	}
	public static function setWirePlugMode(val:Boolean)
	{
		ExternalInterface.call("sys_setWirePlugMode", val?1:0);
	}
	public static function PlugPlay(val:Boolean)
	{
		ExternalInterface.call("sys_PlugPlay", val?1:0);
	}
	
}	
