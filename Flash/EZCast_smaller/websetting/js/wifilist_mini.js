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
var scan_wifi_cnt=0;
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
var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime(); //To solve the IE cache ,by cxf

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
						$("#"+ssid_signal_img_id[i]).attr("src","svg/wifi1.png");
						break;
					case "2":
						$("#"+ssid_signal_img_id[i]).attr("src","svg/wifi2.png");
						break;
					case "3":
						$("#"+ssid_signal_img_id[i]).attr("src","svg/wifi3.png");
						break;
					case "4":
						$("#"+ssid_signal_img_id[i]).attr("src","svg/wifi4.png");
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
				}	
				else
				{
					var index=auto_connect_result.substring(0,(auto_connect_result.length-1));	
					index=Number(index);
					//$('ul li').eq(Number(index)).children().css({"color":"#ffffff","background":"#0099FF"});  //show select ssid
					$('ul li').eq(Number(index)).find(".ui-li-icon").after("<img src='svg/Check.png' class='ui-li-icon_select'>");

				}			
			}	
			else
			{	
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
			ssid_id[i]=Get_string_from_ssidstring(ssid_string);
			ssid_string=ssid_string_global;	
		}

		for(var i=0;i<wifi_ap_num;i++){
			var wifilist = "<li><a href='#list' onClick='readmsg("+i+","+href_string[i]+")'><div class='ui-li-text'>"+ssid_id[i]+"</div><img src='svg/wifi1.png'  id="+ssid_signal_img_id[i]+" class='ui-li-icon'></a></li>";
			 $("ul").append(wifilist);
		}
		create_ssid_signal();
		$("#add_net_button").css('display','block');  //for wphone display bug by cxf

		if(check_auto_connect_state_id>0)			
		{
			clearInterval(check_auto_connect_state_id);
			check_auto_connect_state_id=-1;
		}
		check_auto_connect_state_id = setInterval(check_auto_connect_state, 2000);
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
			$('#connect_fail_pop').hide();

		}		
	}
}
function show_connect_fail_dialog() 
{
	$('#warn_txt').text("The connection fails, the possible reasons are as follows:");
	$('#warn1_txt').text("1.Password error.");
	$('#warn2_txt').text("2.Failed to obtain an IP address.");
	$('#warn3_txt').text("3.Connection timeout.");

	$('#wait_pop').hide();
	$('#warn_pop').hide();
	$('#connect_fail_pop').show();

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
			if(scan_result_num>1 || scan_wifi_cnt>3)   //fixed 20170927
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
				scan_wifi_cnt++;
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

function Get_language()
{
	ok_string="OK";	
	cancle_string="CANCEL";
	forget_string="FORGET";
	warn1_string="Access Point will be disconnected if you press OK.";
	wait_string="Please wait";
	FORGET_PSW_string="Remove the password successfully.";

	$('#list_warn_txt').text(warn1_string);
	$('#ok_warn_txt').text(wait_string);
	$('#addwarn_txt').text(warn1_string);

}

function refreshlist(){
	// stopwait();
	$('#list').hide(); 
 //   window.location.reload();
}
function check_connect_status()
{
	var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
	$.get(cgiurl, {type:'check_wifi'}, function(wifi_mode) 
	{	
		var wifi_mode_ret=wifi_mode.indexOf("9");//WIFI_CONCURRENT_CLEINT_GETAUTOIP_OK
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
	$('#warn_pop').show();

}
function delaydisp_wait_popup()
{
	$('#wait_pop').show();

}
function show_warn_dialog() 
{
	if(show_waiting_tick_id>0)			
	{
		clearInterval(show_waiting_tick_id);
		show_waiting_tick_id=-1;
	}
	if(check_auto_connect_state_id>0)			
	{
		clearInterval(check_auto_connect_state_id);
		check_auto_connect_state_id=-1;
	}	
	setTimeout(delaydisp_popup,50);
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
					$('#hotspot').hide(); 
					show_warn_dialog();
					//delay_back_to_wifi_list();						
				}
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
						$('#hotspot').hide(); 
						show_warn_dialog();
						//delay_back_to_wifi_list();						
					}
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
			$('#list').hide(); 
			$.post('cgi-bin/set_wifi_POST.cgi', $('#save_ssid').serialize(), function(aaa){	
			}, "text");
			 show_warn_dialog();
		}
}
function set_security_radio()
{
 var that=$("input[type=radio][name=security][value="+security_index+"]");
	that.attr("checked","true");  
}

function  set_network_listener()
{
	$('#addnetwork').click(function(){
		$('#hotspot').show();
	});

	$('#connect_fail').click(function(){
		$('#connect_fail_pop').hide();
	});


	$('#saveok').click(function(){
		ok_ent();
	});
	
	$('#add_net_saveok').click(function(){
		$("input[type=radio][name=security][value="+0+"]").prev().removeClass("ui-focus"); 
		add_enter();
	});
	
	$('#forgetpsk').click(function() 
	{
	//alert("---forgetpsk-----");
	 	if($('#psk').val()=="********")
		{
			$('#list').hide();
			$.post('cgi-bin/delete_password_POST.cgi', $('#save_ssid').serialize());
			show_waiting_dialog();			
		}
	});
	
	$('#close').click(function(e) 
	{
		$('#list').hide();
	});
	
	$('#close2').click(function(e) 
	{
		$('#hotspot').hide();
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
				break;
			case 1:
				$('#AuthenType').val("2");//OPEN
				$('#showpassword').hide();
				break;
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
			$('#ssid_scan').val(ssid.substr(0,ssid_len-2));
			$('#ssid_txt').text($('#ssid_scan').val());
			$('#ssid').val(ssid.substr(0,ssid_len-2));
			$('#psk').val("********");
		}
		else
		{
			$('#psk').val("********");
			ssid_len=ssid.length;
			$('#ssid_scan').val(ssid.substr(0,ssid_len-1));
			$('#ssid_txt').text($('#ssid_scan').val());
			$('#ssid').val(ssid.substr(0,ssid_len-1));
		}			
	}	
	else
	{
		$('#psk').val("");
		$('#ssid_scan').val(ssid);
		$('#ssid_txt').text($('#ssid_scan').val());
		$('#ssid').val(ssid);
	}
  
}


function get_ap_name()
{	
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
	$('#list').show();
	
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

function stopwait(){
	$("#waiting").css('display','none');
}
function startwait(){
	$("#waiting").css('display','block');
}

 function delay_get_data_function()
 {
 	if(delay_get_data_id>0)			
	{
		clearInterval(delay_get_data_id);
		delay_get_data_id=-1;
	}
	Get_language();
	deal_href_string();  //delete wifi last scan wifi time....
	wifi_start();
	scan_wifi_cnt=0;  //clean wifi scan cnt by cxf
	wifi_start_scan();
	startwait();
	set_network_listener();

 }
function delay_get_data()
{
	if(delay_get_data_id>0)			
	{
		clearInterval(delay_get_data_id);
		delay_get_data_id=-1;
	}
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
