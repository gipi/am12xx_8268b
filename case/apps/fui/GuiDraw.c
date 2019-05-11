#include "swf_types.h"
#include "GuiDraw.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <memory.h>
#include <fcntl.h>
#include "am_types.h"
#include "ebook_interface.h"
#include "swf_gui.h"
#include "osapi.h"

static PEN 		pen_current;
static FONT 		font_current;
static COLORREF 	brush_current;
static int page_width=800;
static int page_height=600;
static VDC vdc,*dev=&vdc;


#define LCOUNT 32
typedef POINT CvPoint;


static unsigned long isqrt(unsigned long x)
{
	unsigned long m, y, b;

	m = 0x40000000;
	y = 0;
	while(m != 0)		//Do 16 times
	{
		b = y | m;
		y = y >> 1;
		if(x >= b)
		{
			x = x - b;
			y = y | m;
		}
		m = m >> 2;
	}
	return y;
}
  int distance(int x,int y,POINT* p,int radius)
  {
	int d;

	d=isqrt((x-p->x)*(x-p->x)+(y-p->y)*(y-p->y));
	d=d-radius;
	if(d<0)
		d=-d;
	return d;
  }
  
static 
void bezier_q(int x1, int y1,
			  int x2, int y2,
			  int x3, int y3,
			  CvPoint *pts,
			  int *pnum,
			  int n)
{
	if(*pnum < n)
	{
		int x12, y12;
		int x23, y23;
		int x123, y123;
		int dx13, dy13;
		int dx12, dy12;
		int d0, d1;

		x12 = (x1+x2)/2;
		y12 = (y1+y2)/2;

		x23 = (x2+x3)/2;
		y23 = (y2+y3)/2;

		x123 = (x12+x23)/2;
		y123 = (y12+y23)/2;
		
		dx13 = x3-x1;
		dy13 = y3-y1;
		dx12 = x2-x1;
		dy12 = y2-y1;

		d0 = dx13*dy12-dy13*dx12;
		if(d0 < 0)
			d0 = -d0;
		d1 = isqrt(dx13*dx13+dy13*dy13);
		if(d0 <= d1)
		{
			if(pts[*pnum-1].x != x123 || pts[*pnum-1].y != y123)
			{
				pts[*pnum].x = x123;
				pts[*pnum].y = y123;
				(*pnum)++;
			}
			return;
		}
		bezier_q(x1, y1, x12, y12, x123, y123, pts, pnum, n);
		bezier_q(x123, y123, x23, y23, x3, y3, pts, pnum, n);
	}
}
static
int bezier_quadratic(int x1, int y1,
					 int x2, int y2,
					 int x3, int y3,
					 CvPoint *pts,
					 int n)
{
	int num = 1;
	pts[0].x = x1;
	pts[0].y = y1;

	bezier_q(x1, y1, x2, y2, x3, y3, pts, &num, n);

	if(pts[num-1].x != x3  || pts[num-1].y != y3)
	{
		if(num < n)
			num++;
		pts[num-1].x = x3;
		pts[num-1].y = y3;
	}
	return num;
}
static 
void bezier_c(int x1, int y1,
			  int x2, int y2,
			  int x3, int y3,
			  int x4, int y4,
			  CvPoint *pts,
			  int *pnum,
			  int n)
{
	if(*pnum < n)
	{
		int x12, y12;
		int x23, y23;
		int x34, y34;
		int x123, y123;
		int x234, y234;
		int x1234, y1234;
		int dx12, dy12;
		int dx13, dy13;
		int dx14, dy14;
		int d, d2, d3;

		x12 = (x1+x2)/2;
		y12 = (y1+y2)/2;

		x23 = (x2+x3)/2;
		y23 = (y2+y3)/2;

		x34 = (x3+x4)/2;
		y34 = (y3+y4)/2;

		x123 = (x12+x23)/2;
		y123 = (y12+y23)/2;

		x234 = (x23+x34)/2;
		y234 = (y23+y34)/2;

		x1234 = (x123+x234)/2;
		y1234 = (y123+y234)/2;
	
		dx12 = x2-x1;
		dy12 = y2-y1;
		dx13 = x3-x1;
		dy13 = y3-y1;
		dx14 = x4-x1;
		dy14 = y4-y1;

		d2 = dx14*dy12-dy14*dx12;
		if(d2 < 0)
			d2 = -d2;
		d3 = dx14*dy13-dy14*dx13;
		if(d3 < 0)
			d3 = -d3;
		d = isqrt(dx14*dx14+dy14*dy14);
		if(d2 < d3)
			d2 = d3;
		if(d2 <= d)
		{
			if(pts[*pnum-1].x != x1234 || pts[*pnum-1].y != y1234)
			{
				pts[*pnum].x = x1234;
				pts[*pnum].y = y1234;
				(*pnum)++;
			}
			return;
		}
		bezier_c(x1, y1, x12, y12, x123, y123, x1234, y1234, pts, pnum, n);
		bezier_c(x1234, y1234, x234, y234, x34, y34, x4, y4, pts, pnum, n);
	}
}

int bezier_cubic(int x1, int y1,
				 int x2, int y2,
				 int x3, int y3,
				 int x4, int y4,
				 CvPoint *pts,
				 int n)
{
	int num = 1;
	pts[0].x = x1;
	pts[0].y = y1;

	bezier_c(x1, y1, x2, y2, x3, y3, x4, y4, pts, &num, n);

	if(pts[num-1].x != x4  || pts[num-1].y != y4)
	{
		if(num < n)
			num++;
		pts[num-1].x = x4;
		pts[num-1].y = y4;
	}
	return num;
}

static
void path_trans(PATH* out,PATH* in)
{
	int i,j;
	POINT* out_point=out->pointArray; 
	POINT* in_point; 
	int x1=0,y1=0;
	
	
	for(i=0;i<in->npoints;)
	{
		in_point=in->pointArray+i;
		if((in_point->flag==MOVETO)||(in_point->flag==LINETO))
		{
			OSmemcpy(out_point,in_point,sizeof(POINT));
			i++;
			out_point++;
		}else if(in_point->flag==CURVETO)
		{
			int num=0;
			
			num=bezier_quadratic(x1,y1,in_point->x,in_point->y,(in_point+1)->x,(in_point+1)->y,	out_point,LCOUNT*2);
			if(num>=LCOUNT*2)
				OSprintf("LCOUNT is too small!");
			i=i+2;
			for(j=0;j<num;j++,out_point++)	
				out_point->flag=LINETO;
		}else if(in_point->flag==CUBICTO)
		{
			int num=0;
			
			num=bezier_cubic(x1,y1,in_point->x,in_point->y,(in_point+1)->x,(in_point+1)->y,
							(in_point+2)->x,(in_point+2)->y,out_point,LCOUNT*3);
			if(num>=LCOUNT*3)
				OSprintf("LCOUNT is too small!");
			i=i+3;
			for(j=0;j<num;j++,out_point++)	
				out_point->flag=LINETO;
		}
		x1=(out_point-1)->x;
		y1=(out_point-1)->y;
	}
	out->npoints=out_point-out->pointArray;	
}	

EXPORT_SYMBOL
int GuiSetPen(PEN *pen)
{
	//printf("GuiSetPen\n");
	pen_current=*pen;
	return 0;
}
EXPORT_SYMBOL
int GuiSetFont(FONT* lpLogFont)
{
	font_current=*lpLogFont;
	return 0;	
}
EXPORT_SYMBOL
int GuiSetBrush(COLORREF crColor)
{
	brush_current=crColor;
	return 0;
}
EXPORT_SYMBOL
int GuiDeleteBrush()
{
	brush_current=0;
	return 0;
}
EXPORT_SYMBOL
void GuiSetVdc(HVDC dev_info)
{
	dev=NULL;	
}
EXPORT_SYMBOL
int GuiDrawLine(LINE* line)
{
	 UIPATH Upath;
	 int a,r,g,b;
	 UIPOINT pointArray[2];
	
	ARGB_DEMUX(pen_current.crColor,a,b,g,r);
	Upath.color=RGB(b,g,r)|0xff000000;
	Upath.width=pen_current.nWidth*16;
	pointArray[0].x=line->xStart*16;
	pointArray[0].y=line->yStart*16;
	pointArray[0].flag=TYPE_MOVETO;
	pointArray[1].x=line->xEnd*16;
	pointArray[1].y=line->yEnd*16;
	pointArray[1].flag=TYPE_LINETO;
	Upath.pointArray=pointArray;
	Upath.npoints=2;	
  	SWF_GUIDrawLine(dev,&Upath);
	return 0;
}
EXPORT_SYMBOL
int GuiDrawRect(RECT*rect)
{
	 LINE line;

	line.xStart=rect->left;
	line.yStart=rect->top;
	line.xEnd=rect->right;
	line.yEnd=rect->top;
	GuiDrawLine(&line);
	line.xStart=rect->right;
	line.yStart=rect->top;
	line.xEnd=rect->right;
	line.yEnd=rect->bottom;
	GuiDrawLine(&line);
	line.xStart=rect->right;
	line.yStart=rect->bottom;
	line.xEnd=rect->left;
	line.yEnd=rect->bottom;
	GuiDrawLine(&line);
	line.xStart=rect->left;
	line.yStart=rect->bottom;
	line.xEnd=rect->left;
	line.yEnd=rect->top;
	GuiDrawLine(&line);
	return 0;
	
}
static int GuiGetEllipsePath(UIPATH* upath,RECT*rect)
{
	UIPOINT* pointArray;
	int tg= 106;//0.4142*256;
	int sin=181;//0.7071*256;
	int x=(rect->left+rect->right)/2;
	int y=(rect->top+rect->bottom)/2;
	int r=(rect->right-rect->left)/2;
	
	if((rect->right-rect->left)!=(rect->bottom-rect->top))
	{
	pointArray=(UIPOINT*)OSmalloc(sizeof(UIPOINT)*9);
	pointArray[0].x=rect->left*16;
	pointArray[0].y=(rect->top+rect->bottom)/2*16;
	pointArray[0].flag=TYPE_MOVETO;
	pointArray[1].x=rect->left*16;
	pointArray[1].y=rect->top*16;
	pointArray[1].flag=TYPE_CURVETO;		
	pointArray[2].x=(rect->left+rect->right)/2*16;
	pointArray[2].y=rect->top*16;
	pointArray[2].flag=TYPE_CURVETO;
	pointArray[3].x=rect->right*16;
	pointArray[3].y=rect->top*16;
	pointArray[3].flag=TYPE_CURVETO;
	pointArray[4].x=rect->right*16;
	pointArray[4].y=(rect->top+rect->bottom)/2*16;
	pointArray[4].flag=TYPE_CURVETO;
	pointArray[5].x=rect->right*16;
	pointArray[5].y=rect->bottom*16;
	pointArray[5].flag=TYPE_CURVETO;
	pointArray[6].x=(rect->left+rect->right)/2*16;
	pointArray[6].y=rect->bottom*16;
	pointArray[6].flag=TYPE_CURVETO;
	pointArray[7].x=rect->left*16;
	pointArray[7].y=rect->bottom*16;
	pointArray[7].flag=TYPE_CURVETO;
	pointArray[8].x=rect->left*16;
	pointArray[8].y=(rect->top+rect->bottom)/2*16;
	pointArray[8].flag=TYPE_CURVETO;
	upath->pointArray=pointArray;
	upath->npoints=9;	
	}else
	{
		pointArray=(UIPOINT*)OSmalloc(sizeof(UIPOINT)*17);
		pointArray[0].x=(x+r)*16;
		pointArray[0].y=y*16;
		pointArray[0].flag=TYPE_MOVETO;
		pointArray[1].x=(x+r)*16;
		pointArray[1].y=(y+r*tg/256)*16;
		pointArray[1].flag=TYPE_CURVETO;		
		pointArray[2].x=(x+r*sin/256)*16;
		pointArray[2].y=(y+r*sin/256)*16;
		pointArray[2].flag=TYPE_CURVETO;
		pointArray[3].x=(x+r*tg/256)*16;
		pointArray[3].y=(r+y)*16;
		pointArray[3].flag=TYPE_CURVETO;
		pointArray[4].x=x*16;
		pointArray[4].y=(r+y)*16;
		pointArray[4].flag=TYPE_CURVETO;
		pointArray[5].x=(x-r*tg/256)*16;
		pointArray[5].y=(y+r)*16;
		pointArray[5].flag=TYPE_CURVETO;
		pointArray[6].x=(x-r*sin/256)*16;
		pointArray[6].y=(y+r*sin/256)*16;
		pointArray[6].flag=TYPE_CURVETO;
		pointArray[7].x=(x-r)*16;
		pointArray[7].y=(y+r*tg/256)*16;
		pointArray[7].flag=TYPE_CURVETO;
		pointArray[8].x=(x-r)*16;
		pointArray[8].y=y*16;
		pointArray[8].flag=TYPE_CURVETO;	
		pointArray[9].x=(x-r)*16;
		pointArray[9].y=(y-r*tg/256)*16;
		pointArray[9].flag=TYPE_CURVETO;
		pointArray[10].x=(x-r*sin/256)*16;
		pointArray[10].y=(y-r*sin/256)*16;
		pointArray[10].flag=TYPE_CURVETO;		
		pointArray[11].x=(x-r*tg/256)*16;
		pointArray[11].y=(y-r)*16;
		pointArray[11].flag=TYPE_CURVETO;
		pointArray[12].x=(x)*16;
		pointArray[12].y=(y-r)*16;
		pointArray[12].flag=TYPE_CURVETO;
		pointArray[13].x=(x+r*tg/256)*16;
		pointArray[13].y=(y-r)*16;
		pointArray[13].flag=TYPE_CURVETO;
		pointArray[14].x=(x+r*sin/256)*16;
		pointArray[14].y=(y-r*sin/256)*16;
		pointArray[14].flag=TYPE_CURVETO;
		pointArray[15].x=(x+r)*16;
		pointArray[15].y=(y-r*tg/256)*16;
		pointArray[15].flag=TYPE_CURVETO;
		pointArray[16].x=(x+r)*16;
		pointArray[16].y=(y)*16;
		pointArray[16].flag=TYPE_CURVETO;
		upath->pointArray=pointArray;
		upath->npoints=17;
	}
	
	return 0;
}
EXPORT_SYMBOL
int GuiDrawEllipse(RECT*rect)
{
	 UIPATH Upath;
	 int a,r,g,b;
	
	ARGB_DEMUX(pen_current.crColor,a,b,g,r);
	Upath.color=RGB(b,g,r)|0xff000000;
	Upath.width=pen_current.nWidth*16;
	GuiGetEllipsePath(&Upath,rect);
  	SWF_GUIDrawLine(dev,&Upath);
  OSfree(Upath.pointArray);
	return 0;
}
EXPORT_SYMBOL
int GuiDrawPie(PIE *pie)
{
	RECT rect;
	
	rect.left=pie->x1;
	rect.top=pie->y1;
	rect.right=pie->x2;
	rect.bottom=pie->y2;
	GuiDrawEllipse(&rect);
	return 0;
}
EXPORT_SYMBOL
int GuiDrawArc(ARC *arc)
{
	GuiDrawPie((PIE *)arc);
	return 0;
}
EXPORT_SYMBOL
int GuiDrawPolyline(POLYGON *Polygon)
{

	int i;
	LINE line;
	
	for(i=0;i<Polygon->npoints-1;i++)
	{
		line.xStart=Polygon->pointArray[i].x;
		line.yStart=Polygon->pointArray[i].y;
		line.xEnd=Polygon->pointArray[i+1].x;
		line.yEnd=Polygon->pointArray[i+1].y;
		GuiDrawLine(&line);	
	}
	return 0;

	
}
EXPORT_SYMBOL
int GuiDrawPolygon(POLYGON *Polygon)
{
	LINE line;
	
	GuiDrawPolyline(Polygon);
	line.xStart=Polygon->pointArray[0].x;
	line.yStart=Polygon->pointArray[0].y;
	line.xEnd=Polygon->pointArray[Polygon->npoints-1].x;
	line.yEnd=Polygon->pointArray[Polygon->npoints-1].y;
	GuiDrawLine(&line);
	return 0;
	
}
EXPORT_SYMBOL
int GuiDrawText(TEXT *text)
{

	 int a,r,g,b;
	 UITEXT Utext;

	//printf("GuiDrawText start\n");
	ARGB_DEMUX(font_current.color,a,b,g,r);
	Utext.color=RGB(b,g,r)|0xff000000;	
       Utext.size=font_current.PointSize;
 	Utext.x=text->x;
 	Utext.y=text->y;
 	Utext.string=text->lpszString;
	SWF_GUIDrawText(dev,&Utext);

	//printf("GuiDrawText end\n");	
	return 0;
}
EXPORT_SYMBOL
int GuiDrawPoint(int x, int y, COLORREF crColor)
{
      
 	 UIPATH Upath;
	 int a,r,g,b;
	 UIPOINT pointArray[2];
	 	
	ARGB_DEMUX(crColor,a,b,g,r);
	Upath.color=RGB(b,g,r)|0xff000000;
	pointArray[0].x=x*16;
	pointArray[0].y=y*16;
	pointArray[0].flag=TYPE_MOVETO;
	pointArray[1].x=(x+1)*16;
	pointArray[1].y=(y+1)*16;
	pointArray[1].flag=TYPE_LINETO;
	Upath.pointArray=pointArray;
	Upath.npoints=2;
	Upath.width=16;
	SWF_GUIDrawLine(dev,&Upath);      
	return 0;
}
static int SaveFile(void* fp,char* suffixpic)
{
	int buf_size;
	char* buf;
	void* tmp;
	static int no=0;
	char filename[60];
	
	OSfseek_set(fp,0);
	OSfseek_end(fp, 0);
	buf_size=OSftell(fp);
	buf=(char *)OSmalloc(buf_size);
	if(buf!=NULL)
	{
		OSprintf("buf_size=%x\n",buf_size);
		OSfseek_set(fp,0);
		OSfread(buf, 1, buf_size, fp);
		OSsprintf(filename, "/mnt/udisk/%d%s", no,suffixpic);
		tmp=(void*)OSfopen(filename,"wb+");
		if(tmp!=NULL)
		{
			OSprintf("%s out ok\n",filename);
			OSfwrite(buf,1, buf_size, tmp);
			OSfclose(tmp);
			no++;
		}else
			OSprintf("SaveFile OSfopen error\n");
	}else
		OSprintf("SaveFile OSmalloc error\n");
	OSfree(buf);
	return 0;
}

EXPORT_SYMBOL
void GuiDrawImage(const char* filename,int xpos,int ypos,int picwidth,int picheight,char*suffixpic)
{
	//return 0;
}
static int GuiGetPolygonPath(UIPATH* upath,POLYGON *Polygon)
{
	 UIPOINT* pointArray;
	 int i;
	 	
		pointArray=(UIPOINT*)OSmalloc(sizeof(UIPOINT)*(Polygon->npoints+1));
		for(i=0;i<Polygon->npoints+1;i++)
		{
			if(i==0)
			{
				pointArray[i].x=Polygon->pointArray[i].x*16;
				pointArray[i].y=Polygon->pointArray[i].y*16;
				pointArray[i].flag=TYPE_MOVETO;
			}else if(i==Polygon->npoints)
			{
				pointArray[i].x=Polygon->pointArray[0].x*16;
				pointArray[i].y=Polygon->pointArray[0].y*16;
				pointArray[i].flag=TYPE_LINETO;
			}else
			{
				pointArray[i].x=Polygon->pointArray[i].x*16;
				pointArray[i].y=Polygon->pointArray[i].y*16;
				pointArray[i].flag=TYPE_LINETO;
			}
		}
		upath->pointArray=pointArray;
		upath->npoints=Polygon->npoints+1;	
		return 0;
}
EXPORT_SYMBOL
int GuiFillPolygon(POLYGON *Polygon)
{
	
	 UIPATH Upath;
	 int a,r,g,b;
	 	
	ARGB_DEMUX(brush_current,a,b,g,r);
	Upath.color=RGB(b,g,r)|0xff000000;
	GuiGetPolygonPath(&Upath,Polygon);
	SWF_GUIFillPolygon(dev,&Upath);
	OSfree(Upath.pointArray);
	return 0;
}
EXPORT_SYMBOL
int GuiFillEllipse(RECT*rect)
{
	UIPATH Upath;
	int a,r,g,b;
	 	
	ARGB_DEMUX(brush_current,a,b,g,r);
	Upath.color=RGB(b,g,r)|0xff000000;
	GuiGetEllipsePath(&Upath,rect);
	SWF_GUIFillPolygon(dev,&Upath);
	OSfree(Upath.pointArray);
	return 0;
}
EXPORT_SYMBOL
int GuiClearScrean()
{
	POLYGON polygon;
	POINT pointArray[4];

	OSprintf("GuiClearScrean\n");

	GuiSetBrush(0xffffff);
	 pointArray[0].x=0;
	 pointArray[0].y=0;
	 pointArray[1].x=LCD_WIDTH_E;
	 pointArray[1].y=0;  
	 pointArray[2].x=LCD_WIDTH_E;
	 pointArray[2].y=LCD_HEIGHT_E;
	 pointArray[3].x=0;
	 pointArray[3].y=LCD_HEIGHT_E;
	 
	 polygon.pointArray=pointArray;
	 polygon.npoints=4;
	GuiFillPolygon(&polygon);
	return 0;
}


EXPORT_SYMBOL
int GuiGetText(TEXT *text)
{

	UITEXT Utext;

	Utext.size=font_current.PointSize;
 	Utext.string=text->lpszString;
  	SWF_GUIGetText(dev,&Utext);
	text->w=Utext.w;
	text->h=Utext.h;

	return 0;
}	
EXPORT_SYMBOL
int GuiDrawPath(PATH* in_path)
{
	PATH Path,*path=&Path;
	UIPATH Upath;
	int a,r,g,b;
	UIPOINT *pointArray;
	int i;

	ARGB_DEMUX(pen_current.crColor,a,b,g,r);
	Upath.color=RGB(b,g,r)|0xff000000;
	Upath.width=pen_current.nWidth*16;
	path->pointArray=(POINT *)OSmalloc(sizeof(POINT)*(in_path->npoints)*LCOUNT);
	path_trans(path,in_path);
	#if 0
	pointArray=(UIPOINT *)OSmalloc(sizeof(UIPOINT)*(path->npoints));
	for(i=0;i<path->npoints;i++)
	{
		pointArray[i].x=(path->pointArray)[i].x*16;
		pointArray[i].y=(path->pointArray)[i].y*16;
		pointArray[i].flag=(path->pointArray)[i].flag;
	}
	Upath.pointArray=pointArray;
	#else
	Upath.pointArray=(UIPOINT*)(path->pointArray);
	#endif
	Upath.npoints=path->npoints;
	SWF_GUIDrawLine(dev,&Upath);
	//OSfree(pointArray);
	OSfree(path->pointArray);

	return 0;
}

EXPORT_SYMBOL
int GuiFillPath(PATH* in_path)
{
	PATH Path,*path=&Path;
	UIPATH Upath;
	int a,r,g,b;
	UIPOINT *pointArray;
	int i;
	
	ARGB_DEMUX(brush_current,a,b,g,r);
	Upath.color=RGB(b,g,r)|0xff000000;
	path->pointArray=(POINT *)OSmalloc(sizeof(POINT)*(in_path->npoints)*LCOUNT);
	path_trans(path,in_path);
	#if 0
	pointArray=OSmalloc(sizeof(UIPOINT)*(path->npoints));
	for(i=0;i<path->npoints;i++)
	{
		pointArray[i].x=(path->pointArray)[i].x*16;
		pointArray[i].y=(path->pointArray)[i].y*16;
		pointArray[i].flag=(path->pointArray)[i].flag;
	}
	Upath.pointArray=pointArray;
	#else
	Upath.pointArray=(UIPOINT*)(path->pointArray);
	#endif
	Upath.npoints=path->npoints;	
  SWF_GUIFillPolygon(dev,&Upath);
  //OSfree(pointArray);
  OSfree(path->pointArray);
  
	return 0;
}
EXPORT_SYMBOL
void GuiDrawImagePath(void *fp,int xpos,int ypos,int picwidth,int picheight,PATH* in_path,char*suffixpic)
{
	PATH Path,*path=&Path;
	UIPIC Upic;
	UIPOINT *pointArray;
	
	int i;
	//OSprintf("GuiDrawImagePath\n");
	//SaveFile(fp,suffixpic);
	OSfseek_set(fp,0);
	Upic.filename=suffixpic;
	Upic.fp=fp;
	Upic.x=xpos/2*2;
	Upic.y=ypos;
	Upic.width=picwidth;
	Upic.height=picheight;	
	path->pointArray=(POINT *)OSmalloc(sizeof(POINT)*(in_path->npoints)*LCOUNT);
	path_trans(path,in_path);
	#if 0	
	pointArray=OSmalloc(sizeof(UIPOINT)*(path->npoints));
	for(i=0;i<path->npoints;i++)
	{
		pointArray[i].x=(path->pointArray)[i].x*16;
		pointArray[i].y=(path->pointArray)[i].y*16;
		pointArray[i].flag=(path->pointArray)[i].flag;
	}
	Upic.clippath.pointArray=pointArray;
	#else
	Upic.clippath.pointArray=(UIPOINT*)(path->pointArray);
	#endif
	Upic.clippath.npoints=path->npoints;
	SWF_GUILoadPic(dev,&Upic);
	//OSfree(pointArray);
	OSfree(path->pointArray);
}
EXPORT_SYMBOL
void GuiDrawImagePic(void *fp,int xpos,int ypos,int picwidth,int picheight,char*suffixpic)
{
	PATH clippath;
	POINT pointArray[5];
	
	pointArray[0].x=0*16;
	pointArray[0].y=0*16;
	pointArray[0].flag=TYPE_MOVETO;
	pointArray[1].x=picwidth*16;
	pointArray[1].y=0*16;
	pointArray[1].flag=TYPE_LINETO;
	pointArray[2].x=picwidth*16;
	pointArray[2].y=picheight*16;
	pointArray[2].flag=TYPE_LINETO;
	pointArray[3].x=0*16;
	pointArray[3].y=picheight*16;
	pointArray[3].flag=TYPE_LINETO;
	pointArray[4].x=0*16;
	pointArray[4].y=0*16;
	pointArray[4].flag=TYPE_LINETO;
	clippath.pointArray=pointArray;
	clippath.npoints=5;
	GuiDrawImagePath(fp,xpos,ypos,picwidth,picheight,&clippath,suffixpic);
}

#pragma pack(1)
typedef struct _bmp_header_
{
	/* 00h */	unsigned short BM;			
	/* 02h */	unsigned long file_size;	
	/* 06h */	unsigned short reserved0;	
	/* 08h */	unsigned short reserved1;	
	/* 0Ah */	unsigned long data_offset;	
	/* 0Eh */	unsigned long header_size;	
	/* 12h */	unsigned long width;		
	/* 16h */	long height;				
	/* 1Ah */	unsigned short planes;
	/* 1Ch */	unsigned short num_of_bits;
	/* 1Eh */	unsigned long compression;
	/* 22h */	unsigned long bmp_data_size;
	/* 26h */	unsigned long hor_resolution;
	/* 2Ah */	unsigned long ver_resolution;
	/* 2Eh */	unsigned long palette_colors;
	/* 32h */	unsigned long important_colors;
}BMP_HEADER;
#pragma pack()

static 
int output_bmp_RGB888(void* fp,unsigned char *img, int width, int height, int x, int y, int step)
{
	BMP_HEADER hdr;
	//char fname[128];
	void *fptr = NULL;
	//unsigned long time_sec;
	int i;

	// create BMP file header //////////////////////////////////////////////////////////////////////////
	hdr.BM					= 0x4d42;
	hdr.file_size			= (((width*3+3)/4)*4)*height+((sizeof(BMP_HEADER)+3)/4)*4;
	hdr.reserved0			= 0;
	hdr.reserved1			= 1;
	hdr.data_offset			= sizeof(BMP_HEADER);
	hdr.header_size			= 40;
	hdr.width				= width;
	hdr.height				= -height;
	hdr.planes				= 1;
	hdr.num_of_bits			= 24;
	hdr.compression			= 0;
	hdr.bmp_data_size		= (((width*3+3)/4)*4)*height;
	hdr.hor_resolution		= 0;
	hdr.ver_resolution		= 0;
	hdr.palette_colors		= 0;
	hdr.important_colors	= 0;
	//////////////////////////////////////////////////////////////////////////
	
	//create new BMP file
	//OSGetTime(&time_sec, NULL);
	//OSsprintf(fname, "%dx%d_%08X.bmp", width, height, time_sec);
	//OSprintf("output file name: %s\n", fname);
	//fptr = OSfopen(fname, "wb");
	fptr=fp;
	if(fptr != NULL)
	{
		unsigned char *p = img;
		int line_size = width*3;
		unsigned char pad[4] = {0, 0, 0, 0};

		//write BMP file header
		OSfwrite(&hdr, sizeof(char), sizeof(BMP_HEADER), fptr);

		//write BMP pixel data
		if(x)
			p += x*3;
		if(y)
			p += y*step;
		if(line_size%4)
		{
			int pad_size = 4-(line_size%4);
			for(i=0;i<height;i++)
			{
				OSfwrite(p, sizeof(char), line_size, fptr);
				OSfwrite(pad, sizeof(char), pad_size, fptr);
				p += step;
			}
		}
		else
		{
			for(i=0;i<height;i++)
			{
				OSfwrite(p, sizeof(char), line_size, fptr);
				p += step;
			}
		}
		if(sizeof(BMP_HEADER)%4)
		{
			int pad_size = 4-(sizeof(BMP_HEADER)%4);
			OSfwrite(pad, sizeof(char), pad_size, fptr);
		}

		//close file
		//OSfclose(fptr);
		fptr = NULL;
		//OSprintf("BMP output finished\n");
	}

	return 0;
}

void fillLine(void* fp,int width,int height,POINT* ptStart, POINT* ptEnd,int crStart, int crEnd)
{
	int buf_size=width*height*3;
	unsigned char* buf=(unsigned char*)OSmalloc(buf_size);
	int x,y;
	int a;
	int x1,y1;
	unsigned char a1,r1,g1,b1;
	int x2,y2;
	unsigned char a2,r2,g2,b2;
	int x0,y0;
	int d1,d2;
	int k=0;
	
	x1=ptStart->x;
	y1=ptStart->y;
	x2=ptEnd->x;
	y2=ptEnd->y;
	if((x2-x1)!=0)
		k=(y2-y1)*256/(x2-x1);
	ARGB_DEMUX(crStart,a1,b1,g1,r1);
	ARGB_DEMUX(crEnd,a2,b2,g2,r2);
//	OSmemset(buf,0xf0,buf_size);
	for(y=height-1;y>=0;y--)
		for(x=0;x<width;x++)
		{
			if(k==0)
				x0=x;
			else if(x1==x2)
				x0=x1;
			else
				x0=((y-y2)*256+k*(x2+x))/(2*k);
			if((x2-x1)!=0)
				y0=y2-k*(x2-x0)/256;
			else
				y0=y;
			d1=(y0-y1)*(y0-y1)+(x0-x1)*(x0-x1);
			d2=(y0-y2)*(y0-y2)+(x0-x2)*(x0-x2);
			if(x==y)
				x=y;
			if((d1+d2)==0)
				a=256;
			else
				a=d2*256/(d1+d2);
			*(buf+(y*width+x)*3+2)=r2+a*(r1-r2)/256;
			*(buf+(y*width+x)*3+1)=g2+a*(g1-g2)/256;
			*(buf+(y*width+x)*3)=b2+a*(b1-b2)/256;			
		}
	output_bmp_RGB888(fp,buf, width, height, 0, 0, width*3);
	OSfree(buf);
}


 void fillRadial(void* fp,int width,int height,POINT* ptStart, int radiusStart, POINT* ptEnd, int radiusEnd,int crStart, int crEnd)
{
	int buf_size=width*height*3;
	unsigned char* buf=(unsigned char*)OSmalloc(buf_size);
	int x,y;
	int a;
	unsigned char a1,r1,g1,b1;
	unsigned char a2,r2,g2,b2;
	int d1,d2;

	ARGB_DEMUX(crStart,a1,b1,g1,r1);
	ARGB_DEMUX(crEnd,a2,b2,g2,r2);
//	OSmemset(buf,0xf0,buf_size);
	for(y=height-1;y>=0;y--)
		for(x=0;x<width;x++)
		{
			d1=distance(x,y,ptStart,radiusStart);
			d2=distance(x,y,ptEnd,radiusEnd);
			if((d1+d2)==0)
				a=256;
			else
				a=d2*256/(d1+d2);
			*(buf+(y*width+x)*3+2)=r2+a*(r1-r2)/256;
			*(buf+(y*width+x)*3+1)=g2+a*(g1-g2)/256;
			*(buf+(y*width+x)*3)=b2+a*(b1-b2)/256;			
		}
	output_bmp_RGB888(fp,buf, width, height, 0, 0, width*3);
	OSfree(buf);
}
/*
	渐进区域按线性填充
	 * @param	rect		填充区域的外接矩形
	 * @param	ptStart		填充区域的起始点
	 * @param	ptEnd		填充区域的结束点
	 * @param	crStart		起始颜色
	 * @param	crEnd		结束颜色
	 * @param	extStart		是否为渐变的第一个边界点
	 * @param	extEnd			是否为渐变的最后一个边界点
	
	 */
  void GuiFillLinear(RECT* rect, POINT* ptStart, POINT* ptEnd,
									COLORREF crStart, COLORREF crEnd,int extStart, int extEnd)
  {
		int width,height;
		void *fp;
		POINT ps,pe;
		int scale=10;

		width=(rect->right-rect->left)/scale;
		height=(rect->bottom-rect->top)/scale;
		ps.x=(ptStart->x-rect->left)/scale;
		ps.y=(ptStart->y-rect->top)/scale;
		pe.x=(ptEnd->x-rect->left)/scale;
		pe.y=(ptEnd->y-rect->top)/scale;
		//fp=OSfopen("/mnt/udisk/test.bmp","wb+");
		fp=(void*)OStmpfile();
		fillLine(fp,width,height,&ps, &pe,crStart,crEnd);
		GuiDrawImagePic(fp,rect->left,rect->top,rect->right-rect->left,rect->bottom-rect->top,".bmp");
		OSfclose(fp);
  } 
/**
	 * 渐进区域按辐射状填充
	 * @param	rect			填充区域的外接矩形
	 * @param	ptStart			填充区域的起始圆心
	 * @param	radiusStart		起始半径
	 * @param	ptEnd			填充区域的结束圆心
	 * @param	radiusEnd		结束半径
	 * @param	crStart			起始颜色
	 * @param	crEnd			结束颜色
	 * @param	extStart		是否为渐变的第一个边界点
	 * @param	extEnd			是否为渐变的最后一个边界点
*/
 
	void GuiFillRadial(RECT* rect, POINT* ptStart, int radiusStart,
									POINT* ptEnd, int radiusEnd,
									COLORREF crStart, COLORREF crEnd, int extStart, int extEnd)
	{
		int width,height;
		void *fp;
		POINT ps,pe;
		int rs,re;
		int scale=10;

		width=(rect->right-rect->left)/scale;
		height=(rect->bottom-rect->top)/scale;
		ps.x=(ptStart->x-rect->left)/scale;
		ps.y=(ptStart->y-rect->top)/scale;
		rs=radiusStart/scale;
		pe.x=(ptEnd->x-rect->left)/scale;
		pe.y=(ptEnd->y-rect->top)/scale;	
		re=radiusEnd/scale;
		//fp=OSfopen("/mnt/udisk/test.bmp","wb+");
		fp=(void*)OStmpfile();
		fillRadial(fp,width,height,&ps,rs,&pe,re,crStart,crEnd);
		GuiDrawImagePic(fp,rect->left,rect->top,rect->right-rect->left,rect->bottom-rect->top,".bmp");
		OSfclose(fp);	

	}

