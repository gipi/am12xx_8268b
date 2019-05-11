/**
*		@author   sllin
*		@version 1.0
* 		@date: 2010/05/25  
*	<pre> 
*  Printer Api Class : 
*	Access to Actions Micro's FileSystem
*  
*/


 dynamic class Actions.FileSystem {

 /**
*@addtogroup FileSystem_as
*@{
*/

 	public static var FS_ERR_OK 			= 0x00000000;				//normal 
	public static var FS_ERR_EOF 			= 0x00000001;				//the end of file
	public static var FS_ERR_Path 			= 0x00000002;				//the path is incorrect
	public static var FS_ERR_FileExist 		= 0x00000003;				//there is the same file name exits
	public static var FS_ERR_Unformat 		= 0x00000004;				//the disk is unformated
	public static var FS_ERR_DiskFull 		= 0x00000005;				//the disk is full
	public static var FS_ERR_DiskErr 		= 0x00000006;				//write or read disk error
	public static var FS_ERR_WriteOnly 		= 0x00000007;				//the file is opened with writeonly mode
	public static var FS_ERR_ReadOnly		= 0x00000008;				//the file is opened with readonly mode			
	public static var FS_ERR_FileInuse 		= 0x00000009;				//the file is using
	public static var FS_ERR_NoRes 			= 0x0000000a;				//the path is error
	public static var FS_ERR_COPYFILE_STOP= 0x0000000b;				//ap stop copying files
	public static var FS_ERR_DELFILE_STOP 	= 0x0000000c;				//ap stop deleting files
	public static var FS_ERR_COPYDIR_STOP = 0x0000000d;				//ap stop copying dir
	public static var FS_ERR_DELDIR_STOP 	= 0x0000000e;				//ap stop deleting dir
	/** other unknown error */
	public static var FS_ERR_Unknown 		= 0x0000000f;	
		
	/**
	@brief  remove a specified file from FileSystem
	@param[in] filefullpath	: the full path name of the specified file
	@return
	 	- 1:succeed
	 	- 0:falied
	 **/
 	public static function removeFile(filefullpath:String):Number
	{
		return ExternalInterface.call("fs_removeFile",filefullpath);
	}

	/**
	@brief copy a specified file to the specified dir
	@param[in] filefullpath	: the full path name of the file to be copied
	@param[in] destdir		: the dir where the file to be stored
	@return see the defination of FS_Err_OK etc.
	**/
	public static function copyFile(filefullpath:String,destdir:String):Number
	{
		return ExternalInterface.call("fs_copyFile",filefullpath,destdir);
	}


	/**
	@brief overlap the file in the destdir which has the same long name as the specified file
	@param[in] filefullpath	: the full path name of the file , it is a full path name get from filelist or medialib
	@param[in] destdir		: the dir where the file to be overlap
	@return
	  	- 0 :overlap succeed.
	  	- others:  failed
	**/
	public static function overlapFile(filefullpath:String,destdir:String):Number
	{
		return ExternalInterface.call("fs_overlapCopyFile",filefullpath,destdir);
	}

	/**
	@brief check whether there is a file in the destdir which has the same long name as the specified file
	@param[in] filefullpath	: the full path name of the file , it is a full path name get from filelist or medialib
	@param[in] destdir		: the dirname
	@return
	  	- 0 :the file exist 
	  	- others: file not exit
	**/
	public static function checkFileExist(filefullpath:String,destdir:String):Number
	{
		return ExternalInterface.call("fs_checkFileExist",filefullpath,destdir);
	}


	/**
	@brief get the short name of the file in the destdir which has the same long name as the specified file
	@param[in] filefullpath	: the full path name of the file , it is a full path name get from filelist or medialib
	@param[in] destdir		: the dirname
	@return
	  	- NULL :get the short name failed;
	  	- else : the short name
	**/
	public static function getShortName(filefullpath:String,destdir:String):String
	{
		return ExternalInterface.call("fs_getShortName",filefullpath,destdir);
	}

	/**
	@brief get  short full path of  specified file
	@param[in] filefullpath	: the full path name of the file,such as "C:\\1234567890\\1234hjh.jpg"
	@return the short name of full path
	**/
	public static function getShortFullPathName(filefullpath:String):String
	{
		return ExternalInterface.call("fs_getShortFullPath",filefullpath);
	}

	/**
	@brief make dir
	@param[in] dir_path	: the full path name of the dir which is to be created
	@return
		- 0 :make dir successfully
	  	- others: make dir failed
	**/
	public static function MakeDir(dir_path:String):Number
	{
		return ExternalInterface.call("fs_mkdir",dir_path);
	}

/**
 *@}
 */	

	
 }
