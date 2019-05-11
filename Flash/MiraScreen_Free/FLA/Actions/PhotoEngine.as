
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

 /**
*@addtogroup PhotoEngine_as
*@{
*/

	/**display mode:full screen with no distortion*/
	public static var PE_RATIO_LETTER_BOX=0;		///<full screen with no distortion
	/**display mode:full screen may be distorted*/
	public static var PE_RATIO_FULL_SCREEN=1;		///<full screen may be distorted
	/**display mode:original size*/
	public static var PE_RATIO_ACTUAL_SIZE=2;		///<original size
	/**display mode:zoom out*/
	public static var PE_MODE_ZOOMOUT=3;			///<zoom out
	/**display mode:zoom in*/
	public static var PE_MODE_ZOOMIN=4;			///<zoom in
	/**display mode:rotate left*/
	public static var PE_MODE_ROTATE_LEFT=5;		///<rotate left
	/**display mode:rotate right*/
	public static var PE_MODE_ROTATE_RIGHT=6;	///<rotate right
	/**display mode:move left*/
	public static var PE_MODE_MOVE_LEFT=7;		///<move left
	/**display mode:move right*/
	public static var PE_MODE_MOVE_RIGHT=8;		///<move right		
	/**display mode:move up*/
	public static var PE_MODE_MOVE_UP=9;			///<move up
	/**display mode:move down*/
	public static var PE_MODE_MOVE_DOWN=10;		///<move down
	/**display mode:resize photo from zoom status */
	public static var PE_RATIO_RESIZE=11;			///<resize the photo

	/**back ground effect**/
	public static var PE_BACKGROUND_EFFECT_BLACK=0;
	public static var PE_BACKGROUND_EFFECT_WHITE=1;
	public static var PE_BACKGROUND_EFFECT_SILVER=2;
	public static var PE_BACKGROUND_EFFECT_GREEN=3;
	public static var PE_BACKGROUND_EFFECT_RADIANT=4;
	public static var PE_BACKGROUND_EFFECT_RADIANTMIRROR=5;
	
	/**
	* four kinds of decode mode
	*/
	public static var PE_DECODE_MODE_LBOX=0;
	public static var PE_DECODE_MODE_PAN_AND_SCAN=1;
	public static var PE_DECODE_MODE_FULLSCREEN=2;
	public static var PE_DECODE_MODE_ACTUAL_SIZE=3;

	/**
	Decoded result 
	*/
	//haven't decoded yet
	public static var PE_NO_DECODED = -2;				///<haven't decoded yet
	//decoded error
	public static var PE_DECODED_ERROR = -1;			///<decoded error
	//decoded success
	public static var PE_DECODED_OK = 0;				///<decoded ok
	// decode actions not supported
	public static var PE_DECODED_NOT_SUPPORT = 1;		///<the file is not support
	//decoded stop
	public static var PE_DECODED_STOP = 2;				///<the decoding is be stopped

	/**information to get, decoded pic**/
	public static var PE_GET_SRC_WIDTH=0;				///< get the original width of the photo
	public static var PE_GET_SRC_HEIGHT=1;				///< get the original height of the photo
	public static var PE_GET_SCALE_RATE=3;				///< get the scale ratio, it is the value *100.
	public static var PE_GET_ACTUAL_WIDTH=4;			///< get the acutal width of the photo be decoded
	public static var PE_GET_ACTUAL_HEIGHT=5;			///< get the actual height of the photo be decoded
	
	 /**face rect*/
	public static var POINT_X0=0;						///<x coordinate on the left-top conner
	public static var POINT_Y0=1;						///<y coordinate on the left-top conner
	public static var POINT_X1=2;						///<x coordinate on the right-bottom conner					
	public static var POINT_Y1=3;						///<y coordinate on the right-bottom conner		

	/**function switch related to face**/
	public static var FACEDETECTION_ENABLE=0;			///<the face dectction function is enable
	public static var FACEBEAUTIFY_ENABLE=1;			///<the face beautify function is enable
	public static var DYNAMLICLIGHTING_ENABLE=2;		///<the dynamic lighting function is enable
	public static var COLORENHANCE_ENABLE=3;			///<the color enhancement function is enable
	public static var EDGEENHANCE_ENABLE=4;			///<the edge enhancement function is enable
	public static var FUNC_CLEAR_ALL=5;				///<clear all the function be opened
	/**
	* rotation status
	*/
	public static var PE_ROTATE_NONE=0;		///<rotate none
	public static var PE_ROTATE_90=1;			///<rotate 90 degree
	public static var PE_ROTATE_180=2;			///<rotate 180 degree
	public static var PE_ROTATE_270=3;			///<rotate 270 degree
	
	
	/**
	* effect status
	*/
	public static var PE_EFFECT_OLDPHOTO=1;		///<photo effect: old photo
	public static var PE_EFFECT_BW=9;				///<photo effect: Black white




	/**Exif Information cmds**/
	public static var PE_EXIF_TIME=1;				///<get exif : time 
	public static var PE_EXIF_RESOLUTION=2;		///<get exif : resolution
	public static var PE_EXIF_USERTAG=4;			///<get exif : user tag

	/** cmds for getting status of decoding gif file**/
	public static var PE_GIF_GET_FRAME_DELAYTIME=0;  ///< get the delay time of current frame 1/100s. if the value is 100, it means 1s
	public static var PE_GIF_GET_FRAME_DEC_RET=1;	///< get the decoded result of current frame 

	/**the decoded result gets from middle ware**/
	public static var PE_GIF_DEC_RET_OK	= 0;		///< decoded ok
	public static var PE_GIF_DEC_DECODED_ERROR =-1; ///<decoded error
	public static var PE_GIF_DEC_UNSUPPORT_FMT=-2;	///< the file format is not support
	public static var PE_GIF_DEC_GIF_END	=1;		///< gif file end
	/**
	@brief open PhotoEngine
	@param[in] path	: path working path
	@return 
	 	- true 	: if success
	 	- false	: failed
	@warning don't forget to call close()
	@see close()
	 */
	public static function Open(path:String):Boolean
	{
		return ExternalInterface.call("pe_Open",path);
	}
	
	/**
	@brief attach photo to a specified MovieClip
	@param[in] target	: target MovieClip
	@param[in] width	: target width
	@param[in] height	: target height
	@return 
		 - true 	: if success
	 	- false	: failed
	@warning don't forget to call Detach()
	@see Detach()
	 */
	public static function Attach(target:MovieClip, width:Number, height:Number):Number
	{
		return ExternalInterface.call("pe_Attach",target,width,height);
	}
	
	/**
	@brief detach the previously attached photo
	@param[in] clone 	: indication whether copy the attached photo to movieclip
	@return 
		- true 	: if success
	 	- false	: if failed
	@see Attach()
	 */
	public static function Detach(target:MovieClip,clone:Boolean):Boolean
	{
		return ExternalInterface.call("pe_Detach", target,clone);
	}
	
	/**
	@brief close PhotoEngine
	@param[in] none
	@return 0:close success
	@		1:close failed
	@warning remember call this function after the open() had been called when the engine is unused
	@see Open()
	 */
	public static function Close():Number
	{
		return ExternalInterface.call("pe_Close");
	}
	
	/**
	@brief show photo according to mode
	@param[in] path	: photo path
	@param[in] mode 	: decode mode
	@see PE_RATIO_ZOOMOUT
	@see PE_RATIO_ZOOMIN
	@see PE_RATIO_LETTER_BOX
	@see PE_RATIO_FULL_SCREEN
	@see PE_RATIO_ACTUAL_SIZE
	@return 
		 - true 	: if success
	 	- false	: failed
	 */
	public static function Show(path:String, mode:Number):Boolean
	{
		return ExternalInterface.call("pe_Show",path,mode);
	}
	
	/**
	@brief get Music Enable or not
	@param[in] none
	@return 
		- 0	: disable
		- 1	: enable
	@see SetMusicEnable()
	 */
	public static function GetMusicEnable():Number
	{
		return ExternalInterface.call("pe_GetMusicEnable");
	}
	
	/**
	@brief set  Music Enable 
	@param[in] mode	:
		- 0	: disable
		- 1	: enable
	@return none
	@see GetMusicEnable
	 */
	public static function SetMusicEnable(mode:Number):Void
	{
		ExternalInterface.call("pe_SetMusicEnable",mode);
	}		
	
	
	/**
	@brief get display ratio
	@param[in] none
	@return  ratio, one of  PE_RATIO_LETTER_BOX,PE_RATIO_FULL_SCREEN,PE_RATIO_ACTUAL_SIZE
	 */
	public static function GetPlayRatio():Number
	{
		return ExternalInterface.call("pe_GetPlayRatio");
	}
	
	/**
	@brief set display ratio
	@param[in] mode	: one of  PE_RATIO_LETTER_BOX,PE_RATIO_FULL_SCREEN,PE_RATIO_ACTUAL_SIZE
	@return none
	 */
	public static function SetPlayRatio(mode:Number):Void
	{
		ExternalInterface.call("pe_SetPlayRatio",mode);
	}		
	
	
	/**
	@brief get interval time
	@param[in] none
	@return interval time
	 */
	public static function GetSlideShowTime():Number
	{
		return ExternalInterface.call("pe_GetSlideShowTime");
	}
	
	/**
	@brief set slide interval time
	@param[in] time	: interval time in second 
	@return none
	 */
	public static function SetSlideShowTime(time:Number):Void
	{
		ExternalInterface.call("pe_SetSlideShowTime",time);
	}			
/*---add by richard 04142012-*/	
		public static function GetSlideShowMode():Number
	{
		return ExternalInterface.call("pe_GetSlideShowMode");
	}
	
	/**
	@brief set slide interval time
	@param[in] time	: interval time in second 
	@return none
	 */
	public static function SetSlideShowMode(Mode:Number):Void
	{
		ExternalInterface.call("pe_SetSlideShowMode",Mode);
	}			
/*---add by richard 04142012-*/	

	
	/**
	@brief get slide show effect
	@param[in] none
	@return effect 
	 */
	public static function GetSlideShowEffect():Number
	{
		return ExternalInterface.call("pe_GetSlideShowEffect");
	}
	
	/**
	@brief set slide effect
	@param[in] effect 	: new effect to be set
	@return none
	 */
	public static function SetSlideShowEffect(effect:Number):Void 
	{
		ExternalInterface.call("pe_SetSlideShowEffect",effect);
	}	

	/**
	@brief get background effect
	@param[in] none
	@return effect 
	 */
	public static function GetBackGroundEffect():Number
	{
		return ExternalInterface.call("pe_GetBackGroundEffect");
	}

	/**
	@brief set background effect
	@param[in] effect 	: new effect to be set, see PE_BACKGROUND_EFFECT_BLACK
	@return none
	 */
	public static function SetBackGroundEffect(effect:Number):Void 
	{
		ExternalInterface.call("pe_SetBackGroundEffect",effect);
	}	

	/**
	@brief Attach face detection malloc the space which face detection needed and install face.drv
	@param[in] none
	@return
		- 0	: can't install face.drv
		- 1	: success 
	*/
	public static function AttachFace():Boolean
	{
		return ExternalInterface.call("pe_AttachFace");
	}

	/**
	@brief Detach face detection free the space whice face detection needed and uninstall face.drv
	@param[in] none
	@return
	 	- 1	: success 
	 	- 2	: fail
	*/
	public static function DetachFace():Boolean
	{
		return ExternalInterface.call("pe_DetachFace");
	}

	/**
	@brief Detect face
	@param[in] filename	: file name of picture
	@return the face number detected
	*/
	public static function DetectFace(filename:String):Number
	{
		return ExternalInterface.call("pe_DetectFace",filename);
	}

	/**
	@brief Get face rect
	@param[in] whichface	: the face index , it must be litter than the face number 
	@param[in] whichpoint	: one of POINT_X0,POINT_X1,POINT_Y0,POINT_Y1. 
	@return the coordianate of X0,X1,Y0,Y1
	*/
	public static function GetFaceRect(whichface:Number,whichpoint:Number):Number
	{
		return ExternalInterface.call("pe_GetFaceRect",whichface,whichpoint);
	}

	/**
	@brief call this function to set some functions enable or disable.
	@param[in] funcswitchcmd,see FACEDETECTION_ENABLE etc
	**/
	public static function setFaceFuncSwitch(funcswitchcmd:Number)
	{
		return ExternalInterface.call("pe_setFaceFuncSwitch",funcswitchcmd);
	}

	/**
	@brief call this function to get all status of the switch related to face.
	@param[in] none
	@return FACEDETECTION_ENABLE etc
	**/
	public static function getFaceFuncSwitch():Number
	{
		return ExternalInterface.call("pe_getFaceFuncSwitch");
	}

	/**
	@brief call this function before deleting file
	@param[in] none
	@return none
	*/
	public static function prepareForDel():Void
	{
		ExternalInterface.call("pe_PrepareForDel");
	}
	
	/**
	@brief Stop decode task 
	@param[in] cmd	:
			- 0 just stopping current task
			- 1 stopping all decode tasks 
	*/
	public static function stopDecode(cmd:Number):Void
	{
		ExternalInterface.call("pe_StopDecode",cmd);
	}

	/**
	@brief get decode status
	@param[in] filename	: the file be decoding
	@return PE_NO_DECODED etc
	@waring call this function after calling movieClip.loadClip() or Show() to get the status of decoding
	@see Show()
	 */
	public static function GetDecodedErr(fileName:String):Number
	{
		return ExternalInterface.call("pe_GetDecodedErr",fileName);
	}
	
	/**
	@brief write the rotation status to the end of the file
	@param[in] file		: file to be processed
	@param[in] rotation 	: the rotation status 
	@return 
		- 1 if success 
		- 0 if fail
	*/
	public static function StoreRotation(file:String,rotation:Number):Number
	{
		return ExternalInterface.call("pe_StoreRotation",file,rotation);
	}
	
	/**
	@brief get the rotation status of the file
	@param[in] file 	: file name
	@return rotation status if success or 0x7f(127) if fail
	*/
	public static function GetRotation(file:String):Number
	{
		return ExternalInterface.call("pe_GetRotation",file);
	}

	/**
	@brief : enable the auto rotating by exif infomation in the photo file, it is not added by micro-actions
	@param[in] autorotation 	: 0: disable, 1: enable
	@return : always return 0
	*/
	public static function SetAutoRotationbyExifEnable(autorotation:Number):Number
	{
		return ExternalInterface.call("pe_SetAutoRotationEnByExif",autorotation);
	}

	/**
	@brief : get the status of the auto rotation 
	@param[in] none
	@return : 0: disable, 1: enable
	*/
	public static function GetAutoRotationbyExifEnable():Number
	{
		return ExternalInterface.call("pe_GetAutoRotationEnByExif");
	}


	/**
	@brief : enable the auto rotating by adhere infomation in the photo file, it is added by micro-actions
	@param[in] autorotation 	: 0: disable, 1: enable
	@return : always return 0
	*/
	public static function SetAutoRotationbyAdhereEnable(autorotation:Number):Number
	{
		return ExternalInterface.call("pe_SetAutoRotationEnByAdhere",autorotation);
	}

	/**
	@brief : get the status of the auto rotation adhere to the end of the file
	@param[in] none
	@return : 0: disable, 1: enable
	*/
	public static function GetAutoRotationbyAdhereEnable():Number
	{
		return ExternalInterface.call("pe_GetAutoRotationEnByAdhere");
	}
	
	/**
	@brief write the effect status to the end of the file
	@param[in] file 	: file to be processed
	@param[in] effect 	: the effect see PE_EFFECT_OLDPHOTO etc
	@return
		- 1 if success
		- 0 if fail
	@see PE_EFFECT_OLDPHOTO etc
	*/
	public static function StoreEffect(file:String,effect:Number):Number
	{
		return ExternalInterface.call("pe_StoreEffect",file,effect);
	}
	
	/**
	@brief check some status before resize copy.
	@param[in] srcFile 	: full path of the file that to be copied
	@param[in] dstDir 	: destination directory that will be copied to 
	@return see file system error in "FileList" engine.
	*/
	public static function CheckResizeCopy(srcFile:String,dstDir:String):Number
	{
		return ExternalInterface.call("pe_CheckResizeCopy",srcFile,dstDir);
	}
	
	/**
	@brief copy the selected photo smartly
	@param[in] srcFile 	: full path of the file that to be copied
	@param[in] dstDir 	: destination directory that will be copied to 
	@return 
		- true if success 
		- false if fail
	*/
	public static function ResizeCopy(srcFile:String,dstDir:String):Boolean
	{
		return ExternalInterface.call("pe_ResizeCopy",srcFile,dstDir);
	}
	
	/**
	@brief ps photo after decoded
	@param[in] effect 	: the effect that to be processed. -1 for disable all.
	@return always 0
	*/
	public static function PSEffect(effect:Number):Number
	{
		return ExternalInterface.call("pe_PSEffect",effect);
	}
	
	/**
	@brief photo zoom via decoder
	@param[in] filename 	: the file to be zoom.
	@param[in] cmd 		: PE_MODE_ZOOMIN for zoom in and PE_RATIO_RESIZE for return to normal.
	*
	*/
	public static function Zoom(filename:String,cmd:Number):Number
	{
		return ExternalInterface.call("pe_Zoom",filename,cmd);
	}
	
	/**
	@brief rotate photo via decoder
	@param[in] filename : the file to be rotated.
	@param[in] angle : rotate angle.
	*        - PE_ROTATE_NONE=0; 
	*        - PE_ROTATE_90=1;   
	*        - PE_ROTATE_180=2;  
	*        - PE_ROTATE_270=3;  
	*
	*/
	public static function Rotate(filename:String,angle:Number):Number
	{
		return ExternalInterface.call("pe_Rotate",filename,angle);
	}
	
	/**
	@brief enable auto rotate according to sensor
	@param[in] enable 	: 1 to enable and 0 to disable.
	*/
	public static function SetAutoRotate(enable:Number):Number
	{
		return ExternalInterface.call("pe_SetAutoRotate",enable);
	}
	
	/**
	@brief set photo album name
	@param[in] albumIndex 	: album index
	@param[in] albumname 	: album name
	@return 1 if success or 0 if fail
	*/
	public static function SetPhotoAlbumName(albumIndex:Number,albumName:String):Boolean
	{
		return ExternalInterface.call("pe_SetPhotoAlbumName",albumIndex,albumName);
	}
	
	/**
	@brief get photo album name
	@param[in] albumIndex 	: album index
	@return album name
	*/
	public static function GetPhotoAlbumName(albumIndex:Number):String
	{
		return ExternalInterface.call("pe_GetPhotoAlbumName",albumIndex);
	}

	/**
	@brief get album sum
	@param[in] none
	@return album sum
	 */
	public static function GetPhotoAlbumSum():Number
	{
		return ExternalInterface.call("pe_GetPhotoAlbumSum");
	}
	
	/**
	@brief  set album sum
	@param[in] sum	: album sum
	@return none
	 */
	public static function SetPhotoAlbumSum(sum:Number):Void
	{
		ExternalInterface.call("pe_SetPhotoAlbumSum",sum);
	}	

	/**
	@brief set photo album status
	@param[in] albumIndex 	: album index
	@param[in] albumStatus 	: album status
	@return
		- 1 if success 
		- 0 if fail
	*/
	public static function SetPhotoAlbumStatus(albumIndex:Number,albumStatus:Number):Boolean
	{
		return ExternalInterface.call("pe_SetPhotoAlbumStatus",albumIndex,albumStatus);
	}
	
	/**
	@brief get photo album status
	@param[in] albumIndex		:album index
	@return album status
	*/
	public static function GetPhotoAlbumStatus(albumIndex:Number):Number
	{
		return ExternalInterface.call("pe_GetPhotoAlbumStatus",albumIndex);
	}
	/**
	@brief init photo album all info
	@param[in] albumIndex		:album index
	@return none
	*/
	public static function InitPhotoAlbumStatus()
	{
		return ExternalInterface.call("pe_InitPhotoAlbum");
	}

	/**
	@brief get the exif information  of the specified file
	@param[in] filepath	: the file will be extracted
	@param[in] cmd  	: PE_EXIF_TIME etc
			- if cmd==PE_EXIF_TIME, the format of the string returned is yyyymmdd
			- if cmd==PE_EXIF_RESOLUTION,the format of the string returned is width*height
	@return the information matched with the cmd
	**/
	public static function getExitInfo(filepath:String,cmd:Number):String
	{
		return ExternalInterface.call("pe_getExitInfo",filepath,cmd);
	}

	/**
	@brief call this function to get the information of the photo which had been decoded, call this fucntion after calling the GetDecodedErr()
	when the function return PE_DECODED_OK
	@param[in] cmd : see PE_GET_SRC_WIDTH etc
	@return 
	 	-1 : failed
	 	others : the num to be get according to the cmd
	**/
	public static function getDecodedPicInfo(cmd:Number):String
	{
		return ExternalInterface.call("pe_getDecodedPicInfo",cmd);
	}


	/**
	@brief start to load a gif file
	@param[in] filepath	: the audio file 
	@param[in] target		: the movieclip which will be attached to
	@param[in] target_w	: the width of movieclip 
	@param[in] target_h	: the height of movieclip
	@return
		- 0 		: faild to get the handle 
		- others	: the handle attached with the file, which will be used when calling gifGetNextFrame(), gifLoadEnd(),gifGetStatus()
	@see  gifGetNextFrame(), gifLoadEnd(),gifGetStatus()
	**/
	public static function gifLoadStart(filepath:String,target:MovieClip,target_w:Number,target_h:Number):Number
	{
		return ExternalInterface.call("pe_gifLoadStart",filepath,target,target_w,target_h);
	}

	/**
	@brief get the next frame of an gif  which had been opened
	@param[in] gif_handle	: the handle attatched with the file which had been opened, it got from gifLoadStart()
	@return 
		- 0 	: succeed
		- -1	: failed
	@see gifLoadStart()
	**/
	public static function gifGetNextFrame(gif_handle:Number):Number
	{
		return ExternalInterface.call("pe_gifGetNextFrame",gif_handle);
	}


	/**
	@brief release the handle which attatched with the file had been opened
	@param[in] gif_handle	: the handle attatched with the file which had been opened, it got from gifLoadStart()
	@return 
		- 0 	: succeed
		- -1	: failed
	@see gifLoadStart()
	**/
	public static function gifLoadEnd(gif_handle:Number):Number
	{
		return ExternalInterface.call("pe_gifLoadEnd",gif_handle);
	}

	/**
	@brief get the status of the gif decoding
	@param[in] gif_handle	: the handle attatched with the file which had been opened, it got from gifLoadStart()
	@param[in] cmd		: the cmd for getting status of decoding gif file. see PE_GIF_GET_FRAME_DELAYTIME etc
	@return 
		- if cmd==PE_GIF_GET_FRAME_DELAYTIME the unit of the value is 1/100s, if the value is 100 , it means delay 1s
		- if cmd==PE_GIF_GET_FRAME_DEC_RET, the value is one of PE_GIF_DEC_RET_OK, PE_GIF_DEC_DECODED_ERROR,
		PE_GIF_DEC_UNSUPPORT_FMT, PEF_GIF_DEC_GIF_END
	@see gifLoadStart(),PE_GIF_GET_FRAME_DELAYTIME etc
	**/
	public static function gifGetStatus(gif_handle:Number,cmd:Number):Number
	{
		return ExternalInterface.call("pe_gifGetStatus",gif_handle,cmd);
	}



	public static function DecodeImage(file_uri:String,width:Number, height:Number):Number
	{
		return ExternalInterface.call("pe_DecodeImage",file_uri,width,height);
	}

	
	/**
	* @brief Check if moble upload a picture, and move it to another file for show it.
	* @param[in] NULL
	* @return Return 1 while the moble had upload a picture, else return 0; 
	**/
	public static function check_help_picture():Number
	{
		return ExternalInterface.call("pe_check_help_picture");
	}

	/**
	* @brief Show help picture in EZCast function, it can only show one jpeg picture.
	* @param[in] NULL
	* @return NULL
	**/
	public static function show_help_picture()
	{
		ExternalInterface.call("pe_show_help_picture");
	}

	/**
	* @brief Clean help picture in EZCast function.
	* @param[in] NULL
	* @return NULL
	**/
	public static function clean_help_picture()
	{
		ExternalInterface.call("pe_clean_help_picture");
	}
	/**
	* @brief Show icon picture in EZCast function, it can show more than one jpeg/png pictures.
	* @param[in] 		path		: 	The path of the icon picture;
	*				x/y/w/h	:	The coordinate of picture, it is based on 1920x1080;
	* @return NULL
	**/
	public static function show_icon(path:String, x:Number, y:Number, w:Number, h:Number)
	{
		ExternalInterface.call("pe_show_icon_in_display", path, x, y, w, h);
	}

	/**
	* @brief Clean png icon in EZCast function.
	* @param[in] NULL
	* @return NULL
	**/
	public static function clean_icon()
	{
		ExternalInterface.call("pe_clean_icon");
	}
	/**
	* @brief clean a rect area png icon in EZCast function.
	* @param[in] 		x/y/w/h	:	The coordinate of picture, it is based on 1920x1080;
	* @return NULL
	**/
	public static function clean_icon_rect(x:Number, y:Number, w:Number, h:Number)
	{
		ExternalInterface.call("pe_icon_clean_rect", x, y, w, h);
	}

	/**
	* @brief Show text picture in EZCast function, it can show more than one jpeg/png pictures.
	* @param[in] 		textstring: 	The text string to show;
	*				fontsize	:	The font size of text;
	*				color		:	The color of text, the format is 8888 ARGB, for example: if A=0x0, R=0x0, G=0x33, B=0xFF, then the color is 0x000033FF;
	*				x/y		:	The coordinate of picture, it is based on 1920x1080;
	* @return NULL
	**/
	public static function show_text(textstring:String, fontsize:Number, color:Number, x:Number, y:Number)
	{
		ExternalInterface.call("pe_show_text_in_display", textstring, fontsize, color, x, y);
	}


/**
 *@}
 */	

 }
 
