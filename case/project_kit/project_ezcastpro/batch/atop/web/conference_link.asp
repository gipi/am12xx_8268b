<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN">
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=<%conferlink_items("charset");%>" />
<title></title>
<style type="text/css">
<!--
* {margin:0; padding:0; color:#FFF; font-size:12px; font-family: Arial,sans-serif; overflow-x:hidden;}
td {overflow-y:hidden;}
A {color:#FFF; text-decoration: none; font-weight: bold;}
A:hover {text-decoration: underline;}
#lnk td {padding-right:5px;}
.copy_right {padding-left:10px; font-size:12px; color:#494949; font-weight: bold;}
#apply {padding-right:10px;}
-->
</style>
<script src="js/jquery.js"></script>
<script src="js/ConferenceControl.js"></script>
<script type="text/javascript">
	$(function() {
  	$('a.active').removeAttr('href');
  });
</script>
<script language="javascript">
	function showIMG(obj,tp){
		if(tp)
			obj.style.background = "url(image/menu_f.png) no-repeat right";
		else
			obj.style.background = "";
	}
	function chgPIC(obj,md) {
		switch(md) {
			case 1: obj.src="image/btn_end_conference2.bmp"; break;
			case 2: 
				obj.src="image/btn_end_conference3.bmp"; 
				EndConf();
				break;
			default: obj.src="image/btn_end_conference1.bmp"; break;
		}
	}
</script>
</head>

<body>
	<table bgcolor="#000" width="760" border="0" cellpadding="0" cellspacing="0" style="margin-left:auto; margin-right:auto;">
		<tr><td colspan="2" width="760" height="70" background="image/TopLogo.gif"></td>
			<!--<td width="760" colspan="2" height="70"><iframe src="title.asp" scrolling="No" frameborder="no" width="760" height="70"></iframe></td>-->
		</tr>
		<tr>
			<form name="logoutform" method="post" action="/form/logout_cgi">
				<input id="logoutbtn" name="logout" value="showmsg" type="submit" style="display:none">
			<td height="25" style="font-size:12px;"><a href="javascript:EndConf();">ACER</a><b> &gt;</b> 
				<a href="conference_link.asp"><%conferlink_items("LINK_CONFERENCE");%></a>
			</td>
			</form>
		</tr>
			
		<tr><td bgcolor="#000" width="200" valign="top" style="font-size:12px;">
				<!--<iframe src="link_adm.asp" scrolling="No" frameborder="no" width="200"></iframe>-->
			<table id="lnk" width="200" border="0" cellpadding="0" cellspacing="0">
				<tr><td width="200" height="11"></td></tr>
				<tr><td height="20" align="right"></td></tr>
				<tr><td height="20" align="right"></td></tr>
				<tr><td height="20" align="right"></td></tr>
				<tr><td height="20" align="right"></td></tr>
			</table>
			</td>
			
			<!--<td width="5"></td>-->
			<td bgcolor="#494949" width="560" valign="top">
				<table width="555" border="0" cellpadding="0" cellspacing="0">
					<form name="conferlinkform" method="post" action="/form/conferlink_cgi">
						<input id="EndApply" name="Endbtn" value="1" type="submit" style="display:none">
					<tr><td width="60" height="10"></td><td width="495"></td></tr>
					<tr><td height="20" width="60"></td>
							<td width="495" style="font-family:Arial,sans-serif;">
								<!--Click 'Project' button in the following list to autorize someone to project, or click-->
								<%conferlink_items("CONFERENCE_MSG1");%></td>
					</tr>
					<tr><td height="20" width="60"></td>
							<td width="495" style="font-family:Arial,sans-serif;">
								<!--'Stop' to stop someone projecting.-->
								<%conferlink_items("CONFERENCE_MSG2");%></td>
					</tr>
					<tr><td width="30" height="36"></td>
							<td width="495" align="right" valign="bottom">
								<a href="#"><img id="apply" src="image/btn_end_conference1.bmp" onmouseover="chgPIC(this,1)" onmouseout="chgPIC(this,0)" onclick="chgPIC(this,2)" border=0></a></td>
					</tr>
					<tr><td width="30" height="4"></td>
							<td width="495"></td>
					</tr>
					</form>
				</table>
			</td>
		</tr>
		
		<tr><td bgcolor="#000" width="200" valign="top">
			<table width="200" border="0" cellpadding="0" cellspacing="0">
			</table>
			</td>
			
			<td width="560" valign="top">
				<table border=0 cellpadding="0" cellspacing="0" width="560" id="Connection_Status">
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