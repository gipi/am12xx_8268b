import Actions.SystemInfo;

var bgloader:MovieClipLoader = new MovieClipLoader();
var bgMC:MovieClip = this.ezcast_background_mc;
_global.EZCastSWF = this; 
function loadUI()
{
	trace("to load ezcast_background.swf");
	MainSWF.ezBgShowCompleted=false;
	bgloader.loadClip("ezcast_background.swf", this.ezcast_background_mc);
}

if(MainSWF.isLoadUi() && MainSWF.USER_MANUAL_DISPLAY)
	loadUI();

//*****************************       variable        ***************************************************
var wifiInterval:Number = -1;

var MUSIC_STOP:Number = 1
var MUSIC_PAUSE:Number = 2
var MUSIC_PLAY:Number = 3
var MUSIC_QUIT:Number = 4


var text_arr:Array=[
					ezcase_log_txt,
					dlna_log_txt,
					miracast_log_txt,
					ezair_log_txt,
					setting_log_txt
					]
var wifi_mode:Number=0;
var wifi_display_on:Boolean = false;
var showSSID:Boolean = false;

var current_status:String = MainSWF.STATUS_EZCAST_OFF;

var checkphotoInterval:Number = -1;
var isRouterCtlEn = SystemInfo.getRouterEnable();
var count_num:Number=0;
//******************************      functions     *****************************************************

trace("now into wifidisplay.swf");
//SystemInfo.setVolume(14);
//set language
//#include "ezcast_p2p_go.as"

trace("MainSWF.start_concurren_clientap_mode========"+MainSWF.start_concurren_clientap_mode);
trace("MainSWF.start_concurrent_softap_mode========"+MainSWF.start_concurrent_softap_mode);

if(!MainSWF.USER_MANUAL_DISPLAY)
{
	if(MainSWF.CASTCODE_W < 0)
		MainSWF.CASTCODE_W = 0;
		
	if(MainSWF.CASTCODE_H < 0)
		MainSWF.CASTCODE_H = 0;
		
	if(MainSWF.CASTCODE_SIZE < 0)
		MainSWF.CASTCODE_SIZE = 0;

	if(MainSWF.MIRACODE_W < 0)
		MainSWF.MIRACODE_W = 0;
		
	if(MainSWF.MIRACODE_H < 0)
		MainSWF.MIRACODE_H = 0;
		
	if(MainSWF.MIRACODE_SIZE < 0)
		MainSWF.MIRACODE_SIZE = 0;

	this.createTextField("costCode", this.getNextHighestDepth(), MainSWF.CASTCODE_X, MainSWF.CASTCODE_Y, MainSWF.CASTCODE_W, MainSWF.CASTCODE_H);
	var my_fmt:TextFormat = costCode.getTextFormat();
	my_fmt.color = MainSWF.CASTCODE_COLOR;
	my_fmt.size = MainSWF.CASTCODE_SIZE;
	costCode.setTextFormat(my_fmt);
}

function setTopBarIcon(){
	if(isRouterCtlEn){
		TopBarSWF.iconMode(TopBarSWF.ICONMODE_ROUTERALLOW);
	}else{
		TopBarSWF.iconMode(TopBarSWF.ICONMODE_APONLY);
	}
	if(MainSWF.EZCASTPRO_MODE==8075 && !MainSWF.LAN_ONLY)
	{

		if(MainSWF.get_ConnectMode() == SystemInfo.ROUTER_ONLY)
		{
			MainSWF.showRouterInfoForP2pClient();
			trace("------------MainSWF.EZCASTPRO_MODE"+MainSWF.EZCASTPRO_MODE);
		}else{
			MainSWF.setSoftapPosition(1);
			MainSWF.showSoftapInfo();
			}
	}
	
}

function topBarShowCheck(){
	if(!MainSWF.topBarShowCompleted){
		return;
	}

	if(wifiInterval > 0){
		clearInterval(wifiInterval);
		wifiInterval = -1;
	}
	//Miracast_init();
	setTopBarIcon();
	if(!MainSWF.LAN_ONLY){
		bgMC.show_dongle_ssid(MainSWF.mySsid);
		bgMC.show_websrv_IP();
		if(MainSWF.EZCASTPRO_MODE==8075){ 
			bgMC.show_dongle_24gssid(MainSWF.mySsid_24g);
		}
	}
	if(MainSWF.mySsid != MainSWF.devName)
		bgMC.show_dongle_devname("("+MainSWF.devName+")");
	else
		bgMC.show_dongle_devname("");
	bgMC.show_dongle_devname2("("+MainSWF.devName+")");
	MainSWF.netDisplayReady4TopBar();
	wifiInterval=setInterval(wifiDisplayStart,200);
}

function start(){
	if(wifiInterval > 0){
		clearInterval(wifiInterval);
		wifiInterval = -1;
	}
	if(MainSWF.CastCodeEnable && MainSWF.castCode != NULL && MainSWF.castCode != ""){
		if(MainSWF.USER_MANUAL_DISPLAY)
			bgMC.show_cast_code(MainSWF.castCode);
		else
			costCode.text = "Castcode("+MainSWF.castCode+")";
	}
	if(!MainSWF.topBarShowCompleted){
		wifiInterval=setInterval(topBarShowCheck,1000);
	}else{
		//Miracast_init();
		setTopBarIcon();
		if(!MainSWF.LAN_ONLY){
			bgMC.show_dongle_ssid(MainSWF.mySsid); 
			if(MainSWF.EZCASTPRO_MODE==8075){ 
				bgMC.show_dongle_24gssid(MainSWF.mySsid_24g);
			}
		}
		if(MainSWF.mySsid != MainSWF.devName)
			bgMC.show_dongle_devname("("+MainSWF.devName+")");
		else
			bgMC.show_dongle_devname("");
		bgMC.show_dongle_devname2("("+MainSWF.devName+")");
		MainSWF.netDisplayReady4TopBar();
		wifiInterval=setInterval(wifiDisplayStart,200);
	}
	if(MainSWF.topBarShowCompleted || MainSWF.start_concurrent_softap_mode==1){
		showSSID = true;
	}
}	

function wifiDisplayStart()
{
	trace("**********MainSWF.ezBgShowCompleted ="+MainSWF.ezBgShowCompleted+"   count_num="+count_num);
	if(!MainSWF.ezBgShowCompleted || count_num<3)
	{
		count_num++;
		return;
	}
	count_num=0;
	trace("wifi display ON");
	if(MainSWF.topBarShowCompleted || MainSWF.start_concurrent_softap_mode==1){
		showSSID = true;
	}
	
	if(wifiInterval > 0){
		clearInterval(wifiInterval);
		wifiInterval = -1;
	}
	if(wifi_display_on == false){
		if(MainSWF.CastCodeEnable && MainSWF.castCode != ""){
			SystemInfo.setdisplaypassword(MainSWF.castCode);
		}
		MainSWF.wifi_display=SystemInfo.StartNetDisplay();
		wifi_display_on = true;
		MainSWF.netDisplayOn4TopBar();
		if(MainSWF.mouse_in_flag==true)
			show_mouse();
		refresh_router_ssid();
	}
	
	setDefaultVolume();
	current_status = MainSWF.STATUS_EZCAST_ON;
	MainSWF.feedback(MainSWF.EZCASTON);
}

function wifiDisplayStop()
{
	MainSWF.netDisplayOff4TopBar();
	if(wifi_display_on == true){
		SystemInfo.StopNetDisplay();
		SystemInfo.setdisplaypassword("");
		wifi_display_on = false;
		if(MainSWF.mouse_in_flag==true)
			clean_mouse();
	}
	current_status = MainSWF.STATUS_EZCAST_OFF;
	MainSWF.feedback(MainSWF.EZCASTOFF);
	//SystemInfo.setVolume(SystemInfo.sys_volume_ctrl_get());
}

function currentswf_out(){
	trace("wifidisplay escape");
	if(wifiInterval > 0){
		clearInterval(wifiInterval);
		wifiInterval = -1;
	}
	wifiDisplayStop();
	//p2pGoOut();
}

function check_current_status():String
{
	//MainSWF.feedback(current_status);
	return current_status;
}

trace("********************  in wifi display ****************");

if(MainSWF.feedbackenable == true){
	MainSWF.sendMsgToApp(MainSWF.EZCAST_SWITCHTO);
}else{
	MainSWF.feedbackenable = true;
}

function check_help_photo_upload(){
	var ret:Number = MainSWF.ezCheckHelpPicture();	
	if(ret){
		MainSWF.ezShowHelp();
	}

	return;
}

function showHelpPhoto(){
	check_help_photo_upload();
}

function language_change() 
{
	setLanguageSWFName("ezcast", localeCallback);
}

function show_router_ssid(ssid:String){
	return;//Add by Denny
	trace("---------1----show_router_ssid");
	if(!wifi_display_on)
		return;

	bgMC.showRouterSsid(ssid);
}
function show_dongle_ip(ip:String){
	bgMC.show_dongle_ip(ip);
}
function clean_dongle_ip(){
	bgMC.clean_dongle_ip();
}
function updata_connect_icon(){
	bgMC.updata_connect_icon();
}
function clean_router_ssid(){
	if(!wifi_display_on)
		return;

	bgMC.cleanRouterSsid();
}
function show_mouse(){
	bgMC.ezMouseShow();
}
function clean_mouse(){
	bgMC.ezMouseClean();
}

function show_ezlauncher_icon(){
	bgMC.ezlauncher_icon_show();
}
function clean_ezlauncher_icon(){
	bgMC.ezlauncher_icon_clean();
}

function show_miracode(ip:String){
	if(MainSWF.USER_MANUAL_DISPLAY)
		bgMC.show_mira_code(ip);
	else
	{
		var miracode:String = SystemInfo.get_miracode(ip);
		trace("miracode:"+miracode);
		if(miracode != "error")
		{
			miracode = "Miracode("+miracode+")";
			MainSWF.ezCleanRect(MainSWF.MIRACODE_X, MainSWF.MIRACODE_Y, MainSWF.MIRACODE_W, MainSWF.MIRACODE_H);
			MainSWF.ezShowText(miracode, MainSWF.MIRACODE_SIZE, MainSWF.MIRACODE_COLOR, MainSWF.MIRACODE_X, MainSWF.MIRACODE_Y);
		}
	}
}
function clean_miracode(){
	if(MainSWF.USER_MANUAL_DISPLAY)
		bgMC.clean_mira_code();
	else
		MainSWF.ezCleanRect(MainSWF.MIRACODE_X, MainSWF.MIRACODE_Y, MainSWF.MIRACODE_W, MainSWF.MIRACODE_H);
}
function refresh(){
	if(MainSWF.isEZCastpro()&&SystemInfo.getConnectMode()!=0&&MainSWF.lanstatus_priv!=0)
	{
		
		   var ipForLan = MainSWF.lanIpForDisplay();
		   if(ipForLan=="")
		   		return;
		   var ret=ipForLan.indexOf(".", 2);
		   if(ipForLan != "" && ret >= 0)
		   {
				if(SystemInfo.get_miracode_enable())
				{  
	
					show_miracode(ipForLan);
					MainSWF.isLanMiracodeShowing=true;
				}
				show_dongle_ip(ipForLan);
				MainSWF.dongleLanIpShowing=true;
				
		    }
		
	}
	if(!MainSWF.LAN_ONLY){
		var wifi_select=MainSWF.GetIndexofSelectAP();
		if(wifi_select > -1){
			var clientAP = MainSWF.getconnectedssid();
			show_router_ssid(clientAP);
		}
	}
}

//checkphotoInterval = setInterval(check_help_photo_upload, 200);


