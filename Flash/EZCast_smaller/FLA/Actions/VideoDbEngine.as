dynamic class Actions.VideoDbEngine {
	
	public static var DBVIDEO_SORT_BY_NAME=0;
	public static var DBVIDEO_SORT_BY_FILESIZE=1;
	public static var DBVIDEO_SORT_BY_FILEEXTENSION=2;
	public static var DBVIDEO_SORT_BY_TIME=3;
	/**
	 *@brief Open the video database.
	 *@param[in] path : the data base path.
	 *
	 *@return 
	 *  0 for success.
	 *  1 for failed.
	 */
	public static function Open(path:String):Number
	{
		
		return ExternalInterface.call("video_db_open__c",path);
	}

	/**
	 *@brief Close the video database.
	 *@param[in] NULL
	 *@return NULL
	 */
	public static function Close():Void
	{
		
		ExternalInterface.call("video_db_close__c");
	}


	/**
	 *@brief Get the total number of videos
	 *@param[in] NULL
	 *@return the total number
	**/
	public static function getVideoNumber():Number
	{
		return ExternalInterface.call("video_db_get_video_number__c");
	}
	
	/**
	 *@brief Sort the Video.
	 *@param[in] sortType : sort type that wanted.
	 *@return 
	 *  if success return the total number that sorted.
	 *  if failed return 0.
	 */
	public static function sortVideo(sortType:Number):Number
	{
		return ExternalInterface.call("video_db_sort__c",sortType);
	}

	/**
	 *@brief Get information after video has been sorted.
	 *@param[in] index: the index of the item.
	 *@return 
	 *  if success will return a string containing the item information.
	 *  if failed will return a string "unknown"
	 */
	public static function getSortedVideoItemInfo(index:Number):String
	{
		return ExternalInterface.call("video_db_get_info_from_sorted__c",index);
	}
	
	/**
	 *@brief Create the video database.
	 *@param[in] filesDir: The video files directory.
	 *@param[in] dbFullPath: The full path of the database file including the name.
	 *@return 
	 *  if success will return 0.
	 *  if failed will return 1.
	 */
	public static function Create(filesDir:String,dbFullPath:String):Number
	{
		
		return ExternalInterface.call("video_db_create__c",filesDir,dbFullPath);
	}
	
	/**
	 *@brief Get the creating process.
	 *@return Will return the process like 10,50,90,etc. which represent the percentage.
	 */
	public static function getCreateStatus():Number
	{
		
		return ExternalInterface.call("video_db_get_create_stat__c");
	}
	
	/**
	 *@brief Stop the creating if impatient.
	 *
	 *@note After calling Stop(), you still need to call the getCreateStatus() to make sure 100 has returned.
	 */
	public static function Stop():Void
	{
		
		ExternalInterface.call("video_db_stop_create__c");
	}
	
	/**
	 *@brief Check if the database exists.
	 *@param[in] path: The full path of the database file including the name.
	 *@return 
	 *  if exist will return 0.
	 *  if not exist will return 1.
	 */
	public static function Exist(path:String):Number
	{
	
		return ExternalInterface.call("video_db_check_database__c",path);
	}
	
	/**
	 *@brief Delete a database file.
	 *@param[in] path: The full path of the database file including the name.
	 *@return 
	 *  if success will return 0.
	 *  if fail will return 1.
	 */
	public static function Delete(path:String):Number
	{
		return ExternalInterface.call("video_db_delete__c",path);
	}
	
	
}
