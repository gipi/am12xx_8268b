<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN">
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=<%login_items("charset");%>" />
<title></title>
<style type="text/css">
<!--
* {margin:0; padding:0; color:#FFF; font-size:12px; font-family: Arial,sans-serif; overflow-x:hidden;}
A {color:#FFF; text-decoration: none; font-weight: bold;}
A:hover {text-decoration: underline;}
#lnk td {padding-right:5px;}
.copy_right {padding-left:10px; font-size:12px; color:#494949; font-weight: bold;}
input {color:#000;}
.btn {margin-left:15px; height:23px; font-size:13px; padding-right:5px; padding-left:5px;}
-->
</style>
<script language="javascript">
	function showIMG(obj,tp){
		if(tp)
			obj.style.background = "url(image/menu_f.png) no-repeat right";
		else
			obj.style.background = "";
	}
	function calcResponse(){
		document.getElementById('loginApply').click();
	}
	function txtPassword_onkeypress(event) {
		kcode = event.keyCode;
		if (kcode==13) { calcResponse(); return; }
		return;
	}
</script>
</head>

<body>
	<table bgcolor="#000" width="760" border="0" cellpadding="0" cellspacing="0" style="margin-left:auto; margin-right:auto;">
		<!--<form name="loginform" method="post" action="/form/login_cgi">
			<input id="loginApply" name="loginbtn" value="" type="submit" style="display:none">-->
		<tr><td colspan="2" width="760" height="70" background="image/TopLogo.gif"></td>
			<!--<td width="760" colspan="2" height="70"><iframe src="title.htm" scrolling="No" frameborder="no" width="760" height="70"></iframe></td>-->
		</tr>
		<tr>
			<form name="logoutform" method="post" action="/form/logout_cgi">
				<input id="logoutbtn" name="logout" value="log_out" type="submit" style="display:none">
			<td height="25" style="font-size:12px;"><a href="javascript:document.getElementById('logoutbtn').click();">ACER</a><b> &gt;</b> 
				<a href="login.asp"><%login_items("LINK_ADVANCE");%></a>
			</td>
			</form>
		</tr>
			
		<tr><td bgcolor="#000" width="200" height="180" valign="top" style="font-size:12px;">
				<!--<iframe src="link_adm.htm" scrolling="No" frameborder="no" width="200"></iframe>-->
			<table id="lnk" width="200" border="0" cellpadding="0" cellspacing="0">
				<tr><td width="200" height="15"></td></tr>
				<tr><td height="24" align="right" onmouseover="showIMG(this,1)" onmouseout="showIMG(this,0)">
					<a href="download.asp"><%login_items("LINK_DOWNLOAD");%></a></td></tr>
				<tr><td height="24" align="right" onmouseover="showIMG(this,1)" onmouseout="showIMG(this,0)">
					<a href="conference.asp"><%login_items("LINK_CONFERENCE");%></a></td></tr>
				<tr><td height="24" align="right" onmouseover="showIMG(this,1)" onmouseout="showIMG(this,0)">
					<a href="login.asp"><%login_items("LINK_ADVANCE");%></a></td></tr>
				<tr><td height="24" align="right" onmouseover="showIMG(this,1)" onmouseout="showIMG(this,0)">
					<a href="crestron.asp">Crestron</a></td></tr>
			</table>
			</td>
			
			<!--<td width="5"></td>-->
			<td bgcolor="#494949" width="560" valign="top">
				<table width="555" border="0" cellpadding="0" cellspacing="0">
					<form name="loginform" method="post" action="/form/login_cgi">
						<input id="loginApply" name="loginbtn" value="" type="submit" style="display:none">
					<tr><td height="50"></td></tr>
					<tr><td height="26" width="175" align="right"><%login_items("LOGIN_MSG1");%></td>
							<td height="26" align="left" style="color:#000;font-weight:bold; padding-left:15px;"><%login_items("LOGIN_MSG2");%></td>
					</tr>
					<tr><td height="10"></td></tr>
					<tr><td height="26" width="175" align="right" valign="top" style="padding-top:3px;"><%login_items("LOGIN_MSG3");%></td>
							<td height="26" align="left" valign="top" style="padding-left:15px;">
								<input id="pwd" name="password" type="password" size="20" maxlength="16" style="height:18px;" onkeypress="return txtPassword_onkeypress(event)">
								<input class="btn" name="login" type="button" value="<%login_items("LOGIN_MSG4");%>" onclick="calcResponse(); return false;">
							</td>
					</tr>
					</form>
				</table>
			</td>
		</tr>
		
		<tr><td class="copy_right" colspan="3" height="55" bgcolor="#000" valign="bottom">
			Copyright &copy; 2013 Acer Inc. All Rights Reserved 
			</td></tr>
		<tr><td colspan="3" height="10" bgcolor="#000"></td></tr>
	</table>
</body>
</html>