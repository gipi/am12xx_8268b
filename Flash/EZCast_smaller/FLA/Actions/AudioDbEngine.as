dynamic class Actions.AudioDbEngine {
	
	public static var DBAUDIO_SORT_BY_NAME=0;
	public static var DBAUDIO_SORT_BY_FILESIZE=1;
	public static var DBAUDIO_SORT_BY_FILEEXTENSION=2;
	public static var DBAUDIO_SORT_BY_ALBUM=3;
	public static var DBAUDIO_SORT_BY_GENRE=4;
	public static var DBAUDIO_SORT_BY_YEAR=5;
	public static var DBAUDIO_SORT_BY_ARTIST=6;
	public static var DBAUDIO_SORT_BY_COMPOSER=7;
	public static var DBAUDIO_SORT_BY_SINGER=8;
	public static var DBAUDIO_SORT_BY_TRACK_NUMBER=9;
	public static var DBAUDIO_SORT_BY_TOTAL_TIME=10;

	/**
	 *@brief Open the audio database.
	 *@param[in] path : the data base path.
	 *
	 *@return 
	 *  0 for success.
	 *  1 for failed.
	 */
	public static function Open(path:String):Number
	{
		return ExternalInterface.call("audio_db_open__c",path);
	}

	/**
	 *@brief Close the audio database.
	 *@param[in] NULL
	 *@return NULL
	 */
	public static function Close():Void
	{
		ExternalInterface.call("audio_db_close__c");
	}

	/**
	 *@brief Get the number of albums.
	 *@param[in] NULL
	 *@return 
	 *  return the total album number.
	 */
	public static function getAlbumNumber():Number
	{
		return ExternalInterface.call("audio_db_get_album_number__c");
	}

	/**
	 *@brief Get one album information.
	 *@param[in] index : the range of index is 0..[totalAlbumNumer-1].
	 *@return 
	 *  if success will return a String like <AlbumName>=...;<TotalSongs>=...;
	 *    which contains the album name and total songs in this album.
	 *  if failed will return a string "unknow".
	 */
	public static function getAlbumInfo(index:Number):String
	{
		return ExternalInterface.call("audio_db_get_album_info__c",index);
	}
	
	/**
	 *@brief Sort the album.
	 *@param[in] albumName : The name of the album to be sorted.
	 *@param[in] sortType : sort type that wanted.
	 *@return 
	 *  if success return the total number that sorted.
	 *  if failed return 0.
	 */
	public static function sortAlbum(albumName:String,sortType:Number):Number
	{
		return ExternalInterface.call("audio_db_sort_album__c",albumName,sortType);
	}

	/**
	 *@brief Get information after album has been sorted.
	 *@param[in] albumName : the album name.
	 *@param[in] index: the index of the item.
	 *@return 
	 *  if success will return a string containing the item information.
	 *  if failed will return a string "unknown"
	 */
	public static function getSortedAlbumItemInfo(albumName:String,index:Number):String
	{
		return ExternalInterface.call("audio_db_get_info_from_sorted_album__c",albumName,index);
	}

	/**
	 *@brief Get the number of artist.
	 *@param[in] NULL
	 *@return 
	 *  return the total artist number.
	 */
	public static function getArtistNumber():Number
	{
		return ExternalInterface.call("audio_db_get_artist_number__c");
	}

	/**
	 *@brief Get one artist information.
	 *@param[in] index : the range of index is 0..[totalArtist-1].
	 *@return 
	 *  if success will return a String like <ArtistName>=%s;<TotalSongs>=%d;<TotalAlbums>=%d;
	 *    which contains the artist name and total songs in this artist and total albums in
	 *    this artist.
	 *  if failed will return a string "unknown".
	 */
	public static function getArtistInfo(index:Number):String
	{
		return ExternalInterface.call("audio_db_get_artist_info__c",index);
	}
	
	/**
	 *@brief Sort the artist.
	 *@param[in] artistName : The name of the artist to be sorted.
	 *@param[in] sortType : sort type that wanted.
	 *@return 
	 *  if success return the total number that sorted.
	 *  if failed return 0.
	 */
	public static function sortArtist(artistName:String,sortType:Number):Number
	{
		return ExternalInterface.call("audio_db_sort_artist__c",artistName,sortType);
	}

	/**
	 *@brief Get information after artist has been sorted.
	 *@param[in] artistName : the artist name.
	 *@param[in] index: the index of the item.
	 *@return 
	 *  if success will return a string containing the item information.
	 *  if failed will return a string "unknown"
	 */
	public static function getSortedArtistItemInfo(artistName:String,index:Number):String
	{
		return ExternalInterface.call("audio_db_get_info_from_sorted_artist__c",artistName,index);
	}
	

	/**
	 *@brief Get the number of genre.
	 *@param[in] NULL
	 *@return 
	 *  return the total genre number.
	 */
	public static function getGenreNumber():Number
	{
		return ExternalInterface.call("audio_db_get_genre_number__c");
	}

	/**
	 *@brief Get one genre information.
	 *@param[in] index : the range of index is 0..[totalGenre-1].
	 *@return 
	 *  if success will return a String like <GenreName>=...;<TotalSongs>=...;<TotalAlbums>=..;
	 *    which contains the genre name and total songs in this artist and total albums in
	 *    this genre.
	 *  if failed will return a string "unknown".
	 */
	public static function getGenreInfo(index:Number):String
	{
		return ExternalInterface.call("audio_db_get_genre_info__c",index);
	}
	
	/**
	 *@brief Sort the genre.
	 *@param[in] genreName : The name of the genre to be sorted.
	 *@param[in] sortType : sort type that wanted.
	 *@return 
	 *  if success return the total number that sorted.
	 *  if failed return 0.
	 */
	public static function sortGenre(genreName:String,sortType:Number):Number
	{
		return ExternalInterface.call("audio_db_sort_genre__c",genreName,sortType);
	}

	/**
	 *@brief Get information after genre has been sorted.
	 *@param[in] genreName : the genre name.
	 *@param[in] index: the index of the item.
	 *@return 
	 *  if success will return a string containing the item information.
	 *  if failed will return a string "unknown"
	 */
	public static function getSortedGenreItemInfo(genreName:String,index:Number):String
	{
		return ExternalInterface.call("audio_db_get_info_from_sorted_genre__c",genreName,index);
	}

	/**
	 *@brief Get the number of songs.
	 *@param[in] NULL
	 *@return 
	 *  return the total song number.
	 */
	public static function getSongNumber():Number
	{
		return ExternalInterface.call("audio_db_get_song_number__c");
	}
	
	/**
	 *@brief Sort the songs.
	 *@param[in] sortType : sort type that wanted.
	 *@return 
	 *  if success return the total number that sorted.
	 *  if failed return 0.
	 */
	public static function sortSong(sortType:Number):Number
	{
		return ExternalInterface.call("audio_db_sort_song__c",sortType);
	}

	/**
	 *@brief Get information after song has been sorted.
	 *@param[in] index: the index of the item.
	 *@return 
	 *  if success will return a string containing the item information.
	 *  if failed will return a string "unknown"
	 */
	public static function getSortedSongItemInfo(index:Number):String
	{
		return ExternalInterface.call("audio_db_get_info_from_sorted_song__c",index);
	}
	
	/**
	 *@brief Create the audio database.
	 *@param[in] filesDir: The music files directory.
	 *@param[in] dbFullPath: The full path of the database file including the name.
	 *@return 
	 *  if success will return 0.
	 *  if failed will return 1.
	 */
	public static function Create(filesDir:String,dbFullPath:String):Number
	{
		return ExternalInterface.call("audio_db_create__c",filesDir,dbFullPath);
	}
	
	/**
	 *@brief Get the creating process.
	 *@return Will return the process like 10,50,90,etc. which represent the percentage.
	 */
	public static function getCreateStatus():Number
	{
		return ExternalInterface.call("audio_db_get_create_stat__c");
	}
	
	/**
	 *@brief Stop the creating if impatient.
	 *
	 *@note After calling Stop(), you still need to call the getCreateStatus() to make sure 100 has returned.
	 */
	public static function Stop():Void
	{
		ExternalInterface.call("audio_db_stop_create__c");
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
		return ExternalInterface.call("audio_db_check_database__c",path);
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
		return ExternalInterface.call("audio_db_delete__c",path);
	}
	
	
}