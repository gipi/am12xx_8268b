/**
*		@author   sllin
*		@version 2.0
* 		@date: 2010/07/26  
*	<pre> 
*  medialib Api Class : 
*	Access to Actions Micro's medialib
*  	
*/
//////code examples
/**


**/
dynamic class Actions.MediaLib {

/**
*@addtogroup MediaLib_as
*@{
*/

/**Attr Type:**/
	public static var ML_ST_FILE_NAME 						= 0x01;  ///<file name, it must be included
	public static var ML_ST_FILE_SIZE 						= 0x02;  ///<file size
	public static var ML_ST_FILE_TIME						= 0x04;  ///<file time when created
	public static var ML_ST_FILE_PATH						= 0x08; ///<file path, it must be included
	
	///photo
	public static var ML_ST_EXIF_EXPTIME 					= 0x10; 	///<photo capture time
	public static var ML_ST_EXIF_HEIGHT					= 0x20;	///<the height of the photo
	public static var ML_ST_EXIF_WIDTH					= 0x40;	///<the width of the photo

	//music
	public static var ML_ST_ID3_ARTISTS					= 0x100;		///<artists in an audio file
	public static var ML_ST_ID3_ALBUM						= 0x200;		///<album in an audio file
	public static var ML_ST_ID3_GENRES					= 0x400;		///<geners in an audio file
	public static var ML_ST_ID3_COMPOSER					= 0x800;		///<composer in an audio file
	public static var ML_ST_ID3_TIME						= 0x1000;	///<time in an audio file

	//user tag
	public static var ML_ST_USER_TAG1						= 0x2000;		///<user tag to be stored

	//the system is using TAG1 to store the user tag,and others is not used recently
	public static var ML_ST_USER_TAG2						= 0x4000;
	public static var ML_ST_USER_TAG3						= 0x8000;
	public static var ML_ST_USER_TAG4						= 0x10000;
	public static var ML_ST_USER_TAG5						= 0x20000;
/**Attr Type  End**/

//minfo_type
	public static var ML_MTYPE_PHOTO						=0;		///<photo information will be stored
	public static var ML_MTYPE_MUSIC						=1;		///<music information will be stored	
	public static var ML_MTYPE_VIDEO						=2;		///<video information will be stored

//view mode
	public static var ML_VM_REMOVE_DUPLCATE				=2;		///<list the view which remove the duplicate items
	public static var ML_VM_FITLER_BYVALUE_EQUAL			=4;		///<list the view which value is equal to the value be given
	public static var ML_VM_FITLER_BYVALUE_BITEQUAL		=8;		///<list the view which value has the bit equal to the value be given
	public static var ML_VM_PURE_SORT						=16;	///<list the view which is sort

//search  mode
	public static var ML_SEARCH_MODE_SEQUENCE			=1;		///<search in sequence
	public static var ML_SEARCH_MODE_REVERSE				=2;		///<search by reverse

//modify mode 
	public static var ML_MODIFY_MODE_CHANGETO			=1;		///<change the context to the value which is specified
	public static var ML_MODIFY_MODE_SETBITS				=2;		///<set the bit  according to the value be given
	public static var ML_MODIFY_MODE_CLRBITS				=3;		///<clear the bit according to the value be given

/**Tag value*/
 	public static var ML_TAG_TAG1							= 0x01;		///<tag type 1
	public static var ML_TAG_TAG2							= 0x02;		///<tag type 2
	public static var ML_TAG_TAG3							= 0x04;		///<tag type 3
	public static var ML_TAG_TAG4							= 0x08;		///<tag type 4
	public static var ML_TAG_TAG5							= 0x10;		///<tag type 5
	public static var ML_TAG_TAG6							= 0x20;		///<tag type 6
	public static var ML_TAG_TAG7							= 0x40;		///<tag type 7
	public static var ML_TAG_TAG8							= 0x80;		///<tag type 8
	public static var ML_TAG_TAG9							= 0x100;		///<tag type 9
	public static var ML_TAG_TAG10							= 0x200;		///<tag type 10
	public static var ML_TAG_TAG11							= 0x400;		///<tag type 11
	public static var ML_TAG_TAG12							= 0x800;		///<tag type 12
	public static var ML_TAG_TAG13							= 0x1000;	///<tag type 13
	public static var ML_TAG_TAG14							= 0x2000;	///<tag type 14
	public static var ML_TAG_TAG15							= 0x4000;	///<tag type 15
	public static var ML_TAG_TAG16							= 0x8000;	///<tag type 16
/**Tag Vaue End**/


/**command get info**/
	public static var CMD_GET_FILE_PATH						=0x0;	///<get file path
	public static var CMD_GET_FILE_NAME						=0x1;	///<get file name
	public static var CMD_GET_FILE_TIME						=0x2;	///<get file created time
	public static var CMD_GET_FILE_SIZE						=0x3;	///<get file size, in bytes

	public static var CMD_GET_EXIF_WIDTH						=0x4;	///<get photo width
	public static var CMD_GET_EXIF_HEIGHT						=0x5;	///<get photo height
	public static var CMD_GET_EXIF_EXPTIME					=0x6;	///<get photo capture time	

	public static var  CMD_GET_ID3_ARTISTS					=0x7;	///<get audio artist
	public static var  CMD_GET_ID3_ALBUM						=0x8;	///<get audio album
	public static var  CMD_GET_ID3_GENRES						=0x9;	///<get audio genes
	public static var  CMD_GET_ID3_COMPOSER					=0xb;	///<get audio composer
	public static var  CMD_GET_ID3_TIME						=0xc;	///<get audio time

	public static var CMD_GET_USER_TAG1						=0x0a;	///<get user tag

/**get db create status**/
	public static var DB_CREATING							=0x0;  	///<db is creating
	public static var DB_CREATE_OK							=0x1;	///<creating db succeed
	public static var DB_CREATE_FAIL						=0x2;	///<creating db failed


	
	/**
	@brief this fuction is call to open the database of specified mediatype
	@param[in] m_type	: ML_MTYPE_PHOTO etc,which is the same as createDB
	@param[in] db_name	: the name of db which will be opened
	@return
		- 0		: failed
		- others	: the handle of db which is opened
	**/
      	public static function openDB(m_type:Number,dbname:String):Number
	{
		return ExternalInterface.call("album_openDB",m_type,dbname);
	}

	
	/**
	@brief this fuction is call to close the database of specified mediatype
	@param[in] db_handle	: which is returned from openDB()
	@return
		- -1	: failed
		- 0	: succeed
	@see openDB
	**/
	public static function closeDB(db_handle:Number):Number
	{
		return ExternalInterface.call("album_closeDB",db_handle);
	}	


	/**\
	@brief this fuction is call to create the database of specified mediatype
	@param[in] rootpath	: the db is created based on the files which were searched under rootpath
	@param[in] attrtype	: ML_ST_FILE_NAME,etc. this value specified which information should be stored in the database,
	@param[in] fileext 		: the extention of files such as "JPG BMP JPEG TIFF"
	@param[in] dbname	: name the db, it will be used when openDB is called 
	@param[in] m_type	: ML_MTYPE_PHOTO etc;
	@return
		the total file num under the media 
		- if value = -1 it means it is failed to get the number or creat the sacntask failed;
		- if value = -2 it means create the task succeed
	**/
	public static function createDB(rootpath:String,attrtype:Number,fileext:String,dbname:String,m_type:Number):Number
	{
		return ExternalInterface.call("album_createDB",rootpath,attrtype,fileext,dbname,m_type);
	}		


	/**
	@brief this function is call to destory the specified db
	@param[in] m_type	: ML_MTYPE_PHOTO etc,which is the same as createDB()
	@param[in] db_name	: the name of db which will be destroyed
	@return
		- -1	: failed
		- 0	: succeeded
	@see createDB()
	**/
	public static function destroyDB(m_type:Number,dbname:String):Number
	{
		return ExternalInterface.call("album_destoryDB",m_type,dbname);
	}


	/**
	@brief this function is call for updating DB
	@param[in] dbhandle	:the handle of the db which will be updated
	@return
		- -1	: failed
		- 0	: succeed
	*/
	public static function updateDB(dbhandle:Number):Number
	{
		return ExternalInterface.call("album_updateDB",dbhandle);
	}

	/**
	@brief this function is used for getting the task status 
	@param[in] none
	@return DB_CREATING etc
	*/
	public static function getDBCreateStatus():Number
	{
		return ExternalInterface.call("album_getDBCreateStatus");
	}


	/**
	@brief this function is called for creating a view which match specified condition
	@param[in] dbhandle		: get from opendb etc
	@param[in] metadata_type	: the same as Attr Type, ML_ST_FILE_NAME etc
	@param[in] value			: the value is used to filteing the useless data
	@param[in] viewmode		: ML_VM_REMOVE_DUPLCATE etc
		- the viewmode will determine whether the value be used, if viewmode==ML_VM_PURE_SORT or ==ML_VM_REMOVE_DUPLCATE,the value will not be used
	@param[in] viewhandle:create the new view based on the view which is specified by viewhandle. if based on default view, the viewhandl==0
	@return
		- 0	: failed
		- others	: the handle to the view which is opened
	@warning don't forget to call closeView() when the view is unused
	@see ML_ST_FILE_NAME etc, closeView()
	*/
	public static function openView(dbhandle:Number,metadata_type:Number,value:String,viewmode:Number,viewhandle:Number):Number
	{
		return ExternalInterface.call("album_openView",dbhandle,metadata_type,value,viewmode,viewhandle);
	}

	/**
	@brief close a view which is opened
	@param[in] viewhandle	: the handle of a view which is opened, see openView()
	@return
		- 0	: succeed
		- -1	: failed
	@see openView()
	*/
	public static function closeView(viewhandle:Number):Number
	{
		return ExternalInterface.call("album_closeView",viewhandle);
	}


	/**
	@brief get the number of files in the specified view
	@param[in] viewhandle	: the handle of a view which is opened, see openView()
		- if the viewhandle==0, it will get the num of default view
	@return
		- 0	: failed
		- -1	: failed
	@see openView()
	*/
	 public static function getNum(viewhandle:Number):Number
	{
		return ExternalInterface.call("album_getNum",viewhandle);
	}




	/**
	@brief this function is called for deleting an item in database
	@param[in] viewhandle	: the handle of view which got from openview()
	@param[in] fileidx		: the logical index in the view, the range of this value is from 0 to the returned value of getNum()
	@return
		- 0	: succeed
		- -1	: failed
	@see getNum()
	*/
	public static function delItem(viewhandle:Number,fileidx:Number):Number
	{
		return ExternalInterface.call("album_delItem",viewhandle,fileidx);
	}


	/**
	@brief this function is called for adding the specified to the dabase which is opened
	@param[in] dbhandle	: the handle of db which is opened, see openDB()
	@param[in] filefullpath	: the full path of the file
	@param[in] tagvalue	: consider as a attribute of the file
	@return
		- -1	: failed
		- 0	: succeed
	@see openDB()
	*/
	public static function addItem(dbhandle:Number,filefullpath:String,tagvalue:Number):Number
	{
		return ExternalInterface.call("album_addItem",dbhandle,filefullpath,tagvalue);
	}

	/**
	@brief this function is called for finding an item in database
	@param[in] viewhandle		: the handle of a view that is opened, see openView()
	@param[in] metadata_type	: the same as Attr Type,ML_ST_FILE_NAME etc
	@param[in] value			: the value to be search,it is a String type,you should change the Number to String
	@param[in] search_mode	: ML_SEARCH_MODE_SEQUENCE etc
	@return
		- -1		: failed, can't find the index of a file which is suited for the condition
		- others	: the index of the file which is suited for the condition
	@see ML_ST_FILE_NAME etc, ML_SEARCH_MODE_SEQUENCE etc 
	*/
	public static function findItem(viewhandle:Number,metadata_type:Number,value:String,search_mode:Number):Number
	{
		return ExternalInterface.call("album_findItem",viewhandle,metadata_type,value,search_mode);
	}


	/**
	@brief this function is called for modifying an item in database
	@param[in] viewhandle		: the handle of a view which is opened, see openView()
	@param[in] fileidx			: the logical index in the view, the range of this value is from 0 to the returned value of getNum()
	@param[in] metadata_type	: the same as Attr Type, ML_ST_FILE_NAME etc
	@param[in] value			: the new value to be set
	@param[in] md_mode		: ML_MODIFY_MODE_CHANGETO etc
	@return
		- 0	: succeed
		- -1	: failed
	@see openView(), getNum(), ML_ST_FILE_NAME etc, ML_MODIFY_MODE_CHANGETO etc
	*/
	public static function modifyItem(viewhandle:Number,fileidx:Number,metadata_type:Number,value:String,md_mode:Number):Number
	{
		return ExternalInterface.call("album_modifyItem",viewhandle,fileidx,metadata_type,value,md_mode);
	}


	/**
	@brief this function is called for getting information
	@param[in] viewhandle	: the value returned from openView, see openView()
	@param[in] fileidx		: the logical index in the view, the range of this value is from 0 to the returned value of getNum()
	@param[in] info_mode	: CMD_GET_FILE_PATH etc
	@return
		- if info_mode == CMD_GET_FILE_PATH \n 
			return the filepath of the file
		- if info_mode == CMD_GET_FILE_NAME \n 
			return the file name
		- if info_mode == CMD_GET_FILE_TIME \n 
			return time of last modification
		- if info_mode == CMD_GET_USER_TAG \n
			get the user tag which is stored in USER_TAG1
	@see openView(), getNum(), CMD_GET_FILE_PATH etc, 
	*/
	public static function getInfo(viewhandle:Number,fileidx:Number,info_mode:Number):String
	{
		return ExternalInterface.call("album_getInfo",viewhandle,fileidx,info_mode);
	}

	/**
	@brief check whether the udisk is dirty,if it is , you must create the album again
	@return 
		1 	: the udisk is dirty
		0	: the udisk is not changed	
	**/
	public static function check_udisk_dirty():Number
	{
		return ExternalInterface.call("album_checkudiskdirty");
	}

	/**
	@brief get the tag value which is adhered to the file 
	@param[in] filepath : the fullpath of the file 
	@return 
		the tag value
	**/
	public static function getTagPhotoAttached(filepath:String):Number
	{
		return ExternalInterface.call("album_getTagPhotoAttached",filepath);
	}

	/**
	@brief store the tag value in the file
	@param[in] filepath : the fullpath of the file 
	@param[in] tagvalue : the tag value to be stored
	@return 
		0 : succeed
		-1 : failed
	**/
	public static function storeTagPhotoAttached(filepath:String,tagvalue:Number):Number
	{
		return ExternalInterface.call("album_storeTagPhotoAttached",filepath,tagvalue);
	}
/**
 *@}
 */	

}

