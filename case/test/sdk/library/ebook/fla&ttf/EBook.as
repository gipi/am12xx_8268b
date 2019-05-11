
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
	/**ebook mark : current page cmd******/
	 public static var BOOK_INFO_MARKPAGE=0;
	 /**ebook mark : total page cmd******/
	 public static var BOOK_INFO_TOTALPAGE=1;
	/**
	 *Open one EBook
	 *@param 
	 path ebook file path
	 *@param 
	 width characters per line
	  *@param 
	 page number that the offset will be stored. used for quick searching
	 *@return 
	 true if succsee or false
	 */
	public static function Open(path:String, width:Number,pagesavestep:Number,maxline:Number):Boolean
	{
		return ExternalInterface.call("eb_Open",path,width,pagesavestep,maxline);
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
	 *get the filename
	 *@return 
	 	the name of bookmark file
	 */
	public static function GetFileName():String
	{
		return ExternalInterface.call("eb_GetFileName");
	}
	
	/**
	 *set filename
	 *@param 
	 name of bookmark file
	 *@return 
	 true if success or false
	 */
	public static function SetFileName(str:String):Number
	{
		return ExternalInterface.call("eb_SetFileName",str);
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

	/**
	store  the information of the book that opened into Vram
	*@param
		index: the index of array less than EBOOK_MARK_MAX
	 @return
	   1: succeed
	   0: flase
	*/
	public static function SaveBookInfo(index:Number):Number
	{
		return ExternalInterface.call("eb_SaveBookInfo",index);
	}

	/**
	get the information of the book that store in the vram
	*@param
		index: the index of array less than EBOOK_MARK_MAX
	 	cmd: BOOK_INFO_MARKPAGE or BOOK_INFO_TOTALPAGE
	  @return:
	  	the information associated with cmd
	*/
	
	public static function GetBookInfo(index:Number,cmd:Number):Number
	{
		return ExternalInterface.call("eb_GetBookInfo",index,cmd);
	}

	
	/**
	get the name of the book that store in the vram
	*@param
		index: the index of array less than EBOOK_MARK_MAX
	  @return:
	  	the name of the book
	*/
	public static function GetBookMarkName(index:Number):String
	{
		return ExternalInterface.call("eb_GetBookMarkName",index);
	}
	public static function LoadDLL(path:String):Number
	{
		return ExternalInterface.call("eb_LoadDLL",path);
	}
	public static function OpenDocument(path:String):Number
	{
		return ExternalInterface.call("eb_open_document",path);
	}
	public static function CloseDocument():Number
	{
		return ExternalInterface.call("eb_close_document");
	}	
	public static function JumpPage(no:Number):Number
	{
		return ExternalInterface.call("eb_jump_page",no);
	}	
	public static function ZoomIn():Number
	{
		return ExternalInterface.call("eb_zomm_in");
	}	
	public static function ZoomOut():Number
	{
		return ExternalInterface.call("eb_zoom_out");
	}
	public static function Zoom(scale:Number):Number
	{
		return ExternalInterface.call("eb_zoom",scale);
	}		
	public static function GetPageTotal():Number
	{
		return ExternalInterface.call("eb_get_page_total");
	}	
	public static function GetPageNo():Number
	{
		return ExternalInterface.call("eb_get_page_no");
	}	
	public static function Move(x:Number,y:Number):Number
	{
		return ExternalInterface.call("eb_move",x,y);
	}				
 }
