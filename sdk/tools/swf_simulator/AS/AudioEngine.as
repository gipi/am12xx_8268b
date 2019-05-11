
/**
 *	@author   wuxiaowen
 *	@version 1.0
 *  @date: 2009/06/08
 *	<pre>   
 *  AudioEngine Class:
 		for music playback control
 *	Example:
 *		AudioEngine.Open();
 *		AudioEngine.SetFile("c:\\a.mp3");
 *		trace(AudioEngine.GetSinger());
 *		AudioEngine.Play();
 *		AudioEngine.Stop();
 *		AudioEngine.Close();
 *	</pre>
 */

 dynamic class Actions.AudioEngine {
	 
	/**play status:idle*/
	public static var AE_IDLE=0;
	/**play status:play*/
	public static var AE_PLAY=1;
	/**play status:pause*/
	public static var AE_PAUSE=2;
	/**play status:fast forward*/
	public static var AE_FF=3;
	/**play status:fast backward*/
	public static var AE_FB=4;
	/**play status:stop*/
	public static var AE_STOP=5;
	
	/**play mode:sigle*/
	public static var AE_PLAYMODE_ONCE = 0;
	/**play mode:repeat*/
	public static var AE_PLAYMODE_REPEAT = 1;
	
	/**sound effect:NORMAL*/
	public static var AE_EFFECT_NORMAL = 0;
	/**sound effect:DBB*/
	public static var AE_EFFECT_DBB = 1;
	/**sound effect:JAZZ*/
	public static var AE_EFFECT_JAZZ = 2;
	/**sound effect:SOFT*/
	public static var AE_EFFECT_SOFT = 3;
	/**sound effect:CLASSIC*/
	public static var AE_EFFECT_CLASSIC = 4;
	/**sound effect:POPULAR*/
	public static var AE_EFFECT_POP = 5;
	/**sound effect:ROCK*/
	public static var AE_EFFECT_ROCK = 6;
	 
	/**
	 *open AudioEngine
	 *@return 
	 	true if success \n
	 	false if fail
	 */
	public static function Open():Boolean
	{
		return ExternalInterface.call("ae_Open");
	}
	
	/**
	 * close AudioEngine
	 */
	public static function Close()
	{
		return ExternalInterface.call("ae_Close");
	}
	
	/**
	 * set file to be played 
	 *@param 
	 	path file path
	 *@return 
	 	true if success \n
	 	false if fail
	 */
	public static function SetFile(path:String):Boolean
	{
		return ExternalInterface.call("ae_SetFile",path);
	}
	
	/**
	 * play music
	 *@return 
	 	true if success or false
	 */
	public static function Play()
	{
		return ExternalInterface.call("ae_Play");
	}
	
	/**
	 *stop play current music
	 */
	public static function Stop()
	{
		return ExternalInterface.call("ae_Stop");
	}
	
	/**
	 *pause current music
	 */
	public static function Pause()
	{
		return ExternalInterface.call("ae_Pause");
	}
	
	/**
	 * replay current music if paused
	 */
	public static function Resume()
	{
		return ExternalInterface.call("ae_Resume");
	}
	
	/**
	 *fast forward play
	 */
	public static function FF()
	{
		return ExternalInterface.call("ae_FF");
	}
	
	/**
	 *fast backward play
	 */
	public static function FB()
	{
		return ExternalInterface.call("ae_FB");
	}
	
	/**
	 *get music player status
	 *@return 
	 	the player status
	 *@see AE_IDLE
	 *@see AE_PLAY
	 *@see AE_PAUSE
	 *@see AE_FF
	 *@see AE_FB
	 *@see AE_STOP
	 */
	public static function GetState():Number
	{
		return ExternalInterface.call("ae_State");
	}
	
	/**
	 *get the total time
	 *@return 
	 	total time with resolution millisecond
	 */
	public static function GetTotalTime():Number
	{
		return ExternalInterface.call("ae_TotalTime");
	}
	
	/**
	 *get the current play time
	 *@return 
	 	time eclipsed
	 */
	public static function GetCurTime():Number
	{
		return ExternalInterface.call("ae_CurTime");
	}
	
	/**
	 *get the singer name
	 *@return 
	   singer name with utf-8 format
	 */
	public static function GetSinger():String
	{
		return ExternalInterface.call("ae_Singer");
	}
	
	/**
	 *get the album name
	 *@return 
	 album name with utf-8 format
	 */
	public static function GetAlbum():Number
	{
		return ExternalInterface.call("ae_Album");
	}
	
	/**
	 * attach the image embeded in the ID3 information to a movieclip
	 *@param 
	 target the MovieClip to be attached to, usually created by createEmptyMovieClip
	 *@param 
	 width image width
	 *@param 
	 height image height
	 *@return 
	 	true if success
	 */
	public static function AttachPicture(target:MovieClip, width:Number, height:Number):Boolean
	{
		return ExternalInterface.call("ae_Picture",target,width,height);
	}
	
	/**
	 *get music type
	 *@return 
	 	music type , i.e. mp3,wma,mav,agg,aac,ac3 etc.
	 */
	public static function GetExtention():String
	{
		return ExternalInterface.call("ae_Extention");
	}
	
	/**
	 *get the sample rate
	 *@return 
	 	sample rate
	 */
	public static function GetSampleRate():Number
	{
		return ExternalInterface.call("ae_SampleRate");
	}
	
	/**
	 *get the lyrics
	 *@return 
	 	lyrics string
	 */
	public static function GetLyrics():String
	{
		return ExternalInterface.call("ae_Lyrics");
	}

	
	/**
	 *get sound effect
	 *@return 
	 	sound effect
	 *@see AE_EFFECT_NORMAL
	 *@see AE_EFFECT_DBB
	 *@see AE_EFFECT_JAZZ 
	 *@see AE_EFFECT_SOFT
	 *@see AE_EFFECT_CLASSIC
	 *@see AE_EFFECT_POP 	
	 *@see AE_EFFECT_ROCK	 
	 */
	public static function GetEffect():Number
	{
		return ExternalInterface.call("ae_GetEffect");
	}
	
	/**
	 *set sound effect
	 *@param 
	 	effect to be set
	 *@see AE_EFFECT_NORMAL
	 *@see AE_EFFECT_DBB
	 *@see AE_EFFECT_JAZZ 
	 *@see AE_EFFECT_SOFT
	 *@see AE_EFFECT_CLASSIC
	 *@see AE_EFFECT_POP 	
	 *@see AE_EFFECT_ROCK		 
	 */
	public static function SetEffect(effect:Number):Number
	{
		return ExternalInterface.call("ae_SetEffect",effect);
	}					
		
	/**
	 *get music play mode
	 *@return 
	 	current music play mode
	 *@see AE_PLAYMODE_ONCE
	 *@see AE_PLAYMODE_REPEAT
	 */
	public static function GetPlayMode():Number
	{
		return ExternalInterface.call("ae_GetPlayMode");
	}
	
	/**
	 *set music play mode
	 *@param 
	 	mode to be set
	 *@see AE_PLAYMODE_ONCE
	 *@see AE_PLAYMODE_REPEAT
	 */
	public static function SetPlayMode(mode:Number):Number
	{
		return ExternalInterface.call("ae_SetPlayMode",mode);
	}							
	
 }
 