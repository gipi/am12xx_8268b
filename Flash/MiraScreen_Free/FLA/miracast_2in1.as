import Actions.WidiEngine;
import Actions.Ime;
import Actions.Locale;
import Actions.MiracastEngine;
import Actions.WifiEngine;
import Actions.SystemInfo;
import Actions.PhotoEngine;
import Actions.CSwfKey;

var SHOW_IP:Boolean = MainSWF.showIpInMiracastLite;

var wifi_direct_enbale_kernel_timer_id:Number = -1;
var wifi_direct_check_connection_request_timer_id:Number = -1;
var defaultVolume:Number ;
var scan_ap_flag:Number = -1;
var scanCount:Number = 0;
var wifi_display_on:Boolean = false;
var wifiInterval:Number = -1;
var initInterval:Number = -1;
var showIpInterval:Number = -1;
var STARTDISPLAY:Boolean = true;
var status_string:String = "";
var ip_addr:String;
var ip_x:Number = 1310;
var ip_y:Number = 588;
var ip_w:Number = 400;
var ip_h:Number = 49;

var isMiracastInit:Boolean = false;
var isLastApExist = -1; //WifiEngine.isLastRouterApExist();
var autoConnectInterval:Number = -1;
var connectCheckCount:Number = 0;
var dongleSSID:String = "null";
var ip_port:Number = -1;
var isInMiracast:Boolean = false;
var toRestartMiracast:Boolean = false;
var scanInterval:Number = -1;
var checkScanAndConnect:Number = -1;
var devName:String = "";
var pinCode:String = "";
var checkGetPeerIpInterval:Number = -1;
var checkGetPeerIpStartTime:Number = 0;
var negoInterval:Number = -1;
var negoHnadleStartTime:Number = 0;

MainSWF.backMenu=false;		//if not ,the screen won't be back to the last interface.
step2_mc.mirroring_mc._visible = false;

/****************************/
/*
*p2p+concurrent mode open or not
*/
var p2p_client_mode:Boolean =  false;						//enable p2p+client or not
var p2p_softap_mode:Boolean = true;						//enable p2p+softap or not
var wifi_direct_concurrent_client_status_id:Number = -1;
var wifi_inactive_count:Number = 0;
var p2p_client_order_check:Boolean = p2p_client_mode;		//used for set the flag that check the client reseted before p2p restart or not 

function ezcastProUI(){
	trace("EZCASTPRO_MODE=="+MainSWF.EZCASTPRO_MODE);
	if(MainSWF.EZCASTPRO_PROJECTOR){
		step1_mc.app_search_txt.text="EZCastPro";
		step1_mc.qr_mc.gotoAndStop(2);
		step2_mc.dongle_mc.gotoAndStop(3);
		step2_mc.dongle2_mc.gotoAndStop(3);
	}
	else if(MainSWF.EZCASTPRO_MODE){
	//if(1){
		step1_mc.app_search_txt.text="EZCastPro";
		step1_mc.qr_mc.gotoAndStop(2);
		if(MainSWF.EZCASTPRO_MODE == 8075 || MainSWF.EZWILAN_ENABLE){
			step2_mc.dongle_mc.gotoAndStop(4);
			step2_mc.dongle2_mc.gotoAndStop(4);
		}else{
			step2_mc.dongle_mc.gotoAndStop(2);
			step2_mc.dongle2_mc.gotoAndStop(2);
		}
	}else{
		step1_mc.app_search_txt.text="EZCast";
		step1_mc.qr_mc.gotoAndStop(1);
		if(MainSWF.EZWILAN_ENABLE){
			step2_mc.dongle_mc.gotoAndStop(4);
			step2_mc.dongle2_mc.gotoAndStop(4);
		}else{
			step2_mc.dongle_mc.gotoAndStop(1);
			step2_mc.dongle2_mc.gotoAndStop(1);	
		}
	}
}

function __fontsize_adjust(){
	var sys_w:Number=SystemInfo.getCurscreenParam(1);
	var sys_h:Number=SystemInfo.getCurscreenParam(2);
	var txt_format:TextFormat=new TextFormat();
	var adjust_ratio:Number=0.8;
	if(sys_w<1920&&sys_h==768){
		txt_format=step2_mc.dongle_ssid_txt.getTextFormat();
		txt_format.size=Math.round(txt_format.size*adjust_ratio);
		trace("txt_format.size"+txt_format.size);
		switch(step2_mc._currentframe){
			case 2:
				step2_mc.dongle_devname2_txt.setTextFormat(txt_format);
				step2_mc.router_2_ssid_txt.setTextFormat(txt_format);
				step2_mc.dongle_2_ssid_txt.setTextFormat(txt_format);
			case 1:
				step2_mc.dongle_ssid_txt.setTextFormat(txt_format);
				step2_mc.dongle_devname_txt.setTextFormat(txt_format);
				step2_mc.router_ssid_txt.setTextFormat(txt_format);
				break;
		}
	}
}

function setDefaultVolume(){
	var curVolume = SystemInfo.sys_volume_ctrl_get();
	if(curVolume != MainSWF.systemVolume){
		SystemInfo.setVolume(MainSWF.systemVolume);
	}
}

function show_ip_delay(){
	if(showIpInterval!= -1){
		clearInterval(showIpInterval);
		showIpInterval = -1;
	}
	var show_str:String = "http://"+ip_addr;
	trace("show_str: "+show_str+", ip_x: "+ip_x+", ip_y: "+ip_y);
	//PhotoEngine.show_text(show_str, 27, 0xD9D9D9, ip_x, ip_y);
}

function show_ip(current_mode:Number){
	var port:Number = -1;
//trace("__show ip, MainSWF.isNetDisplayOn: "+MainSWF.isNetDisplayOn+", MainSWF.isStreamOn: "+MainSWF.isStreamOn);	
	if(MainSWF.isNetDisplayOn && !MainSWF.isStreamOn){
		if(current_mode == WidiEngine.MIXED_STATUS_CLIENT_P2P){
			port = 0;
		}else if(current_mode == WidiEngine.MIXED_STATUS_P2P_SOFTAP){
			port = 1;
		}else{
			return;
		}
		//trace("port: "+port+", ip_port: "+ip_port);
		if(port != ip_port){
			trace("x: "+ip_x+", y: "+ip_y+", w: "+ip_w+", h: "+ip_h+"");
			PhotoEngine.clean_icon_rect(ip_x, ip_y, ip_w, ip_h);
			ip_port = port;
			ip_addr = SystemInfo.getIP(ip_port);
			trace("ip_addr: "+ip_addr);
			if(ip_addr != "error"){
				if(showIpInterval!= -1){
					clearInterval(showIpInterval);
					showIpInterval = -1;
				}
				showIpInterval = setInterval(show_ip_delay, 200);
			}
		}
	}else{
		ip_port = -1;
	}
}

function reShowIP(){
	//trace("reShowIP");
	return;
	ip_port = -1;
	var current_mode = WidiEngine.GetMode();
	show_ip(current_mode);
}

function clear_wifi_direct_concurrent_client_status_id(){
	wifi_inactive_count = 0;
	if(wifi_direct_concurrent_client_status_id!= -1){
		clearInterval(wifi_direct_concurrent_client_status_id);
		wifi_direct_concurrent_client_status_id = -1;
	}
}
/*check the client status if the p2p disconnected*/
function wifi_direct_concurrent_client_status(){
	if(MainSWF.wifi_mode_get_from_case==WifiEngine.WIFI_AUTO_IP_SUCCESS || MainSWF.wifi_mode_get_from_case==WifiEngine.WIFI_AUTO_IP_ERROR || MainSWF.wifi_mode_get_from_case == WifiEngine.WIFI_CONNECT_FAILED || wifi_inactive_count>=15){
		trace("restart P2P......");
		clear_wifi_direct_concurrent_client_status_id();
		WidiEngine.ReStart();
		wifi_direct_check_connection_request_timer_id = setInterval(wifi_direct_check_connection_request,200);
		//status_show("Wait for connection...");
		//status_show(" ");
		mirringShowOff();
		WifiEngine.WifiDisableRouting();			//disalbe the routing if the client connect successfully
	}
	else if(MainSWF.wifi_mode_get_from_case == WifiEngine.WIFI_INACTIVE){
		wifi_inactive_count++;
	}
}
/****************************/

/**
* for Miracast, check the RTSP status
*/
var rtsp_status_check_timer_id:Number=-1;
function clear_rtsp_status_check_timer_id(){
	if(rtsp_status_check_timer_id != -1){
		clearInterval(rtsp_status_check_timer_id);
		rtsp_status_check_timer_id = -1;
	}	
}

function autoConnectCheck4Restart(){
	if(!MainSWF.topBarShowCompleted){
		return;
	}
	trace("Restart miracast and check auto connect!!!");
	if(isLastApExist < 0){
		isLastApExist = WifiEngine.isLastRouterApExist();
		trace("__________isLastApExist = "+isLastApExist);
	}
	
	if(isLastApExist < 0){
		isLastApExist = 0;
	}
	// Check client connect, and when connect to a router, then start client+p2p mode.
	var wifi_select=WifiEngine.GetIndexofSelectAP();
	trace(":::restart::: wifi_select: "+wifi_select);
	if(wifi_select > -1){   //MainSWF.wifi_mode_get_from_case == WifiEngine.WIFI_CONCURRENT_CLIENT_GETAUTOIP_OK){
		var ip = SystemInfo.getIP(0);
		trace(":::: restart::: IP: "+ip);
		if(ip != "error"){
			if(autoConnectInterval > 0){
				clearInterval(autoConnectInterval);
				autoConnectInterval = -1;
			}
			//initMiracast();
			isLastApExist = 0;
			toRestartMiracast = false;
			var current_mode:Number = WidiEngine.GetMode();
			if(current_mode != WidiEngine.MIXED_MODE_CLIENT_P2P && wifi_display_on)
				wifiDisplayStop();
			initClientP2P();
			return;
		}
	}

	// If the system is start and enter first swf, then will wait auto connect. (max 10s)
	trace("isLastApExist = "+isLastApExist);
	if(isLastApExist){
		trace("connectCheckCount = "+connectCheckCount);
		if(connectCheckCount<13){
			connectCheckCount++;
			return;
		}
	}

	// If client is not connected and could not auto connect to router, then will start p2p+softap mode.
	if(autoConnectInterval > 0){
		clearInterval(autoConnectInterval);
		autoConnectInterval = -1;
	}
	isLastApExist = 0;
	toRestartMiracast = false;
	//trace("---------------------1---------------------initP2P >>>>>>>>>>>>>>>>");
	//trace("Miracast device name is "+MainSWF.devName);
	//WidiEngine.SetLocalDevName(MainSWF.devName);
	var current_mode:Number = WidiEngine.GetMode();
	if(current_mode != WidiEngine.MIXED_MODE_P2P_SOFTAP && wifi_display_on)
		wifiDisplayStop();
	initP2PSoftap();
}

function miracast_restart()
{
	/** detect stop status */
	clear_rtsp_status_check_timer_id();
	MainSWF.statusMiracastStop();
		
	/** call the RtspStop() to stop RTSP service. */
	
	var current_mode = MainSWF.current_lite_mode = WidiEngine.GetMode();
	trace("current_mode: "+current_mode+", WidiEngine.MIXED_STATUS_P2P_WLAN1: "+WidiEngine.MIXED_STATUS_P2P_WLAN1+", WidiEngine.MIXED_STATUS_CLIENT_P2P: "+WidiEngine.MIXED_STATUS_CLIENT_P2P);
	if(current_mode == WidiEngine.MIXED_STATUS_P2P_WLAN1 || current_mode == MIXED_STATUS_CLIENT_P2P){
		isLastApExist = 1;
	}
	MiracastEngine.RtspStop();
	MainSWF.statusMiracastStop();
	isMiracastInit = false;
		
	if(MainSWF.current_lite_mode != WidiEngine.MIXED_STATUS_CLIENT_SOFTAP){
		WidiEngine.ModeChange(WidiEngine.MIXED_MODE_CLIENT_SOFTAP);
		MainSWF.showSoftapInfo();		
	}
	
	connectCheckCount = 0;
	if(autoConnectInterval > 0){
		clearInterval(autoConnectInterval);
		autoConnectInterval = -1;
	}
	autoConnectInterval = setInterval(autoConnectCheck4Restart, 2000);
}

function rtsp_status_check(){
	var rtsp_status:Number;
	var p2p_status:Number;

	rtsp_status = MiracastEngine.RtspStatus();
	if(rtsp_status==MiracastEngine.RTSP_STAT_STOP){
		miracast_restart();
	}
	
	else if(rtsp_status==MiracastEngine.RTSP_STAT_PLAYING){
		
		p2p_status = WidiEngine.GetConnectionStatus();
		if(p2p_status != 0){
			trace("p2p disconnected when do miracast!!");
			MiracastEngine.DropMira();
			miracast_restart();
		}
		
	}
	
}

function checkGetPeerIpAddrResult():Number{
	var ret_1:Number = WidiEngine.GetPeerIpAddrResult();
	if(ret_1 < 0){
		var curTime:Number = SystemInfo.time();
		if(curTime - checkGetPeerIpStartTime > 55){
			trace(">>>>>> RTSP ip address timeout");
			if(checkGetPeerIpInterval > 0)
			{
				clearInterval(checkGetPeerIpInterval);
				checkGetPeerIpInterval = -1;
			}
			wifi_direct_handle_nego_out();
		}
	}else if(ret_1 == 0)
	{
		trace(">>>>>> will start RTSP");
		if(checkGetPeerIpInterval > 0)
		{
			clearInterval(checkGetPeerIpInterval);
			checkGetPeerIpInterval = -1;
		}
		clear_wifi_direct_enbale_kernel_timer_id();
		clear_wifi_direct_check_connection_request_timer_id();
		SystemInfo.setVolume(14);
		MiracastEngine.RtspStart();
		//status_show("Do RTSP Connection...");
		mirringShowOn();
		/** start a timer to listen the rtsp status*/
		clear_rtsp_status_check_timer_id();
		rtsp_status_check_timer_id = setInterval(rtsp_status_check,1000);
		return ret_1;
	}
	else
	{
		trace(">>>>>> RTSP ip address error");
		if(checkGetPeerIpInterval > 0)
		{
			clearInterval(checkGetPeerIpInterval);
			checkGetPeerIpInterval = -1;
		}
		wifi_direct_handle_nego_out();
	}
	return ret_1;
}

function wifi_direct_handle_nego_out(){
	trace("__________ nego_result: "+nego_result);
	clear_wifi_direct_enbale_kernel_timer_id();
	clear_wifi_direct_check_connection_request_timer_id();
	checkGetPeerIpStartTime = SystemInfo.time();
	miracast_restart();
}

function wifi_direct_handle_nego(nego_result:Number)
{
	var ret_1:Number;
	
	if(nego_result==0){
		
		/** Miracast */
		/*
		WidiEngine.GetPeerRTSPPort();
		ret_1 = WidiEngine.GetPeerIpAddr();
		if(ret_1 == 0){
			trace(">>>>>> will check get peer IP status");
			clear_wifi_direct_enbale_kernel_timer_id();
			clear_wifi_direct_check_connection_request_timer_id();
			checkGetPeerIpStartTime = SystemInfo.time();
			if(checkGetPeerIpInterval > 0)
			{
				clearInterval(checkGetPeerIpInterval);
				checkGetPeerIpInterval = -1;
			}
			checkGetPeerIpInterval = setInterval(checkGetPeerIpAddrResult, 1000);
			return;
		}
		else{
			trace(">>>>>> RTSP ip address error");
		}
		*/
		trace(">>>>>> will check get peer IP status");
		clear_wifi_direct_enbale_kernel_timer_id();
		clear_wifi_direct_check_connection_request_timer_id();
		checkGetPeerIpStartTime = SystemInfo.time();
		var retV:Number = checkGetPeerIpAddrResult();
		if(checkGetPeerIpInterval > 0)
		{
			clearInterval(checkGetPeerIpInterval);
			checkGetPeerIpInterval = -1;
		}
		if(retV != 0)
			checkGetPeerIpInterval = setInterval(checkGetPeerIpAddrResult, 200);
		return;
	}
	wifi_direct_handle_nego_out();
}

var connect_pin_flag:Number = WidiEngine.WIDI_PINCODE_NONE;

function negoHandle(){
	var nego_ok = WidiEngine.GetDoNegoResult();
	if(nego_ok < 0){
		var cur_time:Number = SystemInfo.time();
		if(cur_time - negoHnadleStartTime > 55){
			trace("get DoNego result timeout!!!");
			if(negoInterval > 0){
				clearInterval(negoInterval);
				negoInterval = -1;
			}
			negoHnadleStartTime = SystemInfo.time();
			wifi_direct_handle_nego(1);
		}
	}else{
		if(negoInterval > 0){
			clearInterval(negoInterval);
			negoInterval = -1;
		}
		wifi_direct_handle_nego(nego_ok);
	}
}

function toNegoHandle(val:Number){
	if(val == 0){
		if(negoInterval > 0){
			clearInterval(negoInterval);
			negoInterval = -1;
		}
		negoHnadleStartTime = SystemInfo.time();
		negoInterval = setInterval(negoHandle, 1000);
		connect_pin_flag = WidiEngine.WIDI_PINCODE_NONE;
		MainSWF.statusMiracastStart();
	}
}

function wifi_direct_check_connection_request(){
	//var pin_conde_flag:Number = -1;
	var pin_code:String;
	var nego_ok:Number = 0;
//trace("__________  connect_pin_flag: "+connect_pin_flag);	
	//pin_conde_flag = WidiEngine.GetPinCodeFlag();
	if(connect_pin_flag == WidiEngine.WIDI_PINCODE_PBC){
		trace(">>>>>> PBC:");
		//nego_ok = WidiEngine.DoNegotiation(WidiEngine.WIDI_PINCODE_PBC);
		toNegoHandle(0);
	}
	else if(connect_pin_flag == WidiEngine.WIDI_PINCODE_INPUT){
		/** not support for Miracast */
		wifi_direct_handle_nego(1);
		connect_pin_flag = WidiEngine.WIDI_PINCODE_NONE;
	}
	else if(connect_pin_flag == WidiEngine.WIDI_PINCODE_DISPLAY){
		/** default will show the PIN */
		//WidiEngine.GeneratePincode();
		//wifi_pincode_display_content = WidiEngine.GetPincode();
		nego_ok = WidiEngine.DoNegotiation(WidiEngine.WIDI_PINCODE_DISPLAY);
		toNegoHandle(nego_ok);
	}
	else if(connect_pin_flag == WidiEngine.WIDI_PINCODE_INVITED){
		/** invited to join a group */
		trace(">>>>>>> invited to join a group");
		nego_ok = WidiEngine.DoJoinGroup();
		wifi_direct_handle_nego(nego_ok);
		connect_pin_flag = WidiEngine.WIDI_PINCODE_NONE;
	}
	else if(connect_pin_flag == WidiEngine.WIDI_PINCODE_PERSISTENT_GP_INVITED){
		/** invited to join a group */
		trace(">>>>>>> invited to join a persistent group and role is client");
		//WidiEngine.DoJoinPersistentGroup();
		wifi_direct_handle_nego(0);
		connect_pin_flag = WidiEngine.WIDI_PINCODE_NONE;
	}
	else{
		if(p2p_client_order_check && MainSWF.start_concurren_clientap_mode == 1 && MainSWF.wifi_mode_get_from_case!=WifiEngine.WIFI_AUTO_IP_SUCCESS && MainSWF.wifi_mode_get_from_case!=WifiEngine.WIFI_AUTO_IP_ERROR ){
			p2p_client_order_check=false;			//all the conditions were set to check if the client was disconnected ,stop the client until the next term
			clear_wifi_direct_check_connection_request_timer_id();
			WifiEngine.Stop();				//if the client was disconnected and reset ,stop the client  and restart p2p
			WidiEngine.ReStart();
			wifi_direct_check_connection_request_timer_id = setInterval(wifi_direct_check_connection_request,200);
			//status_show("Wait for connection...");
			//status_show(" ");
			mirringShowOff();
		}
		connect_pin_flag = WidiEngine.GetPinCodeFlag();
		if(connect_pin_flag != WidiEngine.WIDI_PINCODE_NONE){
			//WidiEngine.CloseClientSoftap();			//useful in the p2p+concurrent mode when client need not always work
			WidiEngine.ModeChange(WidiEngine.MIXED_MODE_P2P);
			MainSWF.statusMiracastStart();
			isInMiracast = true;
			if(p2p_softap_mode && MainSWF.wifi_remote_control_mc._visible==true){
			//	WifiEngine.remotecontrol(1);
			//	MainSWF.wifi_remote_control_mc._visible=false;
			}
			if(wifiInterval>0){
				clearInterval(wifiInterval);
				wifiInterval = -1;
			}
			if(wifi_display_on){
				wifiDisplayStop();
			}
			//status_show("Do P2P Connection...");
			mirringShowOn();
			if(MainSWF.clientConnect)
				MainSWF.router_disconnect();
			trace("Do P2P Connection...");
		}
	}

}

function clear_wifi_direct_enbale_kernel_timer_id(){
	if(wifi_direct_enbale_kernel_timer_id != -1){
		clearInterval(wifi_direct_enbale_kernel_timer_id);
		wifi_direct_enbale_kernel_timer_id = -1;
	}	
}

function clear_wifi_direct_check_connection_request_timer_id(){
	if(wifi_direct_check_connection_request_timer_id != -1){
		clearInterval(wifi_direct_check_connection_request_timer_id);
		wifi_direct_check_connection_request_timer_id = -1;
	}	
}

function wifi_direct_enbale_kernel(){

	clear_wifi_direct_enbale_kernel_timer_id();
	
	/** start P2P */
	//WidiEngine.Start();
	
	/** check connection request */
	clear_wifi_direct_check_connection_request_timer_id();
	wifi_direct_check_connection_request_timer_id = setInterval(wifi_direct_check_connection_request,200);
}

function wifi_direct_enable(){
	clear_wifi_direct_enbale_kernel_timer_id();
	wifi_direct_enbale_kernel_timer_id = setInterval(wifi_direct_enbale_kernel,500);
}

var goto_ezmsg_timer_id:Number = -1; 

function show_ezmsg_func()
{
	if(goto_ezmsg_timer_id != -1)
	{
		clearInterval(goto_ezmsg_timer_id);
		goto_ezmsg_timer_id = -1;
	}	
	trace("-------------------------------show_ezmsg_func");	
	//MainSWF.ezShowPng("/usr/share/ezdata/ezcast_icon_setting.png",468,388,1000,126);	
	MainSWF.refreshIcon();
	init_mc._visible = true;
	init_mc.gotoAndStop(2);

}

function uninitMiracast()
{
	clear_rtsp_status_check_timer_id();
	clear_wifi_direct_enbale_kernel_timer_id();
	clear_wifi_direct_check_connection_request_timer_id();
		
	if(isMiracastInit){
		MiracastEngine.RtspDestroy();
		isMiracastInit = false;
	}
	MainSWF.statusMiracastStop();
}

var mira_key_object = new Object();
mira_key_object.onKeyDown = function() {
	var get_key_code:Number;
	get_key_code = Key.getCode();
	trace("----mira_get_key_code=0x"+get_key_code.toString(16));
	switch (get_key_code) {
		case 0x57:	
			//status_txt._visible=true;
			//status_txt.text= "Please wait...";
			//trace("----status_txt.text="+status_txt.text);	
			break;
		case 0xdf:	
			//TopBarSWF.iconApLink();
			TopBarSWF.ezApLinkDelay();
			TopBarSWF.ezPop(TopBarSWF.routerSsid);
			//trace("what is MainSWF.currentSWF  = " + MainSWF.currentSWF);
			//MainSWF.to_loadswf("miracast_lite.swf");
			break;
		case CSwfKey.SWF_MSG_KEY_Z:
   			MainSWF.router_disconnect();
   			TopBarSWF.ezPopClean();
   			TopBarSWF.ezshowip();
   			if(!TopBarSWF.softap_info._visible){
   					TopBarSWF.ezshowssid();
   			}
   			break;

			
		case CSwfKey.SWF_MSG_KEY_MENU:
			trace("----------> show msg...,WidiEngine.GetMode() ="+WidiEngine.GetMode());
			//if(WidiEngine.GetMode() == MIXED_STATUS_CLIENT_P2P){
			/*if(WidiEngine.GetMode() == WidiEngine.MIXED_STATUS_CLIENT_P2P){
                   trace("----------> This is Clent+P2P Mode...");
                   return;//Add by Denny
            }*/
			trace("----------> show msg...");
			//MainSWF.ezShowPng("/usr/share/ezdata/ezcast_icon_setting.png",468,388,1000,240);	
			MainSWF.webconnectionsnum = true;
			MainSWF.refreshIcon();
			if(!TopBarSWF.softap_info._visible){
   					TopBarSWF.ezshowssid();
   			}
			//wifiDisplayStop();	
			//goto_ezmsg_timer_id = setInterval(show_ezmsg_func,3000);		
			break;
		case CSwfKey.SWF_MSG_KEY_WIFI_CHANGE:
			var curMode:Number = WidiEngine.GetMode();
			 trace("----------> go to Websetting Mode...,WidiEngine.GetMode() ="+curMode);
			 if(curMode == WidiEngine.MIXED_STATUS_CLIENT_SOFTAP)
			 	break;
			
			if(MainSWF.isStreamOn)
			{
				wifiDisplayStop();
				MainSWF.netDisplayReady4TopBar();
				if(wifiInterval>0){
					clearInterval(wifiInterval);
					wifiInterval = -1;
				}
				wifiInterval=setInterval(wifiDisplayStart,1500);
			}
			uninitMiracast();
				
			if(MainSWF.current_lite_mode != WidiEngine.MIXED_STATUS_CLIENT_SOFTAP){
				WidiEngine.ModeChange(WidiEngine.MIXED_MODE_CLIENT_SOFTAP);
				MainSWF.showSoftapInfo();		
			}
			//WifiEngine.websettingConnect();
			
			if(MainSWF.isNetDisplayEnable() && !TopBarSWF.softap_info._visible){
				TopBarSWF.ezshowssid();
			}

			TopBarSWF.iconPopClean();
			MainSWF.webconnectionsnum = true;
			break;
		case CSwfKey.SWF_MSG_KEY_WIFI_FAILED:
			trace("----------> test go to Websetting Mode...,WidiEngine.GetMode() ="+WidiEngine.GetMode());
			if(WidiEngine.GetMode() == WidiEngine.MIXED_STATUS_CLIENT_SOFTAP){
				trace("-----------> This is Clent+P2P Mode...goto softap p2p");
				//initP2P(WidiEngine.MIXED_MODE_P2P_SOFTAP);
				trace("-----------> This softap_info._visible  "+TopBarSWF.softap_info._visible);
				TopBarSWF.iconPopClean();
				if(MainSWF.isNetDisplayEnable() && !TopBarSWF.softap_info._visible){
					TopBarSWF.ezshowssid();
				}
				//TopBarSWF.aplinkDisplay =  true;
				//TopBarSWF.isApLinkiconEnable =	true;
				//TopBarSWF.Key_Wifi_Faild = true;
				//initP2PSoftap();
				trace("-----------> This is TopBarSWF.softap_info._visible  "+TopBarSWF.softap_info._visible);
				
				//trace("----------> go to Websetting Mode...,MainSWF.isInWebMode ="+MainSWF.isInWebMode);
				//MainSWF.isInWebMode = false;
				return;//Add by Denny
			}
			break;

		case CSwfKey.SWF_MSG_KEY_RIGHT_PASSWORD_CONNECT:
				trace("-----------> SWF_MSG_KEY_RIGHT_PASSWORD_CONNECT");
				uninitMiracast();
					
				if(MainSWF.current_lite_mode != WidiEngine.MIXED_STATUS_CLIENT_SOFTAP){
					WidiEngine.ModeChange(WidiEngine.MIXED_MODE_CLIENT_SOFTAP);
					MainSWF.showSoftapInfo();		
				}
				WifiEngine.websettingConnect();
			break;
             		
		default:

			break;
	}
}

Key.addListener(mira_key_object);

/****************open miracast*************************/
function openMiracast(){
	if(MainSWF.start_concurrent_softap_mode == 1 || MainSWF.start_softap_mode == 1 ){
			
		close_softap_wifi();
		WifiEngine.WifiDisableRouting();
	}		
	trace("__________ MainSWF.start_concurren_clientap_mode = "+MainSWF.start_concurren_clientap_mode);
	if(MainSWF.start_concurren_clientap_mode == 1){
		MainSWF.client_ap_back_from_miracast = 1;			//keep the client ap back from miracast
		if(p2p_client_mode == false){
			trace("____________________________________________close client_______________________________________________");
			close_clientap_wifi();
		}
	}
	if(!p2p_softap_mode){
		if(MainSWF.wifi_remote_control_mc._visible==true){
			WifiEngine.remotecontrol(1);
			//MainSWF.wifi_remote_control_mc._visible=false;
		}
	}
}
/****************************************************/

/******************exit miracast***********************/
function exitMiracast()
{
	if(MainSWF.start_concurren_clientap_mode == 1)
	{		
		trace(">>back from miracast ,open softap......");
		WifiEngine.wifi_start_softap();		
		MainSWF.concurrent_softap_mode_from_miracast = 1;
		MainSWF.start_concurren_clientap_mode = 0;
	}	
	if(MainSWF.start_concurren_clientap_mode == 1)
	{
		/*necessary in the p2p+concurrent mode*/
		if(MainSWF.wifi_connect_status==WifiEngine.WIFI_AUTO_IP_SUCCESS || MainSWF.wifi_connect_status==WifiEngine.WIFI_AUTO_IP_ERROR){  
			trace(">>back from miracast , enable routing......");
			WifiEngine.WifiStartRouting();			//start the routing
			MainSWF.start_concurren_clientap_mode = 1;
		}
		else{
			trace(">>back from miracast ,open client......");
			open_clientap_wifi();
			MainSWF.client_ap_back_from_miracast = 0;
			MainSWF.start_concurren_clientap_mode = 0;
		}
	}
}
function scan_ap()
{	
	trace("---StartScan");
	WifiEngine.StartScan();
	scanCount ++;
	if(scanCount >= 2){
		scanCount = 0;
		clearInterval(scan_ap_flag);		
		scan_ap_flag = -1;
	}
}
function set_scan_ap()
{
	if(scan_ap_flag!=-1){
		clearInterval(scan_ap_flag);		
		scan_ap_flag = -1;
	}
	scan_ap_flag = setInterval(scan_ap,2000);			
}
function open_clientap_wifi()
{
	WifiEngine.Stop();
	WifiEngine.Open();
	WifiEngine.Start();
	set_scan_ap();
}
function close_softap_wifi(){
	if(MainSWF.start_concurrent_softap_mode == 0){
		return;
	}
	trace("********close the softap***********");
	MainSWF.start_concurrent_softap_mode = 0 ;
	SystemInfo.Putwifimode(WifiEngine.WIFI_DISABLE_MANUAL);
	
}

function close_clientap_wifi(){
	if(MainSWF.start_concurren_clientap_mode == 0){
		return;
	}	
	trace("*******close the client*************");
	clear_client_main_thread_interval();
	
	//MainSWF.start_concurren_clientap_mode = 0 ;
	WifiEngine.Stop();
	WifiEngine.Close();
	SystemInfo.Putwifimode(WifiEngine.WIFI_DISABLE_MANUAL);	
}
/*********************************************/

/**
*if the dongle plug out, unload the current swf
*/
var eventCardChange = function():Void{
	trace("dongle out ----->>> out of the moracast.swf");
	setDefaultVolume();

	uninitMiracast();
		
	WidiEngine.Stop();
	close_clientap_wifi();			//necessary in p2p+concurrent mode 
	clear_MainSWF_icon_flag();
	//_global.MainSWF.loadswf("mainmenu.swf");
	if(wifi_display_on == true){
		SystemInfo.StopNetDisplay();
		wifi_display_on = false;
	}
}

function clear_MainSWF_icon_flag(){
	//MainSWF.save_start_concurrent_softap_mode = 0;
	//MainSWF.save_start_softap_mode = 0;
	//MainSWF.save_start_clientap_mode = 0;
	//MainSWF.save_start_concurren_clientap_mode = 0;
}

function wifiDisplayStart()
{
	trace("wifi display ON");
	if(!MainSWF.topBarShowCompleted)
		return;
	if(wifiInterval>0){
		clearInterval(wifiInterval);
		wifiInterval = -1;
	}
	if(isInMiracast)
	{
		trace("It's in miracast, Can't start wifi display.");
		return;
	}
	trace("********************wifiDisplayStart****************");
	setDefaultVolume();
	if(wifi_display_on == false){
		MainSWF.wifi_display=SystemInfo.StartNetDisplay();
		wifi_display_on = true;
		MainSWF.netDisplayOn4TopBar();
		trace("wifiDisplayStart -- MainSWF.isNetDisplayOn: "+MainSWF.isNetDisplayOn);
	}
	
	MainSWF.setLoadSwfEnable();
}

function wifiDisplayStop(){
	if(wifiInterval>0){
		clearInterval(wifiInterval);
		wifiInterval = -1;
	}
	if(wifi_display_on == true){
		PhotoEngine.clean_icon();
		trace("_________wifiDisplayStop___________");
		SystemInfo.StopNetDisplay();
		wifi_display_on = false;
	}
	MainSWF.netDisplayOff4TopBar();
}

currentswf_out = function():Void
{
	trace("-----miracast off,isMiracastInit="+isMiracastInit);
	//setDefaultVolume();	
	wifiDisplayStop();

	uninitMiracast();
		
	if(MainSWF.current_lite_mode != WidiEngine.MIXED_STATUS_CLIENT_SOFTAP){
		WidiEngine.ModeChange(WidiEngine.MIXED_MODE_CLIENT_SOFTAP);
		MainSWF.showSoftapInfo();		
	}
	
	//MainSWF.statusMiracastStop();

}

function mirringShowOn(){
	step2_mc.mirroring_mc._visible = true;
}

function mirringShowOff(){
	step2_mc.mirroring_mc._visible = false;
}

function status_show(text:String){
	status_string = text;
	trace("________ status_string: "+status_string+", STARTDISPLAY: "+STARTDISPLAY+", wifi_display_on: "+wifi_display_on);
	if(STARTDISPLAY){
		MainSWF.show_miracast_status(status_string);
	}
	if(!wifi_display_on){
		status_mc.txt.text = status_string;
	}
}
var status_count:Number = 0;
function connection_status_check(){
	if(isInMiracast)
		return;
	
	var current_mode = MainSWF.current_lite_mode;
	//trace("connection_status_check: current_mode: "+current_mode+", WidiEngine.MIXED_STATUS_P2P_WLAN1: "+WidiEngine.MIXED_STATUS_P2P_WLAN1+", WidiEngine.MIXED_STATUS_CLIENT_P2P: "+WidiEngine.MIXED_STATUS_CLIENT_P2P);
	if(!toRestartMiracast && (current_mode == WidiEngine.MIXED_STATUS_P2P_WLAN1 || current_mode == WidiEngine.MIXED_STATUS_CLIENT_P2P)){
		var wifi_status=WifiEngine.getConnectionStatus();
		//trace("[miracast_lite] -- wifi_status: "+wifi_status);
		if(wifi_status != WifiEngine.WIFI_COMPLETED){
			trace("client wifi disconnection!!!");
			_global.MainSWF.ip_mc.set5.text = "http://192.168.203.1";
			/** detect stop status */
			if(MainSWF.clientConnect)
				MainSWF.router_disconnect();
			//TopBarSWF.setting_info.txt1.text = "http://192.168.203.1";
			/** call the RtspStop() to stop RTSP service. */
			isLastApExist = -1; //WifiEngine.isLastRouterApExist();;
			//WidiEngine.ModeChange(WidiEngine.MIXED_MODE_CLIENT_SOFTAP);
			connectCheckCount = 0;
			toRestartMiracast = true;

			uninitMiracast();
				
			if(MainSWF.current_lite_mode != WidiEngine.MIXED_STATUS_CLIENT_SOFTAP){
				WidiEngine.ModeChange(WidiEngine.MIXED_MODE_CLIENT_SOFTAP);
				MainSWF.showSoftapInfo();		
			}
			
			trace("Start auto connect.");
			WifiEngine.AutoConnect();

			if(autoConnectInterval > 0){
				clearInterval(autoConnectInterval);
				autoConnectInterval = -1;
			}
			autoConnectInterval = setInterval(autoConnectCheck4Restart, 2000);
		}
	}
}

function showIP(port:Number){
	trace("---show ip port: "+port);
	step2_mc.ip_mc.ip_txt.text = "http://"+SystemInfo.getIP(port);
	var show_str:String = "http://"+SystemInfo.getIP(port);
	trace("------show_str: "+show_str+", ip_x: "+ip_x+", ip_y: "+ip_y);
//	PhotoEngine.show_text(show_str, 20, 0xD9D9D9, ip_x, ip_y);
}

/**
* initial p2p
*/
function initP2P(port:Number){
	MainSWF.setLoadSwfDisable();
	
	trace("init P2P to port: "+port);
	// init P2P to WLAN0 or WLAN1
	trace("initP2P >>>>>>>>>>>>>>>>");
	trace("Miracast device name is "+MainSWF.devName);
	WidiEngine.SetLocalDevName(MainSWF.devName);
	WidiEngine.ModeChange(port);
	MainSWF.statusMiracastStop();
	isInMiracast = false;
	/** start the P2P */
	//trace("Miracast device name is "+MainSWF.devName);
	//WidiEngine.SetLocalDevName(MainSWF.devName);
	wifi_direct_enable();

	if(STARTDISPLAY){
		trace("____________________ port: "+port);
		if(port == 2){
			MainSWF.mainSoftapP2p();
			MainSWF.showSoftapInfo();
			step2_mc.gotoAndStop(1);
			UI_init();
			if(SHOW_IP){
				step2_mc.ip_connect_mc._visible = true;
				step2_mc.ip_mc._visible = true;
				showIP(1);
			}else{
				step2_mc.ip_connect_mc._visible = false;
				step2_mc.ip_mc._visible = false;
			}
		}else{
			MainSWF.mainP2pClient();
			MainSWF.showRouterInfoForP2pClient();
			step2_mc.gotoAndStop(2);
			UI_init();
			step2_mc.router_ssid_txt.text = WifiEngine.getconnectedssid();
			if(SHOW_IP){
				step2_mc.ip_connect_mc._visible = true;
				step2_mc.ip_mc._visible = true;
				showIP(0);
			}else{
				step2_mc.ip_connect_mc._visible = false;
				step2_mc.ip_mc._visible = false;
			}
		}
		MainSWF.netDisplayReady4TopBar();
		step2_mc.dongle_2_ssid_txt.text = devName;
		step2_mc.pincode_txt.text = "PIN: "+ pinCode;
		step2_mc.dongle_ssid_txt.text = dongleSSID;
		init_mc._visible = false;
		//TopBarSWF.icon5g();
		if(dongleSSID != MainSWF.devName){
			step2_mc.dongle_devname_txt.text = "(" + MainSWF.devName + ")";
		}else{
			step2_mc.dongle_devname_txt.text = "";
		}
		mirringShowOff();
		if(wifiInterval>0){
			clearInterval(wifiInterval);
			wifiInterval = -1;
		}
		wifiInterval=setInterval(wifiDisplayStart,300);
	}
	
}

/**
*initial the miracast
*/
function initMiracast(port:Number){
	/*get the default volume and set the miracast volume to 10*/	
	//defaultVolume = SystemInfo.getVolume();

	/** init the RTSP engine */
	if(port <= WidiEngine.MIXED_MODE_CLIENT_SOFTAP || port > WidiEngine.MIXED_MODE_P2P){
		trace("_____________  port error["+port+"]!!!");
		return;
	}

	if(!isMiracastInit)
	{
		trace("init port: "+port);
		MiracastEngine.RtspInit();
		isMiracastInit = true;
	}

	
	/** init P2P */
	initP2P(port);

	/**
	* Note: Default device name is the hostname. We can set it to a new one
	*       after call WidiEngine.Init() and before call WidiEngine.Start()
	*/
	devName = WidiEngine.GetWidiLocalDeviceName();
	pinCode = WidiEngine.GetPincode();
	step2_mc.dongle_2_ssid_txt.text = devName;
	step2_mc.pincode_txt.text = "PIN: "+ pinCode;
	step2_mc.dongle_ssid_txt.text = dongleSSID;
	init_mc._visible = false;
	//TopBarSWF.icon5g();
	if(dongleSSID != MainSWF.devName){
		step2_mc.dongle_devname_txt.text = "(" + MainSWF.devName + ")";
	}else{
		step2_mc.dongle_devname_txt.text = "";
	}

}
/*
function init(){
	if(!MainSWF.topBarShowCompleted)
		return;
	if(initInterval>0){
		clearInterval(initInterval);
		initInterval = -1;
	}
	initMiracast(); 		
}
//initInterval=setInterval(init,1000);
*/
function initClientP2P(){
	trace("init miracast to WLAN1!!!");
	if(initInterval>0){
		clearInterval(initInterval);
		initInterval = -1;
	}
	initMiracast(WidiEngine.MIXED_MODE_CLIENT_P2P);
}

function initP2PSoftap(){
	trace("init miracast to WLAN0!!!");
	initMiracast(WidiEngine.MIXED_MODE_P2P_SOFTAP);
}

function streamStart(){
	if(!isInMiracast){
		trace("----- stream start");
		uninitMiracast();
			
		WidiEngine.ModeChange(WidiEngine.MIXED_MODE_P2P_CLOSE);
	}
}

function streamStop(){
	trace("-----Stream stop");
	if(!isMiracastInit)
	{
		clear_wifi_direct_enbale_kernel_timer_id();
		clear_wifi_direct_check_connection_request_timer_id();
		if(MainSWF.current_lite_mode != WidiEngine.MIXED_STATUS_CLIENT_SOFTAP)
		{
			MiracastEngine.RtspInit();
			isMiracastInit = true;
			WidiEngine.SetLocalDevName(MainSWF.devName);
			WidiEngine.ModeChange(WidiEngine.MIXED_MODE_P2P_OPEN);
			wifi_direct_check_connection_request_timer_id = setInterval(wifi_direct_check_connection_request,200);
		}
	}
}

function autoConnectCheck(){
	if(!MainSWF.topBarShowCompleted){
		return;
	}
	
	trace("--------auto connect check");
	if(isLastApExist < 0)
		isLastApExist = WifiEngine.isLastRouterApExist();

	trace("__________isLastApExist = "+isLastApExist);

	if(isLastApExist <= 0){
		isLastApExist = 0;
		if(autoConnectInterval > 0){
			clearInterval(autoConnectInterval);
			autoConnectInterval = -1;
		}
		initP2PSoftap();
		return;
	}
	if(connectCheckCount % 3 == 0)
	{
		var wifi_status:Number = WifiEngine.GetStatus();
		if(wifi_status == -1 || wifi_status == 1)
		{
			trace("--------Notice:Start WiFi Scanning");
			wifi_scan();
			WifiEngine.AutoConnect();
		}
	}
	if(dongleSSID == "null")
		dongleSSID = WifiEngine.getssid();
	// Check client connect, and when connect to a router, then start client+p2p mode.
	var wifi_select=WifiEngine.GetIndexofSelectAP();
	trace("::::::::: wifi_select: "+wifi_select+" ::::::::");
	if(wifi_select > -1){   //MainSWF.wifi_mode_get_from_case == WifiEngine.WIFI_CONCURRENT_CLIENT_GETAUTOIP_OK){
		if(MainSWF.isHotspotReady() && connectCheckCount<8){
			connectCheckCount++;
			return;
		}
		var ip = SystemInfo.getIP(0);
		trace(":::::::: IP: "+ip);
		if(ip != "error"){
			if(autoConnectInterval > 0){
				clearInterval(autoConnectInterval);
				autoConnectInterval = -1;
				isLastApExist = 0;
			}
			//initMiracast();
			if(initInterval>0){
				clearInterval(initInterval);
				initInterval = -1;
			}
			initInterval = setInterval(initClientP2P, 3000);
			return;
		}
	}

	// If the system is start and enter first swf, then will wait auto connect. (max 10s)
	trace(":::::::: isLastApExist = "+isLastApExist+" ::::::::");
	if(isLastApExist){
		trace("connectCheckCount = "+connectCheckCount);
		if(connectCheckCount<8){
			connectCheckCount++;
			return;
		}else{
			isLastApExist = 0;
			initP2PSoftap();
		}
	}

	// If client is not connected and could not auto connect to router, then will start p2p+softap mode.
/*
	if(autoConnectInterval > 0){
		clearInterval(autoConnectInterval);
		autoConnectInterval = -1;
	}
	isLastApExist = 0;
	initP2PSoftap();
*/
}

function wifi_scan(){
	WifiEngine.StartScan();
}

function start(){
	connectCheckCount = 0;
	isLastApExist = -1;
	//wifi_scan();
	if(autoConnectInterval > 0){
		clearInterval(autoConnectInterval);
		autoConnectInterval = -1;
	}
	autoConnectInterval = setInterval(autoConnectCheck, 1000);
}
start();

var temp_count = -1;
function websetting_get_scanresults()
{
	if(temp_count < 0){
		trace("::::::::::: do scan ::::::::::");
		WifiEngine.StartScan();
		var app_total_tmp:Number = 0;
		temp_count = 0;
		return;
	}

	trace("::::::::::: get scanresults ::::::::::");
	temp_count++;
	app_total_tmp = WifiEngine.GetScanResults();
	trace("________ app_total_tmp = "+app_total_tmp);
	if(app_total_tmp > 1 || temp_count >= 4){
		if(checkScanAndConnect != -1)
		{	   
			clearInterval(checkScanAndConnect); 
			checkScanAndConnect = -1;
		}
		trace("::::::::::: connect ::::::::::");
		WifiEngine.websettingConnect();
		isLastApExist = 1;
		start();
	}else{
		if(app_total_tmp == 1){
			WifiEngine.StartScan();
		}
	}

}

function websettingConnectScan(){
	var ret:Number = 0; 
	trace("::::::::::: scan ::::::::::");

	WifiEngine.Open(); 
	ret = WifiEngine.Start(); 
	//trace(" wifi Start========== " + ret);    
    
	if(ret == 0)
	{        
		temp_count = -1;
		if(checkScanAndConnect != -1)
		{	   
			clearInterval(checkScanAndConnect); 
			checkScanAndConnect = -1;
		}
		checkScanAndConnect = setInterval(websetting_get_scanresults, 2000); 
		
	}else{
		trace("::::::::::: not scan and start miracast ::::::::::");
		start();
	}
}

var startTime:Number = 0;
var CONNECT_DELAY_TIME:Number = 5;
var surplusTime:Number = 0;
function toScanAndConnectDelay(){
	var currentTime:Number = SystemInfo.time();
	var runTime:Number = currentTime - startTime;
	trace(":::::: currentTime: "+currentTime+", runTime: "+runTime);
	if(scanInterval > 0){
		clearInterval(scanInterval);
		scanInterval = -1;
	}
	if(runTime < surplusTime){
		toScanAndConnect((surplusTime - runTime) * 1000);
	}else{
		if(MainSWF.isHotspotReady()){
			WifiEngine.websettingConnect();
			isLastApExist = 1;
			start();
		}else{
			websettingConnectScan();	
		}
	}
	
}
function toScanAndConnect(time:Number){
	startTime = SystemInfo.time();
	surplusTime = time/1000;
	trace(":::::: time: "+time+", start time: "+startTime);
	if(scanInterval > 0){
		clearInterval(scanInterval);
		scanInterval = -1;
	}
	scanInterval = setInterval(toScanAndConnectDelay, time);
}

function toWebsettingConnect(){
	trace("--------------------goto websetting connect!!!");
	wifiDisplayStop();
	uninitMiracast();
		
	if(MainSWF.current_lite_mode != WidiEngine.MIXED_STATUS_CLIENT_SOFTAP){
		WidiEngine.ModeChange(WidiEngine.MIXED_MODE_CLIENT_SOFTAP);
		MainSWF.showSoftapInfo();		
	}
	
	//toScanAndConnect(CONNECT_DELAY_TIME * 1000);
}

function check_help_photo_upload(){
	var ret:Number = PhotoEngine.check_help_picture();	
	if(ret){
		//PhotoEngine.show_help_picture();
	}

	return;
}

function showHelpPhoto(){
	check_help_photo_upload();
}

function UI_init(){
	if(SHOW_IP){
		step2_mc.ip_connect_mc._visible = true;
		step2_mc.ip_mc._visible = true;
		//showIP(1);
	}else{
		step2_mc.ip_connect_mc._visible = false;
		step2_mc.ip_mc._visible = false;
	}
	ezcastProUI();
	if(MainSWF.isAutoplayEnable){
		step2_mc.dongle_mc.ezchannel_mc.gotoAndStop(2);
		step2_mc.dongle2_mc.ezchannel_mc.gotoAndStop(2);
	}
	__fontsize_adjust();	
}

UI_init();
//#include "multilanguage.as"
//setLanguageSWFName("miracast", localeCallback);
MainSWF.setLanguageSWFName("ezcast", localeCallback);

function localeCallback() {
	trace("+++++++++++++++++localeCallback+++++++++++++++++++++++");
	var i:Number=0;
	prompts1_txt.text=Locale.loadString("IDS_prompts4_txt");
	prompts2_txt.text=Locale.loadString("IDS_prompts5_txt");
}


