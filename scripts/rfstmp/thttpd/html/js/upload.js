var show_waiting_tick=-1;
var show_waiting_tick_id=0;
var intl=0;



window.onload = function() 
{
parameter_check();
click_listener();

}

function parameter_check()
{
	urlinfo=window.location.href; 
	var len=urlinfo.length; 
	var offset=urlinfo.indexOf("?");        
	if(offset==-1){ return 0;
	}
	else
	{
	var newsidinfo=urlinfo.substr(offset+1,len);
	var newsids=newsidinfo.split("=");
	var rtn=decodeURI(newsids[1]);
	if(parseInt(rtn)==0)
		{
			//showok_dlg();	
			
			intl=setTimeout(showok_dlg,500);
			$('#warn_tim').text("10");	
		}
	else{
		intl=setTimeout(showreuplod_dlg,500);
		}

	}
}


function timetick_function()
{
	if(show_waiting_tick==10)
	{	
	show_waiting_tick=9;
	}
	$('#warn_tim').html(show_waiting_tick);	
	if(show_waiting_tick==0)
	{
		if(show_waiting_tick_id>0)			
		{
			clearInterval(show_waiting_tick_id);
			show_waiting_tick_id=-1;	
			show_waiting_tick=10;
			$("#alert_warn").popup('close');
			reboot();
		}		
	}
	else
		show_waiting_tick=show_waiting_tick-1;	

}
function set_timetick()
{
	if(show_waiting_tick_id>0)			
	{
		clearInterval(show_waiting_tick_id);
		show_waiting_tick_id=-1;
		//$('#tick').html("");	
	}
	show_waiting_tick=10;
	$('#warn_tim').html(show_waiting_tick);	
	show_waiting_tick_id=self.setInterval(timetick_function,1000);	
}
 
function showok_dlg()
{
	set_timetick();
	$('#alert_warn').popup("open");
}
 
function showreuplod_dlg()
{
	$('#fail_warn').popup("open");
}



 function reboot()
{
	var myDate = new Date();
	var year=myDate.getFullYear(); 
	var month=myDate.getMonth()+1;
	var date=myDate.getDate();
	var hour=myDate.getHours();
	var min=myDate.getMinutes();
	var sec=myDate.getSeconds();
	if(month<10)month="0"+month;
	if(date<10)date="0"+date;
	if(hour<10)hour="0"+hour;
	if(min<10)min="0"+min;
	if(sec<10)sec="0"+sec;
	var fulltime=year.toString()+month.toString()+date.toString()+hour.toString()+min.toString()+sec.toString();
	fulltime="reboot*"+fulltime;
	//alert(fulltime);
	$.get("cgi-bin/windir.cgi", {fullname:fulltime}, function(dir_list)
	{	

	  	});

}

 function  click_listener(){
	 	
	 
$("#reupload_ok").click(function(){
	$('#fail_warn').popup("close");
	});

$("#reupload_cal").click(function(){
	$('#fail_warn').popup("close");
});

$("#sys_boot").click(function(){

	$('#alert_warn').popup("close");
	var myDate = new Date();
	var year=myDate.getFullYear(); 
	var month=myDate.getMonth()+1;
	var date=myDate.getDate();
	var hour=myDate.getHours();
	var min=myDate.getMinutes();
	var sec=myDate.getSeconds();
	if(month<10)month="0"+month;
	if(date<10)date="0"+date;
	if(hour<10)hour="0"+hour;
	if(min<10)min="0"+min;
	if(sec<10)sec="0"+sec;
	var fulltime=year.toString()+month.toString()+date.toString()+hour.toString()+min.toString()+sec.toString();
	fulltime="reboot*"+fulltime;
	//alert(fulltime);
	$.get("cgi-bin/windir.cgi", {fullname:fulltime}, function(dir_list)
	{	

			
	  	});
		
});

$("#sys_canel").click(function(){
	$('#alert_warn').popup("close");
	clearInterval(show_waiting_tick_id);
	show_waiting_tick_id=-1;	
});

$("#upload_ok").click(function(){

	fileName.submit();
	//$('#alert_warn').popup("open");
	//set_timetick();
	});

	
$("#upload_ok2").click(function(){
 	fileName1.submit();
	intl1=setInterval(a1,3000);
	});
$("#upload4_ok").click(function(){
      // alert("12345");
	fileName4.submit();
	});
}

var intl1=0;
var intl2=0;
function a1()
{
	fileName2.submit();
	clearInterval(intl1);
	intl2=setInterval(a2,3000);
}
function a2()
{
	fileName3.submit();
	clearInterval(intl2);
}


