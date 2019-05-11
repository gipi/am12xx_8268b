
dynamic class Actions.OsdEngine {

 /**
 *@addtogroup OsdEngine_as
 *@{
 */
 
 	public static var GET_ICON_W=0;
 	public static var GET_ICON_H=1;
 	
 	public static var OSD_MODE=2;          /// for 4bits osd.
 	public static var OSD_MODE_2BITS=1;    /// for 2bits osd.
 	public static var OSD_MODE_8BITS=3;    /// for 8bits osd.

	/**
	* @brief enable the osd
	* @return 
    - 0 : failed
    - 1 : success
	*/
	public static function enableOsd():Number
	{
		return ExternalInterface.call("osdengine_enable");
	}	
	
	/**
	* @brief disable the osd
	* @return 
    - 0 : failed
    - 1 : success
	*/
	public static function disableOsd():Number
	{
		return ExternalInterface.call("osdengine_disable");
	}		
	
	/**
	* @brief init the osd
	* @param 
    - x : the x position that the osd will appeared on the screen.
    - y : the y position that the osd will appeared on the screen.
    - w : the width of the osd.
    - h : the height of the osd.
    - mode : currently only .
    - palette : the palette file's full path.
	* @return 
    - 0 : failed
    - 1 : success
	*/
	public static function initOsd(x:Number,y:Number,w:Number,h:Number,mode:Number,palette:String):Number
	{
		return ExternalInterface.call("osdengine_init",x,y,w,h,mode,palette);
	}	
	
	/**
	* @brief reverse operation of the "init". This should be called when you nolonger need this class.
	* @return 
    - 0 : failed
    - 1 : success
	*/
	public static function releaseOsd():Number
	{
		return ExternalInterface.call("osdengine_release");
	}	
	
	public static function prepareforreleaseOsd()
	{
		 ExternalInterface.call("osdengine_prepareforreleaseosd");
	}	
	/**
	* @brief Update a rectangle area when finish drawing which will make it to be seen.
	* @param 
    - x : the x position of the rect.
    - y : the y position of the rect.
    - w : the width of the rect.
    - h : the height of the rect.
	* @return none
	*/
	public static function updateOsdRect(x:Number,y:Number,w:Number,h:Number):Void
	{
		ExternalInterface.call("osdengine_updateRect",x,y,w,h);
	}
	
	
	/**
	* @brief Update the whole osd.
	* @return none
	*/
	public static function updateOsd():Void
	{
		ExternalInterface.call("osdengine_update");
	}												
	
	/**
	* @brief Clear a rectangle area.
	* @param 
    - x : the x position of the rect.
    - y : the y position of the rect.
    - w : the width of the rect.
    - h : the height of the rect.
	* @return none
	*/
	public static function clearOsdRect(x:Number,y:Number,w:Number,h:Number):Void
	{
		ExternalInterface.call("osdengine_clearRect",x,y,w,h);
	}
	
	/**
	* @brief Update the whole osd.
	* @return none
	*/
	public static function clearOsd():Void
	{
		ExternalInterface.call("osdengine_clear");
	}	
	
	/**
	* @brief Draw an osd icon.
	* @param 
    - x : the x position that the icon will be draw.
    - y : the y position that the icon will be draw.
    - path : the icon full path.
    - updateNow : update to screen immediately or not.
	* @return 
    - 0 : failed
    - 1 : success
	*/
	public static function showOsdIcon(x:Number,y:Number,path:String,updateNow:Number):Number
	{
		return ExternalInterface.call("osdengine_showIcon",x,y,path,updateNow);
	}
	
	
	/**
	* @brief Draw an osd string.
	* @param 
    - x : the x position that the string will be draw.
    - y : the y position that the string will be draw.
    - xrange : the range of the string in x direction. If string is too long then some text will not be shown.
    - fontsize : the font size of the string.
    - theText : the string in UTF-8 format.
	* @return 
    - 0 : failed
    - 1 : success
	*/
	public static function showOsdString(x:Number,y:Number,xrange:Number,fontsize:Number,theText:String,fontcolor:Number):Number
	{
		return ExternalInterface.call("osdengine_showString",x,y,xrange,fontsize,theText,fontcolor);
	}
	
	/**
	* @brief Draw an osd string.
	* @param 
    - x : the x position that the string will be draw.
    - y : the y position that the string will be draw.
    - xrange : the range of the string in x direction. If string is too long then some text will not be shown.
    - fontsize : the font size of the string.
    - sheight : the string field height.
    - theText : the string in UTF-8 format.
	* @return 
    - 0 : failed
    - 1 : success
	*/
	public static function showOsdString2(x:Number,y:Number,xrange:Number,fontsize:Number,sheight:Number,theText:String,fontcolor:Number):Number
	{
		return ExternalInterface.call("osdengine_showString2",x,y,xrange,fontsize,sheight,theText,fontcolor);
	}
	
	/**
	* @brief Get some information for the icon.
	* @param 
    - path : the icon full path.
    - infotype : GET_ICON_W for getting icon width and GET_ICON_H for gettting icon height.
	* @return return the icon information.
	*/
	public static function getOsdIconInfo(path:String,infotype:Number):Number
	{
		return ExternalInterface.call("osdengine_getIconInfo",path,infotype);
	}

	/**
	* @brief Get the current x mouse.
	* @return return the x mouse.
	*/
	public static function getXMouse():Number
	{
		return ExternalInterface.call("osdengine_getXMouse");
	}

	/**
	* @brief Get the current y mouse.
	* @return return the y mouse.
	*/
	public static function getYMouse():Number
	{
		return ExternalInterface.call("osdengine_getYMouse");
	}

	/**
	* @brief Fill an osd rect with the specified color.
	* @param 
    - x : the x position of the rect.
    - y : the y position of the rect.
    - w : the width of the rect.
    - h : the height of the rect.
    - color: the color to be filled.
	* @return none
	*/
	public static function fillRect(x:Number,y:Number,w:Number,h:Number,color:Number):Void
	{
		ExternalInterface.call("osdengine_fillRect",x,y,w,h,color);
	}

/*********************************  for music osd  ********************************************/

public static function as_show_music_osd(x:Number,y:Number,w:Number,h:Number,mode:Number,palette:String):Number
{
	return ExternalInterface.call("osdengine_showmusic",x,y,w,h,mode,palette);
}

public static function as_disable_music_osd():Number
{
	return ExternalInterface.call("osdengine_disablemusic");
}

public static function as_music_control(as_cmd:Number):Number
{
	return ExternalInterface.call("osdengine_musiccontrol",as_cmd);
}

/******************************************************************************************/

}
