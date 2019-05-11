import Actions.EBook;
import Actions.FileList;
import Actions.Locale;
import Actions.SystemInfo;

//全局使用
var currentPath = MainSWF.currentPath ? MainSWF.currentPath : "/mnt/udisk/";
var Ebook_mc = overallBar_mc;
/**
*****************************************
*Init language
*****************************************
*/
Locale.setLoadCallback(localeCallback); 
function localeCallback(success:Boolean) {
     if (success) { 
		//first-level menu string.
		fullBar_mc.AutoTimeShow_txt.text = Locale.loadString("IDS_EBOOK_AUTOTIME"); 
		fullBar_mc.Pages_txt.text = Locale.loadString("IDS_EBOOK_PAGE");
		overallBar_mc.listBar_mc.Directory_txt.text = Locale.loadString("IDS_EBOOK_DIRECTORY");
		overallBar_mc.Modulename_txt.text = Locale.loadString("IDS_EBOOK_NAME");
	 } else {
		//trace("unable to load lan!");
     }
}

#include "Picsel_fileList_Ebook.as"
/**
*****************************************
*Load language 
*****************************************/
var lang = SystemInfo.getCurLanguage();
if(lang==0){
	Locale.loadLanguageXML("en"); 
	//trace("en");
}else if(lang==1){
	Locale.loadLanguageXML("ja"); 
	//trace("ja");
}

//====================全屏显示===============================
var BOOKMARKINDEX:Number = 0;
var PAGEJUMP_1:Number = 1;
var pageJump:Number = PAGEJUMP_1;
var pageRowNum:Number = 1;        	//当前页索引
var perPageRowNum:Number = 14;  	//每页显示的行数
var totalPageNum:Number;         	//总页数
var totalRow:Number;             	//总行数        
var currentRow:Number;				//当前行
var FULL_PAGE_SAVE:Number = 10;     //每FULL_PAGE_SAVE页将会保存一次偏移量
var currentMCId:Number = 0;        
var Ebook_play_mc = fullBar_mc;
var autoEbookInterval:Number = 5000;
var autoEbookID:Number;
var autoEbookIng:Boolean = false;
var FULLSHOWLINEWIDTH:Number = 12;//36
Ebook_play_mc.key = new Object();

function Ebook_play_mc_setEbookMag(magMC:MovieClip):Void{
	//trace("Ebook_play_mc_setEbookMag");
	//trace("totalRow="+totalRow);
	//trace("pageRowNum="+pageRowNum);
	for(var i:Number = 0; i < perPageRowNum; i++){
		row = (pageRowNum-1)*perPageRowNum + i;
		if(row < totalRow-1){
			currentRow = row;
			magMC["line"+i+"_txt"].text = EBook.GetRow(row);
			//trace("TXT=="+magMC["line"+i+"_txt"].text);
		}else{
			magMC["line"+i+"_txt"].text = ""
		}
	}

}

function Ebook_play_mc_init(str:String):Void{
	EBook.Close();
//	trace("line0.txt="+Ebook_play_mc.mags_mc.mag1_mc.line0_txt._width);
	EBook.Open(str,Ebook_play_mc.mags_mc.mag1_mc.line0_txt._width-FULLSHOWLINEWIDTH,FULL_PAGE_SAVE,perPageRowNum);
	//trace("TxtWidth="+Ebook_play_mc.mags_mc.mag1_mc.line0_txt._width-FULLSHOWLINEWIDTH);
	//trace("SaveMark=="+EBook.GetBookMarkName(BOOKMARKINDEX));
	Ebook_play_mc.AutoTime_txt.text = autoEbookInterval/1000;
	Ebook_play_mc.Pagejump_txt.text = pageJump;
	Ebook_play_mc.transition_mc._visible = true;
	Ebook_play_mc.showtxt_mc.onEnterFrame = function(){
		if(EBook.GetStatus() == EBook.EB_READY){
			//delete Ebook_play_mc.onEnterFrame;
			totalRow = EBook.GetTotalRows();
			totalPageNum = Math.ceil(EBook.GetTotalRows()/perPageRowNum);
			if(EBook.GetBookMarkName(BOOKMARKINDEX)==str){//之前已经存过该文件
				pageRowNum = EBook.GetBookInfo(BOOKMARKINDEX,EBook.BOOK_INFO_MARKPAGE);
				//trace("PageRowNum get==="+pageRowNum);
			}else
			{
				pageRowNum = Math.ceil(1/perPageRowNum);
			}
			//trace("pagesave=="+EBook.GetBookInfo(BOOKMARKINDEX,EBook.BOOK_INFO_MARKPAGE));
			//trace("pageRowNum="+pageRowNum);
			Ebook_play_mc.pageMag_txt.text = pageRowNum+"/"+totalPageNum;
			Ebook_play_mc.title_txt.text = String(nameArray[rollOverNum]);
			if(currentMCId == 0)
			{
				//trace("11--currentMCId = " + currentMCId);
				Ebook_play_mc_setEbookMag(Ebook_play_mc.mags_mc.mag0_mc);
			}
			else if(currentMCId == 1)
			{
				//trace("22--currentMCId = " + currentMCId);
				Ebook_play_mc_setEbookMag(Ebook_play_mc.mags_mc.mag1_mc);
			}
			//Ebook_play_mc_setEbookMag(Ebook_play_mc.mags_mc.mag0_mc);
			Ebook_play_mc.transition_mc._visible = false;
			delete this.onEnterFrame;
		}
	}
}

function showPageContent(endNum:Number):Void{
	//trace("Call ShowPageContent");
	Ebook_play_mcMove(Ebook_play_mc.mags_mc["mag"+currentMCId+"_mc"],endNum,"_y");
	if(currentMCId==0){
		currentMCId = 1;
		Ebook_play_mc_setEbookMag(Ebook_play_mc.mags_mc.mag1_mc);
		Ebook_play_mc.mags_mc.mag1_mc._y = -endNum;
		Ebook_play_mcMove(Ebook_play_mc.mags_mc.mag1_mc,0,"_y");
	}else{
		currentMCId = 0;
		Ebook_play_mc_setEbookMag(Ebook_play_mc.mags_mc.mag0_mc);
		Ebook_play_mc.mags_mc.mag0_mc._y = -endNum;
		Ebook_play_mcMove(Ebook_play_mc.mags_mc.mag0_mc,0,"_y");
	}
	//trace("Call ShowPageContent");
}
function Ebook_play_mcMove(mc:MovieClip,endNum:Number,pro:String):Void{
	mc.onEnterFrame = function(){
		this[pro] += (endNum - this[pro])/2;
		if(Math.abs(endNum - this[pro])<1){
			this[pro] = endNum;
			delete this.onEnterFrame;
		}
	}
}

function autoEbook():Void{
	///全屏自动翻页功能
	if(pageRowNum<totalPageNum){
		pageRowNum++;
		Ebook_play_mc.pageMag_txt.text = pageRowNum+"/"+totalPageNum;
		showPageContent(Ebook_play_mc.mask_mc._height);
	}else {
		Ebook_play_mc.autoplay_mc.gotoAndStop(1);
		clearInterval(autoEbookID);
	}
}

Ebook_play_mc.key.onKeyDown = function() {
	//全屏阅读时的Key响应
	//trace("========Ebook_play_mc.key======");
}

Ebook_play_mc.mags_mc.setMask(Ebook_play_mc.mask_mc);
Ebook_play_mc.transition_mc._visible = false;
Ebook_play_mc._visible = false;
//========================预览显示区域=============================
MainSWF.backMenu = false;
var Ebook_pageRowNum:Number = 1;
var Ebook_perPageRowNum:Number = 12;
var Ebook_totalPageNum:Number;
var Ebook_totalRow:Number; 		
var Ebook_currentRow:Number;
var Ebook_currentMCId:Number = 0;
var Ebook_attachName:String = "item";




Ebook_mc.BookBox_mc._visible = false;
Ebook_mc.BookBox_mc.stop();

function playbarsListener():Void{
	Ebook_mc.playbar_mc._alpha = 100;
	Ebook_mc.listBar_mc._alpha = 50;
	Key.removeListener(Ebook_mc.listBar_mc.content_mc.key);
	Key.addListener(Ebook_mc.playbar_mc.key);
}

function showRollOver():Void{
}

function listBarBack():Void{
	//返回主界面
	Key.removeListener(Ebook_mc.listBar_mc.content_mc.key);
	if(filelistHandle != -1){
		FileList.releasePathNew(filelistHandle);
		filelistHandle = -1;
	}
	EBook.Close();
	//trace("EBook.Close!");
	MainSWF.loadswf("mainmenu.swf");
}

Ebook_mc.playbar_mc.mags_mc.setMask(Ebook_mc.playbar_mc.mask_mc);
Ebook_mc.playbar_mc.transition_mc._visible = false;
function Ebook_setEbookMag(magMC:MovieClip):Void{
	//trace("Ebook_totalRow="+Ebook_totalRow);
	for(var i:Number = 0; i < Ebook_perPageRowNum; i++){
		//注意这里传进去的row为0表示第一行，GetRow函数会对row再进行处理
		row = (Ebook_pageRowNum-1)*Ebook_perPageRowNum + i; 
		if(row < Ebook_totalRow-1){
			Ebook_currentRow = row;
			magMC["line"+i+"_txt"].text = EBook.GetRow(row);
		}else{
			magMC["line"+i+"_txt"].text = ""
		}
	}
}

function Ebook_mcMove(mc:MovieClip,endNum:Number,pro:String):Void{
	//trace("Ebook_mcMove");
	mc.onEnterFrame = function(){
		this[pro] += (endNum - this[pro])/2;
		if(Math.abs(endNum - this[pro])<1){
			this[pro] = endNum;
			delete this.onEnterFrame;
		}
	}
}

Ebook_mc.BookBox_mc.key = new Object();
Ebook_mc.BookBox_mc.key.onKeyDown = function(){
	//trace("======BookBox_mc.key========");
}
function ShowContent_Ebook_mc(endNum:Number):Void{
	Ebook_mcMove(Ebook_mc.playbar_mc.mags_mc["mag"+Ebook_currentMCId+"_mc"],-endNum,"_y");
	//trace("Ebook_currentMCId="+Ebook_currentMCId);
	if(Ebook_currentMCId==0){
		Ebook_currentMCId = 1;
		Ebook_setEbookMag(Ebook_mc.playbar_mc.mags_mc.mag1_mc);
		Ebook_mc.playbar_mc.mags_mc.mag1_mc._y = endNum;
		Ebook_mcMove(Ebook_mc.playbar_mc.mags_mc.mag1_mc,0,"_y");
	}else{
		Ebook_currentMCId = 0;
		Ebook_setEbookMag(Ebook_mc.playbar_mc.mags_mc.mag0_mc);
		Ebook_mc.playbar_mc.mags_mc.mag0_mc._y = endNum;
		Ebook_mcMove(Ebook_mc.playbar_mc.mags_mc.mag0_mc,0,"_y");
	}
}

function initial()
{
	EBook.Open();
	//trace("initial");
}
initial();

Ebook_mc.playbar_mc._alpha=50;
Ebook_mc.playbar_mc.key = new Object();
Ebook_mc.playbar_mc.key.onKeyDown = function() {
	//trace("======palybar_mc.key========");
}
//////////////卡插拔退出调用//////
eventCardChange = function():Void{
	//trace("Card Out====\n");
	FlashEngine.Stop(false);
	MainSWF.ebook_osdid = -1;
	EBook.CloseDocument();
	EBook.Close();
	if(filelistHandle != -1){
		FileList.releasePathNew(filelistHandle);
		filelistHandle = -1;
	}
	MainSWF.loadswf("mainmenu.swf");
}
eventHotKey = function():Void{
	//trace("Hot key====\n");
	FlashEngine.Stop(false);
	MainSWF.ebook_osdid = -1;
	EBook.CloseDocument();
	if(filelistHandle != -1){
		FileList.releasePathNew(filelistHandle);
		filelistHandle = -1;
	}
	EBook.Close();
}
