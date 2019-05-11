var delay_add_data_id=-1;
var airrviewflag,zoom=0;
var airview_playbuttom_flag=1;
var airview_refershTime=2000;
var client_ip=0, host_ip=0;
var host_flag=0,find_host_flag=0;
var splitmenu_array = new Array (0,0,0,0,0,0,0,0,0,0);
var host_authority_array = new Array();
var button=new Array("Conference_set","AirView_set","Setting_set","internet_access_set","password_hide_set","reboot_set","Device_set","Network_set");
var ota_server_ENABLE=0;
var business_customer_ENABLE=0;
var ezpro_pla=0;
var vale="";
var delay_host_check_id=-1;
var cnt=0,cnt2=0;
var text_indeed;
var start;
var stop;
var text_string;
var text_string_global="";
var text_string_length;
var main_text_string=-1,menu_text_string=-1;
var login_txt,logout_txt,admin_txt,main_text,airview_txt;
var confercon_txt,linkstatus_txt,airsetup_txt,addca_txt,hostauthority_txt,accesscontrol_txt,password_txt,otaserver_txt,rebootcontrol_txt,resetcontrol_txt,password_err_txt,device_management_txt,network_setup_txt;
var admin_setup_txt="Admin Setting",about_setup_txt="about";
var leftmenu=new Array("AirView","Link status","Conference Control","Device Management","Network Setup","Host Authority","Access Control","Reboot","Admin Setting","About");
var error_t,chcek_error_txt,chcek_error_txt2,ok_txt,cancle_txt,castcode_warn1,castcode_warn2;
var Refresh_txt,reboot_message,reset_message,yes_txt,no_txt;
var Globel_ssid="";

function setCookie(c_name,value,expiredays)
{
	var exdate=new Date()
	exdate.setDate(exdate.getDate()+expiredays)
	document.cookie=c_name+ "=" +escape(value)+ ((expiredays==null) ? "" : ";expires="+exdate.toGMTString())
}

function getCookie(c_name)
{
        //alert(c_name);
	// alert(document.cookie.length);
	if (document.cookie.length>0)
	{
		var c_start=document.cookie.indexOf(c_name + "=");
		 //  alert(c_start);
		if (c_start!=-1)
		{ 
			c_start=c_start + c_name.length+1;
			var c_end=document.cookie.indexOf(";",c_start);
			if (c_end==-1) c_end=document.cookie.length;
			return unescape(document.cookie.substring(c_start,c_end));
		} 
		 // alert(c_end);
	}
	return "";
}
   function CGiSetCookie(c_name,c_psk)
{
        var cgiurl0= "cgi-bin/user_password_post.cgi?timeStamp=" + new Date().getTime(); //To solve the IE cache ,by cxf
	 var str0="";	
	 var ret0="";
  	 var c_time=new Date().getTime();
		str0=c_name+"\n"+c_psk+"\n"+c_time+"\n";
		$.get(cgiurl0, {type:str0}, function(ret0)
		{	
		      //  alert(ret0);
		       
		}, "text");
}  
   function CGidelCookie(c_name,c_psk)
{
        var cgiurl0= "cgi-bin/user_password_post.cgi?timeStamp=" + new Date().getTime(); //To solve the IE cache ,by cxf
	 var str0="";	
	 var ret0="";
  	 var c_time="86400000";
		str0=c_name+"\n"+c_psk+"\n"+c_time+"\n";
		$.get(cgiurl0, {type:str0}, function(ret0)
		{	
		      //  alert(ret0);
		       
		}, "text");
}  


   
function CGiGetCurrentUsername()
{
        var cgiurl= "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime(); //To solve the IE cache ,by cxf
	 var ret="";
		$.get(cgiurl, {type:"get_user_name"}, function(ret)
		{	
		        $('#show_usrname').text(admin_txt);//admin	     
		        console.log("get_user_name:"+ret)
		         $('#usr_name').val("admin");//admin
 
		}, "text");
}  
function Creat_upload_ui() {
	$("#uploadbk").show();
	$("#txt_eapmschap4").hide();
	login_out_upload_ui();
}

function login_out_upload_ui() {
	$(".userform").hide();
	$("#loginin_txt").show();
}
function del_upload_ui() {
	$("#uploadbk").hide();
	$("#txt_eapmschap4").show();
	login_in_upload_ui();
}

function login_in_upload_ui() {
	$(".userform").show();
	$("#loginin_txt").hide();
}

function CGiGetCookieRusult_uploadui(c_name,c_psk)
{
        var cgiurl= "cgi-bin/user_get_cookie.cgi?timeStamp=" + new Date().getTime(); //To solve the IE cache ,by cxf
	 var ret="";
	    var str=c_name+"\n"+c_psk+"\n";
	       //  alert(str);
		$.get(cgiurl, {type:str}, function(ret)
		{	
		      //alert(ret);
			var retVal=Number(ret);
			if(retVal){
				CGiGetCurrentUsername();
				$('#modify-form').show();
				$("#log_txt").text("admin");//admin
				Creat_upload_ui();
				}
			else
			{
				$("#log_txt").text("Login");
				$('#show_usrname').text("");
				$('#modify-form').hide();
				del_upload_ui();
			}
		       
		}, "text");
}


function CGiGetCookieRusult(c_name,c_psk)
{
        var cgiurl= "cgi-bin/user_get_cookie.cgi?timeStamp=" + new Date().getTime(); //To solve the IE cache ,by cxf
	 var ret="";
	    var str=c_name+"\n"+c_psk+"\n";
	       //  alert(str);
		$.get(cgiurl, {type:str}, function(ret)
		{	
		      //alert(ret);
			var retVal=Number(ret);
			if(retVal){
				CGiGetCurrentUsername();
				$('#modify-form').show();
				creat_admin_menu();// creat admin left menu by cxf
				if(splitmenu_array[7]>0){
					$("#log_txt").text("admin");//admin
				}else{
					$('#log_txt').text("");
					}
				//$("#log_txt").text(admin_txt);//admin
				//$("[href='#add-form']").toggleClass("ui-icon-gear ui-icon-user");

				}
			else
			{
				//$("#log_txt").text(login_txt);
				if(splitmenu_array[7]>0){
					$("#log_txt").text("Login");
				}else{
					$('#log_txt').text("");
					}
				//$("[href='#add-form']").toggleClass("ui-icon-gear ui-icon-user");
				$('#show_usrname').text("");
				$('#modify-form').hide();
				//check_type();
				if(host_flag) 	creat_host_menu();
				check_type();

				
			}
		       
		}, "text");
}
/*
function  read_host_ip(){
	//alert("---read_host_ip----");
	var CGI_URL='./cgi-bin/conference_control.cgi?timeStamp='+ new Date().getTime();
	$.ajax({
			type: 'GET',
			cache: false,
			url: CGI_URL + 'type=query_tcp_connections',
			dataType: 'xml',
			success: function (data) {
				var splitCount = $(data).find("ConnectInfo").find("SplitCount").text();
				var statusFrame = document.getElementById('Connection_Status');
				$(data).find("Connection").each(function() {
					var $Connection = $(this);
					var ipaddress = $Connection.find("IPAddress").text();
					var client_type = $Connection.find("Type").text();
					var client_role = $Connection.find("Role").text();
					
				//alert(ipaddress+"|"+client_type+"|"+client_role);
			

				if(client_role=="Host" || client_role=="host"){
					host_ip=ipaddress;
					find_host_flag=1;
					clearInterval(delay_host_check_id);
					delay_host_check_id=-1;
					check_client_ip();
					//delay_host_check_id=self.setInterval(check_client_ip,200);	
					//$('#host_txt').text("host_ip: "+host_ip+"find_host_flag: "+find_host_flag);
					}
				});
				//alert("---each end---");

			},
			error: function (e) {
			}
		});
	cnt++;
	//$('#cnt1_txt').text(cnt);
	if(cnt>2){
	clearInterval(delay_host_check_id);
	delay_host_check_id=-1;
	//$('#host_txt').text("host_ip: "+host_ip+"find_host_flag: "+find_host_flag);
	delay_add_data_function();
		}		
	}
*/

function  read_host_ip(){
	//alert("---read_host_ip----");
	var CGI_URL='./cgi-bin/conference_control.cgi?timeStamp='+ new Date().getTime();
	$.get(CGI_URL, {type:'find_host'}, function(ret)
	{
		if (ret !== null ) {
			host_ip=ret;
			find_host_flag=1;
			clearInterval(delay_host_check_id);
			delay_host_check_id=-1;
			console.log("read hot ip: "+host_ip+"find_host_flag: "+find_host_flag);
			check_client_ip();
		}
	}, "text");
	
	cnt++;
	if(cnt>2){
		clearInterval(delay_host_check_id);
		delay_host_check_id=-1;
		delay_add_data_function();
	}
	console.log("total cnt:"+cnt);
}

function check_client_ip(){
	//alert("check_host_ip----------");
	$.get("cgi-bin/get_my_ip.cgi?timeStamp="+ new Date().getTime(), {type:'ip_adr'}, function(ret) 
		{	
	//	alert("check_client_ip: "+ret.length);
	//	if(ret.length != 0){
		if (ret !== null || ret !== undefined || ret !== '') { 
				client_ip=ret;
			//alert("check_client_ip: "+client_ip);	

				//read_host_ip();
				//host_ip =ConfCrtl_onoff();
			//alert("check_host_ip: "+host_ip);
			if(host_ip.indexOf(client_ip) >= 0 ) 
			{
				host_flag=1;
				//$('#client_txt').text("client_ip: "+client_ip+"host_flag: "+host_flag);
			}
			console.log("client_ip:"+client_ip+"host_flag: "+host_flag);
			}
		 //$('#client_txt').text("client_ip2: "+client_ip+"host_flag: "+host_flag);
		 delay_add_data_function();
	}, "text");
cnt2++;
//$('#cnt2_txt').text(cnt+"host find end--for test soft by cxf--");

	
}

function jump_psk(){
	window.location.replace("Password.html");
}

function check_pask_modfiy(){

var urlinfo=window.location.href; 
var len=urlinfo.length; 
var offset=urlinfo.indexOf("/");   
var newsidinfo=urlinfo.substr(offset+10,len);

offset=newsidinfo.indexOf("#");
if(offset>0){
newsidinfo=newsidinfo.substr(0,offset-1);
}
//alert("1---:"+newsidinfo);

        var cgiurl= "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime(); //To solve the IE cache ,by cxf
	 var ret="";
		$.get(cgiurl, {type:"compare_psk_modfiy"}, function(psk_mod)
		{	
		   var ret=Number(psk_mod); 
		  // alert("---111--check_pask_modfiy--"+ret);
		   if(ret){
			//$('#admin_psk').text("admin/000000");
				if((newsidinfo.indexOf("Password")>=0) ||(newsidinfo.indexOf("password")>=0))
					{
					//$('#first_modfiy').text("Please modify the initial password.");
					$('#first_modfiy').css('display','block');
					}
				else
					{
					//alert("---222--check_pask_modfiy--"+ret);
					setTimeout(jump_psk,500);
				}

		   	}
		}, "text");
}

function check_type(){
	$.get("cgi-bin/wifi_info_GET.cgi", {type:'EZMUSIC_ENABLE'}, function(ezmusic_enable) 
	{	
	//alert("--index.html---"+ezmusic_enable);
	var value=Number(ezmusic_enable);
		switch (value){
			case 1:
			case 2:
				//window.location.replace(ezmucast_url)
			break;
			case 3:  //EZWILAN_ENABLE
				if(host_flag){
					$("#reset").remove();
					$("#modify-form").remove();
					check_manually_input_host();		
					}
				else{
					$("#reset").remove();
					//$("#list_load").remove();
					$("#modify-form").remove();
					//$("#scan_client").remove();
					check_manually_input();
					}
			break;
			case 4:  //EZCASTPRO_MODE
			case 5:  //EZCASTPRO_wilan
			case 6:  //EZCASTPRO_box
			case 9:  //EZCASTPRO_box lan only
				if(host_flag){
					$("#reset").remove();  //reset html
					$("#modify-form").remove();//password
					check_manually_input_host();
					}
				else{
					$("#reset").remove();  //reset html
					//$("#list_load").remove();//websetting
					$("#modify-form").remove();//password
					//$("#scan_client").remove();//conference
					check_manually_input();
					check_fromto_airsetup();
					}

			break;
		    default:
			break;
			}

	}, "text");		
}
function jump_main(){
	//$('#pass_modfiy').popup("close");
	window.location.replace("main.html");
}

function check_fromto_airsetup()
{
	$.get("cgi-bin/wifi_info_GET.cgi", {type:'EZMUSIC_ENABLE'}, function(ezmusic_enable) 
	{	
		var value=Number(ezmusic_enable);
		value=7;
		if(value <7 || value > 8){   //not wire
			var urlinfo=window.location.href; 
			var len=urlinfo.length; 
			var offset=urlinfo.indexOf("/");   
			var newsidinfo=urlinfo.substr(offset+10,len);

			if((newsidinfo.indexOf("websetting.html")>0) && (newsidinfo.indexOf("websetting.html?advanced=1")<0))
				{
					//alert("---not host ---");
					$('#list_load').remove();
				}
		}
	}, "text");
	
}

function check_manually_input()
{
var urlinfo=window.location.href; 
var len=urlinfo.length; 
var offset=urlinfo.indexOf("/");   
var newsidinfo=urlinfo.substr(offset+10,len);

//alert(newsidinfo);
//alert(newsidinfo.indexOf("main.html"));
//alert(newsidinfo.indexOf("reset.html"));
if((newsidinfo.indexOf("reset.html")>0) ||(newsidinfo.indexOf("reboot.html")>0)  ||(newsidinfo.indexOf("Password.html")>0) ||(newsidinfo.indexOf("websetting.html?advanced=1")>0)||(newsidinfo.indexOf("websetting_admin.html")>0)||(newsidinfo.indexOf("networksetup.html")>0)||(newsidinfo.indexOf("upload.html")>0)||(newsidinfo.indexOf("conference.html")>0)||(newsidinfo.indexOf("host_authority.html")>0)||(newsidinfo.indexOf("wifilist.html")>0)||(newsidinfo.indexOf("ota_server.html")>0)||(newsidinfo.indexOf("access_control.html")>0) )
	{
	//alert("Plese login !");
	setTimeout(jump_main,500);
	}

}
function check_manually_input_host()
{
var urlinfo=window.location.href; 
var len=urlinfo.length; 
var offset=urlinfo.indexOf("/");   
var newsidinfo=urlinfo.substr(offset+10,len);

//alert(newsidinfo);
//alert(newsidinfo.indexOf("main.html"));
//alert(newsidinfo.indexOf("reset.html"));
if((newsidinfo.indexOf("reset.html")>0) ||(newsidinfo.indexOf("Password.html")>0) ||(newsidinfo.indexOf("host_authority.html")>0)||(newsidinfo.indexOf("ota_server.html")>0) ||(newsidinfo.indexOf("upload.html")>0) ||(newsidinfo.indexOf("websetting_admin.html")>0))
	{
	//alert("Plese login !");
	setTimeout(jump_main,500);
	}

}
function psk_check(value)
{
	var regx = /^[A-Za-z0-9]+$/; 
	var checkstring=value;	
	if(!regx.test(checkstring))
	{
		$('#error_txt').text(chcek_error_txt);//"Only the letters and numbers are allowed!"
		return 1;	
	}
	else if(checkstring.length < 6 ||checkstring.length >64)
	{
		$('#error_txt').text(chcek_error_txt2);//"Password must be between 6 to 64 characters long."
		return 1;	
	}
	return 0;
}

function Add_listener()
{
       $('#save_usrname_ok').click(function(e) 
	{
		var name0="";
		var psk0="";
		var psk1="";
		var ret0="0";
		var cgiurl0= "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime(); //To solve the IE cache ,by cxf
		var str0="";
		name0="admin";
		
		psk0=$('#password').val();
		psk1=$('#password1').val();
		//alert(psk0+"---"+psk1);
		if(psk0==psk1)
			{
				if(psk_check(psk0)){
					return 0;
					}
				else{
				//alert("--=====-");
				psk0=$('#password').val();
				//alert("save_usrname_ok");		psk0=$('#password').val();
				str0=name0+"\n"+psk0+"\n"+"modify_usr_password\n";
				$.get(cgiurl0, {type:str0}, function(ret0)
					{	
					      //  alert(ret0);
					         var val0=Number(ret0);
						   if(val0==1)
						   {
			                             // setCookie("act_username3","0",2);
							// alert("Modify password successfully,need to login again");
							//$("#modify-form").hide();
							$('#pass_modfiy').popup("open");
							//check_manually_input();
							  // init();
						   }
						   else
						   	alert(error_t);
					}, "text");
				   }
			}
		else{
			$('#pass_warn').popup("open");
			}
		
	});	


	$('#reload_main').click(function(e) 
	{
		$('#pass_modfiy').popup("close");
		check_manually_input();
	});	

	
	$('#cancle_usrname').click(function(e) 
	{
		
	    
	});	

      $('#login_usrname_ok').click(function(e) 
	{
	   
		var name="";
		var psk="";
	       var ret="0";
		var cgiurl= "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime(); //To solve the IE cache ,by cx
		//alert("login_usrname_ok");
		name=Globel_ssid+"_admin"; //$('#usr_name').val();
		console.log("-11--login_usrname_ok: name= "+name);
		psk=$('#usr_password').val();
		 var str=name+"\n"+psk+"\n"+"login_user_password\n";
		 $.get(cgiurl, {type:str}, function(ret)
		{		
                     // alert(ret);
                     console.log("cgi--->callbak : "+ret);
			  var val=Number(ret);
			   if(val==1)
			   {
    
				$('#show_usrname').text(admin_txt);//admin
				if(splitmenu_array[7]>0){
					$('#log_txt').text("admin");//admin	
				}else{
					$('#log_txt').text("");//admin	
					}
		
				//$('#log_txt').text(admin_txt);//admin	
				//$("[href='#add-form']").toggleClass("ui-icon-gear ui-icon-user");
				$('#modify-form').show();
				CGiSetCookie("act_username3",name);
				CGiSetCookie("act_password3",psk);
				creat_admin_menu();// creat admin menu by cxf
				$('#ap_psk').text(totalData.password);

			   }
			   else
			   	//alert("Password is not correct!");
			   	alert(password_err_txt);
			 
		}, "text");
		
	});	

	$('#cancle_usrname').click(function(e) 
	{
		
	    
	});	

	$('#login_out').click(function(e) 
	{
		var name="";
		var psk="";
		name=Globel_ssid+"_admin"; //name=$('#usr_name').val()
		psk=$('#usr_password').val();
		//alert("name+psk:"+name+psk);
		CGidelCookie("act_username3",name);
		CGidelCookie("act_password3",psk);
		$('#usr_password').val("");
		creat_menu();
		if(splitmenu_array[7]>0){
			$('#log_txt').text("Login");//admin	
			}else{
			$('#log_txt').text("");//admin	
			}
		//$('#log_txt').text(login_txt);//admin	
		//$("[href='#add-form']").toggleClass("ui-icon-gear ui-icon-user");
		//alert("--222---host_flag"+host_flag);
		if(host_flag){
			setTimeout(creat_host_menu,300);
			check_manually_input_host();
			if(psk_visibility_idx){
		   	    $('#ap_psk').text("********");
			    $('#ap_psk').css("vertical-align","middle");
			}
		} else {
			check_manually_input();
		}
	});	

    $("#Refresh_Rate").click(function() {
		var that = $("#rate_val").val();
		var str=that+"\nset\nairview_rate\nCONFNAME_WR\n";
		var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
		$.get(cgiurl, {type:str}, function(ret)
		{	
		}, "text");
        	airview_refershTime = Number(that);
        	$("#aivew_rate").popup("close");
    });
	
	$('#cbv_refersh').click(function(e) 
	{
		//alert("-refersh-");
		 reload_img();
		
	});	
	$('#pause_bot').click(function(e) 
	{
		//alert("-refersh-");
		if(airview_playbuttom_flag){
			$("#pause_bot").addClass("play_botton");
			$("#pause_bot").removeClass("pause_botton");
			airview_playbuttom_flag=0;
		}else {
			$("#pause_bot").removeClass("play_botton");
			$("#pause_bot").addClass("pause_botton");
			airview_playbuttom_flag=1;
			reload_img();
		}
		
	});	
	
	$("#target_img").click(function(e) {
		//alert("-----target_img-----");
		zoom++;
		if(zoom>4){
			$("#target_img").animate({
				left:'0px',
				width:'-=400px'
			    });
			zoom=0;
		}
		else{
			$("#target_img").animate({
			     // height:'+=150px',
			      width:'+=100px'
			    });
		}
		setTimeout(disp_offset,500);
		
	});	
	
	$("#target_img").bind("swipeleft",function(){

		$("#target_img").animate({
		      left:'-=150px',
		    });
		airrviewflag=1;
		setTimeout(disp_offset,500);

	});
	$("#target_img").bind("swiperight",function(){
			//alert("-----target_img-----");
		if($("#target_img").position().left<10){
			$("#target_img").animate({
			       left:'+=150px',
			    });
		}
		airrviewflag=2;
		setTimeout(disp_offset,500);
	 });
	
	$('#reboot').click(function(e)   //reset to default
	{
		$("#rebootpop").popup("open");

	});	
	
	$('#reboot_ok').click(function(e)   //reset to default
	{
		var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
		$.get(cgiurl, {type:"reboot"}, function()
		{	
		      
		}, "text");
		
		$("#rebootpop").popup("close");
		$("#reboot").text(Refresh_txt);
		$("#message_txt").text(reboot_message);

		$('#reboot').removeAttr("href");
		$('#reboot').attr("onClick","window.location.reload()");
		$('#reboot').removeAttr("data-position-to");
		$('#reboot').removeAttr("data-rel");

	});	
	
	
	
}
function login_listener(){
      $('#login_admin_ent').click(function(e) {
		var name="";
		var psk="";
	       var ret="0";
		var cgiurl= "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
		//alert("login_usrname_ok");
		name=Globel_ssid+"_admin"; //$('#usr_name').val();
		
		psk=$('#usr_password').val();
		 var str=name+"\n"+psk+"\n"+"login_user_password\n";
		 $.get(cgiurl, {type:str}, function(ret)
		{		
                     // alert(ret);
			  var val=Number(ret);
			   if(val==1)
			   {
    
				$('#show_usrname').text(admin_txt);//admin
				$('#log_txt').text("admin");//admin	
				$('#modify-form').show();
				CGiSetCookie("act_username3",name);
				CGiSetCookie("act_password3",psk);
				Creat_upload_ui();

			   }
			   else
			   	alert(password_err_txt);
			 
		}, "text");
		
	});

	$('#log_out').click(function(e) 
	{
		var name="";
		var psk="";
		name=Globel_ssid+"_admin"; //name=$('#usr_name').val()
		psk=$('#usr_password').val();
		//alert("name+psk:"+name+psk);
		CGidelCookie("act_username3",name);
		CGidelCookie("act_password3",psk);
		$('#usr_password').val("");
		$('#log_txt').text("Login");//admin	
		del_upload_ui();
	});	
	  
}

function disp_offset(){
	var xletf=Math.abs($("#target_img").position().left);
	var pic_width=$("#target_img").width();
	var win_width=$(window).width();
	var left_offset=pic_width-win_width;

	if(airrviewflag==1){
		if(xletf>(left_offset+20)){
			$("#target_img").animate({
			left:'-'+left_offset+'px'
			});
		}
	}
	if(airrviewflag==2){
		if(xletf>10){
			$("#target_img").animate({
			left:'0px'
			});
		}
	}

}

function creat_menu(){
	var menulist;
	//$("#add_menu li").remove();
	var list_num=$("#add_menu").parent().find("li").length;
	/*
	for(var i=0;i<splitmenu_array.length;i++)
		{
		vale=vale+splitmenu_array[i]+"-|-";
		}
	$("#22txt").text("splitmenu"+"##"+vale);
	*/
	if(list_num>=2){
		for(var i=1;i<list_num;i++){
			$("#add_menu").parent().find("li").eq(i).attr("class","del");
			}
		$(".del").remove();
			//$("[href='#add-form']").toggleClass("ui-icon-gear ui-icon-user");
	}
	// no select password html
	//if(splitmenu_array[7]>0){
	$("[href='#add-form']").addClass("ui-icon-gear");
	$("[href='#add-form']").removeClass("ui-icon-user");

	//}
	/*
	else{
		$("[href='#add-form']").remove();
		}
	*/
	//menulist = "<li data-icon='delete' class='ui-first-child' ><a href='#' data-rel='close' class='ui-btn ui-btn-icon-right ui-icon-delete'>Close menu</a></li>";
	//$("#add_menu").append(menulist);
	$("#add_menu").parent().find("li").eq(0).css("display","none");
	//main_text
	menulist = "<li data-icon='delete'><a href='main.html' target='_self' class='ui-btn'>"+$('#IDS_MENU_LAN6').text()+"</a></li>";
	$("#add_menu").before(menulist);
	/*
	if(splitmenu_array[0]>0) {
		menulist = "<li><a href='cbv.html' target='_self' class='ui-btn ui-btn-icon-right ui-icon-carat-r'>"+airview_txt+"</a></li>";
		$("#add_menu").before(menulist);
	}
	*/
	if(splitmenu_array[2]>0) {
		menulist = "<li class='ui-last-child'><a href='LinkStatus.html' target='_self' class='ui-btn ui-btn-icon-right ui-icon-carat-r'>"+linkstatus_txt+"</a></li>";
		$("#add_menu").before(menulist);
	}
	menulist = "<li class='ui-last-child'><a href='about.html' target='_self' class='ui-btn ui-btn-icon-right ui-icon-carat-r'>"+about_setup_txt+"</a></li>";
	$("#add_menu").before(menulist);

	login_in();
}

function creat_admin_menu(){
	var menulist;
	//alert("--creat_admin_menu-!");
	var list_num=$("#add_menu").parent().find("li").length;
	if(list_num>=2){
		for(var i=2;i<list_num;i++){
			$("#add_menu").parent().find("li").eq(i).attr("class","del");
		}
		$(".del").remove();
		// no select password html
		//if(splitmenu_array[7]>0){
		$("[href='#add-form']").addClass("ui-icon-user");
		$("[href='#add-form']").removeClass("ui-icon-gear");
	}
	$("#add_menu").parent().find("li").eq(0).css("display","none");

	{
		$("#add_menu").parent().find(".ui-last-child").removeClass("ui-last-child");
		/*
		if(splitmenu_array[0]>0) {
			menulist = "<li><a href='cbv.html' target='_self' class='ui-btn ui-btn-icon-right ui-icon-carat-r' id='menu_0'>"+airview_txt+"</a></li>";
			$("#add_menu").before(menulist);
		}
		*/
		if(splitmenu_array[2]>0) {
			menulist = "<li><a href='LinkStatus.html' target='_self' class='ui-btn ui-btn-icon-right ui-icon-carat-r' id='menu_1'>"+linkstatus_txt+"</a></li>";
			$("#add_menu").before(menulist);
		}
		if(splitmenu_array[1]>0) {
			menulist = "<li><a href='conference.html' target='_self' class='ui-btn ui-btn-icon-right ui-icon-carat-r' id='menu_2'>"+confercon_txt+"</a></li>";
			$("#add_menu").before(menulist);
			//$("[href='LinkStatus.html']").parent().remove();//del linkstatus.html
		}
		
		menulist = "<li><a href='websetting.html?advanced=1' target='_self' class='ui-btn ui-btn-icon-right ui-icon-carat-r' id='menu_3'>"+device_management_txt+"</a></li>";
		$("#add_menu").before(menulist);
		
		menulist = "<li><a href='networksetup.html' target='_self' class='ui-btn ui-btn-icon-right ui-icon-carat-r' id='menu_4'>"+network_setup_txt+"</a></li>";
		$("#add_menu").before(menulist);

		if(splitmenu_array[3]>0) {
			menulist = "<li><a href='websetting_admin.html' target='_self' class='ui-btn ui-btn-icon-right ui-icon-carat-r'>"+admin_setup_txt+"</a></li>";
			$("#add_menu").before(menulist);
		}
		/*
		if(splitmenu_array[5]>0) {
			menulist = "<li><a href='host_authority.html' target='_self' class='ui-btn ui-btn-icon-right ui-icon-carat-r' id='menu_5'>"+hostauthority_txt+"</a></li>";
			$("#add_menu").before(menulist);
		}
		
		$("#host_authoryty_txt").text(hostauthority_txt);
		if(splitmenu_array[6]>0) {
			menulist = "<li><a href='access_control.html' target='_self' class='ui-btn ui-btn-icon-right ui-icon-carat-r' id='menu_6'>"+accesscontrol_txt+"</a></li>";
			$("#add_menu").before(menulist);
		}
	
		{
			menulist = "<li><a href='internet_access_control.html' target='_self' class='ui-btn ui-btn-icon-right ui-icon-carat-r'>Internet Access Control</a></li>";
			$("#add_menu").before(menulist);
			menulist = "<li><a href='passcode.html' target='_self' class='ui-btn ui-btn-icon-right ui-icon-carat-r'>PassCode</a></li>";
			$("#add_menu").before(menulist);
			menulist = "<li><a href='password_hide.html' target='_self' class='ui-btn ui-btn-icon-right ui-icon-carat-r'>Hide Password</a></li>";
			$("#add_menu").before(menulist);
		}

		if(splitmenu_array[7]>0) {
			menulist = "<li><a href='Password.html' target='_self' class='ui-btn ui-btn-icon-right ui-icon-carat-r'>"+password_txt+"</a></li>";
			$("#add_menu").before(menulist);
		}

		if(ota_server_ENABLE) {
			menulist = "<li><a href='ota_server.html' target='_self' class='ui-btn ui-btn-icon-right ui-icon-carat-r'>"+otaserver_txt+"</a></li>";
			$("#add_menu").before(menulist);
		}
		*/
		ota_server_ENABLE=1;
		if(ota_server_ENABLE) {
			$("#otaserver").show();
		}
		if(splitmenu_array[8]>0) {
			menulist = "<li><a href='reboot.html' target='_self' class='ui-btn ui-btn-icon-right ui-icon-carat-r' id='menu_7'>"+rebootcontrol_txt+"</a></li>";
			$("#add_menu").before(menulist);
		}
	/*
		if(splitmenu_array[9]>0) {
			menulist = "<li class='ui-last-child'><a href='reset.html' target='_self' class='ui-btn ui-btn-icon-right ui-icon-carat-r'>"+resetcontrol_txt+"</a></li>";
			$("#add_menu").before(menulist);
		}
	*/
		menulist = "<li class='ui-last-child'><a href='about.html' target='_self' class='ui-btn ui-btn-icon-right ui-icon-carat-r'>"+about_setup_txt+"</a></li>";
		$("#add_menu").before(menulist);
		
		login_out();

		$("#add_menu").parent().find("li").each(function(){
			//alert($(this).text());
			if(typeof($(this).text()) == "undefined" || typeof($(this).text()) == "null" )
				creat_admin_menu();	//菜单为空时，重新创建菜单
		  });

		}
      
}

function creat_host_menu(){
	var menulist;
	var list_num=$("#add_menu").parent().find("li").length;
	//alert("--creat_host_menu-:"+list_num);
	//$("#22txt").text(host_flag+"##"+vale);
	//$("[href='about.html']").parent().remove();//first del about.html
	if(list_num>=4){
		for(var i=3;i<list_num;i++){
			$("#add_menu").parent().find("li").eq(i).attr("class","del");
		}
		$(".del").remove();
	}
	$("#add_menu").parent().find("li").eq(0).css("display","none");
	$("#add_menu").parent().find(".ui-last-child").removeClass("ui-last-child");
	//if(list_num>=2)
	{

	
	if(host_authority_array[0]==1 && splitmenu_array[1]>0){
		$("#add_menu").parent().find(".ui-last-child").removeClass("ui-last-child");
		menulist = "<li><a href='conference.html' target='_self' class='ui-btn ui-btn-icon-right ui-icon-carat-r'>"+confercon_txt+"</a></li>";
		$("#add_menu").before(menulist);
		//$("[href='LinkStatus.html']").parent().remove();//del linkstatus.html
		}
	/*
	if(host_authority_array[2]==1 && splitmenu_array[3]>0){
		$("#add_menu").parent().find(".ui-last-child").removeClass("ui-last-child");
		menulist = "<li><a href='websetting_admin.html' target='_self' class='ui-btn ui-btn-icon-right ui-icon-carat-r'>"+airsetup_txt+"</a></li>";
		$("#add_menu").before(menulist);
		}
	*/
	if(host_authority_array[6]==1 ){//host authoriyt contral
		$("#add_menu").parent().find(".ui-last-child").removeClass("ui-last-child");
		menulist = "<li><a href='websetting.html?advanced=1' target='_self' class='ui-btn ui-btn-icon-right ui-icon-carat-r'>"+device_management_txt+"</a></li>";
		$("#add_menu").before(menulist);
		}	
	if(host_authority_array[7]==1 ){//host authoriyt contral
		menulist = "<li><a href='networksetup.html' target='_self' class='ui-btn ui-btn-icon-right ui-icon-carat-r'>"+network_setup_txt+"</a></li>";
		$("#add_menu").before(menulist);
		}
	/*
	if(host_authority_array[3]==1 || host_authority_array[4]==1 && splitmenu_array[6]>0){
		$("#add_menu").parent().find(".ui-last-child").removeClass("ui-last-child");
		menulist = "<li><a href='access_control.html' target='_self' class='ui-btn ui-btn-icon-right ui-icon-carat-r'>"+accesscontrol_txt+"</a></li>";
		$("#add_menu").before(menulist);
		}
	*/
	if(host_authority_array[5]==1 && splitmenu_array[8]>0){
		$("#add_menu").parent().find(".ui-last-child").removeClass("ui-last-child");
		menulist = "<li  class='ui-last-child'><a href='reboot.html' target='_self' class='ui-btn ui-btn-icon-right ui-icon-carat-r'>"+rebootcontrol_txt+"</a></li>";
		$("#add_menu").before(menulist);
		}
	menulist = "<li class='ui-last-child'><a href='about.html' target='_self' class='ui-btn ui-btn-icon-right ui-icon-carat-r'>"+about_setup_txt+"</a></li>";
	$("#add_menu").before(menulist);

	}
	$("#add_menu").parent().find("li").each(function(){
			//alert($(this).text());
			if(typeof($(this).text()) == "undefined" || typeof($(this).text()) == "null" ){
				list_num=$("#add_menu").parent().find("li").length;
				for(var i=3;i<list_num;i++){
					$("#add_menu").parent().find("li").eq(i).attr("class","del");
				}
				$(".del").remove();
				creat_host_menu();	//菜单为空时，重新创建菜单
			}
	});
	
	if(host_authority_array[1]==1){
		//$("#air_onoff").show();
	}
	access_control_host();

}

function access_control_host(){
	//if(host_authority_array[4]==1)
		//$("#host_2").show();
	if(host_authority_array[3]==1)
		$("#host_3").show();
}

function login_in(){
	$(".userform").show();
	$("#loginin_txt").hide();
	$("#modify-form").hide();
	$('#add-form').show();
	//$("#log_txt").text(login_txt);
}

function login_out(){
	$(".userform").hide();
	$("#loginin_txt").show();
	$("#modify-form").show();
	$('#add-form').show();
	check_pask_modfiy();
	//$("#log_txt").text(admin_txt);//admin
	if(ezpro_pla){
		$("#host_0").show();
	}
	$("#host_1").show();
	//$("#host_2").show();
	$("#host_3").show();

}

function CGiGet_compare_psk_modfiy()
{
	var cgiurl= "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime(); //To solve the IE cache ,by cxf
	var ret="";
	$.get(cgiurl, {type:"compare_psk_modfiy"}, function(psk_mod)
	{	
	   var ret=Number(psk_mod); 
	   if(ret){
		$('#admin_psk_txt').text("Admin");
		$('#admin_psk').text("000000");
		$('#default_show').show();
	   	}
	   else{
		$('#admin_psk_txt').text("");
		$('#admin_psk').text("");
		$('#default_show').hide();
	   	}
	}, "text");
} 

function init()
{
	CGiGetCookieRusult("act_username3","act_password3");
}

function Get_AP_ssid_formain()
{
	var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
	$.get(cgiurl, {type:'get_ap_ssid'}, function(ap_ssid)
	{			
		if(ap_ssid.length)
		{
			$('#ssid_txt').text("SSID:"+ap_ssid);
			Globel_ssid=ap_ssid;
		}
	}, "text");		
}
function get_mac_address()
{
    	 var str="";	
	 var cgiurl= "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime(); //To solve the IE cache ,by cxf
	 var ret="";
	 var super_psk="";
		$.get(cgiurl, {type:"get_supper_psk"}, function(super_psk)
		{	
			// super_psk=$.md5(super_psk);
			/*if(super_psk!="")
			{
				 str=super_psk+"set_encrypted_psk";
				 $.get(cgiurl, {type:str}, function(mac_ip)
				{	
				}, "text");
			}*/
		
		}, "text");
}

  function get_config_val()
 {
 	 var str="";	
	 var cgiurl= "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime(); //To solve the IE cache ,by cxf

		$.get(cgiurl, {type:"get_config_val"}, function(ret)
		{
			//alert(ret);
			str=ret;
			var JSONObject = eval ("(" + str + ")");
		if(str!=null)
		{
			var t=0;
			splitmenu_array[t]=JSONObject.AirView_ONOFF;
			t++;
			splitmenu_array[t]=JSONObject.ConferenceCtl_ONOFF;
			t++;
			splitmenu_array[t]=JSONObject.LinkStaus_ONOFF;
			t++;
			splitmenu_array[t]=JSONObject.AirSetup_ONOFF;
			t++;
			splitmenu_array[t]=JSONObject.AddCA_ONOFF;
			t++;
			splitmenu_array[t]=JSONObject.HostAuthority_ONOFF;
			t++;
			splitmenu_array[t]=JSONObject.AccessCtl_ONOFF;
			t++;
			splitmenu_array[t]=JSONObject.PasswordModify_ONOFF;
			t++;
			splitmenu_array[t]=JSONObject.RebootCtl_ONOFF;
			t++;
			splitmenu_array[t]=JSONObject.ResetToDef_ONOFF;
			//alert("cnt:"+t+"ResetToDef_ONOFF_val:"+splitmenu_array[t]);
		}
	else{
		//splitmenu_array=[0,0,0,0,0,0,0,0,0,0]; 
		}

	            
		
		}, "text");
	
	
 }
 function delay_add_data_function()
 {
 	if(delay_add_data_id>0)			
	{
		clearInterval(delay_add_data_id);
		delay_add_data_id=-1;
	}
	//check_client_ip();
	if(!host_flag){
		setTimeout(check_fromto_airsetup,500);
	}
	CGiGet_compare_psk_modfiy();
	//creat_menu();
	setTimeout(creat_menu,500);
	setTimeout(init,600);
	 //init();
	get_mac_address();
	Get_AP_ssid_formain();
	Add_listener();
	CGiGetCurrentUsername();// add for check admin->ssid+admin
	
	
 }

function main()
{
	if(delay_add_data_id>0)			
	{
		clearInterval(delay_add_data_id);
		delay_add_data_id=-1;
	}
	//delay_add_data_id=self.setInterval(delay_add_data_function,1100);	
	if(delay_host_check_id>0)			
	{
		clearInterval(delay_host_check_id);
		delay_host_check_id=-1;
	}
	delay_host_check_id=self.setInterval(read_host_ip,300);	
	$('#add-form').hide();
	get_config_val();// must first run ,  by cxf
	Get_main_language();//main mult language
	read_host_ip();
	Get_host_authority();
	Check_ota_server_ENABLE();
	//Check_business_customer_ENABLE();
	check_pro_pla();

}

function uploadui()
{
	Get_main_language();//main mult language
	CGiGet_compare_psk_modfiy();
	CGiGetCookieRusult_uploadui("act_username3","act_password3");
	login_listener();
}

function check_pro_pla(){
	$.get("cgi-bin/wifi_info_GET.cgi", {type:'EZMUSIC_ENABLE'}, function(ezmusic_enable) 
	{	
	//alert("--index.html---"+ezmusic_enable);
	var value=Number(ezmusic_enable);
		if(value>3)
			ezpro_pla=1;
	}, "text");		
}

/* cbv ----  */
//var b=0;
function loadImage() {  
var hrefid0=document.getElementById("target_img");
    var img =new Image(); 
	window.clearTimeout(intervalID);
	img.src = '../../tmp/snapshot.jpg';   
    img.onload =function(){  
        img.onload =null;  
        //call(img);  
        $("#target_img").attr("src",img.src+ "?r"+Math.random());
		//b++;
		//$("#cnt1_txt").text($("#target_img").attr("src")+"|"+hrefid0.src);
		//$("#cnt2_txt").text("loadimg:"+b);
    }  

return false;
}
/*
function call(img){  
	//alert("-11--"+img.src+"||"+img.width+"||"+img.height);	
	//if((img.width/img.height)<1.78)
	{
	$("#target_img").attr("src",img.src);	
	//alert("---"+$("#target_img2").src);		
	}
}
*/
var intervalID;
var img_now=-1;
function reload_img()
{
	window.clearTimeout(intervalID);
	var hrefid0=document.getElementById("target_img");
	var ret="";
	var viewport;
	//alert("img reload...1");
	//$('head meta[name=viewport]').remove();
	$('head meta[name=viewport]').attr("content","width=device-width,user-scalable=yes");
	var cgiurl = "cgi-bin/htmlsetting_snapshot.cgi?timeStamp=" + new Date().getTime();
	$.get(cgiurl, {type:"get_airview"}, function(ret)
		{
			//alert("img reload..."+ret);
			ret=Number(ret);
			if(ret) {
				//alert("---111--");
				//hrefid0.src="../../tmp/snapshot.jpg";
				//loadImage('../../tmp/snapshot.jpg',call); 
				$('#refersh').show();
				$('#pause_bot').show();
				if(airview_refershTime>0)
					$('#rate_text').show();
				intervalID = window.setTimeout("loadImage();",500);
				//setTimeout(function(){loadImage('../../tmp/snapshot.jpg',call)}, 1000);
			}
			else  {
				//alert("---000--");
				$('#refersh').hide();
				$('#pause_bot').hide();
				$('#rate_text').hide();
				hrefid0.src="img/cbv_disable.jpg";
			}
			$('#refersh_botton').css('background-color','#9D9D9D');
			//alert("---reload_img--");
		},"text");
/*
	$.get("cgi-bin/htmlsetting_snapshot.cgi", function(outcome)
		{	
		  alert("outcome is "+outcome);
		  if(outcome.indexOf("0")){
			if(img_now!=0)hrefid0.src="../tmp/snapshot_0.jpg";
			img_now=0;
		  }else{
			if(img_now!=1)hrefid0.src="../tmp/snapshot_1.jpg";
			img_now=1;		  
		  }
		},"text");
*/	
//var	value=Number($('#air_sw').val());
if(Number(airview_playbuttom_flag)==1 && airview_refershTime>0)
	intervalID = window.setTimeout("reload_img();",airview_refershTime);
	
}


function display_img(){
	
	airview_refershTime=Number(host_authority_array[8]); //airview_rate
	if (airview_refershTime == 5000) {
	    $('#rate_text').text("5 seconds");
	} else if (airview_refershTime == 15000) {
	    $('#rate_text').text("15 seconds");
	} else if (airview_refershTime == 30000) {
	    $('#rate_text').text("30 seconds");
	} else if (airview_refershTime == 60000) {//300-256
	    $('#rate_text').text("1 minute");
	} else {
	    $('#rate_text').text("Manual update");
	    $('#pause_bot').hide();
	}
	var cgiurl= "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime(); //To solve the IE cache ,by cxf
	var ret="";
	var viewport;
	var hrefid=document.getElementById("target_img");
	$.get(cgiurl, {type:"get_airview"}, function(ret)
	{	
		$('#show_airview').text(ret);
		ret=Number(ret);
			 
		 if(ret){
			reload_img();
		  }
		  else{
			hrefid.src="img/cbv_disable.jpg";
			$('#refersh').hide();
			$('#pause_bot').hide();
			$('#rate_text').hide();
		  }	 
  
	}, "text");
}

function set_host_authority(mode){

	var cgiurl= "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime(); //To solve the IE cache ,by cxf
	var	value0=$('#Conference_val').val();
	var	value1=$('#airview_val').val();
	var	value2=$('#setting_val').val();
	var	value3=$('#internet_access_val').val();
	var	value4=$('#password_hide_val').val();
	var	value5=$('#reboot_val').val();
	var	value6=$('#deviceset_val').val();
	var	value7=$('#network_val').val();
	//alert("--2222---"+value0+value1+value2+mode);
	switch (mode)
		{
		case button[0]:
			var str=value0+"\nset\nconferent\nCONFNAME_WR\n";
			 $.get(cgiurl, {type:str}, function(ret)
				{	
				}, "text");
			break;
		case button[1]:
			var str=value1+"\nset\nairview\nCONFNAME_WR\n";
			 $.get(cgiurl, {type:str}, function(ret)
				{	
				}, "text");
			 if(Number(value1)==1)
			 	$('#aivew_rate').popup("open");
			break;
		case button[2]:
			str=value2+"\nset\nairoptions\nCONFNAME_WR\n";
			 $.get(cgiurl, {type:str}, function(ret)
				{	
				}, "text");
			break;
		case button[3]:
			str=value3+"\nset\ninternetenables\nCONFNAME_WR\n";
			 $.get(cgiurl, {type:str}, function(ret)
				{	
				}, "text");
			break;
		case button[4]:
			str=value4+"\nset\npasswordhide\nCONFNAME_WR\n";
			 $.get(cgiurl, {type:str}, function(ret)
				{	
				}, "text");
			break;
		case button[5]:
			str=value5+"\nset\nreloadsystem\nCONFNAME_WR\n";
			 $.get(cgiurl, {type:str}, function(ret)
				{	
				}, "text");
			break;
		case button[6]:
			str=value6+"\nset\ndevicemanager\nCONFNAME_WR\n";
			 $.get(cgiurl, {type:str}, function(ret)
				{	
				}, "text");
			break;
		case button[7]:
			str=value7+"\nset\nnetworksetup\nCONFNAME_WR\n";
			 $.get(cgiurl, {type:str}, function(ret)
				{	
				}, "text");
			break;
		}
}

function set_host_authority_listener(){
	var thatid;
	for(var i=0;i<button.length;i++){
		$('#'+button[i]).click(function(e) 
			{
			thatid=this.id;
			//alert("-----"+that);
			set_host_authority(thatid);
			});
		$('#'+button[i]).bind("swipeleft",function(){
			thatid=this.id;
			set_host_authority(thatid);
		    });
		$('#'+button[i]).bind("swiperight",function(){
			thatid=this.id;
			set_host_authority(thatid);
		    });
		}

}
function set_host_authority_value()
{
	$('#Conference_val').val(Number(host_authority_array[0]));
	Conference_style();
	
	$('#airview_val').val(Number(host_authority_array[1]));
	airview_style();
	
	$('#setting_val').val(Number(host_authority_array[2]));
	airset_style();
	
	$('#internet_access_val').val(Number(host_authority_array[3]));
	internet_style();
	
	$('#password_hide_val').val(Number(host_authority_array[4]));
	passwordhide_style();

	$('#reboot_val').val(Number(host_authority_array[5]));
	reboot_style();
	
	$('#deviceset_val').val(Number(host_authority_array[6]));
	devicemanager_style();
	
	$('#network_val').val(Number(host_authority_array[7]));
	networkset_style();

	airview_refershTime=Number(host_authority_array[8]); //airview_rate
	$('#rate_val').val(airview_refershTime);
    	$("#rate_val").find("option[value="+airview_refershTime+"]").attr("selected",true); 
	if (airview_refershTime == 5000) {
	    $("#rate_val-button").children("span").text("5 seconds");
	} else if (airview_refershTime == 15000) {
	    $("#rate_val-button").children("span").text("15 seconds");
	} else if (airview_refershTime == 30000) {
	    $("#rate_val-button").children("span").text("30 seconds");
	} else if (airview_refershTime == 60000) {//300-256
		$("#rate_val-button").children("span").text("1 minute");
	} else {
		$("#rate_val-button").children("span").text("Manual update");
	}
}
function Get_host_authority()
{
	var value;
	var cgiurl = "cgi-bin/wifi_info_GET.cgi";
	$.get(cgiurl, {type:'1\nhost_val\nconferent\nCONFNAME_WR\n'}, function(ret)
	//$.get(cgiurl, {type:'host_val'}, function(ret)
	{
		value=ret;
		host_authority_array=value.split(",");
		/*	
		if(value.indexOf("conferent")>0)
			{
			var JSONObject = eval ("(" + value + ")");
			
			var i=0;
			host_authority_array[i]=JSONObject.conferent;
			i++;
			host_authority_array[i]=JSONObject.airview;
			i++;
			host_authority_array[i]=JSONObject.airoptions;
			i++;
			host_authority_array[i]=JSONObject.internetenables;
			i++;
			host_authority_array[i]=JSONObject.passwordhide;
			i++;
			host_authority_array[i]=JSONObject.reloadsystem;
			i++;
			host_authority_array[i]=JSONObject.devicemanager;
			i++;
			host_authority_array[i]=JSONObject.networksetup;
			i++;
			host_authority_array[i]=JSONObject.airview_rate;
			}
		*/
	}, "text");	
	
}

function Conference_style(){
	 if($('#Conference_val').val()==1){
		$("#Conference_onoff").children().addClass("ui-flipswitch-active");
		$("#Conference_val option").eq(1).attr("selected",true);  //on
	  }
	  else{
		$("#Conference_onoff").children().removeClass("ui-flipswitch-active");
		$("#Conference_val option").eq(0).attr("selected",true);  //off
	  }
}
function airview_style(){
	 if($('#airview_val').val()==1){
		$("#airview_onoff").children().addClass("ui-flipswitch-active");
		$("#airview_val option").eq(1).attr("selected",true);  //on
	  }
	  else{
		$("#airview_onoff").children().removeClass("ui-flipswitch-active");
		$("#airview_val option").eq(0).attr("selected",true);  //off
	  }
}
function airset_style(){
	 if($('#setting_val').val()==1){
		$("#setting_onoff").children().addClass("ui-flipswitch-active");
		$("#setting_val option").eq(1).attr("selected",true);  //on
	  }
	  else{
		$("#setting_onoff").children().removeClass("ui-flipswitch-active");
		$("#setting_val option").eq(0).attr("selected",true);  //off
	  }
}
function devicemanager_style(){
	 if($('#deviceset_val').val()==1){
		$("#deviceset_onoff").children().addClass("ui-flipswitch-active");
		$("#deviceset_val option").eq(1).attr("selected",true);  //on
	  }
	  else{
		$("#deviceset_onoff").children().removeClass("ui-flipswitch-active");
		$("#deviceset_val option").eq(0).attr("selected",true);  //off
	  }
}
function networkset_style(){
	 if($('#network_val').val()==1){
		$("#networkset_onoff").children().addClass("ui-flipswitch-active");
		$("#network_val option").eq(1).attr("selected",true);  //on
	  }
	  else{
		$("#networkset_onoff").children().removeClass("ui-flipswitch-active");
		$("#network_val option").eq(0).attr("selected",true);  //off
	  }
}
function internet_style(){
	 if($('#internet_access_val').val()==1){
		$("#internet_access_onoff").children().addClass("ui-flipswitch-active");
		$("#internet_access_val option").eq(1).attr("selected",true);  //on
	  }
	  else{
		$("#internet_access_onoff").children().removeClass("ui-flipswitch-active");
		$("#internet_access_val option").eq(0).attr("selected",true);  //off
	  }
}
function passwordhide_style(){
	 if($('#password_hide_val').val()==1){
		$("#password_hide_onoff").children().addClass("ui-flipswitch-active");
		$("#password_hide_val option").eq(1).attr("selected",true);  //on
	  }
	  else{
		$("#password_hide_onoff").children().removeClass("ui-flipswitch-active");
		$("#password_hide_val option").eq(0).attr("selected",true);  //off
	  }
}
function reboot_style(){
	 if($('#reboot_val').val()==1){
		$("#reboot_onoff").children().addClass("ui-flipswitch-active");
		$("#reboot_val option").eq(1).attr("selected",true);  //on
	  }
	  else{
		$("#reboot_onoff").children().removeClass("ui-flipswitch-active");
		$("#reboot_val option").eq(0).attr("selected",true);  //off
	  }
}
function set_ota_server(mode){

	var cgiurl= "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime(); //To solve the IE cache ,by cxf
	var value=$("#ota_path").val();
	var str=value+"set_otaserver_path";
	//alert("set_ota_server"+str);
	 $.get(cgiurl, {type:str}, function(ret)
		{	
		}, "text");
}

function set_ota_server_listener(){

		$('#ota_path_ok').click(function(e) 
			{
			set_ota_server();
			});

}

function get_ota_server()
{
	var cgiurl = "cgi-bin/wifi_info_GET.cgi";
	$.get(cgiurl, {type:'get_otaserver_path'}, function(ret)
	{	
	//alert("get_ota_server:"+ret);
	if(ret=="nil")$('#ota_path').val("");
	else $('#ota_path').val(ret);
	
	}, "text");			
}

function Check_ota_server_ENABLE()
{
	$.get("cgi-bin/wifi_info_GET.cgi", {type:'OTA_SERVER_ENABLE'}, function(ret) 
	{	
		//alert("Check_ota_server_ENABLE:"+ret);
		ota_server_ENABLE=Number(ret);
	}, "text");			
}
var internet_access_idx=0,internet_access_select=0;
function internet_access_control_on_style(){
		$("#internet_access_control_val").val(1);
		$("#internet_access_control").children().addClass("ui-flipswitch-active");
		$("#internet_access_control_val option").eq(1).attr("selected",true);  //on
}
function internet_access_control_off_style(){
		$("#internet_access_control_val").val(0);
		$("#internet_access_control").children().removeClass("ui-flipswitch-active");
		$("#internet_access_control_val option").eq(0).attr("selected",true);  //off
}
function set_internet_access_control_text(){
	var cgiurl= "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime(); //To solve the IE cache ,by cxf
	//var	value=$('#internet_access_control_val').val();
	var value=$('input:radio[name="internet_access_choose"]:checked').val();
	var str=value+"\n"+"set_internet_access_control\n";
	 $.get(cgiurl, {type:str}, function(ret)
		{	
		//alert("Modify successfully");
		}, "text");
}
function set_internet_access_style()
{
	var that=$("input[type=radio][name=internet_access_choose][value="+internet_access_idx+"]");
	that.attr("checked","true");  
	that.prev().removeClass("ui-radio-off"); 
	that.prev().addClass("ui-radio-on");
 }
function clean_internet_access_style() {
    $('input:radio[name="internet_access_choose"]').each(function() {
        $(this).prev().addClass("ui-radio-off");
        $(this).prev().removeClass("ui-radio-on");
        $(this).attr("checked", " ");
    });
}
function get_internet_access_control_text()
{
	var value;
	//$("label[for=internet_access_con_0]").text("All allowed"); 
	//$("label[for=internet_access_con_1]").text("Casting users allowed"); 
	//$("label[for=internet_access_con_2]").text("All denied"); 
	var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
	$.get(cgiurl, {type:'get_internet_access_control'}, function(ret)
	{	
		value=Number(ret);
		internet_access_idx=value;
		set_internet_access_style();
	}, "text");
	
}

function set_internet_access_control_listener()
{

	$("#set_internet_access_ok").click(function(e) 
	{
		internet_access_idx=internet_access_select;
		set_internet_access_control_text();
		$("#internet_access_set").popup("close");
	//	alert($('#router_only_val').val());
	});
	$("#set_internet_access_cel").click(function(e) 
	{	
		if(internet_access_idx != internet_access_select){
			clean_internet_access_style();
			set_internet_access_style();
		}
		$("#internet_access_set").popup("close");
	});
	$("#Internet_Access").click(function(e) 
		{
		$("#internet_access_set").popup("open");
	    });
	$('input:radio[name="internet_access_choose"]').click(function(){
		var that=$(this).val();
		internet_access_select=that;
	});
	
}

/**Password_visibility code**/
var psk_visibility_idx=0;
function set_psk_visibility_control_listener()
{

	$("#wifipassword_hide_ok").click(function(e) 
		{
		set_psk_visibility_control_text();
	//	alert($('#router_only_val').val());
	    });
	$("#wifipassword_hide_cel").click(function(e) 
		{
		if(psk_visibility_idx)
			{
			psk_visibility_control_on_style();
			$('#wifi_pass_hide_txt').text("ON");
			}
		else{
			psk_visibility_control_off_style();
			$('#wifi_pass_hide_txt').text("OFF");
			}
	    });
	
}

function psk_visibility_control_on_style(){
		$("#psk_visibility_val").val(1);
		$("#psk_visibility_control").children().addClass("ui-flipswitch-active");
		$("#psk_visibility_val option").eq(1).attr("selected",true);  //on
}
function psk_visibility_control_off_style(){
		$("#psk_visibility_val").val(0);
		$("#psk_visibility_control").children().removeClass("ui-flipswitch-active");
		$("#psk_visibility_val option").eq(0).attr("selected",true);  //off
}

function set_psk_visibility_control_text(){
	var cgiurl= "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime(); //To solve the IE cache ,by cxf
	var	value=Number($('#psk_visibility_val').val());
	psk_visibility_idx = value;
	var str=value+"\nset\nPSK_hide\nCONFNAME_WR\n";
	if(value)
		{
		psk_visibility_control_on_style();
		$('#wifi_pass_hide_txt').text("ON");
		}
	else{
		psk_visibility_control_off_style();
		$('#wifi_pass_hide_txt').text("OFF");
		}
	 $.get(cgiurl, {type:str}, function(ret)
		{	
			
		}, "text");
	
}
function get_psk_visibility_control_text()
{
	var value;
	var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
	$.get(cgiurl, {type:'1\nget\nPSK_hide\nCONFNAME_WR\n'}, function(ret)
	{	
	value=Number(ret);
	psk_visibility_idx = value;
	$('#psk_visibility_val').val(value);
	if(value)
		{
		psk_visibility_control_on_style();
		$('#wifi_pass_hide_txt').text("ON");
		}
	else{
		psk_visibility_control_off_style();
		$('#wifi_pass_hide_txt').text("OFF");
		}
	}, "text");			
}

var castcode_idx=0,castcode_select=0;
function passcode_on_style(){
		$("#passcode_val").val(1);
		$("#passcode_control").children().addClass("ui-flipswitch-active");
		$("#passcode_val option").eq(1).attr("selected",true);  //on
}
function passcode_off_style(){
		$("#passcode_val").val(0);
		$("#passcode_control").children().removeClass("ui-flipswitch-active");
		$("#passcode_val option").eq(0).attr("selected",true);  //off
}
function set_passcoderadio()
{
	var that=$("input[type=radio][name=castcode_choose][value="+castcode_idx+"]");
	that.attr("checked","true");  
	that.prev().removeClass("ui-radio-off"); 
	that.prev().addClass("ui-radio-on");
 }
function clean_passcoderadio_style() {
    $('input:radio[name="castcode_choose"]').each(function() {
        $(this).prev().addClass("ui-radio-off");
        $(this).prev().removeClass("ui-radio-on");
        $(this).attr("checked", " ");
    });
}
function set_passcode_text(){
	var cgiurl= "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime(); //To solve the IE cache ,by cxf
	var	value;//$('#passcode_val').val();
	if(castcode_idx==0)
		value="0"
	else if(castcode_idx==1)
		value="1"
	else if(castcode_idx==2){
		value=$("#user_input").val();
		if (!(/(^[0-9]\d*$)/.test(value)))
			{
			//alert(value);
			 $("#castcode_warn_text").text("Fixed value must be number!");
			  return 0;
			}
		else if(value.length<4){
			$("#castcode_warn_text").text("4-digit number for fixed Castcode");
			  return 0;
			}
		value="2"+"\n"+$("#user_input").val();

		}
	$("#castcode_set").popup("close");
	var str=value+"\n"+"set_passcode\n";
	 $.get(cgiurl, {type:str}, function(ret)
		{	
			
		}, "text");
	
}
function get_passcode_text()
{
	var value;
	var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
	$.get(cgiurl, {type:'get_passcode'}, function(ret)
	{	
		castcode_idx=Number(ret);
		set_passcoderadio();

	
	}, "text");
	$.get(cgiurl, {type:'get_pass_code_val'}, function(ret)
	{	
		var CodeNum=0;
		var castCode="";
		CodeNum=Number(ret);
		if(CodeNum<10)
			castCode="000"+ret;
		else if(CodeNum<100)
			castCode="00="+ret;
		else if(CodeNum<1000)
			castCode="0"+ret;
		else if(CodeNum<10000)
			castCode=ret;
		$("#user_input").val(castCode);

	
	}, "text");
}

function set_passcode_listener()
{
	$("#passcode_set").unbind().bind("swipeleft click swiperight",function(e)
	{	
		//alert($('#router_only_val').val());
	    });

	$("#set_castcode_ok").click(function(e) 
	{
        //Mos: Simplfied condition, if user click OK, store current setting whatever
		//if(castcode_idx != castcode_select){
			castcode_idx=castcode_select;
			set_passcode_text();
		//} else {
			$('#castcode_set').popup("close");
		//}
	//	alert($('#router_only_val').val());
	});
	$("#set_castcode_cel").click(function(e) 
	{
		if(castcode_idx != castcode_select){
			clean_passcoderadio_style();
			set_passcoderadio();
		}
		$('#castcode_set').popup("close");
	});
	
	$("#txt_castcode").click(function(e) 
	{
		$("#castcode_warn_text").text(castcode_warn2);
		$("#castcode_set").popup("open");
	    });

	$("#castcode_con_2").click(function(e) 
	{
		$("#user_input").focus();
	    });
	
	$('input:radio[name="castcode_choose"]').click(function(){
		var that=$(this).val();
		var b=$(this).prev().text();
		castcode_select=Number(that);
	});
	//alert("333");
}
function miracode_on_style(){
		$('#miracode_txt').text("ON");
		$("#miracode_val").val(1);
		$("#miracode_control").children().addClass("ui-flipswitch-active");
		$("#miracode_val option").eq(1).attr("selected",true);  //on
}
function miracode_off_style(){
		$('#miracode_txt').text("OFF");
		$("#miracode_val").val(0);
		$("#miracode_control").children().removeClass("ui-flipswitch-active");
		$("#miracode_val option").eq(0).attr("selected",true);  //off
}
var miracode_value=0;
function set_miracode_text(){
	var cgiurl= "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime(); //To solve the IE cache ,by cxf
	var	value=$('#miracode_val').val();
	miracode_value=value;
	var str=value+"\n"+"set_miracode\n";
	$.get(cgiurl, {type:str}, function(ret) {	
	}, "text");
	
	if(Number(miracode_value)==1){
		
		miracode_on_style();
	} else {
		
		miracode_off_style();
	}
	
}
function get_miracode_text()
{
	var value;
	var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
	$.get(cgiurl, {type:'get_miracode'}, function(ret)
	{	
	value=Number(ret);
	$('#miracode_val').val(value);
	miracode_value = value;
	if(value)
		{
		miracode_on_style();
		}
	else{
		miracode_off_style();
		}
	}, "text");			
}

function set_miracode_listener()
{
	/*
	$("#miracode_set").unbind().bind("swipeleft click swiperight",function(e)
	{
		set_miracode_text();
	 });
	*/
	$("#set_miracode_ok").unbind().bind("swipeleft click swiperight",function(e)
	{
		set_miracode_text();
		$('#miracode_set').popup("close");
	 });	
	$("#set_miracode_cel").unbind().bind("swipeleft click swiperight",function(e)
	{
		if(miracode_value != $('#miracode_val').val()){
			if(miracode_value)
				miracode_on_style();
			else
				miracode_off_style();
		}
		 $('#miracode_set').popup("close");
	 });
	
}
function snmp_on_style(){
	$("#snmp_val").val(1);
	$("#snmp_control").children().addClass("ui-flipswitch-active");
	$("#snmp_val option").eq(1).attr("selected",true);  //on
}
function snmp_off_style(){
	$("#snmp_val").val(0);
	$("#snmp_control").children().removeClass("ui-flipswitch-active");
	$("#snmp_val option").eq(0).attr("selected",true);  //off
}
function set_snmp_text(){
	var cgiurl= "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime(); //To solve the IE cache ,by cxf
	var	value=$('#snmp_val').val();
	var str=value+"\n"+"set_snmp\n";
	if(Number(value))
	{ 
		snmp_on_style();
		$('#snmp_open').show();
	}
	else
	{	
		snmp_off_style();
		$('#snmp_open').hide();
	}
	 $.get(cgiurl, {type:str}, function(ret)
	{	
			
	}, "text");
	
}
function get_snmp_userinfo()
{
	var value;
	var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
	$.get(cgiurl, {type:'get_userinfo_snmp'}, function(str)
	{	
	      
		var strs= new Array(); //定义一数组 
           	strs=str.split("\n"); 
	//	alert("len="+strs.length);
	 	if(strs.length>1)
		{
	  		 var strs1= new Array(); //定义一数组
			 strs1=strs[1].split(" "); 
			// alert("len1="+strs1.length);
		  	 for (i=0;i<strs1.length ;i++ ) 
			{ 
				//alert("hi="+strs1[i]);//分割后的字符输出 
			} 
			if(strs1.length==6)
			{
    				$('#usm_user').val(strs1[1]);
    	 			$('#auth_algorithm').val(strs1[2]);
      			  	$('#auth_password').val(strs1[3]);
        			$('#privacy_algorithm').val(strs1[4]);
        			$('#privacy_pasword').val(strs1[5]);

			}
			else if(strs1.length==7)
			{
			 	$('#usm_user').val(strs1[1]);
    	 		 	$('#auth_algorithm').val(strs1[2]);
      				$('#auth_password').val(strs1[3]);
        			$('#privacy_algorithm').val(strs1[4]);
        			$('#privacy_pasword').val("");


			}
			else if(strs1.length==4)
			{
 				$('#usm_user').val(strs1[1]);
    	 		 	$('#auth_algorithm').val(strs1[2]);
      				$('#auth_password').val("");
        			$('#privacy_pasword').val("");
			}

		 }
		
	}, "text");			
}
function get_snmp_text()
{
	var value;
      // $('#usm_user').attr("Disabled","false");
     //  $('#usm_user').parent().addClass("ui-state-disabled");get_snmp_userinfo     
	var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
	$.get(cgiurl, {type:'get_snmp'}, function(ret)
	{	
		value=Number(ret);
		$('#snmp_val').val(value);
		if(value)
		{ 
			snmp_on_style();
			$('#snmp_open').show();
		}
		else
		{
                     $('#snmp_open').hide();
			snmp_off_style();
		}
	}, "text");			
}


function creat_snmp_user_listener()
{
	var	value="";
	var 	str="";
	if($('#usm_user').val()=="")
		alert("Please input user name");
	else if($('#auth_password').val()=="")
	{
     		if($('#privacy_pasword').val()!="")
     		{
			alert("Auth password cann't be empty if privacy pasword is not empty");
     		}
		else
		{
			value=$('#usm_user').val()+"\n"+$('#auth_algorithm').val()+"\n"+$('#privacy_algorithm').val()+"\n";
			str=value+"creat_snmp_user_without_psk";
			var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
			$.get(cgiurl, {type:str}, function(ret)
			{	
		
			}, "text");
		}
	
	}
	else
	{
	 	value=$('#usm_user').val()+"\n"+$('#auth_algorithm').val()+"\n"+$('#auth_password').val()+"\n"+$('#privacy_algorithm').val()+"\n"+$('#privacy_pasword').val()+"\n";
		str=value+"creat_snmp_user_with_psk";
		var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
		$.get(cgiurl, {type:str}, function(ret)
		{	
		
		}, "text");

	}
		
}
function set_snmp_listener()
{
	$("#snmp_set").unbind().bind("swipeleft click swiperight",function(e)
	{
		set_snmp_text();
	});
	$('#creat_user_ok').click(function(e) 
	{
	     creat_snmp_user_listener();
	});	
	
}
  
function Check_business_customer_ENABLE()
{
	$.get("cgi-bin/wifi_info_GET.cgi", {type:'CONFIG_BUSINESS_CUSTOMER'}, function(ret) 
	{	
		//alert("Check_ota_server_ENABLE:"+ret);
		business_customer_ENABLE=Number(ret);
	}, "text");			
}


function Get_text_from_textstring(text_string)
{
	var text_string_tmp=text_string.substring(start,text_string_length);	
	stop=text_string_tmp.indexOf("\n");
	start=0;
	text_indeed=text_string_tmp.substring(start,stop);
	start=stop+1;
	text_string_global=text_string_tmp;
	return text_indeed;
}


var Totle_number;
var Totle_index_str;
var split_index_array = new Array();
var Totle_all_str;
var cnt=0;

function Get_Totle_index_from_textstring(text_string)
{
	var text_string_tmp=text_string.substring(start,text_string_length);	
	stop=text_string_tmp.indexOf(":");
	start=0;
	text_indeed=text_string_tmp.substring(start,stop);
	start=stop+1;
	text_string_global=text_string_tmp;
	return text_indeed;
}

function Get_text_from_textstring2(text_stringx)
{
var text_string_tmp="";
	if(cnt>Totle_number){
	 	return 0;
	}
	else{
		
		//stop=text_string_tmp.indexOf("\n");
		//start=0;
		text_indeed=text_stringx.substring(0,split_index_array[cnt]);
		//start+=split_index_array[cnt];
		text_string_tmp=text_stringx.substring(split_index_array[cnt]);	
		text_string_global=text_string_tmp;
		
		//alert(text_stringx+"_"+split_index_array[cnt]+"_"+text_indeed+"_"+text_string_tmp+"_"+);
		cnt++;
		
		return text_indeed;
	}
}





function Get_main_language()
{
	var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
	$.get(cgiurl, {type:'get_main_text'}, function(main_text_string)
	{	
	//alert(main_text_string);
		if(main_text_string.length)
		{
		//var JSONObject = eval ("(" + main_text_string + ")");
		var JSONObject = JSON.parse(main_text_string); 
		//$('#alert_txt').text(JSONObject+"--"+JSONObject.main_string1+"--"+JSONObject.length);
			main_text=JSONObject.main_string1;
			$('#IDS_MAIN_LAN0').text(main_text);
			//text_string=text_string_global;
			$('#center_txt1').text(JSONObject.main_string2);
			$('#center_txt2').text(JSONObject.main_string3);
			login_txt=JSONObject.main_string4;
			$('#user_login').text(login_txt);
			//text_string=text_string_global;	
			logout_txt=JSONObject.main_string5;
			$('#login_out').text(logout_txt);
			//text_string=text_string_global;	
			$('#IDS_MENU_LAN2').text(JSONObject.main_string6);
			//text_string=text_string_global;	
			password_txt=JSONObject.main_string7;
			$('#IDS_STTING_PAS').text(password_txt);//password
			//text_string=text_string_global;	
			$('#IDS_MENU_LAN3').text(JSONObject.main_string8);
			//text_string=text_string_global;	
			admin_txt=JSONObject.main_string9;//admin
			//text_string=text_string_global;	
			$('#IDS_MENU_LAN5').text(JSONObject.main_string10);
			//text_string=text_string_global;	
			$('#IDS_MENU_LAN6').text(JSONObject.main_string11);//close menu
			//text_string=text_string_global;	
			airview_txt=JSONObject.main_string12;
			//text_string=text_string_global;	
			confercon_txt=JSONObject.main_string13;
			
			Get_menu_language();
		}
		else
		{
			$('#paswod_title').text("Password");
			$('#title_confer').text("Conference Control");
			$('#airview_title').text("Air view");
			$('#title_link').text("Link Status");
			$('#upload_ok').text("OK");
			$('#upload_ok2').text("OK");
			$('#ota_path_ok').text("OK");
			
			$('#title_rest').text("Reset to default");

			$("#set_title_admin").text("SETUP");//websetting title
			$('#center_txt1').text("The first connecting user is designated as the Host and the others as the Guests.");
			$('#center_txt2').text("Login for administration and more advanced settings.");

			var i=0;
			airview_txt=leftmenu[i];
			i++;
			linkstatus_txt=leftmenu[i];
			i++;
			confercon_txt=leftmenu[i];
			i++;
			device_management_txt=leftmenu[i];
			i++;
			network_setup_txt=leftmenu[i];
			i++;
			hostauthority_txt=leftmenu[i];
			i++;
			accesscontrol_txt=leftmenu[i];
			i++;
			rebootcontrol_txt=leftmenu[i];
			
		}
	}, "text");	

	
}

function Get_menu_language()
{
	var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
	$.get(cgiurl, {type:'get_menu_text'}, function(menu_text_string)
	{	
		if(menu_text_string.length)
		{
		//var JSONObject = eval ("(" + menu_text_string + ")");
		var JSON2Object = JSON.parse(menu_text_string); 
			//$('#jsprintext').text(main_text_string.length+" | "+main_text_string);
			//text_string_length=menu_text_string.length;
			linkstatus_txt=JSON2Object.menu_string1;
			//text_string=text_string_global;	
			airsetup_txt=JSON2Object.menu_string2;
			//text_string=text_string_global;	
			addca_txt=JSON2Object.menu_string3;
			//text_string=text_string_global;	
			hostauthority_txt=JSON2Object.menu_string4;
			//text_string=text_string_global;	
			accesscontrol_txt=JSON2Object.menu_string5;
			//text_string=text_string_global;	
			otaserver_txt=JSON2Object.menu_string6;
			//text_string=text_string_global;	
			rebootcontrol_txt=JSON2Object.menu_string7;
			//text_string=text_string_global;	
			resetcontrol_txt=JSON2Object.menu_string8;
			//text_string=text_string_global;	
			ok_txt=JSON2Object.menu_string9;//ok
			$('#login_usrname_ok').text(ok_txt);//ok
			//text_string=text_string_global;	
			cancle_txt=JSON2Object.menu_string10;//Cancel
			$('#cancle_login').text(cancle_txt);//Cancel
			//text_string=text_string_global;	
			device_management_txt=JSON2Object.menu_string11;
			network_setup_txt=JSON2Object.menu_string12;
			admin_setup_txt=JSON2Object.menu_string13;
			about_setup_txt=JSON2Object.menu_string14;
			password_err_txt=JSON2Object.menu_string15;

			console.log(main_text+">"+password_err_txt+">"+confercon_txt+">"+linkstatus_txt+">"+device_management_txt+">"+network_setup_txt+">"+admin_setup_txt+">"+about_setup_txt);
			
			$('#airview_title').text(airview_txt);
			//$('#airview_txt').text(airview_txt);
			$('#paswod_title').text(password_txt);
			$('#title_confer').text(confercon_txt);
			$('#title_link').text(linkstatus_txt);

			$('#host_ok').text(ok_txt);
			$('#discon_ok').text(ok_txt);
			$('#host_cancl').text(cancle_txt);
			$('#discon_cancl').text(cancle_txt);

			$('#upload_ok').text(ok_txt);
			$('#upload_ok2').text(ok_txt);
			$('#sys_boot').text(ok_txt);
			$('#reupload_ok').text(ok_txt);
			$('#sys_canel').text(cancle_txt);
			$('#reupload_cal').text(cancle_txt);
			$('#set_internet_access_ok').text(ok_txt);
			$('#set_internet_access_cel').text(cancle_txt);
			$('#set_castcode_ok').text(ok_txt);
			$('#set_castcode_cel').text(cancle_txt);

			$('#ota_path_ok').text(ok_txt);

			$('#title_rest').text(resetcontrol_txt);

			$('#pass_err_ok').text(ok_txt);
			$('#reload_main').text(ok_txt);
			
			$("#set_title_admin").text(admin_setup_txt);
			var urlinfo=window.location.href; 
			var offset=urlinfo.indexOf("advanced=1");   
			if(offset>0){
			    $("#set_title").text(device_management_txt);//websetting title
				console.log("device_management_txt: "+device_management_txt);
			}else{
			    $("#set_title").text(airsetup_txt);
				console.log("airsetup_txt: "+airsetup_txt);
			}
			$("#set_network_title").text(network_setup_txt);
			$("#host_authoryty_txt").text(hostauthority_txt);
			$("#about_title").text(about_setup_txt);//about

			
		} else {
		
			var i=0;
			airview_txt=leftmenu[i];
			i++;
			linkstatus_txt=leftmenu[i];
			i++;
			confercon_txt=leftmenu[i];
			i++;
			device_management_txt=leftmenu[i];
			i++;
			network_setup_txt=leftmenu[i];
			i++;
			hostauthority_txt=leftmenu[i];
			i++;
			accesscontrol_txt=leftmenu[i];
			i++;
			rebootcontrol_txt=leftmenu[i];
			i++;
			admin_setup_txt=leftmenu[i];
			i++;
			about_setup_txt=leftmenu[i];
			
			
		}

	}, "text");	

	
}




var airview_text_string=-1;
function Get_airview_language()
{
	var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
	$.get(cgiurl, {type:'get_view_text'}, function(airview_text_string)
	{	
	//alert(airview_text_string);
		if(airview_text_string.length)
		{
			start=0;
			text_string_length=0;
			text_string=airview_text_string;
			//$('#jsprintext2').text(airview_text_string.length+" | "+airview_text_string);
			text_string_length=airview_text_string.length;
			$('#view_warn').text(Get_text_from_textstring(text_string));

		}
		else
		{
		$('#view_warn').text("Click/Tap for zooming and panning");
		}
	}, "text");	
}
var password_text_string=-1;
function Get_password_language()
{
	var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
	$.get(cgiurl, {type:'get_paswod_text'}, function(password_text_string)
	{	
	//alert(airview_text_string);
		if(password_text_string.length)
		{
			start=0;
			text_string_length=0;
			text_string=password_text_string;
			$('#jsprintext2').text(password_text_string.length+" | "+password_text_string);
			text_string_length=password_text_string.length;
			$('#title_txt').text(Get_text_from_textstring(text_string));
			text_string=text_string_global;	
			$('#new_paswod').text(Get_text_from_textstring(text_string));
			text_string=text_string_global;	
			$('#confirm_paswod').text(Get_text_from_textstring(text_string));
			text_string=text_string_global;	
			$('#save_usrname_ok').text(Get_text_from_textstring(text_string));//apply
			text_string=text_string_global;	
			$('#pass_err_txt').text(Get_text_from_textstring(text_string));
			text_string=text_string_global;	
			$('#pass_warn_txt').text(Get_text_from_textstring(text_string));
			text_string=text_string_global;	
			error_t=Get_text_from_textstring(text_string);//error
			text_string=text_string_global;	
			$('#first_modfiy').text(Get_text_from_textstring(text_string));
			text_string=text_string_global;	
			chcek_error_txt=Get_text_from_textstring(text_string);//Only the letters and numbers are allowed!
			text_string=text_string_global;	
			chcek_error_txt2=Get_text_from_textstring(text_string);//Password must be between 6 and 64 characters long.
		}
		else
		{
		}
	}, "text");	
}

var conference_text_string=-1;
function Get_confer_language()
{
	var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
	$.get(cgiurl, {type:'get_confer_text'}, function(conference_text_string)
	{	
	//alert(airview_text_string);
		if(conference_text_string.length)
		{
			start=0;
			text_string_length=0;
			text_string=conference_text_string;
			//$('#jsprintext2').text(conference_text_string.length+" | "+conference_text_string);
			text_string_length=conference_text_string.length;
			$('#ipaddress').text(Get_text_from_textstring(text_string));
			text_string=text_string_global;	
			$('#ctrlaction').text(Get_text_from_textstring(text_string));
			text_string=text_string_global;	
			$('#shutall_txt').text(Get_text_from_textstring(text_string));
			text_string=text_string_global;	
			var load_txt=Get_text_from_textstring(text_string);//loading
			text_string=text_string_global;	
			$('#warn_host_txt').text(Get_text_from_textstring(text_string));
			text_string=text_string_global;	
			$('#warn_discon_txt').text(Get_text_from_textstring(text_string));
			text_string=text_string_global;	
			$('#link_ip').text(Get_text_from_textstring(text_string));
			text_string=text_string_global;	
			$('#link_role').text(Get_text_from_textstring(text_string));

		}
		else
		{
			$('#ipaddress').text("IP / Client");
			$('#ctrlaction').text("Split");
			$('#shutall_txt').text("disbale All");
			//$('#warn_host_txt').text("");
			//$('#warn_discon_txt').text("");
			$('#link_ip').text("IP / Operation Mode");
			$('#link_role').text("Role");
		
		}
	}, "text");	
}

var fileupload_text_string=-1;
function Get_fileupload_language()
{
	var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
	$.get(cgiurl, {type:'get_filuplod_text'}, function(fileupload_text_string)
	{	
	//alert(airview_text_string);
		if(fileupload_text_string.length)
		{
			start=0;
			text_string_length=0;
			text_string=fileupload_text_string;
			$('#jsprintext2').text(fileupload_text_string.length+" | "+fileupload_text_string);
			text_string_length=fileupload_text_string.length;
			$('#title_txt').text(Get_text_from_textstring(text_string));
			text_string=text_string_global;	
			$('#txt_acuplod').text(Get_text_from_textstring(text_string));
			text_string=text_string_global;	
			$('#txt_eapmschap').text(Get_text_from_textstring(text_string));
			text_string=text_string_global;	
			$('#txt_eaptls').text(Get_text_from_textstring(text_string));
			text_string=text_string_global;	
			$('#upload_warn').text(Get_text_from_textstring(text_string));
			text_string=text_string_global;	
			$('#warn_txt').text(Get_text_from_textstring(text_string));
			text_string=text_string_global;	
			$('#fail_txt').text(Get_text_from_textstring(text_string));

		}
		else
		{
			$('#title_txt').text("file upload");
			$('#txt_acuplod').text("Digital certificate upload");
			$('#txt_eapmschap').text("For WPA2 EAP-MSCHAP, please upload ");
			$('#txt_eaptls').text("For WPA2 EAP-TLS, please upload");
			$('#upload_warn').text("After uploading,the system will reboot!");

		
		}
	}, "text");	
}

var host_authority_text_string=-1;
function Get_host_authority_language()
{
	var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
	$.get(cgiurl, {type:'get_authority_host_text'}, function(host_authority_text_string)
	{	
		if(host_authority_text_string.length)
		{
			start=0;
			text_string_length=0;
			text_string=host_authority_text_string;
			$('#jsprintext2').text(host_authority_text_string.length+" | "+host_authority_text_string);
			text_string_length=host_authority_text_string.length;
			$('#title_host').text(Get_text_from_textstring(text_string));
			text_string=text_string_global;	
			$('#txt_confer').text(Get_text_from_textstring(text_string));
			text_string=text_string_global;	
			$('#txt_network').text(Get_text_from_textstring(text_string));
			text_string=text_string_global;	
			$('#txt_device').text(Get_text_from_textstring(text_string));
			text_string=text_string_global;	
			$('#txt_access').text(Get_text_from_textstring(text_string));
			text_string=text_string_global;	
			$('#txt_pashide').text(Get_text_from_textstring(text_string));
			text_string=text_string_global;	
			$('#txt_rebotcot').text(Get_text_from_textstring(text_string));
			text_string=text_string_global;	
			$('#host_message').text(Get_text_from_textstring(text_string));
			text_string=text_string_global;	
			$('#txt_mis').text(Get_text_from_textstring(text_string));
			text_string=text_string_global;	
			$('#txt_meet').text(Get_text_from_textstring(text_string));
			text_string=text_string_global;	
			$('#txt_meet2').text(Get_text_from_textstring(text_string));
			text_string=text_string_global;	
			$('#txt_guset').text(Get_text_from_textstring(text_string));
			text_string=text_string_global;	
			$('#txt_guset2').text(Get_text_from_textstring(text_string));
			
		}
		else
		{
			$('#title_host').text("Host Authority");
			$('#txt_confer').text("Conference control");
			$('#txt_view').text("AirView enable");
			$('#txt_set').text("Setting control");
			$('#txt_access').text("Internet access");
			$('#txt_pashide').text("Password hiden control");
			$('#txt_rebotcot').text("Reboot control");
			$('#host_message').text("Three-layered authority for MIS, meeting moderator and attendees. Admin configures Host's authority on this page.");

			// no mult lang
			$('#txt_mis').text("Admin for MIS");
			$('#txt_meet').text("Host for");
			$('#txt_meet2').text("meeting moderator");
			$('#txt_guset').text("Guest for");
			$('#txt_guset2').text("meeting attendees");

		
		}
	}, "text");	
}

var control_access_text_string=-1;
function Get_control_access__language()
{
	var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
	$.get(cgiurl, {type:'get_control_access_text'}, function(control_access_text_string)
	{	
		if(control_access_text_string.length)
		{
			start=0;
			text_string_length=0;
			text_string=control_access_text_string;
			$('#jsprintext2').text(control_access_text_string.length+" | "+control_access_text_string);
			text_string_length=control_access_text_string.length;
			$('#title_access').text(Get_text_from_textstring(text_string));
			text_string=text_string_global;	
			$('#txt_castcode').text(Get_text_from_textstring(text_string));
			text_string=text_string_global;	
			$('#txt_passhid').text(Get_text_from_textstring(text_string));
			text_string=text_string_global;	
			$('#Internet_Access').text(Get_text_from_textstring(text_string));
			text_string=text_string_global;	
			$("label[for=internet_access_con_0]").text(Get_text_from_textstring(text_string));
			text_string=text_string_global;	
			$("label[for=internet_access_con_1]").text(Get_text_from_textstring(text_string));
			text_string=text_string_global;	
			$("label[for=internet_access_con_2]").text(Get_text_from_textstring(text_string)); //IDS_ACCESS_CON_LAN6
			//text_string=text_string_global;	
			//$('#host_message').text(Get_text_from_textstring(text_string));
			text_string=text_string_global;	
			$('#access_message').text(Get_text_from_textstring(text_string));
			text_string=text_string_global;	
			castcode_warn1=Get_text_from_textstring(text_string); //Please input the Numbers.
			text_string=text_string_global;	
			castcode_warn2=Get_text_from_textstring(text_string); //Only four digits are allowed.
			text_string=text_string_global;	
			$('#castcode_txt1').text(Get_text_from_textstring(text_string)); //OFF
			text_string=text_string_global;	
			$('#castcode_txt2').text(Get_text_from_textstring(text_string)); //Random
			text_string=text_string_global;	
			$('#castcode_txt3').text(Get_text_from_textstring(text_string)); //Fixed

		}
		else
		{

		
			$("label[for=internet_access_con_0]").text("All allowed"); 
			$("label[for=internet_access_con_1]").text("Casting users allowed"); 
			$("label[for=internet_access_con_2]").text("All denied"); 

			// no mult lang
			$('#access_message').text("Control projection and internet access");

		
		}
	}, "text");	
}
var otaserver_text_string=-1;
function Get_otaserver_language()
{
	var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
	$.get(cgiurl, {type:'get_serverota_text'}, function(otaserver_text_string)
	{	
	//alert(airview_text_string);
		if(otaserver_text_string.length)
		{
			start=0;
			text_string_length=0;
			text_string=otaserver_text_string;
			//$('#jsprintext2').text(airview_text_string.length+" | "+airview_text_string);
			text_string_length=otaserver_text_string.length;
			var otaserver_txt=Get_text_from_textstring(text_string);//OTA SERVER

			$('#title_otaserver').text(otaserver_txt);
			$('#otaserver_txt').text(otaserver_txt+":");

		}
		else
		{
		$('#title_otaserver').text("OTA SERVER");
		$('#otaserver_txt').text("OTA SERVER:");
		}
	}, "text");	
}

var rebrset_text_string=-1;
function Get_reboot_reset_language()
{
	var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
	$.get(cgiurl, {type:'get_rebret_text'}, function(rebrset_text_string)
	{	
	//alert(airview_text_string);
		if(rebrset_text_string.length)
		{
			start=0;
			text_string_length=0;
			text_string=rebrset_text_string;
			$('#jsprintext2').text(rebrset_text_string.length+" | "+rebrset_text_string);
			text_string_length=rebrset_text_string.length;
			$('#title_rbot').text(Get_text_from_textstring(text_string));
			$('#reboot').text($('#title_rbot').text());
			text_string=text_string_global;	
			Refresh_txt=Get_text_from_textstring(text_string);//Refresh
			text_string=text_string_global;	
			reboot_message=Get_text_from_textstring(text_string);//System reboots, please wait.
			text_string=text_string_global;	
			$('#rebot_warn').text(Get_text_from_textstring(text_string));
			text_string=text_string_global;	
			$('#reset').text(Get_text_from_textstring(text_string));
			text_string=text_string_global;	
			reset_message=Get_text_from_textstring(text_string);//The system is reboot!please wait.
			text_string=text_string_global;	
			$('#reset_warn').text(Get_text_from_textstring(text_string));
			text_string=text_string_global;	
			yes_txt=Get_text_from_textstring(text_string);//yes
			text_string=text_string_global;	
			no_txt=Get_text_from_textstring(text_string);//no

			$('#reboot_ok').text(yes_txt);
			$('#reboot_cancl').text(no_txt);
			$('#reset_ok').text(yes_txt);
			$('#reset_ent').text(yes_txt);
			$('#reset_cancl').text(no_txt);
			
		}
		else
		{
		$('#title_rbot').text("Reboot");
		$('#reboot').text("Reboot");
		$('#rebot_warn').text("Are you sure to reboot system?");
		Refresh_txt="Refresh";
		reboot_message="System reboots, please wait.";
		reset_message="The system is reboot!please wait.";
		$('#reboot_ok').text("Yes");
		$('#reboot_cancl').text("No");
		$('#reset_ok').text("Yes");
		$('#reset_cancl').text("No");

		$('#reset').text("Reset");
		$('#reset_warn').text("All the configurations will be lost! Are you sure?");
		}
	}, "text");	
}

function keyPresscheck(){
	var num= $("#user_input").val();
	var keyCode = event.keyCode;
	//alert(keyCode+"  "+num);
	 if ((keyCode >= 48 && keyCode <= 57))    
		{   
			if(num!=""){
				if (!(/(^[0-9]\d*$)/.test(num)))
				{
				//alert("---1---");
				 $("#castcode_warn_text").text(castcode_warn1);
				  event.returnValue = false;   
				}
				else{
					//alert("---0---");
					 $("#castcode_warn_text").text(castcode_warn2);
					 event.returnValue = true;  
					}
					
				}
			else{
			//alert("---2---");
			 $("#castcode_warn_text").text(castcode_warn2);
			 event.returnValue = true;  
			 }  
		 } 
	 else { 
		 	//alert("---3---");
			 $("#castcode_warn_text").text(castcode_warn1);
			   event.returnValue = false;    
		} 
	
}
function To_adminsetup() {
        location.href = "websetting_admin.html";
}
function To_networksetup() {
        location.href = "networksetup.html";
}
function To_devicesetup() {
        location.href = "websetting.html?advanced=1";
}
function To_adminpass() {
        location.href = "Password.html";
}
function To_otaserver() {
        location.href = "ota_server.html";
}
