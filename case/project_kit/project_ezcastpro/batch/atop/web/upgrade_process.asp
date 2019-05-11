<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN">
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=<%fwupdate_items("charset");%>">
<style type="text/css">
<!--
* {margin:0; padding:0; font-size:12px; color:#FFF; font-family: Arial,sans-serif;}
.non_ln {color:#000; font-weight: bold;}
.cont {padding-left:5px;}
-->
</style>
<script language="javascript">
	var upg_resp=0;
	function finish(){
		parent.document.getElementById("logoutbtn").click();
	}
	function process(percentage){
    var o=document.getElementById("process");
    var p=document.getElementById("p");
    if(percentage<100){
      //var newWidth=parseInt(o.style.width)+5;
      o.style.width=percentage+"%";
      p.innerHTML=percentage+"%";
      percentage++;
      setTimeout("process("+percentage+")",1900);  
    }else if(percentage == 100){
    	o.style.width=percentage+"%";
      p.innerHTML=percentage+"%";
    	document.getElementById("proc_msg").innerHTML = '<%upgrade_items("UPGRADE_MSG6");%>';
    	setTimeout(finish,3000);
  	}        
	}
	function createREQ() {
	//--- Define XMLHttpRequest object
		if(window.XMLHttpRequest){
			upg_process_req = new XMLHttpRequest(); //--FF/Safari
		}else if(window.ActiveXObject){
			try{
				upg_process_req = new ActiveXObject("Msxml2.XMLHTTP"); //--IE
			}catch(e){
				try{
					upg_process_req = new ActiveXObject("Microsoft.XMLHTTP");
				}catch(e){}
			}
		}
	}
	function set_upgrade_start(){
		createREQ();
		upg_upgrade_req.open("POST", "/form/upgrade_download_cgi", true);
		upg_upgrade_req.onreadystatechange = function() {
			if(upg_upgrade_req.readyState == 4 && upg_upgrade_req.status == 200){
				var RSPUpgText = upg_upgrade_req.responseText;
				//alert(RSPConsoleText);
				var RSPUpg=eval('('+RSPUpgText.slice(6)+')');
				if(RSPUpg.upgrade == 1) upg_resp = 1; //-- Get start upgrade response
			}
		}
		upg_download_req.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
		var str = "set_upg_start=set_upg_start";
		upg_download_req.send(str);
	}
	function window_onload(){
	  process(0);
	  set_upgrade_start();
	}
	window.onload=window_onload;
</script>
</head>
<body bgcolor="#494949">
	<table width="560" border="0" cellpadding="0" cellspacing="0">
		<tr><td width="40" height="15"></td><td width="25"></td>
				<td width="390"></td><td width="105"></td>
		</tr>
		<tr><td width="40" height="25" align="center"><img src="image/arrayb.gif"></td>
				<td class="non_ln" colspan="3" height="25"><%fwupdate_items("LINK_GENERAL");%></a></td>
		</tr>
		<tr><td width="40" height="25" align="center"><img src="image/arrayb.gif"></td>
				<td class="non_ln" colspan="3" height="25"><%fwupdate_items("LINK_NETWORK");%></a></td>
		</tr>
		<tr><td width="40" height="25" align="center"><img src="image/arrayb.gif"></td>
				<td class="non_ln" colspan="3" height="25"><%fwupdate_items("LINK_SECURITY");%></a></td>
		</tr>
		<tr><td width="40" height="25" align="center"><img src="image/arrayb.gif"></td>
				<td class="non_ln" colspan="3" height="25"><%fwupdate_items("LINK_PASSWORD");%></a></td>
		</tr>
		<tr><td width="40" height="30" align="center"><img src="image/array.gif"></td>
				<td class="non_ln" colspan="3" height="24"><%fwupdate_items("LINK_FWUPDATE");%></td>
		</tr>
<!--Main Content-->
		<tr><td width="40" height="25"></td>
			<td id="proc_msg" colspan="3" class="cont"><%upgrade_items("UPGRADE_MSG3");%></td>
		</tr>
		<tr><td width="40" height="25"></td>
			<td colspan="3" class="cont"></td>
		</tr>
<!-- for OTA-->
		<tr id="msg"><td width="40" height="10"></td>
			<td class="cont" colspan="3" height="10"><%upgrade_items("UPGRADE_MSG5");%></td>
		</tr>
		<tr id="percentage"><td width="40" height="24"></td>
			<td height="24" colspan="2" class="cont">
				<div id="processbar" style="border:2px solid #000000; width:300px; background-color:#ffffff; position:relative">
					<div id="process" style="background-color:#000000; width:0px; height:22px;"></div>
					<span id="p" style="position:absolute; top:5px;left:120%;">0%</span>
				</div>
			</td>
		</tr>
<!---->
		<tr><td colspan="4" height="40"></td></tr>
		<tr><td colspan="3" height="50" align="right" valign="top">
				<img id="apply" src="image/btn_cc1.bmp">
			</td>
			<td width="105"></td>
		</tr>
<!--Main Content-->
	</table>
</body>
</html>