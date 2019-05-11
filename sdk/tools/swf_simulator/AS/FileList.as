
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
	
	/**file type:common directory*/
	public static var FT_DIR=0;
	/**file type:common file*/
	public static var FT_FILE=1;
	/**file type:music file*/
	public static var FT_MUSIC=2;
	/**file type:video file*/
	public static var FT_VIDEO=3;
	/**file type:image file*/
	public static var FT_IMAGE=4;
	/**file type:vvd file*/
	public static var FT_VVD=5;
	/**file type:txt file*/
	public static var FT_TXT=6;
		
	/**
     * set the current working path
     *@param 
         path working path, utf-8 format, if it is NULL, default is internal memory
	 *@return 
		true     -----> if success \n
		false	 -----> if fail  
     */
	public static function setPath(path:String,filetype:String):Boolean
	{
		return ExternalInterface.call("fl_setPath", path,filetype);
	}
	
	/**
	 * get the working path
	 * @return the working path, utf-8 format
	 */
	public static function getPath():String
	{
		return ExternalInterface.call("fl_getPath");
	}

	/**
     * get the total file number including sub directories
     * @return total number
     */
	public static function getTotal():Number
	{
		return ExternalInterface.call("fl_getTotal");
	}
	
	/**
	 * get the selected file type
	 *@param 
	 	i file index
	 *@return type
	 *@see FT_DIR
	 *@see FT_FILE
	 *@see FT_MUSIC
	 *@see FT_VIDEO
	 *@see FT_IMAGE
	 *@see FT_VVD
	 *@see FT_TXT
	 */
	public static function getFileType(i:Number):Number
	{
		return ExternalInterface.call("fl_getFileType", i);
	}
	
	/**
	 * enter sub directory
	 *@param 
	 	i file index
	 *@return 
	 	true if success or false
	 */
	public static function enterDir(i:Number):Boolean
	{
		return ExternalInterface.call("fl_enterDir", i);
	}	
	
	/**
	 * return to previous directory
	 * @return true if success or false
	 */
	public static function exitDir():Boolean
	{
		return ExternalInterface.call("fl_exitDir");
	}	
	
	/**
	 * get the selected file name
	 * @param 
	 	i file index
	 * @return file name with utf-8 format
	 */
	public static function getFileName(i:Number):String
	{
		return ExternalInterface.call("fl_getFileName", i);
	}
	
	/**
	 * get the full path of the file with index i
	 * @param 
	 	i file index
	 * @return full path with utf-8 format
	 */
	public static function getFilePath(i:Number):String
	{
		return ExternalInterface.call("fl_getFilePath", i);
	}
	
	/**
	 * get the last modified time of the file with index i
	 * @param 
	 	i file index
	 * @return return the time with a "Date object" format
	 */
	public static function getFileTime(i:Number):Number
	{
		return ExternalInterface.call("fl_getFileTime", i);
	}
	
	
	/**
	 * get the total directory number under current directory
	 * @param 
	 	i file index
	 * @return total directory number
	 */
	public static function getDirNum(i:Number):Number
	{
		return ExternalInterface.call("fl_getDirNum", i);
	}	
	
	/**
	 * get the file size
	 * @param 
	 	i file index
	 * @return file size by BYTES
	 */
	public static function getFileSize(i:Number):Number
	{
		return ExternalInterface.call("fl_getFileSize", i);
	}
	
	/**
	 * get sub directory's file total number
	 * @param 
	 	i file index
	 * @return file total, if it is not a file with type FT_DIR then return -1
	 */
	public static function getSubDirTotal(i:Number):Number
	{
		return ExternalInterface.call("fl_getSubDirTotal", i);
	}
	
	/**
	 * author of the file
	 * @param 
	 	i file index
	 * @return author with utf-8 format
	 */
	public static function getFileAuthor(i:Number):Number
	{
		return ExternalInterface.call("fl_getFileAuthor", i);
	}
	
	/**
	 * copy a file to internal memory
	 * @param 
	 	i file index
	 * @return if success return true or false
	 */
	public static function copyFile(i:Number):Boolean
	{
		return ExternalInterface.call("fl_copyFile", i);
	}
	 
	/**
	 * delete one file
	 * @param 
	 	i file index
	 * @return if success return true or false
	 */
	public static function deleteFile(i:Number):Boolean
	{
		return ExternalInterface.call("fl_deleteFile", i);
	}
	
	
}
