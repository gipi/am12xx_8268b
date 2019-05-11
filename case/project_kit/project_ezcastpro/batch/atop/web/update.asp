<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN">
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=<%fwupdate_items("charset");%>">
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
	function chgPIC(obj,md) {
			switch(md){
				case 1: obj.src="image/btn_ug2.bmp"; break;
				case 2: obj.src="image/btn_ug3.bmp"; 
								document.getElementById('updateapply').click();
					break;
				default: obj.src="image/btn_ug1.bmp"; break;
			}
	}
	function confst(obj,pth) {
			switch(pth){
				case 1:obj.href="general.asp"; break;
				case 2:obj.href="network.asp"; break;
				case 3:obj.href="security.asp"; break;
				case 4:obj.href="password.asp"; break;
				default: obj.href="#"; break;
			}
	}
</script>
</head>
<body bgcolor="#494949">
	<table width="560" border="0" cellpadding="0" cellspacing="0">
		<form name="updateform" method="post" action="/form/fwupdate_cgi">
			<input id="updateapply" name="updatebtn" value="apply" type="submit" style="display:none">
		<tr><td width="40" height="15"></td><td width="25"></td>
				<td width="390"></td><td width="105"></td>
		</tr>
		<tr><td width="40" height="25" align="center"><img src="image/arrayb.gif"></td>
				<td colspan="3" height="25"><a href="general.asp" onclick="confst(this,1)"><%fwupdate_items("LINK_GENERAL");%></a></td>
		</tr>
		<tr><td width="40" height="25" align="center"><img src="image/arrayb.gif"></td>
				<td colspan="3" height="25"><a href="network.asp" onclick="confst(this,2)"><%fwupdate_items("LINK_NETWORK");%></a></td>
		</tr>
		<tr><td width="40" height="25" align="center"><img src="image/arrayb.gif"></td>
				<td colspan="3" height="25"><a href="security.asp" onclick="confst(this,3)"><%fwupdate_items("LINK_SECURITY");%></a></td>
		</tr>
		<tr><td width="40" height="25" align="center"><img src="image/arrayb.gif"></td>
				<td colspan="3" height="25"><a href="password.asp" onclick="confst(this,4)"><%fwupdate_items("LINK_PASSWORD");%></a></td>
		</tr>
		<tr><td width="40" height="30" align="center"><img src="image/array.gif"></td>
				<td class="non_ln" colspan="3" height="24"><%fwupdate_items("LINK_FWUPDATE");%></td>
		</tr>
<!--Main Content-->
		<tr><td width="40" height="25"></td>
			<td colspan="2" class="cont"><%fwupdate_items("FWUPDATE_MSG1");%></td>
		</tr>
		<tr><td width="40" height="25"></td>
			<td colspan="2" class="cont"><%fwupdate_items("FWUPDATE_MSG2");%></td>
		<td width="105"></td>
		</tr>
		<tr><td colspan="4" height="40"></td></tr>
		<tr><td colspan="3" height="50" align="right" valign="top">
				<img id="apply" src="image/btn_ug1.bmp" onmouseover="chgPIC(this,1)" onmouseout="chgPIC(this,0)" onclick="chgPIC(this,2)">
			</td>
			<td width="105"></td>
		</tr>
<!--Main Content-->
	</form>
	</table>
</body>
</html>