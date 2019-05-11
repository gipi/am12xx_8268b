import Actions.VideoEngine;
import Actions.Locale;
import Actions.SystemInfo;
import Actions.CSwfKey;
import Actions.OsdEngine;
import Actions.FileList;
import Actions.WifiEngine;
import Actions.AudioEngine;
import Actions.PhotoEngine;
import Actions.FlashEngine;

//var tp:Number=0;
var io_flag:Boolean = false;
var flag:Number =1;
var i:Number=0;
var j:Number=0;
var File_test:String ="videoloopplayback";
var cur_index:Number=0;
var file:File=new File();
var File_num:Number =0;
var i:Number = 0;
var filename:String;
var File_name:Array=new Array();
var File_path:Array=new Array();
var searchType:String;
var status_check_ID:Number = -1;
var OSD_interval_ID:Number = -1;
var EZ_media:String;
var Playratio:Number;
var Playmode:Number;
var error_time:Number=0;
var engine_state:Number=-1;

var stopvideo_flag:Number = 0;
var videoopen:Boolean;
videoopen=VideoEngine.Open();
var ts_splitter_name:String="ts_splitter.swf";
var test_config_ssid:String;
var test_config_psk:String;
var test_config_video_name:String = "videoloopplayback.mp4";
var test_config_audio_name:String = "videoloopplayback.mp3";
var test_config_loop_num:Number = 5;
var test_config_wifi_standard:Number = 50;
var test_config_version:String;
var test_config_language:String;
var local_version:String;
var test_loop_count:Number = 0;
var wifi_test_enable:Boolean = false;
var video_test_enable:Boolean = false;
var version_test_enable:Boolean = false;
var language_config_enable:Boolean = false;
var play_status:Boolean = false;
var wifi_connect_timer:Number = -1;
var cur_connect_index:Number;
var start_wifi_timer:Number = -1;
var infiniteSet:Boolean = false;
var scan_count:Number = 0;
var wifi_check_index:Number = -1;
var wifi_check_count:Number = 22;
var total_ap:Number = 0;
var channelsignal_data1:Number=0;
var channelsignal_data6:Number=0;
var channelsignal_data11:Number=0;

var channelsignal_data36:Number=0;
var channelsignal_data100:Number=0;
var channelsignal_data157:Number=0;

var edid="";
var videoOsdName:String;
videoOsdName = "testOSD.swf";
//var wifi_throughput:String ="";
var hdmi_modes:Array;

 hdmi_modes = ["1920*1080_60I","1920*1080_50I","1920*1080_30P","1920*1080_25P","1920*1080_24P"
						 ,"1280*800_60P","1280*800_30P","1280*800_50P","1280*800_25P","1280*800_24P"
						 ,"1920*1152_50I","720*480_60I","720*576_50I","720*480_60P","720*576_50P"
						 ,"1024*768_60P","1280*800_60P","1920x1080_60P","DEFAULT_OUTPUT_RESOLUTION"];

var EZCASTPRO_MODE:Number=SystemInfo.ezcastpro_read_mode();
trace("EZCASTPRO_MODE=="+EZCASTPRO_MODE);   
searchType="AVI avi MP4 MP3 3GP MOV MPG MPEG MOD VOB TS TP M2T M2TS MTS WMV MKV RM RMVB DIVX 3G2 M4V FLV";
EZ_media="video";
Playratio=VideoEngine.GetPlayRatio();
Playmode=VideoEngine.GetPlayMode();

SystemInfo.MemCheck();
var audioopen:Boolean=AudioEngine.Open();
//Playmode=AudioEngine.GetPlayMode();  //1-> sequence play, 0-> only play this song
//SYS_Playmode = AudioEngine.GetPlayMode();

MainSWF.backMenu=false;
lightbar_mc._visible=false;
black_mc._visible=false;

MainSWF.videoOsdRelease=0;
MainSWF.videofilename="";

_global.SubTitleParser_Handle=0; //-- Paul's subtitle ?why reset the handle 
_global.srt_supprot = -1; //-- Paul's subtitle
_global.SubTitleParser_Handle=-1;

MainSWF.usermanualVisible(false);
wifi_test_mc._visible = false;
version_test_mc._visible = false;
languageset_mc._visible=false;
lanset_mc._visible=false;
uartset_mc._visible=false;
edid_test_mc._visible=false;
hdcpkey_test_mc._visible=false;
video_test_mc._visible = false;
wifi_signal_mc._visible = false;
waiting_txt._visible = true;


Factorytest_txt._visible=false;

wifithrough_mc._visible=false;
wifisignal_ch1._visible=false;
wifisignal_ch6._visible=false;
wifisignal_ch11._visible=false;

wifisignal_ch36._visible=false;
wifisignal_ch100._visible=false;
wifisignal_ch157._visible=false;


function show_media_test_result(){
	video_test_mc.test_result_txt.textColor = 0x00ff00;
	video_test_mc.test_result_txt.text = "SUCCESS"
}
function show_media_test_wait(){
	video_test_mc.test_result_txt.textColor = 0x00ff00;
	video_test_mc.test_result_txt.text = "TESTING"
}

function show_media_test_result_fail(){
	video_test_mc.test_result_txt.textColor = 0xff0000;
	video_test_mc.test_result_txt.text = "FAIL"
}
function audio_status_check(){

	var cur_status=AudioEngine.GetState();
	if(engine_state!=cur_status){
		engine_state=cur_status;
	}
	switch(cur_status){		
		case AudioEngine.VE_STOP:
		case AudioEngine.VE_IDLE:
			AudioEngine.Stop();
			if(test_config_loop_num == 0){
				if(io_flag)
					playaudio(0);
			}else if(test_loop_count > 0){
				test_loop_count--;
				if(io_flag)
					playaudio(0);
			}else{
				if(play_status){
					stopaudio();
					show_media_test_result();
					SystemInfo.set_factory_test_video_end(0);
				}
			}
			break;
		case AudioEngine.VE_ERROR:
			stopaudio();
			show_media_test_result_fail();
			break;
	}

}

function status_check(){
	
	var cur_status=VideoEngine.GetState();

	if(engine_state!=cur_status){
		engine_state=cur_status;
	}
	switch(cur_status){		
		case VideoEngine.VE_STOP:
		case VideoEngine.VE_IDLE:
			VideoEngine.Stop();
			if(test_config_loop_num == 0){
				if(io_flag)
					playvideo(0);
			}else if(test_loop_count > 0){
				test_loop_count--;
				if(io_flag)
					playvideo(0);
			}else{
				if(play_status){
					stopvideo();
					show_media_test_result();
					SystemInfo.set_factory_test_video_end(0);
				}
			}
			break;
		case VideoEngine.VE_ERROR:
			stopvideo();
			show_media_test_result_fail();
			break;
	}
}

function playaudio(index:Number){
	black_mc._visible=true;
	//stopaudio();
	var audiofile_type:Number=1;
	// list_file();
	trace("videofile_type================"+audiofile_type);
	var io_flag1 = SystemInfo.checkUsb1Status();
	var io_flag2 = SystemInfo.checkUsb2Status();//
	trace("io_flag1:"+io_flag1);
	trace("io_flag1:"+io_flag2);
	//trace("io_flag3:"+io_flag3);
	//trace("io_flag4:"+io_flag4);
	if(io_flag1 !=0 || io_flag2 != 0)
		io_flag = true;
	else
		io_flag = false;
	var filePath:String=File_path[index];
	if((videofile_type!=0) && io_flag){
		//trace("listd_mc.output_Url[index]==================="+listd_mc.output_Url[index]);
		var audioOK:Boolean=AudioEngine.SetFile(File_path[0]);
		////trace("index= "+index+" page= "+page+" audioOK?"+audioOK);
	if(audioOK==0){ 
		trace("audio play!!");	
		 //update the vol bar
		AudioEngine.Play();
		if(status_check_ID!=null)
				clearInterval(status_check_ID);
		status_check_ID = setInterval(audio_status_check, 500);
		}else{//show not support
		
			////trace("can't play");
			black_mc._visible=false;
			trace("unsupported audio format");
			stopaudio();
			show_media_test_result_fail();
		}
	}
	else
	{
		stopaudio();
		show_media_test_result_fail();
	}
	
//	////trace("set status_check_ID= "+status_check_ID);
}
function playvideo(index:Number){
	black_mc._visible=true;
	var videofile_type:Number=1;
	// list_file();
	trace("videofile_type================"+videofile_type);
	//trace(File_num);
	//trace("current file"+File_name[index]);
	trace(File_path[index]);
    var io_flag1 = SystemInfo.checkUsb1Status();
	var io_flag2 = SystemInfo.checkUsb2Status();//
	trace("io_flag1:"+io_flag1);
	trace("io_flag1:"+io_flag2);
	//trace("io_flag3:"+io_flag3);
	//trace("io_flag4:"+io_flag4);
	if(io_flag1 !=0 || io_flag2 != 0)
		io_flag = true;
	else
		io_flag = false;
	var filePath:String=File_path[index];
	if((videofile_type!=0) && io_flag){
		var videoOK:Boolean=VideoEngine.SetFile(filePath);
		_global.MainSWF.playing_video_url=filePath;
		//trace("videoOK===================="+videoOK);
		if(videoOK){
			play_status = true;
			VideoEngine.Play(Playmode, Playratio);
			if(SystemInfo.get_test_config(SystemInfo.GET_TEST_VIDEO_OSD)==SystemInfo.RETURN_VALUE_TRUE)
			{
				config_laguage();
				FlashEngine.Stop(false);
				var video_osdw = SystemInfo.getCurscreenParam(1);
				var video_osdh= SystemInfo.getCurscreenParam(2);
				var video_osdx0 = 0;
				var video_osdy0= 0;
				FlashEngine.Play(videoOsdName,true,video_osdx0,video_osdy0,video_osdw,video_osdh,30);
			}
				
			//if(OSD_interval_ID > 0){
			//	clearInterval(OSD_interval_ID);
			//	OSD_interval_ID = -1;
			//}
			//OSD_interval_ID = setInterval(OSD_check_state, 500);
			error_time=0;
			trace("clear Interval ");
			clearInterval(status_check_ID);
			status_check_ID=-1;
			status_check_ID = setInterval(status_check, 500);
			
		}else{//show not support
			//black_mc._visible=false;
	        stopvideo();
			show_media_test_result_fail();
		
       }
	}else{
		//trace("play next video");	
		stopvideo();
		show_media_test_result_fail();
	}
	
}

 
// FileList.putsearchfla(0);// search include child path
/* search the vedio file in one path*/
/*
function list_file()
{
	//FileList.putsearchflag(0);
	FileList.setPath("/mnt/usb1","MKV RMVB FLV MP4 AVI 3GP MOV MPG MPEG MOD VOB TS TP M2T M2TS MTS WMV ");
  	File_num = FileList.getTotal();	
 	trace("totalfile:"+File_num);
 	for(i=0;i< File_num;i++){
		File_name[i] = FileList.getFileName(i);
 		trace(File_name);
 		File_path[i] = FileList.getFilePath(i);
 		trace(File_path);
 	}
}
*/
//get test full pathname  form usb1
function search_testfile():String 
{
	var usb_path = SystemInfo.get_test_config(SystemInfo.GET_TEST_USB_PATH);
	FileList.setPath(usb_path,"MKV RMVB FLV MP4 MP3 AVI 3GP MOV MPG MPEG MOD VOB TS TP M2T M2TS MTS WMV ");
	File_num = FileList.getTotal();	
	trace("media file total"+File_num); 
	if(File_num>0)
	{
		//trace("test file"File_num);
		
		for(i=0;i< File_num;i++)
		{
			filename=FileList.getFileName(i);
			trace("[get_test_config in test.as] -- filename: "+filename+", test_config_video_name: "+test_config_video_name);
			if(filename == test_config_video_name){
				flag = 1;
				return FileList.getFilePath(i);
			}
			else if(filename == test_config_audio_name){
				flag = 2;
				return FileList.getFilePath(i);
			}
			else{
				flag = 0;
			}
		}
			
	}
		
}
  

 function stopvideo(){
	if(status_check_ID > 0){
		clearInterval(status_check_ID);
		status_check_ID = -1;
	}
	play_status = false;
	if(SystemInfo.get_test_config(SystemInfo.GET_TEST_VIDEO_OSD)==SystemInfo.RETURN_VALUE_TRUE)
	{
		OsdEngine.clearOsd();
		OsdEngine.updateOsd();
		OsdEngine.releaseOsd();
		FlashEngine.Stop(false);
	}
	
	VideoEngine.Stop();
	//test_after_video();
}
 function stopaudio(){


 if(status_check_ID > 0){
		 clearInterval(status_check_ID);
		 status_check_ID = -1;
	 }
	 play_status = false;
	 AudioEngine.Stop();
 }

 
 //exit swf
 function currentswf_out(){
 	trace("___________ video.swf out");
	if(flag==1)
	{
		stopvideo();
		VideoEngine.Close(); 
	}
	else if(flag==2)
	{

		stopaudio();
		AudioEngine.Close(); 
	}
}

function get_test_config(){
	var test_enable:String;

//	trace("[get_test_config in test.as] -- SystemInfo.GET_TEST_SSID: "+SystemInfo.GET_TEST_SSID+", SystemInfo.GET_TEST_PSK: "+SystemInfo.GET_TEST_PSK+", SystemInfo.GET_TEST_VIDEO_FILE: "+SystemInfo.GET_TEST_VIDEO_FILE+", SystemInfo.GET_TEST_LOOP_NUM: "+SystemInfo.GET_TEST_LOOP_NUM);
	test_enable = SystemInfo.get_test_config(SystemInfo.GET_TEST_WIFI_ENABLE);
	trace("SystemInfo.GET_TEST_WIFI_ENABLE: "+test_enable);
	if(test_enable.toLowerCase() == SystemInfo.RETURN_VALUE_TRUE){
		trace("wifi test enable!!!");
		wifi_test_enable = true;
		test_config_ssid = SystemInfo.get_test_config(SystemInfo.GET_TEST_SSID);
		test_config_psk = SystemInfo.get_test_config(SystemInfo.GET_TEST_PSK);
		var wifi_standard_str = SystemInfo.get_test_config(SystemInfo.GET_TEST_SIGNAL_STANDARD);
		test_config_wifi_standard = Number(wifi_standard_str);
		trace("[get_test_config in test.as] -- test_config_ssid: "+test_config_ssid+", test_config_psk: "+test_config_psk+", test_config_wifi_standard: "+test_config_wifi_standard);
		wifi_test_mc._visible = true;
		wifi_signal_mc._visible = true;
	}
	else{
		trace("wifi test disable!!!");
		wifi_test_enable = false;
		wifi_test_mc._visible = false;
		wifi_signal_mc._visible = false;
	}
	
	test_enable = SystemInfo.get_test_config(SystemInfo.GET_TEST_VIDEO_ENABLE);
	trace("SystemInfo.GET_TEST_VIDEO_ENABLE: "+test_enable);
	if(test_enable.toLowerCase() == SystemInfo.RETURN_VALUE_TRUE){
		trace("video test enable!!!");
		video_test_enable = true;
		test_config_video_name = SystemInfo.get_test_config(SystemInfo.GET_TEST_VIDEO_FILE);
		var num = SystemInfo.get_test_config(SystemInfo.GET_TEST_LOOP_NUM);
		test_config_loop_num = Number(num);
		if(test_config_loop_num == 0){
			infiniteSet = true;
			trace("Video infinite loop test!!!");
		}
		test_loop_count = test_config_loop_num-1;
		trace("[get_test_config in test.as] -- test_config_video_name: "+test_config_video_name+", test_config_loop_num: "+test_config_loop_num);
		video_test_mc._visible = true;
	}
	else{
		trace("video test disable!!!");
		video_test_enable = false;
		video_test_mc._visible = false;
	}
	
	test_enable = SystemInfo.get_test_config(SystemInfo.GET_TEST_VERSION_ENABLE);
	trace("SystemInfo.GET_TEST_VERSION_ENABLE: "+test_enable);
	if(test_enable.toLowerCase() == SystemInfo.RETURN_VALUE_TRUE){
		trace("version test enable!!!");
		version_test_enable = true;
		version_test_mc._visible = true;
		test_config_version = SystemInfo.get_test_config(SystemInfo.GET_TEST_VERSION);
		local_version = SystemInfo.getOtaLocalVersion();
		_global.test_config_version=test_config_version;
		_global.local_version=local_version;
		
	}else{
		trace("version test disable!!!");
		version_test_enable = false;
		version_test_mc._visible = false;
	}
	_global.version_test_enable=version_test_enable;
	
	
	test_enable = SystemInfo.get_test_config(SystemInfo.GET_TEST_LAG_CONF_ENABLE);
	trace("SystemInfo.GET_TEST_LAG_CONF_ENABLE: "+test_enable);
	if(test_enable.toLowerCase() == SystemInfo.RETURN_VALUE_TRUE){
		trace("language test enable!!!");
		language_config_enable = true;
		//version_test_mc._visible = true;
		test_config_language = SystemInfo.get_test_config(SystemInfo.GET_TEST_CONFIG_LANGUAGE);
		//local_version = SystemInfo.getOtaLocalVersion();
	}else{
		trace("language test disable!!!");
		language_config_enable = false;
		//version_test_mc._visible = false;
	}
	_global.language_config_enable=language_config_enable;
	
	test_enable = SystemInfo.get_test_config(SystemInfo.GET_TEST_EDID_CONF_ENABLE);
	
	trace("SystemInfo.GET_TEST_EDID_CONF_ENABLE: "+test_enable);
	if(test_enable.toLowerCase() == SystemInfo.RETURN_VALUE_TRUE){
		trace("EDID test enable!!!");
	 	edid=SystemInfo.sysinfo_test_edid();
		trace("edid="+edid);
		if(edid=="EDID_FAIL")
			edid_test_mc.test_result_txt.textColor =0xff0000;
		else
			edid_test_mc.test_result_txt.textColor = 0x00ff00;
		edid_test_mc.test_result_txt.text=edid;
		edid_test_mc._visible=true;

	}else{
		edid_test_mc._visible=false;
		trace("EDID test Disable!!!");
	}
	test_enable = SystemInfo.get_test_config(SystemInfo.GET_TEST_HDCPKEY_CONF_ENABLE);
	
	trace("SystemInfo.GET_TEST_HDCPKEY_CONF_ENABLE: "+test_enable);
	if(test_enable.toLowerCase() == SystemInfo.RETURN_VALUE_TRUE){
		trace("HDCPKEY test enable!!!");
		var hdcpkey:String="";
	 	hdcpkey=SystemInfo.sysinfo_test_hdcpkey();
		trace("hdcpkey="+hdcpkey);
		if(hdcpkey=="HDCPKEY_FAIL")
			hdcpkey_test_mc.test_result_txt.textColor =0xff0000;
		else
			hdcpkey_test_mc.test_result_txt.textColor = 0x00ff00;
		hdcpkey_test_mc.test_result_txt.text=hdcpkey;
		hdcpkey_test_mc._visible=true;

	}else{
		hdcpkey_test_mc._visible=false;
		trace("HDCPKEY test Disable!!!");
	}
	

	

	waiting_txt._visible = false;
	//trace("[get_test_config in test.as] -- test_config_ssid: "+test_config_ssid+", test_config_psk: "+test_config_psk+", test_config_video_name: "+test_config_video_name+", test_config_loop_num: "+test_config_loop_num);
}

var test_timer:Number=0;
var factory_test_timer:Number=0;
var old_item:Number=0;
var fact_language_result:String="";//success[]  fail
var fact_version_result:String="";//success[]    fail[local version]
var old_edid:String="";
var old_through:String="";
var old_wifi_channel_data:Number=0;
var show_timer:Number=0;

Entertest_mode();
function Entertest_mode()
{

	trace("_global.MainSWF.test_mode"+_global.MainSWF.test_mode);

	//auto test mode 
	
	//factory test mode
	if((_global.MainSWF.test_mode)==1)
	{
		trace("---Factory Test Mode---");
		Factorytest_txt._visible=true;
		waiting_txt._visible = true;
		
		channelsignal_data1=0;
		channelsignal_data6=0;
		channelsignal_data11=0;
		if(EZCASTPRO_MODE == 8075)
		{
			channelsignal_data36=0;
			channelsignal_data100=0;
			channelsignal_data157=0;
		}
		
		clearInterval(factory_test_timer);
		factory_test_timer=-1;
		factory_test_timer=setInterval(start_factory_test, 3000);
	}
	else
	{
		trace("---Auto Test Mode---");
		Factorytest_txt._visible=false;
		test_timer=setInterval(start_test, 2000);
	}
	
}




function start_factory_test(){
	var count:Number=0;
	Factorytest_txt._visible=true;
	var testitem:Number=SystemInfo.sysinfo_itemlist_factorytest();
	trace("testitem"+testitem);
	if(testitem==0)
	{

		trace("---Waiting test result  ---");
		version_test_mc._visible = false;
		languageset_mc._visible=false;
		edid_test_mc._visible=false;
		hdcpkey_test_mc._visible=false;
		wifithrough_mc._visible=false;
		wifisignal_ch1._visible=false;
		wifisignal_ch6._visible=false;
		wifisignal_ch11._visible=false;
		
		wifisignal_ch36._visible=false;
		wifisignal_ch100._visible=false;
		wifisignal_ch157._visible=false;
		
		waiting_txt._visible = true;
		old_edid="";
		old_through="";
		old_wifi_channel_data=0;
		fact_language_result="";
		fact_version_result="";
		//show_timer=0;
	}else{//start show test result
	waiting_txt._visible =false;
	if((testitem%10)>0)
		wifithrough_mc._visible=true;
	else
		wifithrough_mc._visible=false;
	if((testitem%100)/10>=1)
	{
		wifisignal_ch1._visible=true;
		wifisignal_ch6._visible=true;
		wifisignal_ch11._visible=true;
		if(EZCASTPRO_MODE == 8075)//for probox
		{
			wifisignal_ch36._visible=true;
			wifisignal_ch100._visible=true;
			wifisignal_ch157._visible=true;
		}
	}
	else
	{
		wifisignal_ch1._visible=false;
		wifisignal_ch6._visible=false;
		wifisignal_ch11._visible=false;
		
		wifisignal_ch36._visible=false;
		wifisignal_ch100._visible=false;
		wifisignal_ch157._visible=false;
	}
	if((testitem%1000)/100>=1)
		edid_test_mc._visible=true;
	else
		edid_test_mc._visible=false;
	if((testitem%10000)/1000>=1)
		version_test_mc._visible=true;
	else
		version_test_mc._visible=false;
	if((testitem/10000)>1)
		languageset_mc._visible=true;
	else
		languageset_mc._visible=false;
	var factorytest_version:String="";
	var factorytest_language:String="";
	var factorytest_edid:String="";
	factorytest_edid=SystemInfo.sysinfo_get_edid_factorytest();
	if((factorytest_edid!="" )&&(old_edid!=factorytest_edid))
	{
		trace("factorytest_edid"+factorytest_edid);
		edid_test_mc.test_result_txt.textColor = 0x00ff00;
		edid_test_mc.test_result_txt.text=factorytest_edid;
		edid_test_mc._visible=true;
		old_edid=factorytest_edid;
	}
	factorytest_version=SystemInfo.sysinfo_version_factorytest();
	if(factorytest_version!="")
	{
	
		trace("factorytest_version"+factorytest_version);
		version_test_enable=true;
		version_test_mc._visible = true;
		test_config_version=factorytest_version;
		local_version= SystemInfo.getOtaLocalVersion();
		version_test();
		SystemInfo.sysinfo_version_result_factorytest(fact_version_result);
		version_test_enable=false;
		
	}
	factorytest_language=SystemInfo.sysinfo_language_factorytest();
	if(factorytest_language!="")
	{
		trace("factorytest_language"+factorytest_language);
		language_config_enable=true;
		languageset_mc._visible = true;
		test_config_language=factorytest_language;
		config_laguage();
		SystemInfo.sysinfo_language_result_factorytest(fact_language_result);
		language_config_enable=false;
		
	}
	var wifi_throughput:String="";
	wifi_throughput=SystemInfo.sysinfo_get_Throughput();
	if((wifi_throughput!="") &&(old_through!=wifi_throughput))
	{
		//wifithrough_mc.test_result_txt.text="";
		old_through=wifi_throughput;
		trace("--wifi_throughput:--"+wifi_throughput);
		wifithrough_mc.test_result_txt.textColor = 0x00ff00;
		wifi_throughput=wifi_throughput+"Mbits/sec";
		wifithrough_mc.test_result_txt.text=wifi_throughput;
		wifithrough_mc._visible=true;
		
	}
	var lan_result:String="";
	lan_result=SystemInfo.sysinfo_get_lan_factorytest();
	if(lan_result!="")
	{
		//trace("factorytest_lan:"+lan_result);
		lanset_mc._visible = true;
		lanset_mc.lan_txt.text="Lan Test Result:"
		lanset_mc.test_result_txt.textColor = 0x00ff00;
		lanset_mc.test_result_txt.text=lan_result;
		lanset_mc._visible=true;
		
	}
	var uart_result:String="";
	uart_result=SystemInfo.sysinfo_get_uart_factorytest();
	if(uart_result!="")
	{
		//trace("factorytest_lan:"+lan_result);
		uartset_mc._visible = true;
		uartset_mc.lan_txt.text="UART2 Test Result:"
		uartset_mc.test_result_txt.textColor = 0x00ff00;
		uartset_mc.test_result_txt.text=uart_result;
		uartset_mc._visible=true;
		
	}
	var channelsignal_data:Number=SystemInfo.sysinfo_get_channel_signal();
	//var data=channelsignal_data;
	//trace("channelsignal_data:"+channelsignal_data);
	if((channelsignal_data>0) &&(old_wifi_channel_data!=channelsignal_data))
	{
		
		if(channelsignal_data>500)
		{
			channelsignal_data157=channelsignal_data-500;
			trace("channelsignal_data157:"+channelsignal_data157);
			wifisignal_ch157._visible=true;
			wifisignal_ch157.test_result_txt.textColor = 0x00ff00;
			wifisignal_ch157.test_result_txt.text=channelsignal_data157;
			
		}
		else if(channelsignal_data>400)
		{
			channelsignal_data100=channelsignal_data-400;
			trace("channelsignal_data100:"+channelsignal_data100);
			wifisignal_ch100._visible=true;
			wifisignal_ch100.test_result_txt.textColor = 0x00ff00;
			wifisignal_ch100.test_result_txt.text=channelsignal_data100;
			
		}
		else if(channelsignal_data>300)
		{
			channelsignal_data36=channelsignal_data-300;
			trace("channelsignal_data36:"+channelsignal_data30);
			wifisignal_ch36._visible=true;
			wifisignal_ch36.test_result_txt.textColor = 0x00ff00;
			wifisignal_ch36.test_result_txt.text=channelsignal_data36;
			
		}
		else if(channelsignal_data>200)
		{
			channelsignal_data11=channelsignal_data-200;
			trace("channelsignal_data11:"+channelsignal_data11);
			wifisignal_ch11._visible=true;
			wifisignal_ch11.test_result_txt.textColor = 0x00ff00;
			wifisignal_ch11.test_result_txt.text=channelsignal_data11;
			
		}
		else if(channelsignal_data>100)
		{
			channelsignal_data6=channelsignal_data-100;
			trace("channelsignal_data6:"+channelsignal_data6);
			wifisignal_ch6._visible=true;
			wifisignal_ch6.test_result_txt.textColor = 0x00ff00;
			wifisignal_ch6.test_result_txt.text=channelsignal_data6;
			
		}
		else if(channelsignal_data<101)
		{
			channelsignal_data1=channelsignal_data;
			trace("channelsignal_data1:"+channelsignal_data1);
			wifisignal_ch1._visible=true;
			wifisignal_ch1.test_result_txt.textColor = 0x00ff00;
			wifisignal_ch1.test_result_txt.text=channelsignal_data1;
			
			
			
		}
		old_wifi_channel_data=channelsignal_data;
	}
//}
}
	
}


function version_test(){
	trace("version_test_enable: "+version_test_enable);
	if(version_test_enable){
		trace("test_config_version: "+Number(test_config_version)+", local_version: "+Number(local_version));
		if(Number(test_config_version) == Number(local_version)){
			trace("version test success!!!");
			version_test_mc.test_result_txt.textColor = 0x00ff00;
			version_test_mc.real_version_txt.textColor = 0x00ff00;
			version_test_mc.test_version_txt.textColor = 0x00ff00;
			version_test_mc.test_result_txt.text = "SUCCESS"
			fact_version_result="Success Test Version[" +test_config_version+"] Real_version["+local_version+"]";
		}else {
			trace("version test fail!!!");
			version_test_mc.test_result_txt.textColor = 0xff0000;
			version_test_mc.real_version_txt.textColor = 0xff0000;
			version_test_mc.test_version_txt.textColor = 0xff0000;
			version_test_mc.test_result_txt.text = "FAIL"
			fact_version_result="Fail!!Test Version[" +test_config_version+"]Real_version["+local_version+"]";
			
		}
		trace("fact_version_result"+fact_version_result);
		version_test_mc.real_version_txt.text = local_version;
		version_test_mc.test_version_txt.text = test_config_version;
	}
}

function video_test(){
	trace("video_test_enable: "+video_test_enable);
	if(video_test_enable){
		File_path[0]=search_testfile();
		if(flag==1)
		{
			trace("Video Test Start"); 
			playvideo(0);
			show_media_test_wait();
		}
		else if(flag==2)
		{
			trace("Audio Test Start"); 
			playaudio(0);
			show_media_test_wait();

		}
	}
}

function language_change() 
{
	SystemInfo.setCurLanguage(_global.MainSWF.language);
	SystemInfo.storeConfig();
}

function config_laguage(){
	var ret,lan_sup;
	trace("language_config_enable: "+language_config_enable);
	if(language_config_enable){
		ret = setLanguageString(test_config_language);
		if(ret){
			language_change();
			languageset_mc._visible = true;
			languageset_mc.test_result_txt.textColor = 0x00ff00;
			languageset_mc.test_result_txt.text = "Success["+test_config_language+"]"
			fact_language_result="Success["+test_config_language+"]";
			trace("Language config success!!!["+test_config_language+"]");
			_global.language_test_support=true;
		}else{

			lan_sup=language_support(test_config_language);
			languageset_mc._visible = true;
			
			if(lan_sup)
			{
				languageset_mc.test_result_txt.textColor = 0x00ff00;
				languageset_mc.test_result_txt.text = "Success["+test_config_language+"]set Already"
				fact_language_result="Success["+test_config_language+"]set Already";
				trace("Language config fail set Already!!!["+test_config_language+"]");
				_global.language_test_support=true;
			}
			else	
			{
				languageset_mc.test_result_txt.textColor = 0xff0000;
				languageset_mc.test_result_txt.text = "Fail!!!["+test_config_language+"]Not supported"
				fact_language_result="Fail!!!["+test_config_language+"]Not supported";
				trace("Language config fail!!! Not supported["+test_config_language+"]");
				_global.language_test_support=false;

			}
		}
		_global.language_test_result=languageset_mc.test_result_txt.text;
	}
}
function language_support(lang:String):Boolean{
	var i:Number = 0;
	var language_array:Array = new Array("en","fr","de","es","pl","zh-CN","zh-TW","ja","ko","it","cs","da","ru","nl","fi","no","pt","hu","ro","sk","tr","sv","el","ar", "id", "he", "tha", "fa");
	for(i=0; i<language_array.length; i++){
		if(lang == language_array[i]){
			return true;
		}
	}
	return false;
}

var startCount:Number = 0;
function start_test(){
	trace("MainSWF.topBarShowCompleted="+MainSWF.topBarShowCompleted); 
	var test_enable:String;
	test_enable = SystemInfo.get_test_config(SystemInfo.GET_TEST_WIFI_ENABLE);
	if(test_enable.toLowerCase() == SystemInfo.RETURN_VALUE_FALSE){
		;
	}
	else if(!MainSWF.topBarShowCompleted && startCount < 5){
		startCount++;
		return;
	}
	if(test_timer > 0){
		clearInterval(test_timer);
		test_timer = -1;
	}
	get_test_config();
	video_test();
	test_after_video();
}

function test_after_video(){
	version_test();	
	config_laguage();
	trace("wifi_test_enable: "+wifi_test_enable);
	if(wifi_test_enable){
		if(start_wifi_timer > 0){
			clearInterval(start_wifi_timer);
			start_wifi_timer = -1;
		}
		start_wifi_timer = setInterval(wifi_start, 2000);
	}
}

 //trace(File_name);
// trace(File_path);

// ---------------------------------------  wifi test ------------------------------
  
 function add_ap_to_connect(ssid:String, password:String):Void
 {
	var setting_ap_security:Number=1;
	scan_count++;  
	var tmp_total = WifiEngine.GetScanResults();	  
	//var tmp_total:Number = get_ap_total();
	trace("tmp_total: "+tmp_total+", ssid: "+ssid+", password: "+password);
	if(tmp_total<=0 && scan_count <= 4){
		if(wifi_connect_timer > 0){
		 clearInterval(wifi_connect_timer);
		 wifi_connect_timer = -1;
		}
		wifi_connect_timer = setInterval(connect_ap, 2000);
		trace("--------- wifi_connect_timer: "+wifi_connect_timer);		
	}else{
		var i = 0;
		var connect_index:Number = -1;
		for(i=0; i<tmp_total; i++){
			if(WifiEngine.GetSsid(i) == ssid){
				connect_index = i;
				break;
			}
		}
		total_ap = tmp_total;
		if(connect_index != -1){
			trace("connect index: "+connect_index);
			var ssid_AuthenType=WifiEngine.GetAuthenType(connect_index);
			if(ssid_AuthenType==WifiEngine.AUTHEN_OPEN
				|| ssid_AuthenType==WifiEngine.AUTHEN_WEP
				|| ssid_AuthenType==WifiEngine.AUTHEN_WPA
			  )
			{
				WifiEngine.SetAuthenType(connect_index, ssid_AuthenType);
			}
			
			WifiEngine.SetSsid(connect_index, ssid);
			WifiEngine.SetPassword(connect_index, password);
			cur_connect_index = connect_index;
			rtn = WifiEngine.SaveConf();
			if(rtn==0)
			{
				WifiEngine.ConnectAP();
				show_wifi_test_result(1);
			}
		}else{
			tmp_total -= 1;
			WifiEngine.AddNetwork();
			WifiEngine.SetSsid(tmp_total, ssid);

			WifiEngine.SetAuthenType(tmp_total, WifiEngine.AUTHEN_WPA);

			WifiEngine.SetPassword(tmp_total, password);
			cur_connect_index = tmp_total;
			total_ap++;
			if(WifiEngine.SaveaddedAPConf()==0)
			{
				WifiEngine.ConnectAP();
				MainSWF.wifi_test_start = true;
				MainSWF.wifi_test_count = 22;
				show_wifi_test_result(1);
			} else{
				trace("save config file failture!");
			}
		}
	}
}
 
function connect_ap(){
	var ssid = test_config_ssid;
	var passwd = test_config_psk;

	trace("-----connect_ap---- wifi_connect_timer: "+wifi_connect_timer);
	if(wifi_connect_timer > 0){
		clearInterval(wifi_connect_timer);
		wifi_connect_timer = -1;
	}
	trace("ssid: "+ssid+", passwd: "+passwd);
	if(ssid != "" && ssid != WifiEngine.getconnectedssid())
	{
		if(passwd.length >= 8 && passwd.length < 64)
		{		 
			WifiEngine.wifi_disconnect();
			add_ap_to_connect(ssid, passwd);
		}
	}
}

function wifi_connect_status_check(){
	if(wifi_check_count >= 0){
		var wifi_mode_get_from_case = SystemInfo.Getwifimode();
		if(wifi_mode_get_from_case==WifiEngine.WIFI_CONCURRENT_CLIENT_GETAUTOIP_OK || wifi_mode_get_from_case==WifiEngine.WIFI_CONCURRENT_CLIENT_GETAUTOIP_ERR){
			wifi_check_count = -1;
			trace("wifi test success!!!");
			show_wifi_test_result(0);
			if(wifi_check_index > 0){
				clearInterval(wifi_check_index);
				wifi_check_index = -1;
			}
		}else{
			if(wifi_check_count == 0){
				show_wifi_test_result(2);
				if(wifi_check_index > 0){
					clearInterval(wifi_check_index);
					wifi_check_index = -1;
				}
			}
			wifi_check_count --;
		}
	}
	
}

function wifi_start():Void
{
	var ret:Number = 0; 
 
	if(scan_clentap_flag == 0){
		return;
	}
	
	if(start_wifi_timer > 0){
		clearInterval(start_wifi_timer);
		start_wifi_timer = -1;
	}
	scan_count = 0;
	wifi_test_mc.test_result_txt.textColor = 0xffffff;
	wifi_test_mc.test_result_txt.text = "TESTING...";
	MainSWF.wifi_test_count = 22;
	if(wifi_connect_timer > 0){
		clearInterval(wifi_connect_timer);
		wifi_connect_timer = -1;
	}
	wifi_connect_timer = setInterval(connect_ap, 2000);
	trace("--------- wifi_connect_timer: "+wifi_connect_timer);
	if(wifi_check_index > 0){
		clearInterval(wifi_check_index);
		wifi_check_index = -1;
	}
	wifi_check_index = setInterval(wifi_connect_status_check, 1000);
}

function show_wifi_test_result(result:Number){
	trace("wifi test result: "+result);
	if(result == 0){
		wifi_test_mc.test_result_txt.textColor = 0x00ff00;
		wifi_test_mc.test_result_txt.text = "SUCCESS";
		trace("cur_connect_index: "+cur_connect_index);
		trace("ssid["+cur_connect_index+"]: "+WifiEngine.GetSsid(cur_connect_index));
		trace("ssid[0]: "+WifiEngine.GetSsid(0));
		var ssid = test_config_ssid;
		if(WifiEngine.GetSsid(cur_connect_index) != ssid ){
			var i = 0;
			for(i=0; i<total_ap; i++){
				if(WifiEngine.GetSsid(i) == ssid){
					cur_connect_index = i;
					break;
				}
			}
		}
		trace("ssid["+cur_connect_index+"]: "+WifiEngine.GetSsid(cur_connect_index));
		if(WifiEngine.DeleteConf(cur_connect_index) == 0)
		{
			trace("-delete password success!");
		}
		else
		{
			trace("-delete password error!");
		} 
	}else if(result == 1){
		wifi_test_mc.test_result_txt.textColor = 0xffffff;
		wifi_test_mc.test_result_txt.text = "TESTING...";
	}else{
		wifi_test_mc.test_result_txt.textColor = 0xff0000;
		wifi_test_mc.test_result_txt.text = "FAIL"
	}
}

function show_wifi_test_signal(signal:Number){
	//trace("show wifi test signal: "+signal);
	if(signal >= test_config_wifi_standard){
		wifi_signal_mc.test_result_txt.textColor = 0x00ff00;
	}else{
		wifi_signal_mc.test_result_txt.textColor = 0xff0000;
	}
	wifi_signal_mc.test_result_txt.text = signal;
}

