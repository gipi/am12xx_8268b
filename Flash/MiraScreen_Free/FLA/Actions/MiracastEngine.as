
dynamic class Actions.MiracastEngine {
	
	public static var ME_STAT_IDLE=0;
	public static var ME_STAT_CONNECTING=1;
	public static var ME_STAT_CONNECTED=2;
	
	public static var RTSP_STAT_IDLE=0;
	public static var RTSP_STAT_PLAYING=1;
	public static var RTSP_STAT_STOP=2;

	/**
	* @brief start the miracast engine.
	* @return 
    - 0 : failed
    - 1 : success
	*/
	public static function Start():Number
	{
		return ExternalInterface.call("miracastEnigneStart");
	}	
	
	/**
	* @brief stop the miracast engine
	*/
	public static function Stop():Void
	{
		ExternalInterface.call("miracastEnigneStop");
	}
	
	
	
	/**
	* @brief get miracast engine status.
	* @see ME_STAT_* definition
	*/
	public static function GetStatus():Number
	{
		return ExternalInterface.call("miracastEnigneGetStatus");
	}
	
	/**
	* @brief Init the RTSP function.
	* @return 
    - 0 : sucess
    - 1 : failed
	*/
	public static function RtspInit():Number
	{
		return ExternalInterface.call("miracast_engine_rtsp_init_as");
	}	
	
	/**
	* @brief Destroy the RTSP function.
	*/
	public static function RtspDestroy():Void
	{
		ExternalInterface.call("miracast_engine_rtsp_destroy_as");
	}	
	
	/**
	* @brief Start the RTSP service.
	* @return 
    - 0 : sucess
    - 1 : failed
	*/
	public static function RtspStart():Number
	{
		return ExternalInterface.call("miracast_engine_rtsp_start_as");
	}
	
	/**
	* @brief Stop the RTSP service.
	*/
	public static function RtspStop():Void
	{
		ExternalInterface.call("miracast_engine_rtsp_stop_as");
	}
	
	/**
	* @brief Get RTSP service status.
	*/
	public static function RtspStatus():Number
	{
		return ExternalInterface.call("miracast_engine_rtsp_get_stat_as");
	}
	
	/**
	* @brief For Miracast sigma test.
	*/
	public static function SigmaStart():Number
	{
		return ExternalInterface.call("miracast_engine_sigma_start_as");
	}
	
	public static function SigmaStop():Void
	{
		ExternalInterface.call("miracast_engine_sigma_destory_as");
	}
	
	/**
	* @brief drop the mira process mainly because Wifi's disconnection.
	*/
	public static function DropMira():Void
	{
		ExternalInterface.call("miracast_engine_rtsp_drop_mira_as");
	}

}
