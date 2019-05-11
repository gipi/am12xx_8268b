/**
*		@author   sllin
*		@version 1.0
* 		@date: 2010/10/11
*	<pre> 
*  ftp client  Api Class : 
*	Access to Actions Micro's FtpClient
*  
*/



dynamic class Actions.FtpClient {

/**
*@addtogroup FtpClient_as
*@{
*/
	public static var FTP_CMD_FILETYPE 		=0;   ///<get file type
	public static var FTP_CMD_FILENAME		=1;	///<get file name
	public static var FTP_CMD_FILESIZE			=2;	///<get file size
	public static var FTP_CMD_FILECREATEDATE	=3; ///<get the date of file be created

	/**
	*@brief connect to the server
	*@param[in] ipAddr	: ip address to be connected, such as 192.168.1.1
	*@param[in] username	: the user used to login
	*@param[in] passWd	: password 
	*@param[in] rootPath	: use for extension, do nothing now
	*@return 
		- if failed	: 0
		- success	: 0
	**/
	public static function connectServer(ipAddr:String,userName:String,passWd:String,rootPath:String):Number
	{
		return ExternalInterface.call("ftp_connectServer",ipAddr,userName,passWd,rootPath);
	}

	/**
	*@brief disconnect from server
	*@param[in] none
	*@return if succeed	: 0 
	**/
	public static function disconnectServer():Number
	{
		return ExternalInterface.call("ftp_disconnectServer");
	}

	/**
	*@brief get the file num under current directory
	*@param[in] none
	*@return the file num under current directory
	**/
	public static function getFileNum():Number
	{
		return ExternalInterface.call("ftp_getFileNum");
	}

	/**
	*@brief get the file type of the index
	*@param[in] cmd	: FTP_CMD_FILETYPE etc
	*@param[in] idx	: the range of this value is from 0 to the value returned from getFileNum()
	*@return 
	* 	- if cmd==FTP_CMD_FILETYPE
			return "D" if it is a directory or "F" when it is a common file
	**/
	public static function getFileInfo(cmd:Number,idx:Number):String
	{
		return ExternalInterface.call("ftp_getFileInfo",cmd,idx);
	}

	/**
	*@brief down load the file
	*@param[in] idx		: the range of this value is from 0 to the value returned from getFileNum()
	*@param[in] newName	: the new file name
	*@return if succeed : 0 
	**/
	public static function donwloadFile(idx:Number,newName:String):Number
	{
		return ExternalInterface.call("ftp_downloadFile",idx,newName);
	}

	/**
	*@brief enter the dir,make sure the idx is a dir
	*@param[in] idx	: the range of this value is from 0 to the value returned from getFileNum()
	*@return if succeed: 0 
	**/
	public static function enterDir(idx:Number):Number
	{
		return ExternalInterface.call("ftp_enterDir",idx);
	}

	/**
	*@brief exit  the dir,make sure the idx is a dir
	*@param[in] none
	*@return if succeed: 0 
	**/
	public static function exitDir():Number
	{
		return ExternalInterface.call("ftp_exitDir");
	}

/**
 *@}
 */	

}
