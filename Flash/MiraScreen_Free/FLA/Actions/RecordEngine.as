/**
*	@author   zhuzhenfang
*	@version 1.0
* 	@date: 2010/11/09  
*	<pre> 
*   Audio Record Api Class : 
*		Access to Actions Micro's RecordEngine
*	Example:
*		RecordEngine.Open();
*		RecordEngine.SetRate(800);
*		RecordEngine.SetGain(5);
*		RecordEngine.StartRecord();
*		RecordEngine.StopRecord();
*		RecordEngine.Close();
*	</pre>
*  
*/

dynamic class Actions.RecordEngine {

/**
*@addtogroup RecordEngine_as
*@{
*/

	public static var REC_IDLE=0;			///< play status:idle
	public static var REC__RECORDING=1;		///< play status:recording
	public static var REC_PAUSE=2;			///< play status:pause
	public static var REC_STOPPED=3;		///< play status:stop
	public static var REC_ERROR=4;			///< play status:error

	/**
	 *@brief  open RecordEngine
	 *@param[in] NULL
	 *@return 
	 * - 0		success
	 * - -1		fail
	 */
	public static function Open():Number
	{
		return ExternalInterface.call("ar_Open");
	}


	/**
	*@brief		the function will set record rate 
	*@param[in] sample_rate : 8000,11025,12000,16000,22050,24000,32000,44100,48000,96000
	*@return 
	* - 0		success
	* - -1		fail
	*/
	public static function SetRate(sample_rate:Number):Number
	{
		return ExternalInterface.call("ar_Set_Rate",sample_rate);
	}

	/**
	*@brief		the function will set record gain
	*@param[in] gain : 0~7
	*@return 
	* - 0		success
	* - -1		fail
	*/
	public static function SetGain(gain:Number):Number
	{
		return ExternalInterface.call("ar_Set_Gain",gain);
	}

	/**
	*@brief		the function will set record time
	*@param[in] record_time : total record time second
	*@return 
	* - 0		success
	* - -1		fail
	*/
	public static function SetRecordTime(record_time:Number):Number
	{
		return ExternalInterface.call("ar_Set_Record_Time",record_time);
	}
	
	/**
	*@brief		the function will set record filename
	*@param[in] record_filename : record filename to set
	*@return 
	* - 0		success
	* - -1		fail
	*/
	public static function SetRecordFilename(record_filename:String):Number
	{
		return ExternalInterface.call("ar_Set_Record_Filename",record_filename);
	}

	/**
	*@brief		the function will start record
	*@param[in] NULL
	*@return 
	* - 0		success
	* - -1		fail
	*/
	public static function StartRecord():Number
	{
		return ExternalInterface.call("ar_Start_Record");
	}

	/**
	*@brief		the function will get record status
	*@param[in]:NULL
	*@return 
	* - REC_IDLE		0
	* - REC__RECORDING	1
	* - REC_PAUSE		2
	* - REC_STOPPED		3
	* - REC_ERROR		4
	*/
	public static function GetRecordStatus():Number
	{
		return ExternalInterface.call("ar_Get_Record_Status");
	}

	/**
	*@brief		the function will pause record
	*@param[in] NULL
	*@return 
	* - 0		success
	* - -1		fail
	*/
	public static function PauseRecord():Number
	{
		return ExternalInterface.call("ar_Pause_Record");
	}

	/**
	*@brief		the function will resume record
	*@param[in] NULL
	*@return
	* - 0		success
	* - -1		fail
	*/
	public static function ResumeRecord():Number
	{
		return ExternalInterface.call("ar_Resume_Record");
	}

	/**
	*@brief		the function will stop record
	*@param[in] NULL
	*@return 
	* - 0		success
	* - -1		fail
	*/
	public static function StopRecord():Number
	{
		return ExternalInterface.call("ar_Stop_Record");
	}

	/**
	*@brief		the function will close RecordEngine
	*@param[in] NULL
	*@return 
	* - 0		success
	* - -1		fail
	*/
	public static function Close():Number
	{
		return ExternalInterface.call("ar_Close");
	}

	/**
	*@brief		the function will get recording current time
	*@param[in] NULL
	*@return 
	* - recording current time second
	*/
	public static function Get_Recording_Time():Number
	{
		return ExternalInterface.call("ar_Get_Recording_Time");
	}

/**
 *@}
 */
}