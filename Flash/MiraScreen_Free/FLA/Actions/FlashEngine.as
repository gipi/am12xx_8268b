
/**
 *	@author   wuxiaowen
 *	@version 1.0
 *	@date: 2009/11/16
 *	<pre>  
 *	FlashEngine Class
 		Flash play control 
 	</pre>
 *	<pre>
 		Example:
 *		FlashEngine.Play("c:\\a.swf",true,0,400,800,200,8);
 *		FlashEngine.Stop();
 *	</pre>
 */


dynamic class Actions.FlashEngine {

/**
*@addtogroup FlashEngine_as
*@{
*/

	/**
	@brief play independent flash
	@param[in] path		: path of flash
	@param[in] transparent	: true if background is transparent(only used in video mode)
	@param[in] x			: x coordinate
	@param[in] y			: y coordinate
	@param[in] width		: width of flash area
	@param[in] height 		: height of flash area
	@param[in] fps		: fps of the specific flash
	@return none
	 */
	public static function Play(path:String, transparent:Boolean, x:Number, y:Number, width:Number, height:Number, fps:Number)
	{
		return ExternalInterface.call("flash_Play",path,transparent,x,y,width,height,fps);
	}									
	
	/**
	@brief stop independent flash playing
	@param[in] active	: active previous SWF instance is active flag is true
	@return none
	 */
	public static function Stop(active:Boolean)
	{
		return ExternalInterface.call("flash_Stop",active);
	}
	
	/**
	@brief add region of interest, osd will display in these areas(only used in video mode)
	@param[in] x 		: x coordinate
	@param[in] y 		: y coordinate
	@param[in] width 	: width of flash area
	@param[in] height	: height of flash area
	@return roi id
	*/
	public static function AddROI(x:Number, y:Number, width:Number, height:Number):Number
	{
		return ExternalInterface.call("flash_AddROI",x,y,width,height);
	}
	
	/**
	@brief set region of interest, osd will display in these areas(only used in video mode)
	@param[in] id		: roi id
	@param[in] x		: x coordinate
	@param[in] y		: y coordinate
	@param[in] width 	: width of flash area
	@param[in] height	: height of flash area
	@return roi id
	 */
	public static function SetROI(id:Number, x:Number, y:Number, width:Number, height:Number):Number
	{
		return ExternalInterface.call("flash_SetROI",id,x,y,width,height);
	}
	
	/**
	@brief remove region of interest
	@param[in] id	: id of region of interst, see AddROI()
	@return true if successfully removed
	 */
	public static function RemoveROI(id:Number):Boolean
	{
		return ExternalInterface.call("flash_RemoveROI",id);
	}
	
	/**
	@brief enable flash player auto frame rate function
	@param[in] none
	@return none
	 */
	public static function autoFPSEn():Void
	{
		ExternalInterface.call("flash_AutoFPSEn");
	}
	
	/**
	@brief disable flash player auto frame rate function
	@param[in] none
	@return none
	 */
	public static function autoFPSDisable():Void
	{
		ExternalInterface.call("flash_AutoFPSDisable");
	}

	/**
	@brief set flash player frame rate
	@param[in] none
	@return none
	 */
	public static function setFrameRate(rate:Number):Void
	{
		ExternalInterface.call("flash_SetFrameRate",rate);
	}
	
	/**
	@brief check encrypt swf
	@param[in] file	: file the file to be check
	@return 
		- 1 for valid
		- 0 for invalid.
	 */
	public static function checkValidation(file:String):Number
	{
		return ExternalInterface.call("flash_CheckValidation",file);
	}
	/**
	@brief get videoOsdRelease_flag
	@return 
		 1 videoOSD Release
		 0 videoOSD exsit
	 */
	public static function get_videoOsdRelease_flag():Number
	{
		return ExternalInterface.call("flash_getexitflag");
	}
	

/**
 *@}
 */	

}
