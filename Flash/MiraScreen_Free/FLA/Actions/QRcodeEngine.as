
dynamic class Actions.QRcodeEngine {

	/**
	* @brief open the QR code generator.
	* @return 
    - 0 : failed
    - 1 : success
	*/
	public static function open():Number
	{
		return ExternalInterface.call("qrcode_open");
	}
	
	/**
	* @brief close the QR code generator.
	*/
	public static function close():Void
	{
		ExternalInterface.call("qrcode_close");
	}	
	
	/**
	* @brief do QR code encode.
	* @param 
    - str : the string to be encoded.
	* @return 
    - 0 : failed
    - 1 : success
	*/
	public static function encode(str:String):Number
	{
		return ExternalInterface.call("qrcode_encode",str);
	}
	
	/**
	* @brief get the QR code width.
	* @return the width of the code.
	*/
	public static function getWidth():Number
	{
		return ExternalInterface.call("qrcode_get_width");
	}
	
	/**
	* @brief get the QR code value via position.
	* @param 
    - x : x position relative to QR code area.
    - y : y position relative to QR code area.
	* @return 
    - 0 : indicate white
    - 1 : indicate black
	*/
	public static function getValueByPosition(x:Number,y:Number):Number
	{
		return ExternalInterface.call("qrcode_get_value_by_pos",x,y);
	}
	
	/**
	* @brief Generate jpeg file for QR code.
	* @param 
    - filename : Full path of the jpeg file.
	* @return 
    - 0 : failed
    - 1 : success
	*/
	public static function generateBitmap(filename:String):Number
	{
		return ExternalInterface.call("qrcode_generate_bitmap",filename);
	}
}
