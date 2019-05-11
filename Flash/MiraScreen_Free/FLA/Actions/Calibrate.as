
/**
 *	@author   Kewen
 *	@version 1.0
 *  @date: 2010/08/26
 *	<pre>   
 *  Calibrate Class:
 		for touch panel calirate
 *	Example:
 *		Calibrate.Open();
 *		Calibrate.GetSamplePoint();
 *		Calibrate.SetCalirateData();
 *		Calibrate.Close();
 *	</pre>
 */



 dynamic class Actions.Calibrate {

/**
*@addtogroup Calibrate_as
*@{
*/
	 
	/**first sample point num*/
	public static var POINT_N=0; ///<first sample point num
	
	 
	/**
	*@brief  open Calibrate
	*@return 
		- true if success
		- false if fail
	*/
	public static function Open():Number
	{
		return ExternalInterface.call("cl_Open");
	}
	
	/**
	*@brief close Calirate
	*/
	public static function Close()
	{
		return ExternalInterface.call("cl_Close");
	}
	
	/**
	*@brief  get one valid sample point
	*@return 
		- true if success
		- false if fail
	*/
	public static function GetSamplePoint(istep:Number)
	{
		return ExternalInterface.call("cl_GetSamplePoint",istep);
	}
	
	/**
	*@brief Set new calibrate data to /etc/pointercal
	*@return 
		- true if success
		- false if failed
	*/
	public static function SetCalibrateData():Number
	{
		return ExternalInterface.call("cl_SetCalibrateData");
	}
	
/**
 *@}
 */		
 }
 