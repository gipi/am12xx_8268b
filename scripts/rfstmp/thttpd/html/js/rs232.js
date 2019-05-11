var CGI_URL='./cgi-bin/rs232cmd.cgi?'
//var CGI_URL='http://192.168.32.143/cgi-bin/rs232cmd.cgi?'
var IMAGE_FOLDER='./img/rs232/'
var intervalID;

var Source_Selection=new Array("COMPUTER","COMPUTER2","Component","Component2","DVI-A","DVI-D","HDMI","HDMI2","Composite","S-Video","Network","USB Display","USB Reader","Wireless");
var Source_value=new Array("rgb","rgb2","component","component2","dvia","dvid","hdmi","hdmi2","composite","svid","network","usbdisp","usbread","wireless");
var Source_Array = new Array(Source_Selection, Source_value);

var Picture_Selection=new Array("Dynamic","Presentation","sRGB","Bright","Living Room","Game","Cinema","Standard","User1","User2","User3","ISF Day","ISF Night","3D");
var Picture_value=new Array("dynamic","presentation","srgb","bright","livingroom","game","cine","standard","user1","user2","user3","isfday","isfnight","3d");
var Picture_Array = new Array(Picture_Selection, Picture_value);

var Audiosource_Selection=new Array("pass Through off","Computer1","Computer2","Video/S-Video","Component","HDMI","HDMI2");
var Audiosource_value=new Array("off","rgb","rgb2","vid","component","hdmi","hdmi2");
var Audiosource_Array = new Array(Audiosource_Selection, Audiosource_value);

var color_Selection=new Array("Color Temperature-Warmer","Color Temperature-Warm","Color Temperature-Normal","Color Temperature-Cool","Color Temperature-Cooler","Color Temperature-native");
var color_value=new Array("warmer","warm","normal","cool","cooler","native");
var Color_Array = new Array(color_Selection, color_value);

var aspect_Selection=new Array("Aspect 4:3","Aspect 16:9","Aspect 16:10","Aspect Auto","Aspect Real","Aspect Letterbox","Aspect Wide","Aspect Anamorphic");
var aspect_value=new Array("4:3","16:9","16:10","auto","real","lbox","wide","ana");
var Aspect_Array = new Array(aspect_Selection, aspect_value);

var prjpos_Selection=new Array("Front Table","Rear Table","Rear Ceiling","Front Ceiling");
var prjpos_value=new Array("ft","re","rc","fc");
var Prjpos_Array = new Array(prjpos_Selection, prjpos_value);

var lambctl_Selection=new Array("Normal mode","Eco mode","Smart Eco mode","Smart Eco mode(LambCare)","Smart Eco mode(lumenCare)","Dual Brightest","Dual Reliable","Single Alternative","Single Alternative Eco");
var lambctl_value=new Array("lnor","eco","seco","seco1","seco2","seco3","dualbr","dualre","single","singleeco");
var Lampctl_Array = new Array(lambctl_Selection, lambctl_value);

var d3_Selection=new Array("3D Sync Off","3D Auto","3D Sync Top Bottom","3D Sync Frame Sequential","3D Frame Packing","3D Side by side","3D inverter disable","3D inverter","2D to 3D","3D nVidia");
var d3_value=new Array("off","at","tb","fs","fp","sbs","da","iv","2d3d","nvidia");
var D3_Array = new Array(d3_Selection, d3_value);

var rrecv_Selection=new Array("Remote Receiver-front+rear","Remote Receiver-front","Remote Receiver-rear","Remote Receiver-top","Remote Receiver-top+front","Remote Receiver-top+rear");
var rrecv_value=new Array("fr","rrecvf","rrecvr","rrecvt","rrecvtf","rrecvtr");
var rrecv_Array = new Array(rrecv_Selection, rrecv_value);
var connected_flag=0;

var initdata="";
var initdata_array=new Array(15);
var initdata_key=new Array("POW=","VOL=","MUTE=","SOUR=","APPMOD=","CT=","ASP=","PP=","LAMPM=","3D=","RR=","CON=","BRI=","COLOR=","SHARP=");


var id_table = [
{ElementId:"vol", catelog:"audctr", query:"qvol"},
{ElementId:"source", catelog:"source", query:"qsource", array:Source_Array},
{ElementId:"Picture", catelog:"pmode", query:"qpmode", array:Picture_Array},
//{ElementId:"Audiosource", catelog:"audsrc", query:"qaudsrc", array:Audiosource_Array},
{ElementId:"color", catelog:"coltemp", query:"qcoltemp", array:Color_Array},
{ElementId:"aspect", catelog:"aspect", query:"qaspect", array:Aspect_Array},
{ElementId:"prjpos", catelog:"prjpos", query:"qprjpos", array:Prjpos_Array},
{ElementId:"lambctl", catelog:"lambctl", query:"qlambctl", array:Lampctl_Array},
{ElementId:"d3", catelog:"3d", query:"q3d", array:D3_Array},
{ElementId:"rrecv", catelog:"rrecv", query:"qrrecv", array:rrecv_Array},
{ElementId:"Contrast", catelog:"picset", query:"qcontrast"},
{ElementId:"Brightness", catelog:"picset", query:"qbright"},
{ElementId:"Color", catelog:"picset", query:"qcolor"},
{ElementId:"Sharpness", catelog:"picset", query:"qsharp"},
]

window.onload = function(){
	showLog("this is log");
	get_curinitval();
	
	setTimeout("show_initsetting()",2000);
	
	
}

function show_initsetting()
{
	
	prase_initdata();
	//alert(initdata);
	if(initdata!="")
		connected_flag=1;
	show_page(connected_flag);
	

}
function show_page(value)
{

	var key="";
	var key_value="";
	var i=0;
	if(value==0){
		document.getElementById("hid_div").style.overflow="hidden";
	}
	else{

		document.getElementById("hid_div").style.overflow="visible";
		document.getElementById("con_log").innerHTML="";
		
	}
	//init power stat  
	if(initdata_array[0]=="pow=on")
		document.power.src="img_rs232/power-gray.png";
	else
		document.power.src="img_rs232/power.png";
	//inin vol vlaue
	
	document.getElementById("vol").innerHTML=initdata_array[1].substring(4);
	//mute
	if(initdata_array[2].indexOf("on"))
		document.mute.src="img_rs232/Volume-color-gray.png";
	else
		document.mute.src="img_rs232/Volume.png";


	//sour
	if(initdata_array[3]!=" "){
		key_value=initdata_array[3].substring(5);

		
		for(i=0;i<14;i++){
			if(Source_value[i]==key_value)
				Source_i=i;

		}
		document.getElementById("source").innerHTML=Source_Selection[Source_i];
	}else{
		//Source_i
		Source_i=0;
		document.getElementById("source").innerHTML=Source_Selection[Source_i];
	}


	 // picture mode
	 //sour
	 if(initdata_array[4]!=" "){
		key_value=initdata_array[4].substring(7);

		 i=0;
		for(i=0;i<14;i++){
			if(Picture_value[i]==key_value)
				Picture_i=i;

		}
		 document.getElementById("Picture").innerHTML=Picture_Selection[Picture_i];
	 }else{
		Picture_i=0;
		 document.getElementById("Picture").innerHTML=Picture_Selection[Picture_i];

	 }
	 
	//clour tempture
	 if(initdata_array[5]!=" "){
		key_value=initdata_array[5].substring(3);

		 i=0;
		for(i=0;i<6;i++){
			if(color_value[i]==key_value)
				color_i=i;

		}

		document.getElementById("color").innerHTML=color_Selection[color_i];
	 }else{
		color_i=0;
		document.getElementById("color").innerHTML=color_Selection[color_i];


	 }

	//asp
	 if(initdata_array[6]!=" "){
		key_value=initdata_array[6].substring(4);

		 i=0;
		for(i=0;i<8;i++){
			if(aspect_value[i]==key_value)
				aspect_i=i;

		}

		document.getElementById("aspect").innerHTML=aspect_Selection[aspect_i];
	 }else{

		aspect_i=0;

		document.getElementById("aspect").innerHTML=aspect_Selection[aspect_i];
	 }

	//pp
	 if(initdata_array[7]!=" "){
		key_value=initdata_array[7].substring(3);

		 i=0;
		for(i=0;i<4;i++){
			if(prjpos_value[i]==key_value)
				prjpos_i=i;

		}
		document.getElementById("prjpos").innerHTML=prjpos_Selection[prjpos_i];
	 }else
	 {

		prjpos_i=0;
		document.getElementById("prjpos").innerHTML=prjpos_Selection[prjpos_i];

	 }

	//LAMPM
	if(initdata_array[8]!=" "){
		key_value=initdata_array[8].substring(6);

		i=0;
		for(i=0;i<10;i++){
			if(lambctl_value[i]==key_value)
				lambctl_i=i;

		}
		document.getElementById("lambctl").innerHTML=lambctl_Selection[lambctl_i];

		
	}else{

		lambctl_i=0;

		document.getElementById("lambctl").innerHTML=lambctl_Selection[lambctl_i];
	}
	if(initdata_array[9]!=" "){
		//3d
		key_value=initdata_array[9].substring(6);
		i=0;
		for(i=0;i<10;i++){
			if(d3_value[i]==key_value)
				d3_i=i;

		}

		document.getElementById("d3").innerHTML=d3_Selection[d3_i];
		
	}else
	{
		d3_i=0;
		document.getElementById("d3").innerHTML=d3_Selection[d3_i];
	}
	//rr
	//alert("rr");
	if(initdata_array[10]!=" "){
		//alert("rr1");
		key_value=initdata_array[10].substring(3);
		i=0;
		for(i=0;i<6;i++){
			if(rrecv_value[i]==key_value)
				rrecv_i=i;

		}

		document.getElementById("rrecv").innerHTML=rrecv_Selection[rrecv_i];
	}else{

		rrecv_i=0;
		document.getElementById("rrecv").innerHTML=rrecv_Selection[rrecv_i];
	}
	//CON
	if(initdata_array[11]!=" "){
		key_value=initdata_array[11].substring(4);
		Contrast_i=Number(key_value);
		//alert(Contrast_i);
		document.getElementById("Contrast").innerHTML=Contrast_i;
	}else
	{

		document.getElementById("Contrast").innerHTML=Contrast_i;

	}
	if(initdata_array[12]!=" "){
	//brightless
		key_value=initdata_array[12].substring(4);
		Brightness_i=Number(key_value);
			//alert(Brightness_i);
		document.getElementById("Brightness").innerHTML=Brightness_i;
	}else{
	
		document.getElementById("Brightness").innerHTML=Brightness_i;
	
	}

	//color
	if(initdata_array[13]!=" "){
		key_value=initdata_array[13].substring(4);
		Color_i=Number(key_value);
		document.getElementById("Color").innerHTML=Color_i;
		//alert(Color_i);
	}else{

		document.getElementById("Color").innerHTML=Color_i;
		

	}
		//sharp
	if(initdata_array[14]!=" "){
		key_value=initdata_array[14].substring(6);
		Sharpness_i=Number(key_value);
		//alert(Sharpness_i);
		document.getElementById("Sharpness").innerHTML=Sharpness_i;
	}else{
		document.getElementById("Sharpness").innerHTML=Sharpness_i;

	}
	

	
}


function prase_initdata()
	
{	
	
	//initdata=">*pow=?# *POW=ON# > >*vol=?# *VOL=10# > >*mute=?# *MUTE=OFF# > >*sour=?# *SOUR=RGB# > >*appmod=?# *APPMOD=DYNAMIC# > >*ct=?# *CT=NORMAL# > >*asp=?# *ASP=16:9# > >*pp=?# *PP=RE# > >*lampm=?# *LAMPM=ECO# > >*3d=?# *3D=AT# > >*rr=?# >*con=?# *CON=0# > >*bri=?# *BRI=50# > >*color=?# Block item > >*sharp=?# *SHARP=5# >";
	var index_start=0;
	var index_end=0;
	var tmp_data;
	var tmpdata;
	var id=0;
	for(id=0;id<15;id++)
	{

		index_start=initdata.indexOf(initdata_key[id]);
		
		if(index_start>0){
			tmp_data=initdata.substring(index_start);
			
			index_end=tmp_data.indexOf("#");
			
			if(index_end>0){
				tmpdata=tmp_data.substring(0,index_end);
				initdata_array[id]=tmpdata.toLowerCase();
				showLog(initdata_array[id]);
				//alert(initdata_array[id]);
			}else{
				initdata_array[id]=" ";

			}


		}

		else
			initdata_array[id]=" ";


	}

}
function showLog(str){
	document.getElementById("log").innerHTML = str;
}

function sendCmd(cmd){
	sendCmdArg(cmd,null);
}

function sendCmdArg(cmd,value){
	var cmdStr = "cmd="+cmd+"&value=";
	
	if(value != null){
		cmdStr = cmdStr+value;
	}else{
		cmdStr = cmdStr+"null";
	}
	showLog(cmdStr);
    $.ajax({
            type: 'GET',
            cache: false,
            url: CGI_URL+cmdStr,
            dataType: 'xml',
            success: function (data) {
				var msg = $(data).find("result").text();
				showLog(msg);
			
           },
           error: function (e) {
				showLog("error");
               }
           });
}
function get_curinitval()
{
	var cmdStr ="cmd=BENQINITVAL&value=1";
	showLog(cmdStr);
    $.ajax({
            type: 'GET',
            cache: false,
            url: CGI_URL+cmdStr,
            dataType: 'xml',
            success: function (data) {
				var msg = $(data).find("result").text();
				initdata=msg;
				showLog(msg);
				//alert(initdata);
			
           },
           error: function (e) {
				showLog("error");
               }
           });
}
function sendCmdArgAsync(cmd,value, noncached){
	var cmdStr = "cmd="+cmd+"&value=";
	var msg;
	
	if(value != null){
		cmdStr = cmdStr+value;
	}else{
		cmdStr = cmdStr+"null";
	}
	console.log("noncached:"+noncached);
	if (noncached != null)
		cmdStr = cmdStr+"&nc=1";
	showLog(cmdStr);
    $.ajax({
            type: 'GET',
			async: false,
            cache: false,
            url: CGI_URL+cmdStr,
            dataType: 'xml',
            success: function (data) {
				msg = $(data).find("result").text();
				showLog(msg);
           },
           error: function (e) {
				showLog("error");
				msg = "error";
               }
           });
	return msg;
}

function GetCurrentSetting(Catelog, command, array, noncached){
	var msg = sendCmdArgAsync(Catelog, command, noncached);
	console.debug("msg:"+msg);
	if (msg != 'error'){
	var key = msg.split(/[=#]/)[1];
	console.debug("key:"+key);
	var index = array[1].indexOf(key);
	
	return array[0][index];
	}
	else{
		return 'error';
	}
}

function GetCurrentValue(Catelog, command, noncached){
	var msg = sendCmdArgAsync(Catelog, command, noncached);
	if (msg != 'error'){
		var value = msg.split(/[=#]/)[1];
		if(connected_flag==0){
			connected_flag=1;
			show_page(connected_flag);
		}
		return value;
	}
	else{
		return 'error';
	}
}

function sourceChange(theSelect){
	var str = theSelect.value;
	showLog(str);
	sendCmdArg("source",str);
}

function pictureModeChange(theSelect){
	var str = theSelect.value;
	showLog(str);
	sendCmdArg("pmode",str);
}

//Source Selection------
var Source_i=0;

var Source_max=Source_Selection.length-1;

function source_change(value){
	switch(value){
		case "left":
		if(Source_i==0)Source_i=Source_max;
		else Source_i--;
		break;
		case "right":
		if(Source_i==Source_max)Source_i=0;
		else Source_i++;
		break;
	}
document.getElementById("source").innerHTML=Source_Selection[Source_i];
sendCmdArg('source',Source_value[Source_i]);
}

//Picture Mode-----
var Picture_i=0;

var Picture_max=Picture_Selection.length-1;
function Picture(value){
	switch(value){
		case "left":
		if(Picture_i==0)Picture_i=Picture_max;
		else Picture_i--;
		break;
		case "right":
		if(Picture_i==Picture_max)Picture_i=0;
		else Picture_i++;
		break;
	}
document.getElementById("Picture").innerHTML=Picture_Selection[Picture_i];
sendCmdArg('pmode',Picture_value[Picture_i]);
}

//Audio source select-----
var Audiosource_i=0;

var Audiosource_max=Audiosource_Selection.length-1;
function Audiosource(value){
	switch(value){
		case "left":
		if(Audiosource_i==0)Audiosource_i=Audiosource_max;
		else Audiosource_i--;
		break;
		case "right":
		if(Audiosource_i==Audiosource_max)Audiosource_i=0;
		else Audiosource_i++;
		break;
	}
document.getElementById("Audiosource").innerHTML=Audiosource_Selection[Audiosource_i];
sendCmdArg('audsrc',Audiosource_value[Audiosource_i]);
}

//colorTempList-----
var color_i=0;

var color_max=color_Selection.length-1;
function color(value){
	switch(value){
		case "left":
		if(color_i==0)color_i=color_max;
		else color_i--;
		break;
		case "right":
		if(color_i==color_max)color_i=0;
		else color_i++;
		break;
	}
document.getElementById("color").innerHTML=color_Selection[color_i];
sendCmdArg('coltemp',color_value[color_i]);
}

//aspectList-----
var aspect_i=0;

var aspect_max=aspect_Selection.length-1;
function aspect(value){
	switch(value){
		case "left":
		if(aspect_i==0)aspect_i=aspect_max;
		else aspect_i--;
		break;
		case "right":
		if(aspect_i==aspect_max)aspect_i=0;
		else aspect_i++;
		break;
	}
document.getElementById("aspect").innerHTML=aspect_Selection[aspect_i];
sendCmdArg('aspect',aspect_value[aspect_i]);
}

//prjPosList-----
var prjpos_i=0;

var prjpos_max=prjpos_Selection.length-1;
function prjpos(value){
	switch(value){
		case "left":
		if(prjpos_i==0)prjpos_i=prjpos_max;
		else prjpos_i--;
		break;
		case "right":
		if(prjpos_i==prjpos_max)prjpos_i=0;
		else prjpos_i++;
		break;
	}
document.getElementById("prjpos").innerHTML=prjpos_Selection[prjpos_i];
sendCmdArg('prjpos',prjpos_value[prjpos_i]);
}

//Lamp Control-----
var lambctl_i=0;

var lambctl_max=lambctl_Selection.length-1;
function lambctl(value){
	switch(value){
		case "left":
		if(lambctl_i==0)lambctl_i=lambctl_max;
		else lambctl_i--;
		break;
		case "right":
		if(lambctl_i==lambctl_max)lambctl_i=0;
		else lambctl_i++;
		break;
	}
document.getElementById("lambctl").innerHTML=lambctl_Selection[lambctl_i];
sendCmdArg('lambctl',lambctl_value[lambctl_i]);
}
//3dList-----
var d3_i=0;

var d3_max=d3_Selection.length-1;
function d3(value){
	switch(value){
		case "left":
		if(d3_i==0)d3_i=d3_max;
		else d3_i--;
		break;
		case "right":
		if(d3_i==d3_max)d3_i=0;
		else d3_i++;
		break;
	}
document.getElementById("d3").innerHTML=d3_Selection[d3_i];
sendCmdArg('3d',d3_value[d3_i]);
}

//remoteRecvList-----
var rrecv_i=0;

var rrecv_max=rrecv_Selection.length-1;
function rrecv(value){
	switch(value){
		case "left":
		if(rrecv_i==0)rrecv_i=rrecv_max;
		else rrecv_i--;
		break;
		case "right":
		if(rrecv_i==rrecv_max)rrecv_i=0;
		else rrecv_i++;
		break;
	}
document.getElementById("rrecv").innerHTML=rrecv_Selection[rrecv_i];
sendCmdArg('rrecv',rrecv_value[rrecv_i]);
}


//Contrast-----
var Contrast_i=50;
var Contrast_max=100;
function Contrast(value){
	switch(value){
		case "left":
		if(Contrast_i>0) Contrast_i--;
		sendCmdArg('picset',"contrastdown");
		break;
		case "right":
		if(Contrast_i<100) Contrast_i++;
		sendCmdArg('picset',"contrastup");
		break;
	}
document.getElementById("Contrast").innerHTML=Contrast_i;
//sendCmdArg('picset',Contrast_i)
}
	
//Brightness-----
var Brightness_i=50;
var Brightness_max=100;
function Brightness(value){
	switch(value){
		case "left":
		if(Brightness_i>0) Brightness_i--;
		 sendCmdArg('picset',"brightdown");
		break;
		case "right":
		if(Brightness_i<Brightness_max) Brightness_i++;
		sendCmdArg('picset',"brightup");
		break;
	}
document.getElementById("Brightness").innerHTML=Brightness_i;
//sendCmdArg('picset',Brightness_i)
}	

//Color-----
var Color_i=50;
var Color_max=100;
function Color(value){
	switch(value){
		case "left":
		if(Color_i>0) Color_i--;
		 sendCmdArg('picset',"coldown");
		break;
		case "right":
		if(Color_i<Color_max) Color_i++;
		 sendCmdArg('picset',"colup");
		break;
	}
document.getElementById("Color").innerHTML=Color_i;
//sendCmdArg('picset',Color_i)
}	

//Sharpness-----
var Sharpness_i=50;
var Sharpness_max=100;
function Sharpness(value){
	switch(value){
		case "left":
		if(Sharpness_i>0) Sharpness_i--;
		sendCmdArg('picset',"sharpdown");
		break;
		case "right":
		if(Sharpness_i<Sharpness_max) Sharpness_i++;
		sendCmdArg('picset',"sharpup");
		break;
	}
document.getElementById("Sharpness").innerHTML=Sharpness_i;
//sendCmdArg('picset',Sharpness_i)
}	
	
//Zoom In-----
var Zoom_i=50;
var Zoom_max=100;
function Zoom(value){
	switch(value){
		case "left":
		if(Zoom_i>0) Zoom_i--;
		sendCmdArg('picset',"zoomin");
		break;
		case "right":
		if(Zoom_i<Zoom_max) Zoom_i++;
		sendCmdArg('picset',"zoomout");
		break;
	}
//document.getElementById("Zoom").innerHTML=(Zoom_i);
//sendCmdArg('picset',Zoom_i)
}

//power on /off-----
var pow_i=1;
function pow11(){
pow_i=~pow_i;
	switch(pow_i){
		case 1:
		document.power.src="img_rs232/power.png";		
		sendCmdArg('power',"shutdown");
		break;
		case -2:   //?
		document.power.src="img_rs232/power-gray.png";
		sendCmdArg('power',"boot");	
		break;
	}
//sendCmdArg('power',pow_i)	
}

//menu on /off-----
var menu_i=1;
function menu(obj){
menu_i=~menu_i;
	switch(menu_i){
		case 1:
		obj.style.color="black";
		sendCmdArg('miscel',"menuoff");
		break;
		case -2:   //?
		obj.style.color="gray";
		sendCmdArg('miscel',"menuon");
		break;
	}
//document.getElementById("vol").innerHTML=menu_i;  //print test
//sendCmdArg('miscel',menu_i);
}
//Blank
var blank_i=1;
function blank(obj){
blank_i=~blank_i;
	switch(blank_i){
		case 1:
		obj.style.color="black";
		sendCmdArg('miscel',"blankoff");
		break;
		case -2:   //?
		obj.style.color="gray";
		sendCmdArg('miscel',"blankon");
		break;
	}
//sendCmdArg('miscel',blank_i);
}

//Freeze
var freeze_i=1;
function freeze(obj){
freeze_i=~freeze_i;
	switch(freeze_i){
		case 1:
		obj.style.color="black";
		sendCmdArg('miscel',"freezeoff");
		break;
		case -2:   //?
		obj.style.color="gray";
		sendCmdArg('miscel',"freezeon");
		break;
	}
//sendCmdArg('miscel',freeze_i);
}
//Volume-----
//var vol_i=50;
var vol_i=10;
var vol_max=10;
function vol(value){
	switch(value){
		case "left":
		//if(vol_i==0) vol_i=vol_max;
		//else vol_i--;
		if(vol_i>0) vol_i--;
		sendCmdArgAsync('audctr',"voldown", null);
		//sendCmdArg('audctr',"voldown");
		break;
		case "right":
		//if(vol_i==vol_max)vol_i=0;
		//else vol_i++;
		//alert("rithg");
		if(vol_i<100) vol_i++;
		sendCmdArgAsync('audctr',"volup",null);
		//sendCmdArg('audctr',"volup");
		
		break;
	}
document.mute.src="img_rs232/Volume.png";
document.getElementById("vol").innerHTML=vol_i;
//sendCmdArg('audctr',vol_i)
}	
//mute on /off-----
var mute_i=1;
function mute11(){
mute_i=~mute_i;
	switch(mute_i){
		case 1:
		document.mute.src="img_rs232/Volume.png";
		document.getElementById("vol").innerHTML=vol_i;
		sendCmdArg('audctr',"muteoff");
		break;
		case -2:   //?
		document.mute.src="img_rs232/Volume-color-gray.png";
		document.getElementById("vol").innerHTML="X";
		sendCmdArg('audctr',"muteon");
		break;
	}
//sendCmdArg('audctr',mute_i)
}

//change img----
function mouseOver(value)
{
	switch(value){
		case "up":
		document.up.src="img_rs232/up-color2-gray.png";
		break;
		case "down":
		document.down.src="img_rs232/down-color2-copy.png";
		break;
		case "left":
		document.left.src="img_rs232/left-gray.png";
		break;
		case "right":
		document.right.src="img_rs232/right-color2-gray.png";
		break;
	}
}
function mouseOut(value)
{
	switch(value){
		case "up":
		document.up.src="img_rs232/up.png";
		break;
		case "down":
		document.down.src="img_rs232/down.png";
		break;
		case "left":
		document.left.src="img_rs232/left.png";
		break;
		case "right":
		document.right.src="img_rs232/right.png";
		break;
	}
}

