#ifndef _RB_GUI_H
#define _RB_GUI_H

#ifdef MODULE_CONFIG_RB_GUI

#include <stdio.h>
#include <stdlib.h>
#include "swf_gui.h"
#include "am_types.h"


typedef struct _rbCOLOR {
	unsigned int alpha:8;
	unsigned int red:8;
	unsigned int green:8;
	unsigned int blue:8;
} rbCOLOR;

typedef struct _RECT {
	long left;
	long top;
	long right;
	long bottom;
}RECT;


/**
* @brief type:
*     0:16bit bitmap(565)
*     1:24bit bitmap
*     2:png
*     3:gif
*     4:jpg
*     5:raw data (565)
*/
typedef struct _rbIMAGE {
	int type;             ///> 图片类型
	int width;            ///> 图片宽度
	int height;           ///> 图片高度
	int datasize;         ///> 图片数据大小
	int reserved;         ///> 保留位
	char *data;           ///> 图片数据
}rbIMAGE;

extern HVDC rbCreateVDC(int width,int height);

extern int rbDeleteVDC(HVDC hvDC);

extern int rbSetTextColor(HVDC hvDC,rbCOLOR color);

extern int rbSetBkColor(HVDC hvDC,rbCOLOR color);

extern int rbFillRect(HVDC hvDC,RECT *lprc,rbCOLOR color);

extern int rbTextOut(HVDC hvDC,int x,int y,char * lpString,int cbString);

extern int rbInvertRect(HVDC hvDC,RECT *lprc);

extern int rbAttachVdc(void *target,int width,int height,HVDC hvDC);

extern int rbDettachVdc(void *target);

extern int rbDrawLine(HVDC hvDC,int x1,int y1,int x2,int y2, rbCOLOR color);

extern int rbSetPixel(HVDC hvDC,int x,int y,rbCOLOR color);

extern int rbDrawRectangle(HVDC hvDC,  RECT *lprc,rbCOLOR bordercolor, rbCOLOR fillcolor);

extern int rbDrawEllipse(HVDC hvDC,  RECT *lprc,rbCOLOR bordercolor, rbCOLOR fillcolor);

extern int rbSetFont(HVDC hvDC,char fontname);

extern int rbSetFontSize(HVDC hvDC,int size);

extern int rbExtSetFont(HVDC hvDC,char fontname,int size,int isBold,int isItalic,int isUnderline);

extern int rbDrawImage(HVDC hvDC,rbIMAGE *img,int x,int y);


extern int rbBitBlt(HVDC hvDC_dst,int xDes,int yDes,int 	width,int height,HVDC hvDC_src,int xSrc,int ySrc);

extern int rbGetPixel(HVDC hvDC,int x,int y,rbCOLOR *color);

#endif /** MODULE_CONFIG_RB_GUI */

#endif

