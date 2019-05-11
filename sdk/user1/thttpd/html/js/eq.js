//EQ mode select------
var Source_i;
var Source_Selection = new Array();
var Source_Selection=new Array("Club","Dance","Flat","Classical","Laptop speakers/headphones","Large hall","Party","Pop","Reggae","Rock","Soft","Ska","Full Bass","Soft Rock","Full Treble","Full Bass & Treble","Live","Techno","Custom");
var custom_index_id=new Array("eq70","eq180","eq320","eq600","eq1K","eq3K","eq6K","eq12K","eq14K","eq16K");
var get_connected_ssid_id=-1;
var back_string="";
var text_indeed;
var text_start;
var text_stop;
var text_string_global="";
var text_string;
var delay_get_data_id=-1;
var EQ_Value=[[50,50,41,34,34,34,41,50,50,50],
[25,31,44,50,50,66,69,69,50,50],
[50,50,50,50,50,50,50,50,50,50],
[50,50,50,50,50,50,69,69,69,76],
[38,22,36,60,57,46,38,25,17,12],
[23,23,34,34,50,63,63,63,50,50], 
[31,31,50,50,50,50,50,50,31,31], 
[55,38,31,30,36,53,57,57,55,55], 
[50,50,52,66,50,33,33,50,50,50], 
[30,38,65,71,60,39,26,22,22,22], 
[38,46,53,57,53,39,28,25,22,19], 
[57,63,61,52,39,34,26,25,22,25], 
[25,25,25,34,46,61,73,77,79,79], 
[39,39,44,52,61,65,60,52,42,26], 
[76,76,76,61,42,22,9,9,9,6], 
[31,34,50,69,63,46,28,22,19,19], 
[63,50,39,36,34,34,39,42,42,44], 
[30,34,50,65,63,50,30,25,25,26], 
[50,50,50,50,50,50,50,50,50,50]];

var Source_max=Source_Selection.length-1;
var screen_width,screen_height;

function soruce(value)
{
	switch(value)
	{
		case "left":
		if(Source_i==0)Source_i=Source_max;
		else Source_i--;
		break;
		case "right":
		if(Source_i==Source_max)Source_i=0;
		else Source_i++;
		break;
	}
//document.getElementById("eq_mode").innerHTML=Source_Selection[Source_i];
	$('#EQ_default_mode').text(Source_Selection[Source_i]);
	document.getElementById("eq70").value=EQ_Value[Source_i][0];
	document.getElementById("eq180").value=EQ_Value[Source_i][1];
	document.getElementById("eq320").value=EQ_Value[Source_i][2];
	document.getElementById("eq600").value=EQ_Value[Source_i][3];
	document.getElementById("eq1K").value=EQ_Value[Source_i][4];
	document.getElementById("eq3K").value=EQ_Value[Source_i][5];
	document.getElementById("eq6K").value=EQ_Value[Source_i][6];
	document.getElementById("eq12K").value=EQ_Value[Source_i][7];
	document.getElementById("eq14K").value=EQ_Value[Source_i][8];
	document.getElementById("eq16K").value=EQ_Value[Source_i][9];

	//--send date to cgi-----
	//eq_index=Source_Selection[Source_i];
	/********set  custom value to selection default*********/
	$('#custom_or_other').val("1");
	$('#eq_value').val(""+(-1));
	custom_index=Source_i;
	$('#custom_index').val(""+custom_index);
	$.post('cgi-bin/set_EQ.cgi', $('#config-form').serialize());	
	/**********************************************/

	$('#custom_or_other').val("0");
	$('#eq_index').val(""+Source_i);
	$.post('cgi-bin/set_EQ.cgi', $('#config-form').serialize());
}

function setValue(index,textId) 
{  //inputµÄid,,Ë÷ÒýºÅ
	//var xx = document.getElementById(textId).value;

	var xx =document.getElementById(textId).value;
	//document.getElementById("eq_mode").innerHTML="Custom";
	$('#EQ_default_mode').text(Source_Selection[18]);
	//document.getElementById("text11").innerHTML=xx;
	//document.getElementById("text22").innerHTML=index;
	//--send date to cgi-----
	/********set index to custom*********/
	$('#custom_or_other').val("0");
	$('#eq_index').val(""+18);
	$.post('cgi-bin/set_EQ.cgi', $('#config-form').serialize());
	/********************************/

	$('#custom_or_other').val("1");
	$('#eq_value').val(""+xx);
	custom_index=index;
	$('#custom_index').val(""+custom_index);
	$.post('cgi-bin/set_EQ.cgi', $('#config-form').serialize());
}
function Get_text_from_textstring(text_string)
{
	var text_string_tmp=text_string.substring(text_start,text_string_length);	
	text_stop=text_string_tmp.indexOf("\n");
	text_start=0;
	var text_temp=text_string_tmp.substring(text_start,text_stop);
	var start_indeed=text_temp.indexOf(":")+1;
	text_indeed=text_temp.substring(start_indeed,text_stop);
	text_start=text_stop+1;
	text_string_global=text_string_tmp;
	return text_indeed;
}
function Get_height_width_of_screen()
{

	sWidth=document.documentElement.scrollWidth
	  || document.body.scrollWidth;
	 sHeight=document.documentElement.scrollHeight
	   || document.body.scrollHeight;
   
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
	//document.getElementById('msg').value  ='width:' + screen_width + '; height:'+screen_height;
}
function Get_connected_ssid_tick()
{
	Get_height_width_of_screen();
	$.get("cgi-bin/wifi_info_GET.cgi", {type:'get_connected_ssid'}, function(connected_ssid)
	{			
		if(connected_ssid.length)
		{			
			back_string="websetting_Vertical.html";		
				if (screen_width >= 800) 
					{
					back_string="websetting_Horizontal.html";
					}

		}
		else
		{
			back_string="websetting_Vertical.html?back_not_connect_ap";		
				if (screen_width >= 800) 
					{
					back_string="websetting_Horizontal.html?back_not_connect_ap";	
					}
		}
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
function get_EQ_Custom()
{
	$.get("cgi-bin/wifi_info_GET.cgi", {type:'get_EQ_Custom'}, function(EQ_Custom_value)
	{			
		if(EQ_Custom_value.length>0)
		{
			text_start=0;
			text_string=EQ_Custom_value;
			text_string_length=EQ_Custom_value.length;
			for(var i=0;i<10;i++)
			{
				$("#"+custom_index_id[i]).val(Get_text_from_textstring(text_string));	
				text_string=text_string_global;						
			}
		}
	}, "text");	
}
function get_EQ_default()
{
	$.get("cgi-bin/wifi_info_GET.cgi", {type:'get_EQ_default'}, function(EQ_default_mode)
	{			
		Source_i=Number(EQ_default_mode);
		$('#EQ_default_mode').text(Source_Selection[Source_i]);
		if(18==Source_i)
		{
			get_EQ_Custom();
		}
		else
		{
			document.getElementById("eq70").value=EQ_Value[Source_i][0];
			document.getElementById("eq180").value=EQ_Value[Source_i][1];
			document.getElementById("eq320").value=EQ_Value[Source_i][2];
			document.getElementById("eq600").value=EQ_Value[Source_i][3];
			document.getElementById("eq1K").value=EQ_Value[Source_i][4];
			document.getElementById("eq3K").value=EQ_Value[Source_i][5];
			document.getElementById("eq6K").value=EQ_Value[Source_i][6];
			document.getElementById("eq12K").value=EQ_Value[Source_i][7];
			document.getElementById("eq14K").value=EQ_Value[Source_i][8];
			document.getElementById("eq16K").value=EQ_Value[Source_i][9];	
		}
	}, "text");	
}


function Back_home_listener()
{
	$('#Home').click(function(e) 
	{
		e.preventDefault();
		location.href=back_string;
	});	
}
 function delay_get_data_function()
 {
 	if(delay_get_data_id>0)			
	{
		clearInterval(delay_get_data_id);
		delay_get_data_id=-1;
	}
	Back_home_listener();
	get_EQ_default();
	Get_connected_ssid();
 }
function delay_get_data()
{
	if(delay_get_data_id>0)			
	{
		clearInterval(delay_get_data_id);
		delay_get_data_id=-1;
	}
	delay_get_data_id=self.setInterval(delay_get_data_function,400);		
}