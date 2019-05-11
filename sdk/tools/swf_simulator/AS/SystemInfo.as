
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
	
	/**clock mode:12*/
	public static var SYS_CLOCKMODE_12 = 0;
	/**clock mode:24*/
	public static var SYS_CLOCKMODE_24 = 1;
	/**alarm ring 1*/
	public static var SYS_ALARMRING_1 = 0;
	/**alarm ring 2*/
	public static var SYS_ALARMRING_2 = 1;
	/**alarm ring 3*/
	public static var SYS_ALARMRING_3 = 2;
	/**store media:internal memory*/
	public static var SYS_MEDIA_INTERNAL = 0;
	/**store media:SD/MMC/MS/XD card*/
	public static var SYS_MEDIA_SD = 1;
	/**store media:CF card*/
	public static var SYS_MEDIA_CF = 2;
	/**store media:Usb mass storage media*/
	public static var SYS_MEDIA_UDISK = 3;
	
	

    /**
	 *  set system volume
	 * @param 
	 newVolume range is 0~31
	 * @return 
	 new volume if success or <0
	 */
	public static function setVolume(newVolume:Number):Number
	{
		return ExternalInterface.call("sys_setVolume", newVolume);
	}
    
    /**
	 *  get system volume
	 * @return 
	 	volume (0~31), <0:failed
	 */
	public static function getVolume():Number
	{
		return ExternalInterface.call("sys_getVolume");
	}
    
    /**
	 *  get firmware version
	 * @return 
	 	version string
	 */
	public static function getFwVersion():String
	{
		return ExternalInterface.call("sys_getFwVersion");
	}
	
	 /**
	 *  get internal storage total capacity(kbytes)
	 * @return 
	 total capacity if success or <0 failed
	 */
	public static function getDiskSpace():Number
	{
		return ExternalInterface.call("sys_getDiskSpace");
	}
	
	 /**
	 *  get internal storage free capacity(kbytes)
	 * @return 
	 free capacity if success or <0 failed
	 */
	public static function getDiskSpaceLeft():String
	{
		return ExternalInterface.call("sys_getDiskSpaceLeft");
	}
	
	/**
	 *  get card storage total capacity(kbytes)
	 * @return 
	 total capacity if success or <0 failed
	 */
	public static function getCardSpace():String
	{
		return ExternalInterface.call("sys_getCardSpace");
	}
	
	/**
	 *  get card storage free capacity(kbytes)
	 * @return 
	 free capacity if success or <0 failed
	 */
	public static function getCardSpaceLeft():String
	{
		return ExternalInterface.call("sys_getCardSpaceLeft");
	}
	
	 /**
	 *  get CF card storage total capacity(kbytes)
	 * @return 
	 total capacity if success or <0 failed
	 */
	public static function getCFSpace():String
	{
		return ExternalInterface.call("sys_getCFSpace");
	}
	
	/**
	 *  get CF card storage free capacity(kbytes)
	 * @return 
	 free capacity if success or <0 failed
	 */
	public static function getCFSpaceLeft():String
	{
		return ExternalInterface.call("sys_getCFSpaceLeft");
	}
	
	/**
	 *  get usb mass storage media total capacity(kbytes)
	 * @return 
	 total capacity if success or <0 failed
	 */
	public static function getUSBSpace():String
	{
		return ExternalInterface.call("sys_getUSBSpace");
	}
	
	/**
	 *  get usb mass storage media free capacity(kbytes)
	 * @return 
	 free capacity if success or <0 failed
	 */
	public static function getUSBSpaceLeft():String
	{
		return ExternalInterface.call("sys_getUSBSpaceLeft");
	}	
	
	/**
	 * set backlight strength
	 * @param 
	 strength to be set
	 * @return 
	 true if success or false
	 */
	public static function setBackLightStrength(strength:Number):Boolean
	{
		return ExternalInterface.call("sys_setBackLightStrength", strength);
	}
	
	/**
	 * get backlight strength
	 * @return 
	 back light strenght if success of <0 failed
	 */
	public static function getBackLightStrength():Number
	{
		return ExternalInterface.call("sys_getBackLightStrength");
	}
	
	/**
	 *  get current time: year
	 * @return
	 year if success or <0 failed
	 */
	public static function getYear():Number
	{
		return ExternalInterface.call("sys_getYear");
	}
	
	/**
	 *  set current: year
	 * @param  
	 newVal year to be set
	 * @return 
	 true if success or false
	 */
	public static function setYear(newVal:Number):Boolean
	{
		return ExternalInterface.call("sys_setYear", newVal);
	}
	
	/**
	 *  get current time: month
	 * @return
	 month if success or <0 failed
	 */
	public static function getMonth():Number
	{
		return ExternalInterface.call("sys_getMonth");
	}
	
	/**
	 *  set current: month
	 * @param  
	 newVal month to be set
	 * @return 
	 true if success or false
	 */
	public static function setMonth(newVal:Number):Boolean
	{
		 return ExternalInterface.call("sys_setMonth", newVal);
	}
	
	
	/**
	 *  get current time: day
	 * @return
	 day if success or <0 failed
	 */
	public static function getDay():Number
	{
		 return ExternalInterface.call("sys_getDay");
	}
	/**
	 *  set current: day
	 * @param  
	 newVal day to be set
	 * @return 
	 true if success or false
	 */
	public static function setDay(newVal:Number):Boolean
	{
		 return ExternalInterface.call("sys_setDay", newVal);
	}
	
	/**
	 *  get current time: hour
	 * @return
	 hour if success or <0 failed
	 */
	public static function getHour():Number
	{
		 return ExternalInterface.call("sys_getHour");
	}
	/**
	 *  set current: hour
	 * @param  
	 newVal hour to be set
	 * @return 
	 true if success or false
	 */
	public static function setHour(newVal:Number):Boolean
	{
		 return ExternalInterface.call("sys_setHour", newVal);
	}	
	
	/**
	 *  get current time: minute
	 * @return
	 minute if success or <0 failed
	 */
	public static function getMin():Number
	{
		 return ExternalInterface.call("sys_getMin");
	}
	/**
	 *  set current: minute
	 * @param  
	 newVal minute to be set
	 * @return 
	 true if success or false
	 */
	public static function setMin(newVal:Number):Boolean
	{
		 return ExternalInterface.call("sys_setMin", newVal);
	}		
	
	
	/**
	 *  get current time mode
	 * @return  	 
	 *@see SYS_CLOCKMODE_12
	 *@see SYS_CLOCKMODE_24
	 */
	public static function getClockMode():Number
	{
		 return ExternalInterface.call("sys_getClockMode");
	}
	/**
	 *  set time mode
	 * @param  
	 newVal new time mode
	 *@see SYS_CLOCKMODE_12
	 *@see SYS_CLOCKMODE_24	 
	 * @return 
	 true if success
	 
	 */
	public static function setClockMode(newVal:Number):Boolean
	{
		 return ExternalInterface.call("sys_setClockMode", newVal);
	}			
	
	/**
	 *  get alarm time: hour
	 * @return
	  alarm time hour or <0 if failed
	 */
	public static function getAlarmHour():Number
	{
		 return ExternalInterface.call("sys_getAlarmHour");
	}
	/**
	 *  set alarm time: houre
	 * @param  
	 newVal new hour
	 * @return 
	 true if success
	 */
	public static function setAlarmHour(newVal:Number):Boolean
	{
		 return ExternalInterface.call("sys_setAlarmHour", newVal);
	}		
	
	
	/**
	 *  get alarm time: minute
	 * @return
	  alarm time minute or <0 if failed
	 */
	public static function getAlarmMin():Number
	{
		 return ExternalInterface.call("sys_getAlarmMin");
	}
	/**
	 *  set alarm time: minute
	 * @param  
	 newVal new minute
	 * @return 
	 true if success
	 */
	public static function setAlarmMin(newVal:Number):Boolean
	{
		 return ExternalInterface.call("sys_setAlarmMin", newVal);
	}			
	
	
	
	/**
	 *  get alarm enable status
	 * @return  
	 */
	public static function getAlarmEnable():Number
	{
		 return ExternalInterface.call("sys_getAlarmEnable");
	}
	/**
	 * set alarm enable 
	 * @param  
	 newVal new alarm number
	 * @return 
	 	true if success or false
	 */
	public static function setAlarmEnable(newVal:Number):Boolean
	{
		 return ExternalInterface.call("sys_setAlarmEnable", newVal);
	}				
	
	
	/**
	 *  get alarm ring
	 *@return
	 *@see SYS_ALARMRING_1
	 *@see SYS_ALARMRING_2
	 *@see SYS_ALARMRING_3 	 
	 */
	public static function getAlarmRing():Number
	{
		 return ExternalInterface.call("sys_getAlarmRing");
	}
	/**
	 *  set alarm ring
	 *@param  
	 	newVal new alarm ring
	 *@see SYS_ALARMRING_1
	 *@see SYS_ALARMRING_2
	 *@see SYS_ALARMRING_3 	 
	 *@return 成功返回true
	 */
	public static function setAlarmRing(newVal:Number):Boolean
	{
		 return ExternalInterface.call("sys_setAlarmRing", newVal);
	}					
	
	
	/**
	 *  get current language identification
	 * @return 
	 current language id
	 */
	public static function getCurLanguage():Number
	{
	    return ExternalInterface.call("sys_getCurLanguage");
	}
	
	/**
	 *  set current language id
	 * @param 
	 newId the new language id
	 * @return 
	 	true if success or false
	 */
	public static function setCurLanguage(newId:Number):Boolean
	{
	    return ExternalInterface.call("sys_setCurLanguage", newId);
	}
	
	/**
	 *   check if card inserted
	 * @return 
	 	true: already insert \n
	 	false: not inserted
	 */
	public static function checkCardStatus():Boolean
	{
		return ExternalInterface.call("sys_checkCardStatus");
	}
	
	/**
	 *   check if CF card inserted
	 * @return 
	 	true: already insert \n
	 	false: not inserted
	 */
	public static function checkCFStatus():Boolean
	{
		return ExternalInterface.call("sys_checkCFStatus");
	}
	
	/**
	 *   check if USB inserted
	 * @return 
	 	true: already insert \n
	 	false: not inserted
	 */
	public static function checkUsbStatus():Boolean
	{
		return ExternalInterface.call("sys_checkUsbStatus");
	}
	
	/**
	 *   reset system to default setting
	 */
	public static function restoreSysDefaultConfig():Boolean
	{
	    return ExternalInterface.call("sys_resotreSysDefaultConfig");
	}
	
	/**
	 *  get media type from media number
	 *@param  
	 	newVal media type
	 *@see SYS_MEDIA_INTERNAL
	 *@see SYS_MEDIA_SD
	 *@see SYS_MEDIA_CF 	 
	 *@see SYS_MEDIA_UDISK 		 
	 * @return  
	 "C""D""E""F"......
	 */
	public static function media2Disk(newVal:Number):String
	{
		 return ExternalInterface.call("sys_media2Disk", newVal);
	}	
	
	/**
	 *  set the media to be active
	 *@param  media: media to set
	 *@see SYS_MEDIA_INTERNAL
	 *@see SYS_MEDIA_SD
	 *@see SYS_MEDIA_CF 	 
	 *@see SYS_MEDIA_UDISK 		 
	 * 
	 */
	public static function setActiveMedia(media:Number):Number
	{
		 return ExternalInterface.call("sys_setActiveMedia", media);
	}					
	
	/**
	 * get the working path, e.g. c:\ d:\ ...
	 *
	 * @return return the working path
	 */
	public static function getActiveWorkPath():String
	{
		return ExternalInterface.call("sys_getActiveWorkPath");
	}
	
};

 
