<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN">
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=<%security_items("charset");%>">
<style type="text/css">
<!--
* {margin:0; padding:0; font-size:12px; color:#FFF; font-family: Arial,sans-serif;}
A {color:#000; text-decoration: none; font-weight: bold;}
A:hover {text-decoration: underline;}
.non_ln {color:#000; font-weight: bold;}
.mark {color:#000; font-size:14px; font-weight: bold;
	background:url('image/line.gif') repeat-x; background-position:100% 100%;}
.secu {padding-left:10px;}
.inp {border: 1px solid #000000;}
input[type="password"] {font-size:13px; color:#000; height:20px;}
-->
</style>
<script language="javascript">
	var chg = 0;
	function window_onload() {
		var md;
		if(document.getElementById('secu_3').checked) md = '2';
		else if(document.getElementById('secu_4').checked) md = '3';
		else md = '0';
		show_msg(md,0);
		
		if(document.getElementById('wifi_mode').value != 2){
			document.getElementById('secu_1').disabled=true;
			document.getElementById('secu_3').disabled=true;
			document.getElementById('secu_4').disabled=true;
			document.getElementById('_key').disabled=true;
			document.getElementById('_ckey').disabled=true;
		}
	}
	function chgbtnst() {
		chg = 1;
		document.getElementById('apply').src="image/btn_apply1.bmp";
	}
	function chgPIC(obj,md) {
		if(chg)
			if(md)
				obj.src="image/btn_apply2.bmp";
			else
				obj.src="image/btn_apply1.bmp";
	}
	function confst(obj,pth) {
		if(chg) {
			if(confirm(<%security_items("ALERT_LEAVEMSG");%>))
				switch(pth){
					case 1:obj.href="general.asp"; break;
					case 2:obj.href="network.asp"; break;
					case 3:obj.href="password.asp"; break;
					case 4:obj.href="update.asp"; break;
					default: obj.href="#"; break;
				}
			else obj.href="#";
		}
	}
	function show_msg(val,chg_st){
		switch(val){
			case '2':
			case '3':
				document.getElementById('msg_box').innerHTML="<%security_items("ALERT_SECURITY_PWD");%>";
				document.getElementById('msg').style.display="";
				grayout(0);
				break;
			default:
				document.getElementById('msg').style.display="none";
				grayout(1);
				break;
		}
		if(chg_st) chgbtnst();
	}
	function grayout(tp) {
		if(tp){
			document.getElementById('_ckey').value="";
			document.getElementById('_key').value="";
		}
		document.getElementById('_ckey').disabled=tp;
		document.getElementById('_key').disabled=tp;
	}
	function send_security(){
		if(chg) document.getElementById('SecurityApply').click();
		else return;
	}
</script>
</head>
<body bgcolor="#494949" onload="window_onload()">
	<table width="560" border="0" cellpadding="0" cellspacing="0">
		<input id="wifi_mode" name="wifi" value="<%security_value("WIFI_MODE");%>" type="text" style="display:none">
		<tr><td width="40" height="15"></td><td width="25"></td>
				<td width="390"></td><td width="105"></td>
		</tr>
		<tr><td width="40" height="25" align="center"><img src="image/arrayb.gif"></td>
				<td colspan="3" height="25"><a href="general.asp" onclick="confst(this,1)"><%security_items("LINK_GENERAL");%></a></td>
		</tr>
		<tr><td width="40" height="25" align="center"><img src="image/arrayb.gif"></td>
				<td colspan="3" height="25"><a href="network.asp" onclick="confst(this,2)"><%security_items("LINK_NETWORK");%></a></td>
		</tr>
		<tr><td width="40" height="30" align="center"><img src="image/array.gif"></td>
				<td class="non_ln" colspan="3" height="24"><%security_items("LINK_SECURITY");%></td>
		</tr>
<!--Main Content-->
		<tr><form name="securityform" method="post" action="/form/security_cgi">
					<input id="SecurityApply" name="Securitybtn" value="1" type="submit" style="display:none">
			<td width="40" height="35" align="center">
					<img src="image/Icon_Encryption_Key.bmp"></td>
				<td colspan="2" height="35">
					<table width="415">
						<tr><td width="5"></td>
								<td width="410" class="mark"><%security_items("SECURITY_MSG1");%></td>
						</tr>
					</table>
				</td>
				<td width="105"></td>
		</tr>
		<tr><td width="40" height="30"></td>
				<td colspan="2">
					<input type="radio" class="secu" name="security" id="secu_1" value="0" onclick="show_msg(this.value,1)" <%security_value("SECURITY_NONE");%>> <%security_items("SECURITY_MSG2");%>
					<input type="radio" class="secu" name="security" id="secu_3" value="2" onclick="show_msg(this.value,1)" <%security_value("SECURITY_WPA");%>> <%security_items("SECURITY_MSG3");%>
					<input type="radio" class="secu" name="security" id="secu_4" value="3" onclick="show_msg(this.value,1)" <%security_value("SECURITY_WPA2");%>> <%security_items("SECURITY_MSG4");%>
				</td>
				<td width="105"></td>
		</tr>
		<tr><td colspan="2" height="5"></td></tr>
		<tbody id="msg" style="display:none"><tr><td width="40" height="30"></td>
			<td colspan="2" style="padding-left:10px;">
				<span id="msg_box" style="float:left; width:380px;">
					<!--MSG will show on there-->
				</span></td>
			<td width="105"></td>
		</tr></tbody>
		<tr><td colspan="2" height="5"></td></tr>
		<tr><td width="40" height="30"></td>
				<td colspan="2">
					<table width="415">
						<tr><td width="5"></td>
								<td width="115"><%security_items("SECURITY_MSG5");%></td>
								<td width="295">
									<input type="password" class="inp" name="key" id="_key" maxlength="32" size="33" onblur="chgbtnst()">
								</td>
						</tr>
					</table>
				</td>
				<td width="105"></td>
		</tr>
		<tr><td width="40" height="30"></td>
				<td colspan="2">
					<table width="415">
						<tr><td width="5"></td>
								<td width="115"><%security_items("SECURITY_MSG6");%></td>
								<td width="295">
									<input type="password" class="inp" name="ckey" id="_ckey" maxlength="32" size="33" onblur="chgbtnst()">
								</td>
						</tr>
					</table>
				</td>
				<td width="105"></td>
		</tr>
		<tr><td colspan="3" height="50" align="right" valign="top">
				<img id="apply" src="image/btn_apply4.bmp" onmouseover="chgPIC(this,1)" onmouseout="chgPIC(this,0)" onclick="send_security()">
			</td>
			<td width="105"></td>
			</form>
		</tr>
<!--Main Content-->
		<tr><td width="40" height="25" align="center"><img src="image/arrayb.gif"></td>
				<td colspan="3" height="25"><a href="password.asp" onclick="confst(this,3)"><%security_items("LINK_PASSWORD");%></a></td>
		</tr>
		<tr><td width="40" height="25" align="center"><img src="image/arrayb.gif"></td>
				<td colspan="3" height="25"><a href="update.asp" onclick="confst(this,4)"><%security_items("LINK_FWUPDATE");%></a></td>
		</tr>
	</table>
</body>
</html>