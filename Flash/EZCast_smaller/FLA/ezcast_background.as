
import Actions.Locale;
import Actions.SystemInfo;

var isRouterSSIDShowing:Boolean = false;
MainSWF.ezBgShowCompleted=false;
function iconLeftMc(val:Boolean){
	if(val)
	{	
		//trace("show iconLeftMc");
		TopBarSWF.icon_mc.phone_mc._visible = true;
		TopBarSWF.icon_mc.ap_link_mc._visible = true;
		TopBarSWF.icon_mc.num_txt._visible = true;	
	}
	else
	{
		//trace("hide iconLeftMc");
		TopBarSWF.icon_mc.phone_mc._visible = false;
		TopBarSWF.icon_mc.ap_link_mc._visible = false;
		TopBarSWF.icon_mc.num_txt._visible = false;	
	}


}

function updata_connect_icon(){
	var connectMode:Number = MainSWF.get_ConnectMode();
	//trace("updata_connect_icon_connectMode: "+connectMode+"SystemInfo.ROUTER_ONLY: "+SystemInfo.ROUTER_ONLY);
	if(connectMode == SystemInfo.DIRECT_ONLY){
		//trace("connectMode-11111---:  "+connectMode);
		step2_mc.gotoAndStop(1);
		iconLeftMc(true);
	}else if(connectMode == SystemInfo.ROUTER_ALLOW){
		//trace("connectMode--2222---:  "+connectMode);
		step2_mc.gotoAndStop(2);
		iconLeftMc(true);
	}else if(connectMode == SystemInfo.ROUTER_ONLY){
		//trace("connectMode--333-33--:  "+connectMode);
		step2_mc.gotoAndStop(3);
		iconLeftMc(false);
	}
}

function show_websrv_IP(){
	//ip_mc.ip_txt.text = "http://"+SystemInfo.getIP(1);
}

function init(){
	var connectMode:Number = MainSWF.get_ConnectMode();
	trace("connectMode: "+connectMode+"SystemInfo.ROUTER_ALLOW: "+SystemInfo.ROUTER_ALLOW);
	if(connectMode == SystemInfo.DIRECT_ONLY){
		step2_mc.gotoAndStop(1);
		iconLeftMc(true);
	}else if(connectMode == SystemInfo.ROUTER_ALLOW){
		step2_mc.gotoAndStop(2);
		iconLeftMc(true);
	}else if(connectMode == SystemInfo.ROUTER_ONLY){
		if(TopBarSWF.isCurrectStrSsid()){
			step2_mc.gotoAndStop(1);
			iconLeftMc(false);
		}
		else {
			step2_mc.gotoAndStop(3);
			iconLeftMc(false);
		}
	}
	if(MainSWF.isAutoPlayEn() && MainSWF.EZCASTPRO_MODE != 8075){
		step2_mc.dongle_mc.ezchannel_mc.gotoAndStop(2);
		step2_mc.dongle2_mc.ezchannel_mc.gotoAndStop(2);
	}else{
		step2_mc.dongle_mc.ezchannel_mc.gotoAndStop(1);
		step2_mc.dongle2_mc.ezchannel_mc.gotoAndStop(1);
	}
	//cast_code_txt.text = "";
	prompts1_txt.text="Please use 5V/1A adapter.";
	trace("MainSWF.topBarShowCompleted================================="+MainSWF.topBarShowCompleted)
	if(MainSWF.topBarShowCompleted)
		show_websrv_IP();
	MainSWF.ezBgShowCompleted=true;
}

function show_cast_code(code:String){
	if(code == "")
		cast_code_txt.text = "";
	else
		cast_code_txt.text = "Castcode("+code+")";
}
function show_dongle_ssid(ssid:String){
	step2_mc.dongle_2_ssid_txt.text = step2_mc.dongle_ssid_txt.text =ssid;
}
function show_dongle_24gssid(ssid:String){
	step2_mc.dongle_2_ssid24g_txt.text = step2_mc.dongle_ssid24g_txt.text = ssid;
}

function show_dongle_devname(name:String){
	step2_mc.dongle_devname_txt.text = name;
}

function show_dongle_devname2(name:String){
	step2_mc.dongle_devname2_txt.text = name;
}

function show_router_ssid(ssid:String){
	step2_mc.router_2_ssid_txt.text = step2_mc.router_ssid_txt.text = ssid;
}
function show_dongle_ip(ip:String){
	if(step2_mc.dongle_internet_ip_txt._visible == false)
		return;
	
	var txt_x = step2_mc._x + step2_mc.dongle_internet_ip_txt._x;
	var txt_y = step2_mc._y + step2_mc.dongle_internet_ip_txt._y;
	var textlen = MainSWF.lengthCalculationFontSize_25(ip);
	var textOffset = 0;
	if(MainSWF.isEZCastpro())
	{
		if(textlen < 200){
			textOffset = (200-textlen)/2;
		}
		textOffset += 55;
		clean_dongle_ip();
		MainSWF.ezShowText(ip, 25, 0xC9C9C9, txt_x + textOffset, txt_y+5);
	}
	
}
function clean_dongle_ip(){
	var txt_x = step2_mc._x + step2_mc.dongle_internet_ip_txt._x;
	var txt_y = step2_mc._y + step2_mc.dongle_internet_ip_txt._y;
	if(MainSWF.isEZCastpro())
	{
		MainSWF.ezCleanRect(txt_x, txt_y, 300, 30);
	}
}
var _ipaddr:String;
function show_mira_code(ip:String){
	var wlan0_ip:String;
	var miracode:String;
	var miracode_enable:Number;
	_ipaddr=ip;
	miracode_enable=SystemInfo.get_miracode_enable();
	if(miracode_enable==1&&SystemInfo.getConnectMode()!=0&&MainSWF.isEZCastpro())
	{
		miracode=SystemInfo.get_miracode(ip);
		trace("miracode:"+miracode);
		var txt_x = mira_code_txt._x ;
		var txt_y = mira_code_txt._y;
		if(miracode != "error")
		{
			miracode = "Miracode("+miracode+")";
			MainSWF.ezCleanRect(txt_x, txt_y, 380, 54);
			MainSWF.ezShowText(miracode, 45, 0x00CCFF, txt_x, txt_y);
		}
		
		
	}
		
}
function clean_mira_code(){
	var txt_x = mira_code_txt._x ;
	var txt_y = mira_code_txt._y;
	MainSWF.ezCleanRect(txt_x, txt_y, 380, 54);
		
}


function ezMouseShow(){
	var icon_path:String = "/usr/share/ezdata/ezcast_icon_mouse.png"
	var x:Number = mouse_icon._x;
	var y:Number = mouse_icon._y;
	//trace("Mouse.x="+x+"   Mouse.y="+y+"icon_path="+icon_path);
	MainSWF.ezShowPng(icon_path, x, y, mouse_icon._width,mouse_icon._height);
}
function ezMouseClean(){
	var x:Number =  mouse_icon._x;
	var y:Number =  mouse_icon._y;
	//trace("Mouse.x="+x+"   Mouse.y="+y);
	MainSWF.ezCleanRect(x, y, mouse_icon._width,mouse_icon._height);
}

function ezlauncher_icon_show(){
	var icon_path:String = "/usr/share/ezdata/ezlauncher.png"
	var x:Number = ezlauncher_icon._x;
	var y:Number = ezlauncher_icon._y;
	MainSWF.ezShowPng(icon_path, x, y, ezlauncher_icon._width,ezlauncher_icon._height);
}
function ezlauncher_icon_clean(){
	var x:Number =  ezlauncher_icon._x;
	var y:Number =  ezlauncher_icon._y;
	MainSWF.ezCleanRect(x, y, ezlauncher_icon._width,ezlauncher_icon._height);
}



var showSsidInterval:Number = -1;
var _ssid:String = "";
function doRouterSsidShow(ssid:String){
	if(!MainSWF.isNetDisplayEnable()){
		return;
	}
	
	var textlen = MainSWF.lengthCalculationFontSize_25(ssid);
	var textOffset = 0;
	if(textlen < 200){
		textOffset = (200-textlen)/2;
	}
	textOffset += 30;
	if(EZCastSWF.isRouterCtlEn == 0){
		var txt_x = step2_mc._x + step2_mc.router_ssid_txt._x;
		var txt_y = step2_mc._y + step2_mc.router_ssid_txt._y;
		MainSWF.ezShowText(ssid, 25, 0xC9C9C9, txt_x + textOffset, txt_y+5);
	}else{
		if(step2_mc.router_ssid_txt._visible == true){
			var txt_x = step2_mc._x + step2_mc.router_ssid_txt._x;
			var txt_y = step2_mc._y + step2_mc.router_ssid_txt._y;
			MainSWF.ezShowText(ssid, 25, 0xC9C9C9, txt_x + textOffset, txt_y+5);
		}

		if(step2_mc.router_2_ssid_txt._visible == true){
			var txt_x = step2_mc._x + step2_mc.router_2_ssid_txt._x;
			var txt_y = step2_mc._y + step2_mc.router_2_ssid_txt._y;
			MainSWF.ezShowText(ssid, 25, 0xC9C9C9, txt_x + textOffset, txt_y+5);
		}
	}	
	isRouterSSIDShowing = true;
}


function showRouterSsidDelay(){
	if(showSsidInterval > 0){
		clearInterval(showSsidInterval);
		showSsidInterval = -1;
	}
	doRouterSsidShow(_ssid);
	show_mira_code(_ipaddr);
	show_dongle_ip(_ipaddr);
}

function showRouterSsid(ssid:String){
	if(isRouterSSIDShowing){
		cleanRouterSsid();
		clean_mira_code();
		_ssid = ssid;
		if(showSsidInterval > 0){
			clearInterval(showSsidInterval);
			showSsidInterval = -1;
		}
		showSsidInterval = setInterval(showRouterSsidDelay, 200);
	}else{
		doRouterSsidShow(ssid);
	}	
}

function cleanRouterSsid(){

	if(isRouterSSIDShowing){
		if(!MainSWF.isNetDisplayEnable()){
			isRouterSSIDShowing = false;
			return;
		}
		
		isRouterSSIDShowing = false;
		if(EZCastSWF.isRouterCtlEn == 0){
			var txt_x = step2_mc._x + step2_mc.router_ssid_txt._x;
			var txt_y = step2_mc._y + step2_mc.router_ssid_txt._y;
			MainSWF.ezCleanRect(txt_x, txt_y, 300, 30);
		}else{
			var txt_x = step2_mc._x + step2_mc.router_ssid_txt._x;
			var txt_y = step2_mc._y + step2_mc.router_ssid_txt._y;
			MainSWF.ezCleanRect(txt_x, txt_y, 300, 30);

			var txt_x = step2_mc._x + step2_mc.router_2_ssid_txt._x;
			var txt_y = step2_mc._y + step2_mc.router_2_ssid_txt._y;
			MainSWF.ezCleanRect(txt_x, txt_y, 300, 30);
		}
	}
}

init();

