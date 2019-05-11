
/**
 *	@author   wuxiaowen
 *	@version 1.0
 *  @date: 2009/06/08
 *	<pre>  
 *  VideoEngine 
 		video play control 
 *	</pre>
 *  <pre>
 	Example:
 *		VideoEngine.Open();	
 *		VideoEngine.SetFile("c:\\a.avi");
 *		VideoEngine.AttachPicture(mc,400,300);
 *		VideoEngine.Play(VideoEngine.VE_PLAYMODE_ONCE, VideoEngine.VE_RATIO_LETTER_BOX);
 *		when exit:
 *		VideoEngine.Close();
 *	</pre>
 */

 dynamic class Actions.VideoEngine {
	 
	/**play status:idle*/
	public static var VE_IDLE=0;
	/**play status:play*/
	public static var VE_PLAY=1;
	/**play status:pause*/
	public static var VE_PAUSE=2;
	/**play status:fast forward*/
	public static var VE_FF=3;
	/**play status:fast backward*/
	public static var VE_FB=4;
	/**play status:stop*/
	public static var VE_STOP=5;
	
	/**display ratio: full screen with no distortion*/
	public static var VE_RATIO_LETTER_BOX = 2;
	/**display ratio: full screen with distortion*/
	public static var VE_RATIO_FULL_SCREEN = 3;
	/**display ratio: original size*/
	public static var VE_RATIO_ACTUAL_SIZE = 4;
	
	/**play mode:once*/
	public static var VE_PLAYMODE_ONCE
	/**play mode:repeated*/
	public static var VE_PLAYMODE_REPEAT

	/**
	 *open VideoEngine
	 *@return 
	 	true if success or false
	 */
	public static function Open():Boolean
	{
		return ExternalInterface.call("ve_Open");
	}
	
	/**
	 *close VideoEngine
	 */
	public static function Close()
	{
		return ExternalInterface.call("ve_Close");
	}
	
	/**
	 *Set the video file to be played
	 *@param path:Full path name
	 *@return  success:true fail:false
	 */
	public static function SetFile(path:String):Boolean
	{
		return ExternalInterface.call("ve_SetFile",path);
	} 
	
	/**
	 *attach the moview preview to pointed MovieClip
	 *@param 
	 	target destination MovieClip, usually created by createEmptyMovieClip
	 *@param 
	 	width destination width
	 *@param 
	 	height destination height
	 *@return
	 	true if success or false
	 */
	public static function AttachPicture(target:MovieClip, width:Number, height:Number):Boolean
	{
		return ExternalInterface.call("ve_Picture",target,width,height);
	}
	
	/**
	 * play the given video
	 *@param 
	 	mode play mode
	 *@see VE_PLAYMODE_ONCE
	 *@see VE_PLAYMODE_REPEAT
	 *@param 
	 	ratio display ratio
	 *@see VE_RATIO_LETTER_BOX
	 *@see VE_RATIO_FULL_SCREEN
	 *@see VE_RATIO_ACTUAL_SIZE
	 *@return
	 	true if success or false
	 */
	public static function Play(mode:Number,ratio:Number):Boolean
	{
		return ExternalInterface.call("ve_Play",mode,ratio);
	}
	
	/**
	 * stop play
	 */
	public static function Stop()
	{
		return ExternalInterface.call("ve_Stop");
	}
	
	/**
	 *pause
	 */
	public static function Pause()
	{
		return ExternalInterface.call("ve_Pause");
	}
	
	/**
	 * resume if paused
	 */
	public static function Resume()
	{
		return ExternalInterface.call("ve_Resume");
	}
	
	/**
	 * fast forward 
	 */
	public static function FF()
	{
		return ExternalInterface.call("ve_FF");
	}
	
	/**
	 *fast backward
	 */
	public static function FB()
	{
		return ExternalInterface.call("ve_FB");
	}
	
	/**
	 *get current play status
	 *@return
	  status
	 *@see VE_IDLE
	 *@see VE_PLAY
	 *@see VE_PAUSE
	 *@see VE_FF
	 *@see VE_FB
	 *@see VE_STOP
	 */
	public static function GetState():Number
	{
		return ExternalInterface.call("ve_State");
	}
	
	/**
	 * get total play time
	 *@return
	 	total time with resolution millisecond
	 */
	public static function GetTotalTime():Number
	{
		return ExternalInterface.call("ve_TotalTime");
	}
	
	/**
	 *get current play time
	 *@return
	 	current time with resolution millisecond
	 */
	public static function GetCurTime():Number
	{
		return ExternalInterface.call("ve_CurTime");
	}
	
	/**
	 *get play ratio
	 *@return 
	 	ratio
	 *@see VE_RATIO_LETTER_BOX
	 *@see VE_RATIO_FULL_SCREEN
	 *@see VE_RATIO_ACTUAL_SIZE
	 */
	public static function GetPlayRatio():Number
	{
		return ExternalInterface.call("ve_GetPlayRatio");
	}
	
	/**
	 *set display ratio
	 *@param 
	 ratio the ration to be set	 	
	 *@see VE_RATIO_LETTER_BOX
	 *@see VE_RATIO_FULL_SCREEN
	 *@see VE_RATIO_ACTUAL_SIZE
	 */
	public static function SetPlayRatio(ratio:Number):Number
	{
		return ExternalInterface.call("ve_SetPlayRatio",ratio);
	}		
	
	/**
	 *get play mode
	 *@return
	 	the play mode
	 *@see VE_PLAYMODE_ONCE
	 *@see VE_PLAYMODE_REPEAT
	 */
	public static function GetPlayMode():Number
	{
		return ExternalInterface.call("ve_GetPlayMode");
	}
	
	/**
	 * set play mode
	 *@param
	 	mode mode to be set 
	 *@see VE_PLAYMODE_ONCE
	 *@see VE_PLAYMODE_REPEAT
	 */
	public static function SetPlayMode(mode:Number):Number
	{
		return ExternalInterface.call("ve_SetPlayMode",mode);
	}
 }
 