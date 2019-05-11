/**
* @brief The video osd reference implementation for the LSDK.
* @author : Evan Liu 2012.04.18
* @modified : Richard Yu 2012.05.07
*/
import Actions.FileList;
import Actions.CSwfKey;
import Actions.VideoEngine;
import Actions.OsdEngine;
import Actions.SystemInfo;

/*Some global properties and functions.*/
///Properties for the osd.
var osdWidth:Number = (SystemInfo.getCurscreenParam(1)/2)+ 350;//SystemInfo.getCurscreenParam(1);                      /// the width of the osd (Must be less than 448)
var osdHeight:Number = SystemInfo.getCurscreenParam(2) - 200;//SystemInfo.getCurscreenParam(2); 
var subtitle_choose_in_osd:Number = 0;




/***********************************
*initial the osd display
*
************************************/

trace("======osdWidth, osdHeight = " + osdWidth + "" + osdHeight);


function change_the_value_for_OSD(value:Number):Number{
	
	if(value%64){
		value=Math.floor(value/64)*64; //64=>30
		
	}
	return value;
	
}
osdWidth=change_the_value_for_OSD(osdWidth);
var panel_height:Number = 224;//200;//300;
var panel_width:Number = 700;
var osdXpos:Number = (SystemInfo.getCurscreenParam(1)/2)-350;//0;   /// the x position of the osd
//var osdXpos:Number = 80;
trace("osdXpos= "+osdXpos);
if(osdXpos%2){ //if osdXpos is odd
	osdXpos--;
}
var osdYpos:Number = 0;   /// the y position of the osd
//var osdYpos:Number = 800-osdHeight/2;   /// the y position of the osd
trace("osdYpos= "+osdYpos);
trace("getCurscreenParam(2)= "+SystemInfo.getCurscreenParam(2));
var OSD_y:Number=osdHeight-panel_height;//osdHeight-panel_height;//osdHeight-200;
var OSD_x:Number=0;//(SystemInfo.getCurscreenParam(1)-600)/2;
var osdMode:Number = OsdEngine.OSD_MODE_8BITS;          /// the osd mode, currently only 4 bits mode supported.
var osdIconPathBase:String = "/am7x/case/data/";   /// the base path for the osd icons.
//var osdIconPathBase:String = "/mnt/udisk/osdicon/videoicon/"; 
var osdPalette:String = osdIconPathBase + "osd.plt";
var checkStateID:Number = -1;
var bar_ori:Number=23;

function draw_bg()  {
	/*original : 1920 x 1080*/
	/*
	OsdEngine.fillRect(OSD_x,OSD_y,1920,panel_height,0);  //(x,y,w,h,color)
	OsdEngine.updateOsdRect(OSD_x,OSD_y,1920,panel_height);
	*/
	trace("draw_bg ::: OSD_x = " + OSD_x + " OSD_y=" + OSD_y);
	OsdEngine.showOsdIcon(OSD_x,OSD_y-22,osdIconPathBase + "new_bg_RGB_Default.bin",1);
	/*end : original : 1920 x 1080*/
}

function show_test_result(){
	/*original : 1920 x 1080*/
	/*
	OsdEngine.fillRect(OSD_x,OSD_y+100,1920,20,3);  //(x,y,w,h,color)
	OsdEngine.updateOsdRect(OSD_x,OSD_y+100,1920,20);
	*/
	trace("draw_bg ::: OSD_x = " + OSD_x + " OSD_y=" + OSD_y + " bar_ori=" + bar_ori);
	var test_enable:String="";
	test_enable = SystemInfo.get_test_config(SystemInfo.GET_TEST_EDID_CONF_ENABLE);
	trace("SystemInfo.GET_TEST_EDID_CONF_ENABLE: "+test_enable);
	if(test_enable.toLowerCase() == SystemInfo.RETURN_VALUE_TRUE){
		trace("EDID test enable!!!");
	 	var edid=SystemInfo.sysinfo_test_edid();
		trace("edid="+edid);
		OsdEngine.showOsdString(OSD_x+bar_ori,OSD_y+10,200,26,"Edid_Resolution:",1);
		OsdEngine.updateOsdRect(OSD_x+bar_ori,OSD_y+10,200,26);
		if(edid=="EDID_FAIL")
		{
			OsdEngine.showOsdString(OSD_x+bar_ori+200,OSD_y+10,150,26,"EDID_FAIL",5);
		}
		else
		{
			OsdEngine.showOsdString(OSD_x+bar_ori+200,OSD_y+10,150,26,"EDID_OK",6);
		}
		OsdEngine.updateOsdRect(OSD_x+bar_ori+200,OSD_y+10,150,26);

	}else{
		trace("EDID test Disable!!!");
	}
	test_enable = SystemInfo.get_test_config(SystemInfo.GET_TEST_HDCPKEY_CONF_ENABLE);
	trace("SystemInfo.GET_TEST_HDCPKEY_CONF_ENABLE: "+test_enable);
	if(test_enable.toLowerCase() == SystemInfo.RETURN_VALUE_TRUE){
		trace("HDCPKEY test enable!!!");
		var hdcpkey:String="";
	 	hdcpkey=SystemInfo.sysinfo_test_hdcpkey();
		//trace("hdcpkey="+hdcpkey);
		OsdEngine.showOsdString(OSD_x+bar_ori+350,OSD_y+10,150,26,"HDCP_KEY:",1);
		OsdEngine.updateOsdRect(OSD_x+bar_ori+350,OSD_y+10,150,26);
		if(hdcpkey=="HDCPKEY_FAIL")
			OsdEngine.showOsdString(OSD_x+bar_ori+500,OSD_y+10,200,26,hdcpkey,5);
		else
			OsdEngine.showOsdString(OSD_x+bar_ori+500,OSD_y+10,200,26,hdcpkey,6);
		OsdEngine.updateOsdRect(OSD_x+bar_ori+500,OSD_y+10,200,26);

	}else{
		trace("HDCPKEY test Disable!!!");
	}
	trace("version_test_enable================="+ _global.version_test_enable);
	if(_global.version_test_enable)
	{
		OsdEngine.showOsdString(OSD_x+bar_ori,OSD_y+60,160,26,"VERSION TEST:",1);
		OsdEngine.updateOsdRect(OSD_x+bar_ori,OSD_y+60,160,26);
		if(Number(_global.test_config_version) == Number(_global.local_version))
		{
			OsdEngine.showOsdString(OSD_x+bar_ori+160,OSD_y+60,100,26,"SUCCESS",6);
			OsdEngine.showOsdString(OSD_x+bar_ori,OSD_y+60+40,150,26,"Test version:",1);
			OsdEngine.updateOsdRect(OSD_x+bar_ori,OSD_y+60+40,150,26);
			OsdEngine.showOsdString(OSD_x+bar_ori+150,OSD_y+60+40,130,26,_global.test_config_version,6);
			OsdEngine.updateOsdRect(OSD_x+bar_ori+150,OSD_y+60+40,130,26);
			OsdEngine.showOsdString(OSD_x+bar_ori+320,OSD_y+60+40,150,26,"Real version:",1);
			OsdEngine.updateOsdRect(OSD_x+bar_ori+320,OSD_y+60+40,150,26);
			OsdEngine.showOsdString(OSD_x+bar_ori+470,OSD_y+60+40,130,26,_global.local_version,6);
			OsdEngine.updateOsdRect(OSD_x+bar_ori+470,OSD_y+60+40,130,26);
			
		}
		else
		{
			OsdEngine.showOsdString(OSD_x+bar_ori+160,OSD_y+60,100,26,"FAIL",5);
			OsdEngine.updateOsdRect(OSD_x+bar_ori+160,OSD_y+60,100,26);
			OsdEngine.showOsdString(OSD_x+bar_ori,OSD_y+60+40,150,26,"Test version:",1);
			OsdEngine.updateOsdRect(OSD_x+bar_ori,OSD_y+60+40,150,26);
			OsdEngine.showOsdString(OSD_x+bar_ori+150,OSD_y+60+40,130,26,_global.test_config_version,5);
			OsdEngine.updateOsdRect(OSD_x+bar_ori+150,OSD_y+60+40,130,26);
			OsdEngine.showOsdString(OSD_x+bar_ori+320,OSD_y+60+40,150,26,"Real version:",1);
			OsdEngine.updateOsdRect(OSD_x+bar_ori+320,OSD_y+60+40,150,26);
			OsdEngine.showOsdString(OSD_x+bar_ori+470,OSD_y+60+40,130,26,_global.local_version,5);
			OsdEngine.updateOsdRect(OSD_x+bar_ori+470,OSD_y+60+40,130,26);
			
		}
			
		
	}
	trace("language_config_enable: "+_global.language_config_enable);
	if(_global.language_config_enable){
		trace("languagetest enable!!!");
		var hdcpkey:String="";
	 	hdcpkey=SystemInfo.sysinfo_test_hdcpkey();
		//trace("hdcpkey="+hdcpkey);
		OsdEngine.showOsdString(OSD_x+bar_ori,OSD_y+150,180,26,"Set Language:",1);
		OsdEngine.updateOsdRect(OSD_x+bar_ori,OSD_y+150,180,26);
		if(_global.language_test_result=="")
			return;
		if(_global.language_test_support)
			OsdEngine.showOsdString(OSD_x+bar_ori+180,OSD_y+150,400,26,_global.language_test_result,6);
		else
			OsdEngine.showOsdString(OSD_x+bar_ori+180,OSD_y+150,400,26,_global.language_test_result,5);
		OsdEngine.updateOsdRect(OSD_x+bar_ori+180,OSD_y+150,400,26);

	}else{
		trace("HDCPKEY test Disable!!!");
	}
	
	
	/*end : original : 1920 x 1080*/
}

function initOSD(){

	/*original :1920 x 1080*/
	//OsdEngine.clearOsdRect(0,OSD_y,1920,panel_height);  //(x,y,w,h,color)
	//OsdEngine.updateOsdRect(0,OSD_y,1920,panel_height);
	trace("OSD_x=" + OSD_x + " OSD_y=" + OSD_y + " panel_width=" + panel_width + " panel_height=" + panel_height);
	OsdEngine.clearOsdRect(OSD_x,OSD_y,panel_width,panel_height);  //(x,y,w,h,color)
	OsdEngine.updateOsdRect(OSD_x,OSD_y,panel_width,panel_height);
	/*end : original :1920 x 1080*/
	draw_bg();
}


function update_progress()  {

	//var videoNowStates:Number = VideoEngine.GetState();
   
	trace("_global.osdStatueUpdateCount =============== "+_global.osdStatueUpdateCount+"=============================================="+_global.osdStatueUpdateCount);
	
	
	if(_global.osdStatueUpdateCount == -1){
		initOSD();
		show_test_result();
		_global.osdStatueUpdateCount = 0;
		clearInterval(checkStateID);
		checkStateID = -1;
		
	}
}

this.onUnload = function(){  //be called while FlashEngine.Stop(false);
       
		clearInterval(checkStateID);
		checkStateID = -1;
		//trace("----------------------------------------------------------------------------------close fn");
}



//===============  Main code  ==============================================
//trace("<V_OSD>:init draw video OSD");

//checkStateID = 0;

_global.osdStatueUpdateCount = -1;


trace("osdXpos=" + osdXpos + " osdYpos=" + osdYpos + " OSD_x=" + OSD_x + " OSD_y=" + OSD_y + " osdWidth=" + osdWidth + " osdHeight=" + osdHeight + " osdPalette=" + osdPalette);
OsdEngine.initOsd(osdXpos,osdYpos,osdWidth,osdHeight,osdMode,osdPalette);
checkStateID = setInterval(update_progress, 500);







