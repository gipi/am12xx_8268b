
/**
 *	@author   chenshouhui
 *	@version 1.0
 *  @date	: 2012/05/22
 *	<pre>  
 *  lanEngine 
 		lan setting control
 *	</pre>
 *  <pre>
 	Example:
 *		LanEngine.Open();	
 *		when exit:
 *		LanEngine.Close();
 *	</pre>
 */

 dynamic class Actions.LanEngine 
 {
	/**
	 *@brief	open LanEngine
	 *@param[in] NULL
	 *@return NULL
	 */
	
	public static function Open():Number
	{
		return ExternalInterface.call("lan_Open");
	}
	
	/**
	 *@brief	close LanEngine
	 *@param[in] NULL
	 *@return NULL
	 */
	public static function Close()
	{
		ExternalInterface.call("lan_Close");
	}
	/**
	 *@brief	rmmod KO
	 *@param[in] NULL
	 *@return NULL
	 */
	public static function Stop()
	{
		ExternalInterface.call("lan_Stop");
	}
	/**
	 *@brief	open LanEngine as user setting
	 *@param[in] NULL
	 *@return NULL
	 */
	public static function lan_openmanual(ip:String, mask:String, gateway:String,first_dns_server:String,second_dns_server:String):Number
	{
		return ExternalInterface.call("lan_openmanual",ip,mask,gateway,first_dns_server,second_dns_server);
	}
	public static function lan_ip_for_display():String
	{
		return ExternalInterface.call("lan_ip_display");
	}
	public static function lan_mask_for_display():String
	{
		return ExternalInterface.call("lan_mask_display");
	}
	public static function lan_gateway_for_display():String
	{
		return ExternalInterface.call("lan_gateway_display");
	}
	public static function lan_firstdns_for_display():String
	{
		return ExternalInterface.call("lan_firstdns_display");
	}
	public static function lan_seconddns_for_display():String
	{
		return ExternalInterface.call("lan_seconddns_display");
	}
	public static function ip_and_mask_judge(ip1:Number,ip2:Number,ip3:Number,ip4:Number,mask1:Number,mask2:Number,mask3:Number,mask4:Number):Number
	{
		return ExternalInterface.call("ip_maskmatchjudge",ip1,ip2,ip3,ip4,mask1,mask2,mask3,mask4);
	}
	public static function lan_get_access_mode():Number
	{
		return ExternalInterface.call("lan_acess_mode");
	}
	public static function ip_and_gateway_match(ip1:Number,ip2:Number,ip3:Number,ip4:Number,gateway1:Number,gateway2:Number,gateway3:Number,gateway4:Number):Number
	{
		return ExternalInterface.call("ip_gatewaymatch",ip1,ip2,ip3,ip4,gateway1,gateway2,gateway3,gateway4);
	}
	
	public static function EZCastProAPI_lan_mac_for_display():String
	{
		return ExternalInterface.call("lan_mac_display");
	}
	
	public static function EZCastProAPI_net_config_reload(ctl:Number):String//0->client 1->LAN
	{
		return ExternalInterface.call("ezcastpro_net_config_reload",ctl);
	}
	
	public static function EZCastProAPI_net_config_save(ctl:Number,str:String):Number//0->client 1->LAN
	{
		return ExternalInterface.call("ezcastpro_net_config_save",ctl,str);
	}
	
	public static function getLanAutomaticEnable():Number
	{
		return ExternalInterface.call("lan_getLanAutomaticEnable");
	}
	public static function setLanAutomaticEnable(val:Number)
	{
		ExternalInterface.call("lan_setLanAutomaticEnable", val);
	}
	
	public static var LANSETTING_IP:Number 			= 0;
	public static var LANSETTING_GATEWAY:Number 	= 1;
	public static var LANSETTING_MASK:Number 		= 2;
	public static var LANSETTING_DNS1:Number 		= 3;
	public static var LANSETTING_DNS2:Number 		= 4;
	public static function getLanSettingVal(flag:Number):String
	{
		return ExternalInterface.call("lan_getLanSettingVal", flag);
	}
	public static function StoreLanManualInfo(ip:String, mask:String, gateway:String, dns1:String, dns2:String)
	{
		ExternalInterface.call("lan_storeLanManualInfo", ip, mask, gateway, dns1, dns2);
	}
	public static function getMacAddress():String
	{
		return ExternalInterface.call("lan_getMacAddress");
	}
}
