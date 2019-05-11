/**
*		@author   hmzhou
*		@version 1.0
* 		@date: 2011/04/11 
*	<pre> 
*  medialib Api Class : 
*	Access to Actions Micro's WebMail
*  	
*/
//////code examples
/**

var username:String = "zhangsan@163.com"
var password:String = "test"
var server:String="imap.163.com";
var ssl:Number = 0;
var port:Number = WebMail.IMAP_PORT;

WebMail.set_login(username,password,server,ssl,port);
WebMail.set_workpath("/mnt/udisk");
WebMail.set_attachment_filter("swf");//filter for "swf"

WebMail.start_connect();


....


//maybe in a loop,query connect status

if(WebMail.get_connect_status() == WebMail.WEBMAIL_CONNECT_OK)
{
	var total:Number = WebMail.get_mailnumbers();
	
	//fetch attachment of total mails
	WebMail.start_fetch_attachment(0,total);

}

**/


dynamic class Actions.WebMail {

/**
*@addtogroup WebMail_as
*@{
**/
	/* default imap/imaps port number */
	public static var IMAP_PORT=143;					///< port for imap
	public static var IMAP_SSL_PORT=993;				///<port for imap with ssl
	/* webmail connect status */
	public static var WEBMAIL_CONNECT_OK=0;				///<status connect OK
	public static var WEBMAIL_CONNECTING=1;				///<status connecting
	public static var WEBMAIL_CONNECT_ERROR=-1;			///<status connect error
	/* webmail fetch mail status */
	public static var WEBMAIL_FETCH_FINISHED=0;			///<status fetch finished
	public static var WEBMAIL_FETCHING=1;				///<status fetch finished
	public static var WEBMAIL_FETCH_ERROR=-1;			///<status fetch error
	public static var WEBMAIL_FETCH_STOP=-21;			///<status fetch stopped
	/**
	@brief call this function to set the email addr and password
	@param[in] username	: email addr
	@param[in] password	: password
	@param[in] server	: imap server address e.g. "imap.163.com"
	@param[in] ssl: with ssl or not(1:Yes,2:No)
	@param[in] port : port for imap/imaps,if not specialized, use IMAP_PORT/IMAPS_PORT for default.
	@return
		- 0 	: success
		- -1	: failed
	**/
	public static function set_login(username:String,password:String,server:String,ssl:Number,port:Number):Number
	{
		return ExternalInterface.call("webmail_set_login",username,password,server,ssl,port);
	}


	/**
	@brief call this function to set webmail work path,mainly for save the attachment.
	@param[in] path: webmail work path,e.g. /mnt/udisk

	@return
		- 0 	: success
		- -1	: failed
	**/
	public static function set_workpath(path:String):Number
	{
		return ExternalInterface.call("webmail_set_workpath",path);
	}


	/**
	@brief call this function to set filename's extension of webmail attachment to download.
	@param[in] filter: filename's extension of webmail attachment,e.g. "swf", "jpg bmp jpeg"

	@return
		Void
	**/
	public static function set_attachment_filter(filter:String):Number
	{
		return ExternalInterface.call("webmail_set_attachment_filter",filter);
	}

	/**
	@brief call this function to start to connect webmail server in the background.
	@param[in] :Void

	@return
		Void
	**/
	public static function start_connect():Void
	{
		ExternalInterface.call("webmail_start_connect");
	}

	/**
	@brief call this function to get total mail numbers of webmail server.
	@param[in] :Void

	@return
		- total mail numbers 
	**/
	public static function get_mailnumbers():Number
	{
		return ExternalInterface.call("webmail_get_mailnumbers");
	}

	/**
	@brief call this function to get mail ID for index message.
	@param[in] index:	message index to get mail ID	

	@return
		- mail ID for index message.
		
	**/
	public static function get_mail_id(index : Number ):Number
	{
		return ExternalInterface.call("webmail_get_mail_id",index);
	}


	/**
	@brief call this function to start fetch mail attachment task,'count 'mails from 'start_index'.
	@param[in] start_index:	mail start index
				count : total mails

	@return
		Void
		
	**/
	public static function start_fetch_attachment(start_index:Number,count:Number):Void
	{
		ExternalInterface.call("webmail_start_fetch_attachment",start_index,count);
	}

		
	/**
	@brief call this function to get webmail connect status.
	@param[in] Void	

	@return
		WEBMAIL_CONNECT_OK:connect ok
		WEBMAIL_CONNECTING: connecting
		WEBMAIL_CONNECT_ERROR: connect error
	**/
	
	public static function get_connect_status():Number
	{
		return ExternalInterface.call("webmail_get_connect_status");
	}
	
	
	/**
	@brief call this function to get  webmail fetch status.
	@param[in] Void	

	@return
	
		low 16 bit of return value indicates fetch status,
		WEBMAIL_FETCH_OK:fetch ok
		WEBMAIL_FETCHING: fetching
		WEBMAIL_FETCH_ERROR: fetch error
		WEBMAIL_FETCH_STOP: fetch stopped
		
		high 16 bit of return value indicates current fetching mail index when 
		fetch status is WEBMAIL_FETCHING
	**/
	
	public static function get_fetch_status():Number
	{
		return ExternalInterface.call("webmail_get_fetch_status");
	}
	
	
	/**
	@brief call this function to stop fetching mail attachment.
	@param[in] Void	

	@return
		void
		
	**/
	public static function stop_fetch():Void
	{
		ExternalInterface.call("webmail_stop_fetch");
	}
	
	/**
	@brief call this function to exit webmail,release memory.
	@param[in] Void	

	@return
		void
		
	**/
	public static function exit():Void
	{
		ExternalInterface.call("webmail_exit");
	}
	
	
/**
 *@}
 */	

}


