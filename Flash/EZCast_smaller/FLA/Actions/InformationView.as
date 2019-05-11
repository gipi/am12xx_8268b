

import mx.utils.Delegate;
import Actions.LineMenuSet;


/**
 *	@author   Charles
 *	@version 0.1
 *    @date: 2013/05/30
 *	@description  

 *    InformationView Class

 buttonLayout, eg:
 var menuLayout:Array = [
	{
		tag: 0,	//mark 
		valid: 1, //display 1, undisplay 0
		id: "menu1", //flash lib name
		name: "WIFI",//show at textfiel
		nameMc: "name_txt", //text mc name
		selectDo: mainmenuSelectDo,
		unSelectDo: mainmenuUnSelectDo,
		enterDo: mainmenuEnterDo,
		escDo: mainmenuEscDo,
		callBack: mainmenuCallBack,
		mcAtt: {_x: 15, _y: 280},
	},
];
menuSet = new LineMenuSet(thisObj, menuLayout, 0, 6, 1, 3);

 */

dynamic class Actions.InformationView extends Object{
	public var infoMc:MovieClip;
	public var lineMenuObj:LineMenuSet;
	public var timerId:Number;

	static public var mStageWidth:Number = 1280;//use your flash true stage width
	static public var mStageHeight:Number = 800;//use your flash true stage heigth
	static public var informationMcName:String = "InfomationView";

	
	public function InformationView(attachMc:MovieClip, 
		idBkLib:String, 
		bkMcAtt:Object,
		titleStr:String,
		msShow:Number, //-1 aways
		buttonLayout:Array,
		selectIndex:Number,
		totalButton:Number,
		lineTotal:Number,
		rankTotal:Number,
		otherObj:Object)
	{

		timerId = 0;

		infoViewTrace("Init");
		if(attachMc != undefined && idBkLib != undefined){
			if(bkMcAtt == undefined){
				infoViewTrace("bkMcAtt is null!");
				infoViewTrace("Stage height == " + Stage.height + ", width == " + Stage.width);
				
				infoMc = attachMc.attachMovie(idBkLib, informationMcName, attachMc.getNextHighestDepth());
				infoMc._x = (mStageWidth-infoMc._width)/2;//(Stage.width-infoMc._width)/2;
				if(infoMc._x<0) infoMc._x = 0;
			
				infoMc._y = (mStageHeight-infoMc._height)/2-20;//(Stage.height-infoMc._height)/2;
				if(infoMc._y<0) infoMc._y = 0;
				infoViewTrace("infoMc x == " + infoMc._x + ", y == " + infoMc._y);
			}else{
				infoMc = attachMc.attachMovie(idBkLib, informationMcName, attachMc.getNextHighestDepth(), bkMcAtt);
			}
			
			infoMc.title_txt.text = titleStr;
		}else{
			infoViewTrace("error attachMc || idBkLib is null!");
			return ;
		}

		
		if(buttonLayout.length > 0){
			infoViewTrace("init buttonLayout!");
			lineMenuObj = new LineMenuSet(thisObj, buttonLayout, selectIndex, totalButton, lineTotal, rankTotal);
		}

		if(msShow > 0){
			//timerId = setInterval(Delegate.create(this, stopInformationView), msShow);
			infoMc._count = 0;
			infoMc._maxCount = Math.floor(msShow/40);//Frame 40ms
			infoMc.onEnterFrame = Delegate.create(this, frameStopInformationView);

			infoViewTrace("maxCount == " + infoMc._maxCount);
			infoViewTrace("init show timer, and del info wait!timerId = " + timerId);
		}

		
	}





	public function stopInformationView()
	{

		infoViewTrace("stop!");
		lineMenuObj.deleteLineMenuSet();
		
		if(timerId > 0){
			clearInterval(timerId);
			timerId = 0;
		}

		infoMc.removeMovieClip();
	}



	private function frameStopInformationView()
	{

		if(infoMc._maxCount > 0){
			infoMc._count++;
			if(infoMc._count < infoMc._maxCount){
				return ;
			}else{
				infoMc._count = 0;
				infoMc._maxCount = 0;
				delete infoMc.onEnterFrame;
			}
		}
		
		infoViewTrace("frameStopInformationView");
		stopInformationView();
	}

	
	private function infoViewTrace(str:String)
	{
		trace("[InformationView]" + str);
	}
	

}
