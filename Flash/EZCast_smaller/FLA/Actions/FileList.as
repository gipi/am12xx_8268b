
/**
 *	@author   wuxiaowen
 *	@version 1.0
 *  @date: 2009/06/08
 *	<pre> 
 *  FileList Class : 
 *	Access to Actions Micro's file system
 *  Example:
 *	FileList.setPath("c:\\","mp3 jpg bmp");
 *	File_num = FileList.getTotal();	
 *	For(i=0;i< File_num;i++){
 *  	File_name = FileList.getFileName(i);
 *		File_path = FileList.getFilePath(i);
 *		trace(File_name);
 *		trace(File_path);
 *	}</pre>
 */


 
dynamic class Actions.FileList {

/**
*@addtogroup FileList_as
*@{
*/
	
	/**file type:common directory*/
	public static var FT_DIR	=0;   							///<it is a directory
	/**file type:common file*/
	public static var FT_FILE	=1; 								///<it is a common file 
	/**file type:music file*/
	public static var FT_MUSIC	=2;								///<it is a music file
	/**file type:video file*/
	public static var FT_VIDEO	=3;								///< it is a video file
	/**file type:image file*/
	public static var FT_IMAGE	=4;								///<it is a image file
	/**file type:vvd file*/
	public static var FT_VVD	=5;								///<it is a vvd file
	/**file type:txt file*/
	public static var FT_TXT	=6;								///<it is a txt file
	
	/** new open file list mode */
	public static var FL_MODE_FILES_BROWSE		=0;  			///<all the files with the specified extension will be included
	public static var FL_MODE_DIR_BORWSE_VALID	=1; 				///<only the valid file and the directory which include the vaild file will be listed
	
	/** daemon task status */
	public static var FL_STAT_NO_SENSE			=0;       			///< it is an initial value 
	public static var FL_STAT_FIND_NOFILE			=1;				///<no file will be found, 
	public static var FL_STAT_FIND_FIRST_FILE		=2;  			///<the first file is found
	public static var FL_STAT_FIND_FILE			=3;				///<there are files be found
	public static var FL_STAT_FIND_COMPLETE		=4;				///<the scan is completed
	
	
	/** 
	* file system error type
	*/
	/** no error */
	public static var FS_ERR_OK:Number 			= 0;			///<the file operation is succeed
	/** path error */
	public static var FS_ERR_Path:Number 		= 2;			///<the path is error
	/** file exists */
	public static var FS_ERR_FileExist:Number 	= 3;			///<the file is exist
	/** disk full */
	public static var FS_ERR_DiskFull:Number 	= 5;			///<the disk is full
	/** other unknown error */
	public static var FS_ERR_Unknown:Number 	= 15;		///<other file operation error
	
	/**file operation error**/
	
	/*file copy del ok*/
	public static var FS_OK 		= 0;		///<copy succeed		
	/*fs full*/
	public static var FS_ERR 	= 1;		///<copy failed
	/*fs exist*/
	public static var FS_EXIST 	=2;		///<file exist
	/*fs readonly*/
	public static var FS_RO 		=3;		///<file readonly

	/**Work Status   only used under linux**/
	public static var WORK_IN_PROCESS				=0;		 ///<the background task is working
	public static var WORK_FINISH_OK				=1;		 ///<the background task is finish
	public static var WORK_STOPED_FILE_EXSIT		=2;		///<the background task is stopped because the file is exist
	public static var WORK_STOPED_RDONLY_FS		=3;		///<the background task is stopped because the file is readonly
	public static var WORK_STOPED_IO_ERROR		=4;		///<the background task is stopped because of  the error of IO operation
	public static var WORK_STOPED_INVALID_ARG	=5;		///<the background task is stopped because the parameters is invalid		
	public static var WORK_STOPED_USER_CANCEL	=6;		///<the background task is stopped because of the cancel from user 
	public static var WORK_STOPED_DISK_FULL		=7;		///<the background task is stopped because the disk is full

	/**search status by program thread**/
	public static var scan_status_inti_status=0	;	///the initation scan status
	public static var find_the_first_file=1;/////find the  first file
	public static var search_completely=2;///search all the files
	/**
     *@brief set the current working path
     *@param[in] path	: working path, utf-8 format, if it is NULL, default is internal memory
     *@prame[in] filetype	: the file extension
     *@return
     		- true if success
     		- false if fail  
     **/
	public static var do_sort_by_name=0;		///dosort by filename
	public static var do_sort_by_time=1;	//dosort by file time
	
	public static function setPath(path:String,filetype:String):Boolean
	{
		return ExternalInterface.call("fl_setPath", path,filetype);
	}

	/**
	*@brief  release current working path which is the para of setPath
	*@param[in]  none
	*@return
		- true:succeed
		- false:failed
	*@see setPath()
     	**/
	public static function releasePath():Boolean
	{
		return ExternalInterface.call("fl_releasePath");
	}
	
	/**
	*@brief  get the working path
	*@param[in] none
	*@return the working path, utf-8 format
	**/
	public static function getPath():String
	{
		return ExternalInterface.call("fl_getPath");
	}

	/**
	*@brief  get the total file number under the path which be set including sub directories
	*@param[in]  none
	*@return total number
	*@see setPath()
	*/
	public static function getTotal():Number
	{
		return ExternalInterface.call("fl_getTotal");
	}
	
	/**
	 *@brief get the selected file type
	 *@param[in] i	: file index, the range from 0 to the num which is return from getTotal()
	 *@return type,FT_DIR etc
	 *@see FT_DIR etc
	 **/
	public static function getFileType(i:Number):Number
	{
		return ExternalInterface.call("fl_getFileType", i);
	}
	
	/**
	*@brief enter sub directory,make sure  the file type is FT_DIR before calling this function
	*@param[in] i		: file index, the range from 0 to the num which is return from getTotal()
	*@return
		- true:succeed
		- false:failed
	*@see FT_DIR,getTotal()
	*/
	public static function enterDir(i:Number):Boolean
	{
		return ExternalInterface.call("fl_enterDir", i);
	}	
	
	/**
	*@brief  return to previous directory
	*@param[in]  none
	*@return
		- true:succeed
		- false:failed
	*/
	public static function exitDir():Boolean
	{
		return ExternalInterface.call("fl_exitDir");
	}	
	
	/**
	*@brief  get the selected file name
	*@param[in]  i 	: file index,the range from 0 to the num which is return from getTotal()
	*@return file name with utf-8 format
	**/
	public static function getFileName(i:Number):String
	{
		return ExternalInterface.call("fl_getFileName", i);
	}
	
	/**
	*@brief  get the full path of the file with index i
	*@param[in] i	: file index,the range from 0 to the num which is return from getTotal()
	*@return full path with utf-8 format
	**/
	public static function getFilePath(i:Number):String
	{
		return ExternalInterface.call("fl_getFilePath", i);
	}
	
	/**
	*@brief get the last modified time of the file with index i
	*@param[in] i	: file index,the range from 0 to the num which is return from getTotal()
	*@return  the time with a "Date object" format
	**/
	public static function getFileTime(i:Number):String
	{
		return ExternalInterface.call("fl_getFileTime", i);
	}
	
	
	/**
	*@brief get the total directory number under current directory
	*@param[in] i	: file index,the range from 0 to the num which is return from getTotal()
	*@return  total directory number
	**/
	public static function getDirNum(i:Number):Number
	{
		return ExternalInterface.call("fl_getDirNum", i);
	}	
	
	/**
	*@brief  get the file size
	*@param[in] i	: file index,the range from 0 to the num which is return from getTotal()
	*@return file size by BYTES
	**/
	public static function getFileSize(i:Number):Number
	{
		return ExternalInterface.call("fl_getFileSize", i);
	}
	
	/**
	*@brief get sub directory's file total number
	*@param[in] i	: file index,the range from 0 to the num which is return from getTotal()
	*@return file total, if it is not a file with type FT_DIR then return -1
	**/
	public static function getSubDirTotal(i:Number):Number
	{
		return ExternalInterface.call("fl_getSubDirTotal", i);
	}
	
	/**
	*@brief  copy a file to other media
	*@param[in] i		: file index,the range from 0 to the num which is return from getTotal()
	*@param[in] des	: the destination where the file be stored
	*@return if success return true or false number
	*@see getLastProcess(),getLastStatus(),stopWork()
	**/
	public static function copyFile(i:Number,des:String):Number
	{
		return ExternalInterface.call("fl_copyFile", i,des);
	}

	/**
	*@brief  overlap the file if the file in the destination is exist
	*@param[in] i		: file index,the range from 0 to the num which is return from getTotal()
	*@return if success return true or false number
	*@see getLastProcess(),getLastStatus(),stopWork()
	**/
	public static function overlapCopy(i:Number,des:String):Number
	{
		return ExternalInterface.call("fl_overlapCopy",i,des);
	}

	/**
	*@brief  copy one dir,make sure  the file type is FT_DIR before calling this function
	*@param[in] i		: file index,the range from 0 to the num which is return from getTotal()
	*@param[in] des	: destination like "/mnt/udisk/"
	*@return	if success return true or false number
	*@see getLastProcess(),getLastStatus(),stopWork()
	**/
	public static function copyDir(i:Number,des:String):Number
	{
		return ExternalInterface.call("fl_copyDir",i,des)
	}
	 
	/**
	*@brief  delete one file
	*@param[in] i		: file index,the range from 0 to the num which is return from getTotal()
	*@return   if success return true or false
	*@see getLastProcess(),getLastStatus(),stopWork()
	**/
	public static function deleteFile(i:Number):Number
	{
		return ExternalInterface.call("fl_deleteFile", i);
	}
	
	/**
	*@brief  del one dir
	*@param[in] i		: file index,the range from 0 to the num which is return from getTotal()
	*@return if success return true or false number
	*@see getLastProcess(), getLastStatus(),stopWork()
	**/
	public static function deleteDir(i:Number):Number
	{
		return ExternalInterface.call("fl_deleteDir",i);
	}
	
	/**
	*@brief get last process after calling the function copyFile,overlapCopy,copyDir,deleteFile,deleteDir
	*@param[in] none
	*@return 
		- if success return process value
		- -1: something wrong is happened, call getLastStatus() to get the reason
		- -2: the background task had been killed
	*@see getLastStatus(),stopWork()
	**/
	public static function getLastProcess():Number
	{
		return ExternalInterface.call("fl_get_process");
	}

	/**
	*@brief get the status of the background task after calling the function copyFile,overlapCopy,copyDir,deleteFile,deleteDir
	*@param[in] none
	*@return WORK_IN_PROCESS etc. 
	*@see WORK_IN_PROCESS etc.
	**/
	public static function getLastStatus():Number
	{
		return ExternalInterface.call("fl_get_status");
	}

	/**
	*@brief  stop the background task
	*@param[in] none
	*@return  always 0
	**/
	public static function stopWork():Number
	{
		return ExternalInterface.call("fl_stop_work");
	}
	

	
	/**
	*@brief  new interface for file list, mainly used for searching files by a daemon task.
	*@param[in] path	: file list path
	*@param[in] flext	: file extension
	*@param[in] flmode	:
	* 		- FL_MODE_FILES_BROWSE
	* 		- FL_MODE_DIR_BORWSE_ALL
	* 		- FL_MODE_DIR_BORWSE_VALID
	*		- FL_MODE_DIR_BORWSE_HIDE
	*@param[in] flscan	: need daemon task or not
	*@return 
	*	- a handle point to file list if succeed
	* 	- -1 if failed
	*/
	public static function setPathNew(path:String,flext:String,flmode:Number,flscan:Number):Number
	{
		return ExternalInterface.call("fl_setPathNew",path,flext,flmode,flscan);
	}
	
	/**
	*@brief new interface for getting the  file-search daemon task's status
	*@param[in] flhandle	: file list handle
	*@return task status	: FL_STAT_FIND_COMPLETE etc
	*@see FL_STAT_FIND_COMPLETE etc		
	*/
	public static function getScanTaskStatNew(flhandle:Number):Number
	{
		return ExternalInterface.call("fl_getScanTaskStatNew",flhandle);
	}
	
	/**
	*@brief  new interface for getting file path
	*@param[in] flhandle	: file list handle
	*@param[in] index		: index of the file
	*@param[in] fbuf		: need extern buffer for store file name,1-->need 0-->not need
	*@return  file path identified by findex
	* 		
	*/
	public static function getPathNew(flhandle:Number,findex:Number,fbuf:Number):String
	{
		return ExternalInterface.call("fl_getPathNew",flhandle,findex,fbuf);
	}
	
	
	/**
	*@brief new interface for release file handle
	*@param[in] flhandle	: file list handle
	*/
	public static function releasePathNew(flhandle:Number):Void
	{
		ExternalInterface.call("fl_releasePathNew",flhandle);
	}
	
	
	/**
	*@brief 	new interface for getting file total number
	*@param[in] flhandle	: file list handle
	*/
	public static function getTotalNew(flhandle:Number):Number
	{
		return ExternalInterface.call("fl_getTotalNew",flhandle);
	}
	
	/**
	*@brief	 new interface for getting file's long name
	*@param[in] flhandle	: file list handle
	*@param[in] findex	: index of the file
	*@return file name identified by findex
	* 		
	*/
	public static function getLongnameNew(flhandle:Number,findex:Number):String
	{
		return ExternalInterface.call("fl_getLongnameNew",flhandle,findex);
	}
	
	/**
	*@brief new interface for getting file size
	*@param[in] flhandle	: file list handle
	*@param[in] findex	: index of the file
	*@return size of the file
	* 		
	*/
	public static function getFileSizeNew(flhandle:Number,findex:Number):Number
	{
		return ExternalInterface.call("fl_getFileSizeNew",flhandle,findex);
	}
	
	
	/**
	*@brief new interface for getting file last modified time
	*@param[in] flhandle	: file list handle
	*@param[in] findex		: index of the file
	*@return last modified time of the file
	* 		
	*/
	public static function getFileTimeNew(flhandle:Number,findex:Number):Number
	{
		return ExternalInterface.call("fl_getFileTimeNew",flhandle,findex);
	}


	/**
	*@brief new interface for copy a file
	*@param[in] flhandle	: file list handle
	*@param[in] findex	: index of the file
	*@param[in] fpath		: destination path
	*@return 0 if succeed
	* 		
	*/
	public static function copyFileNew(flhandle:Number,findex:Number,fpath:String):Number
	{
		return ExternalInterface.call("fl_copyFileNew",flhandle,findex,fpath);
	}
	
	/**
	*@brief new interface for overlap copy a file
	*@param[in] flhandle	: file list handle
	*@param[in] findex	: index of the file
	*@param[in] fpath		: destination path
	*@return 0 if succeed
	* 		
	*/
	public static function overlapCopyNew(flhandle:Number,findex:Number,fpath:String):Number
	{
		return ExternalInterface.call("fl_overlapCopyNew",flhandle,findex,fpath);
	}
	
	/**
	*@brief new interface for deleting a file
	*@param[in] flhandle	: file list handle
	*@param[in] findex	: index of the file
	*@return 0 if succeed
	* 		
	*/
	public static function deleteFileNew(flhandle:Number,findex:Number):Number
	{
		return ExternalInterface.call("fl_deleteFileNew",flhandle,findex);
	}

	/**
	*@brief new interface for entering a dir
	*@param[in] flhandle	: file list handle
	*@param[in] findex	: index of the file
	*@return
		- 0 :succeed
		- -1:failed
	*/
	public static function enterDirNew(flhandle:Number,findex:Number):Number
	{
		return ExternalInterface.call("fl_enterDirNew",flhandle,findex);
	}


	/**
	*@brief new interface for exiting a dir
	*@param[in]: flhandle	: file list handle
	*@param[in]: findex	: index of the file
	*@return
		- 0 :succeed
		- -1:failed
	*/
	public static function exitDirNew(flhandle:Number,findex:Number):Number
	{
		return ExternalInterface.call("fl_eixtDirNew",flhandle,findex);
	}

	/**
	*@brief  new interface for getting  the type of the index
	*@param[in] flhandle	: file list handle
	*@param[in] findex	: index of the file
	*@return
		- 0 :succeed, this file is a directory
		- -1:failed
	*/
	public static function getFileTypeNew(flhandle:Number,findex:Number):Number
	{
		return ExternalInterface.call("fl_getFileTypeNew",flhandle,findex);
	}

	/**
	*@brief  call this funciton after calling deleteFileNew to fresh the filelist
	*@param[in] flhandle	: the file list handle
	*@return 0 if success
	**/
	public static function refreshFileList(fileindex:Number):Number
	{
		return ExternalInterface.call("fl_freshFileList",fileindex);
	}

	/**
	*@brief  call this funciton after calling deleteFileNew to fresh the filelist
	*@param[in] flhandle	: the file list handle
	*@param[in] flhandle	: index of the deleted file
	*@return 0 if success
	**/
	public static function refreshFileListNew(flhandle:Number,findex:Number):Number
	{
		return ExternalInterface.call("fl_freshFileListNew",flhandle,findex);
	}


	/**
	*@brief  call this funciton to creat a sort view
	*@param[in] flhandle	: the file list handle
	*@return 0 if success
	**/
	public static function attachSortview():Number
	{
		return ExternalInterface.call("fl_attachSortview");
	}

	/**
	*@brief  call this funciton to delete a sort view
	*@param[in] flhandle	: the file list handle
	*@return 0 if success
	**/
	public static function detachSortview():Number
	{
		return ExternalInterface.call("fl_detachSortview");
	}


	/**
	*@brief  call this funciton to attach a sort view
	*@param[in] flhandle	: the file list handle
	*@return 0 if success
	**/
	public static function attachSortviewNew(flhandle:Number):Number
	{
		return ExternalInterface.call("fl_attachSortviewNew",flhandle);
	}

	
	/**
	*@brief  call this funciton to delete a sort view
	*@param[in] flhandle	: the file list handle
	*@return 0 if success
	**/
	public static function detachSortviewNew(flhandle:Number):Number
	{
		return ExternalInterface.call("fl_detachSortviewNew",flhandle);
	}
	
	public static function processudisk()
	{
		 ExternalInterface.call("fl_processudisk");
	}
	/**
	*@brief  call this funciton to put searchflag before setpath,then the MNAVI will just search current path
	*@param[in] flag	: the search flag:
		1 :search current path
		0:search current path include child path
	**/
	public static function putsearchflag(flag:Number)
	{
		 ExternalInterface.call("fl_putsearchflag",flag);
	}
	/**
		*@brief  new interface for file list, mainly used for searching files by a thread.
		*@param[in] path	: file list path
		*@param[in] flext	: file extension
		*@param[in] flmode	:
		*		- FL_MODE_FILES_BROWSE
		*		- FL_MODE_DIR_BORWSE_ALL
		*		- FL_MODE_DIR_BORWSE_VALID
		*		- FL_MODE_DIR_BORWSE_HIDE
		*@param[in] flscan	: need daemon task or not
		*@return 
		*	- a handle point to file list if succeed
		*	- -1 if failed
		*/
	public static function setPathdeamon(path:String,flext:String,flmode:Number,flscan:Number):Number
	{
			return ExternalInterface.call("fl_setPathNewdeamon",path,flext,flmode,flscan);
	}
	/**
		*@brief  stop the deamon search files task
		*@param[in] flhandle	: the file list handle
		*@return NULL
			
	*/
	public static function stopDeamonTask(flhandle:Number)
	{
		 ExternalInterface.call("fl_stoptaskdeamon",flhandle);
	}

	/**
		*@brief  do sort by key_word
		*@param[in] flhandle	: the file list handle
		*@param[in] key_word: the key word ,such as do_sort_by_name,do_sort_by_time
		
		*@return NULL
			
	*/
	public static function attachSortviewBykeyword(flhandle:Number,key_word:Number)
	{
		return ExternalInterface.call("fl_attachSortviewNewbykey",flhandle,key_word);
	}
	
	  public static function set_serch_path_type(path:String,type:String):Number
	{
           // trace("set_serch_path_type");
		 	return  ExternalInterface.call("fl_set_serch_path_type",path,type);
		 
	}
	
     public static function creat_search_file_thread():Number
	{
			//  trace("creat_search_file_thread");
		 	return  ExternalInterface.call("fl_creat_search_file_thread");
		 
	}
	 public static function creat_default_info_file():Number
	{
			//  trace("creat_search_file_thread");
		 	return  ExternalInterface.call("fl_creat_default_info_file");
		 
	}
	
/**
 *@}
 */	
}
