<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN">
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=<%general_items("charset");%>">
<style type="text/css">
<!--
* {margin:0; padding:0; font-size:12px; color:#FFF; font-family: Arial,sans-serif;}
A {color:#000; text-decoration: none; font-weight: bold; font-family:Arial, sans-serif;}
A:hover {text-decoration: underline;}
.non_ln {color:#000; font-weight: bold;}
.mark {color:#000; font-weight: bold; background:url('image/line.gif') repeat-x; background-position:100% 85%;}
.cont {font-size:12px; font-family:Arial, sans-serif;}
-->
</style>
<script language="javascript">
	var chg = 0;
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
			if(confirm(<%general_items("ALERT_LEAVEMSG");%>))
				switch(pth){
					case 1:obj.href="network.asp"; break;
					case 2:obj.href="security.asp"; break;
					case 3:obj.href="password.asp"; break;
					case 4:obj.href="update.asp"; break;
					default: obj.href="#"; break;
				}
			else obj.href="#";
		}
	}
	function general_send() {
		if(chg) document.getElementById('generalapply').click();
		else return;
	}
</script>
</head>
<body bgcolor="#494949">
	<table width="560" border="0" cellpadding="0" cellspacing="0">
		<form name="generalform" method="post" action="/form/general_cgi">
			<input id="generalapply" name="general" value="" type="submit" style="display:none">
		<tr><td colspan="4" height="15"></td></tr>
		<tr><td width="40" height="30" align="center"><img src="image/array.gif"></td>
				<td class="non_ln" colspan="3" height="24"><%general_items("LINK_GENERAL");%></td>
		</tr>
<!--Main Content-->
		<tr><td width="40" height="30" align="right">
					<img src="image/Icon_Projector_ID.bmp"></td>
				<td width="25"></td>
				<td width="390" class="mark"><%general_items("GENERAL_MSG1");%></td>
				<td width="105"></td>
		</tr>
		<tr><td width="40" height="30"></td>
				<td width="25"></td>
				<td width="390">
					<table>
						<tr><td width="25" height="30">
									<input type="checkbox" name="idchk" id="_idchk" value="1" onclick="chgbtnst()" <%general_value("PROJ_ID");%>><td>
								<td width="405" class="non_ln"><%general_items("GENERAL_MSG2");%></td>
						</tr>
						<tr><td width="25" height="50"><td>
								<td width="405" class="cont"><%general_items("GENERAL_MSG3");%>
								</td>
						</tr>
					</table>
				</td>
				<td width="105"></td>
		</tr>
		<tr><td width="40" height="30" align="right">
					<img src="image/Icon_Internet_connection.bmp"></td>
				<td width="25"></td>
				<td width="390" class="mark"><%general_items("GENERAL_MSG4");%></td>
				<td width="105"></td>
		</tr>
		<tr><td width="40" height="30"></td>
				<td width="25"></td>
				<td width="390">
					<table>
						<tr><td width="25" height="50" valign="top"><td>
								<td width="405" class="cont"><%general_items("GENERAL_MSG6");%>
								</td>
						</tr>
					</table>
				</td>
				<td width="105"></td>
		</tr>
		<tr><td colspan="3" height="50" align="right" valign="top">
				<img id="apply" src="image/btn_apply4.bmp" onmouseover="chgPIC(this,1)" onmouseout="chgPIC(this,0)" onclick="general_send()">
			</td>
			<td width="105"></td>
		</tr>
<!--Main Content-->
		<tr><td width="40" height="25" align="center"><img src="image/arrayb.gif"></td>
				<td colspan="3" height="25"><a href="network.asp" onclick="confst(this,1)"><%general_items("LINK_NETWORK");%></a></td>
		</tr>
		<tr><td width="40" height="25" align="center"><img src="image/arrayb.gif"></td>
				<td colspan="3" height="25"><a href="security.asp" onclick="confst(this,2)"><%general_items("LINK_SECURITY");%></a></td>
		</tr>
		<tr><td width="40" height="25" align="center"><img src="image/arrayb.gif"></td>
				<td colspan="3" height="25"><a href="password.asp" onclick="confst(this,3)"><%general_items("LINK_PASSWORD");%></a></td>
		</tr>
		<tr><td width="40" height="25" align="center"><img src="image/arrayb.gif"></td>
				<td colspan="3" height="25"><a href="update.asp" onclick="confst(this,4)"><%general_items("LINK_FWUPDATE");%></a></td>
		</tr>
		</form>
	</table>
</body>
</html>