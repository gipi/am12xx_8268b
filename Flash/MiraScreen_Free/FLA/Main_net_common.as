/*****************************     file start        **************************************************/
/**  @author   luoyuping
**	 @data   2013/06/27
**	used for setting wifi in Main.swf		
**/
/*****************************       variable        ***************************************************/
var wifi_select:Number=0;                      //the index of clientap had connected  
var show_softap_flag:Number=0;                 //it will be set 1 affer the information of softap had show
var show_clientap_flag=0;                      //it will be set 1 affer the information of clientap had show
var save_show_clientap_flag:Number=0;//0:init   1:client enable
var scan_clentap_flag=0;                       //it will be set 1 affer scanning the clientap
var start_concurrent_softap_mode:Number=0;     //the flag of softap,1:on 0:off
var start_concurren_clientap_mode:Number=0;    //the flag of clientap,1:on 0:off
var count_clean_clientap:Number=0;             //count backwards to clean the clentap information
var	wifi_mode_get_from_case:Number=-1;         //wifi mode
var hotspotStatus:Number = -1;				   // hotspot status
var pre_wifi_mode_get_from_case:Number=-1;     //wiif mode last time
var softp_start2show_timer:Number=-1;          //the timer to start show softap information
var set_dongle_timer:Number=-1;                //the timer to check netstatus
var checkscanres_timer:Number=-1;              //the timer to check if there is any clientap that  has connected
var SOFTAP_INDEX:Number=0;                     //index about softaop
var CLIENTAP_INDEX:Number=1;                   //index about clientap
var interval_scan:Number = -1;
var topBarShowCompleted:Boolean = false;
var MiracastHomePage:Boolean = false;
var MiracastDefault:Boolean = false;

var ezBgShowCompleted:Boolean = false;
//EZCastPro LAN value, richardyu 112514
var EZCASTPRO_LAN_ENABLE:Boolean=(EZCASTPRO_MODE>0&&EZCASTPRO_MODE!=8251&&EZCASTPRO_MODE!=8075)?true:false;//richardyu 112514
var ezcastpro_lan_plugin:Number=0;// plug in or out
var ezcastpro_lan_working:Number=0;//-1(unstable)->0(stop) or 1(DHCP) or 2(Manual)

var fw_update_main_flag:Boolean = false;
var check_status_timer_id;
var check_ota_status_count:Number = 0;
var isIconShowInFlash:Boolean = true;
var qr_encode:Boolean = false;
var Jpegpath:String = "/tmp/ezcastdownload.jpg";
var clientAP:String = "";
var softapIgnoreCount:Number = 0;
var softapIgnoreCountEnable:Boolean = false;
var firstStartSoftap:Boolean = true;
var wifiStartWaitCount:Number = 60;
var isWifiWarningShowing:Boolean = false;
var connections_num:Number = 0;
var connections_check_count:Number = 0;
var isIpError:Boolean = false;
var hide_wifiWarning_time:Number = 5000; // -1: Never to hide wifi warning message
var hide_wifiWarning_process:Number = -1;

var schema_set_time_count:Number = 0;
var ota_check_count:Number = 0;
var client_ap_changed_flag:Boolean = false;
var tocheckotaversion_inter:Number = 0;
var haveCheckOtaVersion:Boolean = false;
//var set_Airplay_Id_flag:Number = 1;
var internet_status:Number = 0;
var current_lite_mode:Number = 0;
var ezcast5gChnCount:Number = 0;
var clientConnectStatus:Boolean = false;
var isFirstOTACheck:Boolean = true;
var isGreenLedOn:Boolean = false;
var toShowSoftap:Boolean = false;
var isLanMiracodeShowing:Boolean = false;
var iswLanMiracodeShowing:Boolean = false;
var donglewLanIpShowing:Boolean = false;
var dongleLanIpShowing:Boolean = false;
var signal_data:Object = {signal_level_prev:-1, signal:0, count:0};
var webconnectionsnum:Boolean = false;

function checkCurChannel(){
	var curChn:Number = WifiEngine.getCurrentChannel();
	if(curChn > 13){
		TopBarSWF.icon5g();
		if(EZCASTPRO_MODE!=8075&&MainSWF.connectMode == SystemInfo.DIRECT_ONLY && wifi_mode_get_from_case==WifiEngine.WIFI_CONCURRENT_CLIENT_GETAUTOIP_OK){
			SystemInfo.setConnectMode(SystemInfo.ROUTER_ALLOW);
			MainSWF.connectMode = SystemInfo.getConnectMode();
			if(MainSWF.currentSWF == SETTING_SWF){
				MainSWF.currentMC.set_ctl_mc();
			}
		}
	}else{
		TopBarSWF.icon5gClean();
	}
}

/*****************************       function        ***************************************************/
function softapIgnore(){
	return;
	if(firstStartSoftap){
		trace("_____________________sotfapIgnore");
		if(WifiEngine.isLastRouterApExist()){
			trace("___________________ main.swf softap ignore");
			WifiEngine.softapIgnore();
			softapIgnoreCount = 30;
			softapIgnoreCountEnable = true;
		}else{
			trace("_____________________softapShow");
			softapShow();
		}
	}
}
function softapShow(){
	return;
	if(firstStartSoftap){
		trace("___________________ main.swf softap show");
		softapIgnoreCountEnable = false;
		firstStartSoftap = false;
		WifiEngine.softapShow();
	}
}

function check_OTA_ver_status()
{
	check_ota_status_count++;

	var ret = SystemInfo.getOtaCheckStatus();
	trace("----ret="+ret);
	if(ret == 1){
		if(check_status_timer_id > 0){
			clearInterval(check_status_timer_id);
			check_status_timer_id = NULL;
		}
		var server_version = SystemInfo.getOtaServerVersion();
		trace("----server_version="+server_version);
		if(server_version != "error" && server_version != "newest") {
			var ota_enforce_val = SystemInfo.get_ota_enforce();
			if(MainSWF.currentSWF == SETTING_SWF) {
				MainSWF.currentMC.fw_update_show();  
			} 
			showUpgradeIcon();
			if(ota_enforce_val == 1){
				isOtaEnforce = true;
				feedbackenable = false;
				if(MainSWF.currentSWF != "test.swf")
				{
					var retval = SystemInfo.ota_upgrade(1,"http://www.iezvu.com/upgrade/ezcast/ezcast.bin");	
					if(retval==0)
					{
						usermanualVisible(false);
						loadswf("OTA_download.swf");
					}
					else
						trace("OTA download failed");
				   
				}
					
			}
			//HaveEnterWifiSetting = false;
			if(VersionChecked != true)
				haveNewVersion = true;
		}else{
			hideUpgradeIcon();
		}
		VersionChecked = true;
	}else if(check_ota_status_count >= 20){
		if(check_status_timer_id > 0){
			clearInterval(check_status_timer_id);
			check_status_timer_id = NULL;
		}
		check_ota_status_count = 0;
		trace("[check_OTA_ver_status] Network error!!!");
	}
}

var isWaitConnect:Boolean = false;
var connCount:Number = 0;
/******* show softap information **********/
function show_softap_info(){
	if(softp_start2show_timer > 0){
		clearInterval(softp_start2show_timer);
		softp_start2show_timer = 0;
	}
	showSoftapInfo();
}
function to_show_softap_info(){
	if(!isWaitConnect){
		var ret = WifiEngine.isLastRouterApExist();
		if(ret == 0){
			if(softp_start2show_timer > 0){
				clearInterval(softp_start2show_timer);
				softp_start2show_timer = 0;
			}
			softp_start2show_timer = setInterval(show_softap_info,3000);
			return;
		}else
			isWaitConnect = true;
	}
}

/******* show clientap information **********/
function lengthCalculationFontSize_25(str:String):Number{
	var i = 0;
	var len:Number = 0;
	for(i=0; i<str.length; i++){
		var ch = str.charAt(i);
		if((ch>="A" && ch<="Z") || ch=="_"){
			//trace("A: ch = "+ch);
			len += 17;
		}else if((ch>=" " && ch<="~")){
			//trace("a: ch = "+ch);
			len += 14;
		}else{
			//trace("o: ch = "+ch);
			len += 17;
		}
	}

	return len;
}

function show_clentap_info(){
	if(isWaitConnect){
		showSoftapInfo();
	}
	clientAP = WifiEngine.getconnectedssid();//WifiEngine.GetIPAddress();
	TopBarSWF.iconPop(clientAP);
	if("setting.swf" == currentSWF){
		//trace(":::::: connection successful!!!");
		currentMC.set_ctl_mc();
		currentMC.auto_connect_show();
	}else if(EZCAST_SWF == currentSWF){
		var ip:String;
		ip=SystemInfo.getIP(0);
		if(ip != "" && ip!="error")
		{
			currentMC.show_miracode(ip);
			iswLanMiracodeShowing=true;
			currentMC.show_dongle_ip(ip);
			donglewLanIpShowing=true;
		}
		currentMC.show_router_ssid(clientAP);
		
	}else if(EZMIRR_LITE_SWF == currentSWF){
		//currentMC.wifi_scan();
		if(current_lite_mode != WidiEngine.MIXED_STATUS_CLIENT_P2P && currentMC.wifi_display_on)
			currentMC.wifiDisplayStop();
		
		currentMC.start();
	}
	if(EZCAST5G_ENABLE)
		checkCurChannel();
	
	if(get_ConnectMode() == SystemInfo.ROUTER_ONLY){
		//if(TopBarSWF.isCurrectStrSsid())
		trace("-------------------------------------12345678990-----------------");
		showRouterInfoForP2pClient();
	}
}

function signal_data_clean(){
	signal_data.signal =0;
	signal_data.count = 0;
}

function signal_data_clean_all(){
	signal_data_clean();
	signal_data.signal_level_prev = -1;
}

function signal_plus_and_square(signal:Number):Number{
	if(signal >= 0 && signal <= 100){
		signal_data.signal += signal;
		signal_data.count++;
	}
	if(signal_data.count >= 5 || signal_data.signal_level_prev < 0){
		var signl_tmp:Number = Math.floor(signal_data.signal/signal_data.count);
		signal_data.signal_level_prev = Math.floor(signl_tmp/25)+1;
		signal_data_clean();
	}

	return signal_data.signal_level_prev;
}

/******* get the signal strength of clentap **********/
function get_wifi_sinal():Number{
	var signl_strlen_tmp=0;
	signl_strlen_tmp = WifiEngine.GetSignalLevel();
	//trace("signl_strlen_tmp="+signl_strlen_tmp);
	if(currentSWF == "test.swf"){
		//trace("signal is: "+signal);
		currentMC.show_wifi_test_signal(signl_strlen_tmp);
	}
	signl_strlen_tmp=signal_plus_and_square(signl_strlen_tmp); //Math.floor(signl_strlen_tmp/25)+1;
	//trace("signl_strlen_tmp="+signl_strlen_tmp);
	return signl_strlen_tmp;
}

/****** show the signal strength of clentap **********/
function show_wifi_sinal(){
	var signal:Number=0;
	if(isLanConnect())
		return;
	var status:Number = WifiEngine.getConnectionStatus();
	//trace("::::::: status: "+status);
	if(status == WifiEngine.WIFI_COMPLETED){
		signal=get_wifi_sinal();
		//trace("signal..."+signal);
		if(signal > 4)
			signal = 4;
		if(isIpError){
			TopBarSWF.iconSignalIpErr(signal);
		}else{
			TopBarSWF.iconSignal(signal);
		}
		clientConnectStatus = true;
	}else{
		TopBarSWF.iconSignalNull();
	}
}

/****** get the results scanning clentap **********/
function get_scanres(){
	var ssid_select_tmp:String="";
	var AP_total =-1;
    AP_total = WifiEngine.GetScanResults();
	ssid_select_tmp=WifiEngine.getssidfordisplay();
	if(AP_total==0){
		
		return;
	}
	else
	{
		wifi_select=WifiEngine.GetIndexofSelectAP();
		//trace("wifi_select====="+wifi_select);
		if(wifi_select>-1){
			//SystemInfo.Putwifimode(WifiEngine.WIFI_CONCURRENT_CLEINT_GETAUTOIP_OK);
			clearInterval(checkscanres_timer);
			checkscanres_timer = null;
			
		}
		
	}
}
/****** scanning clentap **********/
function scan_clientap(){
	
	var rtn:Number=-1;
	trace("-------------scan_clientap");
	rtn=WifiEngine.StartScan();
	if(WifiEngine.getdongltype()==2){
		WifiEngine.StartScan();
	}
	clearInterval(check_scan_clientap);
	if(rtn==0){
		if(checkscanres_timer!=null){
			clearInterval(checkscanres_timer);
			checkscanres_timer = null;
		}
		checkscanres_timer=setInterval(get_scanres,1000);
		trace("checkscanres_timer="+checkscanres_timer);
	}
	
}

/****** open and start scan clentap **********/
function open_and_scan_clientap(){
	if((start_concurrent_softap_mode==1)&&(scan_clentap_flag==0)){
		scan_clentap_flag=1;
	 	WifiEngine.Open();
     		WifiEngine.Start();
		scan_clientap();	
	}
}

function show_client_loading(value:Number){
	hotspotStatus = WifiEngine.getHotspotStatus();
	if(MainSWF.topBarShowCompleted && hotspotStatus != WifiEngine.AS_HOTSPOT_UNCONNECT && hotspotStatus != WifiEngine.AS_HOTSPOT_TIMEOUT && hotspotStatus != WifiEngine.AS_HOTSPOT_FAIL){
		TopBarSWF.iconHotspot(true);
	}else{
		TopBarSWF.iconHotspot(false);
	}
	if(value >= 2 && value <= 6){
		trace("[Main_net_common.as] [show_client_loading] connecting value: "+value);
		if(isIconShowInFlash)
			client_loading_mc._visible=true;
		show_clientap_flag = 0;
		TopBarSWF.iconPopClean();
		TopBarSWF.iconWifiConnecting();
		count_clean_clientap=0;
		client_ap_changed_flag = true;	// ******** client_ap_changed_flag = true means to connected a new internet AP, and then the file dongleInfo.json should record this AP info. **********
	}else{
		if(MainSWF.topBarShowCompleted && (hotspotStatus == WifiEngine.AS_HOTSPOT_READY || hotspotStatus == WifiEngine.AS_HOTSPOT_SCAN || hotspotStatus == WifiEngine.AS_HOTSPOT_CONNECTING))
			TopBarSWF.iconWifiConnecting();
		else
			TopBarSWF.iconWifiConnectingClean();
		
		if(client_ap_changed_flag){
			//softapShow();
		//	if(currentSWF != "setting.swf")
		//		ExternalInterface.call("wifi_StartScan");			
			SystemInfo.json_set_value(4, NULL);	//param: 4 is set internet ap ssid
			client_ap_changed_flag = false;
		}
	}
	if(MainSWF.topBarShowCompleted && hotspotStatus == WifiEngine.AS_HOTSPOT_SUCCESS){
		if(!isHotspotMode()){
			isHotspotmode = true;
			if(currentSWF == SETTING_SWF){
				currentMC.itemRefresh();
			}
		}
	}else{
		isHotspotmode = false;
	}
}

function tocheckotaversion(){
	if(tocheckotaversion_inter > 0){
		clearInterval(tocheckotaversion_inter);
		tocheckotaversion_inter = NULL;
	}
	if(haveCheckOtaVersion){
		return;
	}
	ota_check_count = 0;
	SystemInfo.ota_upgrade(4,"http://www.iezvu.com/upgrade/ezcast/ezcast.conf"); 
	if(check_status_timer_id > 0){
		clearInterval(check_status_timer_id);
		check_status_timer_id = NULL;
	}
	check_status_timer_id = setInterval(check_OTA_ver_status, 1000);	
}

function clearcheckotaversion(){
	trace("_________clearcheckotaversion");
	if(tocheckotaversion_inter > 0){
		clearInterval(tocheckotaversion_inter);
		tocheckotaversion_inter = NULL;
	}
	if(check_status_timer_id > 0){
		clearInterval(check_status_timer_id);
		check_status_timer_id = NULL;
	}
}


var warnInterval:Number = -1;
var warnTimeCount:Number = 30;

function warnTimerOpt(){
	if(warnTimeCount > 0){
		warning_mc.warn_info2.text = warnning_str2+(--warnTimeCount)+warnning_str3;
	}else{
   		SystemInfo.reboot_system();
	}
}


function wifiWarning(){
	if(LAN_ONLY)
		return;
	
	if(currentSWF != "test.swf"){
		this.attachMovie("warning","warning_mc",this.getNextHighestDepth());
		warning_mc._x=467; //1402;
		warning_mc._y=268; //153;
		trace("warnning: "+warnning_str);
		warning_mc.warn_info.text = warnning_str;
		if(EZCASTPRO_MODE!=8074 && EZCASTPRO_MODE!=8250&& EZCASTPRO_MODE!=8075){
			warnTimeCount = 30;
			warning_mc.warn_info2.text = warnning_str2+warnTimeCount+warnning_str3;
			if(warnInterval > 0){
				clearInterval(warnInterval);
				warnInterval = -1;
			}
			warnInterval = setInterval(warnTimerOpt, 1000);
		}
		isWifiWarningShowing = true;
		
		if((EZCASTPRO_MODE==8074||EZCASTPRO_MODE==8075) && hide_wifiWarning_time > 0){
			trace("hide wifi warning in 5 seconds.....");
			if(hide_wifiWarning_process > 0){
				clearInterval(hide_wifiWarning_process);
				hide_wifiWarning_process = -1;
			}
			hide_wifiWarning_process = setInterval(hide_wifiWarning_delay, hide_wifiWarning_time);
		}
	}
}

function cleanWarning(){
	if(warnInterval > 0){
		clearInterval(warnInterval);
		warnInterval = -1;
	}
	isWifiWarningShowing = false;
	warning_mc._visible=false;
	warning_mc.removeMovieClip();
}

function hide_wifiWarning_delay(){
	trace("hide wifi warning message....");
	if(hide_wifiWarning_process > 0){
		clearInterval(hide_wifiWarning_process);
		hide_wifiWarning_process = -1;
	}
	if(isWifiWarningShowing){
		cleanWarning();
	}
}

var autoConnectCount:Number = 0;
var clientConnect:Boolean = false;
function show_connections_num(){
	connections_check_count++;
	if(connections_check_count > 2){
		connections_check_count = 0;
		//main_bar.icon_mc.connections_mc._visible = true;
		var num:Number = WifiEngine.wifi_countAccessDevices_softap();
		if(EZCASTPRO_MODE!=0 && connections_num>0 && num==0){
			if(internet_status==1||SystemInfo.getRouterEnable()==0){//no router connected or Direct link only
				trace("only softap accessed, user from "+connections_num+" to "+num+", clean hostcontrol user!!");
				WifiEngine.hostcontrol_user_clean();
			}
		}	
		if(num >= 0){
			connections_num = num;
			TopBarSWF.iconConnectNumber(num);
			if(num == 0){
				if(isGreenLedOn){
					//SystemInfo.setLEDGreenOff();
					isGreenLedOn = false;
					trace("^^^^^^^^^^^^^^^^^ LED GREEN OFF ^^^^^^^^^^^");
				}
			}else if(!isGreenLedOn){
				//SystemInfo.setLEDGreenOn();
				isGreenLedOn = true;
				trace("^^^^^^^^^^^^^^^^^ LED GREEN ON ^^^^^^^^^^^");
			}
		} 
		if(!clientConnect && lanstatus_priv == 0 && num == 0){
			autoConnectCount++;
		}else{
			autoConnectCount = 0;
		}
		if(autoConnectCount == 25){
			//trace("-------------show_connections_num");
			WifiEngine.StartScan();
		}
		if(autoConnectCount > 30){
			autoConnectCount = 0;
			trace("Try to auto connect!!!");
			WifiEngine.AutoConnect();
		}
	}
}

function check_lite_status(){
	var current_mode = current_lite_mode;
	if(current_mode == WidiEngine.MIXED_STATUS_CLIENT_SOFTAP){
		show_wifi_sinal();
	}
}

function router_disconnect(){
	clientConnect = false;
	if(!isLanConnect()){
		clientConnectStatus = false;
		TopBarSWF.iconSignalNull();
	}
	if(EZCAST_SWF == currentSWF){
		currentMC.clean_router_ssid();
	}
	TopBarSWF.iconPopClean();
}

function iconApLinkStatus()
{
	//trace("*******iconApLinkStatus connectMode="+MainSWF.connectMode);
	if(MainSWF.connectMode==2)
		TopBarSWF.iconApLinkEnable(false);
	else
		TopBarSWF.iconApLinkEnable(true);
}



/****** clientao information setting **********/
function net_top_bar_setting(){
	var concurrent_softap_mode_tmp:Number = 0;
	var ip_error_flag:Boolean = false;
	var wifistatus = WifiEngine.GetStatus();
	//trace("----wifi_mode_get_from_case="+wifi_mode_get_from_case+"----wifistatus="+wifistatus);	
	var internetAPstatus = 1;		//Client status flag
	show_client_loading(wifistatus);
	if(TEST_ENABLE&&(currentSWF != EZMIRR_PROBOX_SWF))
		test_status_check_timer(wifistatus);
	if(is_ezcast_lite()){
		current_lite_mode = WidiEngine.GetMode();
		if(MIRR_SWF == currentSWF){
			currentMC.connection_status_check();
		}else{
			check_lite_status();
		}
	}
	if(softapIgnoreCountEnable){
		softapIgnoreCount--;
		if(softapIgnoreCount < 0){
			softapShow();
		}
	}
	if(wifiStartWaitCount > 0){
		wifiStartWaitCount--;
	}else if(wifiStartWaitCount == 0){
		wifiStartWaitCount = -1;
		wifiWarning();
	}
	
	if((WifiEngine.WIFI_INACTIVE == wifistatus) || (WifiEngine.WIFI_SCANNING == wifistatus) || (10 == wifistatus)|| (WifiEngine.WIFI_DISCONNECTED == wifistatus)){////Client is disconnect.
		internetAPstatus = 0;   // *********** internetAPstatus = 0 means client is not connected to any AP, and then should not to show the wifi signal icon ***********
		if(wifi_mode_get_from_case==9)
			concurrent_softap_mode_tmp=1;
		//trace("[internetAPstatus = "+internetAPstatus+"]signal show disable");
		if(!is_ezcast_lite()){
			router_disconnect();
		}
		//showIconInDisplay();
		//clientap_mc._visible = false;
		//TopBarSWF.iconPopClean();
	}
	if(wifi_mode_get_from_case==WifiEngine.WIFI_CONCURRENT_CLIENT_GETAUTOIP_OK || wifi_mode_get_from_case == WifiEngine.WIFI_CONCURRENT_CLIENT_GETAUTOIP_ERR || wifistatus == WifiEngine.WIFI_AUTO_IP_SUCCESS || wifistatus == WifiEngine.WIFI_AUTO_IP_ERROR){//WifiEngine.WIFI_CONCURRENT_CLEINT_GETAUTOIP_OK){
		
		start_concurren_clientap_mode=1;
		save_show_clientap_flag=1;
		concurrent_softap_mode_tmp=1;
		// *********** internetAPstatus = 0 means client is not connected to any AP, and then should not to show the wifi signal icon ***********
		//trace("internetAPstatus="+internetAPstatus);
		if(internetAPstatus){
			clientConnect = true;
			if(show_clientap_flag==0){
				//trace("luo----------------------wifi_mode_get_from_case="+wifi_mode_get_from_case);
				 show_clentap_info();
				 show_clientap_flag=1;
				 signal_data_clean_all();
			}
			if(!is_ezcast_lite())
        		show_wifi_sinal();
		}

		if(EZMIRR_LITE_SWF != currentSWF){
			var currMode:Number = MainSWF.connectMode;
			MainSWF.get_ConnectMode();
			if(MainSWF.connectMode != currMode){
				currentMC.wifiDisplayStop();
				show_clentap_info();

				//currentMC.updata_connect_icon();	//if want refresh ui immediately,open this.
				iconApLinkStatus();
				currentMC.start();
			}
		}
		
		// ******* get auto ip error ********
		if(wifi_mode_get_from_case == WifiEngine.WIFI_CONCURRENT_CLIENT_GETAUTOIP_ERR){
			ip_error_flag = true;
		}
		else{
			//trace("---fw_update_main_flag="+fw_update_main_flag);
			if(!fw_update_main_flag){
				if(isFirstOTACheck || (currentSWF != STREAM_SWF) || (EZCAST_STATUS_NULL == SystemInfo.wifidisplay_gets_tatus())){
					fw_update_main_flag = true;
					isFirstOTACheck = false;
					trace("***************  check ota version, otaCheckDelayTime = "+otaCheckDelayTime);
					if(tocheckotaversion_inter > 0){
						clearInterval(tocheckotaversion_inter);
						tocheckotaversion_inter = NULL;
					}
					tocheckotaversion_inter = setInterval(tocheckotaversion, 3000);
				}
            } 
		}

	}else {
		start_concurren_clientap_mode=0;
		//trace("@@@@11signal_show_disable");
		if(!is_ezcast_lite()){
			//trace("@@@@22signal_show_disable");
			if(MainSWF.EZCASTPRO_MODE!=0)//for pro
			{
				//currentMC.updata_connect_icon();
				iconApLinkStatus();
			}
			router_disconnect();
		}
	}
	if(wifi_mode_get_from_case==WifiEngine.WIFI_CONCURRENT_SOFTAP_ENABLE || wifi_mode_get_from_case==WifiEngine.WIFI_CONCURRENT_CLIENT_ENABLE){
		wifiStartWaitCount = -1;
		if(isWifiWarningShowing){
			cleanWarning();
		}
		concurrent_softap_mode_tmp=1;
	}
	start_concurrent_softap_mode = concurrent_softap_mode_tmp;
	if(start_concurrent_softap_mode){
		currentMC.ssid_name.text = WifiEngine.getssid();
		if(!is_ezcast_lite()){
			show_connections_num();

			
			if(schema_set_time_count%30 == 0){
				checkCurChannel();
			}
		}
	}

	if(is_ezcast_lite()){
		var current_mode = current_lite_mode;
		//trace(":::::::: current_mode: "+current_mode+", WidiEngine.MIXED_STATUS_CLIENT_P2P: "+WidiEngine.MIXED_STATUS_CLIENT_P2P+", WidiEngine.MIXED_STATUS_CLIENT_SOFTAP: "+WidiEngine.MIXED_STATUS_CLIENT_SOFTAP);
		if(current_mode == WidiEngine.MIXED_STATUS_CLIENT_P2P || current_mode == WidiEngine.MIXED_STATUS_CLIENT_SOFTAP || current_mode == -1){
			if(!client_ap_changed_flag)
				show_wifi_sinal();
			if(current_mode == WidiEngine.MIXED_STATUS_CLIENT_P2P)
				ezcast5gChnCount = 0;
		}
		if(current_mode == WidiEngine.MIXED_STATUS_P2P_SOFTAP || current_mode == WidiEngine.MIXED_STATUS_CLIENT_SOFTAP || current_mode == -1){
			show_connections_num();
			 //if(!webconnectionsnum){
                  // show_connections_num();
                 //  trace("show_connections_num++812+show_connections_num++");
               //    webconnectionsnum = false;
           //  }
		}
	}
	isIpError = ip_error_flag;

}

/****** check and deal the wifi status **********/
function check_the_netstatus(){
	schema_set_time_count++;
	if(schema_set_time_count >= 300){
		set_schema();
		schema_set_time_count = 0;
		ota_check_count++;
		if(ota_check_count >= 12){
			fw_update_main_flag = false;
			haveCheckOtaVersion = false;
			ota_check_count = 0;
		}
	}
	systemNotBusyCount();
	if(set_dongle_timer > 0){
		clearInterval(set_dongle_timer);
		set_dongle_timer = -1;
	}	
	if(LAN_ONLY){
		set_dongle_timer=setInterval(check_the_netstatus,1000);  
		return;
	}
	
	if(isWaitConnect){
		if(connCount >= 10){
			showSoftapInfo();
		}else
			connCount++;
	}
	wifi_mode_get_from_case = SystemInfo.Getwifimode();
   	// SystemInfo.MemCheck();
	//trace("wifi_mode_get_from_case=============================="+wifi_mode_get_from_case+"-------------GetStatus="+WifiEngine.GetStatus());
	//trace("pre_wifi_mode_get_from_case=============================="+pre_wifi_mode_get_from_case);
	
	if((pre_wifi_mode_get_from_case == 1 || pre_wifi_mode_get_from_case == -1) && wifi_mode_get_from_case > 1){ //wifi dongle plug in
		trace("****** wifi dongle plug in******");
		if(store_dongle_state(true)){  //check wifi dongle plug-in
			TopBarSWF.deviceIconRefresh();
		}
	}
	
	switch(wifi_mode_get_from_case){
		case WifiEngine.WIFI_DONGLE_PLUG_OUT:
			 start_concurrent_softap_mode=0;
			 start_concurren_clientap_mode=0;
			 show_softap_flag=0;
			 scan_clentap_flag=0;
			 if(store_dongle_state(false)) //check wifi dongle plug-out
				TopBarSWF.deviceIconRefresh();
				
			if(pre_wifi_mode_get_from_case==WifiEngine.WIFI_DONGLE_PLUG_OUT||pre_wifi_mode_get_from_case==WifiEngine.WIFI_DISABLE_MANUAL)
				break;
		         
			WifiEngine.Stop();
			WifiEngine.Close();
			WifiEngine.Wifidonglechange(WifiEngine.CLOSE_WIFI_PROCESS);
					
			topBarShowCompleted = false;
			show_softap_flag = 0;
			TopBarSWF.iconConnectNumber(0);
			TopBarSWF.showSSID("WAIT...");
			TopBarSWF.showPSK(" ");
			if(STREAM_SWF == currentSWF){
				currentMC.wifiDisplayStop();
				currentMC.start();
			}
			break;
		
		default:
		
			break;
			
		
	}
 
	if((start_concurrent_softap_mode==1)&&(show_softap_flag==0)){
		to_show_softap_info();
		show_softap_flag=1;
		//softapIgnore();
		var curDongle:Number = WifiEngine.getWifiModelType();
		trace("curDongle: "+curDongle+", WifiEngine.WIFI_REALTEK_8811AU: "+WifiEngine.WIFI_REALTEK_8811AU+", WifiEngine.WIFI_REALTEK_8821CU: "+WifiEngine.WIFI_REALTEK_8821CU);
		if(curDongle == WifiEngine.WIFI_REALTEK_8811AU || curDongle == WifiEngine.WIFI_REALTEK_8821CU)
		{
			TopBarSWF.mainIconChangeTo(3);
		}
		else
		{
			TopBarSWF.mainIconChangeTo(2);
		}
	  }
     open_and_scan_clientap();
	 pre_wifi_mode_get_from_case=wifi_mode_get_from_case;
	// trace(" 810 pre_wifi_mode_get_from_case = "+pre_wifi_mode_get_from_case)
/********************************enable & disable LAN*********************************/
	if(EZCASTPRO_LAN_ENABLE&&ezcastpro_lan_plugin!=SystemInfo.getnetlinkstatus()){//richardyu 112514
		//only work with first plug-in
		trace("LAN plug-in status change!! old ezcastpro_lan_plugin=="+ezcastpro_lan_plugin);
		switch(ezcastpro_lan_plugin){
			case 0://0->1, enable
				//check DHCP or manual config
				ezcastpro_lan_working=-1;//richardyu 052814, the value not stable yet
				var ret:Number;
				var tmpstr:String=LanEngine.EZCastProAPI_net_config_reload(1);//LAN
				trace("LAN mode=="+tmpstr.slice(1,2)+"tmpstr.length=="+tmpstr.length);			
				if(tmpstr.slice(1,2)!="M"){//DHCP
					ret=LanEngine.Open();
					if(ret!=0){
						trace("DHCP fail!!");
						ezcastpro_lan_working=0;
					}else{
						trace("DHCP good!!");					
						ezcastpro_lan_working=1;
					}
				}else{//manual config reload
					var tmparr:Array;	
					tmpstr=tmpstr.slice(1,tmpstr.length-1);//take off #
					trace("tmpstr=="+tmpstr);
					tmparr=tmpstr.split(":");
					var ret:Number=LanEngine.lan_openmanual(tmparr[1],tmparr[2],tmparr[3],tmparr[4],tmparr[5]);
					if(ret!=0){
						trace("openmanual fail!!");
						ezcastpro_lan_working=0;					
/*						trace("openmanual fail, do DHCP!!");
						ret=LanEngine.Open();					
						if(ret!=0){
							trace("DHCP fail!!");
							ezcastpro_lan_working=0;
						}else{
							ezcastpro_lan_working=1;
						}
*/
					}else{
						trace("openmanual successfully!!");					
						ezcastpro_lan_working=2;						
					}
				}
				ezcastpro_lan_plugin=1;
//				revalue_ezcastpro_string_arr(INDEX_STRING_LAN_IP);
				break;
			case 1://1->0, disable
				LanEngine.Close();		
				LanEngine.Stop();
				ezcastpro_lan_working=0;
				ezcastpro_lan_plugin=0;
				change_lan_state(0);
				if(_global.show_stroage_icon_flag)
					TopBarSWF.deviceIconRefresh();
				break;
		}	
	}
/*****************************************************************/	 
	 
	 
	net_top_bar_setting();
	set_dongle_timer=setInterval(check_the_netstatus,500);	 
	 
}

/************************* LAN FUNCTION **************************/

var toLanConnectCheck:Boolean = false;
var ipForLan:String = "";

function isLanConnect():Boolean
{
	if(EZWILAN_ENABLE && lanstatus_priv != 0 && ipForLan.length >= 7 && ipForLan.indexOf(".", 2) >= 0)
		return true;
	else
		return false;
}

function isLanPortIn():Boolean
{
	if(EZWILAN_ENABLE && lanstatus_priv != 0)
		return true;
	else
		return false;
}

function lan_open_fun(){
	if(lanopenInterval > 0){
		clearInterval(lanopenInterval);
		lanopenInterval = NULL;
	}
	if((LAN_ONLY || start_concurrent_softap_mode||(EZCASTPRO_MODE==8075&&currentSWF==EZMIRR_SWF)) && SystemInfo.getnetlinkstatus()){
		if(LanEngine.getLanAutomaticEnable() == 0){
			var ip:String = LanEngine.getLanSettingVal(LanEngine.LANSETTING_IP);
			var gateway:String = LanEngine.getLanSettingVal(LanEngine.LANSETTING_GATEWAY);
			var mask:String = LanEngine.getLanSettingVal(LanEngine.LANSETTING_MASK);
			var dns1:String = LanEngine.getLanSettingVal(LanEngine.LANSETTING_DNS1);
			var dns2:String = LanEngine.getLanSettingVal(LanEngine.LANSETTING_DNS2);
			LanEngine.lan_openmanual(ip, gateway, mask, dns1, dns2);
		}else{
			LanEngine.Open();
		}
		toLanConnectCheck = true;
		if(currentSWF == SETTING_SWF){
			currentMC.lanPortCheck();
		}
		if(lancheckInterval > 0){
			clearInterval(lancheckInterval);
			lancheckInterval = NULL;
		}
		lancheckInterval = setInterval(lan_check, 3000);

	}else{
		trace("^^^^^^[lan_open_fun]^^^^^^ LAN port is plug in, but wifi is not start, wait!!! ^^^^^^^^^^^^^");
		lanstatus_priv = 0;
		if(lancheckInterval > 0){
			clearInterval(lancheckInterval);
			lancheckInterval = NULL;
		}
		lancheckInterval = setInterval(lan_check, 2000);
	}
}

function lan_check(){
	var lan_stat:Number = 0;
	var ap_select;
	
	lan_stat = SystemInfo.getnetlinkstatus();
	if(lan_stat != lanstatus_priv){
		//trace("^^^^^^^^^^^^^^^^^^^^^^^^  lan_stat = "+lan_stat);
		lanstatus_priv = lan_stat; 
		if(lan_stat == 0){
			if(EZCAST_SWF == currentSWF&&(isLanMiracodeShowing==true||dongleLanIpShowing==true))
			{
				isLanMiracodeShowing=false;
				currentMC.clean_miracode();
				dongleLanIpShowing=false;
				currentMC.clean_dongle_ip();
				trace("^^^^^^^^^^^^^^^^^cleacn miracode^^^^^^^^^^^");
			}
			
			SystemInfo.setLEDGreenOff();
			trace("^^^^^^^^^^^^^^^^^ LED Blue OFF ^^^^^^^^^^^[lan check]");
			LanEngine.Close();
			if(currentSWF == EZMIRR_PROBOX_SWF)
			{
				currentMC.clean_route_ip();
				currentMC.clean_mira_code();
				return ;
			}
			TopBarSWF.iconSignalNull();
			toLanConnectCheck = false;
			ipForLan = "";
			if(currentSWF == SETTING_SWF){
				currentMC.lanPortCheck();
				currentMC.ipRefresh();
				currentMC.refillMacAddr();
			}
			if(lancheckInterval > 0){
				clearInterval(lancheckInterval);
				lancheckInterval = NULL;
			}
			lancheckInterval = setInterval(lan_check, 2000);
			
			if(LAN_ONLY){
				showLanOnlyInfo();
				topBarShowCompleted = false;
				if(currentSWF == EZCAST_SWF && currentMC.wifi_display_on){
					trace("======== No Wi-Fi and Lan stop =========");
					currentMC.wifiDisplayStop();
					currentMC.bgMC.init();
					currentMC.start();
				}
			}else{
				WifiEngine.setAutoConnect(1);
				WifiEngine.AutoConnect();
				if(!toShowSoftap && !TopBarSWF.isCurrectStrSsid()){
					toShowSoftap = true;
					if(softapInfoShowTimer >= 0){
						clearInterval(softapInfoShowTimer);
						softapInfoShowTimer = -1;
					}
					if(WifiEngine.isLastRouterApExist() == 0){
						trace("LAN: to show softap after 3 sec");
						softapInfoShowTimer = setInterval(toShowSoftapInfo, 3000);
					}else{
						trace("LAN: to show softap after 8 sec");
						softapInfoShowTimer = setInterval(toShowSoftapInfo, 8000);
					}
				}
			}
		}else{
			if(EZCASTPRO_MODE==8075&&currentSWF==EZMIRR_SWF)
			{
				if(lancheckInterval > 0){
					clearInterval(lancheckInterval);
					lancheckInterval = NULL;
				}
				lanopenInterval = setInterval(lan_open_fun, 1500);
				return;
			}
			else if(!start_concurrent_softap_mode && !LAN_ONLY){
				trace("^^^^^^^^^^^^ LAN port is plug in, but wifi is not start, wait!!! ^^^^^^^^^^^^^");
				lanstatus_priv = 0;
				return;
			}
			if(lancheckInterval > 0){
				clearInterval(lancheckInterval);
				lancheckInterval = NULL;
			}

			if(!LAN_ONLY){
				WifiEngine.setAutoConnect(0);
			}
			//LanEngine.Open();
			lanopenInterval = setInterval(lan_open_fun, 1500);
		}
	}
	if(lanstatus_priv != 0 && toLanConnectCheck){
		ipForLan = lanIpForDisplay();
		trace("----------------------------  ipForLan: "+ipForLan);
		if(ipForLan != "" && ipForLan.indexOf(".", 2) >= 0){
			if(LAN_ONLY){
				showLanOnlyInfo();
				topBarShowCompleted = true;
			}
			toLanConnectCheck = false;
			SystemInfo.setLEDGreenOn();
			if(currentSWF == EZMIRR_PROBOX_SWF)
			{
				currentMC.show_route_ip();
				currentMC.show_mira_code(ipForLan);
				
				return ;
			}
			if(!LAN_ONLY){
				if(WifiEngine.GetIndexofSelectAP() >= 0)
					WifiEngine.wifi_disconnect();
				if(isWaitConnect)
					showSoftapInfo();
			}
			TopBarSWF.iconLanOn();
			trace("^^^^^^^^^^^^^^^^^ LED Blue ON ^^^^^^^^^^^[lan check]");
			TopBarSWF.iconPopClean();
			trace("IP get success["+ipForLan+"]!!");
			if(currentSWF == SETTING_SWF){
				currentMC.ipRefresh();
				currentMC.refillMacAddr();
			}
			if(!fw_update_main_flag){
				if(isFirstOTACheck || (currentSWF != STREAM_SWF) || (EZCAST_STATUS_NULL == SystemInfo.wifidisplay_gets_tatus())){
					fw_update_main_flag = true;
					isFirstOTACheck = false;
					//trace("---fw_update_main_flag="+fw_update_main_flag);
					if(tocheckotaversion_inter > 0){
						clearInterval(tocheckotaversion_inter);
						tocheckotaversion_inter = NULL;
					}
					tocheckotaversion_inter = setInterval(tocheckotaversion, 3000);
				}
			}
			//if(TopBarSWF.isCurrectStrSsid())
			if(get_ConnectMode() == SystemInfo.ROUTER_ONLY){
				showRouterInfoForP2pClient();
			}
		}
	}
	if(isEZCastpro()&&lanstatus_priv!=0)
	{
		if(SystemInfo.getConnectMode()!=0&&isLanMiracodeShowing==false&&dongleLanIpShowing==false&&EZCAST_SWF == currentSWF&&currentMC.wifi_display_on)
		{
			ipForLan = lanIpForDisplay();
			if(ipForLan != "" && ipForLan.indexOf(".", 2) >= 0)
			{
				
				if(SystemInfo.get_miracode_enable())
				{
					if(show_clientap_flag)
			  	 		currentMC.clean_miracode();
					currentMC.show_miracode(ipForLan);
					isLanMiracodeShowing=true;
				}
				currentMC.show_dongle_ip(ipForLan);
				dongleLanIpShowing=true;
				
			}
		
		}
	    else if(isLanMiracodeShowing||dongleLanIpShowing)
		{

			if(SystemInfo.getConnectMode()==0&&EZCAST_SWF == currentSWF)
			{
				currentMC.clean_miracode();
				isLanMiracodeShowing=false;
				currentMC.clean_dongle_ip();
				dongleLanIpShowing=false;
			}
			else if(EZCAST_SWF != currentSWF)
			{
				isLanMiracodeShowing=false;
				dongleLanIpShowing=false;
			}

		
		}
	}
	
}

/****** set timer of checking the net(wifi) status **********/
function init_main_net_status(){
	if(set_dongle_timer > 0){
		clearInterval(set_dongle_timer);
		set_dongle_timer = -1;
	}
	var wait_period:Number=1000;
	if(EZCASTPRO_MODE>0&&EZCASTPRO_MODE!=8251){//not EZCastPro dongle
		wait_period=9000;
	}
	set_dongle_timer=setInterval(check_the_netstatus,wait_period);
}

function main_net_status_stop(){
	if(set_dongle_timer > 0){
		clearInterval(set_dongle_timer);
		set_dongle_timer = -1;
	}
}

/*****************************     file  end       ***************************************************/
