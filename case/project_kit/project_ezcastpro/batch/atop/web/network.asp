<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN">
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=<%network_items("charset");%>">
<style type="text/css">
<!--
* {margin:0; padding:0; color:#FFF; font-family: Arial,sans-serif; overflow-x:hidden;}
A {color:#000; text-decoration: none; font-size:12px; font-weight: bold;}
A:hover {text-decoration: underline;}
.non_ln {color:#000; font-size:12px; font-weight: bold;}
.net1 {font-size:12px; padding-left:25px; line-height:20px;}
input[type="text"] {font-size:12px; color:#000;}
select {color:#000;}
option {color:#000;}
.addr {width:130px; height:22px; border:1px solid #158597; font-size:5px; background:#fff;}
.addr_inp {width:25px; height:20px; border:0; text-align:center; line-height:20px;}
span {width:1px; color:#aaa; font-size:12px; font-weight: bold;}
-->
</style>
<script language="javascript">
	var chg = 0;
	function window_onload() {
		if(document.getElementById('wifi_mode').value != 2){
			document.getElementById('_SSID').disabled=true;
			document.getElementById('_channel').disabled=true;
			document.getElementById('serv_off').disabled=true;
			document.getElementById('serv_auto').disabled=true;
			for(var i=1;i<5;i++){
				document.getElementById('_START'+i).disabled=true;
				document.getElementById('_END'+i).disabled=true;
				document.getElementById('_SERVGW'+i).disabled=true;
		}
		}
		if(document.getElementById('dhcp_on').checked)
			for(var i=1;i<5;i++){
				document.getElementById('_IP'+i).disabled=true;
				document.getElementById('_MASK'+i).disabled=true;
				document.getElementById('_GW'+i).disabled=true;
				document.getElementById('_DNS'+i).disabled=true;
		}
		if(document.getElementById('serv_auto').checked)
			for(var i=1;i<5;i++){
				document.getElementById('_START'+i).disabled=true;
				document.getElementById('_END'+i).disabled=true;
				document.getElementById('_SERVGW'+i).disabled=true;
		}
	}
	function sendnet() {
		if(chg) document.getElementById('netapply').click();
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
			if(confirm(<%network_items("ALERT_LEAVEMSG");%>))
				switch(pth){
					case 1:obj.href="general.asp"; break;
					case 2:obj.href="security.asp"; break;
					case 3:obj.href="password.asp"; break;
					case 4:obj.href="update.asp"; break;
					default: obj.href="#"; break;
				}
			else obj.href="#";
		}
	}
	function GrayOutIP(tp) {
		for(var i=1;i<5;i++){
			document.getElementById('_IP'+i).disabled=tp;
			document.getElementById('_MASK'+i).disabled=tp;
			document.getElementById('_GW'+i).disabled=tp;
			document.getElementById('_DNS'+i).disabled=tp;
		}
		chgbtnst();
	}
	function GrayOutDHCPserv(tp) {
		for(var i=1;i<5;i++){
			document.getElementById('_START'+i).disabled=tp;
			document.getElementById('_END'+i).disabled=tp;
			document.getElementById('_SERVGW'+i).disabled=tp;
		}
		chgbtnst();
	}
	function INPVALUE_CHECK(obj, md){
		if(md) {
			if(obj.value > 223) obj.value = 223;
			else if(obj.value == 127) obj.value = 1;
		}
		else if(obj.value > 255) obj.value = 255;
	}
	function KEYCODE_CHECK(event) {
		var kcode = event.keyCode;
		if(event.shiftKey) {window.event.returnValue=false; return;}
		if(((kcode > 47) && (kcode < 58)) || ((kcode > 95) && (kcode < 106))) return;
		else {
			switch(kcode) {
				case 8: //--Backspace
				case 37: //--Left
				case 39: //--Right
				case 46: //--Del
				case 110: //--Del(number)
					break;
				case 9: //--Tab
				case 190: //--point
					window.event.keyCode=9;
					break;
				default: window.event.returnValue=false;
					break;
			}
			return;
		}
  }
</script>
</head>
<body bgcolor="#494949" onload="window_onload()">
	<table width="560" border="0" cellpadding="0" cellspacing="0">
		<form name="networkform" method="post" action="/form/network_cgi">
		<input id="netapply" name="netbtn" value="apply" type="submit" style="display:none">
		<input id="wifi_mode" name="wifi" value="<%network_value("WIFI_MODE");%>" type="text" style="display:none">
		<tr><td width="40" height="15"></td><td width="25"></td>
				<td width="390"></td><td width="105"></td>
		</tr>
		<tr><td width="40" height="25" align="center"><img src="image/arrayb.gif"></td>
				<td colspan="3" height="25"><a href="general.asp" onclick="confst(this,1)"><%network_items("LINK_GENERAL");%></a></td>
		</tr>
		<tr><td width="40" height="30" align="center"><img src="image/array.gif"></td>
				<td class="non_ln" colspan="3" height="24"><%network_items("LINK_NETWORK");%></td>
		</tr>
<!--Main Content-->
		<tr><td height="30" colspan="4" class="net1">
					<%network_items("NETWORK_MSG1");%>
				</td>
		</tr>
		<tr><td height="30" colspan="4" class="net1">
					<%network_items("NETWORK_MSG2");%>
					&nbsp<input type="text" name="SSID" id="_SSID" value="<%network_value("SSID");%>" maxlength="32" size="11" style="height:20px;" onblur="chgbtnst()">
				</td>
		</tr>
		<tr><td height="30" colspan="4" class="net1">
					<%network_items("NETWORK_MSG3");%>
					&nbsp<select id="_channel" name="channel" style="height:23px;" onchange="chgbtnst()">
          	<option value="0" <%network_value("chann0");%>><%network_items("NETWORK_AUTO");%></option>
            <option value="1" <%network_value("chann1");%>>1</option>
            <option value="2" <%network_value("chann2");%>>2</option>
            <option value="3" <%network_value("chann3");%>>3</option>
            <option value="4" <%network_value("chann4");%>>4</option>
            <option value="5" <%network_value("chann5");%>>5</option>
            <option value="6" <%network_value("chann6");%>>6</option>
            <option value="7" <%network_value("chann7");%>>7</option>
            <option value="8" <%network_value("chann8");%>>8</option>
            <option value="9" <%network_value("chann9");%>>9</option>
            <option value="10" <%network_value("chann10");%>>10</option>
            <option value="11" <%network_value("chann11");%>>11</option>
          </select>
				</td>
		</tr>
		<tr><td height="30" colspan="4" class="net1">
					<input type="radio" name="dhcp" id="dhcp_on" value="1" onclick="GrayOutIP(1)" <%network_value("dhcp_on");%>>
					<%network_items("NETWORK_MSG4");%>
				</td>
		</tr>
		<tr><td height="30" colspan="4" class="net1">
					<input type="radio" name="dhcp" id="dhcp_off" value="0" onclick="GrayOutIP(0)" <%network_value("dhcp_off");%>>
					<%network_items("NETWORK_MSG5");%>
				</td>
		</tr>
		<tr><td height="150" colspan="4" class="net1">
					<table>
						<tr><td width="230" height="30"><%network_items("NETWORK_MSG6");%></td>
								<td width="330" height="30"><%network_items("NETWORK_MSG7");%></td>
						</tr>
						<tr><td colspan="4" height="26">
									<table width="560">
										<tr><td width="66"><%network_items("NETWORK_IP");%></td>
												<td width="140" align="center">
													<div class="addr">
														<input type="text" class="addr_inp" name="IP1" id="_IP1" maxlength="3" value="<%network_value("IP_1");%>" onkeydown="KEYCODE_CHECK(event)" onkeyup="INPVALUE_CHECK(this,1)" onblur="chgbtnst()">
															<span>.</span>
														<input type="text" class="addr_inp" name="IP2" id="_IP2" maxlength="3" value="<%network_value("IP_2");%>" onkeydown="KEYCODE_CHECK(event)" onkeyup="INPVALUE_CHECK(this,0)" onblur="chgbtnst()">
															<span>.</span>
														<input type="text" class="addr_inp" name="IP3" id="_IP3" maxlength="3" value="<%network_value("IP_3");%>" onkeydown="KEYCODE_CHECK(event)" onkeyup="INPVALUE_CHECK(this,0)" onblur="chgbtnst()">
															<span>.</span>
														<input type="text" class="addr_inp" name="IP4" id="_IP4" maxlength="3" value="<%network_value("IP_4");%>" onkeydown="KEYCODE_CHECK(event)" onkeyup="INPVALUE_CHECK(this,0)" onblur="chgbtnst()">
													</div>
												</td>
												<td colspan="2">
													<input type="radio" name="dhcp_serv" id="serv_auto" value="1" onclick="GrayOutDHCPserv(1)" <%network_value("serv_auto");%>>  <%network_items("NETWORK_AUTO");%>
													<input type="radio" name="dhcp_serv" id="serv_off" value="0" style="padding-left:15px;" onclick="GrayOutDHCPserv(0)" <%network_value("serv_off");%>>  <%network_items("NETWORK_OFF");%> 
												</td>
										</tr>
                  </table>
								</td>
						</tr>
						<tr><td colspan="4" height="26">
									<table width="560">
										<tr><td width="62"><%network_items("NETWORK_MASK");%></td>
												<td width="140" align="center">
													<div class="addr">
														<input type="text" class="addr_inp" name="MASK1" id="_MASK1" maxlength="3" value="<%network_value("MASK_1");%>" onkeydown="KEYCODE_CHECK(event)" onkeyup="INPVALUE_CHECK(this,0)" onblur="chgbtnst()">
															<span>.</span>
														<input type="text" class="addr_inp" name="MASK2" id="_MASK2" maxlength="3" value="<%network_value("MASK_2");%>" onkeydown="KEYCODE_CHECK(event)" onkeyup="INPVALUE_CHECK(this,0)" onblur="chgbtnst()">
															<span>.</span>
														<input type="text" class="addr_inp" name="MASK3" id="_MASK3" maxlength="3" value="<%network_value("MASK_3");%>" onkeydown="KEYCODE_CHECK(event)" onkeyup="INPVALUE_CHECK(this,0)" onblur="chgbtnst()">
															<span>.</span>
														<input type="text" class="addr_inp" name="MASK4" id="_MASK4" maxlength="3" value="<%network_value("MASK_4");%>" onkeydown="KEYCODE_CHECK(event)" onkeyup="INPVALUE_CHECK(this,0)" onblur="chgbtnst()">
													</div>
												</td>
												<td width="60"><%network_items("NETWORK_MSG8");%></td>
												<td width="273" align="left">
													<div class="addr" align="center">
														<input type="text" class="addr_inp" name="START1" id="_START1" maxlength="3" value="<%network_value("STARTIP_1");%>" onkeydown="KEYCODE_CHECK(event)" onkeyup="INPVALUE_CHECK(this,1)" onblur="chgbtnst()">
															<span>.</span>
														<input type="text" class="addr_inp" name="START2" id="_START2" maxlength="3" value="<%network_value("STARTIP_2");%>" onkeydown="KEYCODE_CHECK(event)" onkeyup="INPVALUE_CHECK(this,0)" onblur="chgbtnst()">
															<span>.</span>
														<input type="text" class="addr_inp" name="START3" id="_START3" maxlength="3" value="<%network_value("STARTIP_3");%>" onkeydown="KEYCODE_CHECK(event)" onkeyup="INPVALUE_CHECK(this,0)" onblur="chgbtnst()">
															<span>.</span>
														<input type="text" class="addr_inp" name="START4" id="_START4" maxlength="3" value="<%network_value("STARTIP_4");%>" onkeydown="KEYCODE_CHECK(event)" onkeyup="INPVALUE_CHECK(this,0)" onblur="chgbtnst()">
													</div>
												</td>
										</tr>
                  </table>
								</td>
						</tr>
						<tr><td colspan="4" height="26">
									<table width="560">
										<tr><td width="62"><%network_items("NETWORK_GW");%></td>
												<td width="140" align="center">
													<div class="addr">
														<input type="text" class="addr_inp" name="GW1" id="_GW1" maxlength="3" value="<%network_value("GW_1");%>" onkeydown="KEYCODE_CHECK(event)" onkeyup="INPVALUE_CHECK(this,1)" onblur="chgbtnst()">
															<span>.</span>
														<input type="text" class="addr_inp" name="GW2" id="_GW2" maxlength="3" value="<%network_value("GW_2");%>" onkeydown="KEYCODE_CHECK(event)" onkeyup="INPVALUE_CHECK(this,0)" onblur="chgbtnst()">
															<span>.</span>
														<input type="text" class="addr_inp" name="GW3" id="_GW3" maxlength="3" value="<%network_value("GW_3");%>" onkeydown="KEYCODE_CHECK(event)" onkeyup="INPVALUE_CHECK(this,0)" onblur="chgbtnst()">
															<span>.</span>
														<input type="text" class="addr_inp" name="GW4" id="_GW4" maxlength="3" value="<%network_value("GW_4");%>" onkeydown="KEYCODE_CHECK(event)" onkeyup="INPVALUE_CHECK(this,0)">
													</div>
												</td>
												<td width="60"><%network_items("NETWORK_MSG9");%></td>
												<td width="273" align="left">
													<div class="addr" align="center">
														<input type="text" class="addr_inp" name="END1" id="_END1" maxlength="3" value="<%network_value("ENDIP_1");%>" onkeydown="KEYCODE_CHECK(event)" onkeyup="INPVALUE_CHECK(this,1)" onblur="chgbtnst()">
															<span>.</span>
														<input type="text" class="addr_inp" name="END2" id="_END2" maxlength="3" value="<%network_value("ENDIP_2");%>" onkeydown="KEYCODE_CHECK(event)" onkeyup="INPVALUE_CHECK(this,0)" onblur="chgbtnst()">
															<span>.</span>
														<input type="text" class="addr_inp" name="END3" id="_END3" maxlength="3" value="<%network_value("ENDIP_3");%>" onkeydown="KEYCODE_CHECK(event)" onkeyup="INPVALUE_CHECK(this,0)" onblur="chgbtnst()">
															<span>.</span>
														<input type="text" class="addr_inp" name="END4" id="_END4" maxlength="3" value="<%network_value("ENDIP_4");%>" onkeydown="KEYCODE_CHECK(event)" onkeyup="INPVALUE_CHECK(this,0)" onblur="chgbtnst()">
													</div>
												</td>
										</tr>
                  </table>
								</td>
						</tr>
						<tr><td colspan="4" height="26">
									<table width="560">
										<tr><td width="62"><%network_items("NETWORK_DNS");%></td>
												<td width="140" align="center">
													<div class="addr">
														<input type="text" class="addr_inp" name="DNS1" id="_DNS1" maxlength="3" value="<%network_value("DNS_1");%>" onkeydown="KEYCODE_CHECK(event)" onkeyup="INPVALUE_CHECK(this,1)" onblur="chgbtnst()">
															<span>.</span>
														<input type="text" class="addr_inp" name="DNS2" id="_DNS2" maxlength="3" value="<%network_value("DNS_2");%>" onkeydown="KEYCODE_CHECK(event)" onkeyup="INPVALUE_CHECK(this,0)" onblur="chgbtnst()">
															<span>.</span>
														<input type="text" class="addr_inp" name="DNS3" id="_DNS3" maxlength="3" value="<%network_value("DNS_3");%>" onkeydown="KEYCODE_CHECK(event)" onkeyup="INPVALUE_CHECK(this,0)" onblur="chgbtnst()">
															<span>.</span>
														<input type="text" class="addr_inp" name="DNS4" id="_DNS4" maxlength="3" value="<%network_value("DNS_4");%>" onkeydown="KEYCODE_CHECK(event)" onkeyup="INPVALUE_CHECK(this,0)" onblur="chgbtnst()">
													</div>
												</td>
												<td width="60"><%network_items("NETWORK_GW");%></td>
												<td width="273" align="left">
													<div class="addr" align="center">
														<input type="text" class="addr_inp" name="SERVGW1" id="_SERVGW1" maxlength="3" value="<%network_value("SERVGW_1");%>" onkeydown="KEYCODE_CHECK(event)" onkeyup="INPVALUE_CHECK(this,1)" onblur="chgbtnst()">
															<span>.</span>
														<input type="text" class="addr_inp" name="SERVGW2" id="_SERVGW2" maxlength="3" value="<%network_value("SERVGW_2");%>" onkeydown="KEYCODE_CHECK(event)" onkeyup="INPVALUE_CHECK(this,0)" onblur="chgbtnst()">
															<span>.</span>
														<input type="text" class="addr_inp" name="SERVGW3" id="_SERVGW3" maxlength="3" value="<%network_value("SERVGW_3");%>" onkeydown="KEYCODE_CHECK(event)" onkeyup="INPVALUE_CHECK(this,0)" onblur="chgbtnst()">
															<span>.</span>
														<input type="text" class="addr_inp" name="SERVGW4" id="_SERVGW4" maxlength="3" value="<%network_value("SERVGW_4");%>" onkeydown="KEYCODE_CHECK(event)" onkeyup="INPVALUE_CHECK(this,0)" onblur="chgbtnst()">
													</div>
												</td>
										</tr>
                  </table>
								</td>
						</tr>
					</table>
				</td>
		</tr>
		<tr><td colspan="3" height="32" align="right" valign="top">
				<img id="apply" src="image/btn_apply4.bmp" onmouseover="chgPIC(this,1)" onmouseout="chgPIC(this,0)" onclick="sendnet()">
			</td>
			<td width="105"></td>
		</tr>
<!--Main Content-->
		<tr><td width="40" height="25" align="center"><img src="image/arrayb.gif"></td>
				<td colspan="3" height="25"><a href="security.asp" onclick="confst(this,2)"><%network_items("LINK_SECURITY");%></a></td>
		</tr>
		<tr><td width="40" height="25" align="center"><img src="image/arrayb.gif"></td>
				<td colspan="3" height="25"><a href="password.asp" onclick="confst(this,3)"><%network_items("LINK_PASSWORD");%></a></td>
		</tr>
		<tr><td width="40" height="25" align="center"><img src="image/arrayb.gif"></td>
				<td colspan="3" height="25"><a href="update.asp" onclick="confst(this,4)"><%network_items("LINK_FWUPDATE");%></a></td>
		</tr>
	</from>
	</table>
</body>
</html>