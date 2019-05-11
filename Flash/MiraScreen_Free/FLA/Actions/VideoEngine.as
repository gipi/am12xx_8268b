
/**
 *	@author   wuxiaowen
 *	@version 1.0
 *  @date	: 2009/06/08
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

/**
*@addtogroup VideoEngine_as
*@{
*/

	public static var VE_IDLE=0;	///< play status:idle
	public static var VE_PLAY=1;	///< play status:play
	public static var VE_PAUSE=2;	///< play status:pause
	public static var VE_FF=3;		///< play status:fast forward
	public static var VE_FB=4;		///< play status:fast backward
	public static var VE_STOP=5;	///< play status:stop
	public static var VE_ERROR=6;	///< play status:stop
	public static var VE_READY_STOP=7;	///< play status:stop
	
	public static var VE_RATIO_LETTER_BOX = 0;//2;	///< display ratio: full screen with no distortion, change to 0 by richard
	public static var VE_RATIO_FULL_SCREEN = 1;//3;	///< display ratio: full screen with distortion, change to 1 by richard
	public static var VE_RATIO_ACTUAL_SIZE = 2;//4;	///< display ratio: original size, change to 2 by richard, our LSDK don't have this function

	public static var VE_PLAYMODE_REPEAT_SEQUENCE = 0;		///< play mode:repeat sequence
	public static var VE_PLAYMODE_REPEATONE = 1;			///< play mode:sigle loop
	public static var VE_PLAYMODE_REPEAT_RANDOM = 2;		///< play mode:repeat random
	public static var VE_PLAYMODE_ONCE = 3;					///< play mode:only once
	public static var VE_PLAYMODE_INVALID = 4;				///< play mode boundary

	///< add for TS PS BEGIN
	public static var IDEOINFO_PRO_NUM=0;		///< get programe num
	public static var VIDEOINFO_PRO_INFO=1;		///< get programe information

	////< the following is used with VIDEOINFO_PRO_INFO
	public static var PRO_NAME=3;				///< get program name
	public static var PRO_V_NUM=4;				///< get video program num
	public static var PRO_A_NUM=5;				///< get audio program num
	public static var VIDEO_INFO=6;				///< get video stream information
	public static var AUDIO_INFO=7;				///< get audio stream information

	///< the following is used with VIDEO_INFO and AUDIO_INFO
	public static var STREAM_ID=8;				///< video/audio stream ID
	public static var STREAM_VALID=9;			///< video/audio stream valid
	public static var STREAM_CODECID=10;		///< video/audio stream codecid
	public static var STREAM_TYPE=11;			///< video/audio stream Type
    ///< END

    ///< the following cmd is used under linux
    public static var CMD_GET_TIME_START = 0;	///< get start time
    public static var CMD_GET_TIME_END = 1;		///< get end time

	/**
	 *@brief	open VideoEngine
	 *@param[in] NULL
	 *@return 
	 *		true if success or false if fail
	 */	
	
	public static function Open():Boolean
	{
		return ExternalInterface.call("ve_Open");
	}
	
	/**
	 *@brief	close VideoEngine
	 *@param[in] NULL
	 *@return NULL
	 */
	public static function Close()
	{
		return ExternalInterface.call("ve_Close");
	}
	
	/**
	 *@brief	Set the video file to be played
	 *@param[in] path	: Full path name
	 *@return
	 *		true if success or false if fail
	 */
	public static function SetFile(path:String):Boolean
	{
		return ExternalInterface.call("ve_SetFile",path);
	} 
	
	/**
	 *@brief	attach the moview preview to pointed MovieClip
	 *@param[in] target	: destination MovieClip, usually created by createEmptyMovieClip
	 *@param[in] width	: destination width
	 *@param[in] height	: destination height
	 *@param[in] index	: the logic index of the file
	 *@return
	 *		true if success or false if fail
	 */
	public static function AttachPicture(target:MovieClip, width:Number, height:Number, index:Number):Boolean
	{
		return ExternalInterface.call("ve_Picture",target,width,height,index);
	}
	
	/**
	 *@brief	detach the moview preview
	 *@param[in] target	: destination MovieClip, usually created by createEmptyMovieClip
	 *@param[in] clone	: clone or not
	 *@param[in] index	: the logic index of the file
	 *@return
	 *		true if success or false if fail
	 */
	public static function DttachPicture(target:MovieClip, clone:Number, index:Number):Boolean
	{
		return ExternalInterface.call("ve_DetachPicture",target,clone,index);
	}
	
	/**
	 *@brief	get the movie preview,width and height should be the same with AttachPicture
	 *@param[in] width	: preview width
	 *@param[in] index	: the logic index of the file
	 *@param[in] height	: preview height
	 *@return
	 * - 0	:	true
	 * - 3	:	file not support
	 */
	public static function GetOneFrame(width:Number, height:Number, index:Number):Number
	{
		return ExternalInterface.call("ve_getOneFrame",width,height,index);
	}

	/**
	 *@brief	stop play
	 *@param[in] : max frame buffer width, height is auto calculated according to ratio of screen width and height.
	 *@return NULL
	 */
	public static function SetMaxFrameWidth(width:Number)
	{
		return ExternalInterface.call("ve_Set_max_frame_width",width);
	}
	
	/**
	 *@brief	play the given video
	 *@param[in] mode	: play mode
	 *		- VE_PLAYMODE_ONCE
	 *		- VE_PLAYMODE_REPEAT_SEQUENCE
	 *		- VE_PLAYMODE_REPEAT_RANDOM
	 *		- VE_PLAYMODE_REPEATONE
	 *@param[in] ratio	: display ratio
	 *		- VE_RATIO_LETTER_BOX
	 *		- VE_RATIO_FULL_SCREEN
	 *		- VE_RATIO_ACTUAL_SIZE
	 *@return
	 *		true if success or false if fail
	 */
	public static function Play(mode:Number,ratio:Number):Boolean
	{
		return ExternalInterface.call("ve_Play",mode,ratio);
	}
	
	/**
	 *@brief	stop play
	 *@param[in] NULL
	 *@return NULL
	 */
	public static function Stop()
	{
		return ExternalInterface.call("ve_Stop");
	}
	
	/**
	 *@brief	pause
	 *@param[in] NULL
	 *@return NULL
	 */
	public static function Pause()
	{
		return ExternalInterface.call("ve_Pause");
	}
	
	/**
	 *@brief	resume if paused
	 *@param[in] NULL
	 *@return NULL
	 */
	public static function Resume()
	{
		return ExternalInterface.call("ve_Resume");
	}
	
	/**
	 *@brief	fast forward
	 *@param[in] step:the step for fast forward
	 *@return NULL
	 */
	public static function FF(step:Number)
	{
		ExternalInterface.call("ve_FF",step);
	}
	
	/**
	 *@brief	fast backward
	 *@param[in] step:the step for fast backward
	 *@return NULL
	 */
	public static function FB(step:Number)
	{
		ExternalInterface.call("ve_FB",step);
	}

	/**
	 *@brief	seek play
	 *@param[in] seek	: seek point to be set,abs time in this audio
	 *@return
	 * - 0	:	success
	 * - -1	:	fail
	 */
	public static function SeekPlay(seektime:Number):Number
	{
		return ExternalInterface.call("ve_SeekPlay",seektime);
	}
	/**
	 *@brief	get current play status
	 *@param[in] NULL
	 *@return status
	 *		- VE_IDLE
	 *		- VE_PLAY
	 *		- VE_PAUSE
	 *		- VE_FF
	 *		- VE_FB
	 *		- VE_STOP
	 */
	public static function GetState():Number
	{
		return ExternalInterface.call("ve_State");
	}
	
	/**
	 *@brief	get total play time
	 *@param[in] NULL
	 *@return
	 *		total time with resolution millisecond
	 */
	public static function GetTotalTime():Number
	{
		return ExternalInterface.call("ve_TotalTime");
	}
	
	/**
	 *@brief	get current play time
	 *@param[in] NULL
	 *@return
	 *		current time with resolution millisecond
	 */
	public static function GetCurTime():Number
	{
		return ExternalInterface.call("ve_CurTime");
	}
	
	/**
	 *@brief	get play ratio
	 *@param[in] NULL
	 *@return ratio
	 *		- VE_RATIO_LETTER_BOX
	 *		- VE_RATIO_FULL_SCREEN
	 *		- VE_RATIO_ACTUAL_SIZE
	 */
	public static function GetPlayRatio():Number
	{
		return ExternalInterface.call("ve_GetPlayRatio");
	}
	
	/**
	 *@brief	set display ratio
	 *@param[in] ratio	: the ratio to be set	 	
	 *		- VE_RATIO_LETTER_BOX
	 *		- VE_RATIO_FULL_SCREEN
	 *		- VE_RATIO_ACTUAL_SIZE
	 *@return
	 *		true if success or false if fail
	 */
	public static function SetPlayRatio(ratio:Number):Number
	{
		return ExternalInterface.call("ve_SetPlayRatio",ratio);
	}		
	
	/**
	 *@brief	get play mode
	 *@param[in] NULL
	 *@return the play mode
	 *		- VE_PLAYMODE_ONCE
	 *		- VE_PLAYMODE_REPEAT_SEQUENCE
	 *		- VE_PLAYMODE_REPEAT_RANDOM
	 *		- VE_PLAYMODE_REPEATONE
	 */
	public static function GetPlayMode():Number
	{
		return ExternalInterface.call("ve_GetPlayMode");
	}
	
	/**
	 *@brief	set play mode
	 *@param[in] mode	: mode to be set 
	 *		- VE_PLAYMODE_ONCE
	 *		- VE_PLAYMODE_REPEAT_SEQUENCE
	 *		- VE_PLAYMODE_REPEAT_RANDOM
	 *		- VE_PLAYMODE_REPEATONE
	 *@return
	 *		true if success or false if fail
	 */
	public static function SetPlayMode(mode:Number):Number
	{
		return ExternalInterface.call("ve_SetPlayMode",mode);
	}


	/**
	 *@brief	Get video file infomation
	 *@param[in] info_tag	:	
	 *			- VIDEOINFO_PRO_NUM=0
	 *			- VIDEOINFO_PRO_INFO=1
	 *@param[in] prg_tag	:	
	 *			- PRO_NAME=3,
	 *			- PRO_V_NUM=4,
	 *			- PRO_A_NUM=5,
	 *			- VIDEO_INFO=6,
	 *			- AUDIO_INFO=7,
	 *@param[in] prg_index	: lesser than the returned num of VIDEOINFO_PRO_NUM
	 *@param[in] stream_tag	:	
	 *			- STREAM_ID=8;
	 *			- STREAM_VALID=9;
	 *			- STREAM_TYPE=10;
	 *@param[in] stream_index	: lesser than  the returned num of VIDEO_INFO or AUDIO_INFO
	 *@return
	 *		if the content is number ,then it  will be changed to string. 
	 */
	public static function GetVideoFileInfoStr(info_tag:Number,prg_tag:Number,prg_index:Number,stream_tag:Number,stream_index:Number):String
	{
		return ExternalInterface.call("ve_GetVideoFileInfoStr",info_tag,prg_tag,prg_index,stream_tag,stream_index);
	}

	/**
	 *@brief	Get video file infomation
	 *@param[in] info_tag	:	
	 *			- VIDEOINFO_PRO_NUM=0
	 *			- VIDEOINFO_PRO_INFO=1
	 *@param[in] prg_tag	:	
	 *			- PRO_NAME=3,
	 *			- PRO_V_NUM=4,
	 *			- PRO_A_NUM=5,
	 *			- VIDEO_INFO=6,
	 *			- AUDIO_INFO=7,
	 *@param[in] prg_index	: lesser than the returned num of VIDEOINFO_PRO_NUM
	 *@param[in] stream_tag	:	
	 *			- STREAM_ID=8;
	 *			- STREAM_VALID=9;
	 *			- STREAM_TYPE=10;
	 *@param[in] stream_index	: lesser than  the returned num of VIDEO_INFO or AUDIO_INFO
	 *@return
	 * 		get video/audio program num
	 */
	public static function GetVideoFileInfoNum(info_tag:Number,prg_tag:Number,prg_index:Number,stream_tag:Number,stream_index:Number):Number
	{
		return ExternalInterface.call("ve_GetVideoFileInfoNum",info_tag,prg_tag,prg_index,stream_tag,stream_index);
	}
	/**
	 *@brief	Set the channnel of TS or PS
	 *@param[in] v_id	: video channel id
	 *@param[in] a_id	: audio channel id
	 *@return
	 * - -1		: error
	 * - others	: succeed
	 **/
	public static function SetVideoPro(v_id:Number,v_codecid:Number,a_id:Number,a_codecid:Number,a_onlyswitch:Number):Number
	{
		return ExternalInterface.call("ve_SetVideoPro",v_id,v_codecid,a_id,a_codecid,a_onlyswitch);
	}

	////the follow function is used in linux
	/**
	 *@brief: init an instance of parser for subtitle
	 *@param[in] filepath	: the path of the file which contains subtitle
	 *@return
	 * - 0	: error
	 * - !0	: the handle of parser which will be used in other place
	 **/
	public static function initParserInstance(filepath:String):Number
	{
		return ExternalInterface.call("sp_initInstance",filepath);
	}

	/**
	 *@brief remove the instance of parser for subtitle
	 *@param[in] parser_handle	: the handle got from initParserInstance
	 *@return
	 * - 0	: succeed
	 * - !0	: fail
	 **/
	public static function rmParserinstance(parser_handle:Number):Number
	{
		return ExternalInterface.call("sp_removeInstance",parser_handle);
	}

	/**
	 *@brief parse the subtitle
	 *@param[in] parser_handle	: the handle got from initParserInstance
	 *@return
	 * - 0	: succeed
	 * - !0	: fail
	 **/
	public static function parseSubtitle(parser_handle:Number):Number
	{
		return ExternalInterface.call("sp_parseSubtitle",parser_handle);
	}

	/**
	 *@brief tell the parer to get next line info
	 *@param[in] parser_handle	: the handle got from initParserInstance
	 *@return
	 * - 0	: succeed
	 * - !0	: fail
	 **/
	public static function getNextLine(parser_handle:Number):Number
	{
		return ExternalInterface.call("sp_getNextLine",parser_handle);
	}


	public static function get_linenum():Number
	{
		return ExternalInterface.call("sp_getlinenum");
	}
 	/**
	 *@brief	get the head pionters of each line in the subtitle after been parting
	 *@param[in] NULL
	 *@return the head pionters
	 *		
	 **/
	public static function get_linecontent(line_idx:Number):String
	{
		return ExternalInterface.call("sp_getlinecontent",line_idx);
	}
	public static function get_linewidth(line_idx:Number):Number
	{
		return ExternalInterface.call("sp_getlinewidth",line_idx);
	}
	/**
	 *@brief	query the info of subtitle which is close to the specify time
	 *@param[in] parser_handle	: the handle got from initParserInstance
	 *@param[in] time_point		: the unit is second
	 *@return
	 * - -1	:	failed
	 * - 0	:	it is a txt context
	 * - 1	:	it is an image context
	 **/
	public static function queryLineInfo(parser_handle:Number,time_point:Number):Number
	{
		return ExternalInterface.call("sp_queryLineInfo",parser_handle,time_point);
	}

	/**
	 *@brief	get the info of the subtitle which is stored after the funciton queryLineInfo was called and returned 0
	 *@param[in] NULL
	 *@return
	 *		the information of this line
	 **/
	public static function getLineInfo():String
	{
		return ExternalInterface.call("sp_getLineInfo");
	}

	/**
	 *@brief	return the time of the subtitle which is stored when the function queryLineInfo is called, unit is second
	 *@param[in] cmd	: CMD_GET_TIME_START etc
	 *@return  time in second
	 **/
	public static function getTimeTag(cmd:Number):Number
	{
		return ExternalInterface.call("sp_getTimeTag",cmd);
	}

	/**
	 *@brief	attach the subtitle to a movieclip after calling queryLineInfo function when it returned 1
	 *@param[in] mc	: the movieClip to be attached with the subtitle
	 *@param[in] w	: the width of the movieClip
	 *@param[in] h	: the height of the movieClip
	 *@return
	 * - 0	: succeed
	 * - -1	: failed
	 **/
	public static function attachSubtitle(target:MovieClip,w:Number,h:Number):Number
	{
		return ExternalInterface.call("sp_attachBitmap",target,w,h);
	}

	/**
	 *@brief	attach the subtitle to a movieclip after calling queryLineInfo function when it returned 1
	 *@param[in] mc	: the movieClip to be attached with the subtitle
	 *@param[in] w	: the width of the movieClip
	 *@param[in] h	: the height of the movieClip
	 *@return
	 * - 0	: succeed
	 * - -1	: failed
	 **/
	public static function Seekable():Number
	{
		return ExternalInterface.call("ve_Seekable");
	}
	/**
	 *@brief store the FB step for OSD display
	 *@param[in] step:the step which the last FB() has used 
	 *@return NULL
	 **/
	public static function put_video_fast_backward_step(step:Number)
	{
		ExternalInterface.call("put_videofastbackwardstep",step);
	}
	/**
	 *@brief store the FF step for OSD display
	 *@param[in] step:the step which the last FF() has used 
	 *@return NULL
	 **/
	public static function put_video_fast_forward_step(step:Number)
	{
		ExternalInterface.call("put_videofastforwardstep",step);
	}
	/**
	 *@brief get the FB step for OSD display
	 *@param[in] step:the step which the last FB() has used 
	 *@return the step
	 **/
	public static function get_video_fast_backward_step():Number
	{
		return ExternalInterface.call("get_videofastbackwardstep");
	}
	/**
	 *@brief get the FF step for OSD display
	 *@param[in] step:the step which the last FF() has used 
	 *@return the step
	 **/
	public static function get_video_fast_forward_step():Number
	{
		return ExternalInterface.call("get_videofastforwardstep");
	}


/**
 *@}
 */
 
 }
 
