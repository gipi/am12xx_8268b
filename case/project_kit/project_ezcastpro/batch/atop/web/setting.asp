<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN">
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=<%email_items("charset");%>" />
<title></title>
<style type="text/css">
<!--
* {margin:0; padding:0; color:#FFF; font-size:12px; font-family: Arial,sans-serif; overflow-x:hidden;}
td {overflow-y:hidden;}
A {color:#FFF; text-decoration: none; font-weight: bold;}
A:hover {text-decoration: underline;}
#lnk td {padding-right:5px;}
.copy_right {padding-left:10px; font-size:12px; color:#494949; font-weight: bold;}
.subtitle {color:#000; font-weight: bold; font-size:13px;}
.btn {color:#000; height:23px; padding-right:5px; padding-left:5px;}
input {color:#000;}
.tit {color:#000; font-weight: bold;}
-->
</style>
<script language="javascript">
	var chg = 0;
	var email_req;
	function chgbtnst() {
		chg = 1;
		document.getElementById('apply').src="image/btn_apply1.bmp";
	}
	function chkbtn(obj) {
		if(!chg) {
			if(obj.value.length > 0)
				chgbtnst();
		}
	}
	function showIMG(obj,tp){
		if(tp)
			obj.style.background = "url(image/menu_f.png) no-repeat right";
		else
			obj.style.background = "";
	}
	function chgPIC(obj,md) {
		if(chg)
			if(md)
				obj.src="image/btn_apply2.bmp";
			else
				obj.src="image/btn_apply1.bmp";
	}
	function sendapply() {
		if(chg) document.getElementById('applybtn').click();
	}
	function createREQ() {
	//--- Define XMLHttpRequest object
		if(window.XMLHttpRequest){
			email_req = new XMLHttpRequest(); //--FF/Safari
		}else if(window.ActiveXObject){
			try{
				email_req = new ActiveXObject("Msxml2.XMLHTTP"); //--IE
			}catch(e){
				try{
					email_req = new ActiveXObject("Microsoft.XMLHTTP");
				}catch(e){}
			}
		}
	}
	function sendmail(obj){
		createREQ();
		email_req.open("POST", "/form/email_cgi", true);
		//console_req.open("POST", "http://10.0.51.9", true);
		email_req.onreadystatechange = function() {
			if(email_req.readyState == 4 && email_req.status == 200){
				alert("send test mail successed.");
			}
		}
		email_req.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
		var str = obj.name+"="+obj.name;
		email_req.send(str);
	}
</script>
</head>

<body>
	<table bgcolor="#000" width="760" border="0" cellpadding="0" cellspacing="0" style="margin-left:auto; margin-right:auto;">
		<tr><td colspan="2" width="760" height="70" background="image/TopLogo.gif"></td>
			<!--<td width="760" colspan="2" height="70"><iframe src="title.asp" scrolling="No" frameborder="no" width="760" height="70"></iframe></td>-->
		</tr>
		<tr><td colspan="2" height="25" style="font-size:12px;"><a href="javascript:document.getElementById('logoutbtn').click();">ACER</a><b> &gt;</b> 
			<a href="login.asp"><%email_items("LINK_ADVANCE");%></a><b> &gt;</b>
			<a href="setting.asp"><%email_items("LINK_MAIL");%></a>
			</td></tr>
			
		<tr><td bgcolor="#000" width="200" height="200" valign="top" style="font-size:12px;">
				<!--<iframe src="link_adm.asp" scrolling="No" frameborder="no" width="200"></iframe>-->
			<table id="lnk" width="200" border="0" cellpadding="0" cellspacing="0">
				<form name="logoutform" method="post" action="/form/logout_cgi">
					<input id="logoutbtn" name="logout" value="log_out" type="submit" style="display:none">
				<tr><td width="200" height="15"></td></tr>
				<tr><td height="24" align="right" onmouseover="showIMG(this,1)" onmouseout="showIMG(this,0)">
					<a href="console.asp"><%email_items("LINK_CONSOLE");%></a></td></tr>
				<tr><td height="24" align="right" onmouseover="showIMG(this,1)" onmouseout="showIMG(this,0)">
					<a href="config.asp"><%email_items("LINK_CONFIGURE");%></a></td></tr>
				<tr><td height="24" align="right" onmouseover="showIMG(this,1)" onmouseout="showIMG(this,0)">
					<a href="setting.asp"><%email_items("LINK_MAIL");%></a></td></tr>
				<tr><td height="24" align="right" onmouseover="showIMG(this,1)" onmouseout="showIMG(this,0)">
					</td></tr>
				<tr><td height="24" align="right" onmouseover="showIMG(this,1)" onmouseout="showIMG(this,0)">
					<a href="javascript:document.getElementById('logoutbtn').click();"><%email_items("LINK_LOGOUT");%></a></td></tr>
				</form>
			</table>
			</td>
			
			<!--<td width="5"></td>-->
			<td bgcolor="#494949" width="560" valign="top">
				<form name="emailform" method="post" action="/form/email_cgi">
					<input id="applybtn" value="apply" type="submit" style="display:none">
				<table width="555" border="0" cellpadding="0" cellspacing="0">
					<tr><td width="35" height="15"></td>
							<td width="10"></td>
							<td width="10"></td>
							<td width="180"></td>
							<td width="320"></td>
					</tr>
					<tr><td width="35" height="30" align="right"><img src="image/array.gif"></td>
							<td width="10"></td>
							<td colspan="3" class="tit"><%email_items("MAIL_MSG1");%></td>
					</tr>
					<tr><td width="35" height="30"></td>
							<td width="10"></td><td width="10"></td>
							<td colspan="3">
								<table width="500" border="0" cellpadding="0" cellspacing="0">
									<td width="20"><input type="checkbox" name="ALT_1" id="FAN" value="1" onblur="chgbtnst()" <%email_value("FAN_ALT");%>></td>
									<td width="480"><%email_items("MAIL_MSG2");%></td>
								</table>
							</td>
					</tr>
					<tr><td width="35" height="30"></td>
							<td width="10"></td><td width="10"></td>
							<td colspan="3">
								<table width="500" border="0" cellpadding="0" cellspacing="0">
									<td width="20"><input type="checkbox" name="ALT_2" id="LAMP" value="2" onblur="chgbtnst()" <%email_value("LAMP_ALT");%>></td>
									<td width="480"><%email_items("MAIL_MSG3");%></td>
								</table>
							</td>
					</tr>
					<tr><td width="35" height="30"></td>
							<td width="10"></td><td width="10"></td>
							<td colspan="3">
								<table width="500" border="0" cellpadding="0" cellspacing="0">
									<td width="20"><input type="checkbox" name="ALT_3" id="LAMPHR" value="3" onblur="chgbtnst()" <%email_value("HOURS_ALT");%>></td>
									<td width="480"><%email_items("MAIL_MSG4");%></td>
								</table>
							</td>
					</tr>
					<tr><td width="35" height="30"></td>
							<td width="10"></td><td width="10"></td>
							<td colspan="3">
								<table width="500" border="0" cellpadding="0" cellspacing="0">
									<td width="20"><input type="checkbox" name="ALT_4" id="OVERHEAT" value="4" onblur="chgbtnst()" <%email_value("OVERHEAT_ALT");%>></td>
									<td width="480"><%email_items("MAIL_MSG5");%></td>
								</table>
							</td>
					</tr>
					<tr><td width="35" height="30"></td>
							<td width="10"></td><td width="10"></td>
							<td colspan="3">
								<table width="500" border="0" cellpadding="0" cellspacing="0">
									<td width="20"><input type="checkbox" name="ALT_5" id="COLOR" value="5" onblur="chgbtnst()" <%email_value("COLORWHEEL_ALT");%>></td>
									<td width="480"><%email_items("MAIL_MSG6");%></td>
								</table>
							</td>
					</tr>
					<tr><td width="40" height="30" align="right"><img src="image/array.gif"></td>
							<td width="10"></td>
							<td colspan="3" class="tit"><%email_items("MAIL_MSG7");%></td>
					</tr>
					<tr><td width="40" height="30"></td>
							<td width="10"></td><td width="10"></td>
							<td colspan="3">
								<table width="500" border="0" cellpadding="0" cellspacing="0">
									<td width="80"><%email_items("MAIL_MSG7");%></td>
									<td width="420">
											<input type="radio" name="alert" id="alt_on" value="1" <%email_value("mail_on");%>> <%email_items("MAIL_MSG8");%> &nbsp;
											<input type="radio" name="alert" id="alt_off" value="0" <%email_value("mail_off");%>> <%email_items("MAIL_MSG9");%> &nbsp;
											<input type="button" class="btn" name="btntest" id="test_mail" value="<%email_items("MAIL_MSG10");%>" onclick="sendmail(this)">
									</td>
								</table>
							</td>
					</tr>
					<tr><td width="40" height="30"></td>
							<td width="10"></td><td width="10"></td>
							<td width="180"><%email_items("MAIL_MSG11");%></td>
							<td width="320">
								<input type="text" name="toaddr" id="_to" value="<%email_value("to_addr");%>" size="32" maxlength="180" onblur="chkbtn(this)">
							</td>
					</tr>
					<tr><td width="40" height="30"></td>
							<td width="10"></td><td width="10"></td>
							<td width="180"><%email_items("MAIL_MSG12");%></td>
							<td width="320">
								<input type="text" name="ccaddr" id="_cc" value="<%email_value("cc_addr");%>" size="32" maxlength="180" onblur="chkbtn(this)">
							</td>
					</tr>
					<tr><td width="40" height="30"></td>
							<td width="10"></td><td width="10"></td>
							<td width="180"><%email_items("MAIL_MSG13");%></td>
							<td width="320">
								<input type="text" name="fromaddr" id="_from" value="<%email_value("from_addr");%>" size="32" maxlength="50" onblur="chkbtn(this)">
							</td>
					</tr>
					<tr><td width="40" height="30"></td>
							<td width="10"></td><td width="10"></td>
							<td width="180"><%email_items("MAIL_MSG14");%></td>
							<td width="320">
								<input type="text" name="mailsubject" id="_subj" value="<%email_value("subject");%>" size="36" maxlength="32" onblur="chkbtn(this)">
							</td>
					</tr>
					<tr><td width="40" height="30" align="right"><img src="image/array.gif"></td>
							<td width="10"></td>
							<td colspan="3" class="tit"><%email_items("MAIL_MSG15");%></td>
					</tr>
					<tr><td width="40" height="30"></td>
							<td width="10"></td><td width="10"></td>
							<td width="180"><%email_items("MAIL_MSG16");%></td>
							<td width="320">
								<input type="text" name="smtp" id="_smtp" value="<%email_value("smtp");%>" size="36" maxlength="32" onblur="chkbtn(this)">
							</td>
					</tr>
					<tr><td width="40" height="30"></td>
							<td width="10"></td><td width="10"></td>
							<td width="180"><%email_items("MAIL_MSG17");%></td>
							<td width="320">
								<input type="text" name="mailusername" id="_usrname" value="<%email_value("username");%>" size="36" maxlength="32" onblur="chkbtn(this)">
							</td>
					</tr>
					<tr><td width="40" height="30"></td>
							<td width="10"></td><td width="10"></td>
							<td width="180"><%email_items("MAIL_MSG18");%></td>
							<td width="320">
								<input type="password" name="mailpwd" id="_password" value="<%email_value("mail_pwd");%>" size="16" maxlength="16" onblur="chkbtn(this)">
							</td>
					</tr>
					<tr><td width="40" height="70"></td>
							<td width="10"></td><td width="10"></td>
							<td width="180"></td>
							<td width="320" align="right" valign="top" style="padding-right:15px;">
								<img id="apply" src="image/btn_apply4.bmp" onmouseover="chgPIC(this,1)" onmouseout="chgPIC(this,0)" onclick="sendapply()"></td>
					</tr>
				</table>
				</form>
			</td>
		</tr>
		
		<tr><td class="copy_right" colspan="3" height="55" bgcolor="#000" valign="bottom">
			Copyright &copy; 2013 Acer Inc. All Rights Reserved
			</td></tr>
		<tr><td colspan="3" height="10" bgcolor="#000"></td></tr>
	</table>
</body>
</html>