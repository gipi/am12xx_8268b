<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN">
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=<%password_items("charset");%>">
<style type="text/css">
<!--
* {margin:0; padding:0; font-size:12px; color:#FFF; font-family: Arial,sans-serif;}
A {color:#000; text-decoration: none; font-weight: bold;}
A:hover {text-decoration: underline;}
.non_ln {color:#000; font-weight: bold;}
.inp {border: 1px solid #000000;}
input[type="password"] {font-size:12px; color:#000; height:17px; border: 1px solid #000000;}
.cont {padding-left:5px;}
-->
</style>
<script language="javascript">
	var chg = 0;
	function sendpwd() {
		if(chg) {
			if(chkvalue() == true) document.getElementById('pwdapply').click();
		}else	chkvalue();
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
			if(confirm(<%password_items("ALERT_LEAVEMSG");%>))
				switch(pth){
					case 1:obj.href="general.asp"; break;
					case 2:obj.href="network.asp"; break;
					case 3:obj.href="security.asp"; break;
					case 4:obj.href="update.asp"; break;
					default: obj.href="#"; break;
				}
			else obj.href="#";
		}
	}
	function chkbtn(obj) {
		if(!chg) {
			if(obj.value.length > 5)
				chgbtnst();
		}
	}
	function chkvalue() {
		if(document.getElementById('_pwd').value.length < 6) {
			alert("<%password_items("ALERT_PWD_INPLIMIT");%>");
			return false;
		}else return true;
	}
</script>
</head>
<body bgcolor="#494949">
	<table width="560" border="0" cellpadding="0" cellspacing="0">
		<form name="emailform" method="post" action="/form/password_cgi">
			<input id="pwdapply" name="pwdbtn" value="apply" type="submit" style="display:none">
		<tr><td width="40" height="15"></td><td width="25"></td>
				<td width="390"></td><td width="105"></td>
		</tr>
		<tr><td width="40" height="25" align="center"><img src="image/arrayb.gif"></td>
				<td colspan="3" height="25"><a href="general.asp" onclick="confst(this,1)"><%password_items("LINK_GENERAL");%></a></td>
		</tr>
		<tr><td width="40" height="25" align="center"><img src="image/arrayb.gif"></td>
				<td colspan="3" height="25"><a href="network.asp" onclick="confst(this,2)"><%password_items("LINK_NETWORK");%></a></td>
		</tr>
		<tr><td width="40" height="25" align="center"><img src="image/arrayb.gif"></td>
				<td colspan="3" height="25"><a href="security.asp" onclick="confst(this,3)"><%password_items("LINK_SECURITY");%></a></td>
		</tr>
		<tr><td width="40" height="30" align="center"><img src="image/array.gif"></td>
				<td class="non_ln" colspan="3" height="24"><%password_items("LINK_PASSWORD");%></td>
		</tr>
<!--Main Content-->
		<tr><td width="40" height="25"></td>
			<td colspan="3" class="cont"><%password_items("PASSWORD_MSG1");%></td>
		</tr>
		<tr><td width="40" height="25"></td>
			<td colspan="2">
				<table width="415">
					<td width="2"></td>
					<td width="158"><%password_items("PASSWORD_MSG2");%></td>
					<td width="255">
						<input type="password" name="newpwd" id="_pwd" value="" maxLength="16" size="32" onblur="chkbtn(this)">
					</td>
				</table>
			</td>
			<td width="105"></td>
		</tr>
		<tr><td width="40" height="25"></td>
			<td colspan="2">
				<table width="415">
					<td width="2"></td>
					<td width="158"><%password_items("PASSWORD_MSG3");%></td>
					<td width="255">
						<input type="password" name="confpwd" id="_cpwd" value="" maxLength="16" size="32" onblur="chkbtn(this)">
					</td>
				</table>
			</td>
			<td width="105"></td>
		</tr>
		<tr><td colspan="4" height="5"></td></tr>
		<tr><td colspan="3" height="50" align="right" valign="top">
				<img id="apply" src="image/btn_apply4.bmp" onmouseover="chgPIC(this,1)" onmouseout="chgPIC(this,0)" onclick="sendpwd()">
			</td>
			<td width="105"></td>
		</tr>
<!--Main Content-->
		<tr><td width="40" height="25" align="center"><img src="image/arrayb.gif"></td>
				<td colspan="3" height="25"><a href="update.asp" onclick="confst(this,4)"><%password_items("LINK_FWUPDATE");%></a></td>
		</tr>
	</form>
	</table>
</body>
</html>