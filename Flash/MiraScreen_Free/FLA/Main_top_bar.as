
import Actions.PhotoEngine;
import Actions.SystemInfo;
import Actions.WifiEngine;
_global.TopBarSWF = this; 
var DELAY_TIME:Number = 200;

var ICONMODE_APONLY:Number = 1;
var ICONMODE_ROUTERALLOW:Number = 2;
var ICONMODE_AP_P2P:Number = 3;
var ICONMODE_P2P_CLIENT:Number = 4;

var ICON_NULL:Number = 1;
var ICON_APLINK:Number = 2;
var ICON_WD:Number = 3;
var ICON_IPERRSTART:Number = 4;
var ICON_SIGNALSTART:Number = 9;
var ICON_LAN:Number = 14;

var ICON_PATH_DIR:String = "/usr/share/ezdata/";
var ICON_FILE_PRE:String = "ezcast_icon_";

var setting_info:MovieClip = MainSWF.setting_info;

var WHICHICON_AP:MovieClip = this.icon_mc.ap_link_mc;
var WHICHICON_INTER:MovieClip = this.icon_mc.internet_icon_mc;
var WHICHICON_UPGRADE:MovieClip = this.icon_mc.fw_update_tip;
var WHICHICON_5G:MovieClip = this.icon_mc.wifi5g_mc;
var WHICHICON_POP:MovieClip = this.icon_mc.pop_mc;
var WHICHICON_CONNECTING:MovieClip = this.icon_mc.client_loading_mc;
var WHICHICON_DONGLE:MovieClip = this.icon_mc.dongle;
var WHICHICON_INTERNET_LINK:MovieClip = this.icon_mc.internet_mc;
var WHICHICON_ROUTER:MovieClip = WHICHICON_INTERNET_LINK.router_mc;

var WHICHICON_SETTINGIP:MovieClip = this.setting_info;

var POP_TIMEOUT:Number = 16;		// seconds

var TEXT_OFFSET_X:Number = 0;
var TEXT_OFFSET_Y:Number = 3;
var ICON_MC_OFFSET_X:Number = this.icon_mc._x;
var ICON_MC_OFFSET_Y:Number = this.icon_mc._y;

var currentIconMode:Number = 0;
var isFlaEnable:Boolean = true;
var isSSIDShow:Boolean = true;

var popInterval:Number = -1;
var popStartTime:Number = 0;

var signal_14:Number = 0;


var SETTING_INFO_TEXT_OFFSET_X:Number = 0;
var SETTING_INFO_TEXT_OFFSET_Y:Number = 6//SETTING_INFO_H/2;
var SETTING_INFO_OFFSET_X:Number = this.setting_info._x;
var SETTING_INFO_OFFSET_Y:Number = this.setting_info._y;

var swf_arr:Array=[
	{idx:1,name:"ezcast.swf"},
	{idx:2,name:"dlna.swf"	},
	{idx:3,name:"miracast.swf"},
	{idx:4,name:"EZ_Air.swf"},
	{idx:5,name:"setting.swf"},
	{idx:1,name:"miracast_lite.swf"},
	{idx:1,name:"miracast_probox.swf"}
]

// ----------------- mc size ----------------------------------------

var WHICHICON_AP_X:Number = WHICHICON_AP._x;
var WHICHICON_AP_Y:Number = WHICHICON_AP._y;
var WHICHICON_AP_W:Number = WHICHICON_AP._width;
var WHICHICON_AP_H:Number = WHICHICON_AP._height;

var WHICHICON_INTER_X:Number = WHICHICON_INTER._x;
var WHICHICON_INTER_Y:Number = WHICHICON_INTER._y;
var WHICHICON_INTER_W:Number = WHICHICON_INTER._width;
var WHICHICON_INTER_H:Number = WHICHICON_INTER._height;

var WHICHICON_5G_X:Number = WHICHICON_5G._x;
var WHICHICON_5G_Y:Number = WHICHICON_5G._y;
var WHICHICON_5G_W:Number = WHICHICON_5G._width;
var WHICHICON_5G_H:Number = WHICHICON_5G._height;

var WHICHICON_UPGRADE_X:Number = WHICHICON_UPGRADE._x;
var WHICHICON_UPGRADE_Y:Number = WHICHICON_UPGRADE._y;
var WHICHICON_UPGRADE_W:Number = WHICHICON_UPGRADE._width;
var WHICHICON_UPGRADE_H:Number = WHICHICON_UPGRADE._height;

var WHICHICON_POP_X:Number = WHICHICON_POP._x;
var WHICHICON_POP_Y:Number = WHICHICON_POP._y;
var WHICHICON_POP_W:Number = WHICHICON_POP._width;
var WHICHICON_POP_H:Number = WHICHICON_POP._height;

var WHICHICON_CONNECTING_X:Number = WHICHICON_CONNECTING._x;
var WHICHICON_CONNECTING_Y:Number = WHICHICON_CONNECTING._y;
var WHICHICON_CONNECTING_W:Number = WHICHICON_CONNECTING._width;
var WHICHICON_CONNECTING_H:Number = WHICHICON_CONNECTING._height;

var WHICHICON_INTERNET_LINK_X:Number = WHICHICON_INTERNET_LINK._x;
var WHICHICON_DONGLE_X:Number = WHICHICON_DONGLE._x;

var POP_STRING_OFFSET_X:Number = WHICHICON_POP.ip._x;
var POP_STRING_OFFSET_Y:Number = WHICHICON_POP.ip._y;

var NUM_X:Number = icon_mc.num_txt._x;
var NUM_Y:Number = icon_mc.num_txt._y;
var NUM_W:Number = icon_mc.num_txt._width;
var NUM_H:Number = icon_mc.num_txt._height;


var SETTING_INFO_X:Number = setting_info.txt1._x;
var SETTING_INFO_Y:Number = setting_info.txt1._y;
var SETTING_INFO_W:Number = setting_info.txt1._width;
var SETTING_INFO_H:Number = setting_info.txt1._height;



var ICON_X:Number = icon_mc._x;
var MIRR_ICON_X:Number = mirr_icon_mc._x;
var SSID_24G_X:Number = 0;
var SSID_24G_Y:Number = 0;
// ----------------- wifi display status of all icon ----------------

var signalPosition:Number = 0;
var aplinkDisplay:Boolean = false;
var interDisplay:Boolean = false;
var isIpError:Boolean = false;
var curSignal:Number = -1;
var upgradeDisplay:Boolean = false;
var wifi5gDisplay:Boolean = false;
var connectingDisplay:Boolean = false;
var connectNum:Number = -1;
var connectNumDisplay:Boolean = false;
var popDisplay:Boolean = false;
var routerSsid:String = "";
var isPrevPopRight:Boolean = false;
var isApLinkiconEnable:Boolean = true;

var mainIconIndex:Number = 0;

// ==================================================================
// These two function has not used now.

function getTopBarVersion():Number
{
	return 1.0;
}

function getTopBarVendor(){
	return "ezcast";
}

// =================================================================

function get_currentswf_index(str:String):Number{
	var i=0;
	for(i=0;i<swf_arr.length;i++){
		if(swf_arr[i].name==str)
		   break;
	}
	//trace("swf_arr[i].idx"+swf_arr[i].idx)
	return swf_arr[i].idx;
}

function getPopDisplay():Boolean
{
	return popDisplay;
}

// ==================================================================

function funChange(num:Number){
	if(!MainSWF.LOGO_DISPLAY)
	{
		//trace("[funChange] LOGO hide!!!["+num+"]");
		return;
	}
	
	mode_icon.gotoAndStop(num);
	if(num==1){
		mode_icon.upleft_logo_mc.gotoAndStop(mainIconIndex);
	}	

}

function mainIconChangeTo(num:Number)
{
	mainIconIndex = num;
	if(!MainSWF.LOGO_DISPLAY)
	{
		//trace("[funChange] LOGO hide!!!["+num+"]");
		return;
	}
	
	mode_icon.upleft_logo_mc.gotoAndStop(num);
}

function iconHotspot(bool:Boolean){
	if(!MainSWF.ICON_DISPLAY)
	{
		//trace("[iconHotspot] LOGO hide!!!["+bool+"]");
		return;
	}

	if(bool){
		WHICHICON_ROUTER.gotoAndStop(2);
	}else{
		WHICHICON_ROUTER.gotoAndStop(1);
	}
}

function iconApLinkEnable(val:Boolean){
	//trace("::::::::::: isApLinkiconEnable: "+isApLinkiconEnable);
	isApLinkiconEnable=val;
	//trace("::::::::::: isApLinkiconEnable: "+isApLinkiconEnable);
}

function showSSID(ssid:String){
	setting_info.txt1._visible = false;
	setting_info.txt2._visible = false;
	if(MainSWF.SSID_DISPLAY){
		softap_info._visible = true;
		softap_info.ssid.text = ssid;
	}
	isSSIDShow = true;
	trace("ShowSSID: "+ssid+", visible["+setting_info._visible+","+softap_info._visible+"]");
}
function ProboxshowshowSSID_24G(ssid:String){
	setting_info.txt1._visible = false;
	setting_info.txt2._visible = false;
	if(MainSWF.SSID_DISPLAY){
		softap_info._visible = true;
		softap_info.ssid_24g.text = ssid;
	}
	isSSIDShow = true;
	trace("ProboxshowshowSSID_24G: "+ssid+", visible["+setting_info._visible+","+softap_info._visible+"]");
}

function showPSK(psk:String){
	setting_info.txt1._visible = false;
	setting_info.txt2._visible = false;
	if(MainSWF.PSK_DISPLAY){
		softap_info._visible = true;
		softap_info.psk.text = psk;
	}
	isSSIDShow = true;
	trace("showPSK: "+psk+", visible["+setting_info._visible+","+softap_info._visible+"]");
}

function showHostname(str:String){
	setting_info.txt1._visible = false;
	setting_info.txt2._visible = false;
	if(MainSWF.SSID_DISPLAY){
		softap_info._visible = true;
		softap_info.hostname.text =str;
	}
	isSSIDShow = true;
	trace("showHostname: "+str+", visible["+setting_info._visible+","+softap_info._visible+"]");
}

setting_info.txt1.text = "";

function showPrompt1(str1:String){
	softap_info._visible = false;
	setting_info._visible = true;
	setting_info.txt1._visible = true;
	setting_info.txt1.text = str1;
	isSSIDShow = false;
	trace("showPrompt1: "+str1+", visible["+setting_info._visible+","+softap_info._visible+"]");
}

function showPrompt2(str1:String){
	softap_info._visible = false;
	setting_info._visible = true;
	setting_info.txt2._visible = true;
	setting_info.txt2.text = str1;
	isSSIDShow = false;
	trace("showPrompt2: "+str1+", visible["+setting_info._visible+","+softap_info._visible+"]");
}

function isCurrectStrSsid():Boolean
{
	return isSSIDShow;
}

function iconMode(num:Number){
	trace("::::::::::: iconMode: "+num);
	/*
	if(signal_14 == 4){
		currentIconMode = 3;
	}
	else{
		currentIconMode = 1;
	}
	currentIconMode = 1;
	*/
	currentIconMode = num;
	if(MainSWF.ICON_DISPLAY)
	{
		icon_mc.internet_mc.gotoAndStop(currentIconMode);
	}
	funIcon();
	if(curSignal < 0){
		flaSignalNull();
	}else{
		if(isIpError){
			flaSignalIpErr(curSignal);
		}else{
			flaSignal(curSignal);
		}
	}
	if(popDisplay)
		iconPop(routerSsid);
	//if(upgradeDisplay)
		//flaUpgrade();

}

function toEZMirrOn(){
	icon_mc._x = MIRR_ICON_X;
	mirr_icon_mc._x = ICON_X;
}

function toEZMirrOff(){
	icon_mc._x = ICON_X;
	mirr_icon_mc._x = MIRR_ICON_X;
}

// ==================================================================
function isNewApLinkNull():Boolean
{
	if(connectNum > 0){
		return false;
	}else{
		return true;
	}
}
function flaApLink(){
	//trace("::::::::::: flaApLink, isFlaEnable: "+isFlaEnable+", ICON_APLINK: "+ICON_APLINK);
	if(!MainSWF.ICON_DISPLAY)
	{
		//trace("[flaApLink] LOGO hide!!!");
		return;
	}
//	trace("currentIconMode = " + currentIconMode + "ICONMODE_P2P_CLIENT = " + ICONMODE_P2P_CLIENT + "ICONMODE_AP_P2P = " + ICONMODE_AP_P2P);
//	trace("ICON_SIGNALSTART = " + ICON_SIGNALSTART + "signal" + curSignal);
	
	if(isFlaEnable){
		if(currentIconMode == ICONMODE_P2P_CLIENT){
			WHICHICON_AP.gotoAndStop(ICON_SIGNALSTART + signal);
		}else if(currentIconMode != ICONMODE_AP_P2P){
			WHICHICON_INTER.gotoAndStop(ICON_SIGNALSTART + signal);
		}
/*		
		if(isNewApLinkNull()){
			WHICHICON_AP.gotoAndStop(ICON_NULL);
		}else{
			WHICHICON_AP.gotoAndStop(ICON_APLINK);
		}
*/
	}
}

var isApLinkNull:Boolean = true;
var ezApLinkInterval:Number = -1;
function ezApLinkDelay(){
	var icon_path:String;

	if(ezApLinkInterval > 0){
		clearInterval(ezApLinkInterval);
		ezApLinkInterval = -1;
	}
	isApLinkNull = isNewApLinkNull();
/*	if(isApLinkNull){
		icon_path = ICON_PATH_DIR + ICON_FILE_PRE + ICON_NULL + ".png";
	}else{
		icon_path = ICON_PATH_DIR + ICON_FILE_PRE + ICON_APLINK + ".png";
	}
*/
	//curSignal
	var num:Number;
	if(_signal < 0){
		num = ICON_NULL;
	}else if(_ipErr){
		num = ICON_IPERRSTART + _signal;
	}else{
		num = ICON_SIGNALSTART + _signal;
	}
	if(num>3)
        num=13;
	var icon_path:String = ICON_PATH_DIR + ICON_FILE_PRE + num + ".png";
	var x:Number = ICON_MC_OFFSET_X + WHICHICON_AP_X;
	var y:Number = ICON_MC_OFFSET_Y + WHICHICON_AP_Y;
	
	MainSWF.ezShowPng(icon_path, x, y, WHICHICON_AP_W, WHICHICON_AP_H);
}
var ezssidInterval:Number = -1;
var ezSsid:String ="";
var ezpsk:String ="";
function ezshowssidDelay(){
       if(ezssidInterval > 0){
               clearInterval(ezssidInterval);
               ezssidInterval = -1;
      }
       var x:Number = 475 + 74 + 0;
       var y:Number = 70 + 15 + 3;
       ezSsid = WifiEngine.getssid();
       ezpsk = WifiEngine.getpassword();
       var str:String = "SSID : "+ezSsid+"                    "+"PSK :"+ezpsk ;
       MainSWF.ezShowText(str, 32, 0xFFFFFF, x, y);
}
function ezshowssid(){
      var x:Number = 475 + 74 + 0;
       var y:Number = 70 + 15 + 3;
       MainSWF.ezCleanRect(x, y, 896, 35);
       
       if(ezssidInterval > 0){
               clearInterval(ezssidInterval);
              ezssidInterval = -1;
      }
       ezssidInterval = setInterval(ezshowssidDelay, DELAY_TIME);
}
function ezApLink(){
//	trace("ezApLink isApLinkiconEnable="+isApLinkiconEnable);
	if(!MainSWF.isNetDisplayEnable() || !isApLinkiconEnable)
		return;

	if(!MainSWF.ICON_DISPLAY)
	{
		//trace("[ezApLink] LOGO hide!!!");
		return;
	}
	trace("::::::::::: ezApLink");
	var x:Number = ICON_MC_OFFSET_X + WHICHICON_AP_X;
	var y:Number = ICON_MC_OFFSET_Y + WHICHICON_AP_Y;
	MainSWF.ezCleanRect(x, y, WHICHICON_AP_W, WHICHICON_AP_H);
	if(ezApLinkInterval > 0){
		clearInterval(ezApLinkInterval);
		ezApLinkInterval = -1;
	}
	ezApLinkInterval = setInterval(ezApLinkDelay, DELAY_TIME);
}

function iconApLink(){
	if(MainSWF.LAN_ONLY || !isApLinkiconEnable)
		return;
	//trace("::::::::::: iconApLink, isNetDisplayEnable: "+MainSWF.isNetDisplayEnable()+", aplinkDisplay: "+aplinkDisplay+", isNewApLinkNull: "+isNewApLinkNull()+", isApLinkNull: "+isApLinkNull+", isApLinkiconEnable: "+isApLinkiconEnable);
/*	if(currentIconMode == 3){
		if(!aplinkDisplay || (isNewApLinkNull() != isApLinkNull))
			ezApLink();
	}
*/
	if(MainSWF.isNetDisplayEnable()){
		if(!aplinkDisplay || (isNewApLinkNull() != isApLinkNull))
			ezApLink();
	}else if(isFlaEnable){
		flaApLink();
	}
	aplinkDisplay = true;
	
}

// -----------------------------------------------------------------

function flaMirrAllow(){
	//trace("::::::::::: flaMirrAllow");
	if(!MainSWF.ICON_DISPLAY)
	{
		//trace("[flaMirrAllow] LOGO hide!!!");
		return;
	}
	if(isFlaEnable){
		WHICHICON_INTER.gotoAndStop(ICON_WD);
	}
}

var ezMirrAllowInterval:Number = -1;
function ezMirrAllowDelay(){
	if(ezMirrAllowInterval > 0){
		clearInterval(ezMirrAllowInterval);
		ezMirrAllowInterval = -1;
	}
	var icon_path:String = ICON_PATH_DIR + ICON_FILE_PRE + ICON_WD + ".png";
	var x:Number = ICON_MC_OFFSET_X + WHICHICON_INTER_X;
	var y:Number = ICON_MC_OFFSET_Y + WHICHICON_INTER_Y;

	MainSWF.ezShowPng(icon_path, x, y, WHICHICON_INTER_W, WHICHICON_INTER_H);
}
function ezMirrAllow(){
	if(!MainSWF.isNetDisplayEnable())
		return;
	if(!MainSWF.ICON_DISPLAY)
	{
		//trace("[ezMirrAllow] LOGO hide!!!");
		return;
	}

	//trace("::::::::::: ezMirrAllow");
	var x:Number = ICON_MC_OFFSET_X + WHICHICON_INTER_X;
	var y:Number = ICON_MC_OFFSET_Y + WHICHICON_INTER_Y;
	MainSWF.ezCleanRect(x, y, WHICHICON_INTER_W, WHICHICON_INTER_H);
	if(ezMirrAllowInterval > 0){
		clearInterval(ezMirrAllowInterval);
		ezMirrAllowInterval = -1;
	}
	ezMirrAllowInterval = setInterval(ezMirrAllowDelay, DELAY_TIME);
}

function iconMirrAllow(){
	//trace("::::::::::: iconMirrAllow");
	if(MainSWF.isNetDisplayEnable()){
		if(!interDisplay)
			ezMirrAllow();
	}else if(isFlaEnable){
		flaMirrAllow();
	}
	interDisplay = true;
}

// -----------------------------------------------------------------

function funIcon(){
	//trace("::::::::::: funIcon");
	trace("currentIconMode = " + currentIconMode + "ICONMODE_APONLY = " + ICONMODE_APONLY + "ICONMODE_ROUTERALLOW =" + ICONMODE_ROUTERALLOW);
	if(currentIconMode == ICONMODE_APONLY || currentIconMode == ICONMODE_ROUTERALLOW){
		iconApLink();
		interDisplay = false;
	}else{
		iconMirrAllow();
		if(currentIconMode == ICONMODE_AP_P2P){
			iconApLink();
		}else{
			aplinkDisplay = false;
			iconConnectNumberClean();
		}
	}
}
// -----------------------------------------------------------------
var ezSignalInterval:Number = -1;
var _signal:Number = 0;
var _ipErr:Boolean = false;
function ezSignalDisplayDelay(){
	if(ezSignalInterval > 0){
		clearInterval(ezSignalInterval);
		ezSignalInterval = -1;
	}
	var num:Number;
	if(_signal < 0){
		num = ICON_NULL;
	}else if(_ipErr){
		num = ICON_IPERRSTART + _signal;
	}else{
		num = ICON_SIGNALSTART + _signal;
	}
	var icon_path:String = ICON_PATH_DIR + ICON_FILE_PRE + num + ".png";
	if(1 == num)
	{
		iconPopClean();
	}
	
	var png_x:Number;
	var png_y:Number;
	var w:Number;
	var h:Number;
	//trace("::::::::::: currentIconMode: "+currentIconMode+", num: "+num);
	if(currentIconMode == ICONMODE_P2P_CLIENT){
		png_x = WHICHICON_AP_X;
		png_y = WHICHICON_AP_Y;
		w = WHICHICON_AP_W;
		h = WHICHICON_AP_H;
	}else if(currentIconMode == ICONMODE_AP_P2P){
		return;
	}else{
		png_x = WHICHICON_INTER_X;
		png_y = WHICHICON_INTER_Y;
		w = WHICHICON_INTER_W;
		h = WHICHICON_INTER_H;
	}
	var x:Number = ICON_MC_OFFSET_X + png_x;
	var y:Number = ICON_MC_OFFSET_Y + png_y;//+ 10;
	MainSWF.ezShowPng(icon_path, x, y, w, h);
}

function ezSignalDisplay(signal:Number, ipErr:Boolean){
	var png_x:Number;
	var png_y:Number;
	var w:Number;
	var h:Number;
	//trace("::::::::::: currentIconMode: "+currentIconMode);
	_signal = signal;
	_ipErr = ipErr;
	if(!MainSWF.ICON_DISPLAY)
	{
		//trace("[ezSignalDisplay] LOGO hide!!!");
		return;
	}
	if(currentIconMode == ICONMODE_P2P_CLIENT){
		png_x = WHICHICON_AP_X;
		png_y = WHICHICON_AP_Y;
		w = WHICHICON_AP_W;
		h = WHICHICON_AP_H;
	}else if(currentIconMode == ICONMODE_AP_P2P){
		return;
	}else{
		png_x = WHICHICON_INTER_X;
		png_y = WHICHICON_INTER_Y;
		w = WHICHICON_INTER_W;
		h = WHICHICON_INTER_H;
	}
	var x:Number = ICON_MC_OFFSET_X + png_x;
	var y:Number = ICON_MC_OFFSET_Y + png_y;
	MainSWF.ezCleanRect(x, y, w, h);
	if(ezSignalInterval > 0){
		clearInterval(ezSignalInterval);
		ezSignalInterval = -1;
	}
	ezSignalInterval = setInterval(ezSignalDisplayDelay, DELAY_TIME);
}

// -----------------------------------------------------------------

function flaSignal(signal:Number){
	if(!MainSWF.ICON_DISPLAY)
	{
		//trace("[flaSignal] LOGO hide!!!");
		return;
	}
	if(isFlaEnable){

		if(currentIconMode == ICONMODE_P2P_CLIENT){
			WHICHICON_AP.gotoAndStop(ICON_SIGNALSTART + signal);
		}else if(currentIconMode != ICONMODE_AP_P2P){
			WHICHICON_INTER.gotoAndStop(ICON_SIGNALSTART + signal);
		}
		
	}
}

function ezSignal(signal:Number){
	if(!MainSWF.isNetDisplayEnable())
		return;

	ezSignalDisplay(signal, false);
}

function iconSignal(signal:Number){
	var position:Number = 0;
	if(currentIconMode == ICONMODE_P2P_CLIENT){
		position = 1;
		aplinkDisplay = false;
	}else if(currentIconMode != ICONMODE_AP_P2P){
		position = 2;
		interDisplay = false;
	}else{
		position = 0;
	}
	
	if(MainSWF.isNetDisplayEnable()){
		if(isIpError || signalPosition != position || curSignal != signal)
			ezSignal(signal);
	}else if(isFlaEnable){
		flaSignal(signal);
	}
	
	isIpError = false;
	curSignal = signal;
	signalPosition = position;
}

// -----------------------------------------------------------------

function flaSignalIpErr(signal:Number){
	if(!MainSWF.ICON_DISPLAY)
	{
		//trace("[flaSignalIpErr] LOGO hide!!!");
		return;
	}
	if(isFlaEnable){
		if(currentIconMode == ICONMODE_P2P_CLIENT){
			WHICHICON_AP.gotoAndStop(ICON_IPERRSTART + signal);
		}else if(currentIconMode != ICONMODE_AP_P2P){
			WHICHICON_INTER.gotoAndStop(ICON_IPERRSTART + signal);
		}
	}
}

function ezSignalIpErr(signal:Number){
	if(!MainSWF.isNetDisplayEnable())
		return;

	ezSignalDisplay(signal, true);
}

function iconSignalIpErr(signal:Number){
	var position:Number = 0;
	if(currentIconMode == ICONMODE_P2P_CLIENT){
		position = 1;
		aplinkDisplay = false;
	}else if(currentIconMode != ICONMODE_AP_P2P){
		position = 2;
		interDisplay = false;
	}else{
		position = 0;
	}
	
	if(MainSWF.isNetDisplayEnable()){
		if(!isIpError || signalPosition != position || curSignal != signal)
			ezSignalIpErr(signal);
	}else if(isFlaEnable){
		flaSignalIpErr(signal);
	}
	
	isIpError = true;
	curSignal = signal;
	signalPosition = position;
}

// -----------------------------------------------------------------

function flaSignalNull(){
	if(!MainSWF.ICON_DISPLAY)
	{
		//trace("[flaSignalNull] LOGO hide!!!");
		return;
	}
	if(isFlaEnable){
		if(currentIconMode == ICONMODE_P2P_CLIENT){
			WHICHICON_AP.gotoAndStop(ICON_NULL);
		}else if(currentIconMode != ICONMODE_AP_P2P){
			WHICHICON_INTER.gotoAndStop(ICON_NULL);
		}
	}
}

function ezSignalNull(){
	if(!MainSWF.isNetDisplayEnable())
		return;

	ezSignalDisplay(-1, false);
}

function iconSignalNull(){
	var position:Number = 0;
	if(currentIconMode == ICONMODE_P2P_CLIENT){
		position = 1;
		aplinkDisplay = false;
	}else if(currentIconMode != ICONMODE_AP_P2P){
		position = 2;
		interDisplay = false;
	}else{
		position = 0;
	}
	//trace("isIpError: "+isIpError+", signalPosition = "+signalPosition + ", position = "+ position + ", curSignal = "+curSignal);
	if(MainSWF.isNetDisplayEnable()){
		if(isIpError || signalPosition != position || curSignal >= 0)
			ezSignalNull();
	
	}else if(isFlaEnable){
		flaSignalNull();
	}
	//setting_info.txt1.text = "http://192.168.203.1"
	isIpError = false;
	curSignal = -1;
	signalPosition = position;
	display_setup = 0;
}

// -----------------------------------------------------------------

function flaLanOn(){
	if(!MainSWF.ICON_DISPLAY)
	{
		//trace("[flaLanOn] LOGO hide!!!");
		return;
	}
	if(isFlaEnable){
		if(currentIconMode == ICONMODE_P2P_CLIENT){
			WHICHICON_AP.gotoAndStop(ICON_LAN);
		}else if(currentIconMode != ICONMODE_AP_P2P){
			WHICHICON_INTER.gotoAndStop(ICON_LAN);
		}
	}
}

function ezLanOn(){
	if(!MainSWF.isNetDisplayEnable())
		return;

	ezSignalDisplay(5, false);
}

function iconLanOn(){
	var position:Number = 0;
	if(currentIconMode == ICONMODE_P2P_CLIENT){
		position = 1;
		aplinkDisplay = false;
	}else if(currentIconMode != ICONMODE_AP_P2P){
		position = 2;
		interDisplay = false;
	}else{
		position = 0;
	}
	if(MainSWF.isNetDisplayEnable()){
		if(isIpError || signalPosition != position || curSignal != 5)
			ezLanOn();
	}else if(isFlaEnable){
		flaLanOn();
	}
	
	isIpError = false;
	curSignal = 5;
	signalPosition = position;
}

// -----------------------------------------------------------------
var ezDongleInterval:Number = -1;
function ez5gDisplay(sta:Boolean){
	var icon_path_5g:String = ICON_PATH_DIR + ICON_FILE_PRE + "5g.png";
	var icon_path_24g:String = ICON_PATH_DIR + ICON_FILE_PRE + "24g.png";
	var x:Number = ICON_MC_OFFSET_X + WHICHICON_5G_X;
	var y:Number = ICON_MC_OFFSET_Y + WHICHICON_5G_Y;

	if(sta)
		MainSWF.ezShowPng(icon_path_5g, x, y, WHICHICON_5G_W, WHICHICON_5G_H);
	else
		MainSWF.ezShowPng(icon_path_24g, x, y, WHICHICON_5G_W, WHICHICON_5G_H);
	
}

function ezUpgradeDisplay(){
	var icon_path:String = ICON_PATH_DIR + ICON_FILE_PRE + "upgrade.png";
	var x:Number = ICON_MC_OFFSET_X + WHICHICON_UPGRADE_X;
	var y:Number = ICON_MC_OFFSET_Y + WHICHICON_UPGRADE_Y;
	
	MainSWF.ezShowPng(icon_path, x, y, WHICHICON_UPGRADE_W, WHICHICON_UPGRADE_H);
}

function ezDongleDelay(){
	if(ezDongleInterval > 0){
		clearInterval(ezDongleInterval);
		ezDongleInterval = -1;
	}
	if(!MainSWF.isNetDisplayEnable())
		return;
	
	ez5gDisplay(wifi5gDisplay);
	if(upgradeDisplay)
		ezUpgradeDisplay();
}

function ezDongle(){
	if(!MainSWF.isNetDisplayEnable())
		return;
	if(!MainSWF.ICON_DISPLAY)
	{
		//trace("[ezDongle] LOGO hide!!!");
		return;
	}
	
	var x:Number = ICON_MC_OFFSET_X + WHICHICON_5G_X;
	var y:Number = ICON_MC_OFFSET_Y + WHICHICON_5G_Y;
	MainSWF.ezCleanRect(x, y, WHICHICON_5G_W, WHICHICON_5G_H);
	
	x = ICON_MC_OFFSET_X + WHICHICON_UPGRADE_X;
	y = ICON_MC_OFFSET_Y + WHICHICON_UPGRADE_Y;
	MainSWF.ezCleanRect(x, y, WHICHICON_UPGRADE_W, WHICHICON_UPGRADE_H);
	
	if(ezDongleInterval > 0){
		clearInterval(ezDongleInterval);
		ezDongleInterval = -1;
	}
	ezDongleInterval = setInterval(ezDongleDelay, DELAY_TIME);
}

// -------------------------------------------------------------------

function fla5g(){
	if(!MainSWF.ICON_DISPLAY)
	{
		//trace("[fla5g] LOGO hide!!!");
		return;
	}
	if(isFlaEnable){
		WHICHICON_5G._visible = true;
		WHICHICON_5G.gotoAndStop(2);
	}
}

function icon5g(){
	if(MainSWF.isNetDisplayEnable()){
		if(!wifi5gDisplay){
			wifi5gDisplay = true;
			ezDongle();
		}
	}else if(isFlaEnable){
		fla5g();
	}
	wifi5gDisplay = true;
}

// -----------------------------------------------------------------

function fla5gClean(){
	if(!MainSWF.ICON_DISPLAY)
	{
		//trace("[fla5g] LOGO hide!!!");
		return;
	}
	WHICHICON_5G._visible = true;
	WHICHICON_5G.gotoAndStop(1);
}

function icon5gClean(){
	if(MainSWF.isNetDisplayEnable()){
		if(wifi5gDisplay){
			wifi5gDisplay = false;
			ezDongle();
		}
	}else if(isFlaEnable){
		fla5gClean();
	}
	wifi5gDisplay = false;

}

// -----------------------------------------------------------------

function flaUpgrade(){
	if(!MainSWF.ICON_DISPLAY)
	{
		//trace("[flaUpgrade] LOGO hide!!!");
		return;
	}
	if(isFlaEnable){
		WHICHICON_UPGRADE._visible = true;
	}
}

function iconUpgrade(){
	if(MainSWF.isNetDisplayEnable()){
		if(!upgradeDisplay){
			upgradeDisplay = true;
			ezDongle();
		}
	}else if(isFlaEnable){
		flaUpgrade();
	}
	upgradeDisplay = true;
}

// -----------------------------------------------------------------

function flaUpgradeClean(){
	if(!MainSWF.ICON_DISPLAY)
	{
		//trace("[flaUpgradeClean] LOGO hide!!!");
		return;
	}
	WHICHICON_UPGRADE._visible = false;
}

function iconUpgradeClean(){
	if(MainSWF.isNetDisplayEnable()){
		if(upgradeDisplay){
			upgradeDisplay = false;
			ezDongle();
		}
	}else if(isFlaEnable){
		flaUpgradeClean();
	}
	upgradeDisplay = false;

}

// -----------------------------------------------------------------

function flaConnectNumber(num:Number){
	if(!aplinkDisplay)
		return;
	if(!MainSWF.ICON_DISPLAY)
	{
		//trace("[flaConnectNumber] LOGO hide!!!");
		return;
	}
		
	if(isFlaEnable){
		icon_mc.num_txt.text = "[ " + num + " ]";
		//flashowip(num);
	}
}

function flashowip(num:Number){		
	//trace("++++++++++++++++++++++++flashowip   "+num);
	var IpSsid:String ="";
	if(isFlaEnable){
		if(num >0){
			setting_info.txt1.text = "http://192.168.203.1";
		}else{
			IpSsid = WifiEngine.getssid();
			setting_info.txt1.text = "Connect to SSID ("+IpSsid+")";
		}
		//icon_mc.num_txt.text = "[ " + num + " ]";
		
	}
	
}

var ezNumInterval:Number = -1;
var showNum:Number = 0;
function ezConnectNumberDelay(){
	if(ezNumInterval > 0){
		clearInterval(ezNumInterval);
		ezNumInterval = -1;
	}
	var x:Number = ICON_MC_OFFSET_X + NUM_X + TEXT_OFFSET_X;
	var y:Number = ICON_MC_OFFSET_Y + NUM_Y + TEXT_OFFSET_Y;
	trace("ezshowipDelay+++6");
	ezshowip();
	var str:String = "[ " + showNum + " ]";
	MainSWF.ezShowText(str, 21, 0xFFFFFF, x, y);
}
function ezConnectNumber(num:Number){
	//	trace("ezConnectNumber isApLinkiconEnable="+isApLinkiconEnable);
	if(MainSWF.isInWebMode){
       aplinkDisplay = true;
    }
	if(!aplinkDisplay || !MainSWF.isNetDisplayEnable()||(!isApLinkiconEnable))
		return;
	showNum = num;
	if(!MainSWF.ICON_DISPLAY)
	{
		//trace("[ezConnectNumber] LOGO hide!!!");
		return;
	}
		
	var x:Number = ICON_MC_OFFSET_X + NUM_X + TEXT_OFFSET_X;
	var y:Number = ICON_MC_OFFSET_Y + NUM_Y + TEXT_OFFSET_Y;
	MainSWF.ezCleanRect(x, y, NUM_W, NUM_H);
	showNum = num;
	if(ezNumInterval > 0){
		clearInterval(ezNumInterval);
		ezNumInterval = -1;
	}
	ezNumInterval = setInterval(ezConnectNumberDelay, DELAY_TIME);

}
function iconConnectNumber(num:Number){
	//trace("iconConnectNumber isApLinkiconEnable="+isApLinkiconEnable);
	if((num < 0)||(!isApLinkiconEnable))
		return;
	
	if(MainSWF.isNetDisplayEnable()){
		if(!connectNumDisplay || connectNum != num)
			ezConnectNumber(num);
	}else if(isFlaEnable){
		flaConnectNumber(num);
	}
	connectNum = num;
	connectNumDisplay = true;
//	iconApLink();
       if(!MainSWF.webconnectionsnum){
               iconApLink();
       }
}

// -----------------------------------------------------------------

function flaConnectNumberClean(){
	if(!MainSWF.ICON_DISPLAY)
	{
		//trace("[flaConnectNumberClean] LOGO hide!!!");
		return;
	}
	icon_mc.num_txt.text = "";
}

function ezshowipClean(){
//	var x:Number = ICON_MC_OFFSET_X + NUM_X + TEXT_OFFSET_X;
//	var y:Number = ICON_MC_OFFSET_Y + NUM_Y + TEXT_OFFSET_Y;
	//var x:Number = 500 + 305 + 0;
	//var y:Number = 183 - 5 + 18;
	var x:Number = SETTING_INFO_OFFSET_X + SETTING_INFO_X + SETTING_INFO_TEXT_OFFSET_X;
	var y:Number = SETTING_INFO_OFFSET_Y + SETTING_INFO_Y + SETTING_INFO_TEXT_OFFSET_Y;
	MainSWF.ezCleanRect(x, y,  SETTING_INFO_W+20, SETTING_INFO_H);	
}

function ezConnectNumberClean(){
	if(!MainSWF.isNetDisplayEnable())
		return;
	if(!MainSWF.ICON_DISPLAY)
	{
		//trace("[ezConnectNumberClean] LOGO hide!!!");
		return;
	}

	var x:Number = ICON_MC_OFFSET_X + NUM_X + TEXT_OFFSET_X;
	var y:Number = ICON_MC_OFFSET_Y + NUM_Y + TEXT_OFFSET_Y;
	MainSWF.ezCleanRect(x, y, NUM_W, NUM_H);
	ezshowipClean();
}

function iconConnectNumberClean(){
	if(MainSWF.isNetDisplayEnable()){
		if(connectNumDisplay)
			ezConnectNumberClean();
	}else if(isFlaEnable){
		flaConnectNumberClean();
	}
	connectNumDisplay = false;
}

// -----------------------------------------------------------------

function flaWifiConnecting(){
	if(!MainSWF.ICON_DISPLAY)
	{
		//trace("[flaWifiConnecting] LOGO hide!!!");
		return;
	}
	if(isFlaEnable){
		icon_mc.client_loading_mc._visible = true;
	}
}

var ezConnectingInterval:Number = -1;
function ezWifiConnectingDelay(){
	if(ezConnectingInterval > 0){
		clearInterval(ezConnectingInterval);
		ezConnectingInterval = -1;
	}
	
	var icon_path:String = ICON_PATH_DIR + ICON_FILE_PRE + "connecting.png";
	var x:Number = ICON_MC_OFFSET_X + WHICHICON_CONNECTING_X;
	var y:Number = ICON_MC_OFFSET_Y + WHICHICON_CONNECTING_Y;
	
	MainSWF.ezShowPng(icon_path, x, y, WHICHICON_CONNECTING_W, WHICHICON_CONNECTING_H);
}

function ezWifiConnecting(){
	if(!MainSWF.isNetDisplayEnable())
		return;

	if(!MainSWF.ICON_DISPLAY)
	{
		//trace("[ezWifiConnecting] LOGO hide!!!");
		return;
	}
	var x:Number = ICON_MC_OFFSET_X + WHICHICON_CONNECTING_X;
	var y:Number = ICON_MC_OFFSET_Y + WHICHICON_CONNECTING_Y;
	MainSWF.ezCleanRect(x, y, WHICHICON_CONNECTING_W, WHICHICON_CONNECTING_H);
	if(ezConnectingInterval > 0){
		clearInterval(ezConnectingInterval);
		ezConnectingInterval = -1;
	}
	ezConnectingInterval = setInterval(ezWifiConnectingDelay, DELAY_TIME);
}
var ezIpInterval:Number = -1;
function ezshowipDelay(){
	var IpSsid:String ="";
	var str:String = "";
	if(ezIpInterval > 0){
		clearInterval(ezIpInterval);
		ezIpInterval = -1;
	}
	var x:Number = SETTING_INFO_OFFSET_X + SETTING_INFO_X + SETTING_INFO_TEXT_OFFSET_X;
	var y:Number = SETTING_INFO_OFFSET_Y + SETTING_INFO_Y + SETTING_INFO_TEXT_OFFSET_Y;
	MainSWF.ezCleanRect(x, y, SETTING_INFO_W+25, SETTING_INFO_H+10);
	var wifi_status=WifiEngine.getConnectionStatus();
	if(wifi_status == WifiEngine.WIFI_COMPLETED)
	{
		IpSsid = WifiEngine.GetIPAddress();
		str = "http://"+IpSsid;
	}
	else
	{
		if(showNum >0){
			str = "http://192.168.203.1";
		}else{
			IpSsid = WifiEngine.getssid();
			str = "Connect to SSID ("+IpSsid+")";
		}
	}

	MainSWF.ezCleanRect(x, y, SETTING_INFO_W+25, SETTING_INFO_H+10);
	MainSWF.ezShowText(str, 26, 0xFFFFFF, x, y);
}

function ezshowip(){
		
	var x:Number = SETTING_INFO_OFFSET_X + SETTING_INFO_X + SETTING_INFO_TEXT_OFFSET_X;
	var y:Number = SETTING_INFO_OFFSET_Y + SETTING_INFO_Y + SETTING_INFO_TEXT_OFFSET_Y;
	MainSWF.ezCleanRect(x, y, SETTING_INFO_W, SETTING_INFO_H);
	if(ezIpInterval > 0){
		clearInterval(ezIpInterval);
		ezIpInterval = -1;
	}
	ezIpInterval = setInterval(ezshowipDelay, DELAY_TIME);
}

function iconWifiConnecting(){
	if(MainSWF.isNetDisplayEnable()){
		if(!connectingDisplay)
			ezWifiConnecting();
	}else if(isFlaEnable){
		flaWifiConnecting();
	}
	connectingDisplay = true;
}

// -----------------------------------------------------------------

function flaWifiConnectingClean(){
	if(!MainSWF.ICON_DISPLAY)
	{
		//trace("[flaWifiConnectingClean] LOGO hide!!!");
		return;
	}
	icon_mc.client_loading_mc._visible = false;
}

function ezWifiConnectingClean(){
	if(!MainSWF.isNetDisplayEnable())
		return;

	if(!MainSWF.ICON_DISPLAY)
	{
		//trace("[ezWifiConnectingClean] LOGO hide!!!");
		return;
	}
	var x:Number = ICON_MC_OFFSET_X + WHICHICON_CONNECTING_X;
	var y:Number = ICON_MC_OFFSET_Y + WHICHICON_CONNECTING_Y;
	MainSWF.ezCleanRect(x, y, WHICHICON_CONNECTING_W, WHICHICON_CONNECTING_H);
}

function iconWifiConnectingClean(){
	if(MainSWF.isNetDisplayEnable()){
		if(connectingDisplay)
			ezWifiConnectingClean();
	}else if(isFlaEnable){
		flaWifiConnectingClean();
	}
	connectingDisplay = false;
}

// -----------------------------------------------------------------

function isPopOnRight(mode:Number){
	trace("mode="+mode+"ICONMODE_P2P_CLIENT="+ICONMODE_P2P_CLIENT);
	//ezConnectNumberClean();
	//ezConnectNumberclear();
	//iconConnectNumberClean();
	//icon_mc.num_txt.text = "";
	if(mode == ICONMODE_P2P_CLIENT)
		return false;
	else
		return true;
}

function flaPop(ssid:String){
	if(!MainSWF.ICON_DISPLAY)
	{
		//trace("[flaPop] LOGO hide!!!");
		return;
	}
/*
	icon_mc.iPhone_txt._visible = false;
	icon_mc.dashed1_txt._visible = false;
	icon_mc.dashed2_txt._visible = true;
	icon_mc.internet_icon_mc._visible = true;
	WHICHICON_INTER._visible = true;
	icon_mc.pop_mc._visible = true;
*/
	//icon_mc.num_txt.text = "";
	 trace("flaPop="+ssid);
	if(isFlaEnable){
		signal_14 = 4;
		icon_mc.pop_mc._visible = true;
		icon_mc.pop_mc.ip.text = ssid;
		if(isPopOnRight(currentIconMode)){
			icon_mc.pop_mc._x = WHICHICON_POP_X;
			trace("cur_pop_x="+icon_mc.pop_mc._x);
		}else{
			icon_mc.pop_mc._x = WHICHICON_POP_X;
			//icon_mc.pop_mc._x = WHICHICON_POP_X - (WHICHICON_INTER_X - WHICHICON_AP_X);
			trace("////////////////////cur_pop_x="+icon_mc.pop_mc._x);
		}
	}
}

function ezPop(ssid:String){
	if(!MainSWF.isNetDisplayEnable())
		return;

	if(!MainSWF.ICON_DISPLAY)
	{
		//trace("[ezPop] LOGO hide!!!");
		return;
	}
	//iconConnectNumberClean();
       trace("ezPop="+ssid);
	var cur_pop_x:Number;
	if(isPopOnRight(currentIconMode)){
		cur_pop_x = WHICHICON_POP_X;
		trace("cur_pop_x="+cur_pop_x);
	}else{
		cur_pop_x = WHICHICON_POP_X;
		//cur_pop_x = WHICHICON_POP_X - (WHICHICON_INTER_X - WHICHICON_AP_X);
		trace("////////////////////cur_pop_x="+cur_pop_x);
	}
	var icon_path:String = ICON_PATH_DIR + ICON_FILE_PRE + "pop.png";
	var x:Number = ICON_MC_OFFSET_X + cur_pop_x;
	var y:Number = ICON_MC_OFFSET_Y + WHICHICON_POP_Y;

	MainSWF.ezCleanRect(x, y, WHICHICON_POP_W, WHICHICON_POP_H);
	MainSWF.ezShowPng(icon_path, x, y, WHICHICON_POP_W, WHICHICON_POP_H);
	
	var textlen = MainSWF.lengthCalculationFontSize_25(ssid);
	var textOffset = 0;
	if(textlen < 260){
		textOffset = (260-textlen)/2;
	}
	var text_x:Number = x + POP_STRING_OFFSET_X + TEXT_OFFSET_X + textOffset;
	var text_y:Number = y + POP_STRING_OFFSET_Y + TEXT_OFFSET_Y;
	var str:String = ssid;
	MainSWF.ezShowText(str, 25, 0xC9C9C9 , text_x, text_y);//0xC9C9C9

	ezshowip();
}

function popTimeout(){
	if(popInterval > 0){
		clearInterval(popInterval);
		popInterval = -1;
	}
	var currentTime:Number = MainSWF.getSystemTime();
	trace("::::::::::: currentTime: "+currentTime);
	var runTime:Number = currentTime - popStartTime;
	if(runTime < POP_TIMEOUT){
		popInterval = setInterval(popTimeout, (POP_TIMEOUT - runTime) * 1000);
		return;
	}
	
	iconPopClean();
}

function popTimeStart(){
	if(popInterval > 0){
		clearInterval(popInterval);
		popInterval = -1;
	}
	popStartTime = MainSWF.getSystemTime();
	trace("::::::::::: popStartTime: "+popStartTime);
	popInterval = setInterval(popTimeout, POP_TIMEOUT * 1000);
}

function iconPop(ssid:String){
	var popRight:Boolean = isPopOnRight(currentIconMode);
	if(MainSWF.isNetDisplayEnable()){
		if(!popDisplay){
			ezPop(ssid);
		}else if(isPrevPopRight != popRight){
			ezPopClean();
			ezPop(ssid);
		}
	}else if(isFlaEnable){
		flaPop(ssid);
	}

	icon_mc.num_txt.text = "";
	iconConnectNumberClean();
	//popTimeStart();
	popDisplay = true;
	routerSsid = ssid;
	isPrevPopRight = popRight;	
	iconApLink();
}

// -----------------------------------------------------------------

function flaPopClean(){
	if(!MainSWF.ICON_DISPLAY)
	{
		//trace("[flaPopClean] LOGO hide!!!");
		return;
	}
	icon_mc.pop_mc._visible = false;
}

function ezPopClean(){
	if(!MainSWF.isNetDisplayEnable())
		return;
	popDisplay = false;
	if(!MainSWF.ICON_DISPLAY)
	{
		//trace("[ezPopClean] LOGO hide!!!");
		return;
	}
	ezRefresh();
}

function iconPopClean(){
	//trace("iconPopClean ---- isNetDisplayEnable: "+MainSWF.isNetDisplayEnable());
	if(popInterval > 0){
		clearInterval(popInterval);
		popInterval = -1;
	}
	if(MainSWF.isNetDisplayEnable()){
		if(popDisplay)
			ezPopClean();
	}else if(isFlaEnable){
		flaPopClean();
	}
	popDisplay = false;
}

// =================================================================

function flaDisable(){
	isFlaEnable = false;
	if(MainSWF.ICON_DISPLAY)
	{
		//if(!aplinkDisplay)
			WHICHICON_AP._visible = false;
		if(!interDisplay)
			WHICHICON_INTER._visible = false;
	}
	flaUpgradeClean();
	fla5gClean();
	flaConnectNumberClean();
	flaWifiConnectingClean();
	flaPopClean();
}

function flaEnable(){
	//trace("::::::::::: flaEnable, aplinkDisplay: "+aplinkDisplay+", interDisplay: "+interDisplay);
	isFlaEnable = true;
	if(MainSWF.ICON_DISPLAY)
	{
		if(MainSWF.LAN_ONLY)
			WHICHICON_AP._visible = false;
		else
			WHICHICON_AP._visible = true;
		WHICHICON_INTER._visible = true;
	}
	if(aplinkDisplay)
		flaApLink();
	if(interDisplay)
		flaMirrAllow();
	if(upgradeDisplay)
		flaUpgrade();
	fla5g();
	//funIcon();
	if(curSignal < 0){
		flaSignalNull();
	}else{
		if(isIpError){
			flaSignalIpErr(curSignal);
		}else{
			flaSignal(curSignal);
		}
	}
	
	if(connectNumDisplay)
		flaConnectNumber(connectNum);
		
	if(connectingDisplay)
		flaWifiConnecting();
		
	if(popDisplay)
		flaPop(routerSsid);

}

var ezRefreshIntervar:Number = -1;
function ezRefreshDelay(){
	if(ezRefreshIntervar > 0){
		clearInterval(ezRefreshIntervar);
		ezRefreshIntervar = -1;
	}
	if(aplinkDisplay && isApLinkiconEnable)
		ezApLink();
			
	//	if(interDisplay)
	//		ezMirrAllow();
			
	if(curSignal < 0){
		ezSignalNull();
	}else if(isIpError){
		ezSignalIpErr(curSignal);
	}else{
		ezSignal(curSignal);
		ezApLink();
	}
	
	ezDongle();

	// Will display in ez5g.
	//if(upgradeDisplay)
		//ezUpgrade();
	
	if(connectNumDisplay)
		ezConnectNumber(connectNum);
		
	if(connectingDisplay)
		ezWifiConnecting();
		
	if(popDisplay)
		ezPop(routerSsid);

	ezshowip();
	
	MainSWF.refreshLocalback();
}
function ezRefresh(){
	//trace("::::::::::: ezRefresh, aplinkDisplay: "+aplinkDisplay+", interDisplay: "+interDisplay);
	if(!MainSWF.isNetDisplayEnable())
		return;
	MainSWF.ezClean();
	if(ezRefreshIntervar > 0){
		clearInterval(ezRefreshIntervar);
		ezRefreshIntervar = -1;
	}
	ezRefreshIntervar = setInterval(ezRefreshDelay, DELAY_TIME);
}

function softapInfoRefresh(){
	if(MainSWF.LAN_ONLY)
		return;
	
	var ssid:String = MainSWF.getSsid();
	var psk:String = MainSWF.getPsk();
	//trace("++++softapInfoRefresh :: ssid =" + ssid);
	//trace("++++softapInfoRefresh :: psk =" + psk);

	if(MainSWF.SSID_DISPLAY || MainSWF.PSK_DISPLAY)
	{
		softap_info._visible = true;
		if(MainSWF.SSID_DISPLAY)
		{
			softap_info.txt0.text = "SSID:";
			softap_info.ssid.text = ssid;
		}
		if(MainSWF.SSID_DISPLAY)
		{
			softap_info.txt1.text = "PSK:";
			softap_info.psk.text = psk;
		}
	}
	isSSIDShow = true;
}

var storage_bar_interval:Number=-1;
function wait_storage_bar()
{
	if(StorageSWF.usb_status_mc._width!=0){
		clearInterval(storage_bar_interval);
		deviceIconRefresh();
		StorageSWF._visible=true;	
	}else{
		trace("wait storage_bar_mc ready..."+StorageSWF.usb_status_mc._width);	
	}
}
function toEZCastOn(){
	trace("*****top bar changes to ezcast*****");
	TopBarSWF._visible=true;
	StorageSWF._visible=false;
	softapInfoRefresh();
}

function toEZCastOff(){
	trace("*****top bar changes to projector*****");
	TopBarSWF._visible=false;	
	storage_bar_interval=setInterval(wait_storage_bar,500)
}

function deviceIconRefresh(){
//	var count:Number = 0;
	
	StorageSWF.all_hide();
	StorageSWF.base_x=0;

	if(!MainSWF.EZCASTPRO_PROJECTOR)
		return;

	trace("----deviceIconRefresh-----");
	if(MainSWF.CardStatus[6].valid){ //wifi dongle
		StorageSWF.add_icon("wifi_dongle_status_mc");
		trace("***wifi dongle status :: " + MainSWF.CardStatus[6].valid);
	}
	
	if(MainSWF.CardStatus[SystemInfo.SYS_MEDIA_UDISK1].valid || MainSWF.CardStatus[SystemInfo.SYS_MEDIA_UDISK2].valid){ // USB
		StorageSWF.add_icon("usb_status_mc");
		trace("***usb status :: 1");
	}
		
	if(MainSWF.CardStatus[SystemInfo.SYS_MEDIA_SD].valid){ // SD/MMC/MS/XD
		StorageSWF.add_icon("sd_status_mc");
		trace("***sd card status :: " + MainSWF.CardStatus[SystemInfo.SYS_MEDIA_SD].valid);
	}
	
	if(CardStatus[SystemInfo.CARDSTATUS_INDEX_FOR_LAN].valid){
		StorageSWF.add_icon("lan_mc");
		trace("***lan status :: " + CardStatus[SystemInfo.CARDSTATUS_INDEX_FOR_LAN].valid);
	}
}

function adjustBarPosition(){
	var offset_x:Number = 20;
	
	WHICHICON_DONGLE_X = WHICHICON_DONGLE_X + offset_x;
	WHICHICON_CONNECTING_X = WHICHICON_CONNECTING_X + offset_x;
	WHICHICON_INTERNET_LINK_X = WHICHICON_INTERNET_LINK_X + offset_x;
	WHICHICON_POP_X = WHICHICON_POP_X + offset_x;
	WHICHICON_UPGRADE_X = WHICHICON_UPGRADE_X + offset_x;
	WHICHICON_INTER_X = WHICHICON_INTER_X + offset_x;
	
	WHICHICON_DONGLE._x = WHICHICON_DONGLE_X;
	WHICHICON_CONNECTING._x = WHICHICON_CONNECTING_X;
	WHICHICON_INTERNET_LINK._x = WHICHICON_INTERNET_LINK_X;
	WHICHICON_POP._x = WHICHICON_POP_X;
	WHICHICON_UPGRADE._x = WHICHICON_UPGRADE_X;
	WHICHICON_INTER._x = WHICHICON_INTER_X;
}

function hideStoragebar(){
	StorageSWF._visible = false;
}

function showStoragebar(){
	StorageSWF._visible = true;
	deviceIconRefresh();
}
// =================================================================
function fontsize_adjust(){
	var sys_w:Number=SystemInfo.getCurscreenParam(1);
	var sys_h:Number=SystemInfo.getCurscreenParam(2);
	var txt_format:TextFormat=new TextFormat();
	var adjust_ratio:Number=1;
	if(sys_w<1920){
		switch(sys_h){
			case 800://1280x800
				adjust_ratio=0.9;
				break;			
			case 768://1024x768
				adjust_ratio=0.8;
				break;
		}
		if(adjust_ratio<1){
			txt_format=softap_info.ssid.getTextFormat();
			txt_format.size=Math.round(txt_format.size*adjust_ratio);
			trace("Main_top_bar txt_format.size"+txt_format.size);							
			softap_info.ssid.setTextFormat(txt_format);
			softap_info.ssid_24g.setTextFormat(txt_format);
			softap_info.txt0.setTextFormat(txt_format);			
			softap_info.txt1.setTextFormat(txt_format);
			softap_info.txtdev.setTextFormat(txt_format);
			softap_info.psk.setTextFormat(txt_format);			
			
		}
	}
}

function setTxtSize(tf:TextField, size:Number){
	var txt_fmt:TextFormat = tf.getTextFormat();
	trace("size("+tf+"): "+txt_fmt.size);
	txt_fmt.size = size;
	tf.setTextFormat(txt_fmt);
}

function modulesInit()
{
	if(!MainSWF.TOPBAR_BG_DISPLAY)
	{
		bg_mc._visible = false;
	}
	if(!MainSWF.LOGO_DISPLAY)
	{
		mode_icon._visible = false;
	}
	
	if(!MainSWF.SSID_DISPLAY && !MainSWF.PSK_DISPLAY)
	{
		softap_info._visible = false;
	}
	else
	{
		if(!MainSWF.SSID_DISPLAY)
		{
			softap_info.ssid_icon_mc._visible = false;
			softap_info.txt0._visible = false;
			softap_info.ssid._visible = false;
			softap_info.ssid_24g._visible = false;
			softap_info.txtdev._visible = false;
			softap_info.hostname._visible = false;
		}
		if(!MainSWF.PSK_DISPLAY)
		{
			softap_info.psk_icon_mc._visible = false;
			softap_info.txt1._visible = false;
			softap_info.psk._visible = false;
		}
	}

	if(!MainSWF.ICON_DISPLAY)
	{
		icon_mc._visible = false;
		mirr_icon_mc._visible = false;
	}
}
function set_ssid_position(mod:Number)
{
	if(mod==0)
	{
		softap_info.ssid._x=SSID_24G_X;
		softap_info.ssid._y=SSID_24G_Y;
		softap_info.txtdev._visible =false;
	}
	else if(mod==1)
	{
		softap_info.ssid._y=SSID_24G_Y+20;
		softap_info.txtdev._visible =true;
	}
	
}
function init(){
	trace("Main_top_bar :: initialization");
	main_bar.mode_icon.upleft_logo_mc._visible=true;
	softap_info.txtdev._visible = false;
	WHICHICON_5G._visible = false;
	if(MainSWF.EZCASTPRO_MODE)fontsize_adjust();
	if(MainSWF.EZWILAN_ENABLE){
		icon_mc.dongle.gotoAndStop(4);//EZCastLANBox
	}

	if(MainSWF.EZCASTPRO_PROJECTOR){
		icon_mc.dongle.gotoAndStop(3);
		mirr_icon_mc.dongle.gotoAndStop(3);
		adjustBarPosition();
		
		if(MainSWF.EZCASTPRO_RUN){
			toEZCastOn();
		}
		else
			toEZCastOff();
			
		StorageSWF.all_hide();
		deviceIconRefresh();
	}
	
	if(!MainSWF.show_softap_flag){
		softap_info._visible = false;
		prompt_info._visible = false;
		isSSIDShow = false;
	}
	softap_info.txt0.text = "SSID:";
	softap_info.txt1.text = "PSK:";

	modulesInit();
	
	main_bar.icon_mc.icon_mc.gotoAndStop(1);

	flaPopClean();
	flaWifiConnectingClean();
	flaUpgradeClean();
	fla5gClean();
	if(MainSWF.getSystemMode2Top()){
		iconMode(ICONMODE_ROUTERALLOW);
	}else{
		iconMode(ICONMODE_APONLY);
	}
	flaApLink();
	flaSignalNull();
	if(!MainSWF.LAN_ONLY){
		//showSSID("WAIT...");
	}else{
		aplinkDisplay = false;
		WHICHICON_AP._visible = false;
		this.icon_mc.phone_mc._visible = false;
	}
	MainSWF.TopbarCallback();
}
init();
/*
function ezConnectNumberclear(){
	if(ezNumInterval > 0){
		clearInterval(ezNumInterval);
		ezNumInterval = -1;
	}
	var x:Number = ICON_MC_OFFSET_X + NUM_X + TEXT_OFFSET_X + 13.8;
	var y:Number = ICON_MC_OFFSET_Y + NUM_Y + TEXT_OFFSET_Y;
	
	var str:String ="";// "[ " + showNum + " ]";
	MainSWF.ezShowText(str, 21, 0x000000, x, y);
}
*/
