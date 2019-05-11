<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN">
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=<%config_items("charset");%>" />
<title></title>
<style type="text/css">
<!--
* {margin:0; padding:0; color:#FFF; font-family: Arial,sans-serif; overflow-x:hidden;}
A {color:#FFF; text-decoration: none; font-weight: bold;}
A:hover {text-decoration: underline;}
#lnk td {padding-right:5px;}
.copy_right {padding-left:10px; font-size:12px; color:#494949; font-weight: bold;}
.subtitle {color:#000; font-weight: bold; font-size:13px;}
.btn {color:#000; width:100px; height:20px; line-height:10px;}
option {color:#000;}
.plus_minus {color:#000; width:18px; height:25px;}
input[type="text"] {color:#000;}
#adv {color:#000000; font-weight: bold; font-family: Arial, sans-serif;}
#adv:hover {border-bottom:1px solid #fff;}
-->
</style>
<script language="javascript">
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
	function logout() {
		document.getElementById('logoutbtn').click();
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
		<tr><td colspan="2" height="25" style="font-size:12px;"><a href="javascript:logout();">ACER</a><b> &gt;</b> 
			<a href="login.asp"><%config_items("LINK_ADVANCE");%></a><b> &gt;</b>
			<a href="console.asp"><%config_items("LINK_CONFIGURE");%></a>
			</td></tr>
			
		<tr><td bgcolor="#000" width="200" height="200" valign="top" style="font-size:12px;">
				<!--<iframe src="link_adm.asp" scrolling="No" frameborder="no" width="200"></iframe>-->
			<table id="lnk" width="200" border="0" cellpadding="0" cellspacing="0">
				<tr><td width="200" height="15"></td></tr>
				<tr><td height="24" align="right" onmouseover="showIMG(this,1)" onmouseout="showIMG(this,0)">
					<a href="console.asp"><%config_items("LINK_CONSOLE");%></a></td></tr>
				<tr><td height="24" align="right" onmouseover="showIMG(this,1)" onmouseout="showIMG(this,0)">
					<a href="config.asp"><%config_items("LINK_CONFIGURE");%></a></td></tr>
				<tr><td height="24" align="right" onmouseover="showIMG(this,1)" onmouseout="showIMG(this,0)">
					<a href="setting.asp"><%config_items("LINK_MAIL");%></a></td></tr>
				<tr><td height="24" align="right" onmouseover="showIMG(this,1)" onmouseout="showIMG(this,0)">
					</td></tr>
				<tr><td height="24" align="right" onmouseover="showIMG(this,1)" onmouseout="showIMG(this,0)">
					<a href="javascript:logout();"><%config_items("LINK_LOGOUT");%></a></td></tr>
			</table>
			</td>
			
			<td bgcolor="#494949" valign="top" style="font-size:12px;" width="560" height="520">
				<!--<iframe id="mwindow" src="general.asp" scrolling="No" frameborder="no" width="560" height="515"></iframe>-->
				<iframe id="mwindow" src="general.asp" frameborder="no" width="560" height="515"></iframe>
			</td>
		</tr>
		
		<tr><td class="copy_right" colspan="3" height="55" bgcolor="#000" valign="bottom">
			Copyright &copy; 2013 Acer Inc. All Rights Reserved 
			</td></tr>
		<tr><td colspan="3" height="10" bgcolor="#000"></td></tr>
	</table>
</body>
</html>