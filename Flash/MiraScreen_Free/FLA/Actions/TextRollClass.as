/**
 *	@author   Charles Ly
 *	@version 0.3
 *    @date: 2013/03/13
 */

import mx.utils.Delegate;


dynamic class Actions.TextRollClass extends Object{
	public static var textIntId:Number = 0;
	public static var DISPLAY_TIME:Number = 500; //ms
	public static var MAX_TEXT_ROLL_NUM:Number = 16;

	public static var rollTextArr:Array = [];

	private var id:Number;
	private var curText:Object;
	private var textLen:Number;
	private var textStart:Number;
	private var curString:String;
	private var getText:Function;
	private var getString:Function;


	public function TextRollClass(getTextFunc:Function, getStringFunc:Function, showLength:Number)
	{
		this.getText = getTextFunc;
		this.getString = getStringFunc;
		this.textLen = showLength;

		this.id = addItem(rollTextArr, this);
		
		trace("@@[TextRollClass] id = "  + id);
		
		if(0 == TextRollClass.textIntId){
			TextRollClass.textIntId = setInterval(Delegate.create(this, rollTextHandle), TextRollClass.DISPLAY_TIME);
			trace("textIntId == " + TextRollClass.textIntId);
			trace("TextRollClass.DISPLAY_TIME == " + TextRollClass.DISPLAY_TIME);
		}
		
	}
	
	public function deleteSelfFromRoll()
	{	
		trace("[deleteSelfFromRoll] id = " + id);
		if(id>= 0 && id < rollTextArr.length){
			rollTextArr.splice( id , 1 );
		}
		
		trace("[delTextRoll] rollTextArr len == " + rollTextArr.length);

		if(rollTextArr.length == 0){
			if(TextRollClass.textIntId > 0){
				clearInterval(TextRollClass.textIntId);
				TextRollClass.textIntId = 0;
			}
		}
	}

	public static function deleteRollTimer()
	{

		trace("[deleteRollTimer]");
		if(TextRollClass.textIntId > 0){
			clearInterval(TextRollClass.textIntId);
			TextRollClass.textIntId = 0;
		}
	}

	private static function rollTextHandle()
	{
		var i:Number = 0;
		var obj:Object;
		
		if(rollTextArr.length > 0){

			for(i = 0; i < rollTextArr.length; i++){
				obj = rollTextArr[i];

				if(obj.curText != obj.getText() || obj.curString != obj.getString()){//跳到另一个text
					//更新当前roll的内容
					obj.curString = obj.getString();
					obj.curText = obj.getText();
					obj.textStart = obj.curString.length - obj.textLen;
				}

				if(obj.curString.length <= obj.textLen){
					continue ;
				}
				
				if(obj.textStart < 0){
					obj.textStart = obj.curString.length - obj.textLen;
				}
				
				obj.getText().text = obj.curString.substr(obj.textStart, obj.textLen);
				obj.textStart--;
			}
			
		}
	}


	private static function addItem(arr:Array, item:Object):Number
	{
		if(arr.length >= MAX_TEXT_ROLL_NUM){
			trace("[addItem] Terrible !! the queue is FULL !!");
			return -1;
		}
		arr.push(item);

		return arr.length-1;
	}

}





