/**
*		@author   sllin
*		@version 1.0
* 		@date: 2010/09/06 
*	<pre> 
*  Alarm Api Class : 
*	Access to Actions Micro's Alarm
*  
*/
/*************************
	addAlarm();
	num = getAlarmInfo(ALARM_CMD_TOTNUM,0);
	for(i=0;i<num;i++){
		ID = getAlarmInfo(ALARM_CMD_IDXTOID,i);
		time = getAlarmInfo(ALARM_CMD_TIME,ID);
		timeset= 12*60+50 // set time to 12:50
		setAlarm(ALARM_CMD_TIME,ID,timeset,NULL);
		setAlarm(ALARM_CMD_NAME,ID,0,"wakeup");
		storeInfo();
	}
	closeAlarm(ID);
	delAlarm(ID);
*****************/

dynamic class Actions.Alarm {

/**
*@addtogroup Alarm_as
*@{
*/

	/**The following CMD used in setAlarm and getAlarmInfo function**/
	public static var ALARM_CMD_ENABLE 		=0;///<enable alarm
	public static var ALARM_CMD_DAY			=1;///<the days to be noticed
	public static var ALARM_CMD_SNOOZE		=2;///<the interval time 
	public static var ALARM_CMD_TIME			=3;///<the time to be set
	public static var ALARM_CMD_USERID		=4;///<user can used the value to do someting
	public static var ALARM_CMD_NAME			=5;///<the name of a alarm

	/**The following CMD used in getAlarmInfo function**/
	public static var ALARM_CMD_TOTNUM		=7; ///<get the total num of alarms in the system
	public static var ALARM_CMD_IDXTOID		=8;///<change the index to the id of the alarm
		

	/**
	*@brief this fuction is called for adding  a new alarm
	*@param[in] NULL
	*@return
		- -1		: add alarm failed 
		- >=0	: the ID of new Alarm, this value will be used in other functions
	**/
	public static function addAlarm():Number
	{
		return ExternalInterface.call("al_addAlarm");
	}
	
	/**
	*@brief this function is called for deleting specified alarm
	*@param[in] ID	: the ID of the alarm which will be deleted
	*@return
		- -1		: failed
		- 0		: succeed
	**/
	public static function delAlarm(ID:Number):Number
	{
		return ExternalInterface.call("al_delAlarm");
	}

	/**
	*@brief this function is called for setting the parameters of a alarm
	*@param[in] cmd	: ALARM_CMD_ENABLE etc
	*@param[in] ID	: the ID of a alarm, each alarm is different
	*@param[in] value	: \n
		- if cmd==ALARM_CMD_ENABLE 
			value=0:the alarm is disable =1:the alarm is enable
		- if cmd==ALARM_CMD_DAY 
			each day has one bit,  0000111 means monday tuesday and wednesday
		- if cmd==ALARM_CMD_SNOOZE 
			value==0~7: snooze time==0,5,10,15,20,25,30,60
		- if cmd==ALARM_CMD_TIME 
			if you want to set the time to 11:20,value==11*60+20(min)
		- if cmd==ALARM_CMD_USERID 
			value is a value of 1Byte
		- if cmd==ALARM_CMD_NAME 
			value is ignored
	*@param[in] name	:
		- if cmd==ALARM_CMD_NAME
			name is the name of the specified alarm, the length of name is less than 11 Bytes
	*@return
		- -1:faied
		- 0:succeed
	**/
	public static function setAlarm(cmd:Number,ID:Number,value:Number,name:String):Number
	{
		return ExternalInterface.call("al_setAlarm",cmd,ID,value,name);
	}

	/**
	*@brief call this function for getting the information of alarms in system
	*@param[in] cmd : ALARM_CMD_TOTNUM,ALARM_CMD_IDXTOID,ALARM_CMD_ENABLE~ALARM_CMD_NAME
		- if cmd!=ALARM_CMD_NAME
			the user must change the return value to a number
	*@param[in] idx	:
		- if cmd ==ALARM_CMD_TOTNUM
			this value is not used
		- if cmd==ALARM_CMD_IDXTOID
			the idx = 0~  getAlarmInfo(ALARM_CMD_TOTNUM,0);
		- if cmd !=ALARM_CMD_IDXTOID,ALARM_CMD_TOTNUM
			the idx is ID value return form getAlarmInfo(ALARM_CMD_IDXTOID,i);
	*@return value	:
		- if cmd==ALARM_CMD_TOTNUM \n
			return the number of alarms in the system
		- if cmd==ALARM_CMD_ENABLE \n
			return=0:the alarm is disable,=1:the alarm is enable
		- if cmd==ALARM_CMD_DAY \n
			each day has one bit,  0000111 means monday tuesday and wednesday
		- if cmd==ALARM_CMD_SNOOZE \n
			return==0~7: snooze time==0,5,10,15,20,25,30,60
		- if cmd==ALARM_CMD_TIME \n
			if you set the time to 11:20
			return==11*60+20(min)
		- if cmd==ALARM_CMD_USERID \n
			return the value user defined
		- if cmd==ALARM_CMD_NAME \n
			name of a alarm
	*@see ALARM_CMD_ENABLE etc
	**/
	public static function getAlarmInfo(cmd:Number,idx:Number):String
	{
		return ExternalInterface.call("al_getAlarmInfo",cmd,idx);
	}

	/**
	*@brief call this function for closing the specified alarm
	*@param[in] ID	: the id  of the alarm to be closed
	*@return  none
	**/
	public static function closeAlarm(ID:Number):Void
	{
		ExternalInterface.call("al_closeAlarm",ID);
	}

	/**
	*@brief call this function for opening the specified alarm
	*@param[in] ID	: the id  of the alarm to be closed
	*@return NULL
	**/
	public static function openAlarm(ID:Number):Void
	{
		ExternalInterface.call("al_openAlarm",ID);
	}
	
	/**
	*@brief call this function for loading the information of alams
	*@param[in] none
	*@return  none
	**/
	public static function loadInfo():Void
	{
		ExternalInterface.call("al_loadInfo");
	}

	/**
	*@brief call this function for storing the information of alams
	*@param[in]  none
	*@return none
	**/
	public static function storeInfo():Void
	{
		ExternalInterface.call("al_storeInfo");
	}


	/**
	*@brief call this function for getting ID of the alarm which is occurred recently
	*@param[in] none
	*@return
		- -1		: failed
		- others	: the ID of the alarm
	**/
	public static function getAlarmOnID():Number
	{
		return ExternalInterface.call("al_getAlarmOnID");
	}

/**
 *@}
 */	

}
