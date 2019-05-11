<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1 Transitional//EN">
<html>
<head>
<meta http-equiv="Pragma" Content="no-cache">
<meta http-equiv="Content-Type" content="text/html; charset=<%console_items("charset");%>" />
<!--<meta http-equiv="cache-control" content="no-cache">
<meta http-equiv="Pragma" Content="no-cache">
<meta http-equiv="Expires" Content="0">-->
<title></title>
<style type="text/css">
<!--
* {margin:0; padding:0; color:#FFF; font-family: Arial,sans-serif; overflow-x:hidden;}
td {overflow-y:hidden;}
A {color:#FFF; text-decoration: none; font-weight: bold;}
A:hover {color:#FFF; text-decoration: underline;}
#lnk td {padding-right:5px;}
.copy_right {padding-left:10px; font-size:12px; color:#494949; font-weight: bold;}
.subtitle {color:#000; font-weight: bold; font-size:13px;}
.btn {color:#000; width:100px; height:20px; line-height:16px;}
select {color:#000;}
option {color:#000;}
.plus_minus {color:#000; width:18px; height:25px;}
input[type="text"] {color:#000;}
#adv {color:#000000; font-weight: bold; font-family: Arial, sans-serif;}
#adv:hover {border-bottom:1px solid #fff;}
-->
</style>
<script language="javascript">
	var console_req;
	var colortemp_str=["CT1","CT2","CT3"];
	var LangEn  = {    	
    projecting			:	"Projecting",
    waitprojecting	:	"Waiting for Projecting"
	}
	var LangGe  = {    	
	  projecting    	: "projezieren",
	  waitprojecting	: "Warte auf Projektion"  
	}
	var LangFr  = {    	
	  projecting      : "Projection en cours",
	  waitprojecting	: "En attente de la projection"
	}
	var LangIt  = {    	
	  projecting      : "Projecting",
	  waitprojecting	: "Waiting for Projecting"  
	}
	var LangSp  = {    	
	  projecting      : "Proyectando",
	  waitprojecting	: "Esperando para proyectar"  
	}
	var LangPor  = {    	
	  projecting      : "Projecting",
	  waitprojecting	: "Waiting for Projecting"  
	}
	var LangJP  = {
	  projecting      : "投影中",
	  waitprojecting	: "投影待ち中"  
	}                              	
	var LangTC  = {        	
	  projecting      : "投影中 ",
	  waitprojecting	: "待機中 "   
	}
	var LangSC  = {
	  projecting      : "投影中 ",
	  waitprojecting	: "待机中 "  
	}
	var LangRu  = {    	
	  projecting      : "Projecting",
	  waitprojecting	: "Waiting for Projecting"   
	}
	function showIMG(obj,tp){
		if(tp)
			obj.style.background = "url(image/menu_f.png) no-repeat right";
		else
			obj.style.background = "";
	}
	function show_hidden(obj){
		if(document.getElementById('nouse').value=="0"){
			document.getElementById('hidden_block').style.display="none";
			window.scrollTo(0,0);
			document.getElementById('nouse').value = "1";
		}else {
			document.getElementById('hidden_block').style.display="";
			window.scrollTo(0,document.body.scrollHeight);
			document.getElementById('nouse').value = "0";
		}
	}
	function chg_projecting(Lang,md) {
		var lang_str;
		//alert(Lang+";"+md);
		switch(Lang){
			case 0: lang_str = LangEn; break;
			case 1: lang_str = LangTC; break;
			case 2: lang_str = LangSC; break;
			case 3: lang_str = LangJP; break;
			case 4: lang_str = LangGe; break;
			case 5: lang_str = LangFr; break;
			case 6: lang_str = LangIt; break;
			case 7: lang_str = LangSp; break;
			case 8: lang_str = LangPor; break;
			case 9: lang_str = LangRu; break;
			default: lang_str = LangEn; break;
		}
		if(md != 0 && md != 1) md = 0;
		if(md == 0) return lang_str['waitprojecting'];
		else	return lang_str['projecting'];
	}
	function change_display(Lang,md) {
		if(md != 3) return colortemp_str[md];
		switch(Lang){
			case 0: return "User"; break;
			case 1: return "自定"; break;
			case 2: return "自定义"; break;
			case 3: return "ユーザー"; break;
			case 4: return "Benutzer"; break;
			case 5: return "utilis"; break;
			case 6: return "utente"; break;
			case 7: return "usuario"; break;
			case 8: return "utilizador"; break;
			case 9: return "Польз."; break;
			default: return "User"; break;
		}
	}
	function createREQ() {
	//--- Define XMLHttpRequest object
		if(window.XMLHttpRequest){
			console_req = new XMLHttpRequest(); //--FF/Safari
		}else if(window.ActiveXObject){
			try{
				console_req = new ActiveXObject("Msxml2.XMLHTTP"); //--IE
			}catch(e){
				try{
					console_req = new ActiveXObject("Microsoft.XMLHTTP");
				}catch(e){}
			}
		}
	}

	function userchange(obj,mode,spec){
		var allInput = document.getElementsByTagName('input');
		createREQ();
		console_req.open("POST", "/form/console_cgi", true);
		//console_req.open("POST", "http://10.0.51.9", true);
		console_req.onreadystatechange = function() {
			if(console_req.readyState == 4 && console_req.status == 200){
				clearTimeout(xmlHttpTimeout); 
				var RSPConsoleText = console_req.responseText;
				//alert(RSPConsoleText);
				//var RSPConsole=eval("("+RSPConsoleText+")");
				var RSPConsole=eval('('+RSPConsoleText.slice(6)+')');
				if(RSPConsole.power == '1')
					document.getElementById('pwron').checked = true;
				else
					document.getElementById('pwroff').checked = true;
				if(RSPConsole.eco == '1')
					document.getElementById('eco_on').checked = true;
				else
					document.getElementById('eco_off').checked = true;
				switch(RSPConsole.display){
					case '0': document.getElementById('displayMD').options[0].selected = true; break;
					case '1': document.getElementById('displayMD').options[1].selected = true; break;
					case '2': document.getElementById('displayMD').options[2].selected = true; break;
					case '3': document.getElementById('displayMD').options[3].selected = true; break;
					case '4': document.getElementById('displayMD').options[4].selected = true; break;
				}
				switch(RSPConsole.asp){
					case '0': document.getElementById('asp_auto').checked = true; break;
					case '1': document.getElementById('asp_full').checked = true; break;
					case '2': document.getElementById('asp_43').checked = true; break;
					case '3': document.getElementById('asp_169').checked = true; break;
					case '4': document.getElementById('asp_lbox').checked = true; break;
				}
				switch(RSPConsole.pjtion){
					case '0': document.getElementById('pjtion_FD').checked = true; break;
					case '2': document.getElementById('pjtion_RD').checked = true; break;
					case '3': document.getElementById('pjtion_RC').checked = true; break;
					case '1': document.getElementById('pjtion_FC').checked = true; break;
				}
				for(var i=0;i<document.getElementById('src_sel').options.length;i++)
					if(document.getElementById('src_sel').options[i].value == RSPConsole.input){
						document.getElementById('src_sel').options[i].selected = true;
						break;
					}
				if(RSPConsole.vgaout == '1')
					document.getElementById('vgaout_on').checked = true;
				else
					document.getElementById('vgaout_off').checked = true;
				switch(RSPConsole.autodown){
					case '0': document.getElementById('autodown_off').checked = true; break;
					case '1': document.getElementById('autodown_15').checked = true; break;
					case '2': document.getElementById('autodown_30').checked = true; break;
					case '3': document.getElementById('autodown_60').checked = true; break;
					case '4': document.getElementById('autodown_120').checked = true; break;
				}
				if(RSPConsole.mute == '1')
					document.getElementById('mute_on').checked = true;
				else
					document.getElementById('mute_off').checked = true;
				//document.getElementById('colortemp').value = RSPConsole.colortemp;
				document.getElementById('colortemp').value = change_display(parseInt(RSPConsole.Lang), parseInt(RSPConsole.colortemp));
				document.getElementById('gamma').value = RSPConsole.gamma;
				document.getElementById('bri').value = RSPConsole.bright;
				document.getElementById('contrast').value = RSPConsole.contrast;
				document.getElementById('vol').value = RSPConsole.vol;
				if(RSPConsole.keystone < 41)
					document.getElementById('keystone').value = RSPConsole.keystone;
				else
					document.getElementById('keystone').value = (RSPConsole.keystone - 256);
				document.getElementById('status').innerHTML = chg_projecting(parseInt(RSPConsole.Lang), parseInt(RSPConsole.status));
				document.getElementById('total').innerHTML = RSPConsole.total;
				//--End of Data
				for(var i = 0; i < allInput.length; i++)
					if(allInput[i].type != "text")
						allInput[i].disabled = false;
				document.getElementById('displayMD').disabled = false;
				document.getElementById('src_sel').disabled = false;
			}
		}
		console_req.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
		//--0:radio & select, 1:button--//
		if(mode == 1) var str = obj.id+"="+obj.id;
		else var str = obj.name+"="+obj.value;
		//alert(str);
		console_req.send(str);
		
		for(var i = 0; i < allInput.length; i++)
			allInput[i].disabled = true;
		document.getElementById('displayMD').disabled = true;
		document.getElementById('src_sel').disabled = true;
		
		var xmlHttpTimeout=setTimeout(ajaxTimeout,5000);
		function ajaxTimeout(){
   		console_req.abort();
   		for(var i = 0; i < allInput.length; i++)
					if(allInput[i].type != "text")
						allInput[i].disabled = false;
			document.getElementById('displayMD').disabled = false;
			document.getElementById('src_sel').disabled = false;
   		//alert("Request timed out");
		}
	}
	
	function dynSRC() {
		//alert("dynSRC");
		document.getElementById('src_sel').options.length = 0;
		//alert(document.getElementById('SER_LIST').contentWindow.document.getElementById('src_hidden').options.length);
		for(var i=0;i<document.getElementById('SER_LIST').contentWindow.document.getElementById('src_hidden').options.length;i++) {
			if(document.getElementById('SER_LIST').contentWindow.document.getElementById('src_hidden').options[i].text != "") {
				document.getElementById('src_sel').options[i]=new Option(document.getElementById('SER_LIST').contentWindow.document.getElementById('src_hidden').options[i].text, document.getElementById('SER_LIST').contentWindow.document.getElementById('src_hidden').options[i].value);
			}
			if(document.getElementById('SER_LIST').contentWindow.document.getElementById('src_hidden').options[i].selected)
					document.getElementById('src_sel').options[i].selected = true;
		}
	}
</script>
</head>

<body>
	<table bgcolor="#000" width="760" border="0" cellpadding="0" cellspacing="0" style="margin-left:auto; margin-right:auto;">
		<form name="logoutform" method="post" action="/form/logout_cgi">
			<input id="logoutbtn" name="logout" value="log_out" type="submit" style="display:none">
		</form>
		<tr><td colspan="2" width="760" height="70" background="image/TopLogo.gif"></td>
			<!--<td width="760" colspan="2" height="70"><iframe src="title.htm" scrolling="No" frameborder="no" width="760" height="70"></iframe></td>-->
		</tr>
		<tr><td colspan="2" height="25" style="font-size:12px;"><a href="javascript:document.getElementById('logoutbtn').click();">ACER</a><b> &gt;</b> 
			<a href="login.asp"><%console_items("LINK_ADVANCE");%></a><b> &gt;</b>
			<a href="console.asp"><%console_items("LINK_CONSOLE");%></a>
			</td></tr>
			
		<tr><td bgcolor="#000" width="200" height="200" valign="top" style="font-size:12px;">
				<!--<iframe src="link_adm.htm" scrolling="No" frameborder="no" width="200"></iframe>-->
			<table id="lnk" width="200" border="0" cellpadding="0" cellspacing="0">
				<tr><td width="200" height="15"></td></tr>
				<tr><td height="24" align="right" onmouseover="showIMG(this,1)" onmouseout="showIMG(this,0)">
					<a href="console.asp"><%console_items("LINK_CONSOLE");%></a></td></tr>
				<tr><td height="24" align="right" onmouseover="showIMG(this,1)" onmouseout="showIMG(this,0)">
					<a href="config.asp"><%console_items("LINK_CONFIGURE");%></a></td></tr>
				<tr><td height="24" align="right" onmouseover="showIMG(this,1)" onmouseout="showIMG(this,0)">
					<a href="setting.asp"><%console_items("LINK_MAIL");%></a></td></tr>
				<tr><td height="24" align="right" onmouseover="showIMG(this,1)" onmouseout="showIMG(this,0)">
					</td></tr>
				<tr><td height="24" align="right" onmouseover="showIMG(this,1)" onmouseout="showIMG(this,0)">
					<a href="javascript:document.getElementById('logoutbtn').click();"><%console_items("LINK_LOGOUT");%></a></td></tr>
			</table>
			</td>
			
			<!--<td width="5"></td>-->
			<td bgcolor="#494949" valign="top" style="font-size:12px;">
				<table width="560" border="0" cellpadding="0" cellspacing="0">
					<tr><td colspan="3" height="15"></td></tr>
					<tr><td width="40" height="25"></td>
							<td colspan="2" class="subtitle"><%console_items("CONSOLE_MSG1");%></td>
					</tr>
					<tr><td width="40" height="30"></td>
							<td width="190"><%console_items("CONSOLE_MSG2");%></td>
							<td width="325" id="status"><%console_value("PROJECTING_STATE");%></td>
					</tr>
					<tr><td width="40" height="30"></td>
							<td width="190"><%console_items("CONSOLE_MSG3");%></td>
							<td width="325" id="total"><%console_value("TOTAL_USER");%></td>
					</tr>
					<tr><td width="40" height="30"></td>
							<td colspan="2" class="subtitle"><%console_items("CONSOLE_MSG4");%></td>
					</tr>
					<tr><td width="40" height="30"></td>
							<td width="190"><%console_items("CONSOLE_MSG5");%></td>
							<td width="325">
								<input type="radio" id="pwron" name="power" value="1" onclick="userchange(this,0,0)" <%console_value("poweron");%>> <%console_items("ITEM_ON");%>
              	<input type="radio" id="pwroff" name="power" value="0" onclick="userchange(this,0,0)" <%console_value("poweroff");%>> <%console_items("ITEM_OFF");%>
							</td>
					</tr>
					<tr><td width="40" height="30"></td>
							<td width="190"><%console_items("CONSOLE_MSG6");%></td>
							<td width="325">
								<input type="radio" id="eco_on" name="eco" value="1" onclick="userchange(this,0,0)" <%console_value("ecoon");%>> <%console_items("ITEM_ON");%>
              	<input type="radio" id="eco_off" name="eco" value="0" onclick="userchange(this,0,0)" <%console_value("ecooff");%>> <%console_items("ITEM_OFF");%> 
							</td>
					</tr>
					<tr style="display:none"><td width="40" height="30"></td>
							<td width="190"><%console_items("CONSOLE_MSG7");%></td>
							<td width="325">
								<input type="radio" id="srclock_on" name="srclock" value="1" onclick="userchange(this,0,0)"> <%console_items("ITEM_ON");%>
              	<input type="radio" id="srclock_off" name="srclock" value="0" onclick="userchange(this,0,0)"> <%console_items("ITEM_OFF");%> 
							</td>
					</tr>
					<tr><td width="40" height="30"></td>
							<td colspan="2">
								<input type="button" class="btn" id="freeze_on" value="<%console_items("CONSOLE_MSG8");%>" onclick="userchange(this,1,0)">&nbsp;&nbsp;
								<input type="button" class="btn" id="freeze_off" value="<%console_items("CONSOLE_MSG9");%>" onclick="userchange(this,1,0)"> 
							</td>
					</tr>
					<tr><td width="40" height="30"></td>
							<td colspan="2">
								<input type="button" class="btn" id="hide_on" value="<%console_items("CONSOLE_MSG10");%>" onclick="userchange(this,1,0)">&nbsp;&nbsp;
								<input type="button" class="btn" id="hide_off" value="<%console_items("CONSOLE_MSG11");%>" onclick="userchange(this,1,0)">
							</td>
					</tr>
					<tr><td width="40" height="30"></td>
							<td colspan="2">
								<input type="button" class="btn" id="resync" value="<%console_items("CONSOLE_MSG12");%>" onclick="userchange(this,1,0)">&nbsp;&nbsp;
								<input type="button" class="btn" id="source" value="<%console_items("CONSOLE_MSG13");%>" onclick="userchange(this,1,0)">&nbsp;&nbsp;
								<input type="button" class="btn" id="reset" value="<%console_items("CONSOLE_MSG14");%>" onclick="userchange(this,1,0)">
							</td>
					</tr>
					<tr><td width="40" height="30"></td>
							<td colspan="2" class="subtitle"><%console_items("CONSOLE_MSG15");%></td>
					</tr>
					<tr><td width="40" height="30"></td>
							<td width="190"><%console_items("CONSOLE_MSG16");%></td>
							<td width="325">
								<select id="displayMD" name="display" onChange="userchange(this,0,0)">
                <option value="0" <%console_value("display1");%>><%console_items("CONSOLE_MSG17");%></option>
                <option value="1" <%console_value("display2");%>><%console_items("CONSOLE_MSG18");%></option>
                <option value="2" <%console_value("display3");%>><%console_items("CONSOLE_MSG19");%></option>
                <option value="3" <%console_value("display4");%>><%console_items("CONSOLE_MSG20");%></option>
                <option value="4" <%console_value("display5");%>><%console_items("CONSOLE_MSG21");%></option>
               </select>
							</td>
					</tr>
					<tr><td width="40" height="30"></td>
							<td width="190"><%console_items("CONSOLE_MSG23");%></td>
							<td width="325">
								<input type="button" class="plus_minus" id="bri_m" value="-" onclick="userchange(this,1,0)">
								<input type="text" id="bri" size="3" value="<%console_value("brightness");%>" disabled >
								<input type="button" class="plus_minus" id="bri_p" value="+" onclick="userchange(this,1,0)">
							</td>
					</tr>
					<tr><td width="40" height="30"></td>
							<td width="190"><%console_items("CONSOLE_MSG24");%></td>
							<td width="325">
								<input type="button" class="plus_minus" id="contrast_m" value="-" onclick="userchange(this,1,0)">
								<input type="text" id="contrast" size="3" value="<%console_value("contrast");%>" disabled >
								<input type="button" class="plus_minus" id="contrast_p" value="+" onclick="userchange(this,1,0)">
							</td>
					</tr>
					<tr><td width="40" height="30"></td>
							<td width="190"><%console_items("CONSOLE_MSG25");%></td>
							<td width="325">
								<label><input type="radio" id="asp_auto" name="asp" value="1" onclick="userchange(this,0,0)" <%console_value("asp1");%>> <%console_items("CONSOLE_MSG26");%>&nbsp;</label>
              	<label style="<%console_value("model");%>"><input type="radio" id="asp_full" name="asp" value="2" onclick="userchange(this,0,0)" <%console_value("asp2");%>> <%console_items("CONSOLE_MSG27");%>&nbsp;</label>
              	<label><input type="radio" id="asp_43" name="asp" value="3" onclick="userchange(this,0,0)" <%console_value("asp3");%>> 4:3&nbsp;</label>
              	<label><input type="radio" id="asp_169" name="asp" value="4" onclick="userchange(this,0,0)" <%console_value("asp4");%>> 16:9&nbsp;</label>
              	<label style="<%console_value("model");%>"><input type="radio" id="asp_lbox" name="asp" value="5" onclick="userchange(this,0,0)" <%console_value("asp5");%>> <%console_items("CONSOLE_MSG28");%></label>
							</td>
					</tr>
					<tr><td width="40" height="30"></td>
							<td width="190" rowspan="2"><%console_items("CONSOLE_MSG29");%></td>
							<td width="325">
								<table width="325" border="0" cellpadding="0" cellspacing="0">
								<tr><td width="150">
									<input type="radio" id="pjtion_FD" name="pjtion" value="1" onclick="userchange(this,0,0)" <%console_value("pjtion1");%>> <%console_items("CONSOLE_MSG30");%>
              	</td>
              	<td width="175">
              		<input type="radio" id="pjtion_FC" name="pjtion" value="2" onclick="userchange(this,0,0)" <%console_value("pjtion2");%>> <%console_items("CONSOLE_MSG31");%>
								</td></tr>
								</table>
							</td>
					</tr>
					<tr><td width="40" height="30"></td>
							<td width="325">
								<table width="325" border="0" cellpadding="0" cellspacing="0">
								<tr><td width="150">
              		<input type="radio" id="pjtion_RD" name="pjtion" value="3" onclick="userchange(this,0,0)" <%console_value("pjtion3");%>> <%console_items("CONSOLE_MSG32");%>
              	</td>
              	<td width="175">
              		<input type="radio" id="pjtion_RC" name="pjtion" value="4" onclick="userchange(this,0,0)" <%console_value("pjtion4");%>> <%console_items("CONSOLE_MSG33");%>
								</td></tr>
								</table>
							</td>
					</tr>
					<tr><td width="40" height="30"></td>
							<td width="190"><%console_items("CONSOLE_MSG13");%></td>
							<td width="325">
								<select id="src_sel" name="src" onChange="userchange(this,0,0)">           
              	</select>            		
							</td>
					</tr>
					<tr><td width="40" height="30" align="center"><img src="image/arrayb.gif"></td>
							<td colspan="2" height="31"><span id="adv" onmouseover="this.style.cursor='pointer'" onclick="show_hidden(this)"><%console_items("LINK_ADVANCE");%></span></td>
					</tr>
					<tr><td width="40" height="1" colspan="3">
						<input type="hidden" id="nouse" value="1">
					</td></tr>
<!--hidden until user click-->
					<tbody id="hidden_block" style="display:none">
						<tr><td width="40" height="30"></td>
							<td colspan="2" class="subtitle"><%console_items("CONSOLE_MSG34");%></td>
					</tr>
					<tr><td width="40" height="30"></td>
							<td width="190"><%console_items("CONSOLE_MSG35");%></td>
							<td width="325">							
								<input type="button" class="plus_minus" id="colortemp_m" value="-" onclick="userchange(this,1,1)">
								<input type="text" id="colortemp" size="3" value="<%console_value("colortemp");%>" disabled >
								<input type="button" class="plus_minus" id="colortemp_p" value="+" onclick="userchange(this,1,1)">
							</td>
					</tr>
					<tr><td width="40" height="30"></td>
							<td width="190"><%console_items("CONSOLE_MSG36");%></td>
							<td width="325">
								<input type="button" class="plus_minus" id="gamma_m" value="-" onclick="userchange(this,1,0)">
								<input type="text" id="gamma" size="3" value="<%console_value("gamma");%>" disabled >
								<input type="button" class="plus_minus" id="gamma_p" value="+" onclick="userchange(this,1,0)">
							</td>
					</tr>
					<tr><td width="40" height="30"></td>
							<td colspan="2" class="subtitle"><%console_items("CONSOLE_MSG37");%></td>
					</tr>
					<tr><td width="40" height="30"></td>
							<td width="190"><%console_items("CONSOLE_MSG43");%></td>
							<td width="325">
								<input type="button" class="plus_minus" id="keystone_m" value="-" onclick="userchange(this,1,0)">
								<input type="text" id="keystone" size="3" value="<%console_value("keystone");%>" disabled >
								<input type="button" class="plus_minus" id="keystone_p" value="+" onclick="userchange(this,1,0)">
							</td>
					</tr>
					<tr><td width="40" height="30"></td>
							<td colspan="2" class="subtitle"><%console_items("CONSOLE_MSG38");%></td>
					</tr>
					<tr><td width="40" height="30"></td>
							<td width="190"><%console_items("CONSOLE_MSG39");%></td>
							<td width="325">
								<input type="radio" id="vgaout_on" name="vgaout" value="1" onclick="userchange(this,0,0)" <%console_value("vgaouton");%>> <%console_items("ITEM_ON");%>
              	<input type="radio" id="vgaout_off" name="vgaout" value="0" onclick="userchange(this,0,0)" <%console_value("vgaoutoff");%>> <%console_items("ITEM_OFF");%> 
							</td>
					</tr>
					<tr><td width="40" height="30"></td>
							<td width="190"><%console_items("CONSOLE_MSG40");%></td>
							<td width="325">
								<input type="radio" id="autodown_off" name="autodown" value="1" onclick="userchange(this,0,0)" <%console_value("autodown1");%>> <%console_items("ITEM_OFF");%>
              	<input type="radio" id="autodown_15" name="autodown" value="2" onclick="userchange(this,0,0)" <%console_value("autodown2");%>> 15 
              	<input type="radio" id="autodown_30" name="autodown" value="3" onclick="userchange(this,0,0)" <%console_value("autodown3");%>> 30
              	<input type="radio" id="autodown_60" name="autodown" value="4" onclick="userchange(this,0,0)" <%console_value("autodown4");%>> 60 
              	<input type="radio" id="autodown_120" name="autodown" value="5" onclick="userchange(this,0,0)" <%console_value("autodown5");%>> 120 
							</td>
					</tr>
					<tr><td width="40" height="30"></td>
							<td width="190"><%console_items("CONSOLE_MSG41");%></td>
							<td width="325">
								<input type="button" class="plus_minus" id="vol_m" value="-" onclick="userchange(this,1,0)">
								<input type="text" id="vol" size="3" value="<%console_value("volume");%>" disabled >
								<input type="button" class="plus_minus" id="vol_p" value="+" onclick="userchange(this,1,0)">
							</td>
					</tr>
					<tr><td width="40" height="30"></td>
							<td width="190"><%console_items("CONSOLE_MSG42");%></td>
							<td width="325">
								<input type="radio" id="mute_on" name="mute" value="1" onclick="userchange(this,0,0)" <%console_value("mute_on");%>> <%console_items("ITEM_ON");%>
              	<input type="radio" id="mute_off" name="mute" value="0" onclick="userchange(this,0,0)" <%console_value("mute_off");%>> <%console_items("ITEM_OFF");%> 
							</td>
					</tr>
					<tr><td colspan="3" width="40" height="15">
						<iframe id="SER_LIST" src="source.asp" style="display:none"></iframe>
						</td></tr>
					</tbody>
<!--hidden until user click-->
				</table>
			</td>
		</tr>
		
		<tr><td class="copy_right" colspan="3" height="55" bgcolor="#000" valign="bottom">
			Copyright &copy; 2013 Acer Inc. All Rights Reserved 
			</td></tr>
		<tr><td colspan="3" height="10" bgcolor="#000"></td></tr>
	</table>
</body>
<HEAD>
<META HTTP-EQUIV="PRAGMA" CONTENT="NO-CACHE">
</HEAD>
</html>