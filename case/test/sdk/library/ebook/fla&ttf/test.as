
import Actions.EBook;
import Actions.FileList;
import Actions.SystemInfo;
import Actions.FlashEngine;

var currentPath =  "/mnt/udisk/";
var filelistHandle:Number=-1;
var fileListTotal:Number=8;
var fileTotal:Number;
var fileNo:Number=0;
var nameArray:Array= new Array();
var urlArray:Array=new Array();

var pageTotal:Number=0;
var pageNo:Number=0;
var pageSize:Number=100;

function GetFileInfo()
{
	var i;
	
	filelistHandle=FileList.setPathNew(currentPath,"txt xls ppt doc pdf docx pptx xlsx",FileList.FL_MODE_FILES_BROWSE,1);
	fileTotal=FileList.getTotalNew(filelistHandle);
	for(i=0;i<fileTotal;i++)
	{
		nameArray[i]=FileList.getLongnameNew(filelistHandle,i);
		urlArray[i]=FileList.getPathNew(filelistHandle,i);
	}
	
	
/*
	fileTotal=20;
	for(i=0;i<fileTotal;i++)
	{
		nameArray[i]=i+".txt";
		urlArray[i]=i;
	}
*/
		

}
function DisplayFileList()
{
	var i:Number;
	var file_no_base:Number;
	
	file_no_base=fileNo-fileNo%fileListTotal;
	trace("fileNo="+fileNo);
	trace("file_no_base="+file_no_base);
	for(i=0;i<fileListTotal;i++)
	{
		if(file_no_base+i<fileTotal)
			this["file_list_"+i].txt.text=nameArray[file_no_base+i];
		else
			this["file_list_"+i].txt.text="";
	}
}
function SelectFile(no:Number)
{
	focus._y=this["file_list_"+no%fileListTotal]._y;
	//trace(focus);

	
}


function EnterEbook()
{
	blank_mc._x=0;
	blank_mc._y=0;

}
function EscEbook()
{

	blank_mc._x=0;
	blank_mc._y=0;
}
var fileListener=new Object();
fileListener.onKeyDown=function()
{

 keycode=Key.getCode();
 trace(keycode);
 switch(keycode)
 {
   case Key.DOWN:
   		fileNo++;
		if(fileNo>fileTotal-1)
			fileNo=fileTotal-1;
		DisplayFileList();
		SelectFile(fileNo);
		break;
	case Key.UP:
		fileNo--;
		if(fileNo<0)
			fileNo=0;
		DisplayFileList();
		SelectFile(fileNo);
		break;
	case Key.LEFT:
		break;
	 case Key.RIGHT:
		 break;
	 case Key.ESCAPE:
	 case Key.ENTER:
		//EnterEbook();
		//FlashEngine.SwfRender();
		EBook.OpenDocument(urlArray[fileNo]);
		page_size=100;
		pageNo=EBook.GetPageNo();
		//EBook.JumpPage(pageNo);
		trace("OpenDocument:"+urlArray[fileNo]);
		Key.removeListener(fileListener);
		Key.addListener(menuListener); 
		break;
	  
 }
}
var menuListener=new Object();
menuListener.onKeyDown=function()
{

 keycode=Key.getCode();
 trace(keycode);
 switch(keycode)
 {
   case Key.DOWN:
		pageNo++;
		pageTotal=EBook.GetPageTotal();
		if(pageNo>pageTotal)
			pageNo=pageTotal;
		EBook.JumpPage(pageNo);
		break;
	case Key.UP:
		pageNo--;
		if(pageNo<1)
			pageNo=1;
		EBook.JumpPage(pageNo);
		break;
	case Key.LEFT:
		page_size=page_size-10;
		if(page_size<10)
			page_size=10;
		EBook.Zoom(page_size);
		break;
	 case Key.RIGHT:
	 	//EBook.ZoomOut();
		page_size=page_size+10;
		if(page_size>300)
			page_size=300;
		EBook.Zoom(page_size);
		 break;
	 case Key.ESCAPE:
	 	EscEbook();
		EBook.CloseDocument();
		Key.removeListener(menuListener);
		Key.addListener(fileListener);
		break;
	 case Key.ENTER:
		break;
	case 77:
		Key.removeListener(menuListener);
		Key.addListener(viewListener);
		break;
	  
 }
}
var viewListener=new Object();
viewListener.onKeyDown=function()
{

 keycode=Key.getCode();
 trace(keycode);
 switch(keycode)
 {
   case Key.DOWN:
		EBook.Move(0,10);
		break;
	case Key.UP:
		EBook.Move(0,-10);
		break;
	case Key.LEFT:
		EBook.Move(10,0);
		break;
	 case Key.RIGHT:
	 	EBook.Move(-10,0);
		 break;
	 case Key.ESCAPE:
	 	EscEbook();
		EBook.CloseDocument();
		Key.removeListener(viewListener);
		Key.addListener(fileListener);
		break;
	 case Key.ENTER:
		break;
	case 77:
		Key.removeListener(viewListener);
		Key.addListener(menuListener);
		break;
	  
 }
}
function initial()
{
	trace("initial");
	//EBook.LoadDLL("/mnt/udisk/libhyfviewer.so");
	EBook.Open();
	//EBook.Close();
	GetFileInfo();
	DisplayFileList();
	if(fileTotal>0)
	{
		SelectFile(fileNo);
	}
	Key.addListener(fileListener);
	
}
initial();


