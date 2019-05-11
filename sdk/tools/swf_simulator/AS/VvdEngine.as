
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
	 
	/**play status:idle*/
	public static var VVD_IDLE=0;
	/**play status:play*/
	public static var VVD_PLAY=1;
	/**play status:paused*/
	public static var VVD_PAUSE=2;
	
	/**play mode:play only once*/
	public static var VVD_PLAYMODE_ONCE=0;
	/**play mode:repeated play*/
	public static var VVD_PLAYMODE_REPEAT=1;

	/**
	 *open VvdEngine
	 *@param 
	 path vvd file to be opend
	 *@param 
	 num number of template file in vvd file, 0 presents infinite
	 *@return 
	 true if success or false
	 */
	public static function Open(path:String, num:Number):Boolean
	{
		return ExternalInterface.call("vvd_Open",path,num);
	}
	
	/**
	 *save edited vvd file
	 *@param 
	 	path saved path  
	 *@return 
	 true if success or false
	 */
	public static function Save(path:String):Boolean
	{
		return ExternalInterface.call("vvd_Save",path);
	}
	
	/**
	 *close VvdEngine
	 */
	public static function Close()
	{
		return ExternalInterface.call("vvd_Close");
	}
	
	/**
	 * load vvd to target movieclip
	 */
	public static function LoadMovie(target:MovieClip, width:Number, height:Number)
	{
		return ExternalInterface.call("vvd_LoadMovie",target,width,height);
	}
	
	/**
	 *get template number in current vvd file
	 *@return 
	 template number
	 */
	public static function GetTemplateNum():Number
	{
		return ExternalInterface.call("vvd_GetTemplateNum");
	}
	
	/**
	 *insert new template at tpl
	 *@param 
	 tpl insert position
	 *@return 
	 if success new template index，or -1
	 */
	public static function InsertTemplate(tpl:Number):Number
	{
		return ExternalInterface.call("vvd_InsertTemplate",tpl);
	}
	
	/**
	 *delete template at tpl
	 *@param 
	 tpl template to be deleted
	 *@return 
	 true if success or false
	 */
	public static function DeleteTemplate(tpl:Number):Boolean
	{
		return ExternalInterface.call("vvd_DeleteTemplate",tpl);
	}
	
	/**
	 *get template path(e.g."c:\\a.swf")
	 *@param 
	 tpl template index
	 *@return 
	 template path
	 */
	public static function GetTemplatePath(tpl:Number):String
	{
		return ExternalInterface.call("vvd_GetTemplatePath",tpl);
	}
	
	/**
	 *get template image file path(如"c:\\a.jpg")
	 *@param 
	 tpl template index
	 *@return 
	 iamge file path
	 */
	public static function GetThumbPath(tpl:Number):String
	{
		return ExternalInterface.call("vvd_GetThumbPath",tpl);
	}
	
	/**
	 *get total photo number of current vvd file
	 *@return 
	 total photo number
	 */
	public static function GetPhotoNum():Number
	{
		return ExternalInterface.call("vvd_GetPhotoNum");
	}
	
	/**
	 *insert new photo to current vvd file
	 *@param 
	 photo photo index
	 *@param 
	 path inserted photo path
	 *@return 
	 new photo index if success or -1
	 */
	public static function InsertPhoto(photo:Number,path:String):Number
	{
		return ExternalInterface.call("vvd_InsertPhoto",photo,path);
	}
	
	/**
	 *delete one photo
	 *@param 
	 photo photo index
	 *@return
	 true if success or false
	 */
	public static function DeletePhoto(photo:Number):Boolean
	{
		return ExternalInterface.call("vvd_DeletePhoto",photo);
	}
	
	/**
	 *get pointed photo path in current vvd file 
	 *@param 
	 	photo index
	 *@return 
	 	photo path in the vvd file
	 */
	public static function GetPhotoPath(photo:Number):String
	{
		return ExternalInterface.call("vvd_GetPhotoPath",photo);
	}
	
	/**
	 *get music file number in current vvd file
	 *@return 
	 total music file number
	 */
	public static function GetMusicNum():Number
	{
		return ExternalInterface.call("vvd_GetMusicNum");
	}
	
	/**
	 *insert a new music before the pointed position
	 *@param 
	 	music the pointed position
	 *@param 
	 	path new music file path
	 *@return 
	 	new music file index if success or -1
	 */
	public static function InsertMusic(music:Number,path:String):Number
	{
		return ExternalInterface.call("vvd_InsertMusic",music,path);
	}
	
	/**
	 *delete one music from current vvd file
	 *@param 
	 	music music index
	 *@return 
	 	true if success or false
	 */
	public static function DeleteMusic(music:Number):Boolean
	{
		return ExternalInterface.call("vvd_DeleteMusic",music);
	}
	
	/**
	 *get the pointed music file path
	 *@param 
	 	music the pointed index
	 *@return 
	 	the path
	 */
	public static function GetMusicPath(music:Number):String
	{
		return ExternalInterface.call("vvd_GetMusicPath",music);
	}
	
	/**
	 *get the file path to be save
	 *@return 
	 	vvd file path
	 */
	public static function GetSavePath():String
	{
		return ExternalInterface.call("vvd_GetSavePath");
	}
	
	/**
	 * play the given vvd file
	 *@param 
	 	mode play mode
	 */
	public static function Play(mode:Number)
	{
		return ExternalInterface.call("vvd_Play",mode);
	}
	
	/**
	 * stop play vvd
	 */
	public static function Stop()
	{
		return ExternalInterface.call("vvd_Stop");
	}
	
	/**
	 * pause current vvd 
	 */
	public static function Pause()
	{
		return ExternalInterface.call("vvd_Pause");
	}
	
	/**
	 * resume play current vvd if paused
	 */
	public static function Resume()
	{
		return ExternalInterface.call("vvd_Resume");
	}
	
	/**
	 * get current play status
	 *@return 
	 	play status
	 *@see VVD_IDLE
	 *@see VVD_PLAY
	 *@see VVD_PAUSE
	 */
	public static function GetState():Number
	{
		return ExternalInterface.call("vvd_GetState");
	}
	
	/**
	 * get vvd play mode
	 *@return 
	   VVD play mode
	 *@see VVD_PLAYMODE_ONCE
	 *@see VVD_PLAYMODE_REPEAT
	 */
	public static function GetPlayMode():Number
	{
		return ExternalInterface.call("vvd_GetPlayMode");
	}
	
	/**
	 * set VVD play mode
	 *@param 
	 	mode mode to be set
	 *@see VVD_PLAYMODE_ONCE
	 *@see VVD_PLAYMODE_REPEAT
	 */
	public static function SetPlayMode(mode:Number):Number
	{
		return ExternalInterface.call("vvd_SetPlayMode",mode);
	}									
	
	/**
	 * play independent flash
	 *@param 
	 	path path of flash
	 */
	public static function PlayFlash(path:String)
	{
		return ExternalInterface.call("vvd_PlayFlash",path);
	}									
	
	/**
	 * stop independent flash playing
	 */
	public static function StopFlash()
	{
		return ExternalInterface.call("vvd_StopFlash");
	}									
 }
 