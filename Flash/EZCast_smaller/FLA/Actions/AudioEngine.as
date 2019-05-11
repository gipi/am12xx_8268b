
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

/**
*@addtogroup AudioEngine_as
*@{
*/
	 
	public static var AE_IDLE=0;		///< play status:idle
	public static var AE_PLAY=1;		///< play status:play
	public static var AE_PAUSE=2;		///< play status:pause
	public static var AE_FF=3;			///< play status:fast forward
	public static var AE_FB=4;			///< play status:fast backward
	public static var AE_STOP=5;		///< play status:stop

	
	public static var AE_PLAYMODE_REPEAT_SEQUENCE = 0;		///< play mode:repeat sequence
	public static var AE_PLAYMODE_REPEATONE = 1;			///< play mode:sigle loop
	public static var AE_PLAYMODE_REPEAT_RANDOM = 2;		///< play mode:repeat random
	public static var AE_PLAYMODE_ONCE = 3;					///< play mode:only once
	public static var AE_PLAYMODE_INVALID = 4;				///< play mode boundary

	
	
	public static var AE_EFFECT_NORMAL = 0;		///< sound effect:NORMAL
	public static var AE_EFFECT_DBB = 1;		///< sound effect:DBB
	public static var AE_EFFECT_JAZZ = 2;		///< sound effect:JAZZ
	public static var AE_EFFECT_SOFT = 3;		///< sound effect:SOFT
	public static var AE_EFFECT_CLASSIC = 4;	///< sound effect:CLASSIC
	public static var AE_EFFECT_POP = 5;		///< sound effect:POPULAR
	public static var AE_EFFECT_ROCK = 6;		///< sound effect:ROCK


	/**audio player error msg**/
	public static var AE_ERR_NO_ERROR = 0;		///< audio player no error
	public static var AE_ERR_OPEN_FILE=1;		///< audio player open file error
	public static var AE_ERR_FILE_NOT_SUPPORT=2;///< audio player file not support
	public static var AE_ERR_DECODER_ERROR=3;	///< audio player decoder error
	public static var AE_ERR_NO_LICENSE=4;		///< audio player no license
	public static var AE_ERR_SECURE_CLOCK=5;	///< audio player secure clock
	public static var AE_ERR_LICENSE_INFO=6;	///< audio player license information
	public static var AE_ERR_OTHER=7;			///< audio player other error


	/**Id3 information cmd**/
	public static var AE_ID3_AUTHOR=1;		///< Id3 author information
	public static var AE_ID3_COMPOSER=2;	///< Id3 composer information
	public static var AE_ID3_ALBUM=4;		///< Id3 album information
	public static var AE_ID3_GENRE=8;		///< Id3 genre information
	public static var AE_ID3_YEAR=16;		///< Id3 year information
	public static var AE_ID3_PIC_WH=32;		///< Id3 picture information width*height
	/**
	 *@brief	open AudioEngine
	 *@param[in] NULL
	 *@return 
	 *		true if success or false if fail
	 */
	public static function Open():Boolean
	{
		return ExternalInterface.call("ae_Open");
	}
	
	/**
	 *@brief	close AudioEngine
	 *@param[in] NULL
	 *@return NULL
	 */
	public static function Close()
	{
		return ExternalInterface.call("ae_Close");
	}
	
	/**
	 *@brief	set file to be played 
	 *@param[in] path	: file path
	 *@return 
	 *		AE_ERR_NO_ERROR etc
	 */
	public static function SetFile(path:String):Boolean
	{
		return ExternalInterface.call("ae_SetFile",path);
	}
	
	/**
	 *@brief	play music
	 *@param[in] NULL
	 *@return NULL
	 */
	public static function Play()
	{
		return ExternalInterface.call("ae_Play");
	}
	
	/**
	 *@brief	stop play current music
	 *@param[in] NULL
	 *@return NULL
	 */
	public static function Stop()
	{
		return ExternalInterface.call("ae_Stop");
	}
	
	/**
	 *@brief	pause current music
	 *@param[in] NULL
	 *@return NULL
	 */
	public static function Pause()
	{
		return ExternalInterface.call("ae_Pause");
	}
	
	/**
	 *@brief	replay current music if paused
	 *@param[in] NULL
	 *@return NULL
	 */
	public static function Resume()
	{
		return ExternalInterface.call("ae_Resume");
	}
	
	/**
	 *@brief	fast forward play
	 *@param[in] NULL
	 *@return NULL
	 */
	public static function FF()
	{
		return ExternalInterface.call("ae_FF");
	}
	
	/**
	 *@brief	fast backward play
	 *@param[in] NULL
	 *@return NULL
	 */
	public static function FB()
	{
		return ExternalInterface.call("ae_FB");
	}

	/**
	 *@brief	seek play
	 *@param[in] seektime	: seek point to be set,abs time in this audio
	 *@return NULL
	 */
	public static function SeekPlay(seektime:Number)
	{
		return ExternalInterface.call("ae_seekplay",seektime);
	}
	/**
	 *@brief	get music player status
	 *@param[in] NULL
	 *@return 
	 *		the player status
	 *			- AE_IDLE
	 *			- AE_PLAY
	 *			- AE_PAUSE
	 *			- AE_FF
	 *			- AE_FB
	 *			- AE_STOP
	 */
	public static function GetState():Number
	{
		return ExternalInterface.call("ae_State");
	}
	
	/**
	 *@brief	get the total time
	 *@param[in] NULL
	 *@return 
	 *		total time with resolution millisecond //richard :resolution is second!!
	 */
	public static function GetTotalTime():Number
	{
		return ExternalInterface.call("ae_TotalTime");
	}
	
	/**
	 *@brief	get the current play time
	 *@param[in] NULL
	 *@return 
	 *		time eclipsed
	 */
	public static function GetCurTime():Number
	{
		return ExternalInterface.call("ae_CurTime");
	}
	
	/**
	 *@brief	get the singer name
	 *@param[in] NULL
	 *@return 
	 *		singer name with utf-8 format
	 */
	public static function GetSinger():String
	{
		return ExternalInterface.call("ae_Singer");
	}
	
	/**
	 *@brief	get the album name
	 *@param[in] NULL
	 *@return 
	 *		album name with utf-8 format
	 */
	public static function GetAlbum():String
	{
		return ExternalInterface.call("ae_Album");
	}
	
	/**
	 *@brief	attach the image embeded in the ID3 information to a movieclip
	 *@param[in] target	: the MovieClip to be attached to, usually created by createEmptyMovieClip
	 *@param[in] width	: image width
	 *@param[in] height	: image height
	 *@return NULL
	 */
	public static function AttachPicture(target:MovieClip, width:Number, height:Number):Boolean
	{
		return ExternalInterface.call("ae_Picture",target,width,height);
	}
	
	/**
	 *@brief	get music type
	 *@param[in] NULL
	 *@return 
	 *		music type , i.e. mp3,wma,mav,agg,aac,ac3 etc.
	 */
	public static function GetExtention():String
	{
		return ExternalInterface.call("ae_Extention");
	}
	
	/**
	 *@brief	get the sample rate
	 *@param[in] NULL
	 *@return 
	 *		sample rate
	 */
	public static function GetSampleRate():Number
	{
		return ExternalInterface.call("ae_SampleRate");
	}
	
	/**
	 *@brief	get the bitrate
	 *@param[in] NULL
	 *@return 
	 *		music bit rate
	 */
	public static function GetBitRate():Number
	{
		return ExternalInterface.call("ae_BitRate");
	}

	/**
	 *@brief	get the channels
	 *@param[in] NULL
	 *@return 
	 *		music channels
	 */
	public static function GetChannels():Number
	{
		return ExternalInterface.call("ae_Channels");
	}
	
	/**
	 *@brief	get sound effect
	 *@param[in] NULL
	 *@return sound effect
	 *			- AE_EFFECT_NORMAL
	 *			- AE_EFFECT_DBB
	 *			- AE_EFFECT_JAZZ 
	 *			- AE_EFFECT_SOFT
	 *			- AE_EFFECT_CLASSIC
	 *			- AE_EFFECT_POP 	
	 *			- AE_EFFECT_ROCK	 
	 */
	public static function GetEffect():Number
	{
		return ExternalInterface.call("ae_GetEffect");
	}
	
	/**
	 *@brief	set sound effect
	 *@param[in] effect	: effect to be set
	 *			- AE_EFFECT_NORMAL
	 *			- AE_EFFECT_DBB
	 *			- AE_EFFECT_JAZZ 
	 *			- AE_EFFECT_SOFT
	 *			- AE_EFFECT_CLASSIC
	 *			- AE_EFFECT_POP 	
	 *			- AE_EFFECT_ROCK
	 *@return NULL
	 */
	public static function SetEffect(effect:Number):Number
	{
		return ExternalInterface.call("ae_SetEffect",effect);
	}					
		
	/**
	 *@brief	get music play mode
	 *@param[in] NULL
	 *@return current music play mode
	 *			- AE_PLAYMODE_REPEAT_SEQUENCE = 0;		///< play mode:repeat sequence
	 *			- AE_PLAYMODE_REPEATONE = 1;			///< play mode:sigle loop
	 *			- AE_PLAYMODE_REPEAT_RANDOM = 2;		///< play mode:repeat random
	 *			- AE_PLAYMODE_ONCE = 3;	
	 */
	public static function GetPlayMode():Number
	{
		return ExternalInterface.call("ae_GetPlayMode");
	}
	
	/**
	 *@brief	set music play mode
	 *@param[in] mode	: to be set
	 *			- AE_PLAYMODE_REPEAT_SEQUENCE = 0;		///< play mode:repeat sequence
	 *			- AE_PLAYMODE_REPEATONE = 1;			///< play mode:sigle loop
	 *			- AE_PLAYMODE_REPEAT_RANDOM = 2;		///< play mode:repeat random
	 *			- AE_PLAYMODE_ONCE = 3;	
	 *@return NULL
	 */
	public static function SetPlayMode(mode:Number):Number
	{
		return ExternalInterface.call("ae_SetPlayMode",mode);
	}	

	/**
	 *Lyric  function
	 *@brief	open a lyric file
	 *@param[in] filename	: the file to be opened
	 *@return
	 *		the handle of the file
	 */
	public static function openLyricFile(filename:String):Number
	{
		return ExternalInterface.call("ae_openLyricFile",filename);
	}
	
	/**
	 *@brief	close the lyric file that had been opened
	 *@param[in] handle	: the handle associated with the file
	 *@return NULL
	 */
	public static function closeLyricFile(handle:Number)
	{
		return ExternalInterface.call("ae_closeLyricFile",handle);
	}
	/**
	 *@brief	get  the time tag of current line
	 *@param[in] NULL
	 *@return
	 *		time tag, the units is second
	 */
	public static function getTagTime():Number
	{
		return ExternalInterface.call("ae_getTagTime");
	}
	/**
	 *@brief	get  the content of current line
	 *@param[in] NULL
	 *@return
	 *		the content of current line
	 */
	public static function getLyric():String
	{
		return ExternalInterface.call("ae_getLyric");
	}

	/**
	 *@brief	change the line index of lyric
	 *@param[in] NULL
	 *@return
	 * - -1 : the buffer is not enough
	 * - -2 : the last line
	 * - others: the length of the content
	 */
	public static function getLyricNextLine():Number
	{
		return ExternalInterface.call("ae_getLyricNextLine");
	}
	
	/**
	 *@brief	get the infomation of lyric file 
	 *@param[in] LYRIC_TAG_AL=0, etc
	 *@return
	 *		the information of the index 
	 */
	public static function getLyricInfo(index:Number):String
	{
		return ExternalInterface.call("ae_getLyricInfo",index);
	}

	/**
	 *@brief	change the lyric line according the current time
	 *@param[in] timetag	: units is second
	 *@return
	 * - 0 : failed
	 * - 1 : succeed
	 */
	public static function changeLyricLine(timetag:Number):Number
	{
		return ExternalInterface.call("ae_changeLyricLine",timetag);
	}

	/**
	 *@brief	get id3 infomation of the specified file
	 *@param[in] filepath	: the file will be extracted
	 *@param[in] cmd		: AE_ID3_AUTHOR etc
	 *@return
	 *	the information matched with the cmd,
	 	- if the cmd==AE_ID3_PIC_WH, it will return the string of width*height, user must parser the string to get the width and the height
	 	for attching a movie clip to the picture of audio file
	 */ 
	public static function getId3Info(filepath:String,cmd:Number):String
	{
		return ExternalInterface.call("ae_GetID3Info",filepath,cmd);
	}

	/**
	@brief attach the picture in the audio file to a movieclip
	@param[in] filepath	: the audio file 
	@param[in] target		: the movieclip which will be attached to
	@param[in] target_w	: the width of movieclip 
	@param[in] target_h	: the height of movieclip
	@return
		- 0 		: faild to get the picture 
		- others	: the handle to the pic, which will be used when calling detachId3Pic function
	@see detachId3Pic
	**/
	public static function attachId3Pic(filepath:String,target:MovieClip,target_w:Number,target_h:Number):Number
	{
		return ExternalInterface.call("ae_AttachID3Pic",filepath,target,target_w,target_h);
	}


	/**
	@brief detach the picture from the movieclip
	@param[in] picHandle	: the handle which get from calling attachId3Pic
	@return 
		- 0 : failed
		- 1 : succeed
	**/
	public static function detachId3Pic(picHandle:Number):Number
	{
		return ExternalInterface.call("ae_DetachID3Pic",picHandle);
	}
	
/**
 *@}
 */	
 }
