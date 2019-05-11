/**
 *	@author   chenshouhui
 *	@version 1.0
 *  @date	: 2012/10/24
 *	<pre>  
 *  WidiEngine 
 		widi setting control
 *	</pre>
 *  <pre>
 	Example:
 *		WidiEngine.Open();	
 *		when exit:
 *		WidiEngine.Close();
 *	</pre>
 */


 dynamic class Actions.WidiEngine {

/**
*@addtogroup WidiEngine_as
*@{
*/
	/***sorry,it is unusful******/
	 public static var WIDI_DEVICE_MODE=1; //widi device mode
	 public static var WIDI_CLIENT_MODE=2; //widi client mode
	 public static var WIDI_SOFTAP_MODE=3; //widi softap mode
	 
	 /***pincode status******/

	public static var WIDI_PINCODE_PBC=0; //widi pincode default flag
	public static var WIDI_PINCODE_INPUT=1; //need input pincode
	public static var WIDI_PINCODE_DISPLAY=2; //need display pincode
	public static var WIDI_PINCODE_INVITED=3; //need display pincode
	public static var WIDI_PINCODE_PERSISTENT_GP_INVITED=4; //need display pincode
	public static var WIDI_PINCODE_NONE=1000; //need display pincode

	/* wifi direct connect flag */
	public static var WIDI_CONNECT_DEFAULT =0;  //widi defualt flag
	public static var WIDI_CONNECT_CLIENT_ING=1; //connecting as client 
	public static var WIDI_CONNECT_CLIENT_OK=2;  //connect ok as client
	public static var WIDI_CONNECT_SOFTAP_ING=3;  //been connected as softap
	public static var WIDI_CONNECT_SOFTAP_OK=4;   //been connected ok as softap
	public static var WIDI_STATUS_DISCONNECT=5;   //been disconnect after connect ok
	public static var WIDI_STATUS_REJECT =6;     //been reject in connecting
	public static var WIDI_STATUS_PROV_FAILURE=7;  //negotiation failed
	
	/**
	 *@brief	init WifiEngine
	 *@param[in] role:WIDI_DEVICE_MODE,WIDI_CLIENT_MODE,or WIDI_SOFTAP_MODE
	 */
	public static function Init(role:Number)
	{
		ExternalInterface.call("wifi_direct_func_init_as",role);
	}
	
	/**
	 *@brief	init WidiEngine at WLAN0 or WLAN1
	 *@param[in] role:WIDI_DEVICE_MODE,WIDI_CLIENT_MODE,or WIDI_SOFTAP_MODE
	 *@param[in] port: 0 -> WLAN0; 1 -> WLAN1
	 */
	public static function Init_select(role:Number, port:Number)
	{
		ExternalInterface.call("wifi_direct_func_init_select_as",role, port);
	}
	
	/**
	 *@brief	start WifiEngine
	 *@param[in] NULL
	 *@return true if success or false if fail
	 */
	
	 
	public static function Start()
	{
		ExternalInterface.call("wifi_dirct_func_start_as");
	}

	/**
	 *@brief	start scan AP
	 *@param[in] NULL
	 *@return true if success or false if fail
	 */
	public static function StartScan()
	{
		ExternalInterface.call("wifi_dirct_scan_ap_inform_as");
	}
	
	/**
	 *@brief	get scan status,must be called after call the StartScan().
	 *@param[in] NULL
	 *@return 1--> scan complete 0-->scan not complete
	 */
	public static function GetScanStatus():Number
	{
		return ExternalInterface.call("wifi_direct_get_scan_staus_as");
	}
	
	/**
	 *@brief	get scan results
	 *@param[in] NULL
	 *@return AP num
	 */
	public static function GetScanResults():Number
	{
		return ExternalInterface.call("wifi_dirct_getap_total_as");
	}

	/**
	 *@brief	stop Wifi
	 *@param[in] NULL
	 *@return NULL
	 */
	public static function Stop()
	{
		ExternalInterface.call("wifi_direct_stop_func_as");
	}
	
	/**
	 *@brief	get AP ssid
	 *@param[in] index : index of AP
	 *@return ap ssid
	 */
	public static function GetSsid(index:Number):String
	{
		return ExternalInterface.call("wifi_direct_getssid_as",index);
	}

	/**
	 *@brief do provision discovery and connect to a seleted device.
	 *       This API used for connecting peer device positively.
	 *@param[in] index : index of AP
	 */
	public static function DoProvisionDiscovery(index:Number)
	{
		ExternalInterface.call("wifi_direct_start_prov_discover_as",index);
	}

	/**
	 *@brief	get widi pincode_flag :display pincode or need input pincode
	 *@param[in] index : index of AP
	 *@return ap signal level
	 */
	public static function GetPinCodeFlag():Number
	{
		return ExternalInterface.call("wifi_direct_pincode_flag_as");
	}
	
	
	/**
	 *@brief Get local pincode for display.
	 */
	public static function GetPincode():String
	{
		
		return ExternalInterface.call("wifi_direct_display_pincode_as");
	}
	
	/**
	 *@brief Generate a random pincode.
	 */
	public static function GeneratePincode()
	{
		
		return ExternalInterface.call("wifi_direct_generate_pincode_app_as");
	}
	
	/**
	 *@brief Set pincode which is obtained from peer device.
	 *@param[in] pincode : pincode obtained from peer device.
	 */
	public static function SetPincode(pincode:String)
	{
		
		return ExternalInterface.call("wifi_direct_set_pincode_app_as",pincode);
	}
	
	/**
	*@brief	get the connecting widi ap index 
	*@return connect_index
	*/
	public static function GetWidiConnectIndex():Number
	{
		
		return ExternalInterface.call("widi_direct_connect_apidex_app_as");
	}
		
	/**
	*@brief	 run function when press cancel btn
	*@return 0:function run successfully
	*/
	public static function RejectConnectPBC():Number
	{
		return ExternalInterface.call("wifi_direct_pbc_cancel_app_as");
	}
		
	/**
	*@brief	  used to dectect if the peer device disconnected.
	*@return  0-->connected; 1-->disconnected; 2-->just ignore
	*/
	public static function GetConnectionStatus():Number
	{	
		 return ExternalInterface.call("widi_check_connect_status_as");
	}
	
	/**
	*@brief	  get the name of which want to connect into DPF 
	*@return     device name
	*/
	public static function GetPeerDeviceName():String
	{
		return ExternalInterface.call("widi_peer_device_name_as");
	}

	/**
	*@brief	  disconnect from wifi direct device
	*@return    NULL
	*/
	public static function Disconnect()
	{
		ExternalInterface.call("widi_disconnect_as");
	}
	
	/**
	*@brief	 get the device name of DPF
	*@return	the device name
	*/
	public static function GetWidiLocalDeviceName():String
	{
			
		return ExternalInterface.call("wifi_display_device_name_as");
	}

	/**
	*@brief	 get the sign length rank of wifi direct ap
	*@param[in] index : index of AP
	*@return	the sign length rank
	*/
	public static function GetWidiSignalLevel(index:Number):Number
	{
			
		return ExternalInterface.call("wifi_direct_get_signalLevel_as",index);
	}
	
	/**
	 *@brief Do the negotiation.
	 *@param[in] cm : config method
	 *
	 *@return 0 success,or failed
	 */
	public static function DoNegotiation(cm:Number):Number
	{
		return ExternalInterface.call("wifi_direct_do_nego_pthread",cm);
	}
	
	/**
	 *@brief For Miracast only. Check is do nego successfull.
	 *@return 0-->success 1-->failed  -1-->not done.
	 */
	public static function GetDoNegoResult():Number
	{
		return ExternalInterface.call("wifi_direct_do_nego_result");
	}
	
	/**
	 *@brief re-start the wifi direct if negotiation failed.
	 */
	public static function ReStart():Void
	{
		ExternalInterface.call("wifi_direct_restart_as");
	}
	
	/**
	 *@brief For Miracast only. Get the peer RTSP port.
	 */
	public static function GetPeerRTSPPort():Void
	{
		ExternalInterface.call("wifi_direct_get_peer_port_as");
	}	
	
	/**
	 *@brief For Miracast only. Get the peer ip address.
	 *@return 0-->success 1-->failed.
	 */
	public static function GetPeerIpAddr():Number
	{
		return ExternalInterface.call("wifi_direct_get_peer_ip_pthread");
	}
	
	/**
	 *@brief For Miracast only. Check is the peer ip address get successfull.
	 *@return 0-->success 1-->failed  -1-->not done.
	 */
	public static function GetPeerIpAddrResult():Number
	{
		return ExternalInterface.call("wifi_direct_get_peer_ip_result");
	}
	
	/**
	 *@brief Set P2P local device name.
	 */
	public static function SetLocalDevName(devname:String)
	{
		ExternalInterface.call("wifi_display_set_local_device_name_as",devname);
	}
	
	/**
	 *@brief Do join a group and role need to negotiate.
	 *
	 *@return 0 success,or failed
	 */
	public static function DoJoinGroup():Number
	{
		return ExternalInterface.call("wifi_direct_do_join_group_as");
	}
	
	/**
	 *@brief Do join a persistent group and role is client.
	 */
	public static function DoJoinPersistentGroup():Void
	{
		ExternalInterface.call("wifi_direct_do_join_persistent_group_as");
	}
	
	/**
	 *@brief mixed mode change.
	 */
	public static var MIXED_MODE_CLIENT_SOFTAP = 0;
	public static var MIXED_MODE_CLIENT_P2P = 1;
	public static var MIXED_MODE_P2P_SOFTAP = 2;
	public static var MIXED_MODE_P2P = 3;
	public static var MIXED_MODE_P2P_CLOSE = 4;
	public static var MIXED_MODE_P2P_OPEN = 5;
	public static function ModeChange(mode:Number):Void
	{
		trace("ModeChange: change mode to: "+mode);
		ExternalInterface.call("wifi_direct_mixed_mode_change", mode);
	}
	
	/**
	 *@brief get mixed mode.
	 */
	public static var MIXED_STATUS_CONNECTING = -2;
	public static var MIXED_STATUS_CLIENT_SOFTAP = 0;
	public static var MIXED_STATUS_CLIENT_P2P = 1;
	public static var MIXED_STATUS_P2P_SOFTAP = 2;
	public static var MIXED_STATUS_P2P_WLAN0 = 3;
	public static var MIXED_STATUS_P2P_WLAN1 = 4;
	public static function GetMode():Number
	{
		return ExternalInterface.call("wifi_direct_get_mixed_mode");
	}
}
