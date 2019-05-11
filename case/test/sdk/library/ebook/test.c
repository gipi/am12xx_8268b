

#include "am_types.h"
#include "ebook_interface.h"
#include "GuiDraw_i.h"



EXPORT_SYMBOL
int InitProgram(void* gui)
{
	GUIFUNC* gui_func;
	
	OSprintf("InitProgram  ok\n");
	gui_func=(GUIFUNC*)gui;
	GuiSetPen=gui_func->SetPen;
	GuiSetBrush=gui_func->SetBrush;
	GuiSetFont=gui_func->SetFont;
	GuiDeleteBrush=gui_func->DeleteBrush;
	GuiDrawLine=gui_func->DrawLine;
	GuiDrawRect=gui_func->DrawRect;
	GuiDrawEllipse=gui_func->DrawEllipse;
	GuiDrawPie=gui_func->DrawPie;
	GuiDrawArc=gui_func->DrawArc;
	GuiDrawPolygon=gui_func->DrawPolygon;
	GuiDrawPolyline=gui_func->DrawPolyline;
	GuiDrawText=gui_func->DrawText;
	GuiGetText=gui_func->GetText;
	GuiDrawPoint=gui_func->DrawPoint;
	GuiDrawImagePic=gui_func->DrawImagePic;	
	GuiDrawImage=gui_func->DrawImage;
	GuiClearScrean=gui_func->ClearScrean;	
	GuiFillPolygon=gui_func->FillPolygon;
	GuiFillEllipse=gui_func->FillEllipse;
	GuiDrawPath=gui_func->DrawPath;	
	GuiFillPath=gui_func->FillPath;
	GuiDrawImagePath=gui_func->DrawImagePath;
	GuiFillLinear=gui_func->FillLinear;
	GuiFillRadial=gui_func->FillRadial;

		
	
	return 0;
}
EXPORT_SYMBOL
int CloseProgram()
{
	return 0;
}
static void test_GuiDrawLine()
{
	PEN pen;
	LINE line;
	
	pen.nWidth=2;
	pen.crColor=0xff0000ff;
	GuiSetPen(&pen);	
	GuiClearScrean();
	line.xStart=800;
	line.yStart=600;	
	line.xEnd=0;
	line.yEnd=600;
	GuiDrawLine(&line);
};
static void test_malloc()
{
	unsigned char* buf;
	buf=(unsigned char*)OSmalloc(8*1024*1024);
	if(buf!=0)
	OSprintf("OSmalloc ok\n");
	else
	OSprintf("OSmalloc error\n");
	OSfree(buf);
	OSprintf("OSfree ok\n");

}	

static void test_GuiDrawText()
{
	FONT font;
	TEXT text;	
  char buf_wendang[10] = {0x87, 0x65, 0x63, 0x68, 0};
	char buf_doc[10] = {0x64, 0x0, 0x6f, 0x0,0x63,0x0,0};	
	
	font.color=0xff00ff00;
	font.PointSize=30;
	GuiSetFont(&font);
	text.x=20;
	text.y=20;	
	text.lpszString=buf_doc;
	GuiDrawText(&text);
	GuiGetText(&text);
	
	GuiClearScrean();  	
	text.x=780;
	text.y=100;
	GuiDrawText(&text);  	
	
	text.x=20;
	text.y=580;
	GuiDrawText(&text);
	
	font.color=0xff000000;
	font.PointSize=40;
	GuiSetFont(&font);
	text.x=200;
	text.y=200;	
	text.lpszString=buf_wendang;
	GuiDrawText(&text);
	GuiGetText(&text);
	OSprintf("font w=%d h=%d\n",text.w,text.h); 
}
static void test_GuiFillPolygon()
{
	POINT pointArray[6];
	POLYGON polygon;
			
	GuiSetBrush(0xffff0000);
	//GuiSetBrush(0xffffffff);
	pointArray[0].x=750;
	pointArray[0].y=550;
	pointArray[1].x=850;
	pointArray[1].y=550;  
	pointArray[2].x=850;
	pointArray[2].y=650;
	pointArray[3].x=800;
	pointArray[3].y=650;
	polygon.pointArray=pointArray;
	polygon.npoints=4;
	GuiFillPolygon(&polygon);
}
static void test_GuiDrawPolygon()
{
	PEN pen;
	POINT pointArray[6];
	POLYGON polygon;
		
	pen.crColor=0xff0000ff;
	GuiSetPen(&pen);
	pointArray[0].x=300;
	pointArray[0].y=300;
	pointArray[1].x=400;
	pointArray[1].y=300;  
	pointArray[2].x=400;
	pointArray[2].y=400;
	pointArray[3].x=350;
	pointArray[3].y=400;
	polygon.pointArray=pointArray;
	polygon.npoints=4;
	GuiDrawPolygon(&polygon);
}
static void test_GuiFillEllipse()
{
	RECT rect;
	
	rect.left=300;
	rect.top=100;
	rect.right=400;
	rect.bottom=200;
	GuiFillEllipse(&rect);
	GuiDrawEllipse(&rect);
}
static void test_GuiDrawImagePath()
{
	char* filename = "/mnt/udisk/test.jpg";
	void *fp,*tmp;
	char* buf;
	int buf_size;
	LINE line;
	int i,j;
	PATH path;
	POINT pointArray[6];
	
	fp=(void *)OSfopen(filename,"rb");
	OSprintf("fp=%d\n",fp);
	if(fp!=NULL)
	{
	OSfseek_end(fp, 0);
	buf_size=OSftell(fp);
	OSfseek_set(fp,0);
	buf=(char *)OSmalloc(buf_size);
	OSfread(buf, 1, buf_size, fp);
	tmp=(void*)OStmpfile();
	//tmp=(void*)OSfopen("/mnt/udisk/a.jpg","wb+");
	OSfwrite(buf,1, buf_size, tmp);
	OSfseek_set(fp,0);	
	OSfseek_set(tmp,0);
	#if 0
	pointArray[0].x=0*16;
	pointArray[0].y=50*16;
	pointArray[0].flag=MOVETO;
	pointArray[1].x=0*16;
	pointArray[1].y=0*16; 
	pointArray[1].flag=CURVETO; 
	pointArray[2].x=50*16;
	pointArray[2].y=0*16;
	pointArray[2].flag=CURVETO; 
	pointArray[3].x=100*16;
	pointArray[3].y=0*16;
	pointArray[3].flag=CURVETO; 	
	pointArray[4].x=100*16;
	pointArray[4].y=50*16;
	pointArray[4].flag=CURVETO; 
	pointArray[5].x=0*16;
	pointArray[5].y=50*16;
	pointArray[5].flag=LINETO; 		
	path.pointArray=pointArray;
	path.npoints=6;
	#else
	pointArray[0].x=0*16;
	pointArray[0].y=0*16;
	pointArray[0].flag=MOVETO;
	pointArray[1].x=100*16;
	pointArray[1].y=0*16; 
	pointArray[1].flag=LINETO; 
	pointArray[2].x=100*16;
	pointArray[2].y=100*16;
	pointArray[2].flag=LINETO; 
	pointArray[3].x=0*16;
	pointArray[3].y=100*16;
	pointArray[3].flag=LINETO; 	
	pointArray[4].x=0*16;
	pointArray[4].y=0*16;
	pointArray[4].flag=LINETO; 	
	path.pointArray=pointArray;
	path.npoints=5;
	#endif
	GuiDrawImagePath(tmp,0,0,600,300,&path,".jpg");	
	OSfclose(tmp);
	OSfclose(fp);
	}	
	
}
static void test_GuiDrawPoint()
{
	int i,j;
	
	for(i=300;i<400;i++)
		for(j=300;j<400;j++)
			GuiDrawPoint(i,j,0xff00);	

	for(i=100;i<200;i++)
		GuiDrawPoint(i,i,0xff00);	

}
static void test_GuiDrawPath()
{
	PEN pen;
	POINT pointArray[6];
	PATH path;
	
	pen.crColor=0xff00ffff;
	pen.nWidth=2;
	GuiSetPen(&pen);
	pointArray[0].x=400*16;
	pointArray[0].y=400*16;
	pointArray[0].flag=MOVETO;
	pointArray[1].x=400.1*16;
	pointArray[1].y=300.2*16; 
	pointArray[1].flag=CURVETO; 
	pointArray[2].x=500*16;
	pointArray[2].y=300*16;
	pointArray[2].flag=CURVETO; 
	pointArray[3].x=600*16;
	pointArray[3].y=300*16; 
	pointArray[3].flag=CURVETO; 
	pointArray[4].x=600*16;
	pointArray[4].y=400*16;
	pointArray[4].flag=CURVETO; 		
			
	path.pointArray=pointArray;
	path.npoints=5;
	GuiDrawPath(&path);
}

static void test_GuiDrawPath_cubic()
{
	PEN pen;
	POINT pointArray[4];
	PATH path;
	
	pen.crColor=0xff00ffff;
	pen.nWidth=2;
	GuiSetPen(&pen);

		
	pointArray[0].x=100;
	pointArray[0].y=100;
	pointArray[0].flag=MOVETO;
	pointArray[1].x=100;
	pointArray[1].y=0; 
	pointArray[1].flag=CUBICTO; 
	pointArray[2].x=200;
	pointArray[2].y=0;
	pointArray[2].flag=CUBICTO; 	
	pointArray[3].x=200;
	pointArray[3].y=100;
	pointArray[3].flag=CUBICTO; 		
	path.pointArray=pointArray;
	path.npoints=4;
	GuiDrawPath(&path);
}
static void test_GuiFillPath()
{
	POINT pointArray[6];
	PATH path;
	
	GuiSetBrush(0xff00ff00);
	pointArray[0].x=500*16;
	pointArray[0].y=300*16;
	pointArray[0].flag=MOVETO;
	pointArray[1].x=600*16;
	pointArray[1].y=300*16; 
	pointArray[1].flag=CURVETO; 
	pointArray[2].x=600*16;
	pointArray[2].y=400*16;
	pointArray[2].flag=CURVETO; 
	pointArray[3].x=500*16;
	pointArray[3].y=300*16;
	pointArray[3].flag=LINETO; 	
	path.pointArray=pointArray;
	path.npoints=4;
	GuiFillPath(&path);
}
static void test_GuiFillLinear()
{
	RECT rect;
	POINT ptStart,ptEnd;
	COLORREF crStart,crEnd;
	int extStart,extEnd;
	
  rect.left=100; 
  rect.top=100; 
  rect.right=400; 
  rect.bottom=400; 
  ptStart.x=100;
  ptStart.y=100;
  ptEnd.x=400;
  ptEnd.y=400;
  crStart=0x0;
  crEnd=0xffffff;
	GuiFillLinear(&rect,&ptStart,&ptEnd,crStart,crEnd,0,0);
}
static void test_GuiFillRadial()
{
	RECT rect;
	POINT ptStart,ptEnd;
	COLORREF crStart,crEnd;
	int radiusStart,radiusEnd;
	int extStart,extEnd;
	
 	rect.left=100; 
  rect.top=100; 
  rect.right=500; 
  rect.bottom=500; 
  ptStart.x=300;
  ptStart.y=300;
  radiusStart=100;
  ptEnd.x=300;
  ptEnd.y=300;
  radiusEnd=200;
  crStart=0x0;
  crEnd=0xffffff;
	GuiFillRadial(&rect,&ptStart,radiusStart,&ptEnd,radiusEnd,crStart,crEnd,0,0);
}
EXPORT_SYMBOL
int OpenDocument(char *fname, int width,int height,int flag)
{


	POINT pointArray[6];
	POLYGON polygon;


	
	OSprintf("OpenDocument  start\n");
	GuiClearScrean();
	test_GuiDrawLine();
	test_malloc();
	test_GuiDrawText();
	test_GuiFillPolygon();	
	test_GuiDrawPolygon();
	test_GuiFillEllipse();
	test_GuiDrawImagePath();
	test_GuiDrawPoint();
	test_GuiDrawPath();
	test_GuiDrawPath_cubic();
	test_GuiFillPath();
	test_GuiFillLinear();
	test_GuiFillRadial();
	OSprintf("OpenDocument end\n");
	return 0;
}
EXPORT_SYMBOL
int CloseDocument()
{
	OSprintf("CloseDocument  ok\n");
	GuiClearScrean();
	return 0;
}
EXPORT_SYMBOL
int GetDocInfo(DOC_INFO *pInfo)
{
	return 0;
}
EXPORT_SYMBOL
int GetPageInfo(PAGE_INFO *pInfo)
{
	return 0;
}
EXPORT_SYMBOL
int JumpPage(int pageNum)
{
	//GuiClearScrean();
	OSprintf("JumpPage %d ok\n",pageNum);
	return 0;
}
EXPORT_SYMBOL
int FindTextBegin(char* text, int iscase, int isforward)
{
	return 0;
}
EXPORT_SYMBOL
int FindNext()
{
	return 0;
}
EXPORT_SYMBOL
int ZoomIn()
{
	OSprintf("ZoomIn  ok\n");
	return 0;
}
EXPORT_SYMBOL
int ZoomOut()
{
	OSprintf("ZoomOut  ok\n");
	return 0;
}
EXPORT_SYMBOL
int MoveUp()
{
	return 0;
}
EXPORT_SYMBOL
int MoveDown()
{
	return 0;
}
EXPORT_SYMBOL
int MoveLeft()
{
	return 0;
}
EXPORT_SYMBOL
int MoveRight()
{
	return 0;
}
EXPORT_SYMBOL
int SetTextProperty(int fontsize)
{
	return 0;
}

EXPORT_SYMBOL
int GetSumPageNum()
{
	return 100;
}

EXPORT_SYMBOL
int ScalePage(int scale)
{
	return 0;
}
EXPORT_SYMBOL
int GetCurrentPageNum(void)
{
	return 0;
}

EXPORT_SYMBOL
int MoveOffset(int x,int y)
{
	return 0;
}
EXPORT_SYMBOL
char** GetOutlinesContent()
{
		return 0;
}
EXPORT_SYMBOL
int	 GetOutlinesSize()
{
		return 0;
}
EXPORT_SYMBOL
int	 OutlinesExist()
{
		return 0;
}
EXPORT_SYMBOL
int	 GotoOutlinesPage(int index)
{
		return 0;
}
