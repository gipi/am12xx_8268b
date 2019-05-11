/**
*	@author   liangwenhua
*	@version 1.0
* 	@date: 2013/06/25 
*	@description  miracast 
*  
*/
import Actions.WidiEngine;
//import Actions.Ime;
import Actions.Locale;
import Actions.WifiEngine;
import Actions.SystemInfo;
import Actions.MiracastEngine;
import Actions.CSwfKey;
import Actions.PhotoEngine;
/*-------------------richardyu 081914----------------------*/
//trace("EZCASTPRO_MODE=="+MainSWF.EZCASTPRO_MODE);
if(MainSWF.EZCASTPRO_PROJECTOR){
	bg_new_mc.gotoAndStop(2);
	
	dongle._visible = false;
	miracast_ssid._width+=30;
	time_mc._x+=30;
	second_mc._x+=30;
	pincode_mc._x+=30;
	time_show.clock._x+=30;
}
else if(MainSWF.EZCASTPRO_MODE){
//if(1){
	dongle.gotoAndStop(2);
	miracast_ssid._width+=30;
	time_mc._x+=30;
	second_mc._x+=30;
	pincode_mc._x+=30;
	time_show.clock._x+=30;

}else{
	dongle.gotoAndStop(1);
}
/*-------------------richardyu 081914----------------------*/
var miracast_has_start:Number=0;
MainSWF.backMenu=false;
var tick:Number=61;
var time_tick_id:Number = -1;
var reconnect_tick:Number=2;
var reconnect_time_tick_id:Number=-1;
var miracast_noconnect:Number=0;
var connect_success_flag:Number=-1;
var connecting_flag:Number=-1;
var enbale_concurrent:Number=-1;
var wifi_direct_enbale_kernel_timer_id:Number = -1;
var wifi_direct_check_connection_request_timer_id:Number = -1;
var delay_for_enable_clientap:Number=-1;
var scan_ap_flag:Number=-1;
var reset_time_tick_flag:Number=-1;
var checkScanRes:Number=null;
var current_status:String=MainSWF.STATUS_MIRACAST_OFF;
var pic_open:Number = 0;
var QR_open:Number = 0;
var checkphotoInterval:Number = -1;
var HomePageOnInterval:Number = -1;
var isHomePageOnTimer:Boolean = false;
var isBackToEZCast:Boolean = true;
var set_waiting_flag:Number = -1;
var jump_next_flag:Number = -1;
var time_out_flag:Number = -1;
var time_out_tick_num:Number = 0;
var check_return_error:Number = 0;
var client_scan_tick:Number=0;
var photo_mc = this.help_photo_mc;
var isLastApExist:Number=1;
var connectCheckCount:Number = 0;
var otaCheck:Boolean = false;
var otaCheck_inter:Number = -1;
var checkUpdateStatusCount:Number = 0;
var current_volume_value = SystemInfo.getVolume();
var miracast_run_status:Boolean = false;
var checkGetPeerIpInterval:Number = -1;
var checkGetPeerIpStartTime:Number = 0;
var negoInterval:Number = -1;
var negoHnadleStartTime:Number = 0;
_global.MiracastSWF = this; 

var thumbListener:Object = new Object ();
thumbListener.onLoadInit = function (target_mc:MovieClip)
{
	target_mc._width = 274;
	target_mc._height = 274;
};
var mc_loader:MovieClipLoader=new MovieClipLoader();
mc_loader.addListener(thumbListener);


pincode_mc._visible=false;
miracast_logo_mc._visible=false;
time_show._visible=false;
status_txt._visible=false;
internet_ap_txt._visible=false;
second_mc._visible=false;
step_one_txt._visible=false;
step_two_txt._visible=false;
wait_mc._visible=false;
waiting_mc._visible=false;
var connect_pin_flag:Number = WidiEngine.WIDI_PINCODE_NONE;
miracast_ssid.text="";
status_txt.text ="";
internet_ap_txt.text ="";
second_mc.text="";
time_mc.text="";
step_one_txt.text="";
step_two_txt.text="";
internet_ap_txt.text="";
function localeCallback()
{
	second_mc.text="seconds"; 
	prompts1_txt.text="EZMirror display resolution is decided by smartphone/tablet";
	prompts2_txt.text="If EZMirror connection fails above twice, please restart your smartphone or check if your smartphone supports miracast.";
	prompts3_txt.text="Please use EZMirror for Netflix, Google video and BBC contents.";
}
localeCallback();

function exitQRcode(){
	trace("exit QR "+QR_open);
	if(QR_open){
		QR_open = 0;
		QR._visible = false;
		mc_loader.unloadClip(QR);
		PhotoEngine.Detach(QR,false);
		PhotoEngine.stopDecode(1);
		PhotoEngine.Close();
		QR._x = 0;
		QR._y = 0;
	}
}

var miracast_status:Number=-1;
function time_tick()
{
	tick--;
       if(tick<0)
          tick=0;
	//trace("=========tick="+tick);
	if(tick<=0)
	{
		check_return_error++;
		if(check_return_error>=2)
			connect_success_flag=-1;
		if(connect_success_flag==-1)
		{
			time_mc.text=tick;
			clearInterval(time_tick_id);
			time_tick_id=-1;
			if(isBackToEZCast){
				isBackToEZCast = false;
				trace("Back to ezcast from time_tick timeout!!!");
				MainSWF.loadswf("ezcast.swf");
			}else{
				stop_miracast_start_concurrent_wifi();	
			}
		}
	}	
	time_mc.text=tick;
}
function reconnect_time_tick()
{
	reconnect_tick--;
	time_mc.text=reconnect_tick;
	if(reconnect_tick==0)
	{
		time_mc.text=reconnect_tick;
		if(reconnect_time_tick_id!=-1)
		{
			clearInterval(reconnect_time_tick_id);	
			reconnect_time_tick_id=-1;
		}
		stop_miracast_start_concurrent_wifi();
	}	
}
function scan_ap()
{	
	//WifiEngine.StartScan();
	ExternalInterface.call("wifi_StartScan");
	client_scan_tick++;
	if(client_scan_tick>2)
	{
		client_scan_tick=0;
		if(scan_ap_flag!=-1)
		{
			clearInterval(scan_ap_flag);
			scan_ap_flag=-1;
		}
	}
}
function set_scan_ap()
{
	//WifiEngine.StartScan();
	ExternalInterface.call("wifi_StartScan");
	if(scan_ap_flag!=-1)
	{
		clearInterval(scan_ap_flag);
		scan_ap_flag=-1;
	}
	scan_ap_flag = setInterval(scan_ap,2000);	
}
function enable_client_mode()
{
	WifiEngine.Stop();
	//WifiEngine.Close();	
	WifiEngine.Open();
	WifiEngine.Start();
	trace("MainSWF.EZCASTPRO_MODE"+MainSWF.EZCASTPRO_MODE);
	if(MainSWF.EZCASTPRO_MODE==8075&&SystemInfo.getConnectMode()!=SystemInfo.ROUTER_ONLY){
		trace("Probox rebridge");
		WifiEngine.Probox_rebridge();
	}
	set_scan_ap();
}
function time_out_tick()
{
	time_out_tick_num++;
	trace("time_out_tick_num=========="+time_out_tick_num);
	if(time_out_tick_num>35)
	{
		time_out_tick_num=0;
		if(time_out_flag!=-1)
		{
			clearInterval(time_out_flag);
			time_out_flag=-1;
			waiting_mc._visible=false;
			waiting_txt._visible=false;
			wait_mc._visible=false;
			status_txt._visible=true;
			status_txt.text= "Please reconnect your phone";
			clearInterval(set_waiting_flag);
			set_waiting_flag=-1;	
		}		
	}
}
function check_waiting_function()
{
	if(1==MainSWF.save_show_clientap_flag)
	{
		jump_next_flag=1;
		if(time_out_flag!=-1)
		{
			clearInterval(time_out_flag);
			time_out_flag=-1;
		}
		time_out_flag=setInterval(time_out_tick,1000);
		if(1==MainSWF.start_concurren_clientap_mode)
		{
			waiting_mc._visible=false;
			waiting_txt._visible=false;
			wait_mc._visible=false;
			status_txt._visible=true;
			status_txt.text= "Please reconnect your phone";
			clearInterval(set_waiting_flag);
			set_waiting_flag=-1;	
			if(time_out_flag!=-1)
			{
	
				clearInterval(time_out_flag);
				time_out_flag=-1;
			}
		}
	}
	else
	{
		if(1==jump_next_flag)
			return;
		if(1==MainSWF.start_concurrent_softap_mode)
		{
			waiting_mc._visible=false;
			waiting_txt._visible=false;
			wait_mc._visible=false;
			status_txt._visible=true;
			status_txt.text= "Please reconnect your phone";
			clearInterval(set_waiting_flag);
			set_waiting_flag=-1;
			if(time_out_flag!=-1)
			{
	
				clearInterval(time_out_flag);
				time_out_flag=-1;
			}
		}		
	}
}
function stop_miracast_start_concurrent_wifi()
{
	trace("---=== stop_miracast_start_concurrent_wifi ===---");
	time_show._visible=false;
	time_mc._visible=false;
	second_mc._visible=false;
	miracast_logo_mc._visible=false;
	miracast_ssid._visible=false;
	step_one_txt._visible=false;
	internet_ap_txt._visible=false;
	step_two_txt._visible=false;
	wait_mc._visible=false;
	status_txt._visible=false;
	waiting_mc._visible=true;
	waiting_txt._visible=true;
	waiting_txt.text= "Waiting";
	//trace("waiting_txt.text========================"+waiting_txt.text);
	clearInterval(time_tick_id);
	time_tick_id=-1;
	help_photo_mc._x = -1920;
	help_photo_mc._y = 0;
	trace("---=== miracast_has_start: "+miracast_has_start+" ===---");
	if(miracast_has_start==1)
	{
		miracast_has_start=0;
		clear_rtsp_status_check_timer_id();
		MiracastEngine.RtspDestroy();
		clear_wifi_direct_enbale_kernel_timer_id();
		clear_wifi_direct_check_connection_request_timer_id();
		WidiEngine.Stop();
		
		//WifiEngine.start_softap();
		ExternalInterface.call("wifi_start_softap");
		
		enable_client_mode();

		MainSWF.main_bar.softap_info.ssid.text=WifiEngine.getssid();
		//MainSWF.main_bar.softap_info.psk.text=WifiEngine.getpassword();
		//psk = WifiEngine.getpassword();
		_global.MainSWF.main_bar.softap_info.txt1.text=_global.MainSWF.psk;
	}
	MainSWF.statusMiracastStop();
	MainSWF.iconChangeToEZMirrorOff();
	if(scan_ap_flag!=-1)
	{
		clearInterval(scan_ap_flag);
		scan_ap_flag=-1;
	}
	current_status=MainSWF.STATUS_MIRACAST_OFF;
	if(set_waiting_flag!=-1)
	{
		clearInterval(set_waiting_flag);
		set_waiting_flag=-1;
	}	
	//set_waiting_flag=setInterval(check_waiting_function,1000);
	if(miracast_run_status){
		MainSWF.set_miracast_status(0);
		miracast_run_status = false;
	}
	if(MainSWF.EZCASTPRO_MODE!=0){//richardyu 090414	
		WifiEngine.hostcontrol_user_clean();			
	}
	
	exitQRcode();
}

function showQRcode()
{
	var hp_url:String = "/tmp/tmp_qrcode.jpg";

	if(QR_open == 0){
		QR_open = 1;
		PhotoEngine.Open();
		//PhotoEngine.decode_bmp();
	}
	trace("show QR photo !!!!!!!!!!!!!!!!!!");
	PhotoEngine.SetPlayRatio(PhotoEngine.PE_RATIO_ACTUAL_SIZE);
	mc_loader.loadClip(hp_url, QR);
	
	QR._x= 819;
	QR._y= 286;
	QR._width = 274;
	QR._height = 274;
	QR._visible = true;
}

function Miracast_init()
{
	MainSWF.iconChangeToEZMirrorOn();
	if(miracast_has_start != 1){
		/*
		if(isHomePageOnTimer){
			isHomePageOnTimer = false;
			tick = 121;
		}
		else{
			tick=61;	
		}
		*/
		isBackToEZCast = true;
		status_txt._visible=false;
		wait_mc._visible=false;
		miracast_logo_mc._visible=true;
		time_show._visible=true;
		step_one_txt._visible=true;
		second_mc._visible=true;	
		step_one_txt.text="Connect 3G/4G or connect to an internet AP :"; 
		step_two_txt._visible=true;
		step_two_txt.text="Connect"; 
		internet_ap_txt._visible=true;
		internet_ap_txt.text=WifiEngine.getconnectedssid();
		trace("internet_ap_txt.text============"+internet_ap_txt.text);
		miracast_has_start=1;
		MainSWF.statusMiracastStart();
		WifiEngine.WifiDisableRouting();
		WifiEngine.Stop();
		if(MainSWF.EZCASTPRO_MODE!=0){//richardyu 080514	
			WifiEngine.hostcontrol_user_clean();			
		}	
		/** init the RTSP engine */
		MiracastEngine.RtspInit();
		/** init P2P */
		WidiEngine.Init(WidiEngine.WIDI_DEVICE_MODE);
		/**
		* Note: Default device name is the hostname. We can set it to a new one
		*		after call WidiEngine.Init() and before call WidiEngine.Start()
		*/
		miracast_ssid._visible=true;
		time_mc._visible=true;
		miracast_ssid.text = MainSWF.devName;
		WidiEngine.SetLocalDevName(MainSWF.devName);
		pincode_mc.txt.text = WidiEngine.GetPincode();
		pincode_mc._visible=true;
		SystemInfo.setVolume(14);
		trace("miracast_ssid.text==================="+miracast_ssid.text);
		/** start the P2P */
		wifi_direct_enable();
		/*
		if(time_tick_id!=-1)
		{
			clearInterval(time_tick_id);
			time_tick_id=-1;
		}
		time_tick_id = setInterval(time_tick,1000);		
		*/
		//MainSWF.main_bar.softap_info._visible=false;
		//MainSWF.main_bar.softap_icon._visible=false;
	}
}
/**
* for Miracast, check the RTSP status
*/
var rtsp_status_check_timer_id:Number=-1;
function clear_rtsp_status_check_timer_id()
{
	if(rtsp_status_check_timer_id != -1)
	{
		clearInterval(rtsp_status_check_timer_id);
		rtsp_status_check_timer_id = -1;
	}	
}
function miracast_restart()
{
	/** detect stop status */
	trace("---=== miracast_restart ===---");
	miracast_noconnect=0;
	connect_success_flag=-1;
	clear_rtsp_status_check_timer_id();
	/** call the RtspStop() to stop RTSP service. */
	MiracastEngine.RtspStop();
	/** Restart the WiFi direct to do next term Miracast */
	WidiEngine.ReStart();
	wifi_direct_check_connection_request_timer_id = setInterval(wifi_direct_check_connection_request,1000);
	status_txt._visible=false;
	status_txt.text="";
	wait_mc._visible=false;
	miracast_logo_mc._visible=true;
	time_show._visible=true;
	time_mc._visible=true;
	step_one_txt._visible=true;
	second_mc._visible=true;	
	miracast_ssid._visible=true;	
	step_one_txt.text="Connect 3G/4G or connect to an internet AP :"; 
	step_two_txt._visible=true;
	step_two_txt.text="Connect"; 
	internet_ap_txt._visible=true;
	internet_ap_txt.text=WifiEngine.getconnectedssid();
	showQRcode();
	/*
	tick=121;
	time_mc.text="120";
	if(time_tick_id!=-1)
	{
		clearInterval(time_tick_id);
		time_tick_id=-1;
	}
	time_tick_id = setInterval(time_tick,1000);	
	*/
}
function rtsp_status_check()
{
	var rtsp_status:Number;
	var p2p_status:Number;
	rtsp_status = MiracastEngine.RtspStatus();
	//trace("rtsp_status========================="+rtsp_status);
	if(rtsp_status==MiracastEngine.RTSP_STAT_STOP)
	{
		/*
		if(connect_success_flag==1)
		{
			trace("_________ MiracastHomePage: "+MiracastHomePage);
			if(MainSWF.MiracastDefault){
				miracast_restart();
				return;
			}
			if(isBackToEZCast){
				isBackToEZCast = false;
				trace("Back to ezcast from RTSP_STAT_STOP!!!");
				MainSWF.loadswf("ezcast.swf");
				return;
			}
			time_show._visible=true;
			time_mc._visible=true;
			second_mc._visible=true;		
			connect_success_flag=-1;
			if(time_tick_id!=-1)
			{
				clearInterval(time_tick_id);
				time_tick_id=-1;
			}
			time_mc.text=reconnect_tick;
			clearInterval(rtsp_status_check_timer_id);
			stop_miracast_start_concurrent_wifi();
			status_txt.text= "Please reconnect your phone";
			return;
		}
		connect_success_flag=-1;
		*/
		miracast_restart();
		if(miracast_run_status){
			MainSWF.set_miracast_status(0);
			miracast_run_status = false;
		}
	}
	else if(rtsp_status==MiracastEngine.RTSP_STAT_IDLE)
	{
		miracast_noconnect++;
		connect_success_flag=-1;
	}
	else if(rtsp_status==MiracastEngine.RTSP_STAT_PLAYING)
	{
		p2p_status = WidiEngine.GetConnectionStatus();
		if(p2p_status != 0)
		{
			trace("p2p disconnected when do miracast!!");
			MiracastEngine.DropMira();
			//miracast_restart();
			miracast_noconnect=0;
			connect_success_flag=1;
			//stop_miracast_start_concurrent_wifi();
		}
		else
		{
			miracast_noconnect=0;
			connect_success_flag=1;
			if(time_tick_id!=-1)
			{
				clearInterval(time_tick_id);
				time_tick_id=-1;
			}				
		}
	}
	if(miracast_noconnect==61)
	{
		trace("---=== rtsp_status_check: timeout ===---");
		miracast_restart();
		if(miracast_run_status){
			MainSWF.set_miracast_status(0);
			miracast_run_status = false;
		}
		miracast_noconnect=0;
		/*
		time_show._visible=true;
		time_mc._visible=true;
		second_mc._visible=true;	
		time_mc.text="0";
		miracast_noconnect=0;
		connect_success_flag=-1;
		if(isBackToEZCast){
			isBackToEZCast = false;
			trace("Back to ezcast from miracast_noconnect timeout!!!");
			MainSWF.loadswf("ezcast.swf");
			return;
		}
		stop_miracast_start_concurrent_wifi();
		*/
	}
}

function checkGetPeerIpAddrResult(){
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
		checkGetPeerIpStartTime = 0;
		clear_wifi_direct_enbale_kernel_timer_id();
		clear_wifi_direct_check_connection_request_timer_id();
		if(time_tick_id!=-1)
		{
			clearInterval(time_tick_id);
			time_tick_id=-1;
		}
		MiracastEngine.RtspStart();
		step_one_txt._visible=false;
		internet_ap_txt._visible=false;
		step_two_txt._visible=false;
		miracast_logo_mc._visible=false;
		//miracast_ssid._visible=false;
		time_show._visible=false;
		time_mc._visible=false;
		second_mc._visible=false;			
		wait_mc._visible=true;
		status_txt._visible=true;
		status_txt.text = "Do RTSP Connection...";
		if(pic_open){
			help_photo_mc._x=-1920;
			PhotoEngine.Detach(help_photo_mc,false);
			
			PhotoEngine.stopDecode(1);
			PhotoEngine.Close();
			pic_open = 0;
		}
		exitQRcode();
		if(!miracast_run_status){
			MainSWF.set_miracast_status(1);
			miracast_run_status = true;
		}
		/** start a timer to listen the rtsp status*/
		rtsp_status_check_timer_id = setInterval(rtsp_status_check,1000);
		return;
	}
	else
	{
		trace(">>>>>> RTSP ip address error");
		if(checkGetPeerIpInterval > 0)
		{
			clearInterval(checkGetPeerIpInterval);
			checkGetPeerIpInterval = -1;
		}
		checkGetPeerIpStartTime = 0;
		reset_time_tick_flag=1;
		wifi_direct_handle_nego_out();
	}
}

function wifi_direct_handle_nego_out(){
	trace("---=== wifi_direct_handle_nego_out: reset_time_tick_flag("+reset_time_tick_flag+") ===---");
	clear_wifi_direct_enbale_kernel_timer_id();
	clear_wifi_direct_check_connection_request_timer_id();
	WidiEngine.ReStart();
	/** after restart, continue to check the connection request */
	wifi_direct_check_connection_request_timer_id = setInterval(wifi_direct_check_connection_request,1000);
	status_txt._visible=false;
	status_txt.text="";
	wait_mc._visible=false;
	status_txt._visible=true;
	miracast_logo_mc._visible=true;
	//time_show._visible=true;
	//time_mc._visible=true;
	step_one_txt._visible=true;
	//second_mc._visible=true;	
	miracast_ssid._visible=true;	
	step_one_txt.text="Connect 3G/4G or connect to an internet AP :"; 
	step_two_txt._visible=true;
	step_two_txt.text="Connect"; 
	internet_ap_txt._visible=true;
	internet_ap_txt.text=WifiEngine.getconnectedssid();
	//if(reset_time_tick_flag==1)
	//{
	reset_time_tick_flag=0;
	time_show._visible=true;
	time_mc._visible=true;
	second_mc._visible=true;	
	tick=70;
	time_mc.text="60";
	if(time_tick_id!=-1)
	{
		reset_time_tick_flag=0;
		time_show._visible=true;
		time_mc._visible=true;
		second_mc._visible=true;
		/*
		tick=70;
		time_mc.text="60";
		if(time_tick_id!=-1)
		{
			clearInterval(time_tick_id);
			time_tick_id=-1;
		}
		time_tick_id = setInterval(time_tick,1000);		
		*/
	}
}

function wifi_direct_handle_nego(nego_result:Number)
{
	var ret_1:Number;
	if(nego_result==0)
	{
		/** Miracast */
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
		}else{
			trace(">>>>>> Get RTSP ip address error");
			reset_time_tick_flag=1;
		}
	}
	else
	{
		reset_time_tick_flag=1;	
	}
	wifi_direct_handle_nego_out();

}

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
	}
}

function wifi_direct_check_connection_request()
{
	//var pin_conde_flag:Number = -1;
	var pin_code:String;
	var nego_ok:Number = 0;
	//pin_conde_flag = WidiEngine.GetPinCodeFlag();
	if(connect_pin_flag == WidiEngine.WIDI_PINCODE_PBC)
	{
		trace(">>>>>> PBC:");
		nego_ok = WidiEngine.DoNegotiation(WidiEngine.WIDI_PINCODE_PBC);
		toNegoHandle(nego_ok);
	}
	else if(connect_pin_flag == WidiEngine.WIDI_PINCODE_INPUT)
	{
		/** not support for Miracast */
		wifi_direct_handle_nego(1);
		connect_pin_flag = WidiEngine.WIDI_PINCODE_NONE;
	}
	else if(connect_pin_flag == WidiEngine.WIDI_PINCODE_DISPLAY)
	{
		/** default will show the PIN */
		//WidiEngine.GeneratePincode();
		//wifi_pincode_display_content = WidiEngine.GetPincode();
		nego_ok = WidiEngine.DoNegotiation(WidiEngine.WIDI_PINCODE_DISPLAY);
		toNegoHandle(nego_ok);
	}
	else if(connect_pin_flag == WidiEngine.WIDI_PINCODE_INVITED)
	{
		/** invited to join a group */
		trace(">>>>>>> invited to join a group");
		nego_ok = WidiEngine.DoJoinGroup();
		wifi_direct_handle_nego(nego_ok);
		connect_pin_flag = WidiEngine.WIDI_PINCODE_NONE;
	}
	else if(connect_pin_flag == WidiEngine.WIDI_PINCODE_PERSISTENT_GP_INVITED)
	{
		/** invited to join a group */
		trace(">>>>>>> invited to join a persistent group and role is client");
		WidiEngine.DoJoinPersistentGroup();
		wifi_direct_handle_nego(0);
		connect_pin_flag = WidiEngine.WIDI_PINCODE_NONE;
	}
	else
	{
		connect_pin_flag = WidiEngine.GetPinCodeFlag();
		if(connect_pin_flag != WidiEngine.WIDI_PINCODE_NONE)
		{
			step_one_txt._visible=false;
			internet_ap_txt._visible=false;
			step_two_txt._visible=false;
			miracast_logo_mc._visible=false;
			//miracast_ssid._visible=false;
			time_show._visible=false;
			time_mc._visible=false;
			second_mc._visible=false;		
			wait_mc._visible=true;
			status_txt._visible=true;
			/*************delete timer start***************************************/
			if(time_tick_id!=-1)
			{
				clearInterval(time_tick_id);
				time_tick_id=-1;
			}
			/**************delete timer end**************************************/
			status_txt.text = "DO P2P Connection";
		}
	}
}
function clear_wifi_direct_enbale_kernel_timer_id()
{
	if(wifi_direct_enbale_kernel_timer_id != -1)
	{
		clearInterval(wifi_direct_enbale_kernel_timer_id);
		wifi_direct_enbale_kernel_timer_id = -1;
	}	
}
function clear_wifi_direct_check_connection_request_timer_id()
{
	if(wifi_direct_check_connection_request_timer_id != -1)
	{
		clearInterval(wifi_direct_check_connection_request_timer_id);
		wifi_direct_check_connection_request_timer_id = -1;
	}	
}

function wifi_direct_enbale_kernel()
{
	clear_wifi_direct_enbale_kernel_timer_id();
	/** start P2P */
	WidiEngine.Start();
	/** check connection request */
	clear_wifi_direct_check_connection_request_timer_id();
	wifi_direct_check_connection_request_timer_id = setInterval(wifi_direct_check_connection_request,1000);
}
function wifi_direct_enable()
{
	clear_wifi_direct_enbale_kernel_timer_id();
	wifi_direct_enbale_kernel_timer_id = setInterval(wifi_direct_enbale_kernel,500);
}
currentswf_out = function():Void
{
	//if(current_status==MainSWF.STATUS_MIRACAST_ON)
	stop_miracast_start_concurrent_wifi();
	current_status=MainSWF.STATUS_MIRACAST_OFF;
	if(checkphotoInterval > 0){
		clearInterval(checkphotoInterval);
		checkphotoInterval = NULL;
	}
	if(pic_open){
		//PhotoEngine.Detach(help_photo_mc,false);
		PhotoEngine.stopDecode(1);
		PhotoEngine.Close();
		help_photo_mc._x = -1920;
		help_photo_mc._y = 0;
	}
	
	SystemInfo.setVolume(current_volume_value);
	MainSWF.iconChangeToEZMirrorOff();
	mc_loader.removeListener(thumbListener);
}
function_on = function():Void
{
	trace("miracast on");
	current_status=MainSWF.STATUS_MIRACAST_ON;
	Miracast_init();
	MainSWF.feedback(MainSWF.MIRACASTON);
}
function_off = function():Void
{
	trace("miracast off");
	if(current_status==MainSWF.STATUS_MIRACAST_ON)
	{	
		trace("start concurrent wifi")
		stop_miracast_start_concurrent_wifi();
	}
	SystemInfo.setVolume(current_volume_value);
	current_status=MainSWF.STATUS_MIRACAST_OFF;
	if(pic_open){		
		photo_mc._x = -1920;
		photo_mc._y = 0;		
	}
	MainSWF.feedback(MainSWF.MIRACASTOFF);
}
function check_current_status():String
{
	//MainSWF.feedback(current_status);
	return current_status;
}



if(MainSWF.feedbackenable == true){
	MainSWF.feedback(MainSWF.MIRACAST_SWITCHTO);
}else{
	MainSWF.feedbackenable = true;
}

function do_help_photo_play(){
	var pic_width:Number = 1920;
	var pic_height:Number = 1080;
	var hp_url:String = "/tmp/helptoview.jpg";
	photo_mc._width = pic_width;
	photo_mc._height = pic_height;
	photo_mc._x=(1920-pic_width)/2;
	photo_mc._y=1080-pic_height;
	//dlna_mc._x=-1920;
	//dlna_mc._y=0;
	if(pic_open == 0){
		pic_open = 1
		PhotoEngine.Open();
	}

	trace("show a photo!!!!!!!!!!!!!!!!!!");

	
	mc_loader.loadClip(hp_url, photo_mc);


	
	//dlna_photo_play = 1;
	//PhotoEngine.Close();
}

function check_help_photo_upload(){
	var ret:Number = PhotoEngine.check_help_picture();
	if(ret){
		//if(current_status==MainSWF.STATUS_MIRACAST_OFF)
			//return;
		do_help_photo_play();
	}

	return;
}

function showHelpPhoto(){
	check_help_photo_upload();
}

function language_change() 
{
	SystemInfo.setCurLanguage(_global.MainSWF.language);
	SystemInfo.storeConfig();
	MainSWF.setLanguageSWFName("ezcast", localeCallback);
}

var startMirrDelay:Number = -1;
function EZMirrorOn(){
	if(startMirrDelay){
		clearInterval(startMirrDelay);
		startMirrDelay = -1;
	}
	current_status=MainSWF.STATUS_MIRACAST_ON;
	Miracast_init();
}

function toStartMirr(){
	if(MainSWF.EZCAST5G_ENABLE){
		if(startMirrDelay){
			clearInterval(startMirrDelay);
			startMirrDelay = -1;
		}
		trace("-----======== delay 5s ===========---------");
		startMirrDelay = setInterval(EZMirrorOn, 5000);
	}else{
		EZMirrorOn();
	}
}

function ota_check_fun()
{
	checkUpdateStatusCount++;
	var ret = SystemInfo.getOtaCheckStatus();
	var connect_flag:Boolean = true;
	if(ret == 1) { //check ok

		server_version = SystemInfo.getOtaServerVersion();	
		trace("[EZMirror First]server_version: "+server_version);
		if(otaCheck_inter > 0){
			clearInterval(otaCheck_inter);
			otaCheck_inter = -1;
		}
		if(server_version != "error" && server_version != "newest")
		{
			trace("____________[show]server_version: "+server_version);
			_global.MainSWF.showUpgradeIcon();
			//SystemInfo.set_lastui(_global.MainSWF.LASTUI_3TO1, 0);
			//trace("Back to ezcast from ota_check_fun");
			//MainSWF.loadswf("ezcast.swf");
		} else {
			_global.MainSWF.hideUpgradeIcon();			
		}
		EZMirrorOn();
	}else if(checkUpdateStatusCount > 20){
		if(otaCheck_inter > 0){
			clearInterval(otaCheck_inter);
			otaCheck_inter = -1;
		}
		checkUpdateStatusCount = 0;

		trace("[EZMirror First]OTA check time out!!!");
		_global.MainSWF.hideUpgradeIcon();
		EZMirrorOn();
	}

}

function HomePageOn(){
	trace("miracast HomePage on");
	if(!MainSWF.topBarShowCompleted)
		return;
		
	showQRcode();
	
	trace("isLastApExist = "+isLastApExist);
	if(isLastApExist){
		trace("connectCheckCount = "+connectCheckCount);
		if(connectCheckCount<5){
			connectCheckCount++;
			trace("otaCheck = "+otaCheck);
			if(!otaCheck){
				if(MainSWF.wifi_mode_get_from_case!=WifiEngine.WIFI_CONCURRENT_CLIENT_GETAUTOIP_OK){
					trace("connect error!!!");
					return;
				}else{
					otaCheck = true;
					return;
				}
			}
		}
	}
	if(HomePageOnInterval > 0){
		clearInterval(HomePageOnInterval);
		HomePageOnInterval = -1;
	}
	trace("______otaCheck = "+otaCheck);
	if(otaCheck){
		_global.MainSWF.fw_update_main_flag = true;
		_global.MainSWF.haveCheckOtaVersion = true;
		_global.MainSWF.clearcheckotaversion();
		SystemInfo.ota_upgrade(4,"http://www.iezvu.com/upgrade/ezcast/ezcast.conf"); 
		checkUpdateStatusCount = 0;
		if(otaCheck_inter > 0){
			clearInterval(otaCheck_inter);
			otaCheck_inter = -1;
		}
		otaCheck_inter = setInterval(ota_check_fun, 1000);
	}else{
		toStartMirr();
	}
}

//if(MainSWF.MiracastHomePage){
	isLastApExist = WifiEngine.isLastRouterApExist();
	trace("__________isLastApExist = "+isLastApExist);
	HomePageOnInterval = setInterval(HomePageOn, 2000);
	isHomePageOnTimer = true;
	MainSWF.MiracastHomePage = false;
//}

dlna_mc.render_mc.help_photo_mc.visible = false;
//checkphotoInterval = setInterval(check_help_photo_upload, 200);


