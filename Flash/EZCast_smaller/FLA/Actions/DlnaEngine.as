
dynamic class Actions.DlnaEngine {

 /**
 *@addtogroup DlnaEngine_as
 *@{
 */

/***********************related to DLNA DMP DMS**********************/


 
  	public static var DLNA_STAT_ERR=0;
 	public static var DLNA_STAT_UNKNOWN=1;
 	public static var DLNA_STAT_OK=2;
	public static var DLNA_STAT_PROCESSING=3;
	public static var DLNA_STAT_NOFILE=4;
	
  	public static var DLNA_DMR_DEFCMD=0;
  	public static var DLNA_DMR_PLAY=1;
 	public static var DLNA_DMR_STOP=2;
 	public static var DLNA_DMR_pause=3;
 	public static var DLNA_DMR_REPEAT_ONE=4;
 	public static var DLNA_DMR_REPEAT_ALL=5;

	
 	public static var DLNA_DMR_SET_SEEK_TIME=4;
 	public static var DLNA_DMR_FAST_FORWARD=6;
	public static var DLNA_DMR_FAST_BACKWARD=7;
	public static var DLNA_DMR_CANCEL_FF=8;
	public static var DLNA_DMR_CANCEL_FB=9;
	
	
	/**
	* @brief start the dlna module.(DMP DMS MDMD included)
	*/
	public static function DLNAStart():Number
	{
		return ExternalInterface.call("dlna_StartWork");
	}	
	
	/**
	* @brief stop the dlna module.((DMP DMS MDMD included))
	*/
	public static function DLNAStop():Void
	{
		ExternalInterface.call("dlna_StopWork");
	}
	/**
	* @brief get obj(cds_Obj) name.
	*/

	public static function DLNADMPGetName(i:Number):String
	{
		return ExternalInterface.call("dlna_DMPGetName", i);
	}


	/**
	* @brief get obj(cds_Obj) name.
	*/

	public static function DLNADMPGetUri(i:Number):String
	{
		return ExternalInterface.call("dlna_DMPGetUri", i);
	}

	/**
	* @brief get Status.
	*/

	public static function DLNADMPGetStatus():Number
	{
		return ExternalInterface.call("dlna_DMPGetStatus");
	}

	

	/**
	* @brief get total numbers.
	*/

	public static function DLNADMPGetTotal():Number
	{
		return ExternalInterface.call("dlna_DMPGetTotal");
	}

	/**
	* @brief enter.
	*/

	public static function DLNADMPEnter(i:Number):Number
	{
		return ExternalInterface.call("dlna_DMPEnter",i);
	}
	
	/**
	* @brief get type of obj(cdsObj).
	*/

	public static function DLNADMPGetMediaType(i:Number):Number
	{
		return ExternalInterface.call("dlna_DMPGetMediaType",i);
	}

	/**
	* @brief escape.
	*/

	public static function DLNADMPEsc(i:Number):Number
	{
		return ExternalInterface.call("dlna_DMPEsc",i);
	}	

	/**
	* @brief download single file.
	*/

	public static function DLNAMDMDDownloadFile(FileIndex:Number,SavePath:String):Number
	{
		return ExternalInterface.call("dlna_MDMDDownloadFile",FileIndex,SavePath);
	}

	/**
	* @brief get Download Status.
	*/

	public static function DLNAMDMDGetDownloadStatus():Number
	{
		return ExternalInterface.call("dlna_MDMDGetDownloadStatus");
	}

	/**
	* @brief Get Total Bytes Expected.
	*/

	public static function DLNAMDMDGetTotalBytesExpected():Number
	{
		return ExternalInterface.call("dlna_MDMDGetTotalBytesExpected");
	}

	/**
	* @brief Get Bytes Received.
	*/

	public static function DLNAMDMDGetBytesReceived():Number
	{
		return ExternalInterface.call("dlna_MDMDGetBytesReceived");
	}


	/**
	* @brief remove devices.
	*/
	public static function DLNARemoveDevice():Void
	{
		ExternalInterface.call("dlna_RemoveDevice");
	}


	/**
	* @brief start DMS.
	*/
	public static function DLNAStartDMS():Number
	{
		return ExternalInterface.call("dlna_StartDMS");
	}	

	/**
	* @brief stop DMS.
	*/
	public static function DLNAStopDMS():Number
	{
		return ExternalInterface.call("dlna_StopDMS");
	}	

	/**
	* @brief start DMP.
	*/
	public static function DLNAStartDMP():Number
	{
		return ExternalInterface.call("dlna_StartDMP");
	}	


/***********************related to DLNA DMR**********************/
	/**
	* @brief START the DLNA DMR module.
	*/
	public static function DLNADMRStart():Number
	{
		return ExternalInterface.call("dlna_DMRStartWork");
	}	


	/**
	* @brief STOP the DLNA DMR module.
	*/
	public static function DLNADMRStop():Number
	{
		return ExternalInterface.call("dlna_DMRStop");
	}	

	/**
	* @brief get DMR uri  
	*/

	public static function DLNADMRGetUri():String
	{
		return ExternalInterface.call("dlna_DMRGetUri");
	}


	/**
	* @brief get Download Status.
	*/

	public static function DLNADMRGetCmd():Number
	{
		return ExternalInterface.call("dlna_DMRGetCmd");
	}

	/**
	* @brief DMR post sem.
	*/

	public static function DLNADMRPostSem():Number
	{
		return ExternalInterface.call("dlna_DMRSemPost");
	}

	/**
	* @brief DMR Get file type.
	*/

	public static function DLNADMRGetMediaType():Number
	{
		return ExternalInterface.call("dlna_DMRGetMediaType");
	}

	/**
	* @brief setting dmr title.
	*/
	public static function DLNADMRSettingTitle(dmr_title:String)
	{
		 ExternalInterface.call("dlna_DMRSettingTitle",dmr_title);
	}
	/**
	* @brief get dmr media title.
	*/

	public static function DLNADMRGetMediaTitle():String
	{
		 return ExternalInterface.call("dlna_DMRgetmediatitle");
	}




/**
 *@}
 */	

}
