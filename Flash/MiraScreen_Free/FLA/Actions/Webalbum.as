/**
*		@author   sllin
*		@version 2.0
* 		@date: 2011/03/17  
*	<pre> 
*  medialib Api Class : 
*	Access to Actions Micro's webalbum
*  	
*/
//////code examples
/**


**/
dynamic class Actions.Webalbum {

/**
*@addtogroup Webalbum_as
*@{
**/
	/* used in set_login()*/
	public static var LOGIN_TYPE_PICASA=0;				 ///< use picasa account
	public static var LOGIN_TYPE_FACEBOOK=1;			///< use facebook account
	public static var LOGIN_TYPE_FLICKR_YAHOO=2;		///< user yahoo account to login flickr
	public static var LOGIN_TYPE_FLICKR_PICASA=3;		///< user picasa account to login flickr
	public static var LOGIN_TYPE_FLICKR_FACEBOOK=4;	///< user facebook account to login flickr

	/*returned from function*/
	public static var PICASA_RTN_OK=0;					///< nothing error happened
	public static var PICASA_RTN_ERR_LOGIN=1;			///< login error
	public static var PICASA_RTN_ERR_CREATALBUM=2;	///< create album error
	public static var PICASA_RTN_ERR_DELETEALBUM=3;	///< delete album error
	public static var PICASA_RTN_ERR_UPDATEALBUM=4;	///< update album info error
	public static var PICASA_RTN_ERR_UPLOAD_OPENFILE=5;	///< open file error when uploading
	public static var PICASA_RTN_ERR_UPLOADPHOTO=6;		///< upload photo error
	public static var PICASA_RTN_ERR_UPDATEPHOTO=7;		///< update photo info error
	public static var PICASA_RTN_ERR_DELETEPHOTO=8;		///< delete photo error
	public static var PICASA_RTN_ERR_DOWNLOADPHOTO=9;	///< download photo error
	public static var PICASA_RTN_ERR_MEMORY=10;			///<  memory error
	public static var PICASA_RTN_ERR_PARA=11;				///< parameters error
	public static var PICASA_RTN_CANCEL=12; 				///<  the process had been canceled

	/*returned from function*/
	public static var FACEBOOK_RTN_OK=0;					///< nothing error happened
	public static var FACEBOOK_RTN_ERR_LOGIN=1;				///< login error
	public static var FACEBOOK_RTN_ERR_CREATALBUM=2;		///< create album error
	public static var FACEBOOK_RTN_ERR_DELETEALBUM=3;		///< delete album error
	public static var FACEBOOK_RTN_ERR_UPDATEALBUM=4;		///< update album info error
	public static var FACEBOOK_RTN_ERR_UPLOAD_OPENFILE=5;	///< open file error when uploading
	public static var FACEBOOK_RTN_ERR_UPLOADPHOTO=6;		///< upload photo error
	public static var FACEBOOK_RTN_ERR_UPDATEPHOTO=7;		///< update photo info error
	public static var FACEBOOK_RTN_ERR_DELETEPHOTO=8;		///< delete photo error
	public static var FACEBOOK_RTN_ERR_DOWNLOADPHOTO=9;		///< download photo error
	public static var FACEBOOK_RTN_ERR_MEMORY=10;			///< memory error
	public static var FACEBOOK_RTN_ERR_PARA=11;				///< parameters error
	public static var FACEBOOK_RTN_CANCEL=12; 				///< the process had been canceled
	public static var FACEBOOK_RTN_ERR_LOGINPASSWORD=13;	///< incorrect email/password combination
	public static var FACEBOOK_RTN_ERR_ACCOUNTEXCEPTION=14;	///< user's account is temporarily unavailable
	public static var FACEBOOK_RTN_ERR_ACCOUNTCONFIRM=15;	///< user's account is needed to go to email to confirm

	public static var  CURLE_OK = 0;
	public static var  CURLE_UNSUPPORTED_PROTOCOL=1;    	///< the protocol is not supported
	public static var  CURLE_FAILED_INIT=2;           			///< init failed
	public static var  CURLE_COULDNT_RESOLVE_HOST=6;    	///< host error
	public static var  CURLE_OUT_OF_MEMORY=27;       		///< out of memory
	/* Note: CURLE_OUT_OF_MEMORY may sometimes indicate a conversion error
	       instead of a memory allocation error if CURL_DOES_CONVERSIONS
	       is defined
	*/
	public static var  CURLE_OPERATION_TIMEDOUT=28;     	///< time out when connetction

	public static var HttpStatus_OK=200;				///< No error.
	public static var HttpStatus_CREATED=201;		///< Creation of a resource was successful.
	public static var HttpStatus_FOUND=302;			///< The data requested actually resides under a different URL, however, the redirection may be altered on occasion as for "Forward".
	public static var HttpStatus_NOT_MODIFIED=304;	///< The resource hasn't changed since the time specified in the request's If-Modified-Since header.
	public static var HttpStatus_BAD_REQUEST=400;	///< Invalid request URI or header, or unsupported nonstandard parameter.
	public static var HttpStatus_UNAUTHORIZED=401;	///< Authorization required.
	public static var HttpStatus_FORBIDDEN=403;		///< Unsupported standard parameter, or authentication or authorization failed.
	public static var HttpStatus_NOT_FOUND=404;	///< Resource (such as a feed or entry) not found.
	public static var HttpStatus_CONFLICT=409;		///< Specified version number doesn't match resource's latest version number.
	public static var HttpStatus_GONE=410;			///< Requested change history is no longer available on the server. Refer to service-specific
	public static var HttpStatus_SERVER_ERROR=500;	///< Internal error. This is the default code that is used for all unrecognized server errors.

	/*used in get_info()*/
	public static var GetInfo_SrcType_FEED = 0;		///< src type is feed for picasa and flickr
	public static var GetInfo_SrcType_ENTRY= 1;		///< src type is entry for picasa
	public static var GetInfo_SrcType_DOWNLOAD_INFO=2; ///< src type is the download info for picasa
	public static var GetInfo_SrcType_UPLOADE_INFO=3;		///< src type is the upload info for picasa
	public static var GetInfo_SrcType_ERR_INIT_DOWNLOAD=4;///< if src type is this value , the src value is useless
	public static var PARA_TYPE_PHOTOALBUMS = 5;		///< src type is photoalbums for facebook and flickr
	public static var PARA_TYPE_SINGLEALBUM = 6;		///< src type is singlealbum for facebook and flickr
	public static var PARA_TYPE_SINGLEPHOTO = 7;		///< src type is singlephoto for facebook
	public static var PARA_TYPE_CONTACT = 8;			///< src type is contact for facebook and flickr
	public static var PARA_TYPE_SINGLEMEMBER = 9;		///< src type is singlemember for facebook


	///for SrcType_FEED
	public static var GetInfo_InfoType_FEED_ENTRYNUM = 0;	///< info type is entry num when the type is feed
	public static var GetInfo_InfoType_FEED_ALBUMID = 1; /// <album id
	public static var GetInfo_InfoType_FEED_ID = 2;	///< The ID of the current element.
	public static var GetInfo_InfoType_FEED_UPDATED = 3;///< updated time
	public static var GetInfo_InfoType_FEED_TITLE = 4;	///< title
	public static var GetInfo_InfoType_FEED_SUBTITLE = 5;///< sbutitle
	public static var GetInfo_InfoType_FEED_AUTHOR_NAME = 6;	///< author name
	public static var GetInfo_InfoType_FEED_USER = 7;	///< user, it is usually a number which is specified by server
	public static var GetInfo_InfoType_FEED_ACCESS = 8; ///< access ,public or private
	public static var GetInfo_InfoType_FEED_BYTESUSED = 9; ///< bytes had been used
	public static var GetInfo_InfoType_FEED_LOCATION = 10;	///< location
	public static var GetInfo_InfoType_FEED_NUMPHOTOS = 11;///<photos num in the account
	public static var GetInfo_InfoType_FEED_HEIGHT = 12;///< The height of the photo in pixels.
	public static var GetInfo_InfoType_FEED_WIDTH = 13;///< The width of the photo in pixels.
	public static var GetInfo_InfoType_FEED_TIMESTAMP = 14;///< The photo's timestamp yyyy-mm-dd-hh-mm-ss
	public static var GetInfo_InfoType_FEED_PHOTO_ADDR = 42;///< The photo's path under cache dir
	public static var GetInfo_InfoType_FEED_PTOTO_THUMB_ADDR = 43;///< The thumbnail photo's path under cache dir
	public static var FLICKR_GET_ACCOUNT_TYPE=0;
	public static var FLICKR_GET_LOGIN_EMAIL=1;
	public static var FLICKR_GET_LOGIN_PWD=2;
	public static var FLICKR_GET_PERMS=3;
	public static var FLICKR_GET_FROB=4;
	public static var FLICKR_GET_CONTACT_COUNT=7;
	
	///<for SrcType_ENTRY
	public static var ENTRY_EXIF_DISTANCE=15; ///<exif distance
	public static var ENTRY_EXIF_EXPOSURE=16;///<exif exposure
	public static var ENTRY_EXIF_FLASH=17;///< exif flash
	public static var ENTRY_EXIF_FOCALLENGTH=18;///< exif focallength
	public static var ENTRY_EXIF_FSTOP=19;///< exif fstop
	public static var ENTRY_EXIF_IMG_UNIQUEID=20;///<exif image unique id
	public static var ENTRY_EXIF_ISO=21;///< eixf iso
	public static var ENTRY_EXIF_MAKE=22;///< exif maker
	public static var ENTRY_EXIF_MODEL=23;///< exif model
	public static var ENTRY_EXIF_TIME=24;///< exif capture time
	public static var ENTRY_PUBLISHED=25;///<pubilsed time
	public static var ENTRY_UPDATED=26;///< update time
	public static var ENTRY_EDITED=27;///<edited time
	public static var ENTRY_RIGHT=28;///< rights
	public static var ENTRY_SUMMARY=29;///< summary
	public static var ENTRY_TITLE=30;///<title
	public static var ENTRY_AUTHOR_NAME=31;///<author name
	public static var ENTRY_WHERE_POS=32;///< position
	public static var ENTRY_ACCESS=33;///< access, this value may be the same as the rights
	public static var ENTRY_BYTESUSED=34;///<bytes had been used
	public static var ENTRY_LOCATION=35;///<location
	public static var ENTRY_NUMPHOTOS=36;///<photo num in the feed
	public static var ENTRY_NUMPHOTOREMAINING=37;///<remain num can be added
	public static var ENTRY_WIDHT=38;///< The height of the photo in pixels.
	public static var ENTRY_HEIGHT=39;///< The width of the photo in pixels.
	public static var ENTRY_TIMESTAMP=40;///< timestamp
	public static var ENTRY_ALBUMTITLE=41;///<album title
	public static var ENTRY_USERID=42;///<user id

	///<for srcType photoalbums of facebook and flickr
	public static var ALBUM_NUM = 0;			///<the num of albums
	public static var FLICKR_GET_PHOTOSET_ID=10;
	public static var FLICKR_GET_PHOTOSET_PHOTOS_COUNT=11;
	public static var FLICKR_GET_PHOTOSET_TITLE=12;
	public static var FLICKR_GET_PHOTOSET_DESCRIPTION=13;
	public static var FLICKR_GET_URL_COVER_SMALL=22;
	public static var FLICKR_GET_URL_COVER_HUGE=23;
	
	///<for srcType singlealbum of facebook
	public static var ALBUM_ID = 1;				///<The photo album ID
	public static var ALBUM_FROM_ID = 2;		///<The profile id that created this album
	public static var ALBUM_FROM_NAME = 3;		///<The profile name that created this album
	public static var ALBUM_NAME = 4;			///<The title of the album
	public static var ALBUM_DESCRIPTION = 5;	///<The description of the album
	public static var ALBUM_LINK = 6;			///<A link to this album on Facebook
	public static var ALBUM_COVER_PHOTO = 7;	///<The cover photo
	public static var ALBUM_PRIVACY = 8;		///<The privacy settings for the album
	public static var ALBUM_PHOTOCOUNT = 9;		///<The number of photos in this album
	public static var ALBUM_TYPE = 10;			///<The type of this album
	public static var ALBUM_CREATED_TIME = 11;	///<The time the photo album was initially created
	public static var ALBUM_UPDATED_TIME = 12;	///<The last time the photo album was updated
	public static var ALBUM_COMMENTS = 13;		///<The comments of this album
	public static var FLICKR_GET_PHOTO_TITLE=16;
	public static var FLICKR_GET_PHOTO_DESCRIPTION=17;
	public static var FLICKR_GET_PHOTO_ID=18;
	public static var FLICKR_GET_PHOTO_DATESPOSTED=19;
	public static var FLICKR_GET_PHOTO_DATESTAKEN=20;
	public static var FLICKR_GET_PHOTO_DATESLASTUPDATE=21;
	public static var FLICKR_GET_URL_PHOTO_SMALL=24;
	public static var FLICKR_GET_URL_PHOTO_HUGE=25;

	///<for srcType photoalbums/singlealbum of facebook
	public static var PHOTO_PATH = 30;			///<The photo's path under cache dir
	public static var PHOTO_THUMB_PATH = 31;	///<The thumbnail photo's path under cache dir
	
	///<for srcType singlephoto of facebook
	public static var PHOTO_ID = 14;			///<The photo ID	
	public static var PHOTO_FROM_ID = 15;		///<The profile id(user or page) that posted this photo
	public static var PHOTO_FROM_NAME = 16;		///<The profile name(user or page) that posted this photo
	public static var PHOTO_NAME = 17;			///<The caption given to this photo
	public static var PHOTO_PICTURE = 18;		///<the address of this photo in 130*97 size
	public static var PHOTO_SOURCE = 19;		///<The full-sized source of the photo
	public static var PHOTO_HEIGHT = 20;		///<The height of the photo in pixels
	public static var PHOTO_WIDTH = 21;			///<The width of the photo in pixels
	public static var PHOTO_LINK = 22;			///<A link to the photo on Facebook
	public static var PHOTO_ICON = 23;			///<The icon that Facebook displays when photos are published to the Feed
	public static var PHOTO_CREATED_TIME = 24;	///<The time the photo was initially published
	public static var PHOTO_UPDATED_TIME = 25;	///<The last time the photo or its caption was updated
	public static var PHOTO_COMMENTS = 26;		///<the comments of this photo

	///<for srcType contact of facebook andf flickr
	public static var FRIENDS_NUM = 27;			///<The user's friends num
	public static var FLICKR_GET_USER_REALNAME=5;
	public static var FLICKR_GET_USER_USERNAME=6;
	public static var FLICKR_GET_CONTACT_REALNAME=8;
	public static var FLICKR_GET_CONTACT_USERNAME=9;
	public static var FLICKR_GET_PHOTOSET_COUNT=14;
	public static var FLICKR_GET_PHOTOS_COUNT=15;

	///<for srcType single member of facebook and flickr
	public static var MEMBER_ID = 28;			///<The friend's id
	public static var MEMBER_NAME = 29;			///<The friend's name
	

	/*used send_msg*/
	public static var MSG_TYPE_DOWNLOADPHOTO=0;	///< download the photo in deamon
	public static var MSG_TYPE_STOPDOWNLOAD=1;	///< stop download
	public static var MSG_TYPE_UPLOADPHOTO=2;	///< upload the photo in deamon
	public static var MSG_TYPE_STOPUPLOAD=3;		///< stop upload
	public static var MSG_TYPE_AUTH=4;			///< auth in deamon
	public static var MSG_TYPE_STOPAUTH=5;		///< stop auth

	/*query cmd*/
	public static var QUERY_CMD_RESULT=0; 	///< get the result of uploading or downloading
	public static var QUERY_CMD_PROGRESS=1; ///< get the progress of  uploading or downloading


	/**
	@brief call this function to set the login type(picasa or facebook or flickr)
	@param[in] login_type : which type of the account, picasa or facebook or flickr
	@return
		- 0 	: success
		- -1	: failed
	**/
	public static function set_logintype(login_type:Number):Number
	{
		return ExternalInterface.call("web_set_logintype",login_type);
	}

	/**
	@brief call this function to set the email addr and password
	@param[in] username	: email addr
	@param[in] password	: password
	@return
		- 0 	: success
		- -1	: failed
	**/
	public static function set_login(username:String,password:String):Number
	{
		return ExternalInterface.call("web_set_login",username,password);
	}


	/**
	@brief initializing the gdata ,call the free_gdata() to free the space
	@return 
		- 0 		: failed
		- others 	: pointer to the gdata
	**/
	public static function init_gdata():Number
	{
		return ExternalInterface.call("web_init_gdata");
	}


	/**
	@brief free the space which had been malloc when the init_gdata() is called
	@param[in] gdata	: the pointer to the gdata which had been filled
	@see init_gdata()
	**/
	public static function free_gdata(gdata:Number):Number
	{
		return ExternalInterface.call("web_free_gdata",gdata);
	}


	/**
	@brief get the login user data
	@return 
		- 0 		: failed
		- others 	: pointer to the userinfo
	**/
	public static function get_userinfo(webalbumdata:Number):Number
	{
		return ExternalInterface.call("web_get_userinfo",webalbumdata);
	}


	/**
	@brief free the space which had been malloc when the get_userinfo() is called
	@param[in] gdata	: the pointer to the userinfo which had been filled
	@see get_userinfo()
	**/
	public static function free_userinfo(userinfo:Number):Number
	{
		return ExternalInterface.call("web_free_userinfo",userinfo);
	}

	/**
	@brief call this function after the authentication is ok for getting the contacts in the account
	@param[in] gdata	: initialize by calling the init_gdata()
	@return 
		- NULL		: get the infomation of the contacts failed
		- others	: the pointer to the feed which the information of the contacts are inclued 
	**/
	public static function get_contact(gdata:Number):Number
	{
		return ExternalInterface.call("web_get_contact",gdata);
	}


	/**
	@brief free the space which is occupied when the get_contact() is called 
	@param[in] feed_contact 	: the pointer to the feed which the information of the album is included when get_contact() is called
	@return always 0
	@see get_contact()
	**/
	public static function free_contact(feed_contact:Number):Number
	{
		return ExternalInterface.call("web_free_contact",feed_contact);
	}

	/**
	@brief call this function after the authentication is ok for getting the friendlists in the account
	@param[in] gdata	: initialize by calling the init_gdata()
	@return 
		- NULL		: get the infomation of the contacts failed
		- others	: the pointer to the friendlist which the information of the friendlists are inclued 
	**/
	public static function get_friendlists(gdata:Number):Number
	{
		return ExternalInterface.call("web_get_friendlists",gdata);
	}

	/**
	@brief free the space which is occupied when the get_friendlists() is called 
	@param[in] friendlist 	: the pointer to the friendlist which the information of the friendlists is included when get_friendlists() is called
	@return always 0
	@see get_member()
	**/
	public static function free_friendlists(friendlists:Number):Number
	{
		return ExternalInterface.call("web_free_friendlists",friendlists);
	}

	/**
	@brief call this function after getting the contacts in the account is OK
	@param[in] gdata	: initialize by calling the init_gdata()
	@param[in] friendlists	: initialize by calling the get_contact()
	@param[in] friendlist_index	: index of to get which friendlist's member
	@return 
		- NULL		: get the infomation of the contacts failed
		- others	: the pointer to the memberlist of this friendlist which the information of the members are inclued 
	**/
	public static function get_member(gdata:Number, friendlists:Number, friendlist_index:Number):Number
	{
		return ExternalInterface.call("web_get_member",gdata,friendlists,friendlist_index);
	}

	/**
	@brief free the space which is occupied when the get_member() is called 
	@param[in] friendlist 	: the pointer to the memberlist of this friendlist which the information of the members is included when get_member() is called
	@return always 0
	@see get_member()
	**/
	public static function free_member(friendlist:Number):Number
	{
		return ExternalInterface.call("web_free_member",friendlist);
	}



	/**
	@brief call this function after the authentication is ok for getting the albums in the account
	@param[in] gdata	: initialize by calling the init_gdata()
	@param[in] feed_contact 	: get from get_cotact(), if this value is null, get albums info of the login account
	@param[in] which_friend	:  if the feed_contact is NULL, this value is useless,if the feed_contact is not NULL, specify which friend will be get, the range of this value is 0~ (friend_num-1),
	see get_info() to get the friend_num
	@return 
		- NULL		: get the infomation of the albums failed
		- others	: the pointer to the picasaweb_feed_t which the information of the albums is inclued 
	**/
	public static function get_albums_info(gdata:Number,feed_contact:Number,which_friend:Number):Number
	{
		return ExternalInterface.call("web_get_albums_info",gdata,feed_contact,which_friend);
	}


	/**
	@brief free the space which is occupied when the get_albums_info() is called 
	@param[in] feed_albums 	: the pointer to the feed which the information of the album is included when get_albums_info() is called
	@return always 0
	@see get_albums_info()
	**/
	public static function free_albums_info(feed_albums:Number):Number
	{
		return ExternalInterface.call("web_free_albums_info",feed_albums);
	}


	/**
	@brief getting the information of an album which is specified by the para used
	@param[in] gdata	: initialize by calling the _init_gdata();
	@param[in] feed_albums	: the pointer to the feed which include all information of the albums in the account
	@param[in] which_album	: which album will specified, the range of this value is 0~(album_num-1),see get_info() to get the album_num
	@return
		NULL	: get the infomation of the album failed
		others	: the pointer to the feed which the information of the album 
	@see call free_album_info() to free the space which is malloced
	**/
	public static function get_album_info(gdata:Number,feed_albums:Number,which_album:Number):Number
	{
		return ExternalInterface.call("web_get_album_info",gdata,feed_albums,which_album);
	}


	/**
	@brief free the space which is occupied when the get_album_info() is called 
	@param[in] feed_albums	: the pointer to the feed which include all information of the albums in the account, if it is NULL
	@param[in] which_album	: which album will specified, the range of this value is 0~(album_num-1),see get_info() to get the album_num
	@return always 0
	@see get_album_info()
	**/
	public static function free_album_info(feed_albums,which_album):Number
	{
		return ExternalInterface.call("web_free_album_info",feed_albums,which_album);
	}


	/**
	@brief call this function for passing the authentication
	@param[in] gdata	: call init_gdata() to initializing the gdata
	@return
		the status of the processing, see macro get_status();
		- HttpStatus_OK : success
	**/
	public static function authentication(gdata:Number):Number
	{
		return ExternalInterface.call("web_authentication",gdata);
	}

	
	/**
	@brief query the auth staus, call this function after sending msg to the thread by calling send_msg()
	@param[in] gdata	: initialize by calling the init_gdata()
	@param[in] query_cmd: QUERY_CMD_RESULT
	@return 
		if query_cmd == QUERY_CMD_RESULT
		- -2		: the param is error
		- -1		: had not down yet
		- HttpStatus_FOUND	: had been done
		if query_cmd == others
		- -2		: the param is error
	**/
	public static function query_auth_status(gdata:Number,query_cmd:Number):Number
	{
		return ExternalInterface.call("web_query_auth_status",gdata,query_cmd);
	}


	/**
	@brief call this function for creating new albums
	@param[in] album_info	: the initial info of the album which will be created
	@param[in] gdata	: call init_gdata() to initializing the gdata
	@return
		the status of the processing, see macro get_status();
		- HttpStatus_CREATED: success
	**/
	public static function create_new_album(gdata:Number,title:String,summary:String,location:String,access:String,timestamp:String):Number
	{
		return ExternalInterface.call("web_create_new_album",gdata,title,summay,location,access,timestamp);
	}

	/**
	@brief call this function for deleting albums
	@param[in] gdata	: call init_gdata() to initializing the gdata
	@param[in] feed_albums	: the album information of the albums, see get_albums_info()
	@param[in] which_album	: which album to be choosen
	@return
		the status of the processing, see macro get_status();
		- HttpStatus_OK : success
	**/
	public static function delete_album(gdata:Number,feed_albums:Number,which_album:Number):Number
	{
		return ExternalInterface.call("web_delete_album",gdata,feed_ablums,which_album);
	}


	/**
	*@brief call this function to update some info of the album
	*@param[in] gdata	: initialize by calling the init_gdata()
	*@param[in] feed_albums	: the pointer to the feed where the info of the albums stored, see get_albums_info()
	*@param[in] which_album	: which album to be changed
	@return
		the status of the processing, see macro get_status();
		- HttpStatus_OK : success
	**/
	public static function update_album_info(gdata:Number,feed_albums:Number,which_album:Number,title:String,summary:String,location:String,access:String,timestamp:String):Number
	{
		return ExternalInterface.call("web_update_album_info",gdata,feed_ablums,which_album,title,summay,location,access,timestamp);
	}


	/**
	@brief call this function to initializing the upload_info which will be used in upload_photo(), call free_upload_info()
	to release the resource
	@param[in]	photo_fullpath	: the full path of the photo which will be uploaded
	@param[in] feed		: it is the picasaweb_feed_t which get from get_albums_info()
	@param[in] which_entry 	: which album does the photo to be added
	@return 
		- NULL	: failed
		- others	: the pointer to the upload_info
	@see free_upload_info()
	**/
	public static function init_upload_info(feed_albums:Number,which_album:Number,photofullpath:String):Number
	{
		return ExternalInterface.call("web_init_upload_info",feed_albums,which_album,photofullpath);
	}


	/**
	@brief call this function to to release the resource which had been occupied by calling init_upload_info()
	@param[in] upload_info	: the pointer to where the information for uploading stored, get it from calling init_upload_info()
	@return 
		always 0
	**/
	public static function free_upload_info(upload_info:Number):Number
	{
		return ExternalInterface.call("web_free_upload_info",upload_info);
	}



	/**
	@brief call this function to upload a photo, it is not run under background
	@param[in] gdata	: initialize by calling the init_gdata()
	@param[in] upload_info	: the pointer to where the information for uploading stored, get it from calling init_upload_info()
	@return 
		the status of the processing, see macro get_status();
		- HttpStatus_CREATED	: success
	**/
	public static function upload_photo(gdata:Number,upload_info:Number):Number
	{
		return ExternalInterface.call("web_upload_photo",gdata,upload_info);
	}


	/**
	@brief query the upload staus,  call this function after sending msg to the thread by calling send_msg()
	@param[in] gdata	: initialize by calling the init_gdata()
	@param[in] para	: it is the pointer to the photo_down_info_t, call the init_upload_info() to get it
	@param[in] query_cmd: see QUERY_CMD_RESULT etc
	@return 
		if query_cmd == QUERY_CMD_RESULT
		- -2		: the param is error
		- -1		: had not up yet
		- others	: had been done
		if query_cmd == QUREY_CMD_PROGRESS
		the progress of downloading 
	**/
	public static function query_upload_status(gdata:Number,upload_info:Number,query_cmd:Number):Number
	{
		return ExternalInterface.call("web_query_upload_status",gdata,upload_info,query_cmd);
	}


	/**
	@brief call this function to update the information of a photo
	@param[in] photo_info 	: the info of the photo which will be replaced
	@param[in] gdata	: initialize by calling the init_gdata()
	@param[in] feed_album	: the photo information in an album, see get_album_info()
	@param[in] which_photo	: which photo to be choosen
	@return 
		the status of the processing, see macro get_status();
		- HttpStatus_OK	: success
	**/
	public static function update_photo_info(gdata:Number,feed_album:Number,which_photo:Number,summary:String,description:String):Number
	{
		return ExternalInterface.call("web_update_photo_info",gdata,feed_album,which_photo,summary,description);
	}


	/**
	@brief call this function to delete a photo
	@param[in] gdata	: initialize by calling the init_gdata()
	@param[in] feed_album	: the photo information in an album, see get_album_info()
	@param[in] which_photo	: which photo to be choosen
	@return 
		the status of the processing, see macro get_status();
		- HttpStatus_OK	: success
	**/
	public static function delete_photo(gdata:Number,feed_album:Number,which_photo:Number):Number
	{
		return ExternalInterface.call("web_delete_photo",gdata,feed_album,which_photo);
	}


	/**
	@brief call this function to initialize the download info ,call free_download_info() to free the space 
	@param[in] iscache	: whether it is cached
	@param[in] cache_dir	: if iscache==1, it is the path of cache dir, such as /mnt/udisk/
	@param[in] feed		: it is the picasaweb_feed_t which get from get_albums_info() or get_album_info()
	@param[in] which_entry 	: which entry that the photo url in
	@param[in] isthumbnail	: whether it is a thumbnail, if it is it will be cached in the thumbnail folder
	@return 
		- NULL 	: failed
		- others : the pointer to the down_info
	@see wfree_download_info()
	**/
	public static function init_download_info(iscache:Number,cache_dir:String,feed:Number,which_entry:Number,isthumbnail:Number):Number
	{
		return ExternalInterface.call("web_init_download_info",iscache,cache_dir,feed,which_entry,isthumbnail);
	}


	/**
	@brief release the space which is malloc when the init_download_info() is called
	@param[in] down_info 	: get from init_download_info()
	@return
		always return 0
	**/
	public static function free_download_info(down_info:Number):Number
	{
		return ExternalInterface.call("web_free_download_info",down_info);
	}


	/**
	@brief download photo
	@param[in] gdata	: initialize by calling the init_gdata()
	@parma[in] down_info	: choose the method to be download, cache or uncache ,see init_download_info()
	@return 
		the status of the processing, see macro get_status();
		- HttpStatus_OK	: success
	**/
	public static function download_photo(gdata:Number,down_info:Number):Number
	{
		return ExternalInterface.call("web_download_photo",gdata,down_info);
	}


	/**
	@brief query the download staus, call this function after sending msg to the thread by calling send_msg()
	@param[in] gdata	: initialize by calling the init_gdata()
	@param[in] down_info	: it is the pointer to the down_info, call the download_info() to get it
	@param[in] query_cmd: see QUERY_CMD_RESULT etc
	@return 
		if query_cmd == QUERY_CMD_RESULT
		- -2		: the param is error
		- -1		: had not down yet
		- others	: had been done
		if query_cmd == QUREY_CMD_PROGRESS
		the progress of downloading 
	**/
	public static function query_download_status(gdata:Number,down_info:Number,query_cmd:Number):Number
	{
		return ExternalInterface.call("web_query_download_status",gdata,down_info,query_cmd);
	}


	/**
	@brief send the message to the thread
	@param[in] gdata	: initialize by calling the init_gdata()
	@param[in] cmd 	: see MSG_TYPE_DOWNLOADPHOTO etc
	@param[in] para	: 
				if cmd ==MSG_TYPE_DOWNLOADPHOTO or MSG_TYPE_STOPDOWNLOAD
					it is the pointer to the photo_down_info_t, call the init_download_info() to get it
				if cmd==MSG_TYPE_UPLOADPHOTO or MSG_TYPE_STOPUPLOAD
					it is the pointer to the photo_upload_info_t, call the init_upload_info() to get it
	@return 
		 - -1	: failed
		 - 0 		: success
	**/
	public static function send_msg(gdata:Number,msg_type:Number,msg_content:Number):Number
	{
		return ExternalInterface.call("web_send_msg",gdata,msg_type,msg_content);
	}


	/**
	@brief get the information of the albums or photos
	@param[in] feed	: the pointer where the information stored, get it from get_albums_info() or get_album_info() or get_contact();
	@param[in] which_entry : each feed have several entry, specified which entry to be got
	@return 
		NULL : get entry error
		others : the pointer to the entry, this value can be send to src parameter in get_info() when the src_type ==GetInfo_SrcType_ENTRY
	**/
	public static function get_entry(feed:Number,which_entry:Number):Number
	{
		return ExternalInterface.call("web_get_entry",feed,which_entry);
	}

	/**
	@brief get the information of the albums or photos
	@param[in] src	: the pointer where the information stored, get it from get_albums_info() or get_album_info();
	@param[in] src_type : which type of the pointer,see GetInfo_SrcType_FEED etc
	@param[in] info_type: which information will be gotten, see GetInfo_InfoType_FEED_ENTRYNUM etc
	@param[in] other_info: usually, it is unused, just when the info_type==GetInfo_InfoType_FEED_PHOTO_ADDR or
		GetInfo_InfoType_FEED_PHOTO_THUMB_ADDR. it is used as the which_entry.
	@return 
		the string of the content
	**/
	public static function get_info(src:Number,src_type:Number,info_type:Number,other_info:Number):String
	{
		return ExternalInterface.call("web_get_info",src,src_type,info_type,other_info);
	}

	/**
	@brief save the albums info into a file
	@param[in] gdata	: initialize by calling the init_gdata()
	@param[in] feed_albums	: get from get_albums_info()
	@param[in] cache_dir	: under which folder will the file be created
	@param[in] filename	: the name of the file to be created,if it is null, the default filename is the login name
	@return :
		-1 	: failed
		0	: succeed
	**/
	public static function save_albums_info(gdata:Number,feed_albums:Number,cache_dir:String,filename:String):Number
	{
		return ExternalInterface.call("web_save_albums_info",gdata,feed_albums,cache_dir,filename);
	}


	/**
	@brief load albums info from the file
	@param[in] gdata	: initialize by calling the init_gdata()
	@param[in] cache_dir	: under which folder will the file be stored
	@param[in] filename	: the file to be read which contains the info 
	@return :
		NULL	: load albums info error
		others	: the pointer to the picasaweb_feed_t which include all information of the albums in the account
	**/
	public static function load_albums_info(gdata:Number,cache_dir:String,filename:String):Number
	{
		return ExternalInterface.call("web_load_albums_info",gdata,cache_dir,filename);
	}

	/**
	@brief save the albums info into a file
	@param[in] gdata	: initialize by calling the init_gdata()
	@param[in] feed_albums	: the pointer to the picasaweb_feed_t which include all information of the albums in the account
	@param[in] which_album	: which album will specified, the range of this value is 0~(feed_albums->entry_num-1)
	@param[in] cache_dir	: under which folder will the file be created
	@return :
		-1 	: failed
		0	: succeed
	**/
	public static function save_album_info(gdata:Number,feed_albums:Number,which_album:Number,cache_dir:String):Number
	{
		return ExternalInterface.call("web_save_album_info",gdata,feed_albums,which_album,cache_dir);
	}


	/**
	@brief load the album info into a file
	@param[in] gdata	: initialize by calling the init_gdata()
	@param[in] feed_albums	: the pointer to the picasaweb_feed_t which include all information of the albums in the account
	@param[in] which_album	: which album will specified, the range of this value is 0~(feed_albums->entry_num-1)
	@param[in] cache_dir	: under which folder will the file be created
	@return :
		NULL	: load album info error
		others	: the pointer to the picasaweb_feed_t which include all information of the album
	**/
	public static function load_album_info(gdata:Number,feed_albums:Number,which_album:Number,cache_dir:String):Number
	{
		return ExternalInterface.call("web_load_album_info",gdata,feed_albums,which_album,cache_dir);
	}


	/**
	@brief save the contact info into a file
	@param[in] gdata	: initialize by calling the init_gdata()
	@param[in] feed_contact	: get from get_contact()
	@param[in] cache_dir	: under which folder will the file be created
	@return :
		-1 	: failed
		0	: succeed
	**/
	public static function save_contact_info(gdata:Number,feed_contact:Number,cache_dir:String):Number
	{
		return ExternalInterface.call("web_save_contact_info",gdata,feed_contact,cache_dir);
	}

	/**
	@brief load contact info from the file
	@param[in] gdata	: initialize by calling the init_gdata()
	@param[in] cache_dir	: under which folder will the file be stored
	@return :
		NULL	: load albums info error
		others	: the pointer to the picasaweb_feed_t which include all information of the contact in the account
	**/
	public static function load_contact_info(gdata:Number,cache_dir:String):Number
	{
		return ExternalInterface.call("web_load_contact_info",gdata,cache_dir);
	}

	/**
	@brief save user information
	@param[in] gdata	: initialize by calling the init_gdata()
	@return :
		-1 	: failed
		0	: succeed
	**/
	public static function save_usernfo(gdata:Number):Number
	{
		return ExternalInterface.call("web_save_usernfo",gdata);
	}

	/**
	@brief check how many user has login
	@param[in] NULL
	@return :
		the num of user who has login
	**/
	public static function check_user():Number
	{
		return ExternalInterface.call("web_check_user");
	}

	/**
	@brief load user account
	@param[in]  user_index	: the user index to get account
	@return :
		the user account
	**/
	public static function load_useraccount(user_index:Number):String
	{
		return ExternalInterface.call("web_load_useraccount",user_index);
	}

	/**
	@brief load user pwd
	@param[in]  user_index	: the user index to get pwd
	@return :
		the user pwd
	**/
	public static function load_userpwd(user_index:Number):String
	{
		return ExternalInterface.call("web_load_userpwd",user_index);
	}

	/**
	@brief create background update thread
	@param[in]  gdata	: initialize by calling the init_gdata()
	@param[in] iscache	: whether it is cached
	@param[in] cache_dir	: if iscache==1, it is the path of cache dir, such as /mnt/udisk/
	@param[in] isthumbnail	: whether it is a thumbnail, if it is it will be cached in the thumbnail folder
	@return :
		-1 	: failed
		0	: succeed
	**/
	public static function create_update(gdata:Number,iscache:Number,cache_dir:String,isthumbnail:Number):Number
	{
		return ExternalInterface.call("web_create_update",gdata,iscache,cache_dir,isthumbnail);
	}

	/**
	@brief select friend whose photo to download
	@param[in] friend_index	: friend index
	@param[in] friend_id	: friend id by calling get_info()
	@return :
		-1 	: failed
		0	: succeed
	**/
	public static function select_friend(friend_index:Number,friend_id:String):Number
	{
		return ExternalInterface.call("web_select_friend",friend_index,friend_id);
	}

/**
 *@}
 */	

}


