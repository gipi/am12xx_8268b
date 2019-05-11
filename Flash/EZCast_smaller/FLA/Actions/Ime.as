/**
 *	@author   wangwei
 *	@version 1.0
 *  @date: 2011/01/21
 *	<pre>  
 *  IME support
 *	</pre>
 */
dynamic class Actions.Ime
{

	public static var HIT_INPUTTEXT=0;
	public static var HIT_OUTSIDE=1;
	public static var ESC_BY_USER=2;
	public static var IME_STATE_CHANGED=0xff;
	
	 /**
	@brief get ime state
	@param[in]  none
	@return
	 	- 0:press one input text
	 	- 1:press area outside of input text and input method
	 	- 2:press esc key
	 	
	 */
	public static function getImeMessage():Number
	{
		return ExternalInterface.call("flash_GetImeMessage");
	}
	 /**
	@brief set input method name
	@param[in]  swfname  :swf name
	@return
	 	- 1:succeed
	 	- 0:falied
	 */	 
	public static function setImeSwfName(swfname:String):Number
	{
		return ExternalInterface.call("flash_SetImeSwfName", swfname);
	}
	
	 /**
	@brief enable or disable ime    set x and y range
	@param[in]  state  	 :0  -disable  1-enable
	@param[in]  xmin 		 :x min
	@param[in]  ymin 		 :y min
	@param[in]  xmax  	 :y max
	@param[in]  ymax 		 :y max
	@return
	 	- 1:succeed
	 	- 0:falied
	 */	
	public static function setImeState(state:Number, xmin:Number,ymin :Number, xmax:Number, ymax:Number):Number
	{
		return ExternalInterface.call("flash_SetImeState",state, xmin, ymin,xmax, ymax);
	}	
	 /**
	@brief set key range 
	@param[in] keymin		:min  key 
	@param[in] keymax	:max key 
	@return none
	 */
	 
	public static function setImeKeyRange(keymin:Number, keymax:Number)
	{
		return ExternalInterface.call("flash_SetImeKeyRange", keymin, keymax);
	}
	
	 /**
	@brief send key message
	@param[in] key		: key value
	@return none
	 */
	public static function sendMessageFromIme(key:Number)
	{
		return ExternalInterface.call("flash_SendMessageFromIME", key);
	}
	
	 /**
	@brief send message
	@param[in] keytype		: assic
	@param[in] str			: srting
	@return none
	 */
	public static function putImeMsg(keytype:Number, str:String)
	{
		return ExternalInterface.call("flash_putIMEMsg", keytype, str);
	}
 }
 