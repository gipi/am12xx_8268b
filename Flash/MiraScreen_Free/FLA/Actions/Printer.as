/**
*		@author   sllin
*		@version 1.0
* 		@date: 2010/03/24  
*	<pre> 
*  Printer Api Class : 
*	Access to Actions Micro's Printer
*  
*/

dynamic class Actions.Printer {

/**
*@addtogroup MediaLib_as
*@{
*/

/**Media Type*/
	public static var PT_MT_NANDDISK						=0;		///<nand disk
	public static var PT_MT_CARDDISK						=1;		///<SD MMC card
	public static var PT_MT_HARDDISK						=2;		///<CF card
	public static var PT_MT_USBDISK						=3;		///<U disk

	
	/**
	@brief the function will start printing photo 
	@param[in] picpath	: path of the photo which is to be printed
	@param[in] copies 		: number of copies to be printed
	@param[in] mediatype	: PT_MT_NANDDISK etc.
	@return
		- 0 failed
		- 1 succeed
	*/
	public static function startPrinterJob(picpath:String,copies:Number,mediatype:Number):Number
	{
		return ExternalInterface.call("pt_startPrinterJob",picpath,copies,mediatype);
	}


	/**
	@brief the function will continue printing photo 
	@param[in] none
	@return
		- 0 failed
		- 1 succeed
	*/
	public static function continuePrinterJob():Number
	{
		return ExternalInterface.call("pt_continuePrinterJob");
	}

	/**
	@brief the function will abort printing photo 
	@param[in] none
	@return
		- 0 failed
		- 1 succeed
	*/
	public static function abortPrinterJob():Number
	{
		return ExternalInterface.call("pt_abortPrinterJob");
	}

	/**
	@brief check whether printer is ready
	@param[in] none
	@return
		- 0 failed
		- 1 succeed
	*/
	public static function isPrinterReady():Number
	{
		return ExternalInterface.call("pt_isPrinterReady");
	}

/**
 *@}
 */	

}