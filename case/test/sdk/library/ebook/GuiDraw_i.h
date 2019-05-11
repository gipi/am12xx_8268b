#ifndef __GUIDRAW_H__
#define __GUIDRAW_H__

typedef unsigned char   BYTE;
typedef unsigned int   WORD;
typedef unsigned int   LONG;
typedef unsigned int   DWORD;
typedef DWORD   COLORREF;


typedef struct _RECT { 
    LONG left; 
    LONG top; 
    LONG right; 
    LONG bottom; 
}RECT; 


#define RGB(r,g,b)          ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))

#define ARGB_DEMUX(v,a,r,g,b)\
{\
    a = (v) >> 24;\
    r = (v) >> 16;\
    g = (v) >> 8;\
    b = (v);\
}

#define PS_SOLID            0
#define PS_DASH             1       /* -------  */
#define PS_DOT              2       /* .......  */
#define PS_DASHDOT          3       /* _._._._  */
#define PS_DASHDOTDOT       4       /* _.._.._  */

typedef struct tagPOINT {
	LONG x;
	LONG y;
	LONG flag;
} POINT;


typedef struct _line
{
	int xStart;
	int yStart;
	int xEnd;
	int yEnd;
}LINE;
typedef struct _font
{
	int hint;
	//int weight;
	int PointSize;
	COLORREF color;
}FONT;
typedef struct _text
{
	int x;
	int y;
	int direction;
	char *lpszString;
	int w;
	int h;
}TEXT;
typedef struct _pen
{
	int style;
	int nWidth;
	COLORREF crColor;
}PEN;
typedef struct _pie
{
	 int x1;
	 int y1; 
	 int x2;
	 int y2; 
	 int x3; 
	 int y3; 
	 int x4; 
	 int y4; 
}PIE;
typedef struct _arc
{
	int x1;
	int y1; 
	int x2;
	int y2; 
	int x3; 
	int y3; 
	int x4; 
	int y4; 
}ARC;

typedef struct _polygon
{
	POINT *pointArray;
	int index;
	int npoints;
}POLYGON;

typedef enum
{
	MOVETO = 0,
	LINETO  = 1,
	CURVETO = 2,
	CUBICTO = 3,
} POINTFLAG;
typedef struct _path
{
	POINT *pointArray;
	int npoints;
} PATH;
int (*GuiSetPen)(PEN *pen);
int (*GuiSetFont)(FONT* lpLogFont);
int (*GuiSetBrush)(COLORREF crColor);
int (*GuiDeleteBrush)(void);
int (*GuiDrawLine)(LINE* line);
int (*GuiDrawRect)(RECT*rect);
int (*GuiDrawEllipse)(RECT*rect);
int (*GuiDrawPie)(PIE *pie);
int (*GuiDrawArc)(ARC *arc);
int (*GuiDrawPolygon)(POLYGON *Polygon);
int (*GuiDrawPolyline)(POLYGON *Polygon);
int (*GuiDrawText)(TEXT *text);
int (*GuiGetText)(TEXT *text);
int (*GuiDrawPoint)(int x, int y, COLORREF crColor);
void (*GuiDrawImagePic)(void *fp,int xpos,int ypos,int picwidth,int picheight,char*suffixpic);
void (*GuiDrawImage)(const char* filename,int xpos,int ypos,int picwidth,int picheight,char*suffixpic);
int  (*GuiClearScrean)(void);
int (*GuiFillPolygon)(POLYGON *Polygon);
int (*GuiFillEllipse)(RECT*rect);
int (*GuiDrawPath)(PATH* path);
int (*GuiFillPath)(PATH* path);
void (*GuiDrawImagePath)(void *fp,int xpos,int ypos,int picwidth,int picheight,PATH* clip,char*suffixpic);
void (*GuiFillLinear)(RECT* rect, POINT* ptStart, POINT* ptEnd,COLORREF crStart, COLORREF crEnd,int extStart, int extEnd);
void (*GuiFillRadial)(RECT* rect, POINT* ptStart, int radiusStart,POINT* ptEnd, int radiusEnd,COLORREF crStart, COLORREF crEnd, int extStart, int extEnd);

typedef struct _gui_function
{
int (*SetPen)();
int (*SetFont)();
int (*SetBrush)();
int (*DeleteBrush)();
int (*DrawLine)();
int (*DrawRect)();
int (*DrawEllipse)();
int (*DrawPie)();
int (*DrawArc)();
int (*DrawPolygon)();
int (*DrawPolyline)();
int (*DrawText)();
int (*DrawPoint)();
void (*DrawImagePic)();
void (*DrawImage)();
int (*ClearScrean)();
int (*FillPolygon)();
int (*GetText)();
int (*FillEllipse)();
int (*DrawPath)();
int (*FillPath)();
void (*DrawImagePath)();
void (*FillLinear)();
void (*FillRadial)();
//please add function here
}GUIFUNC;//for hyf
#endif
