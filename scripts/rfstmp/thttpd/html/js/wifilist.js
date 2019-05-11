var input_id=new Array(50);
var ssid_id=new Array(50);
var href_string=new Array(50);
var connect_success_img_id=new Array(50);
var ssid_signal_img_id=new Array(50);
var ssid_index_id=new Array(50);
var wifi_ap_num;
var ssid_string_length;
var ssid_string="";
var ssid_index_indeed;
var ssid_string_global="";
var ssid_index=0;
var start=0;
var stop=0;
var get_scanresults_id=0;
var check_auto_connect_state_id=-1;
var check_auto_connect_state_tick=0;
var text_indeed;
var text_start;
var text_stop;
var text_string;
var text_string_global="";
var back_string="";
var text_string_length;
var wifi_connect_fail_text;
var ssid_indeed;
var SSID_Signal_String;
var internet_icon_txt_tick=10;
var Set_internet_icon_txt_tick_id=-1;
var show_connections_num_id=-1;
var show_connect_fail_dialog_id=-1;
var connect_fail_tick=5;
var create_wifi_list_indeed_flag=0;
var get_connected_ssid_id=-1;
var connected_ssid_flag=0;
var ssid_index_string_id=-1;
var get_ssid_index_string_id=-1;
var wifi_start_scan_id=-1;
var Check_new_version_connect_wifi_first_id=-1;
var scan_wifi_num_flag=0;
var delay_get_data_id=-1;
var scan_wifi_tick=2000;
var screen_width,screen_height;
var appskValue;
var ssidValue;
var security_index=0;
var EAP_index=1;
var FORGET_PSW_string;
var show_waiting_tick_id=-1;
var show_waiting_tick=15;
var connect_timeout_tick=0;
var connect_success_tick=0;
var ok_string,cancle_string,forget_string,wait_string,warn1_string;
var connected_ssid_string;
var showinfo_flag=0;
var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime(); //To solve the IE cache ,by cxf
var advance_flag=0;

function Get_text_from_textstring(text_string)
{
	var text_string_tmp=text_string.substring(text_start,text_string_length);
	//findnumb();	
	text_stop=text_string_tmp.indexOf("\n");
	text_start=0;
	text_indeed=text_string_tmp.substring(text_start,text_stop);
	text_start=text_stop+1;
	text_string_global=text_string_tmp;
	return text_indeed;
}

function Set_internet_icon_txt_tick_function()
{
	if(internet_icon_txt_tick==0)
	{
		clearInterval(Set_internet_icon_txt_tick_id);
		Set_internet_icon_txt_tick_id=-1;	
		internet_icon_txt_tick=10;
		$('#internet_icon_txt').hide();
	}
	internet_icon_txt_tick=internet_icon_txt_tick-1;
}
function Set_internet_icon_txt_tick()
{
	if(connected_ssid_flag==0)
	{
		connected_ssid_flag=1;
		$('#internet_icon_txt').show();						
		$('#internet_icon').show();
		if(Set_internet_icon_txt_tick_id>0)			
		{
			clearInterval(Set_internet_icon_txt_tick_id);
			Set_internet_icon_txt_tick_id=-1;
		}
		Set_internet_icon_txt_tick_id=self.setInterval(Set_internet_icon_txt_tick_function,1000);			
	}
}
function init_function()
{
	$('#internet_icon').hide();
	document.getElementById("upgrade_status").src="img/dongle.png";
}
function create_input_id_array()
{
	for(var i=0;i<wifi_ap_num;i++)
		input_id[i]=i;
}
 function create_connect_success_img_id_array()
{
	for(var i=0;i<wifi_ap_num;i++)
		connect_success_img_id[i]="connect_success_img"+i;
}
 function create_ssid_signal_img_id_array()
{
	for(var i=0;i<wifi_ap_num;i++)
		ssid_signal_img_id[i]="ssid_signal_img"+i;
}	 
function Get_string_from_ssidstring(ssid_string)
{
	var string_tmp=ssid_string.substring(start,ssid_string_length);	
	findnumb();	
	stop=string_tmp.indexOf("\n");
	start=0;
	ssid_indeed=string_tmp.substring(start,stop);
	start=stop+1;
	ssid_string_global=string_tmp;
	return ssid_indeed;
}
function create_ssid_signal()
{
	var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime(); //To solve the IE cache ,by cxf
	$.get(cgiurl, {type:'get_ssid_signal'}, function(ssid_signal_string)
	{	
	//alert("4: "+ssid_signal_string);  //信号强度
		if(ssid_signal_string.length>0)
		{
			start=0;
			ssid_string_length=ssid_signal_string.length;
			ssid_string_global="";
			SSID_Signal_String=ssid_signal_string;
			for(var i=0;i<wifi_ap_num;i++)
			{	
				var SSID_Signal=Get_string_from_ssidstring(SSID_Signal_String);
				SSID_Signal_String=ssid_string_global;	
				switch(SSID_Signal)
				{
					case "1":
						//document.getElementById(""+ssid_signal_img_id[i]).src="img/internet_icon_1.png";
						$("#"+ssid_signal_img_id[i]).attr("src","img/internet_icon_1.png");
						break;
					case "2":
						//document.getElementById(""+ssid_signal_img_id[i]).src="img/internet_icon_2.png";
						$("#"+ssid_signal_img_id[i]).attr("src","img/internet_icon_2.png");
						break;
					case "3":
						//document.getElementById(""+ssid_signal_img_id[i]).src="img/internet_icon_3.png";
						$("#"+ssid_signal_img_id[i]).attr("src","img/internet_icon_3.png");
						break;
					case "4":
						//document.getElementById(""+ssid_signal_img_id[i]).src="img/internet_icon_4.png";
						$("#"+ssid_signal_img_id[i]).attr("src","img/internet_icon_4.png");
						break;
				}			
			}
		}
	}, "text");	
}

function get_scanresults()
{
	var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime(); //To solve the IE cache ,by cxf
	$.get(cgiurl, {type:'get_scan_result'}, function(wifi_ap_info)
	{		
			//alert("3: "+wifi_ap_info); //ssid name
			if(wifi_ap_info.length>0)
			{
				if(get_scanresults_id>0)			
				{
					clearInterval(get_scanresults_id);
					get_scanresults_id=-1;
				}			
				//hide_waiting_dialog();	
				stopwait();
				ssid_string=wifi_ap_info;				
				ssid_string_length=wifi_ap_info.length;	
				ssid_string=ssid_string.substring(0,ssid_string_length);
				
				create_input_id_array();
				create_connect_success_img_id_array();	
				create_ssid_signal_img_id_array();
				
				create_wifi_list_indeed();
			}			
	}, "text");			
}
function get_ssid_index_string()
{
	var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime(); //To solve the IE cache ,by cxf
	$.get(cgiurl, {type:'get_apindex_string'}, function(SSID_index_string)
	{		
		var i=0;
		var ssid_index_temp;
		//alert("2: "+SSID_index_string);  //信号强度排序
		if(SSID_index_string.length)
		{
			if(get_ssid_index_string_id>0)			
			{
				clearInterval(get_ssid_index_string_id);
				get_ssid_index_string_id=-1;
			}
			text_start=0;
			text_string=SSID_index_string;
			text_string_length=SSID_index_string.length;
			text_string_global="";
			for(i=0;i<wifi_ap_num;i++)  //拆分ssid string...
			{
				ssid_index_temp=Get_text_from_textstring(text_string);
				
				href_string[i]=ssid_index_temp;  //排序索引号
			/*
				if (screen_width >= 800) 
					{
					href_string[i]="set_wifi_Horizontal.html?"+i+"index_indeed"+ssid_index_temp;
					}
				else  
					{
					href_string[i]="set_wifi_Vertical.html?"+i+"index_indeed"+ssid_index_temp;
					}
			*/
				text_string=text_string_global;									
			}
			if(get_scanresults_id>0)			
			{
				clearInterval(get_scanresults_id);
				get_scanresults_id=-1;
			}
			get_scanresults_id = setInterval(get_scanresults, 200);
		}
	}, "text");		
}
function Set_connect_success_background(index)
{
	var i=0;
	document.getElementById(""+input_id[index]).style.backgroundImage='url(img/bar_big.png)';//connect
	document.getElementById(""+input_id[index]).style.color="black";
	for(i=0;i<wifi_ap_num;i++)
	{
		if(i!=index)
		{
			document.getElementById(""+input_id[i]).style.backgroundImage='url(img/bar_big_cover.png)';//no connect
			document.getElementById(""+input_id[i]).style.color="white";			
		}
	}
}
function Check_new_version_connect_wifi_first_function()
{
	var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime(); //To solve the IE cache ,by cxf
	$.get(cgiurl, {type:'Check_new_version_o'}, function(new_ver_flag)
	{			
		var ret=Number(new_ver_flag);
		//if((1==ret)||(2==ret))
		if(2==ret)
		{			
			//location.href="websetting_Vertical.html?back_connect_ap";
			document.getElementById("upgrade_status").src="img/dongle_upgrade.png";
		}	
		else if(3==ret)
		{
			document.getElementById("upgrade_status").src="img/dongle_upgrade.png";
		}
		if(Check_new_version_connect_wifi_first_id>0)			
		{
			clearInterval(Check_new_version_connect_wifi_first_id);
			Check_new_version_connect_wifi_first_id=-1;
		}		
	}, "text");			
}
function Check_new_version_connect_wifi_first()
{
	if(Check_new_version_connect_wifi_first_id>0)			
	{
		clearInterval(Check_new_version_connect_wifi_first_id);
		Check_new_version_connect_wifi_first_id=-1;
	}
	Check_new_version_connect_wifi_first_id=self.setInterval(Check_new_version_connect_wifi_first_function,3000);	
}
function check_auto_connect_state()
{
	check_auto_connect_state_tick=check_auto_connect_state_tick+1;
	$.get(cgiurl, {type:'check_connect_status'}, function(auto_connect_result)
	{		
		if(auto_connect_result.length>0)
		{
		
if(!Array.indexOf){
    Array.prototype.indexOf = function(obj){
        for(var i=0; i<this.length; i++){
            if(this[i]==obj){
                return i;
            }
        }
        return -1;
    }
}
	//alert("5: "+auto_connect_result);

			var ret=auto_connect_result.indexOf("ER");
			if(ret<0)
			{
				if(check_auto_connect_state_id>0)			
				{
					clearInterval(check_auto_connect_state_id);
					check_auto_connect_state_id=-1;
				}		
				check_auto_connect_state_tick=0;
				findnumb();				
				ret=auto_connect_result.indexOf("\n");
				if(ret>=0)
				{
					var index=auto_connect_result.substring(0,(auto_connect_result.length-1));	
					index=Number(index);
					//document.getElementById(""+connect_success_img_id[index]).src="img/connected_noip.png";
					//Set_connect_success_background(index);
					//alert("6: not connect_result");
				}	
				else
				{
					var index=auto_connect_result.substring(0,(auto_connect_result.length-1));	
					index=Number(index);
					//alert("7: "+index);
					$('ul li').eq(Number(index)).children().css("background","#0099FF");  //show select ssid
					$('ul li').eq(Number(index)).children().removeClass("ui-icon-carat-r"); 
					$('ul li').eq(Number(index)).children().addClass("ui-icon-check");
					//document.getElementById(""+connect_success_img_id[auto_connect_result]).src="img/connected.png";
					//Set_connect_success_background(auto_connect_result);
					//Check_new_version_connect_wifi_first();
				}			
			}	
			else
			{	
			//alert("---show_connect_fail_dialog---line:338");
			//location.href="wifilist.html";	
			clearInterval(check_auto_connect_state_id);
			check_auto_connect_state_id=-1;
			show_connect_fail_dialog(); 
			}			
		}
	}, "text");			
	if(check_auto_connect_state_tick>22)     
	{
		check_auto_connect_state_tick=0;
		if(check_auto_connect_state_id>0)			
		{
			clearInterval(check_auto_connect_state_id);
			check_auto_connect_state_id=-1;
		}
	}
}
function Get_password()
{
	var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime(); //To solve the IE cache ,by cxf
	$.get(cgiurl, {type:'get_softap_psk'}, function(appsk) {
	  $('#ap_psk').text(appsk);
	}, "text");
}
function Get_connected_ssid_tick()
{
	var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime(); //To solve the IE cache ,by cxf
	$.get(cgiurl, {type:'get_connected_ssid'}, function(connected_ssid)
	{			
		if(connected_ssid.length)
		{	
			connected_ssid_string=connected_ssid;  // for add ssid user by cxf
			if(connected_ssid_flag==0)
				connected_ssid_flag=1;
			//	$('#internet_icon_txt').text(connected_ssid);											
			//Set_internet_icon_txt_tick();
			//document.getElementById("client_connect_status").src="img/internet_connected.png";
		}
		else
		{	
			connected_ssid_flag=0;
/*
			$('#internet_icon_txt').hide();						
			$('#internet_icon').hide();				
			document.getElementById("upgrade_status").src="img/dongle.png";
			document.getElementById("internet_icon2").style.backgroundImage='url(img/internet_icon2_cover.png)';
			document.getElementById("client_connect_status").src="img/dongle_not_connected.png";
			*/
		}
		//back_string="websetting_Vertical.html";	
	}, "text");		
}
function Get_connected_ssid()
{
	Get_connected_ssid_tick();
	if(get_connected_ssid_id>0)			
	{
		clearInterval(get_connected_ssid_id);
		get_connected_ssid_id=-1;
	}
	get_connected_ssid_id=self.setInterval(Get_connected_ssid_tick,3000);			
}
function show_connections_num_tick()
{
	var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime(); //To solve the IE cache ,by cxf
	$.get(cgiurl, {type:'get_connections'}, function(get_connections_num)
	{			
		if(get_connections_num.length)
		{
		findnumb();
			var ret=get_connections_num.indexOf("-1");
			if(ret<0)
			{
				$('#dongle_icon_txt').text("( "+get_connections_num+" )");
				document.getElementById("dongle_connect_status").src="img/dongle_connected.png";
			}
			else
			{
				$('#dongle_icon_txt').text("( "+0+" )");
				document.getElementById("dongle_connect_status").src="img/dongle_not_connected.png";
			}
		}
	}, "text");			
}

function show_connections_num()
{
	show_connections_num_tick();
	if(show_connections_num_id>0)			
	{
		clearInterval(show_connections_num_id);
		show_connections_num_id=-1;
	}
	show_connections_num_id=setInterval(show_connections_num_tick,3000);			
}
function create_wifi_list_indeed()
{
	//$('#text1').text(ssid_string);
	if(create_wifi_list_indeed_flag==0)
	{
		create_wifi_list_indeed_flag=1;
		start=0;
		ssid_string_length=ssid_string.length;
		ssid_string_global="";
		
		for(i=0;i<wifi_ap_num;i++)
		{				
		//	$('#myTable').append("<tr><td><div align='center'><a href='"+href_string[i]+"'><input style='cursor:pointer;border-radius:10px;width:200px;color:#FFFFFF;margin-left:auto;margin-right:auto;background:transparent;' type='text' disabled='disabled' id='"+input_id[i]+"' ></a></div></td><td><div align='left'><img src='img/internet_icon_1.png' id='"+ssid_signal_img_id[i]+"' width=50px height=50px disabled='disabled'/></div></td><td><div align='left'><img src='img/notconnected.png' id='"+connect_success_img_id[i]+"' width=50px height=50px disabled='disabled'/></div></td></tr>");											
			ssid_id[i]=Get_string_from_ssidstring(ssid_string);
			ssid_string=ssid_string_global;	
		}
		/*
		for(i=0;i<wifi_ap_num;i++)
		{				
			$('#text2').text( $('#text2').text()+"-/-"+href_string[i] );
			$('#text3').text( $('#text3').text()+"-/-"+ssid_id[i] );
		}
		*/
		for(var i=0;i<wifi_ap_num;i++){
var wifilist = "<li class='ui-li-has-icon'><a class='ui-btn ui-btn-icon-right ui-icon-carat-r' aria-expanded='false' aria-haspopup='true' aria-owns='list' href='#list' data-rel='popup' data-transition='none' data-position-to='window'  onClick='readmsg("+i+","+href_string[i]+")'>"+ssid_id[i]+"<img src='img/internet_icon_1.png'  id="+ssid_signal_img_id[i]+" class='ui-li-icon'></a></li>";
		 $("ul").append(wifilist);
		}
		//$('#1").addClass("ui-first-child");
		$('ul li:first').addClass("ui-first-child");
		$('ul li:last').addClass("ui-last-child");
	//$('#text1').text($('ul li:first').html());
		create_ssid_signal();
	//	show_connections_num();
	//	Get_connected_ssid();	
		Get_AP_ssid();	
		Get_password();
	$("#add_net_button").css('display','block');  //for wphone display bug by cxf

		if(check_auto_connect_state_id>0)			
		{
			clearInterval(check_auto_connect_state_id);
			check_auto_connect_state_id=-1;
		}
		check_auto_connect_state_id = setInterval(check_auto_connect_state, 2000);
	}
}
function Get_ap_num(wifi_ap_info)
{
findnumb();
	wifi_ap_num=wifi_ap_info.substring(wifi_ap_info.indexOf("app_total=")+10,wifi_ap_info.length);
	ssid_string_length=wifi_ap_info.length-(10+wifi_ap_num.length);
}
function clear_timer_id(timer_id)
{
	if(timer_id!=-1)			
	{
		clearInterval(timer_id);
		timer_id=-1;
	}		
}
function Get_wifi_connect_fail_text()
{
	var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime(); //To solve the IE cache ,by cxf
	$.get(cgiurl, {type:'get_connectfail_text'}, function(connect_fail_text)
	{		
		if(connect_fail_text.length>0)
		{

			wifi_connect_fail_text=connect_fail_text;
		}
	}, "text");		
}


function show_connect_fail_tick()
{
connect_fail_tick++;
	if(connect_fail_tick>5)
	{
	//alert("--connect_fail_tick==0--");
		//if(show_connect_fail_dialog_id>0)	
		{
			clearInterval(show_connect_fail_dialog_id);
			show_connect_fail_dialog_id=-1;
			connect_fail_tick=0;
			$('#connect_fail_pop').popup('close');
			//	location.href="wifilist.html";
			//	window.location.reload();
			/*
			document.getElementById("light").style.display="none";
			document.getElementById("fade").style.display="none";
			document.getElementById('warn').align="center";
			document.getElementById('warn1').align="center";
			document.getElementById('warn2').align="center";
			document.getElementById('warn3').align="center";
			*/
			/*
			if (screen_width >= 800) 
				{
				 location.href="wifi_list_Horizontal.html";
				}
			else  
				{
				 location.href="wifi_list_Vertical.html";
				}
				*/
		}		
	}
}
function show_connect_fail_dialog() 
{
/*
	document.getElementById("light").style.display="block";
	document.getElementById("light").style.position="fixed";
	document.getElementById("fade").style.display="block";
	document.getElementById("fade").style.position="fixed";
	//document.getElementById("fade").style.height=sHeight+"px";
	document.getElementById("warn_txt").style.width="100px";
	*/
	var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime(); //To solve the IE cache ,by cxf
	$.get(cgiurl, {type:'get_connectfail_text'}, function(connect_fail_text)
	{		
		if(connect_fail_text.length>0)
		{
			text_start=0;
			text_string=connect_fail_text;
			text_string_length=connect_fail_text.length;
			$('#warn_txt').text(Get_text_from_textstring(text_string));	
			//document.getElementById('warn').align="left";
			text_string=text_string_global;	
			$('#warn1_txt').text(Get_text_from_textstring(text_string));	
			//document.getElementById('warn1').align="left";
			text_string=text_string_global;	
			$('#warn2_txt').text(Get_text_from_textstring(text_string));	
			//document.getElementById('warn2').align="left";
			text_string=text_string_global;	
			$('#warn3_txt').text(Get_text_from_textstring(text_string));	
			//document.getElementById('warn3').align="left";			
		}
		else
		{
			$('#warn_txt').text("The connection fails, the possible reasons are as follows:");
			$('#warn1_txt').text("1.Password error.");
			$('#warn2_txt').text("2.Failed to obtain an IP address.");
			$('#warn3_txt').text("3.Connection timeout.");
		}

	$('#connect_fail_pop').popup('open');
	
	}, "text");		
	//document.getElementById("warn_txt").style.color="#FFFFFF";

	if(show_connect_fail_dialog_id>0)			
	{
		clearInterval(show_connect_fail_dialog_id);
		show_connect_fail_dialog_id=-1;
	}
	connect_fail_tick=0;
	show_connect_fail_dialog_id = setInterval(show_connect_fail_tick, 1000);
}


function wifi_start_scan_function_tick()
{
	if(wifi_start_scan_id>0)			
	{
		clearInterval(wifi_start_scan_id);
		wifi_start_scan_id=-1;
	}

var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime(); //To solve the IE cache ,by cxf
	$.get(cgiurl, {type:'wifi_scan'}, function(scan_result)
	{			
		findnumb();
		//alert("1: "+scan_result);  //can_result=22
		var ret=scan_result.indexOf("OK");
		if(ret>=0)
		{
			if(wifi_start_scan_id>0)			
			{
				clearInterval(wifi_start_scan_id);
				wifi_start_scan_id=-1;
			}				
			wifi_ap_num=1;
			if(0==scan_wifi_num_flag)
			{				
				scan_wifi_num_flag=1;
				if(get_ssid_index_string_id>0)			
				{
					clearInterval(get_ssid_index_string_id);
					get_ssid_index_string_id=-1;
				}
				get_ssid_index_string_id = setInterval(get_ssid_index_string, 200);					
			}
		}			
		else
		{
			var scan_result_num=Number(scan_result);
			if(scan_result_num>1)
			{		
				if(wifi_start_scan_id>0)			
				{
					clearInterval(wifi_start_scan_id);
					wifi_start_scan_id=-1;
				}			
				wifi_ap_num=scan_result;
				if(0==scan_wifi_num_flag)
				{				
					scan_wifi_num_flag=1;
					if(get_ssid_index_string_id>0)			
					{
						clearInterval(get_ssid_index_string_id);
						get_ssid_index_string_id=-1;
					}
					get_ssid_index_string_id = setInterval(get_ssid_index_string, 200);					
				}
			}else{
				scan_wifi_tick=2000;
				wifi_start_scan();
			}
		}
	}, "text");				
}
function wifi_start_scan()
{
	if(wifi_start_scan_id>0)			
	{
		clearInterval(wifi_start_scan_id);
		wifi_start_scan_id=-1;
	}
	wifi_start_scan_id = setInterval(wifi_start_scan_function_tick, scan_wifi_tick);	
}
function wifi_start()
{
	var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime(); //To solve the IE cache ,by cxf
	$.get(cgiurl, {type:'wifi_start'}, function(result)
	{			
		
	}, "text");		
}

function show_waiting_dialog_nouse() 
{
	document.getElementById("light").style.display="block";
	document.getElementById("light").style.position="fixed";
	document.getElementById("fade").style.display="block";
	document.getElementById("fade").style.position="fixed";
	document.getElementById("warn1_txt").style.width="100px";
	document.getElementById("warn1_txt").style.color="#FFFFFF";
}


function hide_waiting_dialog() 
{
	document.getElementById('light').style.display="none";
	document.getElementById('fade').style.display="none";
} 
function Back_home_listener()
{
	$('#Home').click(function(e) 
	{
		e.preventDefault();
		if(show_connections_num_id>0)			
		{
			clearInterval(show_connections_num_id);
			show_connections_num_id=-1;
		}
	if (bIsWM) 
	{
	location.href="websetting_Vertical_wp.html";
	}
	else{
		location.href="websetting_Vertical.html";
		}
	});
	
	$('#Home2').click(function(e) //for horixontal html
		{
			e.preventDefault();
			if(show_connections_num_id>0)			
			{
				clearInterval(show_connections_num_id);
				show_connections_num_id=-1;
			}
			location.href="websetting_Horizontal.html";
		});
}

function Get_AP_ssid()
{
	var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime(); //To solve the IE cache ,by cxf
	$.get(cgiurl, {type:'get_ap_ssid'}, function(ap_ssid)
	{			
		if(ap_ssid.length)
		{
			$('#ap_ssid').text("");
			$('#ap_ssid').text(ap_ssid);		
		}
	}, "text");		
}

function Get_AP_mac_ip()
{
	$.get("cgi-bin/wifi_info_GET.cgi", {type:'get_mac_ip'}, function(mac_wlan1_ip)
	{			
		if(mac_wlan1_ip.length)
		{
			text_start=0;
			text_string_length=0;
			text_string=mac_wlan1_ip;
			text_string_length=mac_wlan1_ip.length;
			$('#connect_mac_address').text(Get_text_from_textstring(text_string));
			text_string=text_string_global;	
			$('#ip1_txt').text(Get_text_from_textstring(text_string));	
			//$('#ip1_txt').show();
		}
	}, "text");		
}

function Get_local_version()
{
	$.get("cgi-bin/wifi_info_GET.cgi", {type:'get_OTA_version'}, function(ota_upgrade_string) 
	{	
		if(ota_upgrade_string.length)
		{
			text_start=0;
			text_string_length=0;
			text_string=ota_upgrade_string;
			text_string_length=ota_upgrade_string.length;
			$('#cur_ver').text(Get_text_from_textstring(text_string));
		}
	}, "text");	
}	
function Get_wlan0_ip_address()
{
	$.get("cgi-bin/wifi_info_GET.cgi", {type:'get_wlan0_ip'}, function(wlan0_ip)
	{			
		if(wlan0_ip.length)
		{
			$('#ip0_txt').text(wlan0_ip);	
		}
	}, "text");			
}
function Get_wifi_list_text()
{
	var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime(); //To solve the IE cache ,by cxf
	$.get(cgiurl, {type:'get_wifi_list_text'}, function(wifi_list_text)
	{			
		if(wifi_list_text.length)
		{
			text_string=wifi_list_text;
			text_string_length=wifi_list_text.length;
			$('#warn1_txt').text(Get_text_from_textstring(text_string));
			text_string=text_string_global;	
			$('#internet_access').text(Get_text_from_textstring(text_string));
		}
		else
		{
			$('#warn1_txt').text("Please wait");
			$('#internet_access').text("Connect for Internet Access");				
		}
	}, "text");	
}	

function Add_network_listener()
{
	$('#add_network').click(function(e) 
	{
		e.preventDefault();
		//location.href="add_network_Vertical.html";
			if (screen_width >= 800) 
				{
				 location.href="add_network_Horizontal.html";
				}
			else  
				{
				 location.href="add_network_Vertical.html";
				}
	});	
}

function Get_language()
{
	var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
	$.get(cgiurl, {type:'get_wifi_text'}, function(text)
	{	
//	alert(text);
		if(text.length)
		{
			text_start=0;
			text_string_length=0;
			text_string=text;
			text_string_length=text.length;
			ok_string=Get_text_from_textstring(text_string);
			text_string=text_string_global;	
			cancle_string=Get_text_from_textstring(text_string);
			text_string=text_string_global;	
			forget_string=Get_text_from_textstring(text_string);
			text_string=text_string_global;	
			//Get_text_from_textstring(text_string)
			warn1_string=Get_text_from_textstring(text_string);
			//$('#warn_txt').text(""+$('#warn1_txt').text());
			//$('#warn1_txt').text("Access Point will be disconnected if you tap ok button.");
			text_string=text_string_global;	
			wait_string=Get_text_from_textstring(text_string);
			text_string=text_string_global;	
			FORGET_PSW_string=Get_text_from_textstring(text_string);

			
			text_string=text_string_global;	
			$('#Security_text').text(Get_text_from_textstring(text_string));
			text_string=text_string_global;	
			$('#addnetwork').text(Get_text_from_textstring(text_string));//add by cxf

			//$('#warn_txt').text("Please wait");
			//text_string=text_string_global;	
			//var temp=Get_text_from_textstring(text_string);
			//temp=temp+" :";
			//$('#ap_ssid_txt').text(""+temp);
			//text_string=text_string_global;	
			//var temp=Get_text_from_textstring(text_string);
			//temp=temp+" :";
			//$('#ap_psk_txt').text(""+temp);
		}
		else
		{
			ok_string="OK";	
			cancle_string="CANCEL";
			forget_string="FORGET";
			warn1_string="Access Point will be disconnected if you press OK.";
			//$('#warn_txt').text("Access Point will be disconnected if you press OK.");
			//$('#warn1_txt').text("Access Point will be disconnected if you tap ok button.");
			wait_string="Please wait";
			$('#ap_ssid_txt').text("SSID :");
			$('#ap_psk_txt').text("Password :");	
			FORGET_PSW_string="Remove the password successfully.";
			$('#Security_text').text("Security");
			$('#addnetwork').text("Add network");
		}

	$('#saveok').text(ok_string);
	$('#close').text(cancle_string);
	$('#forgetpsk').text(forget_string);
	$('#add_net_saveok').text(ok_string);
	$('#close2').text(cancle_string);
	$('#list_warn_txt').text(warn1_string);
	$('#ok_warn_txt').text(wait_string);
	$('#addwarn_txt').text(warn1_string);
	$('#connect_fail').text(ok_string);
	
//	alert($('#cur_ver').text()+ok_string+cancle_string+forget_string+warn1_string+wait_string);
		
	}, "text");	



}
function Get_add_language()
{
	var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
	$.get(cgiurl, {type:'get_addnet_text'}, function(text2)
	{	
		if(text2.length)
		{
			text_string=text2;
			text_string_length=text2.length;
			$('#Security_text').text(Get_text_from_textstring(text_string));
			text_string=text_string_global;	
			$('#addnetwork').text(Get_text_from_textstring(text_string));//add by cxf
			/*
			text_string=text_string_global;	
			$('#OK').val(Get_text_from_textstring(text_string));
			text_string=text_string_global;	
			$('#CANCEL').val(Get_text_from_textstring(text_string));
			text_string=text_string_global;	
			$('#warn1_txt').text(Get_text_from_textstring(text_string));
			$('#warn_txt').text(""+$('#warn1_txt').text());
			text_string=text_string_global;	
			var temp=Get_text_from_textstring(text_string);
			temp=temp+" :";
			//$('#ap_ssid_txt').text(""+temp);
			text_string=text_string_global;	
			var temp=Get_text_from_textstring(text_string);
			temp=temp+" :";
			//$('#ap_psk_txt').text(""+temp);
			*/
		}
		else
		{
			$('#Security_text').text("Security");
			$('#addnetwork').text("Add network");
			/*
			$('#OK').val("OK");	
			$('#CANCEL').val("CANCEL");
			$('#warn1_txt').text("Access Point will be disconnected if you tap ok button.");
			$('#warn_txt').text("Please wait...");
			$('#ap_ssid_txt').text("SSID :");
			$('#ap_psk_txt').text("Password :");	
			*/
		}
	}, "text");	
}

function refreshlist(){
	// stopwait();
	$('#list').popup('close'); 
	//alert("---window.location.reload---");
 //   window.location.reload();
}
function check_connect_status()
{
	var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
	$.get(cgiurl, {type:'check_wifi'}, function(wifi_mode) 
	{	
		var wifi_mode_ret=wifi_mode.indexOf("9");//WIFI_CONCURRENT_CLEINT_GETAUTOIP_OK
		$("#text2").text("--||--");	
		if(wifi_mode_ret>=0)
		{
			connect_success_tick++;
			$("#text1").text(connect_success_tick);
			if(connect_success_tick>4)
			{
				connect_success_tick=0;
				if(show_waiting_tick_id>0)			
				{
					clearInterval(show_waiting_tick_id);
					show_waiting_tick_id=-1;				
				}
				if(advance_flag>0)
				    location.href="wifilist.html?advanced=1";	
				else
				    location.href="wifilist.html";	

			}
		}
		else
		{
			connect_timeout_tick++;
			$("#text3").text(connect_timeout_tick);

			if(connect_timeout_tick>10)
			{
				connect_timeout_tick=0;
				if(show_waiting_tick_id>0)			
				{
					clearInterval(show_waiting_tick_id);
					show_waiting_tick_id=-1;				
				}
				if(advance_flag>0)
				    location.href="wifilist.html?advanced=1";	
				else
				    location.href="wifilist.html";	
			}
		}
	}, "text");		
}	
function show_warn_tick_function()
{
	check_connect_status();
}
function delaydisp_popup()
{
	$('#warn_pop').popup('open');
	//startwait();

}
function delaydisp_wait_popup()
{
	$('#wait_pop').popup('open')

}
function show_warn_dialog() 
{
//	alert("---show_warn_dialog--");

	if(show_waiting_tick_id>0)			
	{
		clearInterval(show_waiting_tick_id);
		show_waiting_tick_id=-1;
	}
	setTimeout(delaydisp_popup,450);
	show_waiting_tick_id=self.setInterval(show_warn_tick_function,1000);	
} 

function show_waiting_tick_function()
{
	//$('#wait_pop').popup('open');
	if(show_waiting_tick==1){
	$('#wait_txt').text(FORGET_PSW_string);
	$("#text1").text(show_waiting_tick);

		}
	if(show_waiting_tick==0)
	{
		if(show_waiting_tick_id>0)			
		{
			clearInterval(show_waiting_tick_id);
			show_waiting_tick_id=-1;				
		}
		//href_string="wifi_list_Vertical.html?delete_wifi";
		//alert("---delete:  href_string--");
		if(advance_flag>0)
		    location.href="wifilist.html?delete_wifi?advanced=1";	
		else
		    location.href="wifilist.html?delete_wifi";
	}
	else
		show_waiting_tick=show_waiting_tick-1;
}

function show_waiting_dialog()
{
	//startwait();
	//$('#warn_txt').show();
	if(show_waiting_tick_id>0)			
	{
		clearInterval(show_waiting_tick_id);
		show_waiting_tick_id=-1;
	}
	show_waiting_tick=3;
	$('#wait_txt').text(wait_string);
	setTimeout(delaydisp_wait_popup,450);
	show_waiting_tick_id=self.setInterval(show_waiting_tick_function,2000);	
}
function enterkey(){
	if(event.keyCode==13){
	//alert(event.keyCode);
	ok_ent();
	}
}
function add_enter(){
		//e.preventDefault();
		//alert("-add_enter--");
		appskValue = $('#add_password').val();
		ssidValue= $('#add_ssid').val();
		$('#ssid').val(ssidValue);
		$('#psk').val(appskValue);
	//alert("security_index:+ssidValue+appskValue"+security_index+ssidValue+appskValue);
	//$('#sel_txt').text("security_index:+ssidValue+appskValue"+security_index+ssidValue+appskValue);	
		if(2==security_index)//EAP
		{
		var key_ps_Value=0;
			/*
			Identity_Value = $('#Identity').val();
			*/
			if(3==EAP_index) 
				key_ps_Value = $('#key_psk').val();
			else 
				key_ps_Value=0;
			
			//$('#AuthType').val("0");
			if(ssidValue.length<0)
			{
				alert("No acquaintances ssid.");					
			}
			else
			{
				if(connected_ssid_string!=ssidValue)
				{
    					var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
					var str = ssidValue + "\n" + appskValue + "\n" + $('#Identity').val() + "\n" + key_ps_Value + "\n" + EAP_index + "\n" + "add_802XEPA_network\n";
					//alert("security_index=3: \n"+ str);
					$.get(cgiurl, {type: str}, function() {
    						
    					}, "text");
					$('#hotspot').popup('close'); 
					show_warn_dialog();
					//delay_back_to_wifi_list();						
				}
				else
					if(advance_flag>0)
					    location.href="wifilist.html?advanced=1";	
					else
					    location.href="wifilist.html";	
			}		
		}
		else
		{
			if( appskValue.length < 8 && security_index==0)
			{
				alert("key must be 8-63 characters.");						
			}
			else 
			{
				if(ssidValue.length<0)
				{
					alert("No acquaintances ssid.");					
				}
				else
				{
					if(connected_ssid_string!=ssidValue)
					{
						$.post('cgi-bin/add_network_POST.cgi', $('#save_ssid').serialize());
						//delay_back_flag=1;
						$('#hotspot').popup('close'); 
						show_warn_dialog();
						//delay_back_to_wifi_list();						
					}
					else
						if(advance_flag>0)
						    location.href="wifilist.html?advanced=1";	
						else
						    location.href="wifilist.html";	
				}
			}		
		}		
}
function ok_ent(){
	connect_timeout_tick=0;
	connect_success_tick=0;
	appskValue = $('#psk').val();
		if( appskValue.length < 8)
		{
			//alert("key must be 8-12 characters.");
			$('#message').text("key must be 8-12 characters.");
			errorFlag = 1;			
		}
		else 
		{
			$('#message').text("");
			errorFlag = 0;
		}			
		if( !errorFlag ) 
		{
			$('#list').popup('close'); 
		//	alert("---saveok-----");

		//	startwait();
		//	$('#text1').text($('#save_ssid').serialize());	
			//'#text2').text($('#psk').val());	
			//'#text3').text($('#ssid_index').val());	
			//.post('cgi-bin/set_wifi_POST.cgi', $('#save_ssid').serialize());
				$.post('cgi-bin/set_wifi_POST.cgi', $('#save_ssid').serialize(), function(aaa){	
			//'#text1').text("//--"+aaa+"--//");	
		//	$('#text2').text("//--||-//");	
		//	$('#text3').text(aaa);	
				}, "text");
			 show_warn_dialog();
		}
}
function set_security_radio()
{
 var that=$("input[type=radio][name=security][value="+security_index+"]");
	that.attr("checked","true");  
	that.prev().removeClass("ui-radio-off"); 
	that.prev().addClass("ui-radio-on ui-btn-active ui-focus");
}
function set_eap_set_radio()
{
 var that=$("input[type=radio][name=EAP_SET][value="+EAP_index+"]");
	that.attr("checked","true");  
	that.prev().removeClass("ui-radio-off"); 
	that.prev().addClass("ui-radio-on ui-btn-active ui-focus");
}
function  set_network_listener()
{
/*
$('ul li').each(function(){
	$(this).click(function(){
		var that=$(this);
		alert(""+that);
	  }); 
	
  }); 


$('#psk').click(function(){
    $('#psk').get(0).type='text';
  }); 
  
$('#close').click(function(){
    $('#psk').get(0).type='password';
  }); 
*/


$('#connect_fail').click(function(){
$('#connect_fail_pop').popup('close');
});


$('#saveok').click(function(){
	ok_ent();
});
$('#add_net_saveok').click(function(event){
	event.preventDefault();
	//alert("---add_net_saveok-----");
	$("input[type=radio][name=security][value="+0+"]").prev().removeClass("ui-focus"); 
	add_enter();
});
$('#forgetpsk').click(function() 
{
//alert("---forgetpsk-----");
 	if($('#psk').val()=="********")
	{
		$('#list').popup('close'); 
		$.post('cgi-bin/delete_password_POST.cgi', $('#save_ssid').serialize());
		//alert("delete password success !");
		show_waiting_dialog();			
	}
});
$('#close').click(function(e) 
{
	$('#list').popup('close');
});
$('#close2').click(function(e) 
{
	//alert("---close2-----");
	$('#hotspot').popup('close');
});

set_security_radio();

$('input:radio[name="security"]').click(function(){
	var that=$(this).val();
	security_index=that;
	$('input:radio[name="security"]').each(function(){
		$(this).prev().removeClass("ui-focus");
	});	
	set_security_radio();
	//$('#sel_txt').text(security_index);	
	security_index=Number(security_index);
	switch(security_index)
	{
		case 0:
			$('#AuthenType').val("0");// WPA/WPA2
			$('#showpassword').show();
			$('#EAP_item').show();
			$('#EAP_SEL').hide();
			$('#Identity_set').hide();
			$('#key_psk_set').hide();
			break;
		case 1:
			$('#AuthenType').val("2");//OPEN
			$('#showpassword').hide();
			$('#EAP_item').show();
			$('#EAP_SEL').hide();
			$('#Identity_set').hide();
			$('#key_psk_set').hide();
			break;
		case 2:
			//$('#AuthenType').val("3");//EAP
			$('#showpassword').show();
			$('#EAP_item').show();
			$('#EAP_SEL').show();
			$('#Identity_set').show();
			$('#key_psk_set').hide();
			break;
	}
	EAP_index=1;
	set_eap_set_radio();
});

$('input:radio[name="EAP_SET"]').click(function(){
	var that=$(this).val();
		EAP_index=that;
	$('input:radio[name="EAP_SET"]').each(function(){
		$(this).prev().removeClass("ui-focus");
	});	
	set_eap_set_radio();
	EAP_index=Number(EAP_index);
	switch(EAP_index)
	{
		case 4: //PEAP(MSCHAPv2)
		case 2: //PEAP
			$('#key_psk_set').hide();			
			break;
		case 3: //TLS
			$('#key_psk_set').show();			
			break;
	}
});


$("#showinfo").click(function(){
	if(showinfo_flag){
	$("#more_info").hide();
	showinfo_flag=0;
	$("#showinfo").find("span").removeClass("ui-icon-carat-d");
	$("#showinfo").find("span").addClass("ui-icon-carat-u");
		}
	else{
		$("#more_info").show();
		showinfo_flag=1;
		$("#showinfo span").addClass("ui-icon-carat-d");
		$("#showinfo span").removeClass("ui-icon-carat-u");
		}
  });

}
		
function set_ssid_name(ssid)
{
	var ret=ssid.indexOf("\n");
	if(ret>0)
	{
		check_ssid_AuthenType=ssid.indexOf("O");
		if(check_ssid_AuthenType>0)
		{
			/*$('#psk').hide();	
			$('#psk_txt').hide();
			$('#psk').val("********");
			ssid_len=ssid.length;*/
			$('#ssid_scan').val(ssid.substr(0,ssid_len-2));
			$('#ssid_txt').text($('#ssid_scan').val());
			$('#ssid').val(ssid.substr(0,ssid_len-2));
			$('#psk').val("********");
			//document.getElementById('psk').disabled = true;
			//show_warn_dialog();
		}
		else
		{
			//check_confexist=ssid.indexOf("\n");
			//if(check_confexist>0)
			//{
				$('#psk').val("********");
				ssid_len=ssid.length;
				$('#ssid_scan').val(ssid.substr(0,ssid_len-1));
				$('#ssid_txt').text($('#ssid_scan').val());
				$('#ssid').val(ssid.substr(0,ssid_len-1));
				//document.getElementById('psk').disabled = true;
			//}										
		}			
	}	
	else
	{
//	alert("---set_ssid_name-----");
		//document.getElementById('psk').disabled = false;
		$('#psk').val("");
		$('#ssid_scan').val(ssid);
		$('#ssid_txt').text($('#ssid_scan').val());
		$('#ssid').val(ssid);
	}
  
}


function get_ap_name()
{	
	//var ap_index="ap"+ssid_index;
	var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
	$.get(cgiurl, {type:""+ssid_index+"get_ap_name"}, function(ssid) 
	{				
		set_ssid_name(ssid);
		$('#ssid_index').val(ssid_index_indeed);	
		$('#ssid_index_scan').text(ssid_index_indeed);	
	}, "text");		
}

function readmsg(val,index)   //read ssid,password
{
ssid_index=val;
ssid_index_indeed=index;
//$('#disptxt').text(ret);
//$('#dispwarn_txt').text(index);

$('#ssid_scan').val(" ");
$('#ssid_txt').text(" ");
get_ap_name();  //find password....
}
function deal_href_string()
{
	var temp = location.href;
	findnumb();
	var ret=temp.indexOf("delete_wifi");
	if(ret>=0)
	{
		scan_wifi_tick=3000;
	}
}
function Get_height_width_of_screen()
{
   
	if(!!(window.attachEvent && !window.opera))
	{
		screen_height = document.documentElement.clientHeight;
		screen_width = document.documentElement.clientWidth;
	}
	else
	{
		screen_height =window.innerHeight;
		screen_width = window.innerWidth;
	}
	
}
function stopwait(){
	$("#waiting").css('display','none');
	$("#waitbk").css('display','none');
}
function startwait(){
	$("#waiting").css('display','block');
	$("#waitbk").css('display','block');
}

function Get_router_ctl()
{
	var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
	$.get(cgiurl, {type:'get_router_ctl'}, function(router_ctl)
	{			
		var ret=Number(router_ctl);
		if(ret==1)//router
		{			
			if(1==connected_ssid_flag)
			{
				$('#ip0_txt').show();	
				$('#ip0').show();				
			}
		}		
		else//direct
		{
			$('#ip0_txt').hide();	
			$('#ip0').hide();	
		}
	}, "text");	
}

function Check_EZMUSIC_ENABLE_flag()
{
		$('#ip0show').hide();  //first hide ip0text

	$.get("cgi-bin/wifi_info_GET.cgi", {type:'EZMUSIC_ENABLE'}, function(ezmusic_enable) 
	{	
		ezmusic_flag = Number(ezmusic_enable);

		if(ezmusic_flag==4 ||ezmusic_flag==5 ||ezmusic_flag==6)                //EZCASTPRO_MODE   //EZCast Pro LAN  //pro box
		{
			$('#ip0show').show();
			var urlinfo=window.location.href; 
			var offset=urlinfo.indexOf("advanced=1");   
			advance_flag=Number(offset);
			console.log("advance_flag: "+advance_flag)
			if(advance_flag>0){
			    $('#setup_txt').attr('href', 'networksetup.html');
			    console.log("setup_txt111: "+$('#setup_txt').attr('href'))
			}else{
			    $('#setup_txt').attr('href', 'websetting.html?timeStamp='+ new Date().getTime());
			    console.log("setup_txt222: "+$('#setup_txt').attr('href'))
			}
			
		} else {
			$('#EAP').remove();
			$('#EAP').prev().remove();
			$('#OPEN').prev().addClass("ui-last-child");
			$('#setup_txt').attr('href', 'websetting.html');
			$('#ip0show').hide();
		}

	}, "text");			
}
 function delay_get_data_function()
 {
 	if(delay_get_data_id>0)			
	{
		clearInterval(delay_get_data_id);
		delay_get_data_id=-1;
	}
	//Get_height_width_of_screen();
	//init_function();
	Get_language();

	deal_href_string();  //delete wifi last scan wifi time....
	//Get_wifi_list_text();//wait string
	wifi_start();
	wifi_start_scan();
	startwait();
	//show_waiting_dialog();	
	//Back_home_listener();
	Add_network_listener();	
	set_network_listener();
	
//	Get_add_language();  //合并到上面的	Get_language();
	//	show_connections_num();
	//	Get_connected_ssid();	


 }
function delay_get_data()
{
	if(delay_get_data_id>0)			
	{
		clearInterval(delay_get_data_id);
		delay_get_data_id=-1;
	}
	Get_connected_ssid_tick();
	Check_EZMUSIC_ENABLE_flag();
	Get_wlan0_ip_address();
	Get_router_ctl();
	Get_local_version();
	Get_AP_mac_ip();

	delay_get_data_id=self.setInterval(delay_get_data_function,100);		
}
function findnumb(){
		if(!Array.indexOf){
		Array.prototype.indexOf = function(obj){
			for(var i=0; i<this.length; i++){
				if(this[i]==obj){
					return i;
				}
			}
			return -1;
		}
	}
}
