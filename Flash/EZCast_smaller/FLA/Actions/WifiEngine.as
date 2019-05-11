
/**
 *	@author   zhuzhenfang
 *	@version 1.0
 *  @date	: 2011/02/25
 *	<pre>  
 *  WifiEngine 
 		wifi setting control
 *	</pre>
 *  <pre>
 	Example:
 *		WifiEngine.Open();	
 *		when exit:
 *		WifiEngine.Close();
 *	</pre>
 */

 dynamic class Actions.WifiEngine {

/**
*@addtogroup WifiEngine_as
*@{
*/
	public static var AUTHEN_WPA=0;		///< authen type:wpa
	public static var AUTHEN_WEP=1;		///< authen type:wep
	public static var AUTHEN_OPEN=2;	///< authen type:open

	public static var WIFI_DISCONNECTED=-1;	///< WPA_DISCONNECTED - Disconnected state
	public static var WIFI_COMPLETED=0;		///< WPA_COMPLETED - All authentication completed
	public static var WIFI_INACTIVE=1;		///< WPA_INACTIVE - Inactive state (wpa_supplicant disabled)
	public static var WIFI_SCANNING=2;		///< WPA_SCANNING - Scanning for a network
	public static var WIFI_ASSOCIATING=3;	///< WPA_ASSOCIATING - Trying to associate with a BSS/SSID
	public static var WIFI_ASSOCIATED=4;	///< WPA_ASSOCIATED - Association completed
	public static var WIFI_4WAY_HANDSHAKE=5;///< WPA_4WAY_HANDSHAKE - WPA 4-Way Key Handshake in progress
	public static var WIFI_GROUP_HANDSHAKE=6;///<WPA_GROUP_HANDSHAKE - WPA Group Key Handshake in progress
	public static var WIFI_AUTO_IP_ERROR=7;	///< wlan status:associated
	public static var WIFI_AUTO_IP_SUCCESS=8;	///< wlan status:associated
	public static var WIFI_CONNECT_FAILED=9;	///connect failed
	public static var WIFI_DISCONNECTED_MANUAL = 10;	///connect failed

	public static var INDEX_OUTRANGE=1;		///< set pwd:AP index out of range
	public static var WPA_PWD_ERROR=2;		///< set pwd:WPA mode, password length must be in [8,63]
	public static var WEP_PWD_ERROR=3;		///< set pwd:WEP mode, if Hex, password length must 10 or 26 or 32, if ASCII, password length must 5 or 13 or 16
	public static var PORT_ERROR=4;			///< ifconfig [port]:port is not exist

/*
	
	public static var WIFI_CONCURRENT_CLIENT_CLOSE=9; //wifi concurrent mode client disable
*/

	public static var WIFI_INIT_STATUS=-1;  // init status for wifi dongle   
	public static var WIFI_DONGLE_PLUG_IN=0;  //wifi dongle plug in
	public static var WIFI_DONGLE_PLUG_OUT=1; //wifi dongle plug out
	
	public static var WIFI_CLIENT_MODE_ENABLE=2;      //enable wifi client    
	public static var WIFI_CLIENT_GETAUTOIP_OK=3;   //get auto ip 
	public static var WIFI_CLIENT_GETAUTOIP_ERR=13;   //get auto ip fail ,may be occur on some ap disabe DHCP
	public static var WIFI_SOFTAP_ENABLE=4;        //enable soft ap mode

	public static var WIFI_DIRECT_ANABLE=5;     //wifi direct able
	public static var WIFI_DIRECT_CLIENT_OK=6;   //DPF connect another peer device as client
	public static var WIFI_DIRECT_SOFTAP_OK=7;   //DPF connected by another peer device as softap

	public static var WIFI_CONCURRENT_CLIENT_ENABLE=8;    //anable client while concurrent mode
	public static var WIFI_CONCURRENT_CLIENT_GETAUTOIP_OK=9;	  //connect successfully as client while concurrent mode
	public static var WIFI_CONCURRENT_CLIENT_GETAUTOIP_ERR=14;   //get auto ip fail ,may be occur on some ap disabe DHCP
	public static var WIFI_CONCURRENT_SOFTAP_ENABLE=10;	  //connect successfully as softap  concurrent mode

	public static var WIFI_DISABLE_MANUAL=11;         //user disable manually
	public static var WIFI_MODE_CHANGEING_FOR_RALINK=12;  //special status for ralink wifi dongle when it's swtitch mode 


	public static var ADD_NET_OPEN=0;		///< add network authen type:open
	public static var ADD_NET_WEP=1;		///< aadd network authen type:wep
	public static var ADD_NET_WPA=2;	///< add network authen type:wpa
	public static var ADD_NET_WPA2=3;	///< add network authen type:wpa2


	public static var SWITCH_TO_CLIENT=1;     //swtich to client wifi
	public static var SWITCH_TO_SOFTAP=2;    //switch to softap
	public static var CLOSE_WIFI_PROCESS=3;  //close wifi function
	public static var RESTART_WIFI_PROCESS=4; //restart wifi function

	/**
	*add the back_from state for set_wifi deal with.
	*/
	public static var BACKFROM_WIFISOFTAP=0; 	//back from set_wifi_softap.swf
	public static var BACKFROM_WIFICLIENT=1;	//back from set_wifi_client.swf
	public static var BACKFROM_WIFIEZREMOTE=2;	//back from set_wifi_ezremote.swf
	public static var BACKFROM_WIFIDIRECT=3;	//back from set_wifi_direct.swf
	public static var BACKFROM_WIFICONCURRENT=4;//back from set_wifi_concurrent.swf
	public static var BACKFROM_NONE=-1;			//back from none

	public static var AS_HOTSPOT_UNCONNECT = -1;
	public static var AS_HOTSPOT_READY = 0;
	public static var AS_HOTSPOT_SCAN = 1;
	public static var AS_HOTSPOT_CONNECTING = 2;
	public static var AS_HOTSPOT_FAIL = 3;
	public static var AS_HOTSPOT_SUCCESS =4;
	public static var AS_HOTSPOT_TIMEOUT =5;

	/**
	 *@brief	open WifiEngine
	 *@param[in] NULL
	 *@return 
	 *		true if success or false if fail
	 */
	public static function Open():Boolean
	{
		return ExternalInterface.call("wifi_Open");
	}
	
	/**
	 *@brief	close WifiEngine
	 *@param[in] NULL
	 *@return NULL
	 */
	public static function Close()
	{
		return ExternalInterface.call("wifi_Close");
	}

	/**
	 *@brief	start WifiEngine
	 *@param[in] NULL
	 *@return true if success or false if fail
	 */
	public static function Start():Number
	{
		return ExternalInterface.call("wifi_Start");
	}

	/**
	 *@brief	start scan AP
	 *@param[in] NULL
	 *@return true if success or false if fail
	 */
	public static function StartScan():Number
	{
		return ExternalInterface.call("wifi_StartScan");
	}
	public static function Probox_rebridge()
	{
		 return ExternalInterface.call("wifi_Probox_rebridge");
	}
	/**
	 *@brief	open wifi Engine as user setting
	 *@param[in] 
	 *return 0 success
	 *		 !0  failed  
	 */
	public static function Wifi_openmanual(ip:String, mask:String, gateway:String):Number
	{
		return ExternalInterface.call("wifi_openmanual",ip,mask,gateway);
	}
	
	/**
	*ip and mask judge
	*return 0 success
	*	    !0  failed	
	*/
	public static function ip_and_mask_judge(ip1:Number,ip2:Number,ip3:Number,ip4:Number,mask1:Number,mask2:Number,mask3:Number,mask4:Number):Number
	{
		return ExternalInterface.call("wifi_ipmaskmatchjudge",ip1,ip2,ip3,ip4,mask1,mask2,mask3,mask4);
	}

	/**
	*ip and gateway judge
	*return 0 success
	*	    !0  failed	
	*/
	public static function IpGatewayMatch(ip1:Number,ip2:Number,ip3:Number,ip4:Number,gateway1:Number,gateway2:Number,gateway3:Number,gateway4:Number):Number
	{
		return ExternalInterface.call("wifi_ipgatewaymatch",ip1,ip2,ip3,ip4,gateway1,gateway2,gateway3,gateway4);
	}
	
	/**
	 *@brief	get scan results
	 *@param[in] NULL
	 *@return AP num
	 */
	public static function GetScanResults():Number
	{
		return ExternalInterface.call("wifi_GetScanResults");
	}

	/**
	 *@brief	stop Wifi
	 *@param[in] NULL
	 *@return NULL
	 */
	public static function Stop()
	{
		return ExternalInterface.call("wifi_Stop");
	}

	/**
	 *@brief	get AP ssid
	 *@param[in] NULL
	 *@return 0 if success or -1 if fail
	 */
	public static function AddNetwork():Number
	{
		return ExternalInterface.call("wifi_AddNetwork");
	}
	
	/**
	 *@brief	get AP ssid
	 *@param[in] index : index of AP
	 *@return ap ssid
	 */
	public static function GetSsid(index:Number):String
	{
		return ExternalInterface.call("wifi_GetAPSsid",index);
	}

	/**
	 *@brief	set AP ssid
	 *@param[in] index : index of AP
	 *@param[in] ssid : ssid to set
	 *@return 0 if success or -1 if fail
	 */
	public static function SetSsid(index:Number,ssid:String):Number
	{
		return ExternalInterface.call("wifi_SetAPSsid",index,ssid);
	}

	/**
	 *@brief	get AP signal level
	 *@param[in] index : index of AP
	 *@return ap signal level
	 */
	public static function GetSignal(index:Number):Number
	{
		return ExternalInterface.call("wifi_GetSingal",index);
	}

	/**
	 *@brief	check conf file exists or not
	 *@param[in] index : index of AP
	 *@return 0 if conf file does not exist or 1 if conf file exists
	 */
	public static function GetConfExist(index:Number):Number
	{
		return ExternalInterface.call("wifi_GetConfExist",index);
	}

	/**
	 *@brief	set AP password
	 *@param[in] index : index of AP
	 *@param[in] wep_keyindex : index of wep key [0,3]
	 *@return 0 if success or -1 if fail
	 */
	public static function SetWEPKeyIndex(index:Number,wep_keyindex:Number):Number
	{
		return ExternalInterface.call("wifi_SetWEPKeyIndex",index,wep_keyindex);
	}
	
	/**
	 *@brief	set AP password
	 *@param[in] index : index of AP
	 *@param[in] password : AP password
	 *@return 0 if success or -1 if fail
	 */
	public static function SetPassword(index:Number,password:String):Number
	{
		return ExternalInterface.call("wifi_SetPassword",index,password);
	}

	/**
	 *@brief	save conf file 
	 *@param[in] NULL
	 *@return 0 if success or -1 if fail
	 */
	public static function SaveConf():Number
	{
		return ExternalInterface.call("wifi_SaveConf");
	}

	/**
	 *@brief	save added ap conf file 
	 *@param[in] NULL
	 *@return 0 if success or -1 if fail
	 */
	public static function SaveaddedAPConf():Number
	{
		return ExternalInterface.call("wifi_SaveaddedAPConf");
	}

	/**
	 *@brief	connect selected AP
	 *@param[in] NULL
	 *@return 0 if success or -1 if fail
	 */
	public static function ConnectAP():Number
	{
		return ExternalInterface.call("wifi_ConnectAP");
	}

	/**
	 *@brief	get AP Authen type
	 *@param[in] index : index of AP
	 *@return AP authen type
	 */
	public static function GetAuthenType(index:Number):Number
	{
		return ExternalInterface.call("wifi_GetAuthenType",index);
	}

	/**
	 *@brief	set AP Authen type
	 *@param[in] index : index of AP
	 *@param[in] auth_type : auth_type of AP
	 *@return 0 if success or -1 if fail
	 */
	public static function SetAuthenType(index:Number,auth_type:Number):Number
	{
		return ExternalInterface.call("wifi_SetAuthenType",index,auth_type);
	}

	/**
	 *@brief	get now status
	 *@param[in] NULL
	 *@return now wlan0 status
	 */
	public static function GetStatus():Number
	{
		return ExternalInterface.call("wifi_GetStatus");
	}

	/**
	 *@brief	set wpa_supplicant default config
	 *@param[in] NULL
	 *@return 0 if success or -1 if fail
	 */
	public static function SetDefault():Number
	{
		return ExternalInterface.call("wifi_SetDefault");
	}

	/**
	 *@brief	get index of selected AP
	 *@param[in] NULL
	 *@return index of selected AP
	 */
	public static function GetIndexofSelectAP():Number
	{
		return ExternalInterface.call("wifi_GetIndexofSelectAP");
	}

	/**
	 *@brief	set wlan IP address
	 *@param[in] ip_addr : wlan IP address
	 *@return 0 if success or -1 if fail
	 */
	public static function SetIPAddress(ip_addr:String):Number
	{
		return ExternalInterface.call("wifi_SetIPAddress",ip_addr);
	}

	/**
	 *@brief	get wlan IP address
	 *@param[in] NULL
	 *@return wlan IP address
	 */
	public static function GetIPAddress():String
	{
		return ExternalInterface.call("wifi_GetIPAddress");
	}

	/**
	 *@brief	set wlan router
	 *@param[in] router : wlan router IP address
	 *@return 0 if success or -1 if fail
	 */
	public static function SetRouter(router:String):Number
	{
		return ExternalInterface.call("wifi_SetRouter",router);
	}

	/**
	 *@brief	get wlan router
	 *@param[in] NULL
	 *@return wlan router
	 */
	public static function GetRouter():String
	{
		return ExternalInterface.call("wifi_GetRouter");
	}

	/**
	 *@brief	set wlan DNS server
	 *@param[in] DNS_index : wlan DNS index
	 *@param[in] DNS_server : wlan DNS server IP address
	 *@return 0 if success or -1 if fail
	 */
	public static function SetDNS(DNS_index:Number,DNS_server:String):Number
	{
		return ExternalInterface.call("wifi_SetDNS",DNS_index,DNS_server);
	}

	/**
	 *@brief	get wlan DNS server
	 *@param[in] NULL
	 *@return wlan DNS server
	 */
	public static function GetDNS():String
	{
		return ExternalInterface.call("wifi_GetDNS");
	}

	/**
	 *@brief	get signal level of the connected ap 
	 *@param[in] NULL
	 *@return signal level
	 */
	public static function GetSignalLevel():Number
	{
		return ExternalInterface.call("wifi_get_signal_level_as");
	}

	/**
	 *@brief	delete conf file
	 *@param[in] index : index of AP
	 *@return 0 if success or -1 if fail
	 */
	public static function DeleteConf(index:Number):Number
	{
		return ExternalInterface.call("wifi_DeleteConf",index);
	}

	/**
	 *@brief	auto connect AP which has been connected before
	 *@param[in] NULL
	 *@return 0 if success or -1 if fail
	 */
	public static function AutoConnect():Number
	{
		return ExternalInterface.call("wifi_Autoconnect");
	}

/**
	 *@brief	process  wifi mode change ,such ad switch wifi client mode to wifi ap mode 
	 *@param[in] Wifimode :1 :switch to client 2:switch to ap mode 3: close wifi function 4 :restart wifi function
	 *@return 0 if success or -1 if fail
 */

 	public static function Wifidonglechange(Wifimode:Number):Number
	{
		return ExternalInterface.call("wifi_donglemodechange",Wifimode);
	}
	/**
		 *@brief	process  get softap ssid
		 *@param[in] NULL
		 *@return ssid
	 */

 	public static function getssid():String
	{
		return ExternalInterface.call("wifi_getsoftapssid");
	}
	public static function getssid_Probox24g():String
	{
		return ExternalInterface.call("wifi_getsoftapssid_Probox24g");
	}
	
		/**
		 *@brief	process  get softap channel
		 *@param[in] NULL
		 *@return channel
	 */
 	public static function getchannel():String
	{
		return ExternalInterface.call("wifi_getsoftapchannel");
	}
	/**
	 *@brief	process  get softap security
	 *@param[in] NULL
	 *@return security
	 */
	public static function getsecurity():String
	{
		return ExternalInterface.call("wifi_getsoftapmode");
	}
	/**
	 *@brief	process  get softap psk
	 *@param[in] NULL
	 *@return psk
	 */
	public static function getpassword():String
	{
		return ExternalInterface.call("wifi_getsoftappsk");
	}
	/**
	 *@brief	process  set soft ap 
	 *@param[in] softap_mode :softap security
	 *@return NULL
	 */
	public static function getsoftapconfiginfo(softap_mode:Number)
	{
		 ExternalInterface.call("wifi_getsoftapconfiginfo",softap_mode);
	}
	/**
	 *@brief	process put down the softap info into case 
	 *@param[in] softap_ssid :softap ssid
	 			 softap_psk :softap psk
				 softap_channel :softap channel
	 *@return NULL
	 */
	public static function getsoftapinputconfiginfo(softap_ssid:String,softap_psk:String,softap_channel:String)
	{
		 ExternalInterface.call("wifi_getsoftapinputconfiginfo",softap_ssid,softap_psk,softap_channel);
	}
		/**
	 *@brief	process  get softap best channel
	 *@param[in] NULL
	 *@return best channel
	 */
	public static function getbestchannel():String
	{
		 return ExternalInterface.call("wifi_getbestchannel");
	}
	/**
	 *@brief	process  ez remote
	 *@param[in] direction :0 :ON;1 :OFF
	 *@return 0 if successfully,-1 if fail
	 */
	public static function remotecontrol(direction:Number):Number
	{
		 return ExternalInterface.call("wifi_remotecontrol",direction);
	}
	/**
	 *@brief	get the dongle type
	 *@param[in] NULL
	 *@return 1 :realtek 2:ralink
	 */
	public static function getdongltype():Number
	{
		 return ExternalInterface.call("wifi_getdongletype");
	}
	/**
	 *@brief	get the  connect successfully ssid
	 *@param[in] NULL
	 *@return ssid
	 */
	public static function getconnectedssid():String
	{
		 return ExternalInterface.call("wifi_getconnectedssid");
	}

		/**
	 *@brief	get the connecting  ssid
	 *@param[in] NULL
	 *@return ssid
	 */
	public static function getconnectingssid():String
	{
		 return ExternalInterface.call("wifi_getconnectingssid");
	}
	/**
	 *@brief	get the psk of connecting or connect successfully ap
	 *@param[in] NULL
	 *@return psk
	 */
	public static function getpskfordisplay():String
	{
		 return ExternalInterface.call("wifi_getpasswordfordisplay");
	}
	/**
	 *@brief	get the status of wifi
	 *@param[in] NULL
	 *@return status ,see it in WIFI_DONGLE_PLUG_IN etc
	 */
	public static function getdonglestatus():Number
	{
		 return ExternalInterface.call("wifi_getdonglestatus");
	}
		/**
	 *@brief	get ez wifi valid 
	 *@param[in] NULL
	 *@return 1 :true ;0:false
	 */
	public static function checkwifidisplayvalid():Number
	{
		 return ExternalInterface.call("wifi_checkwifidisplayvalid");
	}

	/**
	 *@brief	auto restart AP mode 
	 *@param[in] NULL
	 *@return NULL
	 */
	 public static function restart_wifi_concurrent_mode():Number
	{
		 return ExternalInterface.call("wifi_restartapmode");
	}
	/**
	 *@brief	get  ez remote start flag
	 *@param[in] NULL
	 *@return the flag  0:start error or close   1:start ok
	 */
	 public static function get_ez_remote_start_flag():Number
	{
		 return ExternalInterface.call("wifi_ezremoteflag");
	}
	/**
	 *@brief	exchange wifi ap 
	 *@param[in] NULL
	 *@return NULL
	 */

	 public static function wifi_exchange_ap():Number
	{
		 return ExternalInterface.call("wifi_exchangeap");
	}

	/**
	*@brief  Send the answer massage to APP
	*@param[in]	The massage to send
	**/
	 public static function wifi_feedbackcast(cmd_as:String):Number
	{
		 return ExternalInterface.call("wifi_feedbackcast", cmd_as);
	}

	/**
	*@brief  Set the psk_changed flag
	*@param[in]	NULL
	**/
	 public static function psk_changed()
	{
		 ExternalInterface.call("wifi_psk_changed");
	}
	/**
	*@brief  Judge whether the last connected router AP exist.
	*@param[in]	NULL
	*@return		1: exist;
	**/
	 public static function isLastRouterApExist():Number
	{
		 return ExternalInterface.call("wifi_isLastRouterApExist");
	}
	/**
	*@brief  websetting wifi connect delay.
	*@param[in]	NULL
	*@return		NULL;
	**/
	 public static function websettingConnect()
	{
		 ExternalInterface.call("wifi_web_connect");
	}
	/**
	*@brief  Let softap can not be found.
	*@param[in]	NULL
	*@return		NULL
	**/
	 public static function softapIgnore()
	{
		 ExternalInterface.call("wifi_softapIgnore");
	}
	/**
	*@brief  Let softap can be found.
	*@param[in]	NULL
	*@return		NULL
	**/
	 public static function softapShow()
	{
		 ExternalInterface.call("wifi_softapShow");
	}

	/**
	*@brief  close wifi connected.
	**/
	 public static function wifi_disconnect():Number
	{
		 return ExternalInterface.call("wifi_disconnect");
    	}
	/**
	*@brief count the access devices in softap mode
	*@return the number of access devices
	*/
	public static function wifi_countAccessDevices_softap():Number
	{
		 return ExternalInterface.call("wifi_countAccessDevices");
	}
	 public static function reset_process_wifi_function_switch_flag():Number
	{
		 return ExternalInterface.call("reset_process_wifi_function_switch_flag");
	}
	/**
	*@brief enable softap mode after return from miracast
	*@return NULL
	*/
	public static function wifi_start_softap()
	{
		 return ExternalInterface.call("wifi_start_softap");
	}
	 
	 public static function set_Airplay_Id(airplay_id:String):Number
	{
		 return ExternalInterface.call("set_Airplay_Id",airplay_id);
	}
	 public static function set_DLNA_Id(dlna_id:String):Number
	{
		 return ExternalInterface.call("set_DLNA_Id",dlna_id);
	}
	public static function wifi_get_connect_mac_address(port:Number):String
	{
		 return ExternalInterface.call("wifi_get_connect_mac_address", port);
	} 
	public static function getConnectionStatus():Number
	{
		 return ExternalInterface.call("wifi_get_connection_status");
	} 
	/**
	*@brief start the wifi routing
	*/
	public static function WifiStartRouting()
	{
		 ExternalInterface.call("wifi_StartRouting");
	}

	/**
	*@brief disable the wifi routing
	*/
	public static function WifiDisableRouting()
	{
		 ExternalInterface.call("wifi_DisableRouting");
	}
	public static function hostcontrol_user_clean()
	{
		ExternalInterface.call("hostcontrol_user_clean");
	} 		
	/**
	*@brief Get 3G/4G hotspot connect status
	*/
	public static function getHotspotStatus():Number
	{
		 return ExternalInterface.call("wifi_get_hotspot_status");
	} 
	/**
	*@brief Set auto connect enable or disable.
	*@param[in]	val: 0-->Disable		1-->Enable
	*/
	public static function setAutoConnect(val:Number)
	{
		 ExternalInterface.call("wifi_setAutoConnect", val);
	} 
	/**
	*@brief Which Wi-Fi dongle is.
	*/
	public static var WIFI_REALTEK_CU=0;
	public static var WIFI_REALTEK_DU=1;
	public static var WIFI_REALTEK_8188EU=2;
	public static var WIFI_REALTEK_8192EU=3;
	public static var WIFI_REALTEK_8811AU=4;
	public static var WIFI_REALTEK_8821CU=5;
	public static function getWifiModelType()
	{
		 return ExternalInterface.call("wifi_getWifiModelType");
	} 
	/**
	*@brief Set Wi-Fi channel mode, 5G or 2.4G.
	*/
	public static var CHN_RTL_BAND_5G = 0x05;
	public static var CHN_RTL_BAND_24G = 0x02;
	public static function setChannelMode(val:Number)
	{
		 ExternalInterface.call("wifi_setChannelMode", val);
	} 
	public static function getChannelMode()
	{
		 return ExternalInterface.call("wifi_getChannelMode");
	} 
	public static function getCurrentChannel()
	{
		 return ExternalInterface.call("wifi_get_cur_channel");
	} 
	public static function getDefaultPsk()
	{
		 return ExternalInterface.call("wifi_get_default_psk");
	} 
}
