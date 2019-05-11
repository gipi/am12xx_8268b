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
	var count = 0;
	function chgPIC(obj,md) {
			switch(md){
				case 1: obj.src="image/btn_cc2.bmp"; break;
				case 2: obj.src="image/btn_cc3.bmp"; 
								document.getElementById('upgradeapply').click();
					break;
				default: obj.src="image/btn_cc1.bmp"; break;
			}
	}
	function createREQ() {
	//--- Define XMLHttpRequest object
		if(window.XMLHttpRequest){
			upgrade_req = new XMLHttpRequest(); //--FF/Safari
		}else if(window.ActiveXObject){
			try{
				upgrade_req = new ActiveXObject("Msxml2.XMLHTTP"); //--IE
			}catch(e){
				try{
					upgrade_req = new ActiveXObject("Microsoft.XMLHTTP");
				}catch(e){}
			}
		}
	}
	function get_link_state(){
		createREQ();
		upgrade_req.open("POST", "/form/upgrade_link_cgi", true);
		upgrade_req.onreadystatechange = function() {
			if(upgrade_req.readyState == 4 && upgrade_req.status == 200){
				var RSPUpgradeText = upgrade_req.responseText;
				//alert(RSPConsoleText);
				var RSPUpgrade=eval('('+RSPUpgradeText.slice(6)+')');
				if(RSPUpgrade.status == 0){ //--The latest Version
					document.getElementById('upgradeapply').value="latest";
					document.getElementById('upgradeapply').click();
				}else if(RSPUpgrade.status == 1){ //--Have new Version
					document.getElementById('upgradeapply').value="newer";
					document.getElementById('upgradeapply').click();
				}else if(RSPUpgrade.status == 2){ //--Network Fail
					document.getElementById('upgradeapply').value="netfail";
					document.getElementById('upgradeapply').click();
				}	
			}
		}
		upgrade_req.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
		var str = "getconn=getconn";
		//alert(str);
		upgrade_req.send(str);	
	}
	window.onload=setTimeout(get_link_state,3000);
</script>
</head>
<body bgcolor="#494949">
	<table width="560" border="0" cellpadding="0" cellspacing="0">
		<form name="upgradelinkform" method="post" action="/form/upgrade_cgi">
			<input id="upgradeapply" name="upgradebtn" value="apply" type="submit" style="display:none">
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
			<td colspan="2" class="cont"><%upgrade_items("UPGRADE_MSG1");%></td>
		</tr>
		<tr><td width="40" height="25"></td>
			<td colspan="2" class="cont"><%upgrade_items("UPGRADE_MSG2");%></td>
		<td width="105"></td>
		</tr>
		<tr><td colspan="4" height="40"></td></tr>
		<tr><td colspan="3" height="50" align="right" valign="top">
				<img id="apply" src="image/btn_cc1.bmp" onmouseover="chgPIC(this,1)" onmouseout="chgPIC(this,0)" onclick="chgPIC(this,2)">
			</td>
			<td width="105"></td>
		</tr>
<!--Main Content-->
	</form>
	</table>
</body>
</html>