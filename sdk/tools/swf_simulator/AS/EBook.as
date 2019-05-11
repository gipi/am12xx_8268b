
/**
 *	@author   wuxiaowen
 *	@version 1.0
 *  @date: 2009/06/08
 *	<pre>  
 *  EBook Engine Class:
 	    Play .txt etc. file
 *	Example:
 *		EBook.Open("c:\\a.txt",80);
 *		if(EBook.GetStatus() == EB_READY){
 *			total = EBook.GetTotalRows();
 *			bookmark = EBook.GetBookmark();
 *			for(i = 0; i < 20 && i < total - bookmark; i++){
 *				this["line"+i].text = EBook.GetRow(i+bookmark);
 *			}
 *		}
 *		EBook.SetBookmark(bookmark);
 *		EBook.Close();
 *	</pre>
 */

 dynamic class Actions.EBook {
	 
	/**ebook status: already to be read*/
	public static var EB_READY=0;
	/**ebook status: busy in processing*/
	public static var EB_BUSY=1;
	 
	/**
	 *Open one EBook
	 *@param 
	 path ebook file path
	 *@param 
	 width characters per line
	 *@return 
	 true if succsee or false
	 */
	public static function Open(path:String, width:Number):Boolean
	{
		return ExternalInterface.call("eb_Open",path,width);
	}
	
	/**
	 *close opened ebook
	 */
	public static function Close()
	{
		return ExternalInterface.call("eb_Close");
	}
	
	/**
	 * get total lines if ready or zero if still in processing
	 *@see EB_BUSY
	 *@return 
	 total lines
	 */
	public static function GetTotalRows():Number
	{
		return ExternalInterface.call("eb_GetTotalRows");
	}
	
	/**
	 *get one row
	 *@param 
	 	row the row numeber
	 *@return 
	 	string of the row
	 */
	public static function GetRow(row:Number):String
	{
		return ExternalInterface.call("eb_GetRow",row);
	}
	
	/**
	 *get current ebook status
	 *@return
	  the status
	 *@see EB_READY
	 *@see EB_BUSY
	 */
	public static function GetStatus():Number
	{
		return ExternalInterface.call("eb_GetStatus");
	}
	
	/**
	 *get the bookmark position
	 *@return 
	 	the row number of bookmark
	 */
	public static function GetBookmark():Number
	{
		return ExternalInterface.call("eb_GetBookmark");
	}
	
	/**
	 *set bookmark
	 *@param 
	 row the row to be set bookmark
	 *@return 
	 true if success or false
	 */
	public static function SetBookmark(row:Number):Number
	{
		return ExternalInterface.call("eb_SetBookmark",row);
	}
	
	/**
	 *get the foreground color
	 *@return 
	 fore ground color with 0xRRGGBB format
	 */
	public static function GetColor():Number
	{
		return ExternalInterface.call("eb_GetColor");
	}
	
	/**
	 *set fore ground color
	 *@param 
	 color format is 0xRRGGBB
	 *@return 
	true if success or false
	 */
	public static function SetColor(color:Number):Number
	{
		return ExternalInterface.call("eb_SetColor",color);
	}
	
	/**
	 *get the background color
	 *@return 
	 fore ground color with 0xRRGGBB format
	 */
	public static function GetBGColor():Number
	{
		return ExternalInterface.call("eb_GetBGColor");
	}
	
	/**
	 *set background color
	 *@param 
	 color format is 0xRRGGBB
	 *@return 
	true if success or false
	 */
	public static function SetBGColor(color:Number):Number
	{
		return ExternalInterface.call("eb_SetBGColor",color);
	}
	
	/**
	 *get font size
	 *@return 
	 font size
	 */
	public static function GetFontSize():Number
	{
		return ExternalInterface.call("eb_GetColor");
	}
	
	/**
	 *set font size
	 *@param 
	 size font size to be set
	 *@return 
	true if success or false
	 */
	public static function SetFontSize(size:Number):Number
	{
		return ExternalInterface.call("eb_SetFontSize",size);
	}
 }