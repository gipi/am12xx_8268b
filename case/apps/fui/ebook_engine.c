#ifdef MODULE_CONFIG_EBOOK
#include "ebook_engine.h"
#include "swf_ext.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include "iconv.h"
#include "ebook_interface.h"
#include "GuiDraw.h"
#include "sys_resource.h"
#include "fileviewer.h"
#include "swfdec.h"
#include "display.h"

#define AM		0
#define HYF		1
#define PICSEL  2
//#define EBOOK AM
#define EBOOK PICSEL 
static int page_size;
extern void *deinst;

void *pdl=NULL;

enum {
	EB_READY=0,
	EB_BUSY=1,
};
enum{
	BOOK_INFO_MARKPAGE,
	BOOK_INFO_TOTALPAGE,
};

#define EBOOK_MARK_MAX 5
#define TEXT_BUF_LENGTH 200
static INT8S bookname[MARK_NAME_SIZE];
static INT32S state = EB_BUSY;
static INT32S width = 0;
static INT32S bookmark = 0;
static INT8S FileName_Mark[MARK_NAME_SIZE];

static EBOOK_APP_VRAM EBookMarkInfo[EBOOK_MARK_MAX];
static INT8S str[] = "Circuit diagrams and other information relating to products of Actions Semiconductor Company, Ltd.";
static INT8S buf[TEXT_BUF_LENGTH];

extern INT8U IsTaskCreate;
extern INT32S totalnum_char;
extern book_var_t  EBookTaskVars;
extern eb_show_var_t bg_show_var;
extern INT8U IS_CJK;//当前选定语言是否为中日韩
extern INT8U CharLengthInfo[CHAR_LENGTH_SIZE];//字符宽度信息
INT8U BookBuff[ONE_SECTOR_LENGTH];//从介质读入数据，存放在这里

static int doc_loaded_fully = 0;

static int picsel_err=0;

#define EB_reading_Tex_X1	780
#define EB_reading_Tex_X0	10
#define EB_reading_Tex_Y0	10
#define EB_reading_Tex_Y1	500

#define EB_reading_LineSpace	10
#define EB_reading_Tex_F		16



int ansichar_width_tableinit(void)
{
	FILE* fp;
	fp = fopen("/am7x/case/data/width.dat","r");
	if(!fp)
	{
		printf("no char width table\n");
		return -1;
	}
	fread(CharLengthInfo,1,CHAR_LENGTH_SIZE,fp);
	fclose(fp);
	return 0;
}
int ansi_2_unicode(unsigned char *pMultiByteStr,  int cchMultiByte, unsigned char *pWideCharStr, int cchWideChar)
{	
	return 100;	
}

/**
 * @brief  initial when open a new book.
 * @param[in] none.
 * @return none.
 */ 
INT8S newbook_init(eb_show_var_t * pEbshow)
{
	INT8U lang_id;
	INT32S type;
	lang_id = 0;//S_GUI_GetLanguageMode();//hikwlu
	//printf("lang_id book init=%d\n",lang_id);
	IS_CJK = 0;//IsCJK(lang_id);//hikwlu
	if(IS_CJK)
	{
		//EBookTaskVars.showvar.TxtW = EBookTaskVars.showvar.TxtW *3 /2; // adjust the txt width 
		printf("IsCJK\n");
	}
	else
	{
		printf("NotCJK\n");
		
	}
	//printf("%s,%d:width = %d\n",__FILE__,__LINE__,EBookTaskVars.showvar.TxtW);
	ansichar_width_tableinit();
	//S_GetWidthTab4CodePage(lang_id, CharLengthInfo);////get information of characters' width accrodding to the langID//hikwlu
	//EBookTaskVars.showvar.PageOffsetSave = (INT32U*)(GetSysInfo()->PluginHeapStart);//100K ,use photo dec buffer
	EBookTaskVars.showvar.PageOffsetSave = (INT32U *)SWF_Malloc(EB_PAGESAVE_LENGTH);//100K ,use photo dec buffer
	if(EBookTaskVars.showvar.PageOffsetSave ==NULL){
		printf("malloc page offsetSave Error!\n");
		return 0;
	}
	if(pEbshow!=NULL)
		EBookTaskVars.showvar.MaxLine  = pEbshow->MaxLine;
	else
		EBookTaskVars.showvar.MaxLine = \
		((EB_reading_Tex_Y1-EB_reading_Tex_Y0+EB_reading_LineSpace)/(EB_reading_Tex_F+EB_reading_LineSpace));

	if(pEbshow!=NULL)
		EBookTaskVars.showvar.LineSaveStep = pEbshow->LineSaveStep;
	else{
		if(LCD_WIDTH_E<=EB_WIDTH_VALUE_MIN)
		{
			EBookTaskVars.showvar.LineSaveStep = SAVE_OFFSET_PAGE_SMALL;
		}
		else
		{
			EBookTaskVars.showvar.LineSaveStep = SAVE_OFFSET_PAGE_LARGE;
		}
	}
	EBookTaskVars.showvar.showbuf = (INT8U*)malloc(LINE_MAX_BYTES);
	EBookTaskVars.showvar.bookbuffer = BookBuff;
	if(EBookTaskVars.showvar.showbuf==NULL)
	{
		printf("ebook malloc error\n");
	}
	else
	{
		//printf("malloc ok\n");
		memset(EBookTaskVars.showvar.showbuf,0,LINE_MAX_BYTES);
		memset(BookBuff,0,ONE_SECTOR_LENGTH);
	}
	return 1;
	//EBookTaskVars.showvar.TxtW = EB_reading_Tex_X1-EB_reading_Tex_X0;

#if 0
	#define EB_Page_X0	10
	#define EB_Page_X1	10
	#define EB_Page_Y0	550
	#define EB_Page_Y1	550
	page_rect.x0 = EB_Page_X0;
	page_rect.y0 = EB_Page_Y0;
	page_rect.x1 = EB_Page_X1;
	page_rect.y1 = EB_Page_Y1;
#endif	
 }

int apps_vram_read_ebook(EBOOK_APP_VRAM *pData, INT16U len)
{
	int ret = len;
	int fd;
	fd = open("//mnt//vram//ebook_info", O_RDONLY);
	if(fd < 0) {
		printf("Can't open/read ebook_info\n");
		return -1;
	}
	if( read(fd, pData, len) != (ssize_t)len ) {
		printf("Can't read!\n");
		ret = -1;
		goto fail;
	}
	fsync(fd);

fail:
	close(fd);
	return ret;
}

int apps_vram_write_ebook(EBOOK_APP_VRAM *pData, INT16U len)
{
	int ret = len;
	int fd;
	fd = open("//mnt//vram//ebook_info", O_RDWR|O_CREAT, 0664);
	if(fd < 0) {
		printf("Can't open/create ebook_info\n");
		return -1;
	}
	if( write(fd, pData, len) != (ssize_t)len ) {
		printf("Can't write!\n");
		ret = -1;
		goto fail;
	}
	fsync(fd);

fail:
	close(fd);
	return ret;
}

#if(EBOOK==AM)
static INT32S eb_open(void * handle)
{
	INT32S total_page=0;
	INT8S * filename;
	INT32S TxtW_Show;
	eb_show_var_t Ebshow;
	printf("%s,%d\n",__FILE__,__LINE__);
	SWFEXT_FUNC_BEGIN(handle);
	//if(_mlang_dll_open()<0)
		//goto OPEN_ERROR;
	///初始化全局变量EBookTaskVars各个成员
	EBookTaskVars.showvar.bookbuffer_len = 0;
	EBookTaskVars.showvar.curLine = 0;
	EBookTaskVars.showvar.CurPage = 0;
	EBookTaskVars.showvar.cur_point = 0;
	EBookTaskVars.showvar.bookbuffer = NULL;
	EBookTaskVars.showvar.fp = 0;
	EBookTaskVars.showvar.LineSaveStep = 0;
	EBookTaskVars.showvar.MaxLine =0 ;
	EBookTaskVars.showvar.PageOffsetSave = NULL;
	EBookTaskVars.showvar.sector = 0;
	EBookTaskVars.showvar.sector_offset = 0;
	EBookTaskVars.showvar.showbuf = NULL;
	EBookTaskVars.showvar.totalLine = 0;
	EBookTaskVars.showvar.TotlePage = 0;
	EBookTaskVars.showvar.TxtW = 0;
	EBookTaskVars.showvar.type = UTF8;//MBCS	//hikwlu
	/////////////////////////////////////
	filename = NULL;
	filename = Swfext_GetString();
	//BG_TaskRelease();
	if(strlen(filename)<MARK_NAME_SIZE){
		strcpy(bookname,filename);
	}
	else{
		memset(bookname,0,sizeof(bookname));
		printf("%s,%d: the length of file name is more than 200\n",__FILE__,__LINE__);
	}
	//width = 
	EBookTaskVars.showvar.TxtW=Swfext_GetNumber();
	Ebshow.LineSaveStep= Swfext_GetNumber();
	Ebshow.MaxLine = Swfext_GetNumber();
	if(newbook_init(&Ebshow)==0){
		printf("Open Book Init Error!\n");
		goto	OPEN_ERROR;
	}
	IsTaskCreate = 1;//= BG_CreateEBookTask();	//hikwlu
	if(IsTaskCreate){
		EBookTaskVars.showvar.fp = fopen(filename,"r");
		ebook_bg_ioctl(GET_FILE_TYPE,&EBookTaskVars.showvar);
		ebook_bg_ioctl(COUNT_TOTAL_PAGE,&EBookTaskVars.showvar);
		//while(1)
		{
			int tmp_page;
			//S_OSTimeDly(50);//hikwlu
			tmp_page = ebook_bg_ioctl(GET_TOTAL_PAGE_NUM,&EBookTaskVars.showvar);	
			//if(EBOOK_BG_ERROR==tmp_page)
				//continue;
			EBookTaskVars.showvar.totalLine = ebook_bg_ioctl(GET_CUR_LINE_NUM, 0);			
			EBookTaskVars.showvar.TotlePage = 1+ ((EBookTaskVars.showvar.totalLine -1)/EBookTaskVars.showvar.MaxLine);
			printf("total page===== %d,totalline=%d,EBOOk_READY\n",tmp_page,EBookTaskVars.showvar.totalLine);
			state = EB_READY;
			//break;
		};
	}
	else{
		printf("%s,%d:Create EBookTask Error\n",__FILE__,__LINE__);
	}
OPEN_ERROR:	
		Swfext_PutNumber(1);
		SWFEXT_FUNC_END();	
}
static INT32S eb_close(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	//printf("ebook close\n");
	ebook_bg_ioctl(BG_EBOOK_TASK_STOP,0);
	//S_OSTimeDly(5);//hikwlu
	//BG_DeleteEBookTask();
	if(EBookTaskVars.showvar.fp)
	{
		fclose(EBookTaskVars.showvar.fp);
		EBookTaskVars.showvar.fp = NULL;
		if(EBookTaskVars.showvar.PageOffsetSave !=NULL){
			SWF_Free(EBookTaskVars.showvar.PageOffsetSave);
		}
		if(EBookTaskVars.showvar.showbuf!=NULL){
			free(EBookTaskVars.showvar.showbuf);
		}
		state = EB_BUSY;
		IsTaskCreate = 0;
		memset(bookname,0,sizeof(bookname));
		//_mlang_dll_close();
	}
	SWFEXT_FUNC_END();	
}
#endif

static INT32S eb_get_totalrow(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	//printf("GetTotalRows %d\n",EBookTaskVars.showvar.totalLine);
	//Swfext_PutNumber(bg_show_var.curLine);
	Swfext_PutNumber(EBookTaskVars.showvar.totalLine);
	SWFEXT_FUNC_END();	
}

static INT32S eb_get_onerow(void * handle)
{
	INT32S row;
	INT8U *tmpbuf=NULL;
	unsigned int out_len = TEXT_BUF_LENGTH;
	SWFEXT_FUNC_BEGIN(handle);
	row = Swfext_GetNumber()+1;
	//sprintf(buf, "[%d] ", row);
	EBookTaskVars.showvar.curLine = row;
	ebook_bg_ioctl(GET_SPECIAL_LINE_DATA,&EBookTaskVars.showvar);
	//strncat(buf, EBookTaskVars.showvar.showbuf, totalnum_char);
	memset(buf,0,TEXT_BUF_LENGTH);
	if(EBookTaskVars.showvar.type == MBCS){
		INT32S i=0;
		INT32S realnum;
		
		tmpbuf = (INT8U*)malloc(TEXT_BUF_LENGTH);
		
		if(tmpbuf ==NULL){
			Swfext_PutString(buf);
			SWFEXT_FUNC_END();
			return 0;
		}
		if(totalnum_char==0){
			free(tmpbuf);
			goto GETROW_END;
		}
		gb2312_to_utf16le((char*)EBookTaskVars.showvar.showbuf,(size_t*)&totalnum_char,(char*)tmpbuf,&out_len);//hikwlu
		//printf("ansi 2 Unicode ,realnum-unicode=%d,tmpbuf=0x%x\n",realnum,tmpbuf);
		realnum = TEXT_BUF_LENGTH - out_len;
		utf16le_to_utf8((char*)tmpbuf,(size_t*)&realnum,buf,&out_len);
		free(tmpbuf);
		tmpbuf = NULL;
	}
	else if(EBookTaskVars.showvar.type == UNI16_LIT ||EBookTaskVars.showvar.type == UNI16_BIG){
		//showData(EBookTaskVars.showvar.showbuf,totalnum_char, 16);
		if(totalnum_char==0)
			goto GETROW_END;
		if(EBookTaskVars.showvar.showbuf){
			if(EBookTaskVars.showvar.type == UNI16_LIT)
				utf16le_to_utf8((char*)EBookTaskVars.showvar.showbuf,(size_t*)&totalnum_char,buf,&out_len);
			else if(EBookTaskVars.showvar.type == UNI16_BIG)
				utf16be_to_utf8((char*)EBookTaskVars.showvar.showbuf,(size_t*)&totalnum_char,buf,&out_len);
		}
	}
	else{
		if(totalnum_char!=0)
		{
			//memcpy(buf, EBookTaskVars.showvar.showbuf, totalnum_char);
			utf16le_to_utf8((char*)EBookTaskVars.showvar.showbuf,(size_t*)&totalnum_char,buf,&out_len);
		}
	}
	//printf("row[%d]=%s",row,buf);
GETROW_END:
	totalnum_char = 0;
	Swfext_PutString(buf);
	SWFEXT_FUNC_END();	
}

static INT32S eb_get_status(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutNumber(state);
	SWFEXT_FUNC_END();	
}

static INT32S eb_get_bookmark(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutNumber(bookmark);
	SWFEXT_FUNC_END();	
}

static INT32S eb_set_bookmark(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	bookmark = Swfext_GetNumber();
	SWFEXT_FUNC_END();	
}

static INT32S GetFileName(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutString(FileName_Mark);
	//printf("Get!!!FileName_Mark is %s\n", FileName_Mark);
	SWFEXT_FUNC_END();	
}

static INT32S SetFileName(void * handle)
{
	INT8S * filename;
	SWFEXT_FUNC_BEGIN(handle);
	filename = NULL;
	filename = Swfext_GetString();
	if(strlen(filename)<MARK_NAME_SIZE){
		strcpy(FileName_Mark,filename);
	}
	else{
		memset(FileName_Mark,0,sizeof(FileName_Mark));
		printf("%s,%d: the length of file name is more than 200\n",__FILE__,__LINE__);
	}
	//printf("Set!!!FileName_Mark is %s\n", FileName_Mark);
	SWFEXT_FUNC_END();	
}

static INT32S eb_get_color(void * handle)
{
	INT32S color = 0;
	SWFEXT_FUNC_BEGIN(handle);
	color = get_ebook_text_color();
	//printf("%s,%d:color is 0x%x\n",__FILE__,__LINE__,color);
	Swfext_PutNumber(color);
	SWFEXT_FUNC_END();	
}

static INT32S eb_set_color(void * handle)
{
	INT32S color = 0;
	SWFEXT_FUNC_BEGIN(handle);
	color = Swfext_GetNumber();
	set_ebook_text_color(color);
	//printf("%s,%d:color is 0x%x\n",__FILE__,__LINE__,color);
	SWFEXT_FUNC_END();	
}

static INT32S eb_get_bgcolor(void * handle)
{
	INT32S bgcolor = 0xFFFFFF;
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutNumber(bgcolor);
	SWFEXT_FUNC_END();	
}

static INT32S eb_set_bgcolor(void * handle)
{
	INT32S bgcolor = 0xFFFFFF;
	SWFEXT_FUNC_BEGIN(handle);
	bgcolor = Swfext_GetNumber();
	SWFEXT_FUNC_END();	
}

static INT32S eb_get_fontsize(void * handle)
{
	INT32S text_size = 0;
	SWFEXT_FUNC_BEGIN(handle);
	text_size = get_ebook_text_size();
	//printf("%s,%d:text_size is %d\n",__FILE__,__LINE__,text_size);
	Swfext_PutNumber(text_size);
	SWFEXT_FUNC_END();	
}

static INT32S eb_set_fontsize(void * handle)
{
	INT32S text_size = 0;
	SWFEXT_FUNC_BEGIN(handle);
	text_size = Swfext_GetNumber();
	//printf("%s,%d:text_size is %d\n",__FILE__,__LINE__,text_size);
	set_ebook_text_size(text_size);
	SWFEXT_FUNC_END();	
}

static INT32S eb_get_markname(void *handle)
{
	INT8U index;
	SWFEXT_FUNC_BEGIN(handle);
	index = Swfext_GetNumber();
	apps_vram_read_ebook(&EBookMarkInfo[index],sizeof(EBOOK_APP_VRAM));
	//printf("mark name name=%s\n",EBookMarkInfo[index].mark.txtFileName);
	Swfext_PutString(EBookMarkInfo[index].mark.txtFileName);
	SWFEXT_FUNC_END();
}

static INT32S eb_get_bookinfo(void *handle)
{
	INT8U index;
	INT8U para;
	SWFEXT_FUNC_BEGIN(handle);
	index = Swfext_GetNumber();
	para = Swfext_GetNumber();
	apps_vram_read_ebook(&EBookMarkInfo[index],sizeof(EBOOK_APP_VRAM));
	switch(para){
		case BOOK_INFO_MARKPAGE:
			Swfext_PutNumber(EBookMarkInfo[index].mark.mark_page);
			//printf("%s,%d:mark_page==%d\n",__FILE__,__LINE__,EBookMarkInfo[index].mark.mark_page);
			break;
		case BOOK_INFO_TOTALPAGE:
			Swfext_PutNumber(EBookMarkInfo[index].mark.total_page);
			//printf("%s,%d:totalpage==%d\n",__FILE__,__LINE__,EBookMarkInfo[index].mark.total_page);
			break;
		default:
			//printf("%s,%d:Error: GetBookInfo\n",__FILE__,__LINE__);
			Swfext_PutNumber(0);
			break;
	}
	
	SWFEXT_FUNC_END();	
}

static INT32S eb_save_bookinfo(void *handle)
{
	INT8U index;
	SWFEXT_FUNC_BEGIN(handle);
	index = Swfext_GetNumber();
	if(index >= EBOOK_MARK_MAX){
		Swfext_PutNumber(0);
		printf("%s,%d:Error: Save EbookInfo\n",__FILE__,__LINE__);
	}
	else{
		if(state==EB_READY){
			if(bookname!=NULL){
				//printf("bookname=%s\n",bookname);
				strcpy(EBookMarkInfo[index].mark.txtFileName,bookname);
				EBookMarkInfo[index].mark.total_page = EBookTaskVars.showvar.TotlePage;
				EBookMarkInfo[index].mark.mark_page=1+ ((EBookTaskVars.showvar.curLine -1)/EBookTaskVars.showvar.MaxLine);
				//printf("%s,%d:totalpage=%d,mark_page=%d\n",__FILE__,__LINE__,EBookMarkInfo[index].mark.total_page,EBookMarkInfo[index].mark.mark_page);
				//printf("name==%s\n",EBookMarkInfo[index].mark.txtFileName);
				if(apps_vram_write_ebook(&EBookMarkInfo[index],sizeof(EBOOK_APP_VRAM))==-1)
				{
					Swfext_PutNumber(0);
					//printf("%s,%d:Error: Vram write failed\n",__FILE__,__LINE__);
				}
				else
					Swfext_PutNumber(1);
			}
			else{
				Swfext_PutNumber(0);
				//printf("%s,%d:Error: Save EbookInfo:filename==NULL\n",__FILE__,__LINE__);
			}
			
		}
		
	}	
	SWFEXT_FUNC_END();
	
}

#if(EBOOK==HYF)

static int outline_exist_not=0;
static int outline_size = 0;
static char **outline;

static INT32S eb_load_dll(char* path)
{

	GUIFUNC gui;
	
	OSprintf("eb_load_dll path=%s\n",path);
	pdl = (void*)OSDLOpen(path);
	if(!pdl)
	{
		OSprintf("dl error\n");
		OSDLClose(pdl);
		goto _eb_LoadDLL_out;
		
	}	
	EbOpenDocument = (int(*)(char*, int, int, int))OSDLGetProcAddr(pdl, "OpenDocument");
	if(!EbOpenDocument)
	{
		OSprintf("load OpenDocument error\n");	
		goto _eb_LoadDLL_out;
	}

	EbJumpPage = (int(*)(int))OSDLGetProcAddr(pdl, "JumpPage");
	if(!EbJumpPage)
	{
		OSprintf("load JumpPage error\n");	
		goto _eb_LoadDLL_out;
	}
	EbCloseDocument = (int(*)())OSDLGetProcAddr(pdl, "CloseDocument");
	if(!EbCloseDocument)
	{
		OSprintf("load CloseDocument error\n");	
		goto _eb_LoadDLL_out;
	}
	EbInitProgram = (int(*)(void*))OSDLGetProcAddr(pdl, "InitProgram");
	if(!EbInitProgram)
	{
		OSprintf("load InitProgram error\n");		
		goto _eb_LoadDLL_out;
	}
	EbCloseProgram = (int(*)())OSDLGetProcAddr(pdl, "CloseProgram");
	if(!EbCloseProgram)
	{
		OSprintf("load CloseProgram error\n");		
		goto _eb_LoadDLL_out;
	}
	EbSetTextProperty = (int(*)(int))OSDLGetProcAddr(pdl, "SetTextProperty");
	if(!EbSetTextProperty)
	{
		OSprintf("load SetTextProperty error\n");		
		goto _eb_LoadDLL_out;
	}	
	EbScalePage = (int(*)(int))OSDLGetProcAddr(pdl, "ScalePage");
	if(!EbScalePage)
	{
		OSprintf("load ScalePage error\n");		
		goto _eb_LoadDLL_out;
	}
	EbGetSumPageNum = (int(*)())OSDLGetProcAddr(pdl, "GetSumPageNum");
	if(!EbGetSumPageNum)
	{
		OSprintf("load GetSumPageNum error\n");		
		goto _eb_LoadDLL_out;
	}	
	EbGetCurrentPageNum = (int(*)())OSDLGetProcAddr(pdl, "GetCurrentPageNum");
	if(!EbGetCurrentPageNum)
	{
		OSprintf("load GetCurrentPageNum error\n");		
		goto _eb_LoadDLL_out;
	}		
	EbMoveOffset = (int(*)(int, int))OSDLGetProcAddr(pdl, "MoveOffset");
	if(!EbMoveOffset)
	{
		OSprintf("load MoveOffset error\n");		
		goto _eb_LoadDLL_out;
	}			
	EbGetOutlinesContent = (char**(*)())OSDLGetProcAddr(pdl, "GetOutlinesContent");
	if(!EbGetOutlinesContent)
	{
		OSprintf("load GetOutlinesContent error\n");		
		goto _eb_LoadDLL_out;
	}
	printf("EbGetOutlinesContent is 0x%x\n",EbGetOutlinesContent);
	EbGetOutlinesSize = (int(*)())OSDLGetProcAddr(pdl, "GetOutlinesSize");
	if(!EbGetOutlinesSize)
	{
		OSprintf("load GetOutlinesSize error\n");		
		goto _eb_LoadDLL_out;
	}
	printf("EbGetOutlinesSize is 0x%x\n",EbGetOutlinesSize);
	EbOutlinesExist = (int(*)())OSDLGetProcAddr(pdl, "OutlinesExist");
	if(!EbOutlinesExist)
	{
		OSprintf("load OutlinesExist error\n");		
		goto _eb_LoadDLL_out;
	}
	//printf("EbOutlinesExist is 0x%x\n",EbOutlinesExist);
	EbGotoOutlinesPage = (int(*)(int))OSDLGetProcAddr(pdl, "GotoOutlinesPage");
	if(!EbGotoOutlinesPage)
	{
		OSprintf("load GotoOutlinesPage error\n");		
		goto _eb_LoadDLL_out;
	}
	//printf("EbGotoOutlinesPage is 0x%x\n",EbGotoOutlinesPage);
			
	gui.ClearScrean=GuiClearScrean;
	gui.DeleteBrush=GuiDeleteBrush;
	gui.DrawArc=GuiDrawArc;
	gui.DrawEllipse=GuiDrawEllipse;
	gui.DrawImage=GuiDrawImage;
	gui.DrawImagePic=GuiDrawImagePic;
	gui.DrawLine=GuiDrawLine;
	gui.DrawPie=GuiDrawPie;
	gui.DrawPoint=GuiDrawPoint;
	gui.DrawPolygon=GuiDrawPolygon;
	gui.DrawPolyline=GuiDrawPolyline;
	gui.DrawRect=GuiDrawRect;
	gui.DrawText=GuiDrawText;
	gui.SetBrush=GuiSetBrush;
	gui.SetFont=GuiSetFont;
	gui.SetPen=GuiSetPen;
	gui.FillPolygon=GuiFillPolygon;
	gui.FillEllipse=GuiFillEllipse;
	gui.GetText=GuiGetText;
	gui.DrawPath=GuiDrawPath;
	gui.FillPath=GuiFillPath;
	gui.DrawImagePath=GuiDrawImagePath;
	gui.FillLinear=GuiFillLinear;
	gui.FillRadial=GuiFillRadial;
	GuiSetVdc(NULL);
	//printf("load %s ok\n",path);
	EbInitProgram(&gui);
	//printf("InitProgram end\n");

_eb_LoadDLL_out:	
	SWFEXT_FUNC_END();	
}


static INT32S eb_open(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	eb_load_dll("/mnt/udisk/libhyfviewer.so");
	SWFEXT_FUNC_END();	
}	

static INT32S eb_close(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	OSDLClose(pdl);
	SWFEXT_FUNC_END();	
}	

static INT32S eb_open_document(void * handle)
{
	unsigned char *fname;	
	
	SWFEXT_FUNC_BEGIN(handle);
	
	fname=(unsigned char *)Swfext_GetString();
	//GuiClearScrean();
	FUI_setSwapFlag(0);
	page_size=100;
	printf("EbOpenDocument %s\n",fname);
	EbOpenDocument((void*)fname,LCD_WIDTH_E,LCD_HEIGHT_E,0);
	while(EbGetSumPageNum()<1)
		OSSleep(20);
	//EbJumpPage(1);
	//EbScalePage(page_size);
	FUI_setSwapFlag(1);
	SWFEXT_FUNC_END();	
	
}
static INT32S eb_close_document(void * handle)
{
	
	SWFEXT_FUNC_BEGIN(handle);
	//printf("EbCloseDocument\n");
	FUI_setSwapFlag(0);
	EbCloseDocument();
	FUI_setSwapFlag(1);
	//OSDLClose(pdl);
	SWFEXT_FUNC_END();	
	
}
static INT32S eb_jump_page(void * handle)
{
	unsigned int no;
	
	SWFEXT_FUNC_BEGIN(handle);
	no=Swfext_GetNumber();
	FUI_setSwapFlag(0);
	//printf("start:EbJumpPage %d \n",no);
	//printf("start:CurrentPageNum %d \n",EbGetCurrentPageNum());
	EbJumpPage(no);
	//printf("end:CurrentPageNum %d \n",EbGetCurrentPageNum());
	//printf("end:EbJumpPage %d \n",no);
	FUI_setSwapFlag(1);
	SWFEXT_FUNC_END();	
	
}

static INT32S eb_zomm_in(void * handle)
{
	
	SWFEXT_FUNC_BEGIN(handle);
	FUI_setSwapFlag(0);
	//EbZoomIn();
	page_size=page_size+10;
	if(page_size>300)
		page_size=300;
	//printf("EbZoomIn %d\n",page_size);
	EbScalePage(page_size);
	FUI_setSwapFlag(1);
	SWFEXT_FUNC_END();		
}
static INT32S eb_zoom_out(void * handle)
{
	
	SWFEXT_FUNC_BEGIN(handle);
	FUI_setSwapFlag(0);
	//EbZoomOut();
	page_size=page_size-10;
	if(page_size<10)
		page_size=10;
	//printf("EbZoomOut %d\n",page_size);
	EbScalePage(page_size);
	FUI_setSwapFlag(1);
	SWFEXT_FUNC_END();		
}

static INT32S eb_zoom(void * handle)
{
	int scale;
	
	SWFEXT_FUNC_BEGIN(handle);
	FUI_setSwapFlag(0);
	scale=Swfext_GetNumber();
	EbScalePage(scale);
	FUI_setSwapFlag(1);
	SWFEXT_FUNC_END();		
}
static INT32S eb_get_page_total(void * handle)
{
	unsigned int total;
	
	SWFEXT_FUNC_BEGIN(handle);
	total=EbGetSumPageNum();
	printf("EbGetSumPageNum=%d\n",total);
	Swfext_PutNumber(total);
	SWFEXT_FUNC_END();		
}
static INT32S eb_get_page_no(void * handle)
{
	unsigned int no;
	
	SWFEXT_FUNC_BEGIN(handle);
	no=EbGetCurrentPageNum();
	Swfext_PutNumber(no);
	SWFEXT_FUNC_END();		
}
static INT32S eb_move(void * handle)
{
	int x,y;
	
	SWFEXT_FUNC_BEGIN(handle);
	FUI_setSwapFlag(0);
	x=Swfext_GetNumber();
	y=Swfext_GetNumber();
	EbMoveOffset(x,y);
	FUI_setSwapFlag(1);
	SWFEXT_FUNC_END();		
}

static void print_test(char* strprint, int len,int eachline)
{
	int i=0;
	//printf("\n~~~~~~~~~~len=%d~~~~~~~~~~~~\n",len);
	for(i=0;i<len;i++){
		if(i%eachline==0 && i!=0)
			//printf("\n");
		//printf("0x%2x,",*(strprint+i));
	}
	//printf("\n~~~~~~~~~~~~~~~~~~~~~~\n");
}


static INT32S eb_outline_exist(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	outline_exist_not = EbOutlinesExist();
	if(outline_exist_not==1)
	{
		printf("%s,%d:Outlines don't exist in this file\n",__FILE__,__LINE__);
		goto eb_outline_exist_out;
	}
eb_outline_exist_out:
	Swfext_PutNumber(outline_exist_not);
	SWFEXT_FUNC_END();		
}

static INT32S eb_outline_size(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	if(outline_exist_not==0)
	{
		outline_size = EbGetOutlinesSize();
		printf("%s,%d:Outlines size is %d\n",__FILE__,__LINE__,outline_size);
	}
	else
	{
		outline_size = 0;
		printf("%s,%d:Outlines don't exist in this file,size is 0\n",__FILE__,__LINE__);
	}
	Swfext_PutNumber(outline_size);
	SWFEXT_FUNC_END();		
}

static INT32S eb_get_outline(void * handle)
{
	char outline_des[256];
	int index;
	int in_len,out_len=256;
	charsetchg_info_t change_info;
	SWFEXT_FUNC_BEGIN(handle);
	index=Swfext_GetNumber();
	if(outline==NULL)
		outline = EbGetOutlinesContent();
	
	memset(outline_des,0,256);
	if(index>=0 && index<outline_size)
	{
		in_len = strlen(outline[index]);	
		change_info.src_charset = LAN_GB2312;
		change_info.dest_charset = LAN_UTF8;
		change_info.inputbuf = outline[index];
		change_info.inputbuflen = (size_t*)&in_len;
		change_info.outputbuf = outline_des;
		change_info.outputbuflen = (size_t*)&out_len;
		//print_test(outline[index],in_len,16);
		change_chardec(&change_info);
		//print_test(outline_des,256-out_len,16);
	}
	else
	{
		printf("%s,%d:index is error,index = %d\n",__FILE__,__LINE__,index);
	}

	Swfext_PutString(outline_des);
	SWFEXT_FUNC_END();
}

static INT32S eb_goto_outline_page(void * handle)
{
	int index;
	int ret=1;
	SWFEXT_FUNC_BEGIN(handle);
	
	index=Swfext_GetNumber();
	//printf("index is %d\n",index);
	if(index>=0 && index<outline_size)
	{
		index++;
		ret = EbGotoOutlinesPage(index);
	}
	else
	{
		printf("%s,%d:index is error,index = %d\n",__FILE__,__LINE__,index);
	}

	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();
}
static INT32S eb_rotate(void * handle)
{

	SWFEXT_FUNC_BEGIN(handle);
	SWFEXT_FUNC_END();
}

static INT32S eb_showmode(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	SWFEXT_FUNC_END();
}

static INT32S eb_prevpage(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	SWFEXT_FUNC_END();
}

static INT32S eb_nextpage(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	SWFEXT_FUNC_END();
}

static INT32S eb_getzoomsize(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	SWFEXT_FUNC_END();
}

static INT32S eb_getloadfully(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	SWFEXT_FUNC_END();	
}
#endif

#if (EBOOK==PICSEL)

static int update_flag=0;
static void* buffer_1;
extern unsigned long _fui_get_cur_framebuf(void);
static int scale_pre=100;
static void* heap=NULL;

#define SCALEBITS	16	/* speediest right-shift on some machines */
#define FIX(x)		((int) ((x) * (1L<<SCALEBITS) + 0.5))

static int set_de_defaultcolor()
{
	DE_config ds_conf;
	
	de_get_config(deinst,&ds_conf,DE_CFG_ALL);
	ds_conf.input.enable=0;
	de_set_Config(deinst,&ds_conf,DE_CFG_ALL);
	
	return 0;
}
static int display(void* buffer)
{
	DE_config ds_conf;
	
	de_get_config(deinst,&ds_conf,DE_CFG_IN);
	ds_conf.input.img = (unsigned long*)buffer;
	ds_conf.input.bus_addr = fui_get_bus_address((unsigned long)buffer);
	ds_conf.input.enable=1;
	de_set_Config(deinst,&ds_conf,DE_CFG_IN);
	
	return 0;
}
static void fv_screen_update_rgb565(void *handle_in,
									void *buffer,
									unsigned int width,
									unsigned int height,
									unsigned int widthBytes,
									unsigned int updateX,
									unsigned int updateY,
									unsigned int updateWidth,
									unsigned int updateHeight)
{
	void * handle;
	void* buffer_now=buffer_1;
	

	if(update_flag)
	{
		//handle=(void*)_fui_get_cur_framebuf();	
		//handle=(void*)SWF_GetCurrentFrame();
		handle=(void*)buffer_now;
		if( handle &&
			buffer &&
			width &&
			height &&
			widthBytes >= width*2 &&
			updateX < width &&
			updateY < height &&
			updateX+updateWidth <= width &&
			updateY+updateHeight <= height)
		{
			FILE *fptr = NULL;
			char file_name[128];
	
			unsigned char *src;
			unsigned int src_stride;
	
			unsigned char *dst;
			unsigned int dst_stride;
	
			unsigned int start_x;
			unsigned int end_x;
			unsigned int update_width;
			unsigned int update_height;
	
			int i;
	
			src = (unsigned char*)buffer;
			src_stride = widthBytes;
	
			dst = (unsigned char*)handle;
			dst_stride = widthBytes;
	
			update_width = updateWidth;
			update_height = updateHeight;
			start_x = updateX;
			end_x = start_x+updateWidth;
	
			if(updateX)
			{
				src += updateX*2;
				dst += updateX*2;
			}
			if(updateY)
			{
				src += updateY*src_stride;
				dst += updateY*dst_stride;
			}
			for(i=0;i<update_height;i++)
			{
				int x;
				for(x=start_x;x<end_x;x++)
				{
					int r, g, b;
					int y, u, v;
	
					r = src[1]&0xf8;
					g = ((src[1]&0x07)<<5)|((src[0]&0xe0)>>3);
					b = (src[0]&0x1f)<<3;
					y = ( FIX(0.29900)*r + FIX(0.58700)*g + FIX(0.11400)*b );
					y >>= SCALEBITS;
					dst[0] = (unsigned char)y;
					if(!(x&1))
					{
						u = (-FIX(0.16874)*r - FIX(0.33126)*g + FIX(0.50000)*b );
						v = ( FIX(0.50000)*r - FIX(0.41869)*g - FIX(0.08131)*b );
						u >>= SCALEBITS;
						v >>= SCALEBITS;
						u += 128;
						v += 128;
						dst[1] = (unsigned char)u;
						dst[3] = (unsigned char)v;
					}
					src += 2;
					dst += 2;
				}
				src += src_stride-update_width*2;
				dst += dst_stride-update_width*2;
			}
			display(buffer_now);
	//			sprintf(file_name, "/mnt/udisk/output_%dx%d.yuv", width, height);
	//			fptr = fopen(file_name, "ab");
	//			if(fptr)
	//			{
	//				fwrite(handle, 1, width*height*2, fptr);
	//				fclose(fptr);
	//			}
	
		}
	}

}

static int translate_err_str(const char*str)
{
	int ret=0;
	if(strcmp(str,"AgentMatchFailed")==0){
		ret = PicselDocumentError_AgentMatchFailed;
	}
	else if(strcmp(str,"AgentMatchFailedNoData")==0){
		ret = PicselDocumentError_AgentMatchFailedNoData;
	}
	else if(strcmp(str,"DocumentTranslationFailed")==0){
		ret = PicselDocumentError_DocumentTranslationFailed;
	}
	else if(strcmp(str,"DocumentPasswordProtected")==0){
		ret = PicselDocumentError_DocumentPasswordProtected;
	}
	else if(strcmp(str,"InternalError")==0){
		ret = PicselDocumentError_InternalError;
	}
	else if(strcmp(str,"UnsupportedCharset")==0){
		ret = PicselDocumentError_UnsupportedCharset;
	}
	else if(strcmp(str,"SettingsPathUndefined")==0){
		ret = PicselError_SettingsPathUndefined;
	}
	else if(strcmp(str,"OutOfMemory")==0){
		ret = PicselError_OutOfMemory;
	}
	else if(strcmp(str,"UnsupportedJSPopup")==0){
		ret = PicselError_UnsupportedJSPopup;
	}
	else if(strcmp(str,"UploadFileTooLarge")==0){
		ret = PicselError_UploadFileTooLarge;
	}
	else if(strcmp(str,"LastErrorUnused")==0){
		ret = PicselError_LastErrorUnused;
	}
	else if(strcmp(str,"Err_Unknown")==0){
		ret = PicselError_Unknow;
	}
	return ret;		
}

static void notify_information(const char *str)
{
	//printf("call %s(), %s\n", __FUNCTION__, str);
	int ret=0;
	//printf("@@@@@@notify_info=%s\n",str);
	if(!strcmp(str,"AlienInformation_DocumentLoaded")){
		//printf("document has been loaded fully!\n");
		doc_loaded_fully = 1;
	}
#if FILE_ENCRYPTION_DETECT
	ret = fv_get_error_code();
#else
	ret = translate_err_str(str);
#endif
	if(ret!=0){
		picsel_err = ret;
		//printf("picsel_err=====0x%x",picsel_err);
	}
}

static INT32S eb_open(void * handle)
{
	unsigned int addr;
	int ret;
	FV_INIT_PARAM init_param;

	SWFEXT_FUNC_BEGIN(handle);
	

	memset(&init_param, 0, sizeof(FV_INIT_PARAM));
	init_param.frame_buf_width = LCD_WIDTH_E;
	init_param.frame_buf_height = LCD_HEIGHT_E;
	
	buffer_1=SWF_Malloc(LCD_WIDTH_E*LCD_HEIGHT_E*2);
	if(!buffer_1){
		printf("eb_open failed:buffer_1\n");
		goto end;
	}
		
	init_param.fv_screen_handle = buffer_1;
	if(!init_param.fv_screen_handle)
		goto end;
	init_param.fv_screen_update_rgb565 = fv_screen_update_rgb565;

	init_param.notify_information = notify_information;
	picsel_err = 0;
	fv_reset_error_code();
	init_param.heap_size = 32*1024*1024;
	heap=SWF_Malloc(init_param.heap_size);
	if(!heap)
		printf("eb_open failed:heap\n");
	init_param.heap_buffer=heap;
	update_flag=0;
	
	strcpy(init_param.locale_set, "zh-cn");
	init_param.group = GROUP_EASTERN_LANGUAGES;
	
	ret = fv_init(&init_param);
	if(ret)
		goto end;

	Swfext_PutNumber(1);
	SWFEXT_FUNC_END();	
	
	end:
		fv_release();
		SWF_Free(buffer_1);
		Swfext_PutNumber(0);
		SWFEXT_FUNC_END();	
}


static INT32S eb_close(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	update_flag=0;
	picsel_err = 0;
	fv_release();
	SWF_Free(buffer_1);
	SWF_Free(heap);
	SWFEXT_FUNC_END();	
}

static INT32S eb_open_document(void * handle)
{
	char* fname;	
	
	SWFEXT_FUNC_BEGIN(handle);
	printf("eb_open_document\n");
	fname=(char *)Swfext_GetString();
	printf("fname==================%s\n",fname);
	/**
	* set fui to sleep.
	*/
	if(SWF_GetState(SWF_GetActiveInst()) & SWF_STATE_ACTIVE)
	{
		/** first set swf to sleep, and buffer will be released */
		printf("SWF_SetState\n");
		SWF_SetState(SWF_GetActiveInst(), SWF_STATE_SLEEP);
	}
	
	set_de_defaultcolor();
	update_flag=1;
	scale_pre=100;
	doc_loaded_fully = 0;
	
	printf("fv_open_file\n");
	fv_open_file(fname);
	SWFEXT_FUNC_END();	
	
}
static INT32S eb_close_document(void * handle)
{
	
	SWFEXT_FUNC_BEGIN(handle);
	printf("eb_close_document\n");
	update_flag=0;
	picsel_err = 0;
	fv_reset_error_code();
	fv_close_file();
	set_de_defaultcolor();
	if((SWF_GetState(SWF_GetActiveInst()) & SWF_STATE_ACTIVE) == 0)
	{
		SWF_SetState(SWF_GetActiveInst(), SWF_STATE_ACTIVE);
	}

	SWFEXT_FUNC_END();	
	
}
static INT32S eb_jump_page(void * handle)
{
	unsigned int no;
	
	SWFEXT_FUNC_BEGIN(handle);
	printf("eb_jump_page\n");
	no=Swfext_GetNumber();
	fv_goto_page(no);
	//fv_next_page();
	//OSSleep(1000);

	SWFEXT_FUNC_END();	
	
}

static INT32S eb_zomm_in(void * handle)
{
	
	SWFEXT_FUNC_BEGIN(handle);
	printf("eb_zomm_in\n");
	fv_zoom_in();

	SWFEXT_FUNC_END();		
}
static INT32S eb_zoom_out(void * handle)
{
	
	SWFEXT_FUNC_BEGIN(handle);
	printf("eb_zoom_out\n");
	fv_zoom_out();

	SWFEXT_FUNC_END();		
}

static INT32S eb_zoom(void * handle)
{

	int scale;

	SWFEXT_FUNC_BEGIN(handle);
	scale=Swfext_GetNumber();
	if(scale>scale_pre)
	{
		//printf("fv_zoom_in\n");
		fv_zoom_in();
	}
	else if(scale<scale_pre)
	{
		//printf("fv_zoom_out\n");
		fv_zoom_out();
	}
	scale_pre=scale;

	SWFEXT_FUNC_END();		
}

static INT32S eb_get_page_total(void * handle)
{
	int total=0;
	int num=0;
	
	SWFEXT_FUNC_BEGIN(handle);
	//printf("eb_get_page_total\n");
	fv_get_page_num(&num, &total);
	Swfext_PutNumber(total);
	SWFEXT_FUNC_END();		
}
static INT32S eb_get_page_no(void * handle)
{
	int total = 0;
	int num = 0;
	
	SWFEXT_FUNC_BEGIN(handle);
	fv_get_page_num(&num, &total);
	//printf("total=%d\n",total);
	//printf("num=%d\n",num);
	Swfext_PutNumber(num);
	SWFEXT_FUNC_END();		
}
static INT32S eb_move(void * handle)
{
	int x,y;
	
	SWFEXT_FUNC_BEGIN(handle);
	x=Swfext_GetNumber();
	y=Swfext_GetNumber();
	printf("eb_move x:%d y:%d\n",x,y);
	if(x>0)
		fv_move_right();
	else if(x<0)
		fv_move_left();

	if(y>0)
	{
		printf("fv_move_down\n");
		fv_move_down();
	}
	else if(y<0)
	{
		printf("fv_move_up\n");
		fv_move_up();
	}
	
	SWFEXT_FUNC_END();		
}

static INT32S eb_outline_exist(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);

	SWFEXT_FUNC_END();		
}
static INT32S eb_outline_size(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);

	SWFEXT_FUNC_END();		
}
static INT32S eb_get_outline(void * handle)
{

	SWFEXT_FUNC_BEGIN(handle);

	SWFEXT_FUNC_END();
}

static INT32S eb_goto_outline_page(void * handle)
{

	SWFEXT_FUNC_BEGIN(handle);
	SWFEXT_FUNC_END();
}

static INT32S eb_rotate(void * handle)
{

	SWFEXT_FUNC_BEGIN(handle);
	printf("eb_rorate\n");
	fv_rotate();
	SWFEXT_FUNC_END();
}

static INT32S eb_showmode(void * handle)
{
	int mode = 0;
	SWFEXT_FUNC_BEGIN(handle);
	printf("eb_showmode\n");
	mode=Swfext_GetNumber();
	switch(mode)
	{
		case 0:
			fv_fit_width();
			break;
		case 1:
			fv_fit_height();
			break;
		case 2:
			fv_fit_screen();
			break;
	}
	SWFEXT_FUNC_END();
}

static INT32S eb_prevpage(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	printf("eb_prevpage\n");
	fv_prev_page();
	SWFEXT_FUNC_END();
}

static INT32S eb_nextpage(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	printf("eb_nextpage\n");
	fv_next_page();
	SWFEXT_FUNC_END();
}

static INT32S eb_getzoomsize(void * handle)
{
	int zoom = 0;
	int zoom_min = 0;
	int zoom_max = 0;
	
	SWFEXT_FUNC_BEGIN(handle);
	//printf("eb_get_zoom\n");
	fv_get_zoom(&zoom, &zoom_min,&zoom_max);
	//printf("zoom=%d\n",zoom);
	Swfext_PutNumber(zoom);
	SWFEXT_FUNC_END();	
}

static INT32S eb_getloadfully(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	//printf("eb_getloadfully\n");
	//printf("doc_loaded_fully=%d\n",doc_loaded_fully);
	Swfext_PutNumber(doc_loaded_fully);
	SWFEXT_FUNC_END();	
}

static INT32S eb_get_errcode(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	//printf("eb_getloadfully\n");
	//printf("doc_loaded_fully=%d\n",doc_loaded_fully);
	Swfext_PutNumber(picsel_err);
	SWFEXT_FUNC_END();	
}

#endif

INT32S swfext_ebook_register(void)
{
	SWFEXT_REGISTER("eb_Open", eb_open);
	SWFEXT_REGISTER("eb_Close", eb_close);
	SWFEXT_REGISTER("eb_GetTotalRows", eb_get_totalrow);
	SWFEXT_REGISTER("eb_GetRow", eb_get_onerow);
	SWFEXT_REGISTER("eb_GetStatus", eb_get_status);
	SWFEXT_REGISTER("eb_GetBookmark", eb_get_bookmark);
	SWFEXT_REGISTER("eb_SetBookmark", eb_set_bookmark);
	SWFEXT_REGISTER("eb_GetFileName", GetFileName);
	SWFEXT_REGISTER("eb_SetFileName", SetFileName);
	SWFEXT_REGISTER("eb_GetColor", eb_get_color);
	SWFEXT_REGISTER("eb_SetColor", eb_set_color);
	SWFEXT_REGISTER("eb_GetBGColor", eb_get_bgcolor);
	SWFEXT_REGISTER("eb_SetBGColor", eb_set_bgcolor);
	SWFEXT_REGISTER("eb_GetFontSize", eb_get_fontsize);
	SWFEXT_REGISTER("eb_SetFontSize", eb_set_fontsize);
	SWFEXT_REGISTER("eb_SaveBookInfo", eb_save_bookinfo);
	SWFEXT_REGISTER("eb_GetBookInfo", eb_get_bookinfo);
	SWFEXT_REGISTER("eb_GetBookMarkName", eb_get_markname);
	#if((EBOOK==HYF)||(EBOOK==PICSEL))
	SWFEXT_REGISTER("eb_open_document", eb_open_document);
	SWFEXT_REGISTER("eb_close_document", eb_close_document);
	SWFEXT_REGISTER("eb_jump_page", eb_jump_page);
	SWFEXT_REGISTER("eb_zomm_in", eb_zomm_in);
	SWFEXT_REGISTER("eb_zoom_out", eb_zoom_out);
	SWFEXT_REGISTER("eb_zoom", eb_zoom);	
	SWFEXT_REGISTER("eb_get_page_total", eb_get_page_total);
	SWFEXT_REGISTER("eb_get_page_no", eb_get_page_no);
	SWFEXT_REGISTER("eb_move", eb_move);
	SWFEXT_REGISTER("eb_outline_exist", eb_outline_exist);
	SWFEXT_REGISTER("eb_outline_size", eb_outline_size);
	SWFEXT_REGISTER("eb_get_outline", eb_get_outline);
	SWFEXT_REGISTER("eb_goto_outline_page", eb_goto_outline_page);
	SWFEXT_REGISTER("eb_rotate", eb_rotate);
	SWFEXT_REGISTER("eb_showmode", eb_showmode);
	SWFEXT_REGISTER("eb_prevpage", eb_prevpage);
	SWFEXT_REGISTER("eb_nextpage", eb_nextpage);
	SWFEXT_REGISTER("eb_getzoomsize", eb_getzoomsize);
	SWFEXT_REGISTER("eb_getloadfully", eb_getloadfully);
	SWFEXT_REGISTER("eb_getErrorCode", eb_get_errcode);
	#endif
	
	return 0;
}
#else
int swfext_ebook_register(void)
{
	return 0;
}
#endif	/** MODULE_CONFIG_EBOOK */
