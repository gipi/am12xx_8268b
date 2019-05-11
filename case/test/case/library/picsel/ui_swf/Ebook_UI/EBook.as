
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

/**
*@addtogroup EBook_as
*@{
*/
	public static var EB_READY=0;				///< ebook status : already to be read
	public static var EB_BUSY=1;				///< ebook status : busy in processing
	public static var BOOK_INFO_MARKPAGE=0;		///< ebook mark : current page cmd
	public static var BOOK_INFO_TOTALPAGE=1;	///< ebook mark : total page cmd

	public static var EB_FIT_WIDTH=0;		///< fit screen width
	public static var EB_FIT_HEIGHT=1;		///< fit screen height
	public static var EB_FIT_SCREEN=2;		///< fit screen size

	/**
	 *@brief	Open one EBook
	 *@param[in] path	: ebook file path
	 *@param[in] width	: characters per line
	 *@param[in] page	: number that the offset will be stored. used for quick searching
	 *@return 
	 *		true if succsee or false if fail
	 */
	public static function Open(path:String, width:Number,pagesavestep:Number,maxline:Number):Boolean
	{
		return ExternalInterface.call("eb_Open",path,width,pagesavestep,maxline);
	}
	
	/**
	 *@brief	close opened ebook
	 *@param[in] NULL
	 *@return NULL
	 */
	public static function Close()
	{
		return ExternalInterface.call("eb_Close");
	}
	
	/**
	 *@brief	get total lines if ready or zero if still in processing
	 *@param[in] NULL
	 *@return 
	 *		total lines
	 */
	public static function GetTotalRows():Number
	{
		return ExternalInterface.call("eb_GetTotalRows");
	}
	
	/**
	 *@brief	get one row
	 *@param[in] row	: the row numeber
	 *@return 
	 *		string of the row
	 */
	public static function GetRow(row:Number):String
	{
		return ExternalInterface.call("eb_GetRow",row);
	}
	
	/**
	 *@brief	get current ebook status
	 *@param[in] NULL
	 *@return the status
	 *			- EB_READY
	 *			- EB_BUSY
	 */
	public static function GetStatus():Number
	{
		return ExternalInterface.call("eb_GetStatus");
	}
	
	/**
	 *@brief	get the bookmark position
	 *@param[in] NULL
	 *@return 
	 *		the row number of bookmark
	 */
	public static function GetBookmark():Number
	{
		return ExternalInterface.call("eb_GetBookmark");
	}
	
	/**
	 *@brief	set bookmark
	 *@param[in] row	: the row to be set bookmark
	 *@return 
	 *		true if success or false if fail
	 */
	public static function SetBookmark(row:Number):Number
	{
		return ExternalInterface.call("eb_SetBookmark",row);
	}

	/**
	 *@brief	get the filename
	 *@param[in] NULL
	 *@return 
	 *		the name of bookmark file
	 */
	public static function GetFileName():String
	{
		return ExternalInterface.call("eb_GetFileName");
	}
	
	/**
	 *@brief	set filename
	 *@param[in] name	: name of bookmark file
	 *@return 
	 *		true if success or false if fail
	 */
	public static function SetFileName(str:String):Number
	{
		return ExternalInterface.call("eb_SetFileName",str);
	}
	
	/**
	 *@brief	get the foreground color
	 *@param[in] NULL
	 *@return 
	 *		fore ground color with 0xRRGGBB format
	 */
	public static function GetColor():Number
	{
		return ExternalInterface.call("eb_GetColor");
	}
	
	/**
	 *@brief	set fore ground color
	 *@param[in] color	: format is 0xRRGGBB
	 *@return 
	 *		true if success or false if fail
	 */
	public static function SetColor(color:Number):Number
	{
		return ExternalInterface.call("eb_SetColor",color);
	}
	
	/**
	 *@brief	get the background color
	 *@param[in] NULL
	 *@return 
	 *		fore ground color with 0xRRGGBB format
	 */
	public static function GetBGColor():Number
	{
		return ExternalInterface.call("eb_GetBGColor");
	}
	
	/**
	 *@brief	set background color
	 *@param[in] color	: format is 0xRRGGBB
	 *@return 
	 *		true if success or false if fail
	 */
	public static function SetBGColor(color:Number):Number
	{
		return ExternalInterface.call("eb_SetBGColor",color);
	}
	
	/**
	 *@brief	get font size
	 *@param[in] NULL
	 *@return 
	 *		font size
	 */
	public static function GetFontSize():Number
	{
		return ExternalInterface.call("eb_GetFontSize");
	}
	
	/**
	 *@brief	set font size
	 *@param[in] size	: font size to be set
	 *@return 
	 *		true if success or false if fail
	 */
	public static function SetFontSize(size:Number):Number
	{
		return ExternalInterface.call("eb_SetFontSize",size);
	}

	/**
	 *@brief	store  the information of the book that opened into Vram
	 *@param[in] index	: the index of array less than EBOOK_MARK_MAX
	 *@return
	 * - 1: succeed
	 * - 0: flase
	*/
	public static function SaveBookInfo(index:Number):Number
	{
		return ExternalInterface.call("eb_SaveBookInfo",index);
	}

	/**
	 *@brief	get the information of the book that store in the vram
	 *@param[in] index	: the index of array less than EBOOK_MARK_MAX
	 *@param[in] cmd		: BOOK_INFO_MARKPAGE or BOOK_INFO_TOTALPAGE
	 *@return 
	 *		the information associated with cmd
	 */
	
	public static function GetBookInfo(index:Number,cmd:Number):Number
	{
		return ExternalInterface.call("eb_GetBookInfo",index,cmd);
	}

	
	/**
	 *@brief	get the name of the book that store in the vram
	 *@param[in] index	: the index of array less than EBOOK_MARK_MAX
	 *@return 
	 *		the name of the book
	 */
	public static function GetBookMarkName(index:Number):String
	{
		return ExternalInterface.call("eb_GetBookMarkName",index);
	}

	/**
	 *@brief	open pdf/doc/xls/ppt document
	 *@param[in] path	: pdf/doc/xls/ppt file path
	 *@return NULL
	 */
	public static function OpenDocument(path:String):Number
	{
		return ExternalInterface.call("eb_open_document",path);
	}

	/**
	 *@brief	close pdf/doc/xls/ppt document
	 *@param[in] NULL
	 *@return NULL
	 */
	public static function CloseDocument():Number
	{
		return ExternalInterface.call("eb_close_document");
	}

	/**
	 *@brief	jump to the pointed page
	 *@param[in] no	: the page number to jump to
	 *@return NULL
	 */
	public static function JumpPage(no:Number):Number
	{
		return ExternalInterface.call("eb_jump_page",no);
	}

	/**
	 *@brief	zoom in the current page
	 *@param[in] NULL
	 *@return NULL
	 */
	public static function ZoomIn():Number
	{
		return ExternalInterface.call("eb_zomm_in");
	}

	/**
	 *@brief	zoom out the current page
	 *@param[in] NULL
	 *@return NULL
	 */
	public static function ZoomOut():Number
	{
		return ExternalInterface.call("eb_zoom_out");
	}

	/**
	 *@brief	zoom according to scale
	 *@param[in] scale	: scale to zoom
	 *@return NULL
	 */
	public static function Zoom(scale:Number):Number
	{
		return ExternalInterface.call("eb_zoom",scale);
	}

	/**
	 *@brief	get the pagetotal of the file
	 *@param[in] NULL
	 *@return 
	 *		the number of the pages
	 */
	public static function GetPageTotal():Number
	{
		return ExternalInterface.call("eb_get_page_total");
	}

	/**
	 *@brief	get the current page number
	 *@param[in] NULL
	 *@return 
	 *		the number of the current page
	 */
	public static function GetPageNo():Number
	{
		return ExternalInterface.call("eb_get_page_no");
	}

	/**
	 *@brief	move the current page
	 *@param[in] x	: the horizontal offset to move
	 *@param[in] y	: the vertical offset to move
	 *@return NULL
	 */
	public static function Move(x:Number,y:Number):Number
	{
		return ExternalInterface.call("eb_move",x,y);
	}

	/**
	 *@brief	judge whether outlines exist or not in the file
	 *@param[in] NULL
	 *@return 
	 * - 0	: outlines exist
	 * - 1	: outlines not exist
	 */
	public static function OutlineExist():Number
	{
		return ExternalInterface.call("eb_outline_exist");
	}

	/**
	 *@brief	get the size of outlines
	 *@param[in] NULL
	 *@return 
	 *		the size of outlines
	 */
	public static function OutlineSize():Number
	{
		return ExternalInterface.call("eb_outline_size");
	}

	/**
	 *@brief	get the content of the pointed outline
	 *@param[in] index	: the index of outline to get content
	 *@return 
	 *		string	: the content of the pointed outline
	 */
	public static function GetOutline(index:Number):String
	{
		return ExternalInterface.call("eb_get_outline",index);
	}

	/**
	 *@brief	goto the page the outline point to
	 *@param[in] index	: the index of outline to goto
	 *@return 
	 * - 0	: success
	 * - !0	: fail
	 */
	public static function GotoOutlinePage(index:Number):Number
	{
		return ExternalInterface.call("eb_goto_outline_page",index);
	}

	/**
	 *@brief	rotate the page
	 *@param[in] NULL
	 *@return 
	 * - 0	: success
	 * - !0	: fail
	 */
	public static function Rotate():Number
	{
		return ExternalInterface.call("eb_rotate");
	}

	/**
	 *@brief	change the page show mode
	 *@param[in] mode : show mode(EB_FIT_WIDTH=0;EB_FIT_HEIGHT=1;EB_FIT_SCREEN=2;)
	 *@return 
	 * - 0	: success
	 * - !0	: fail
	 */
	public static function ShowMode(mode:Number):Number
	{
		return ExternalInterface.call("eb_showmode",mode);
	}

	/**
	 *@brief	goto the prev page
	 *@param[in] NULL
	 *@return 
	 * - 0	: success
	 * - !0	: fail
	 */
	public static function PrevPage():Number
	{
		return ExternalInterface.call("eb_prevpage");
	}

	/**
	 *@brief	goto the next page
	 *@param[in] NULL
	 *@return 
	 * - 0	: success
	 * - !0	: fail
	 */
	public static function NextPage():Number
	{
		return ExternalInterface.call("eb_nextpage");
	}

	/**
	 *@brief	get zoom size
	 *@param[in] NULL
	 *@return 
	 *	the size of zoom
	 */
	public static function GetZoomSize():Number
	{
		return ExternalInterface.call("eb_getzoomsize");
	}
/**
 *@}
 */
 }