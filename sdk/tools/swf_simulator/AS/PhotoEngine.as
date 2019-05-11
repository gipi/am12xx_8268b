
/**
 *	@author   wuxiaowen
 *	@version 1.0
 *  @date: 2009/06/29
 *	<pre>  
 *  PhotoEngine Class
 		phot play control
 	</pre>
 *  <pre>
 	Example1(for full screen display)：
 *		PhotoEngine.Open();	
 *		mc = createEmptyMovieClip("mc",1);	
 *		PhotoEngine.Attach(mc,800,600);
 *		PhotoEngine.Show("c:\\a.jpg",PhotoEngine.PE_RATIO_LETTER_BOX);
 *		when exit：
 *		PhotoEngine.Dettach(false);
 *		PhotoEngine.Close();
 *	</pre>
 */

 dynamic class Actions.PhotoEngine {
	 
	/**slider status:idle*/
	public static var PE_IDLE=0;
	/**slider status:play*/
	public static var PE_PLAY=1;
	/**slider status:pause*/
	public static var PE_PAUSE=2;

	/**display mode:full screen with no distortion*/
	public static var PE_RATIO_LETTER_BOX=0;
	/**display mode:full screen may be distorted*/
	public static var PE_RATIO_FULL_SCREEN=1;
	/**display mode:original size*/
	public static var PE_RATIO_ACTUAL_SIZE=2;
	/**display mode:zoom out*/
	public static var PE_MODE_ZOOMOUT=3;
	/**display mode:zoom in*/
	public static var PE_MODE_ZOOMIN=4;
	/**display mode:rotate left*/
	public static var PE_MODE_ROTATE_LEFT=5;
	/**display mode:rotate right*/
	public static var PE_MODE_ROTATE_RIGHT=6;
	/**display mode:move left*/
	public static var PE_MODE_MOVE_LEFT=7;
	/**display mode:move right*/
	public static var PE_MODE_MOVE_RIGHT=8;			
	/**display mode:move up*/
	public static var PE_MODE_MOVE_UP=9;
	/**display mode:move down*/
	public static var PE_MODE_MOVE_DOWN=10;
	/**display mode:resize photo from zoom status */
	public static var PE_RATIO_RESIZE=11;
	/**
	 * open PhotoEngine
	 *@param 
	 	path working path
	 *@return 
	 	true if success
	 */
	public static function Open(path:String):Boolean
	{
		return ExternalInterface.call("pe_Open",path);
	}
	
	/**
	 * attach photo to a specified MovieClip
	 *@param 
	 	target target MovieClip
	 *@param 
	 	width target width
	 *@param 
	 	height target height
	 *@return 
	 	true if success
	 */
	public static function Attach(target:MovieClip, width:Number, height:Number):Boolean
	{
		return ExternalInterface.call("pe_Attach",target,width,height);
	}
	
	/**
	 *detach the previously attached photo
	 *@param 
	  clone indication wether copy the attached photo to movieclip
	 *@return 
	 	true if success
	 */
	public static function Detach(clone:Boolean):Boolean
	{
		return ExternalInterface.call("pe_Detach", clone);
	}
	
	/**
	 * close PhotoEngine
	 */
	public static function Close()
	{
		return ExternalInterface.call("pe_Close");
	}
	
	/**
	 * show photo according to mode
	 *@param 
	 	path photo path
	 *@param 
	 	mode decode mode
	 *@see PE_RATIO_ZOOMOUT
	 *@see PE_RATIO_ZOOMIN
	 *@see PE_RATIO_LETTER_BOX
	 *@see PE_RATIO_FULL_SCREEN
	 *@see PE_RATIO_ACTUAL_SIZE
	 *@return 
	 	true if success
	 */
	public static function Show(path:String, mode:Number):Boolean
	{
		return ExternalInterface.call("pe_Show",path,mode);
	}
	
	/**
	 * start slider show
	 *@param 
	 	path slide show effect template directory or file
	 *@param 
	 	playmode should always be 0 for slide show
	 *@return 
	 	if success true or false
	 */
	public static function Play(path:String, playmode:Number):Boolean
	{
		/**
		* we use SWF file as slide show template, this is just the same
		* as vvd playing, that's why we redirect play command
		* 
		*/
		//return ExternalInterface.call("pe_Play",effect,interval);
		ExternalInterface.call("vvd_Open",path,playmode);
		ExternalInterface.call("vvd_Play",playmode);
		return true;
	}
	
	/**
	 *stop slide show
	 */
	public static function Stop()
	{
		/**
		* we use SWF file as slide show template, this is just the same
		* as vvd playing, that's why we redirect stop command
		* 
		*/
		//return ExternalInterface.call("pe_Stop");
		ExternalInterface.call("vvd_Stop");
		ExternalInterface.call("vvd_Close");
	}
	
	/**
	 *pause slide show
	 */
	public static function Pause()
	{
		return ExternalInterface.call("pe_Pause");
	}
	
	/**
	 *continue slide show if paused
	 */
	public static function Resume()
	{
		return ExternalInterface.call("pe_Resume");
	}
	
	/**
	 * get decode status
	 *@return 
	 	0  In decoding \
	 	other decode over
	 */
	public static function GetDecStatus():Number
	{
		return ExternalInterface.call("pe_GetStatus");
	}
	
	/**
	 * get slide status
	 *@return 
	 	slide status
	 *@see PE_IDLE
	 *@see PE_PLAY
	 *@see PE_PAUSE
	 */
	public static function GetSlideState():Number
	{
		return ExternalInterface.call("pe_GetState");
	}
	
	/**
	 *get display ratio
	 *@return 
	 	ratio
 	 *@see PE_RATIO_ZOOMOUT
	 *@see PE_RATIO_ZOOMIN
	 *@see PE_RATIO_LETTER_BOX
	 *@see PE_RATIO_FULL_SCREEN
	 *@see PE_RATIO_ACTUAL_SIZE
	 */
	public static function GetPlayRatio():Number
	{
		return ExternalInterface.call("pe_GetPlayRatio");
	}
	
	/**
	 *set display ratio
	 *@return 
	 	display ratio
 	 *@see PE_RATIO_ZOOMOUT
	 *@see PE_RATIO_ZOOMIN
	 *@see PE_RATIO_LETTER_BOX
	 *@see PE_RATIO_FULL_SCREEN
	 *@see PE_RATIO_ACTUAL_SIZE
	 */
	public static function SetPlayRatio(mode:Number):Number
	{
		return ExternalInterface.call("pe_SetPlayRatio",mode);
	}		
	
	
	/**
	 *get interval time
	 *@return 
	 	interval time
	 */
	public static function GetSlideShowTime():Number
	{
		return ExternalInterface.call("pe_GetSlideShowTime");
	}
	
	/**
	 * set slide interval time
	 *@param 
	 	time time to set
	 */
	public static function SetSlideShowTime(time:Number):Number
	{
		return ExternalInterface.call("pe_SetSlideShowTime",time);
	}			
	
	
	
	/**
	 *get slide show effect
	 *@return 
	 	effect 
	 */
	public static function GetSlideShowEffect():Number
	{
		return ExternalInterface.call("pe_GetSlideShowEffect");
	}
	
	/**
	 *set slide effect
	 *@param 
	 	effect new effect to be set
	 */
	public static function SetSlideShowEffect(effect:Number):Number
	{
		return ExternalInterface.call("pe_SetSlideShowEffect",effect);
	}				
	
	/**
	 * get if clock show enable
	 *@return 
	 	0 not show clock\n
	 	1 show clock
	 */
	public static function GetClockEnable():Number
	{
		return ExternalInterface.call("pe_GetClockEnable");
	}
	
	/**
	 * set clock show enable
	 *@param 
	 	enable 0:disable clock show 1:enable clock show
	 	
	 */
	public static function SetClockEnable(enable:Number):Number
	{
		return ExternalInterface.call("pe_SetClockEnable",enable);
	}	
 }
 
