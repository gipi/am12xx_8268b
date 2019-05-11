/**
 *	@author   chavi chen
 *	@version 1.0
 *  @date: 2014/10/28
 *	<pre>  
 *  Customized Class 
 		customer related control
 *  </pre>
 *	<pre>
 	Example:
 *  	
 *		
 *			
 *	</pre>
 */


dynamic class Actions.Customization {

	public static var WIFI_DONGLE_OFF = 0;   // wifi dongle out
	public static var WIFI_DONGLE_IN = 3;    // wifi dongle in
	
	/**
	 * @brief	customized
	 * @param[in] NULL:
 	 * @return NULL		
	 *		
	 */
	public static function send_wifi_status(wifi_status:Number)
	{
		ExternalInterface.call("custom_send_wifistatus", wifi_status);	 
	}
	/*
	public static function send_ap_ip()
	{
		ExternalInterface.call("custom_send_apip");
	}
	*/
	public static function set_volume(vol_diff:Number)
	{
		ExternalInterface.call("custom_set_volume", vol_diff);
	}
	public static function get_poweron_info():String
	{
		return ExternalInterface.call("custom_get_poweron_message");
	}
	
	public static function get_start_source():Number
	{
		return ExternalInterface.call("custom_get_startsource");
	}
	
	public static function get_scaler_language():Number
	{
		return ExternalInterface.call("custom_get_scaler_language");
	}
	
	public static function get_lan_in_flag():Number
	{
		return ExternalInterface.call("custom_get_lan_in_flag");
	}
}