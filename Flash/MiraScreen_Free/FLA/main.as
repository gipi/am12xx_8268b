//dlna_mc.attachMovie("audio","music_play_mc",this.getNextHighestDepth());
import Actions.SystemInfo;
import Actions.WifiEngine;
import Actions.WidiEngine;
import Actions.CSwfKey;
import Actions.Locale;
import Actions.PhotoEngine;
//import Actions.QRcodeEngine;
//import Actions.VideoEngine;
import Actions.AudioEngine;
import Actions.LanEngine;
import Actions.Customization;
//import Actions.FileList;

var test_mode:Number=0;
var oldusbsta=2;
var oldsdsta=2;
var oldusbsta1=2;
var oldusbsta2=2;
var fw_update_setting_flag:Boolean = false;
_global.MainSWF = this; 
_global.StorageSWF = MainSWF.storage_bar_mc;
_global.CustomSWF = MainSWF.main_background_mc;
var language:Number=0; // 0=en, 1=ja, 2=zh-CN, 3=zh-TW
var language_priv:Number=-1;
var language_string:String = "";
var currentSWF:String = ""
var currentMC:MovieClip;
var wifi_display=-1;
var psk:String = "";
var mySsid:String = "";
var mySsid_24g:String ="";
var psk_str:String = "";
var warnning_str:String = "";
var warnning_str2:String = "";
var warnning_str3:String = "";
//var prompt1_str:String = "";
//var prompt2_str:String = "";
var devName:String = "";//SystemInfo.getDevName();

var output_mode=0;
var key_longpress=false;
var key_shortpress=false;
var timer_longcheck=0;
var timer_shortcheck=0;
var timer_key_count=0;
var firstEnterSetting:Boolean = true;
var websettingNeedReboot:Boolean = false;

var ssid_str:String = "";
/*-------------------richardyu 080514----------------------*/
var EZCASTPRO_MODE:Number=SystemInfo.ezcastpro_read_mode();
trace("EZCASTPRO_MODE=="+EZCASTPRO_MODE);
/*-------------------richardyu 080514----------------------*/
if(EZCASTPRO_MODE!=8074)var langdisautostatus:Number = 0;
else var langdisautostatus:Number = 1;
language=SystemInfo.getCurLanguage();
var LANGUAGE_NOT_SET_CALLBACK:Boolean = true;
//#include "multilanguage.as" 
LANGUAGE_NOT_SET_CALLBACK = false;
#include "Main_net_common.as"
function localeCallback(){
	warnning_str = "Wifi abnormality encountered, suggest to use external USB power adapter to guarantee power supply.";
	warnning_str2 = "Auto reboot in ";
	warnning_str3 = " seconds";
}
localeCallback();
ip_mc.set5.text = "192.168.203.1"; 


var EZCASTPRO_PROJECTOR:Number = (EZCASTPRO_MODE==8074)?1:0;
var EZCASTPRO_RUN:Number = 0;
trace("****ezcastprojector is " + EZCASTPRO_PROJECTOR);
var TEST_ENABLE:Boolean = (SystemInfo.getIncNumVal(SystemInfo.INC_TEST_ENABLE)==0)?false:true;	//true;
var EZCAST_LITE_ENABLE:Boolean = true;//(SystemInfo.getIncNumVal(SystemInfo.INC_EZCAST_LITE_ENABLE)==0)?false:true;	//true;
var EZWILAN_ENABLE:Boolean = (SystemInfo.getIncNumVal(SystemInfo.INC_EZWILAN_ENABLE)==0)?false:true;
var EZCAST5G_ENABLE:Boolean = (SystemInfo.getIncNumVal(SystemInfo.INC_EZCAST5G_ENABLE)==0)?false:true;
var LAN_ONLY:Boolean = (SystemInfo.getIncNumVal(SystemInfo.INC_LAN_ONLY)==0)?false:true;
var IS_SNOR:Boolean = (SystemInfo.getIncNumVal(SystemInfo.INC_IS_SNOR_FLASH)==0)?false:true;
trace("EZCASTPRO_MODE: "+EZCASTPRO_MODE);
trace("TEST_ENABLE: "+TEST_ENABLE);
trace("EZCAST_LITE_ENABLE: "+EZCAST_LITE_ENABLE);
trace("EZWILAN_ENABLE: "+EZWILAN_ENABLE);
trace("EZCAST5G_ENABLE: "+EZCAST5G_ENABLE);
trace("LAN_ONLY: "+LAN_ONLY);
trace("IS_SNOR: "+IS_SNOR);
var connectMode:Number = SystemInfo.getConnectMode();
if(SystemInfo.getIncNumVal(SystemInfo.INC_ROUTER_ONLY_ENABLE) == 0 && connectMode == SystemInfo.ROUTER_ONLY){
	connectMode = SystemInfo.ROUTER_ALLOW;
	SystemInfo.setConnectMode(connectMode);
}
var CastCodeEnable:Boolean = (SystemInfo.getCastCodeOnOff() > 0)?true:false;
var castCode:String = "";
if(MainSWF.CastCodeEnable){
	if(SystemInfo.getCastCodeOnOff()==1)
	{
		trace("");
		var n_wifidispay_password:Number = randRange(1000, 9999);
		trace("n_wifidispay_password="+n_wifidispay_password);
		castCode=String(n_wifidispay_password);
	}
	else
	{
		var CodeNum:Number =SystemInfo.getUserDefCastCode();
		if(CodeNum<10)
			castCode="000"+String(CodeNum);
		else if(CodeNum<100)
			castCode="00"+String(CodeNum);
		else if(CodeNum<1000)
			castCode="0"+String(CodeNum);
		else if(CodeNum<10000)
			castCode=String(CodeNum);
		
	}
	
}
trace("castCode: "+castCode);
trace("========= connectMode: "+connectMode);
var AUTO_STANDBY_ENABLE:Boolean = false;
var AUTO_STANDBY_TIME:Number = 60*60;
var cpu_frequency:Number = SystemInfo.get_cpu_frequency();
trace(">>>>[Main] -- TEST_ENABLE: "+TEST_ENABLE+", LAN_ONLY: "+LAN_ONLY+", EZCAST_LITE_ENABLE: "+EZCAST_LITE_ENABLE+", EZWILAN_ENABLE: "+EZWILAN_ENABLE);
var AUTO_GOTO_SETTING:Boolean = false;
var AUTO_GOTO_WIFI_LIST:Boolean = AUTO_GOTO_SETTING;
var AUTO_GOTO_UPGRADE:Boolean = AUTO_GOTO_SETTING;
var AUTO_OPEN_AIRSETUP:Boolean = true;
var UI_ENABLE:Boolean = true;
var START_AUDIO_PATH:String = "/usr/share/ezdata/start.mp3";

var MAIN_SWITCHTO:String = "MAIN_SWITCH_TO";
var EZCAST_SWITCHTO:String = "EZCAST_SWITCH_TO";
var MIRACAST_SWITCHTO:String = "MIRACAST_SWITCH_TO";
var SETTING_SWITCHTO:String = "SETTING_SWITCH_TO";
var EZCASTON:String = "EZCAST_ON";
var EZCASTOFF:String = "EZCAST_OFF";
var MIRACASTON:String = "MIRACAST_ON";
var MIRACASTOFF:String = "MIRACAST_OFF";
var SETTING_CHANGETO:String = "SETTING_CHANGE_TO";

var STATUS_MAIN:String = "STATUS_MAIN";
var STATUS_SETTING:String = "STATUS_SETTING";
var STATUS_EZCAST_ON:String = "STATUS_EZCAST_ON";
var STATUS_EZCAST_OFF:String = "STATUS_EZCAST_OFF";
var STATUS_MIRACAST_ON:String = "STATUS_MIRACAST_ON";
var STATUS_MIRACAST_OFF:String = "STATUS_MIRACAST_OFF";
var HaveEnterWifiSetting:Boolean = false;

var isEZCastlite:Boolean = true;
var isHotspotmode:Boolean = false;

var EZCAST_SWF:String = "ezcast.swf";


var EZMIRR_PROBOX_SWF:String="miracast_probox.swf";
var EZMIRR_LITE_SWF:String = "miracast_lite.swf";
var SETTING_SWF:String = "setting.swf";
var MAIN_BG_SWF:String = "main_customize.swf";
var LOGO_SWF:String = "ctm_main_customize.swf";
var EZMIRR_SWF:String ;
var STREAM_SWF:String;
var MIRR_SWF:String;


var EZCAST_STATUS_NULL:Number = 0;
var EZCAST_STATUS_DISPLAY:Number = 1;
var EZCAST_STATUS_AIRPLAY:Number = 2;
var EZCAST_STATUS_DLNA:Number = 3;


var SCHEMA_OP_MIRACAST:Number = 2;
var SCHEMA_OP_3TO1:Number = 6;
var SCHEMA_OP_AM8252:Number = 7;

var LASTUI_HELP:Number = 0;
var LASTUI_3TO1:Number = 1;
var LASTUI_EZMIRR:Number = 2;
var LASTUI_EZMIRRLITE:Number = 3;

var DEFAULT_MODE_SWF:String = EZMIRR_LITE_SWF;//EZCAST_SWF;
var DEFAULT_MODE_VAL:Number = LASTUI_3TO1;
var SWF_CHANGE_ENABLE:Boolean = true;

var EZCAST5G_TIMEOUT:Number = 60;
// Customize flag
var LOGO_DISPLAY:Boolean 		= true;
var PSK_DISPLAY:Boolean 		= true;
var SSID_DISPLAY:Boolean 		= true;	// if SSID_DISPLAY is false, then the value of PSK_DISPLAY will become invalid
var ICON_DISPLAY:Boolean 		= true;
var TOPBAR_BG_DISPLAY:Boolean 	= true;
var NET_CONNECT_CHECK:Boolean 	= true;
var USER_MANUAL_DISPLAY:Boolean	= true;

// CastCode & MiraCode settings is invalid while USER_MANUAL_DISPLAY is true
var CASTCODE_X:Number			= 1441;
var CASTCODE_Y:Number			= 205;
var CASTCODE_W:Number			= 347;
var CASTCODE_H:Number			= 58;
var CASTCODE_SIZE:Number		= 45;
var CASTCODE_COLOR:Number		= 0x00CCFF;

var MIRACODE_X:Number			= 1441;
var MIRACODE_Y:Number			= 269;
var MIRACODE_W:Number			= 347;
var MIRACODE_H:Number			= 58;
var MIRACODE_SIZE:Number		= 45;
var MIRACODE_COLOR:Number		= 0x0066FF;


var client_switchto_str:String = "";
var isNetDisplayOn:Boolean = false;

var toloadswf:String = "";
var loadInterval:Number = -1;
var displayinterval:Number = -1;
var feedbackenable:Boolean = true;

var enter_wifi_flag:Boolean = false;
var enterVersionCheck:Boolean = false;
var isOtaEnforce:Boolean = false;
var ota_run_flag:Boolean = false;
var VersionChecked:Boolean = false;
var haveNewVersion:Boolean = false;
var isTransferOTA:Boolean = false;
var isStreamOn:Boolean = false;
var isFirstQueryStatus:Boolean = true;		// This status is set to compatible IOS APP
var query_enable:Boolean = true;			// This status is flag if query feedback enable
var query_queue:Boolean = false;			// This status is flag is there a query need feedback
var test_wait_timer:Number = -1;				// seconds
var wifi_test_start:Boolean = false;
var wifi_test_count:Number = 22;			// seconds
var passwdShowEnable:Boolean = true;
var miracastHasStart:Boolean = false;
var isHostnameChange:Boolean = false;
var setPskHidetimer:Number = -1;
var softapInfoShowTimer:Number = -1;

var redDotVal:Number = -1;

var long_key_accept:Boolean =false;
var key_timer_id = -1;
var startAudioTime:Number = -1;
var audioCheckStatus:Number = -1;
var audioopen:Boolean=AudioEngine.Open();
var showIconInDisplayDelayTimer = -1;

var loadSwfEnable:Boolean = true;
var loadSwfStorage:String = "";
var showIpInMiracastLite:Boolean = false;
var lanConnectOk:Boolean = false;
var lanstatus_priv:Number = 0;
var isAutoplayEnable:Boolean = (SystemInfo.getAutoplayEnable() == 0)?false:true;
var MIRACAST_PROBOX_MODE:Boolean = false;
var systemVolume:Number = SystemInfo.getVolume();

var sys_w:Number=SystemInfo.getCurscreenParam(1);
var sys_h:Number=SystemInfo.getCurscreenParam(2);

var bgloader:MovieClipLoader = new MovieClipLoader();
var tbloader:MovieClipLoader = new MovieClipLoader();
if(EZCASTPRO_MODE==8075&&SystemInfo.get_lastui()==2)
{
	
	EZMIRR_SWF = EZMIRR_PROBOX_SWF;
	trace("use miracast_probox.swf");  
}
else
{
	EZMIRR_SWF = "miracast.swf";
	trace("use miracast.swf");
}
swf_init();
resetPngVisible(false);
function isLoadUi():Boolean
{
	if(UI_ENABLE)
		return true;
	return false;
}

function resetPngVisible(val:Boolean)
{
	//trace("garen reset "+val);
	if(currentMC.wifi_display_on){
		//trace("wifi_display_off ");
		currentMC.wifiDisplayStop();
	}
	reset_mc._visible = val;
}

function usermanualVisible(val:Boolean)
{
	step2_mc._visible = val;
}

function getSsid():String
{
	return WifiEngine.getssid();
}

function getPsk():String
{
	return WifiEngine.getpassword();
}

function GetIndexofSelectAP():Number
{
	return WifiEngine.GetIndexofSelectAP();
}

function getconnectedssid():String
{
	return WifiEngine.getconnectedssid();
}

function randRange(min:Number, max:Number):Number {
	var randomNum:Number = Math.floor(Math.random() * (max - min + 1)) + min;
	return randomNum;
}

function get_ConnectMode():Number
{
	if(LAN_ONLY){
		connectMode = SystemInfo.ROUTER_ONLY;
	}else{
		connectMode = SystemInfo.getConnectMode();
		if(is_ezcast_lite()){
			connectMode = SystemInfo.ROUTER_ALLOW;
		}
	}
	return connectMode;
}

var logoCheckInterval:Number = -1;
function checkAndLoadLogo(){
	if(logoCheckInterval > 0){
		clearInterval(logoCheckInterval);
		logoCheckInterval = -1;
	}

	var index_cur = TopBarSWF.get_currentswf_index(currentSWF);
	TopBarSWF.funChange(index);
}

var bgLoadTimer:Number = -1;
function bgLoadCallback()
{
	if(!SSID_DISPLAY)
		PSK_DISPLAY = false;

	if(bgLoadTimer > 0){
		clearInterval(bgLoadTimer);
		bgLoadTimer = -1;
	}
	topbar_mc._visible=false;	
	//bgloader.loadClip("Main_storage_bar.swf",storage_bar_mc);	
	tbloader.loadClip("Main_top_bar.swf", this.topbar_mc);
	topbar_mc._visible=true;	
	if(logoCheckInterval > 0){
		clearInterval(logoCheckInterval);
		logoCheckInterval = -1;
	}
	logoCheckInterval = setInterval(checkAndLoadLogo, 1500);
}

function loadUI()
{
	
	if(isEZCastpro()&&SystemInfo.customerswf_file_exist(LOGO_SWF)==0)//exist customer file 
	{
		bgloader.loadClip(LOGO_SWF, this.main_background_mc);	
		trace("to load bg swf="+LOGO_SWF);
	}
	else 
	{
		bgloader.loadClip(MAIN_BG_SWF, this.main_background_mc);
		trace("to load bg swf="+MAIN_BG_SWF);
	}
	version_txt.text = "ver." + SystemInfo.getOtaLocalVersion();
		
	if(bgLoadTimer > 0){
		clearInterval(bgLoadTimer);
		bgLoadTimer = -1;
	}
	bgLoadTimer = setInterval(bgLoadCallback, 3000);
}

if(isLoadUi())
	loadUI();

SystemInfo.setVolume(systemVolume);

function key_sec_fun()
{
	long_ley_accept = true;
	resetPngVisible(true);
}
	
var swf_sw_str:Array=[
	{idx:1,string:EZCAST_SWITCHTO},
	{idx:2,string:DLNA_SWITCHTO},
	{idx:3,string:MIRACAST_SWITCHTO},
	{idx:4,string:EZAIR_SWITCHTO},
	{idx:5,string:SETTING_SWITCHTO}	
]

function set_schema(){
	var schema_op:Number = -1;
	schema_set_time_count = 0;
	
	if(currentSWF != "ezhelp.swf" && currentSWF != SETTING_SWF){
		if(currentSWF == "miracast.swf"){
			schema_op = SCHEMA_OP_MIRACAST;
		}else{
			schema_op = SCHEMA_OP_3TO1;
		}
		//schema_op = get_currentswf_op(currentSWF);
		SystemInfo.json_set_value(schema_op, NULL);
	}else{
		SystemInfo.json_set_value(-1, NULL);
	}
}

function set_miracast_status(val:Number){
	if(val == 0){
		SystemInfo.json_set_value(1, "NULL");
		trace(":::::::::::: miracast not running :::::::::::::::");
	}else{
		SystemInfo.json_set_value(0, "NULL");
		trace(":::::::::::: miracast running :::::::::::::::");
	}
}

function Deal_CurrentSWF_Out(str:String)
{
	var index=0;
	currentMC.currentswf_out();
			 
}

/* add for loadswf*/
var mcOnTheScene:Number = 0;
var nextMcNum:Number = 0;
var loaderObj:Object = new Object()
loaderObj.onLoadInit = function(target_mc:MovieClip){
	target_mc._width = 1920;  //add by richard
	target_mc._height = 1080;  //add by richard
	swfMove(target_mc, 0, "_x");
}
var mcloader:MovieClipLoader = new MovieClipLoader()
mcloader.addListener(loaderObj)
function swfMove(mc:MovieClip,endNum:Number,pro:String):Void{
	mc.onEnterFrame = function(){
		if((isEZCastpro()) && (EZCASTPRO_MODE==8074)){ // for EZCastPro Projector used
			this[pro] = 0; //remove animation
		}
		else {
			this[pro] += (endNum - this[pro])*0.2;
		}
		if(Math.abs(endNum - this[pro])<1){
			this[pro] = endNum
			delete this.onEnterFrame
			loadswf_callback();
		}
	}
}
function loadswf_callback(){
	trace("loadswf_callback, currentSWF: "+currentSWF);
	if(currentSWF == EZCAST_SWF){
		currentMC.start();
	}
}

function isLoadSwfEnable(str:String):Boolean
{
	if(loadSwfEnable)
		return true;

	trace("*******************************************************");
	trace("*         load SWF is disabled!!! ["+str+"]           *");
	trace("*******************************************************");
	loadSwfStorage = str;
	return false;
}

function loadswf(str:String):Boolean{
	var index=0;
	var index_cur = 0;
	var select_ret:Number = -1;

	if(!isLoadSwfEnable(str))
		return true;
	
	udisk_choose_enter=0;
	trace("currentSWF"+currentSWF);
	trace("str"+str);
	//PreSWF=currentSWF;
	Deal_CurrentSWF_Out(currentSWF);
	if(str!="ezhelp.swf"){
		index=TopBarSWF.get_currentswf_index(str);
		trace("index="+index);

	}else{ //Add By Denny@20130906
		index=1;//richardyu
	}
	TopBarSWF.funChange(index);
	
	if(currentSWF != "ezhelp.swf"){
		index_cur = TopBarSWF.get_currentswf_index(currentSWF);
	}

	if(currentSWF != str && mcOnTheScene <= 1){
		currentSWF = str
		trace("currentSWF="+currentSWF);
		currentMC = MainSWF["load_mc"+nextMcNum]
		if(nextMcNum == 0) {
			load_mc1.unloadMovie()
			if(index > index_cur){
				load_mc0._x = 0;//600;
			}else{
				load_mc0._x = 0;//600;
			}
			load_mc0._y = 0;
			mcloader.loadClip(str, load_mc0)
			MainSWF.nextMcNum = 1
		}else{
			load_mc0.unloadMovie()
			if(index > index_cur){
				load_mc1._x = 0;//600;
			}else{
				load_mc1._x = 0;//600;
			}
			load_mc1._y = 0;
			mcloader.loadClip(str, load_mc1)
			MainSWF.nextMcNum = 0
		}
  
		return true;
	}
	return false;
}

function resend_query(){
	if(loadInterval!=null){
		clearInterval(loadInterval);
		loadInterval = null;
	}

	trace("_______________ resend query!!!");
	query_status_back();
}

function load_swf(){
	trace("load_swf: "+toloadswf);
	if(loadInterval>0){
		clearInterval(loadInterval);
		loadInterval = -1;
	}
	if(toloadswf == ""){
		return;
	}

	if(currentSWF != toloadswf){
		loadswf(toloadswf);
		toloadswf = "";
	}
	query_enable = true;
	if(query_queue){
		if(loadInterval!=null){
			clearInterval(loadInterval);
			loadInterval = null;
		}
		loadInterval=setInterval(resend_query, 200);
	}
	
}

function to_loadswf(str:String){
	trace("to loadswf: "+str);
	trace("---current_status: "+currentMC.current_status);
/*	if((currentMC.current_status == STATUS_DLNA_ON)||(currentMC.current_status ==STATUS_EZAIR_ON)||(currentMC.current_status == STATUS_MIRACAST_ON)){
		trace("Error:Please turn off the DLNA ,EZ Air or MIRACAST");
		return;
	}
*/
	toloadswf = str;
	query_enable = false;
	if(loadInterval>0){
		clearInterval(loadInterval);
		loadInterval = -1;
	}
	loadInterval=setInterval(load_swf, 200);

}

function initSystemVolume(){
	trace(">>>>>>>>>>>>> systemVolume: "+systemVolume);
	if(systemVolume < 7)
		systemVolume = 7;
	
	SystemInfo.setVolume(systemVolume);
}

function audioStatusCheck(){
	var cur_status:Number=AudioEngine.GetState();
	if(cur_status == AudioEngine.AE_STOP || cur_status == AudioEngine.AE_ERROR){
		if(audioCheckStatus>0){
			clearInterval(audioCheckStatus);
			audioCheckStatus = -1;
		}
		AudioEngine.Stop();
		AudioEngine.Close();
		initSystemVolume();
	}
}

function startAudio(){
	if(startAudioTime>0){
		clearInterval(startAudioTime);
		startAudioTime = -1;
	}
	
	var audioOK:Boolean=AudioEngine.SetFile(START_AUDIO_PATH);
	if(audioOK==AE_ERR_NO_ERROR){
		AudioEngine.Play();
		if(audioCheckStatus>0){
			clearInterval(audioCheckStatus);
			audioCheckStatus = -1;
		}
		audioCheckStatus = setInterval(audioStatusCheck, 700);

	}else{
		AudioEngine.Close();
		initSystemVolume();
	}
}

function loadDefaultUi(){
	if(DEFAULT_MODE_SWF == EZMIRR_SWF){
		MiracastHomePage = true;
		MiracastDefault = true;
	}else if(DEFAULT_MODE_SWF == EZMIRR_LITE_SWF){
		isEZCastlite = true;
	}
	swf_init();
	trace("-----------DEFAULT_MODE_SWF = "+DEFAULT_MODE_SWF);
	loadswf(DEFAULT_MODE_SWF);
}

function load_defaultMode(){
	var last_ui:Number = SystemInfo.get_lastui();
	trace("-----------last_ui = "+last_ui);
	//if(last_ui == LASTUI_EZMIRRLITE)
	//	last_ui = 0;
		
	MiracastHomePage = false;
	MiracastDefault = false;
	
	enter_ezcast(1);
	loadDefaultUi();
	
}

function loadfirstswf()
{
	trace("load firstSWF@@@@@@@@@@");
	load_defaultMode();
	if(startAudioTime>0){
		clearInterval(startAudioTime);
		startAudioTime = -1;
	}
	startAudioTime = setInterval(startAudio, 500);

}

function wifidisplayenable(){
	if(displayinterval != null){
		clearInterval(displayinterval);
		displayinterval = null;
	}
	trace("open wifi display!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
	currentMC.wifiDisplayStart();
}

function open_virtural_keyboard(){
	trace("Open virtural keyboard!!!");
	WifiEngine.wifi_feedbackcast("LAUNCH_KEYBOARD");
}

function feedback(str:String){
	WifiEngine.wifi_feedbackcast(str);
}

function sendMsgToApp(str:String){
	WifiEngine.wifi_feedbackcast(str);
}

function query_status_back(){
	if(!query_enable){
		trace("query disable");
		query_queue = true;
		return;
	}
	query_queue = false;
	if(isFirstQueryStatus){
		isFirstQueryStatus = false;
		WifiEngine.wifi_feedbackcast(STATUS_MAIN);
		return;
	}
	if(currentSWF == SETTING_SWF){
		WifiEngine.wifi_feedbackcast(STATUS_SETTING);
	}else if(currentSWF == "ezhelp.swf"){
		WifiEngine.wifi_feedbackcast(STATUS_MAIN);
	}else if(currentSWF == EZCAST_SWF){
		var curDisplayStatus = SystemInfo.wifidisplay_gets_tatus();
		var curstatusstr:String="";
		switch(curDisplayStatus){
			case EZCAST_STATUS_DLNA:
				curstatusstr = STATUS_DLNA_ON;
				break;
			case EZCAST_STATUS_AIRPLAY:
				curstatusstr = STATUS_EZAIR_ON
				break;
			default:
				curstatusstr = STATUS_EZCAST_ON;
				break;
		}
		WifiEngine.wifi_feedbackcast(curstatusstr);
	}else{
		WifiEngine.wifi_feedbackcast(currentMC.check_current_status());
		
	}

}

function statusMiracastStart(){
	miracastHasStart = true;
	systemNotBusy(false);
}

function statusMiracastStop(){
	miracastHasStart = false;
	systemNotBusy(true);
}

function isMirrcastRunning():Boolean
{
	if(currentSWF == MIRR_SWF &&  miracastHasStart)
		return true;
	return false;
}

function skipChange(backStr:String):Boolean
{
	if(!HaveEnterWifiSetting || haveNewVersion){
		if(isMirrcastRunning()){
			return false;
		}
		
		HaveEnterWifiSetting = true;
		isFirstQueryStatus = false;

		if(EZCASTPRO_MODE != 0)
			return false;
		
		if(SystemInfo.isSettingForbid()){
			trace("setting is forbidden!!!");
			return false;
		}
		var select_ret:Number = 0;
		if(!isLanConnect())
			select_ret = WifiEngine.GetIndexofSelectAP();
		if(select_ret < 0){
			if(EZCASTPRO_MODE==8075&&SystemInfo.get_lastui()==2){
				return false;
			}
			if(AUTO_GOTO_WIFI_LIST){
				enter_wifi_flag = true;
				feedbackenable = false;
				feedback(backStr);
				loadswf(SETTING_SWF);
				return true;
				//trace("client_switchto ================ "+client_switchto_str);
			}else if(AUTO_OPEN_AIRSETUP){
				feedback("AIRSETUP_CHANGE_TO");
			}
		}else if(haveNewVersion){
			if(AUTO_GOTO_UPGRADE){
				enterVersionCheck = true;
				feedbackenable = false;
				feedback(backStr);
				loadswf(SETTING_SWF);
				//VersionChecked = true;
				haveNewVersion = false;
				return true;
			}else if(AUTO_OPEN_AIRSETUP){
				feedback("AIRSETUP_CHANGE_TO");
			}
		}
		haveNewVersion = false;
	}
	return false;
}

function test_status_check(){
	var status:String = SystemInfo.get_test_config(SystemInfo.GET_TEST_ENABLE);
	trace("-------------------- status: "+status+" ----------------");
	if(status == SystemInfo.RETURN_VALUE_ENABLE){
		test_wait_timer = -1;
		loadswf("test.swf");
	}
}

function test_status_check_timer(wifistatus:Number){
	if(test_wait_timer >= 0 && test_wait_timer%5 == 0){
		trace("-------------------- test_wait_timer = "+test_wait_timer+" ----------------");
		test_wait_timer--;
		test_status_check();
	}else if(test_wait_timer >= 0){
		trace("-------------------- test_wait_timer = "+test_wait_timer+" ----------------");
		test_wait_timer--;
	}
	if(wifi_test_start && currentSWF == "test.swf"){
		if(wifi_test_count > 0){
			if(wifi_mode_get_from_case==WifiEngine.WIFI_CONCURRENT_CLIENT_GETAUTOIP_OK){
				wifi_test_count = -1;
				trace("wifi test success!!!");
				currentMC.show_wifi_test_result(0);
			}else{
				wifi_test_count --;
			}
		}else if(wifi_test_count == 0){
			wifi_test_count = -1;
			if(wifi_mode_get_from_case==WifiEngine.WIFI_CONCURRENT_CLIENT_GETAUTOIP_OK){
				trace("wifi test success!!!");
				currentMC.show_wifi_test_result(0);
			}else{
				currentMC.show_wifi_test_result(2);
			}
		}
	}
}

function ezsocket_reset(){
	PhotoEngine.clean_help_picture();
	SystemInfo.ezsocket_reset();				

	if(isStreamOn){
		isStreamOn = false;
		showIconInDisplay();
	}
}

function is_ezcast_lite():Boolean {
	return true;
}

function swf_init(){
	if(EZCASTPRO_MODE==8075&&SystemInfo.get_lastui()==2) 
	{
		MIRR_SWF = STREAM_SWF = DEFAULT_MODE_SWF = EZMIRR_SWF=EZMIRR_PROBOX_SWF;
		trace("----------------1");
	}
	else if(is_ezcast_lite()){
		MIRR_SWF = STREAM_SWF = EZMIRR_LITE_SWF;
		trace("----------------2");
		//SystemInfo.setRouterEnable(1, 0);
	}else{
		trace("----------------3");
		MIRR_SWF = EZMIRR_SWF;
		STREAM_SWF = EZCAST_SWF;	
	}
}

function am8252Check():Boolean
{
	if(IS_SNOR)
		return false;
	
	var ezmask = SystemInfo.ezcast_mask();
	if(ezmask == 0){
		DEFAULT_MODE_SWF = EZMIRR_LITE_SWF;
		DEFAULT_MODE_VAL = LASTUI_EZMIRRLITE;
		//SWF_CHANGE_ENABLE = false;
		showIpInMiracastLite = true;
		return true;
	}
	return false;
}

function showQRCodeByWifidisplay()
{
	var qr_x = 404;
	var qr_y = 400;
	var qr_w = 310;
	var qr_h = 310;

	trace("screen: "+sys_w+"x"+sys_h);
	if(sys_w < 1900 && sys_w >= 1260)
	{
		qr_x += 10;
		qr_y += 10;
		qr_w -= 10
	}
	else if(sys_w < 1260)
	{
		qr_x += 22;
		qr_y += 15;
	}
	trace("qr_x: "+qr_x+", qr_y: "+qr_y);
	ezShowPng("/tmp/tmp_qrcode.jpg", qr_x, qr_y, qr_w, qr_h);
	if(redDotVal < 0)
	{
		redDotVal = randRange(0, 100);
		trace("redDotVal from random: "+redDotVal);
		redDotVal %= 10;
		trace("redDotVal: "+redDotVal);
	}
	if(redDotVal > 6)
	{
		var dt_x = 680;
		var dt_y = 390;
		var dt_w = 40;
		var dt_h = 40;
		ezShowPng("/am7x/case/data/red_dot"+(redDotVal-6)+".png", dt_x, dt_y, dt_w, dt_h);
	}
}

function ezCheckHelpPicture():Number
{
	return PhotoEngine.check_help_picture();
}

function ezShowHelp(){
	if(isNetDisplayOn){
		PhotoEngine.show_help_picture();
	}
}

function ezShowPng(path:String, x:Number, y:Number, w:Number, h:Number){
	if(isNetDisplayOn){
		PhotoEngine.show_icon(path, x, y, w, h);
	}
}

function ezShowText(str:String, size:Number, color:Number, x:Number, y:Number){
	if(isNetDisplayOn){
		PhotoEngine.show_text(str, size, color, x, y);
	}
}

function ezCleanRect(x:Number, y:Number, w:Number, h:Number){
	if(isNetDisplayOn){
		PhotoEngine.clean_icon_rect(x, y, w, h);
	}
}

function ezClean(){
	if(isNetDisplayOn){
		PhotoEngine.clean_icon();
	}
}

function netDisplayReady4TopBar(){
	TopBarSWF.flaDisable();
}

function netDisplayOn4TopBar(){
	isNetDisplayOn = true;
//	TopBarSWF.ezRefresh();
	if(showIconInDisplayDelayTimer > 0){
		clearInterval(showIconInDisplayDelayTimer);
		showIconInDisplayDelayTimer = -1;
	}
	showIconInDisplayDelayTimer = setInterval(showIconInDisplayDelay, 1500);

	trace("----------------netDisplayOn4TopBar");
}

function netDisplayOff4TopBar(){
	isNetDisplayOn = false;
	isStreamOn = false;
	TopBarSWF.flaEnable();
}

function mainDirectLinkOlny(){
	TopBarSWF.iconMode(TopBarSWF.ICONMODE_APONLY);
}

function mainRouterAllow(){
	TopBarSWF.iconMode(TopBarSWF.ICONMODE_ROUTERALLOW);
}

function mainSoftapP2p(){
	TopBarSWF.iconMode(TopBarSWF.ICONMODE_AP_P2P);
}

function mainP2pClient(){
	TopBarSWF.iconMode(TopBarSWF.ICONMODE_P2P_CLIENT);
}

function mainSetPskShow(bool:Boolean){
	passwdShowEnable = bool;
	if(bool){
		SystemInfo.setPskHideStatus(0);
	}else{
		SystemInfo.setPskHideStatus(1);
		if(setPskHidetimer >= 0){
			clearInterval(setPskHidetimer);
			setPskHidetimer = -1;
		}
	}
}

function getSystemMode2Top():Number
{
	return SystemInfo.getRouterEnable();
}

function getSystemTime():Number
{
	return SystemInfo.time();
}

function getHostnameChangeStatus():Boolean
{
	return isHostnameChange;
}

function showLanOnlyInfo(){
	var userHostname:String = SystemInfo.getDevName();
	if(devName == "" || devName == "MY_DPF"){
		devName = userHostname;
	}
	if(userHostname != devName){
		isHostnameChange = true;
	}else{
		isHostnameChange = false;
	}
	if(devName == "" || devName == "MY_DPF"){
		TopBarSWF.showPrompt1("DEV: Waiting...");
		return;
	}
	TopBarSWF.showPrompt1("DEV: "+devName);
	
	if(ipForLan != "" && ipForLan.indexOf(".", 2) >= 0){
		TopBarSWF.showPrompt2("IP: " + ipForLan);
	}else{
		TopBarSWF.showPrompt2(" ");
	}
}

function showHostname(){
	if(LAN_ONLY || !topBarShowCompleted)
		return;
	var userHostname:String = SystemInfo.getDevName();
	if(devName == ""){
		devName	= userHostname;
	}
	if(userHostname != devName){
		isHostnameChange = true;
	}else{
		isHostnameChange = false;
	}
	
	if(mySsid != devName||currentSWF == EZMIRR_PROBOX_SWF){
		TopBarSWF.showHostname("(" + devName + ")");
	}else{
		TopBarSWF.showHostname(" ");
	}
}

function lanIpForDisplay():String
{
	return LanEngine.lan_ip_for_display();
}

function showRouterInfoForP2pClient(){
	var str:String = "";
	if(LAN_ONLY)
		return;
	
	var prompt1Ok:Boolean = false;
	
	if(lanstatus_priv != 0){
		ipForLan = lanIpForDisplay();
		if(ipForLan != "" && ipForLan.indexOf(".", 2) > 0){
			TopBarSWF.showPrompt1("http://"+ipForLan);
			prompt1Ok = true;
		}
	}
	
	if(!prompt1Ok && WifiEngine.GetIndexofSelectAP() >= 0){
		//TopBarSWF.showPrompt1("http://"+WifiEngine.GetIPAddress());
		TopBarSWF.showPrompt1(str);
		prompt1Ok = true;
		
	}
	if(prompt1Ok){
		if(softapInfoShowTimer >= 0){
			clearInterval(softapInfoShowTimer);
			softapInfoShowTimer = -1;
		}
		toShowSoftap = false;
		
		if(devName == "")
			devName = SystemInfo.getDevName();
		TopBarSWF.showPrompt2(devName);
		currentMC.ssid_name.text = "";
		topBarShowCompleted = true;
	}
	if(currentSWF == EZCAST_SWF && currentMC.wifi_display_on){
		trace("======== showRouterInfoForP2pClient =========");
		currentMC.wifiDisplayStop();
		currentMC.bgMC.init();
		currentMC.start();
	}
}

function toHidePsk(){
	if(setPskHidetimer >= 0){
		clearInterval(setPskHidetimer);
		setPskHidetimer = -1;
	}

	passwdShowEnable = false;
	psk = "********";
	TopBarSWF.showPSK(psk);
	topBarShowCompleted = true;
	showHostname();
}

function setSoftapPosition(mod:Number)
{
	TopBarSWF.set_ssid_position(mod);
}
function showSoftapInfo(){
	isWaitConnect = false;
	connCount = 0;
	
	if(LAN_ONLY)
		return;
	
	mySsid = WifiEngine.getssid();
	TopBarSWF.showSSID(mySsid);
	
	if(passwdShowEnable){
		psk = WifiEngine.getpassword();
		if(SystemInfo.getPskHideStatus() != 0){
			if(setPskHidetimer >= 0){
				clearInterval(setPskHidetimer);
				setPskHidetimer = -1;
			}
			setPskHidetimer = setInterval(toHidePsk, 10*1000);
		}else{
			topBarShowCompleted = true;
		}
	}else{
		psk = "********";
		topBarShowCompleted = true;
	}
	TopBarSWF.showPSK(psk);
	currentMC.ssid_name.text = WifiEngine.getssid();
	showHostname();
	if(currentSWF == EZCAST_SWF)
		currentMC.bgMC.init();
}

function toShowSoftapInfo(){
	if(softapInfoShowTimer >= 0){
		clearInterval(softapInfoShowTimer);
		softapInfoShowTimer = -1;
	}
	toShowSoftap = false;
	
	if(LAN_ONLY)
		return;

	if(get_ConnectMode() == SystemInfo.ROUTER_ONLY){
		if(WifiEngine.GetIndexofSelectAP() >= 0){
			return;
		}
		
		if(lanstatus_priv != 0){
			ipForLan = lanIpForDisplay();
			if(ipForLan != "" && ipForLan.indexOf(".", 2) > 0){
				return;
			}
		}
	}

	if(currentSWF == EZCAST_SWF && currentMC.wifi_display_on){
		trace("======== toShowSoftapInfo =========");
		currentMC.wifiDisplayStop();
		topBarShowCompleted = false;
		currentMC.start();
	}
	showSoftapInfo();
}

function isHotspotMode():Boolean
{
	return isHotspotmode;
}

function isHotspotReady():Boolean
{
	trace(":::::::::::::::::::: hotspotStatus: "+hotspotStatus);
	if(hotspotStatus == WifiEngine.AS_HOTSPOT_READY || hotspotStatus == WifiEngine.AS_HOTSPOT_SCAN || hotspotStatus == WifiEngine.AS_HOTSPOT_CONNECTING)
		return true;
	return false;
}

function isNetDisplayEnable():Boolean
{
	return (isNetDisplayOn && !isStreamOn);
}

function isEZCastpro():Boolean
{
	if(MainSWF.EZCASTPRO_MODE!=0)
		return true;
	else
		return false;
}

function isAutoPlayEn():Boolean
{
	isAutoplayEnable = (SystemInfo.getAutoplayEnable() == 0)?false:true;
	return isAutoplayEnable;
}

function isDefaultModeSetEnable():Boolean
{
	return SWF_CHANGE_ENABLE;
}

function iconChangeToEZMirrorOn(){
	TopBarSWF.toEZMirrOn();
}

function iconChangeToEZMirrorOff(){
	TopBarSWF.toEZMirrOff();
}

function showIconInDisplayDelay(){
	if(showIconInDisplayDelayTimer > 0){
		clearInterval(showIconInDisplayDelayTimer);
		showIconInDisplayDelayTimer = -1;
	}
	if(SystemInfo.getWifidisplayStatus() == 0){
		isStreamOn = false;
		refreshIcon();
		trace("----------------showIconInDisplayDelay");
	}else{
		trace("Wifi display is on again!!!");
		isStreamOn = true;
		systemNotBusy(false);
	}

}
function showUpgradeIcon(){
	TopBarSWF.iconUpgrade();
}

function hideUpgradeIcon(){
	TopBarSWF.iconUpgradeClean();
}

function refreshLocalback(){
	if(STREAM_SWF == currentSWF){
		showQRCodeByWifidisplay();
		currentMC.refresh();
	}
}

function refreshIcon()
{
	TopBarSWF.ezRefresh();
}

var loadDisableInterval:Number = -1;
var loadDisplayStartTime:Number = 0;
function cleanLoadDisableInterval(){
	if(loadDisableInterval > 0){
		clearInterval(loadDisableInterval);
		loadDisableInterval = -1;
	}
}

function loadSwfDisableCheck(){
	cleanLoadDisableInterval();
	var delayTime:Number = SystemInfo.time() - loadDisplayStartTime;
	if(delayTime < 10){
		loadDisableInterval = setInterval(loadSwfDisableCheck, (10-delayTime)*1000);
		return;
	}
	if(!loadSwfEnable){
		setLoadSwfEnable();
	}
}

function setLoadSwfEnable(){
	//trace("*         setLoadSwfEnable           *");
	cleanLoadDisableInterval();
	loadSwfEnable = true;
	if(loadSwfStorage != ""){
		loadswf(loadSwfStorage);
		loadSwfStorage = "";
	}
}

function setLoadSwfDisable(){
	//trace("*         setLoadSwfDisable           *");
	loadSwfEnable = false;
	cleanLoadDisableInterval();
	loadDisplayStartTime = SystemInfo.time();
	loadDisableInterval = setInterval(loadSwfDisableCheck, 10000);
}



function AM8252_ModeChange(){

	var nextSWF:String;
	if(currentSWF == EZCAST_SWF){
		MiracastHomePage = true;
		MiracastDefault = true;
		nextSWF = EZMIRR_SWF;
	}else{
		MiracastHomePage = false;
		MiracastDefault = false;
		nextSWF = EZCAST_SWF;
	}	
	trace("---AM8252_ModeChange,currentSWF="+currentSWF+" nextSWF="+nextSWF);
	if(nextSWF != currentSWF){
		loadswf(nextSWF);
	}	
}

/*
* switch to 2.4G/5G
*val_t=0(5GHZ),val_t=1(2.4GHZ),val_t=2(Force to change)
*/
function wifi_freq_change(val_t:Number)
{
	var curDongle:Number = WifiEngine.getWifiModelType();
	var curChn:Number = WifiEngine.getCurrentChannel();
	trace("curDongle: "+curDongle+", WifiEngine.WIFI_REALTEK_8811AU: "+WifiEngine.WIFI_REALTEK_8811AU+", curChn: "+curChn);

	if((curDongle == WifiEngine.WIFI_REALTEK_8811AU)&&(val_t == 0)){
		if(curChn > 13)//it's 5g already
			return;
		else
			WifiEngine.setChannelMode(WifiEngine.CHN_RTL_BAND_5G);
	}else if((curDongle == WifiEngine.WIFI_REALTEK_8811AU)&&(val_t == 1)){
		if(curChn <= 13)//it's 2.4g already
			return;
		else
			WifiEngine.setChannelMode(WifiEngine.CHN_RTL_BAND_24G);

	}else if((curDongle == WifiEngine.WIFI_REALTEK_8811AU)&&(val_t == 2)){	
		if(curChn > 13){
			WifiEngine.setChannelMode(WifiEngine.CHN_RTL_BAND_24G);
		}else{
			WifiEngine.setChannelMode(WifiEngine.CHN_RTL_BAND_5G);
		}
	}
	WifiEngine.wifi_disconnect();
	WifiEngine.Stop();

	WifiEngine.restart_wifi_concurrent_mode();
	//WifiEngine.StartScan();
	checkCurChannel();	
}

var show_loading_timer:Number = -1;

function remove_loading() {
	clearInterval(show_loading_timer);
	trace("----loading_mc="+loading_mc);
	loading_mc.removeMovieClip();
}

function show_loading():Void{
	this.attachMovie("loading_mc","loading_mc",this.getNextHighestDepth());
	trace("--loading_mc="+this.loading_mc.loading_mc);
	if(show_loading_timer != -1){
		clearInterval(show_loading_timer);
		show_loading_timer = -1;
	}
	show_loading_timer = setInterval(remove_loading, 3000);	
}



function ProboxDefaultModeChange(){
	if(SWF_CHANGE_ENABLE){
		var last_ui:Number = SystemInfo.get_lastui();
		if(last_ui == MainSWF.LASTUI_EZMIRR){
				swf_init();
			 if(currentSWF==EZCAST_SWF) {
				MiracastHomePage = true;
				MiracastDefault = true;
				//trace("load miracast probox swf DEFAULT_MODE_SWF="+DEFAULT_MODE_SWF);
				loadswf(MIRR_SWF); 
			 }
			 else if(currentSWF == SETTING_SWF){
				currentMC.homepage_changed(); 
			 }
		}
	 	else if(last_ui == MainSWF.LASTUI_3TO1){
			swf_init();
			if(currentSWF==EZMIRR_PROBOX_SWF){
				loadswf(STREAM_SWF);
			 }
			 else if(currentSWF == SETTING_SWF) {
				currentMC.homepage_changed(); 
			 }
		}
	}
}
function DefaultModeChange(){
	if(SWF_CHANGE_ENABLE){
		var firstSWF:String;
		var last_ui:Number = SystemInfo.get_lastui();
		if(last_ui == MainSWF.LASTUI_EZMIRRLITE){
			MiracastHomePage = true;
			MiracastDefault = true;
			if(EZCAST_LITE_ENABLE){
				isEZCastlite = true;
			}
			swf_init();
			firstSWF = MIRR_SWF;
		}else{
			MiracastHomePage = false;
			MiracastDefault = false;
			if(EZCAST_LITE_ENABLE){
				isEZCastlite = false;
			}
			swf_init();
			firstSWF = STREAM_SWF;
		}
		if(currentSWF == SETTING_SWF){
			currentMC.homepage_changed();
		}else if(EZCAST_LITE_ENABLE && (currentSWF == EZCAST_SWF || currentSWF == EZMIRR_LITE_SWF)){
			if(firstSWF != currentSWF){
				loadswf(firstSWF);
			}
		}
	}
}

function setVolumeFromStream(){
	systemVolume = SystemInfo.getVolume();
	var curVolume = SystemInfo.sys_volume_ctrl_get();
	if(curVolume != systemVolume){
		SystemInfo.setVolume(curVolume);
		systemVolume = SystemInfo.getVolume();
	}
}

var sysNotBusyCount:Number = 0;
var isSysNotBusy:Boolean = true;
function systemNotBusy(val:Boolean){
	trace("Set system "+(val?"not busy":"busy"));
	sysNotBusyCountClean();
	isSysNotBusy = val;
}

function systemNotBusyCount(){
	if(!(AUTO_STANDBY_ENABLE && isSysNotBusy && AUTO_STANDBY_TIME > 0))
		return;
	if(sysNotBusyCount%10 == 0){
		if(SystemInfo.getWifidisplayStatus() != 0){
			trace("Wifi display is turn-on!!!");
			isStreamOn = true;
			systemNotBusy(false);
			return;
		}
	}
	
	if(sysNotBusyCount > AUTO_STANDBY_TIME){
		if(SystemInfo.getWifidisplayStatus() != 0){
			trace("Wifi display is turn-on!!!");
			isStreamOn = true;
			systemNotBusy(false);
			return;
		}
		trace("To standby!!!");
		sysNotBusyCount = 0;
		trace("now time: "+SystemInfo.time());
		if(EZWILAN_ENABLE){
			SystemInfo.setLEDBlueOff();
			SystemInfo.setLEDGreenOff();
		}
		if(EZCASTPRO_MODE!=8074) SystemInfo.Standby();
		return;
	}
	sysNotBusyCount++;
}


function standbyInit(){
	sysNotBusyCountClean();
	var standbyTime:Number = SystemInfo.getStandbyTime();
	if(standbyTime > 0){
		AUTO_STANDBY_ENABLE = true;
		AUTO_STANDBY_TIME = standbyTime*60;
	}else{
		AUTO_STANDBY_ENABLE = false;
	}
	trace("AUTO_STANDBY_ENABLE: "+AUTO_STANDBY_ENABLE+", AUTO_STANDBY_TIME: "+AUTO_STANDBY_TIME);
	trace("now time: "+SystemInfo.time());
}

function sysNotBusyCountClean(){
	sysNotBusyCount = 0;
}

var check_stream_status_timer_id:Number=-1;
function check__stream_status()
{
	trace("-------------------------------check__stream_status---------------------- ");
	if(check_stream_status_timer_id > 0){
		clearInterval(check_stream_status_timer_id);
		check_stream_status_timer_id = -1;
	}
	systemNotBusy(true);
	if(SystemInfo.Check_ezmusic_flag())
	SystemInfo.set_stream_play_status(0);
			//showIconInDisplay();
	setVolumeFromStream();
	if(showIconInDisplayDelayTimer > 0){
		clearInterval(showIconInDisplayDelayTimer);
		showIconInDisplayDelayTimer = -1;
		}
	showIconInDisplayDelayTimer = setInterval(showIconInDisplayDelay, 3000);
	if(currentSWF == EZMIRR_LITE_SWF)
		currentMC.streamStop();
	
}

var keyUpCheck:Number = -1;
function key_up_fun()
{
	trace("long_key "+long_ley_accept);
	
	if(keyUpCheck >= 0)
	{
		clearInterval(keyUpCheck);
		keyUpCheck = -1;
	}
	
	if(key_timer_id >= 0)
	{
		clearInterval(key_timer_id);
		key_timer_id = -1;
	}

	if(long_ley_accept) 
	{
		SystemInfo.resetDefaultSetting();
	}
	
	long_ley_accept = false;
}

var currentPath:String="";
var searchType:String="";
var current_index:Number=0;
var keyObject = new Object();
keyObject.onKeyDown = function() {
	var whichKey:Number;
	var ret:Boolean;
	if(!UI_ENABLE)
	{
		if(SystemInfo.mainswf_skip_get_keycode())
			ota_run_flag=true;
	}
	if(ota_run_flag) 
		return;
	whichKey = Key.getCode();
	trace("@@@@whichKeyKey================0x"+whichKey.toString(16));
	set_messeage_back(whichKey);//deal the message
	sysNotBusyCountClean();
	//trace("@@@@backtoMenu========"+backtoMenu);
	switch (whichKey) {

/*		case 0xcc:
			trace("---------Switch 2.4G/5G");
			//show_loading();
			WifiEngine.setAutoConnect(0);
			if(currentSWF != "miracast.swf")
				wifi_freq_change(2);
			break;
		case 0xcf:
			trace("---------Switch to 2.4G");
			//show_loading();
			WifiEngine.setAutoConnect(0);
			if(currentSWF != "miracast.swf")
				wifi_freq_change(1);
			else
				WifiEngine.setChannelMode(WifiEngine.CHN_RTL_BAND_24G);
			break;
		case 0xce:
			trace("---------Switch to 5G");
			//show_loading();
			WifiEngine.setAutoConnect(0);
			if(currentSWF != "miracast.swf")
				wifi_freq_change(0);
			else
				WifiEngine.setChannelMode(WifiEngine.CHN_RTL_BAND_5G);
			break;		*/
		case CSwfKey.SWF_MSG_KEY_EZCAST_SW:
			
			if(currentSWF == "OTA_download.swf")
				break;
			if(SWF_CHANGE_ENABLE){
				if(!skipChange(EZCAST_SWITCHTO) && currentSWF != STREAM_SWF){
					to_loadswf(STREAM_SWF);
				}
			}else{
				if(currentSWF != DEFAULT_MODE_SWF){
					to_loadswf(DEFAULT_MODE_SWF);
				}else{
					skipChange(EZCAST_SWITCHTO);
				}
			}
			feedback(EZCAST_SWITCHTO);
			
			//WifiEngine.wifi_feedbackcast("EZCAST_SWITCH_TO");
			break;
		case CSwfKey.SWF_MSG_KEY_EZCAST_ON:
			trace("ez cast on");
			if(SWF_CHANGE_ENABLE){
				if(STREAM_SWF != currentSWF){
					trace("goto ezcast.swf");
					feedbackenable = false;
					loadswf(STREAM_SWF);
					if(displayinterval!=null){
						clearInterval(displayinterval);
						displayinterval = null;
					}
					//displayinterval=setInterval(wifidisplayenable, 1500);
				}
			}else{
				if(currentSWF != DEFAULT_MODE_SWF){
					to_loadswf(DEFAULT_MODE_SWF);
				}
			}
			feedback(EZCASTON);
			
			break;
		case CSwfKey.SWF_MSG_KEY_EZCAST_OFF:
			feedback(EZCASTOFF);
			break;
			/*
			if("ezcast.swf" == currentSWF){
				trace("ez cast off");
				currentMC.wifiDisplayStop();
			}
			
			break;
			*/
		case CSwfKey.SWF_MSG_KEY_DLNA_SW:
			if(SWF_CHANGE_ENABLE){
				if(!skipChange(DLNA_SWITCHTO) && currentSWF != STREAM_SWF)
					to_loadswf(STREAM_SWF);
			}else{
				if(currentSWF != DEFAULT_MODE_SWF){
					to_loadswf(DEFAULT_MODE_SWF);
				}
			}
			feedback(DLNA_SWITCHTO);

			break;
		case CSwfKey.SWF_MSG_KEY_DLNA_ON:
			feedback(DLNAON);
			break;
			trace("ez dlna on");
			if(currentSWF == "dlna.swf"){
				currentMC.function_on();
			}
			//WifiEngine.wifi_feedbackcast("DLNA_ON");
			break;
		case CSwfKey.SWF_MSG_KEY_DLNA_OFF:
			trace("ez wifi display off");
			feedback(DLNAOFF);
			if(currentSWF == EZCAST_SWF || currentSWF == EZMIRR_LITE_SWF){
				ezsocket_reset();
			}
			break;
		case CSwfKey.SWF_MSG_KEY_MIRACAST_SW:
			if(SWF_CHANGE_ENABLE){
				if(!skipChange(MIRACAST_SWITCHTO))
					to_loadswf(MIRR_SWF);
			}else{
				if(currentSWF != DEFAULT_MODE_SWF){
					to_loadswf(DEFAULT_MODE_SWF);
				}
			}
			feedback(MIRACAST_SWITCHTO);

			break;
		case CSwfKey.SWF_MSG_KEY_MIRACAST_ON:
			trace("ez miracast on");
			if(currentSWF == EZMIRR_SWF){
				currentMC.function_on();
			}
			break;
		case CSwfKey.SWF_MSG_KEY_MIRACAST_OFF:
			trace("ez miracast off");
			if(currentSWF == EZMIRR_SWF){
				currentMC.function_off();
				to_loadswf(STREAM_SWF);
			}
			break;
		case CSwfKey.SWF_MSG_KEY_EZAIR_SW:
			if(SWF_CHANGE_ENABLE){
				if(!skipChange() && currentSWF != STREAM_SWF)
					to_loadswf(STREAM_SWF);
			}else{
				if(currentSWF != DEFAULT_MODE_SWF){
					to_loadswf(DEFAULT_MODE_SWF);
				}
			}
			feedback(EZAIR_SWITCHTO);
			
			break;
		case CSwfKey.SWF_MSG_KEY_EZAIR_ON:
			feedback(EZAIRON);
			break;
			trace("ez EZ_Air on");
			if(currentSWF == "EZ_Air.swf"){
				currentMC.function_on();
			}
			break;
		case CSwfKey.SWF_MSG_KEY_EZAIR_OFF:
			trace("ez wifi display off");
			feedback(EZAIROFF);
			if(currentSWF == EZCAST_SWF || currentSWF == EZMIRR_LITE_SWF){
				ezsocket_reset();
			}
			break;
		case CSwfKey.SWF_MSG_KEY_SETTING_SW:
			
			if(currentSWF == "OTA_download.swf")
				break;
			if(SystemInfo.isSettingForbid()){
				trace("setting is forbidden!!!");
				return false;
			}
			//if(!skipChange(EZAIR_SWITCHTO))
			//	to_loadswf(SETTING_SWF);
			
			break;
		case CSwfKey.SWF_MSG_KEY_MAIN_SW:
			trace("ez wifi display off");
			if(currentSWF == STREAM_SWF){
				ezsocket_reset();
			}
			feedback(MAIN_SWITCHTO);

			break;
		case CSwfKey.SWF_MSG_KEY_QUERY_STATUS:
			trace("SWF_MSG_KEY_QUERY_STATUS");
			query_status_back();
			
			break;
		case CSwfKey.SWF_MSG_KEY_W:
			trace("SWF_MSG_KEY_W");
			long_ley_accept = false;
			if(key_timer_id >= 0)
			{
				clearInterval(key_timer_id);
				key_timer_id = -1;
			}
			key_timer_id = setInterval(key_sec_fun, 5000);

			break;
		/*case CSwfKey.SWF_MSG_KEY_Z:  
			SystemInfo.setCurLanguage(0);
			SystemInfo.storeConfig();
			if(EZCASTPRO_MODE!=8074)SystemInfo.setlangdisautostatus(0);
			WifiEngine.Wifidonglechange(WifiEngine.CLOSE_WIFI_PROCESS); 
			WifiEngine.Stop();
			SystemInfo.deleteWifiConf();	
			SystemInfo.deleteEZCastConf();	
			SystemInfo.reset_userpassword();
			//trace("++++++++++++1730");
			if(SystemInfo.Check_ezmusic_flag()){
				SystemInfo.deleteEZmusicConf();	
			}
			SystemInfo.reboot_system();
			break;
*/
		case CSwfKey.SWF_MSG_KEY_V:
			//trace("SWF_MSG_KEY_V "+long_ley_accept);
			if(keyUpCheck >= 0)
			{
				//trace("V clear "+keyUpCheck);
				clearInterval(keyUpCheck);
				keyUpCheck = -1;
			}
			keyUpCheck = setInterval(key_up_fun, 2000);
			
			break;
		case CSwfKey.SWF_MSG_KEY_VIEW:
			if(STREAM_SWF == currentSWF || MIRR_SWF == currentSWF){
				currentMC.showHelpPhoto();
			}
			break;
		case CSwfKey.SWF_MSG_KEY_APPINFO:
			langdisautostatusGet();
			language_string = SystemInfo.langautoget();
			if("NULL" != language_string && !langdisautostatus){
				ret = setLanguageString(language_string);
				if(ret){
					setLanguageSWFName("ezcast", localeCallback);
					currentMC.language_change();
					//currentMC.localeCallback();
					SystemInfo.setCurLanguage(_global.MainSWF.language);
					SystemInfo.storeConfig();
				}
			}
			break;
		case CSwfKey.SWF_MSG_KEY_STREAMSTART:
			trace("KEY Stream start");
			trace("-----check_stream_status_timer_id="+check_stream_status_timer_id);
			if(check_stream_status_timer_id > 0){
				clearInterval(check_stream_status_timer_id);
				check_stream_status_timer_id = -1;
			}
			else
			{
				if(showIconInDisplayDelayTimer > 0){
				clearInterval(showIconInDisplayDelayTimer);
				showIconInDisplayDelayTimer = -1;
				}
				systemNotBusy(false);
				if(SystemInfo.Check_ezmusic_flag())
					SystemInfo.set_stream_play_status(1);
				isStreamOn = true;
				if(currentSWF == EZMIRR_LITE_SWF)
					currentMC.streamStart();
			}
			
			break;
		case CSwfKey.SWF_MSG_KEY_STREAMSTOP:
			
			trace("-----KEY Stream stop");
			trace("-----check_stream_status_timer_id="+check_stream_status_timer_id);
			if(check_stream_status_timer_id==-1)
				check_stream_status_timer_id = setInterval(check__stream_status,3000);
			break;
			/*//pro box key long_press 
		case CSwfKey.SWF_MSG_KEY_R:
			trace("long");
			//key_longpress=true;
			key_shortpress=false;
			timer_key_count++;
			
			//trace("longpress");
			break;
			*/
		case CSwfKey.SWF_MSG_KEY_S:
			trace("KEY_S");
			key_up_fun();
			break;
		case CSwfKey.SWF_MSG_KEY_USB_HID_IN:			
			trace("SWF_MSG_KEY_USB_HID_IN");
			mouse_in_flag=true;
			if(EZCAST_SWF == currentSWF){
				currentMC.show_mouse();
			}
			break;
		case CSwfKey.SWF_MSG_KEY_USB_HID_OUT:			
			trace("SWF_MSG_KEY_USB_HID_OUT");
			mouse_in_flag=false;
			if(EZCAST_SWF == currentSWF){
				currentMC.clean_mouse();
			}
			break;
		case CSwfKey.SWF_MSG_KEY_EZLAUNCHER_IN:			
			trace("---SWF_MSG_KEY_EZLAUNCHER_IN");
			if(EZCAST_SWF == currentSWF){
				currentMC.show_ezlauncher_icon();
			}
			break;
		case CSwfKey.SWF_MSG_KEY_EZLAUNCHER_OUT:			
			trace("---SWF_MSG_KEY_EZLAUNCHER_OUT");
			if(EZCAST_SWF == currentSWF){
				currentMC.clean_ezlauncher_icon();
			}
			break;
		case CSwfKey.SWF_MSG_KEY_MEDIA_CHANGE:			
   			//process sdcard in and out 

			var card_flag=SystemInfo.checkCardStatus();
			//trace("------------mount memory to http server------------ ");
			//VideoEngine.mount_memory_tohttp();
			
			if((card_flag==0)&&(oldsdsta==1))
			{
				oldsdsta=0;
				trace("-------sdcard  out------");
				SystemInfo.acceptHotplugInfo(SystemInfo.SYS_MEDIA_SD,0);
				trace("-------sdcard  umount------");
				SystemInfo.umount_card();
				trace("-------creat_default_info_file oldusbsta------"+oldusbsta);
			}
			
			if((card_flag==1)&&(card_flag!=oldsdsta))
			{
				oldsdsta=1;
				trace("-------sdcard  in------");
				SystemInfo.acceptHotplugInfo(SystemInfo.SYS_MEDIA_SD,1);
				//VideoEngine.umountcard();
				//change card to httpserver directory
				trace("-------mount card to http directory---------");
				SystemInfo.mount_card_tohttp();
				trace("-------mount card to http directory---------");
				
			}
			
				
			//process usb in and out 
			io_flag=SystemInfo.checkUsb1Status();
			trace("------------------ io_flag = "+io_flag);
			if((io_flag==0)&&(oldusbsta==1))
			{
				oldusbsta=0;
				trace("-------usb  out------");
			    SystemInfo.acceptHotplugInfo(SystemInfo.SYS_MEDIA_UDISK1,0);
			    SystemInfo.umount_usb();
			}
			if((io_flag==1)&&(io_flag!=oldusbsta))
			{
				oldusbsta=1;
				trace("-------usb  in------");
				SystemInfo.acceptHotplugInfo(SystemInfo.SYS_MEDIA_UDISK1,1);
				VideoEngine.umountusb1();
				//change usb to httpserver directory
				trace("-------mount usb to http directory---------");
				SystemInfo.mount_usb_tohttp();
			}
			
			
			if(TEST_ENABLE){
				//usb1	first in flag;
				firsttime++;
				var io_flag_tmp1 = SystemInfo.checkUsb1Status();
				var io_flag_tmp2 = SystemInfo.checkUsb2Status();
				trace( "io_flag_tmp1"+ io_flag_tmp1);
				trace( "io_flag_tmp2"+ io_flag_tmp2);
				trace( "oldusbsta1"+ oldusbsta1);
				trace( "oldusbsta2"+ oldusbsta2);
				//usb1 or usb2  device in and  last state is out 
				if(((io_flag_tmp1 == 1) &&(io_flag_tmp1!=oldusbsta1))|| ((io_flag_tmp2 == 1)&& (io_flag_tmp2!=oldusbsta2)) )
					io_flag = 1;
				else
					io_flag = 0;
				oldusbsta2=io_flag_tmp2;
				oldusbsta1=io_flag_tmp1;
				trace("------------------ io_flag = "+io_flag);
				if(io_flag==0)
				{
					//trace(" ");
					//usb plug out
					test_wait_timer = -1;
					//SystemInfo.acceptHotplugInfo(SystemInfo.SYS_MEDIA_UDISK1,0);
					SystemInfo.acceptHotplugInfo(SystemInfo.SYS_MEDIA_UDISK2,0);
					if(currentSWF == "test.swf")
						to_loadswf(EZCAST_SWF);
						
				}
				else
				{
					trace("Entering test mode-------------------------------");
					test_mode=0;
					//usb plug in 
					if(ip_flag_tmp1 == 1)
						//SystemInfo.acceptHotplugInfo(SystemInfo.SYS_MEDIA_UDISK1,1);
					if(ip_flag_tmp2 == 1)
						SystemInfo.acceptHotplugInfo(SystemInfo.SYS_MEDIA_UDISK2,1);
					if(TEST_ENABLE){
						test_wait_timer = 15;
					}
				}
			}
			
			
		
			
			break;
		case CSwfKey.SWF_MSG_KEY_SETPSK:
			psk = WifiEngine.getpassword();	
			softap_info._visible = false;
			//TopBarSWF.showPSK(psk);
			break;
		case CSwfKey.SWF_MSG_KEY_OTA_DOWNLOAD:
			usermanualVisible(false);
			to_loadswf("OTA_download.swf");
			break;
		case CSwfKey.SWF_MSG_KEY_OTA_FROM_URL:
			if(SystemInfo.ota_upgradeFromApp() == 0)
				to_loadswf("OTA_download.swf");
			break;
		case CSwfKey.SWF_MSG_KEY_MODE_CHANGE:
			if(EZCASTPRO_MODE==8075)
				ProboxDefaultModeChange();
			else
				DefaultModeChange();
			break;
		case CSwfKey.SWF_MSG_KEY_EXT_POWER:
			if(EZCASTPRO_MODE!=8074) SystemInfo.Standby();
			break;	
		case CSwfKey.SWF_MSG_KEY_F24://0x87 IR ESC richardyu 100114
			if(EZCASTPRO_RUN){
				trace("EZCAST PRO -> mainmenu...");
				to_loadswf("mainmenu.swf");
			    enter_ezcast(0);
			}
			break;
		case CSwfKey.SWF_MSG_KEY_M:
			if(currentSWF != "mainmenu.swf"){
				if((currentMC.current_status == STATUS_DLNA_ON)||(currentMC.current_status ==STATUS_EZAIR_ON)||(currentMC.current_status == STATUS_MIRACAST_ON)){
					trace("Error:Please turn off the DLNA ,EZ Air or MIRACAST");
					return;
				}
				if(EZCASTPRO_RUN){
					trace("EZCastPro Back to mainmenu!");
					
					to_loadswf("mainmenu.swf");
					enter_ezcast(0);
				}
			}
			break;
		/**
		* EZCastProProjector used
		**/
		case CSwfKey.SWF_MSG_KEY_EXT_PHOTO: //Scaler sends UART CMD to 8250 to switche swf page to EZmedia
			if(EZCASTPRO_MODE==8074) { // For EZCastProProjector used only.
				Customization.update_current_swf(1);
				if(currentSWF != "ezmedia.swf"){
					currentMC.sourceswf_unload();
					trace("currentSWF=" + currentSWF + " -> " + "ezmedia.swf" + " EZCASTPRO_RUN="+EZCASTPRO_RUN);
					if(EZCASTPRO_RUN){
						trace("EZCAST PRO -> ezmedia.swf...");
						to_loadswf("ezmedia.swf");
						enter_ezcast(0);
					}
					else{
						trace("-----> ezmedia");	
						loadswf("ezmedia.swf");
					}
				}
			}
			break;
		case CSwfKey.SWF_MSG_KEY_EXT_MUSIC: //Scaler sends UART CMD to 8250 to switche swf page to EZCastPro
			if(EZCASTPRO_MODE==8074) { // For EZCastProProjector used only.
				Customization.update_current_swf(2);
				if(currentSWF != "ezcast.swf"){
					currentMC.sourceswf_unload();
					trace("currentSWF=" + currentSWF + " -> " + "ezcast.swf" + " EZCASTPRO_RUN="+EZCASTPRO_RUN);
					if(EZCASTPRO_RUN == 0){
						trace("mainmenu - > EZCastPro...");
						to_loadswf("ezcast.swf");
						enter_ezcast(1);
					}
					else{
						loadswf("ezcast.swf");
					}
				}
			}
			break;
		case CSwfKey.SWF_MSG_KEY_EXT_VIDEO: //Scaler sends UART CMD to 8250 to switche swf page to ConnectPC
			if(EZCASTPRO_MODE==8074) { // For EZCastProProjector used only.
				Customization.update_current_swf(3);
				if(currentSWF != "connectPC.swf"){
					currentMC.sourceswf_unload();
					trace("currentSWF=" + currentSWF + " -> " + "connectPC.swf" + " EZCASTPRO_RUN="+EZCASTPRO_RUN);
					if(EZCASTPRO_RUN){
						trace("EZCAST PRO -> mainmenu...");
						to_loadswf("connectPC.swf");
						enter_ezcast(0);
					}
					else{
						loadswf("connectPC.swf");
					}
				}
			}
			break;
		case CSwfKey.SWF_MSG_KEY_EXT_VIEW: //Scaler sends UART CMD to 8250 to switche swf page to EZmedia Setting System
			if(EZCASTPRO_MODE==8074) { // For EZCastProProjector used only.
				Customization.update_current_swf(3);
				if(currentSWF != "set_sys.swf"){
					currentMC.sourceswf_unload();
					trace("currentSWF=" + currentSWF + " -> " + "set_sys.swf" + " EZCASTPRO_RUN="+EZCASTPRO_RUN);
					if(EZCASTPRO_RUN){
						trace("EZCAST PRO -> mainmenu...");
						to_loadswf("set_sys.swf");
						enter_ezcast(0);
					}
					else{
						loadswf("set_sys.swf");
					}
				}
			}
			break;
		case CSwfKey.SWF_MSG_KEY_PLUG_PLAY:
			if((currentSWF == EZCAST_SWF || currentSWF == EZMIRR_PROBOX_SWF || currentSWF == EZMIRR_LITE_SWF) && MainSWF.isNetDisplayOn)
			{
				currentMC.wifiDisplayStop();
				trace("Stop netdisplay success.\n");
			}
			SystemInfo.PlugPlay(true);
			break;
		case CSwfKey.SWF_MSG_KEY_PLUG_STOP:
			SystemInfo.PlugPlay(false);
			if((currentSWF == EZCAST_SWF || currentSWF == EZMIRR_PROBOX_SWF || currentSWF == EZMIRR_LITE_SWF) && !MainSWF.isNetDisplayOn)
			{
				trace("Starting netdisplay.\n");
				if(currentSWF == EZMIRR_PROBOX_SWF)
					currentMC.wifiDisplayStart_Deley();
				else
					currentMC.start();
			}
			break;
		default:
			break;
		
	}
};

var mediakeyObject = new Object();
mediakeyObject.onKeyDown = function() {
	var whichKey:Number;
	var ret:Boolean;
	whichKey = Key.getCode();
	trace("@@@@Media Key================0x"+whichKey.toString(16));
	switch (whichKey) {
		case CSwfKey.SWF_MSG_KEY_F24:
			if(backMenu){
				loadswf("ezmedia.swf");
			}
			break;
		case CSwfKey.SWF_MSG_KEY_MEDIA_CHANGE:
			trace("=====MEDIA CHANGE=====");
			//process sdcard in and out 

			var card_flag=SystemInfo.checkCardStatus();
			//trace("------------mount memory to http server------------ ");
			//VideoEngine.mount_memory_tohttp();
			
			if((card_flag==0)&&(oldsdsta==1))
			{
				oldsdsta=0;
				trace("-------sdcard  out------");
				SystemInfo.acceptHotplugInfo(SystemInfo.SYS_MEDIA_SD,0);
				trace("-------sdcard  umount------");
				SystemInfo.umount_card();
			}
			
			if((card_flag==1)&&(card_flag!=oldsdsta))
			{
				oldsdsta=1;
				trace("-------sdcard  in------");
				SystemInfo.acceptHotplugInfo(SystemInfo.SYS_MEDIA_SD,1);
				//VideoEngine.umountcard();
				//change card to httpserver directory
				trace("-------mount card to http directory---------");
				SystemInfo.mount_card_tohttp();
				
			}
			
				
			//process usb in and out 
			io_flag=SystemInfo.checkUsb1Status();
			trace("------------------ io_flag = "+io_flag);
			if((io_flag==0)&&(oldusbsta==1))
			{
				oldusbsta=0;
				trace("-------usb  out------");
			    SystemInfo.acceptHotplugInfo(SystemInfo.SYS_MEDIA_UDISK1,0);
			     SystemInfo.umount_usb();
				
				
			}
			if((io_flag==1)&&(io_flag!=oldusbsta))
			{
				oldusbsta=1;
				trace("-------usb  in------");
				SystemInfo.acceptHotplugInfo(SystemInfo.SYS_MEDIA_UDISK1,1);
				VideoEngine.umountusb1();
				//change usb to httpserver directory
				trace("-------mount usb to http directory---------");
				SystemInfo.mount_usb_tohttp();
				//outputmode_change();
			}
			
			
			if(TEST_ENABLE){
				//usb1	first in flag;
				firsttime++;
				var io_flag_tmp1 = SystemInfo.checkUsb1Status();
				var io_flag_tmp2 = SystemInfo.checkUsb2Status();
				trace( "io_flag_tmp1"+ io_flag_tmp1);
				trace( "io_flag_tmp2"+ io_flag_tmp2);
				trace( "oldusbsta1"+ oldusbsta1);
				trace( "oldusbsta2"+ oldusbsta2);
				//usb1 or usb2  device in and  last state is out 
				if(((io_flag_tmp1 == 1) &&(io_flag_tmp1!=oldusbsta1))|| ((io_flag_tmp2 == 1)&& (io_flag_tmp2!=oldusbsta2)) )
					io_flag = 1;
				else
					io_flag = 0;
				oldusbsta2=io_flag_tmp2;
				oldusbsta1=io_flag_tmp1;
				trace("------------------ io_flag = "+io_flag);
				if(io_flag==0)
				{
					//trace(" ");
					//usb plug out
					test_wait_timer = -1;
					//SystemInfo.acceptHotplugInfo(SystemInfo.SYS_MEDIA_UDISK1,0);
					SystemInfo.acceptHotplugInfo(SystemInfo.SYS_MEDIA_UDISK2,0);
					if(currentSWF == "test.swf")
						to_loadswf(EZCAST_SWF);
						
				}
				else
				{
					trace("Entering test mode-------------------------------");
					//usb plug in 
					if(ip_flag_tmp1 == 1)
						//SystemInfo.acceptHotplugInfo(SystemInfo.SYS_MEDIA_UDISK1,1);
					if(ip_flag_tmp2 == 1)
						SystemInfo.acceptHotplugInfo(SystemInfo.SYS_MEDIA_UDISK2,1);
					if(TEST_ENABLE){
						test_wait_timer = 15;
					}
				}
			}
			break;
		case CSwfKey.SWF_MSG_KEY_M:
			if(currentSWF != "mainmenu.swf"){
				trace("Back to mainmenu!");
				_global.ezmediaDeviceIndex = null;
				_global.ezmediaFeatureIndex = null;
				currentMC.sourceswf_unload();
				loadswf("mainmenu.swf");
			}
			break;
		
		/**
		* EZCastProProjector used
		**/
		case CSwfKey.SWF_MSG_KEY_EXT_PHOTO: //Scaler sends UART CMD to 8250 to switche swf page to EZmedia
			if(EZCASTPRO_MODE==8074) { // For EZCastProProjector used only.
				Customization.update_current_swf(1);
				if(currentSWF != "ezmedia.swf"){
					currentMC.sourceswf_unload();
					trace("currentSWF=" + currentSWF + " -> " + "ezmedia.swf" + " EZCASTPRO_RUN="+EZCASTPRO_RUN);

					/*if(loadsourceInterval>0){
						clearInterval(loadsourceInterval);
						loadsourceInterval = -1;
					}
					loadsourceInterval = setInterval(load_by_source, 1000);
					*/
					if(EZCASTPRO_RUN){
						trace("EZCAST PRO -> ezmedia.swf...");
						to_loadswf("ezmedia.swf");
						enter_ezcast(0);
					}
					else{
						trace("-----> ezmedia");	
						loadswf("ezmedia.swf");
					}
				}
			}
			break;
		case CSwfKey.SWF_MSG_KEY_EXT_MUSIC: //Scaler sends UART CMD to 8250 to switche swf page to EZCastPro
			if(EZCASTPRO_MODE==8074) { // For EZCastProProjector used only.
				Customization.update_current_swf(2);
				if(currentSWF != "ezcast.swf"){
					currentMC.sourceswf_unload();
					trace("currentSWF=" + currentSWF + " -> " + "ezcast.swf" + " EZCASTPRO_RUN="+EZCASTPRO_RUN);
					if(EZCASTPRO_RUN == 0){
						trace("mainmenu - > EZCastPro...");
						to_loadswf("ezcast.swf");
						enter_ezcast(1);
					}
					else{
						loadswf("ezcast.swf");
					}
				}
			}
			break;
		case CSwfKey.SWF_MSG_KEY_EXT_VIDEO: //Scaler sends UART CMD to 8250 to switche swf page to ConnectPC
			if(EZCASTPRO_MODE==8074) { // For EZCastProProjector used only.
				Customization.update_current_swf(3);
				if(currentSWF != "connectPC.swf"){
					currentMC.sourceswf_unload();
					trace("currentSWF=" + currentSWF + " -> " + "connectPC.swf" + " EZCASTPRO_RUN="+EZCASTPRO_RUN);
					if(EZCASTPRO_RUN){
						trace("EZCAST PRO -> mainmenu...");
						to_loadswf("connectPC.swf");
						enter_ezcast(0);
					}
					else{
						loadswf("connectPC.swf");
					}
				}
			}
			break;
		case CSwfKey.SWF_MSG_KEY_EXT_VIEW: //Scaler sends UART CMD to 8250 to switche swf page to EZmedia Setting System
			if(EZCASTPRO_MODE==8074) { // For EZCastProProjector used only.
				Customization.update_current_swf(3);
				if(currentSWF != "set_sys.swf"){
					currentMC.sourceswf_unload();
					trace("currentSWF=" + currentSWF + " -> " + "set_sys.swf" + " EZCASTPRO_RUN="+EZCASTPRO_RUN);
					if(EZCASTPRO_RUN){
						trace("EZCAST PRO -> mainmenu...");
						to_loadswf("set_sys.swf");
						enter_ezcast(0);
					}
					else{
						loadswf("set_sys.swf");
					}
				}
			}
			break;
		case CSwfKey.SWF_MSG_KEY_LOAD_LANGUAGE:		
			var indexLanguage:Number = Customization.get_scaler_language() ;
			trace( "[inEZMedia] Scaler requires to change Language to " + indexLanguage + " from uartcom" ) ;
			_global.MainSWF.language=indexLanguage;
			SystemInfo.Set_language_index(_global.MainSWF.language);
			langdisautostatusGet();
			currentMC.language_change();
			break;
		default:
			break;
	}
}

function key_longpress_check()
{
	trace("key_longpress_check!!");
	/*
	if(0<timer_key_count&&timer_key_count<10){
		key_shortpress=true;//long press time <10
		key_longpress=false;
		clearInterval(timer_longcheck);
		trace("long press time <10");
		outputmode_change();
	}*/
	/*//another case,no use.
	else if(timer_key_count>10 && timer_key_count<50){//press time  1~5,not treate as short press nor long press
		key_shortpress=false;
		key_longpress=false;
	}
	*/
	/*
	else if(timer_key_count>50){ //treate as long press
		key_longpress=true;
		key_shortpress=false;
		trace("---swf key:Reset Key long press ,system will reboot!!!---");
		clearInterval(timer_longcheck);
		SystemInfo.reboot_system();
	}
	*/
		long_ley_accept=true;

	
}

function key_shortpress_check()
{
	if(key_shortpress){
		trace("short press!!");
		outputmode_change();
		clearInterval(timer_shortcheck);
	}
}
function outputmode_change()
{
	trace("---swf key:Key short press ,set outputmode---");
	//output_mode=SystemInfo.read_edid_bin_file(0);
	//output_mode=output_mode-1;
	/*
	output_mode++;
	if(output_mode>2)
		output_mode=0;
	switch (output_mode){
		case 0:
			trace("current output_mode HDMI");
			SystemInfo.setOutputMode(1);
			var cur_res =SystemInfo.read_edid_bin_file(1);
			//if not pro, donot supprot 1920X1080
			trace("cur_res from edid_bin "+cur_res);
			SystemInfo.setHDMIMode(cur_res);
			SystemInfo.creat_edid_bin_file(1,cur_res,0,0);
			SystemInfo.save_outputmode(1,cur_res);
			//SystemInfo.reboot_system();
			break;

		case 1:
			trace("current output_mode VGA");
			SystemInfo.setOutputMode(8);
			SystemInfo.setHDMIMode(SystemInfo.VGA_1920_1080);
			SystemInfo.creat_edid_bin_file(0,0,1,SystemInfo.VGA_1920_1080);
			SystemInfo.save_outputmode(8,SystemInfo.VGA_1920_1080);
			//SystemInfo.reboot_system();
			break;
		case 2:
			trace("current output_mode CVBS");
			SystemInfo.setOutputMode(2);
			SystemInfo.setHDMIMode(SystemInfo.CVBS_PAL);
			SystemInfo.save_outputmode(2,SystemInfo.CVBS_PAL);
			//SystemInfo.reboot_system();
			break;
	}	
	*/
}

function enter_ezcast(ctl:Number){//richardyu 100114
	switch(ctl){
		case 1:
			CustomSWF.custom_key_onoff(0);	
			Key.removeListener(mediakeyObject);
			Key.addListener(keyObject);
			EZCASTPRO_RUN = 1;		
			TopBarSWF.toEZCastOn();
			break;
		case 0:
			Key.removeListener(keyObject);
			Key.addListener(mediakeyObject);
			CustomSWF.custom_key_onoff(1);	
			EZCASTPRO_RUN = 0;		
			TopBarSWF.toEZCastOff();
			break;
	}
}

function check_1080p60_and_disable(){
	//if(!(EZCAST5G_ENABLE || EZWILAN_ENABLE) || cpu_frequency < 552){
		if(SystemInfo.read_edid_bin_file(0)==1){
			var cur_res = SystemInfo.read_edid_bin_file(1);
			if(cur_res == SystemInfo.FMT_1920x1080_60P){
				trace("Do not support 1920 x 1080 60p, and turn to 1280 x 720 60p!!!");
				SystemInfo.creat_edid_bin_file(1,SystemInfo.FMT_1650x750_1280x720_60P,0,0);
				SystemInfo.reboot_system();
			}
		}
	//}
}
if(EZCASTPRO_MODE==0){
	//check_1080p60_and_disable();			// Do not support 1920 x 1080 60p, and if resolution is 1920 x 1080 60p, then change to 1280 x 720 60p.
}
//SystemInfo.setRouterEnable(SystemInfo.getRouterEnable()); //after reboot, read stored value and set it into ezrmote

function langdisautostatusGet(){
	if(EZCASTPRO_MODE!=8074)langdisautostatus = SystemInfo.getlangdisautostatus();
	else langdisautostatus = 1;
}
function check_factory_test()
{
	var factory_test_status=SystemInfo.sysinfo_init_factorytest();
	if(factory_test_status==1)
	{
		
		if(currentSWF != "test.swf")
		{
			trace("---load test.swf---");
			to_loadswf("test.swf");
		}
		test_mode=1;
	}
	else if(factory_test_status==2)
	{
		/*factory test start prepare for video play */
		
		if(currentSWF != EZCAST_SWF)
		{
			trace("---prepare for Video play test---");
			to_loadswf(EZCAST_SWF);
		}
		
	}
}

function factory_test_check()
{
	trace("check factory test status:");
	var test_status_check_ID = setInterval(check_factory_test, 2000);
	
	
	
}

var lancheckInterval:Number = -1;
function lan_init(){
	if(lancheckInterval > 0){
		clearInterval(lancheckInterval);
		lancheckInterval = -1;
	}
	lancheckInterval = setInterval(lan_check, 2000);
}

function TopbarCallback(){
	if(LAN_ONLY){
		showLanOnlyInfo();
	}
}

standbyInit();
//am8252Check();
loadfirstswf();
SystemInfo.setgpioforusbswitch(0);
init_main_net_status();
langdisautostatusGet();
if((EZCASTPRO_MODE!=8074) && (currentSWF != EZMIRR_PROBOX_SWF))
	factory_test_check();
if(EZWILAN_ENABLE){
	lan_init();
}

