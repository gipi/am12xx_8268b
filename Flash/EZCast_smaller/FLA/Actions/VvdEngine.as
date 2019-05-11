
/**
 *	@author   wuxiaowen
 *	@vvdrsion 1.0
 *	@date: 2009/06/08
 *	<pre>  
 *	VvdEngine Class
 			VVD play control 
 	</pre>
 *	<pre>
 	Example1(for fixed theme):
 *		VvdEngine.Open("c:\\wedding", 3);
 *		VvdEngine.Save("c:\\my_vvd\\");
 *		VvdEngine.Close();
 *	</pre>
 *	<pre>
 	Example2(for an generated module)
 *		VvdEngine.Open("c:\\myvvd.vd2");
 *		VvdEngine.Play(VvdEngine.VVD_PLAYMODE_ONCE);
 *		VvdEngine.Stop();
 *		VvdEngine.Close();
 *	</pre>
 */

 dynamic class Actions.VvdEngine {

/**
*@addtogroup VvdEngine_as
*@{
*/

	public static var VVD_IDLE=0;				///< play status:idle
	public static var VVD_PLAY=1;				///< play status:play
	public static var VVD_PAUSE=2;				///< play status:paused
	
	public static var VVD_PLAYMODE_ONCE=0;		///< play mode:play only once
	public static var VVD_PLAYMODE_REPEAT=1;	///< play mode:repeated play

	/**
	 *@brief	open VvdEngine
	 *@param[in] path	: vvd file/swf folder to open
	 *@return 
	 *		true if success or false if fail
	 */
	public static function Open(path:String):Boolean
	{
		return ExternalInterface.call("vvd_Open",path);
	}
	
	/**
	 *@brief	save edited vvd file
	 *@param[in] path	: saved path  
	 *@return 
	 *		true if success or false if fail
	 */
	public static function Save(path:String):Boolean
	{
		return ExternalInterface.call("vvd_Save",path);
	}
	
	/**
	 *@brief	close VvdEngine
	 *@param[in] NULL
	 *@return NULL
	 */
	public static function Close()
	{
		return ExternalInterface.call("vvd_Close");
	}
	
	/**
	 *@brief	load vvd to target movieclip
	 *@param[in] target	: destination MovieClip, usually created by createEmptyMovieClip
	 *@param[in] width	: width of movieclip
	 *@param[in] height	: height of movieclip
	 *@return NULL
	 */
	public static function LoadMovie(target:MovieClip, width:Number, height:Number)
	{
		return ExternalInterface.call("vvd_LoadMovie",target,width,height);
	}
	
	/**
	 *@brief	get template number in current vvd file
	 *@param[in] NULL
	 *@return 
	 *		template number
	 */
	public static function GetTemplateNum():Number
	{
		return ExternalInterface.call("vvd_GetTemplateNum");
	}
	
	/**
	 *@brief	insert new template at tpl
	 *@param[in] tpl	: insert position
	 *@return 
	 *		new template index if success or -1 if fail
	 */
	public static function InsertTemplate(tpl:Number):Number
	{
		return ExternalInterface.call("vvd_InsertTemplate",tpl);
	}
	
	/**
	 *@brief	delete template at tpl
	 *@param[in] tpl	: template to be deleted
	 *@return 
	 *		true if success or false if fail
	 */
	public static function DeleteTemplate(tpl:Number):Boolean
	{
		return ExternalInterface.call("vvd_DeleteTemplate",tpl);
	}
	
	/**
	 *@brief	get template path(e.g."c:\\a.swf")
	 *@param[in] tpl	: template index
	 *@return 
	 *		template path
	 */
	public static function GetTemplatePath(tpl:Number):String
	{
		return ExternalInterface.call("vvd_GetTemplatePath",tpl);
	}
	
	/**
	 *@brief	get template image file path(如"c:\\a.jpg")
	 *@param[in] tpl	: template index
	 *@return 
	 *		iamge file path
	 */
	public static function GetThumbPath(tpl:Number):String
	{
		return ExternalInterface.call("vvd_GetThumbPath",tpl);
	}
	
	/**
	 *@brief	get total photo number of current vvd file
	 *@param[in] NULL
	 *@return 
	 *		total photo number
	 */
	public static function GetPhotoNum():Number
	{
		return ExternalInterface.call("vvd_GetPhotoNum");
	}
	
	/**
	 *@brief	insert new photo to current vvd file
	 *@param[in] photo	: photo index
	 *@param[in] path	: inserted photo path
	 *@return 
	 *		new photo index if success or -1 if fail
	 */
	public static function InsertPhoto(photo:Number,path:String):Number
	{
		return ExternalInterface.call("vvd_InsertPhoto",photo,path);
	}
	
	/**
	 *@brief	delete one photo
	 *@param[in] photo	: photo index
	 *@return
	 *		true if success or false if fail
	 */
	public static function DeletePhoto(photo:Number):Boolean
	{
		return ExternalInterface.call("vvd_DeletePhoto",photo);
	}
	
	/**
	 *@brief	get pointed photo path in current vvd file 
	 *@param[in] photo	: photo index
	 *@return 
	 *		photo path in the vvd file
	 */
	public static function GetPhotoPath(photo:Number):String
	{
		return ExternalInterface.call("vvd_GetPhotoPath",photo);
	}
	
	/**
	 *@brief	get music file number in current vvd file
	 *@param[in] NULL
	 *@return 
	 *		total music file number
	 */
	public static function GetMusicNum():Number
	{
		return ExternalInterface.call("vvd_GetMusicNum");
	}
	
	/**
	 *@brief	insert a new music before the pointed position
	 *@param[in] music	: the pointed position
	 *@param[in] path	: new music file path
	 *@return 
	 *		new music file index if success or -1 if fail
	 */
	public static function InsertMusic(music:Number,path:String):Number
	{
		return ExternalInterface.call("vvd_InsertMusic",music,path);
	}
	
	/**
	 *@brief	delete one music from current vvd file
	 *@param[in] music	: music index
	 *@return 
	 *		true if success or false if fail
	 */
	public static function DeleteMusic(music:Number):Boolean
	{
		return ExternalInterface.call("vvd_DeleteMusic",music);
	}
	
	/**
	 *@brief	get the pointed music file path
	 *@param[in] music	: the pointed index
	 *@return 
	 *		the path
	 */
	public static function GetMusicPath(music:Number):String
	{
		return ExternalInterface.call("vvd_GetMusicPath",music);
	}
	
	/**
	 *@brief	get the file path to be save
	 *@param[in] NULL
	 *@return 
	 *		vvd file path
	 */
	public static function GetSavePath():String
	{
		return ExternalInterface.call("vvd_GetSavePath");
	}
	
	/**
	 *@brief	play the given vvd file
	 *@param[in] mode	: play mode
	 *@return NULL
	 */
	public static function Play(mode:Number)
	{
		return ExternalInterface.call("vvd_Play",mode);
	}
	
	/**
	 *@brief	stop play vvd
	 *@param[in] NULL
	 *@return NULL
	 */
	public static function Stop()
	{
		return ExternalInterface.call("vvd_Stop");
	}
	
	/**
	 *@brief	pause current vvd 
	 *@param[in] NULL
	 *@return NULL
	 */
	public static function Pause()
	{
		return ExternalInterface.call("vvd_Pause");
	}
	
	/**
	 *@brief	resume play current vvd if paused
	 *@param[in] NULL
	 *@return NULL
	 */
	public static function Resume()
	{
		return ExternalInterface.call("vvd_Resume");
	}
	
	/**
	 *@brief	get current play status
	 *@param[in] NULL
	 *@return 
	 *		play status
	 *		- VVD_IDLE
	 *		- VVD_PLAY
	 *		- VVD_PAUSE
	 */
	public static function GetState():Number
	{
		return ExternalInterface.call("vvd_GetState");
	}
	
	/**
	 *@brief	get vvd play mode
	 *@param[in] NULL
	 *@return 
	 *		VVD play mode
	 *		- VVD_PLAYMODE_ONCE
	 *		- VVD_PLAYMODE_REPEAT
	 */
	public static function GetPlayMode():Number
	{
		return ExternalInterface.call("vvd_GetPlayMode");
	}
	
	/**
	 *@brief	set VVD play mode
	 *@param[in] mode	: mode to be set
	 *		- VVD_PLAYMODE_ONCE
	 *		- VVD_PLAYMODE_REPEAT
	 *@return 
	 *		true if success or false if fail
	 */
	public static function SetPlayMode(mode:Number):Number
	{
		return ExternalInterface.call("vvd_SetPlayMode",mode);
	}									
	
	/**
	 *@brief	play independent flash
	 *@param[in] path	: path of flash
	 *@return NULL
	 */
	public static function PlayFlash(path:String)
	{
		return ExternalInterface.call("vvd_PlayFlash",path);
	}									
	
	/**
	 *@brief	stop independent flash playing
	 *@param[in] NULL
	 *@return NULL
	 */
	public static function StopFlash()
	{
		return ExternalInterface.call("vvd_StopFlash");
	}	
	
	/**
	 *@brief	get the first template path from the current vvd file
	 *@param[in] NULL
	 *@return 
	 *		the path of the first template
	 */
	public static function GetFirstTemplate():String
	{
		return ExternalInterface.call("vvd_GetFirstTemplate");
	}
	
	/**
	 *@brief	set the photo directory for the selected template
	 *@param[in] dir	: the directory path 
	 *@return 
	 *		true if success or false if fail
	 */
	public static function SetPhotoDir(dir:String):Number
	{
		return ExternalInterface.call("vvd_SetPhotoDir",dir);
	}
	
	/**
	 *@brief	check if photo list has been played for at least once
	 *@param[in] NULL
	 *@return
	 *		- 1 if photo list played for at least once 
	 *		- 0 if not
	 */
	public static function PhotoRollover():Number
	{
		return ExternalInterface.call("vvd_PhotoRollover");
	}
	
	/**
	 *@brief	reset the photo index to the first one
	 *@param[in] NULL
	 *@return 
	 *		true if success or false if fail
	 */
	public static function ResetPhotoIndex():Number
	{
		return ExternalInterface.call("vvd_ResetPhotoIndex");
	}

/**
 *@}
 */
 
 }
 