
/**
 *	@author   wuxiaowen
 *	@version 1.0
 *  @date: 2009/09/07
 *	@description  
 *  Locale compatible with mx.lang.Locale
 *  <pre>Example：
 *	Locale.setLoadCallback(localeListener);
 *	Locale.loadLanguageXML("zh-CN");
 *	function localeListener(success:Boolean):Void {
 *	   if (success) {
 *	       greeting_txt.text = Locale.loadString("IDS_GREETING");
 *	   } else {
 *	       greeting_txt.text = "unable to load language XML file.";
 *	   }
 *	}
 *	</pre>
 */

import flash.external.ExternalInterface;
 
class Actions.Locale {

	private static var defaultLang:String;
	private static var currentLang:String;
	private static var callback:Function;

	/**
	 * Return the default language code.
	 *@return
	 	default language code
	 */
	static function getDefaultLang():String {
		return defaultLang;
	}

	/**
	 * Set the default language code.
	 *@param : default language code
	 */
	static function setDefaultLang(langCode:String):Void {
		defaultLang = langCode;
	}

	/**
	 * Set the callback function that will be called after the xml file is loaded.
	 *@param 
	 	loadCallback callback function
	 */
	static function setLoadCallback(loadCallback:Function) {
		callback = loadCallback;
	}

	/**
	 * Return the string value associated with the given string id in the current language.
	 *@param
	 	id string id in the current language
	 *@return
	 	string value
	 */
	static function loadString(id:String):String {
		if(currentLang == ""){
			currentLang = defaultLang;
			loadLanguageXML(currentLang,null);
		}
		return String(ExternalInterface.call("locale_loadString",id));
	}

	/**
	 * Load the specified language xml file.
	 *@param
	 	xmlLanguageCode language code
	 *@param
		customXmlCompleteCallback callback function
	 */
	static function loadLanguageXML(xmlLanguageCode:String, customXmlCompleteCallback:Function):Void {
		var ret:Boolean = Boolean(ExternalInterface.call("locale_loadLanguage",xmlLanguageCode));
		if(ret){
			currentLang = xmlLanguageCode;
		}
		if (customXmlCompleteCallback) {
			callback = customXmlCompleteCallback;
		}
		if(callback){
			callback(ret);
		}
	}
}
