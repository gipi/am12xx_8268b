import Actions.FlashEngine;
import Actions.CSwfKey;

///=================文件列表显示==================
var dataArray:Array =[];        	//dataArray数组用来存储文件名和路径
var rollOverNum:Number = 0;   		//一页的第几个文件，0开始    
var itemminH:Number = 40;			//列表Item的高度
var itemmaxH:Number = 80;
var pageNum:Number = 1;         	//当前是第几页，1开始
var perNum:Number = 10;				//文件列表每页显示的文件数
var CurrentIndexNum:Number = 0; 	//用来存储当前选择的文件索引
var PREV_PAGE_SAVE:Number = 10;     //每PREV_PAGE_SAVE将会保存一次偏移量
//初始状态不显示文本
Ebook_mc.playbar_mc.pageMag_txt.text ="";
Ebook_mc.playbar_mc.name_txt.text = "";
////////////////////////////////////////////
Ebook_mc.listBar_mc.content_mc.setMask(listBar_mc.mask_mc);

/*************************************************************
* for dynamic file search, you can copy it to other situations
*************************************************************/

/**
* start search files
*/

var searchCnt:Number=0;
var filelistHandle:Number=-1;
var filelistTotal:Number=0;
var nameArray:Array= new Array(16);
var urlArray:Array=new Array(16);
var lenArray:Number=0;
var filelistHandle:Number=-1;
var filenamenow;
var page_size:Number=100;
var ebookSearchDone:Number=0;
var FrameRate:Number=30;
var page_showmode:Number=0;
MainSWF.ebookfilename="";
MainSWF.ebooknowpage=0;
MainSWF.ebooktotalpage=0;
MainSWF.ebooksize=100;

MainSWF.ebook_osdw = SystemInfo.getCurscreenParam(1);
MainSWF.ebook_osdh = 50;
MainSWF.ebook_osdx0 = 0;
MainSWF.ebook_osdy0 = SystemInfo.getCurscreenParam(2)-MainSWF.ebook_osdh;
MainSWF.ebook_osdid = -1;

function EbookGetCurrentPage()
{
	var idx:Number;
	var page:Number;
	var i:Number;
	
	idx=rollOverNum+(pageNum-1)*perNum;
	page = Math.floor(idx/perNum)+1;
	lenArray=0;
	for(i=0;i<perNum;i++){
		if((i+(pageNum-1)*perNum)>=filelistTotal)
			break;
		nameArray[i]=FileList.getLongnameNew(filelistHandle,i+(pageNum-1)*perNum);
		urlArray[i]=FileList.getPathNew(filelistHandle,i+(pageNum-1)*perNum);
		lenArray++;
	}
}

filelistHandle=FileList.setPathNew(currentPath,"txt pdf doc xls ppt docx xlsx pptx pps ppsx",FileList.FL_MODE_FILES_BROWSE,1);

function EbookCreateFirstPage():Void
{
	var idx:Number;
	var page:Number;
	var i:Number;
	idx=rollOverNum+(pageNum-1)*perNum;
	page = Math.floor(idx/perNum)+1;
	if(page>1){
		return;
	}
	else{
		if(lenArray<perNum){
			if(filelistTotal<= 0)
				Ebook_mc.listBar_mc.current_txt.text = "0";
			else{
				Ebook_mc.listBar_mc.current_txt.text = "1";
				Ebook_mc.listBar_mc.all_txt.text = filelistTotal;
				creatlist(0);
			}
		}
	}
}


Ebook_mc.onEnterFrame = function()
{
	var stat:Number;
	var total:Number;
	if(filelistHandle>=0 && ebookSearchDone==0){
		stat = FileList.getScanTaskStatNew(filelistHandle);
		if(stat==FileList.FL_STAT_FIND_NOFILE){
			filelistTotal=0;
			delete Ebook_mc.onEnterFrame;
		}
		else{
			total = FileList.getTotalNew(filelistHandle);
			if(total!=0){
				filelistTotal=total;
				ebookSearchDone=0;
			}
			EbookCreateFirstPage();
			
		}
		if(stat==FileList.FL_STAT_FIND_COMPLETE){
			filelistTotal=total;
			ebookSearchDone=1;
			EbookCreateFirstPage();
			showRollOver();
			delete this.onEnterFrame;
		}
		if(stat==FileList.FL_STAT_FIND_FIRST_FILE){
			showRollOver();
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
function listBarEnter():Void{
	FlashEngine.Stop(false);
	EBook.OpenDocument(urlArray[rollOverNum]);
	page_size=100;
	MainSWF.ebooksize=page_size;
	MainSWF.ebookfilename = urlArray[rollOverNum];
	Key.removeListener(Ebook_mc.listBar_mc.content_mc.key);
	Key.addListener(menuListener);
	FlashEngine.Play("ebookOSD.swf",true,MainSWF.ebook_osdx0,MainSWF.ebook_osdy0,MainSWF.ebook_osdw,MainSWF.ebook_osdh,FrameRate);
	MainSWF.ebook_osdid = 0;
}

function creatlist(center:Number):Void {
	for (var i:Number = 0; i<Ebook_mc.listBar_mc.content_mc.childnum; i++) {
		Ebook_mc.listBar_mc.content_mc["item"+i].removeMovieClip();
	}
	var itemNum:Number = 0;
	EbookGetCurrentPage();
	for (var i:Number =0; i<lenArray; i++) {
		if(i>filelistTotal-1){
			Ebook_mc.listBar_mc.content_mc.childnum = itemNum;
			return;
		}
		Ebook_mc.listBar_mc.content_mc.attachMovie(Ebook_attachName,"item"+itemNum,itemNum);
		Ebook_mc.listBar_mc.content_mc["item"+itemNum].urlid = i;
		Ebook_mc.listBar_mc.content_mc["item"+itemNum].title_mc.title_txt.text =nameArray[i];
		if(itemNum==0){
			Ebook_mc.listBar_mc.content_mc["item"+itemNum]._y=0;
			Ebook_mc.listBar_mc.content_mc["item"+itemNum].y0 = 0;
			Ebook_mc.listBar_mc.content_mc["item"+itemNum].showY = 0;
		}
		else{
			Ebook_mc.listBar_mc.content_mc["item"+itemNum]._y =Ebook_mc.listBar_mc.content_mc["item"+(itemNum-1)]._y+Ebook_mc.listBar_mc.content_mc["item"+(itemNum-1)]._height+10;
			Ebook_mc.listBar_mc.content_mc["item"+itemNum].y0 = (itemNum+1)*itemminH;
			Ebook_mc.listBar_mc.content_mc["item"+itemNum].showY =itemNum*itemminH;
		}
		if (itemNum == center) {
			Ebook_mc.listBar_mc.content_mc["item"+itemNum].b_mc._height = itemmaxH;
			Ebook_mc.listBar_mc.content_mc["item"+itemNum].line_mc._y = itemmaxH+Ebook_mc.listBar_mc.content_mc["item"+itemNum].line_mc._height;
			Ebook_mc.listBar_mc.content_mc["item"+itemNum].title_mc._y = (Ebook_mc.listBar_mc.content_mc["item"+itemNum].line_mc._y -Ebook_mc.listBar_mc.content_mc["item"+itemNum].title_mc._height)/2;
			Ebook_mc.listBar_mc.content_mc["item"+itemNum].title_mc.gotoAndStop(2);
			Ebook_mc.listBar_mc.content_mc["item"+itemNum].title_mc.title_txt.textColor = 0xFF0000;
		}
		itemNum++;
	}
	Ebook_mc.listBar_mc.content_mc.childnum = itemNum;
}

function showCurrentItem(oldnum:Number):Void{
	itemMove(Ebook_mc.listBar_mc.content_mc["item"+rollOverNum],itemmaxH,Ebook_mc.listBar_mc.content_mc["item"+rollOverNum].showY);
	Ebook_mc.listBar_mc.current_txt.text = rollOverNum+(pageNum-1)*perNum+1;
	Ebook_mc.listBar_mc.content_mc["item"+oldnum].title_mc.gotoAndStop(1);
	Ebook_mc.listBar_mc.content_mc["item"+oldnum].title_mc.title_txt.textColor = 0;
	Ebook_mc.listBar_mc.content_mc["item"+rollOverNum].title_mc.gotoAndStop(2);
	Ebook_mc.listBar_mc.content_mc["item"+rollOverNum].title_mc.title_txt.textColor = 0xFF0000;
	showRollOver();
}

function itemMove(mc:MovieClip, endNum:Number, offsetY:Number):Void {
	mc.onEnterFrame = function() {
		this._y += (offsetY-this._y)*0.9;
		this.b_mc._height += (endNum-this.b_mc._height)*0.9;
		this.line_mc._y = this.b_mc._height-this.line_mc._height;
		this.title_mc._y = (this.b_mc._height-this.title_mc._height)/2;
		if (Math.abs(endNum-this.b_mc._height)<1) {
			this.b_mc._height = endNum;
			this._y = offsetY;
			delete this.onEnterFrame;
		}
	};
}

Ebook_mc.listBar_mc.content_mc.key = new Object();
Ebook_mc.listBar_mc.content_mc.key.onKeyDown = function() {
	trace("listBar_mc.content_mc.key");
	switch (Key.getCode()) {
		case Key.ENTER:
			trace("Key Enter");
			if(filelistTotal <= 0 || (ebookSearchDone==0))
				break; 
			listBarEnter();
			break;
		case Key.LEFT:
			trace("Key LEFT");
			if(filelistTotal <= 0 || (ebookSearchDone==0))
				break;
			playbarsListener();
			break;
		case Key.UP:
			trace("Key UP");
			if(filelistTotal <= 0 || (ebookSearchDone==0))
				break;
			if(rollOverNum>0){
				var oldnum:Number = rollOverNum--;
				itemMove(Ebook_mc.listBar_mc.content_mc["item"+oldnum],itemminH,Ebook_mc.listBar_mc.content_mc["item"+oldnum].y0);
				showCurrentItem(oldnum);
			}else if(pageNum>1){
				pageNum--;
				rollOverNum = perNum-1;
				creatlist(rollOverNum);
				showCurrentItem(rollOverNum);
			}else{
				pageNum = Math.ceil(filelistTotal/perNum);
				rollOverNum = (filelistTotal%perNum)-1;
				if(rollOverNum == -1){
					rollOverNum = perNum - 1;
				}
				creatlist(rollOverNum);
				showCurrentItem(rollOverNum);
			}
			break;
		case Key.DOWN:
			trace("Key DOWN");
			if(filelistTotal <= 0 || (ebookSearchDone==0))
				break;
			if(rollOverNum+(pageNum-1)*perNum < filelistTotal-1){
				if(rollOverNum<perNum-1){
					var oldnum:Number = rollOverNum++;
					itemMove(Ebook_mc.listBar_mc.content_mc["item"+oldnum],itemminH,Ebook_mc.listBar_mc.content_mc["item"+oldnum].showY);
					showCurrentItem(oldnum);
				}else if(pageNum<Math.ceil(filelistTotal/perNum)){
					pageNum++;
					rollOverNum = 0;
					creatlist(0);
					showCurrentItem(0);
				}
			}
			else{
				pageNum=1;
				rollOverNum = 0;
				creatlist(0);
				showCurrentItem(0);
			}
				
			break;
		case Key.ESCAPE:
			listBarBack();
			break;
	}
};
Key.addListener(Ebook_mc.listBar_mc.content_mc.key);

var Outline_exist:Number = 0;
var Outline_size:Number = 0;
var Outline_index:Number = 0;
function ShowOutline():Void{
	var idx:Number;
	var page:Number;
	var i:Number;

	idx=Outline_index-Outline_index%8;
	trace("Outline_index="+Outline_index);
	trace("idx="+idx);
	for(i=0;i<8;i++)
	{
		if(idx+i<Outline_size)
		{
			if(idx+i==Outline_index)
				outline_mc["line"+i+"_txt"].textColor = 0xff0000;
			else
				outline_mc["line"+i+"_txt"].textColor = 0xffffff;
			outline_mc["line"+i+"_txt"].text = EBook.GetOutline(idx+i);
		}
		else
			outline_mc["line"+i+"_txt"].text = "";
	}
}

var menuListener=new Object();
menuListener.onKeyDown=function(){
	trace("menuListener key");
	switch(Key.getCode()){
		case Key.DOWN:
			trace("menuListener----key down");
			EBook.NextPage();
			break;
		case Key.UP:
			trace("menuListener----key up");
			EBook.PrevPage();
			break;
		case Key.LEFT:
			trace("menuListener----key left");
			page_size=page_size-10;
			if(page_size<10)
				page_size=10;
			EBook.Zoom(page_size);
			MainSWF.ebooksize=page_size;
			break;
		case Key.RIGHT:
			trace("menuListener----key right");
			page_size=page_size+10;
			if(page_size>300)
				page_size=300;
			EBook.Zoom(page_size);
			MainSWF.ebooksize=page_size;
			break;
		case Key.ESCAPE:
			trace("menuListener----key escape");
			FlashEngine.Stop(false);
			MainSWF.ebook_osdid = -1;
			EBook.CloseDocument();
			overallBar_mc._x=0;
			overallBar_mc._y=-20;
			outline_mc._x=800;
			outline_mc._y=600;
			Key.removeListener(menuListener);
			Key.addListener(Ebook_mc.listBar_mc.content_mc.key);
			break;
		case Key.ENTER:
			break; 
		case CSwfKey.SWF_MSG_KEY_A:
			page_showmode++;
			EB_FIT_WIDTH=0;		///< fit screen width
			if(page_showmode>EBook.EB_FIT_SCREEN)
				page_showmode=EBook.EB_FIT_WIDTH;
			trace("page mode is " + page_showmode);
			EBook.ShowMode(page_showmode);
			break;
		case CSwfKey.SWF_MSG_KEY_B:
			trace("move left 10");
			EBook.Move(-10,0);
			break;
		case CSwfKey.SWF_MSG_KEY_C:
			trace("move down 10");
			EBook.Move(0,10);
			break;
		case CSwfKey.SWF_MSG_KEY_D:
			trace("move right 10");
			EBook.Move(10,0);
			break;
		case CSwfKey.SWF_MSG_KEY_E:
			EBook.Rotate();
			break;
		case CSwfKey.SWF_MSG_KEY_F:
			trace("move up 10");
			EBook.Move(0,-10);
			break;
		case CSwfKey.SWF_MSG_KEY_M:
			if(MainSWF.ebook_osdid == 0)
			{
				FlashEngine.Stop(false);
				MainSWF.ebook_osdid = -1;
			}
			else if(MainSWF.ebook_osdid == -1)
			{
				FlashEngine.Stop(false);
				FlashEngine.Play("ebookOSD.swf",true,MainSWF.ebook_osdx0,MainSWF.ebook_osdy0,MainSWF.ebook_osdw,MainSWF.ebook_osdh,FrameRate);
				MainSWF.ebook_osdid = 0;
			}
			break;
	}
}
