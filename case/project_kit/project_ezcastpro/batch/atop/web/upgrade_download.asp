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
	var resp=0;
	var count=0;
	function chgPIC(obj,md) {
			switch(md){
				case 1: obj.src="image/btn_cc2.bmp"; break;
				case 2: 
					obj.src="image/btn_cc3.bmp"; 
					document.getElementById('upgdownload').click();
					break;
				default: obj.src="image/btn_cc1.bmp"; break;
			}
	}
	function finish_submit(){
		document.getElementById('upgradeapply').value = "finish";
		document.getElementById('upgradeapply').click();
	}
	function process(percentage){
    var o=document.getElementById("process");
    var p=document.getElementById("p");
    if(percentage<=100){
        //percentage++;
        //var newWidth=parseInt(o.style.width)+5;
        o.style.width=percentage+"%";
        p.innerHTML=percentage+"%";
        //setTimeout("process("+percentage+")",500); 
        if(percentage == 100) setTimeout(finish_submit,3000);
    }   	       
	}
	function createREQ() {
	//--- Define XMLHttpRequest object
		if(window.XMLHttpRequest){
			upg_download_req = new XMLHttpRequest(); //--FF/Safari
		}else if(window.ActiveXObject){
			try{
				upg_download_req = new ActiveXObject("Msxml2.XMLHTTP"); //--IE
			}catch(e){
				try{
					upg_download_req = new ActiveXObject("Microsoft.XMLHTTP");
				}catch(e){}
			}
		}
	}
	function set_download_start(){
		createREQ();
		upg_download_req.open("POST", "/form/upgrade_download_cgi", true);
		upg_download_req.onreadystatechange = function() {
			if(upg_download_req.readyState == 4 && upg_download_req.status == 200){
				var RSPUpDownloadText = upg_download_req.responseText;
				//alert("111"+RSPUpDownloadText);
				var RSPUpgdownload=eval('('+RSPUpDownloadText.slice(6)+')');
				if(RSPUpgdownload.download == 1) resp = 1; //-- Get start download response
			}
		}
		upg_download_req.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
		var str = "start_download=start_download";
		//alert(str);
		upg_download_req.send(str);
	}
	function get_download_process(){
		createREQ();
		upg_download_req.open("POST", "/form/upgrade_download_cgi", true);
		upg_download_req.onreadystatechange = function() {
			if(upg_download_req.readyState == 4 && upg_download_req.status == 200){
				var RSPUpDownloadText = upg_download_req.responseText;
				//alert("222"+RSPUpDownloadText);
				var RSPUpgdownload=eval('('+RSPUpDownloadText.slice(6)+')');
				if(RSPUpgdownload.download_status == -1 || RSPUpgdownload.download_status == 0) //-- network status is normal
					process(RSPUpgdownload.process);
				/*else if(RSPUpgdownload.download_status == 0) {//-- download finish, but must check the percentage is 100%
					process(RSPUpgdownload.process);
					if(RSPUpgdownload.process == 100) {
						//document.getElementById('upgradeapply').value = "finish";
						//document.getElementById('upgradeapply').click();
						setTimeout(finish_submit,3000);
					}
				}*/else
					document.getElementById('upgradeapply').click();
			}
		}
		upg_download_req.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
		var str = "getprocess=getprocess";
		//alert(str);
		upg_download_req.send(str);
	}
	function window_onload(){
	  process(0);
	  set_download_start();
	  setTimeout(upg_process,3000);
	}
	function upg_process(){
		if(resp == 1){
			get_download_process();
		}else{
			if(count > 3){
				document.getElementById('upgradeapply').click();
			}else {
				count++;
				set_download_start();
			}
		}
		setTimeout(upg_process,3000);
	}
	window.onload=window_onload;
</script>
</head>
<body bgcolor="#494949">
	<table width="560" border="0" cellpadding="0" cellspacing="0">
		<form name="upgdownloadform" method="post" action="/form/upgrade_cgi">
			<input id="upgradeapply" name="upgradebtn" value="netfail" type="submit" style="display:none">
			<input id="upgdownload" name="processbtn" value="cancel" type="submit" style="display:none">
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
			<td colspan="3" class="cont"><%upgrade_items("UPGRADE_MSG3");%></td>
		</tr>
		<tr><td width="40" height="25"></td>
			<td colspan="3" class="cont"></td>
		</tr>
<!-- for OTA-->
		<tr id="msg"><td width="40" height="10"></td>
			<td class="cont" colspan="3" height="10"><%upgrade_items("UPGRADE_MSG4");%></td>
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
				<img id="apply" src="image/btn_cc1.bmp" onmouseover="chgPIC(this,1)" onmouseout="chgPIC(this,0)" onclick="chgPIC(this,2)">
				<!--<img id="apply" src="image/btn_cc1.bmp">-->
			</td>
			<td width="105"></td>
		</tr>
<!--Main Content-->
		</form>
	</table>
</body>
</html>