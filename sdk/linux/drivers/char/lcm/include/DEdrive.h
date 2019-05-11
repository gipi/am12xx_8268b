#ifndef _DEDRIVER_H_
#define  _DEDRIVER_H_

//#include "amtypedef.h"
//#include "lcm_for_gui.h"
#include "actions_io.h"
#define OS_CPU_SR unsigned long

//#define FIFO_DBG

enum swgamma
{
	GAMMA_OFF,
	GAMMA_ON
};//gamma

enum swosd
{
	OSD_OFF,
	OSD_ON
};//OSD;
enum swcsc
{
	OFF,
	ON
};//CSC;
enum dither
{
	DITHER_OFF,
	Dither_888To565,
	Dither_888To666
};//DitherMode

enum outmode
{
	RGBMODE,
	RGBMODE_I,	
	BTMODE_I,
	BTMODE_P,
	CPUMODE
};//DisplayMode

enum deimagemode
{
	DE_YUV420,
	DE_YUV422,
	DE_RGB32,
	DE_RGB565,
	OSDBIT4MODE,
	OSDBIT2MODE,
	OSDBIT1MODE,
	OSDBIT16MODE,
	OSDBIT8MODE,
	OSDBIT32MODE
};//InImageMode;osdMode

enum source
{
	DE_IMAGE,
	DE_DEFAULTCOLOR
};//DisplaySource

enum errorde
{
	DE_OK,
	ADD_ERROR,
	STRIDE_ERROR,
	COORDINATE_ERROR,
	IMAGESIZE_ERROR,
	OSDOVER_ERROR,
	OSDORDER_ERROR,
	GAMMA_ERROR,
	CSC_ERROR
};

typedef struct _DisplayAttribute_
{
		INT32S  Mode;//enum outmode
		INT32S  DisplayWidth;//activeimagewidth;
		INT32S  DisplayHeight;//activeimageheight		
		INT32S  DitherMode;//enum dither
		INT32S  GammaMode;//enum swde
		INT32S * Gamma;
}DisplayAttribute;

typedef struct _OutImageAttribute_
{
		INT32S  DisplaySource;//enum source
		INT32S  DefaultColor;
		INT32S  OutImageXStart;
		INT32S  OutImageYStart;
		INT32S  OutImageXEnd;
		INT32S  OutImageYEnd;
}OutImageAttribute;

typedef struct _InImageAttribute_ 
{
		INT32S  InImageStride;//%16byte=0;
		INT32S  InImageAdd;//%16byte=0;1203:%4byte=0
		INT32S  InCImageAdd;//%16byte=0;1203:%4byte=0
		INT32S  InImageMode;//enum imagemode(YUV420,YUV422,RGB32,RGB565)
		INT32S  InImageWidth;
		INT32S  InImageHeight;
}InImageAttribute;

typedef struct _OsdAttribute_
{
		INT32S  No;//0-3;1203:0-1
		INT32S  SW;//enum swde
		INT32S  Mode;//enum imagemode(OSDBIT4MODE,OSDBIT2MODE,OSDBIT1MODE);1203:OSDBIT16MODE,OSDBIT8MODE
		INT32S  Alpha;//0-8
		INT32S  Add;//%16byte=0;1203:%4byte=0
		INT32S  Stride;//%16byte=0;
		INT32S  XStart;
		INT32S  YStart;
		INT32S  Width;
		INT32S  Height;
		INT32S  TransColor;//for OSDBIT16MODE,RGB888
}OsdAttribute;

typedef struct _CSCAttribute_
{
		INT32S  SW;//enum swcsc
		INT32S  a1;
		INT32S  a2;
		INT32S  b;
		INT32S  c;
		INT32S  d;
		INT32S  e;
		INT32S  ysub16;
		INT32S  color_coef;
		INT32S  offset1;
		INT32S  offset2;
		INT32S  contrast1;
		INT32S  contrast2;
}CSCAttribute;

typedef struct _LtiCtiAttribute_
{
		INT32S  SW;//1:ON;0:OFF
		INT32S  dlti_thd;
		INT32S  dlti_thd1;
		INT32S  dlti_gain;
		INT32S  dlti_step;
		INT32S  dcti_thd;
		INT32S  dcti_thd1;
		INT32S  dcti_gain;
		INT32S  dcti_step;
}LtiCtiAttribute;
typedef struct _FilterAttribute_
{
		INT32S  SW;//1:ON;0:OFF
		INT32S  c0;
		INT32S  c1;
		INT32S  c2;
		INT32S  lv;
		INT32S  mode; 
}FilterAttribute;
typedef struct _BleWleAttribute_
{
		INT32S  SW;//1:ON;0:OFF
		INT32S  ble_gain;
		INT32S  ble_b;
		INT32S  wle_gain;
		INT32S  wle_w;
		INT32S  offset;
}BleWleAttribute;

typedef struct _CursorAttribute_
{
		INT32S  SW;//1:ON;0:OFF
		INT32S  x;
		INT32S  y;
}CursorAttribute;

INT32S  DisplayDeviceSet(DisplayAttribute* display);
INT32S  ImageSet(OutImageAttribute* outimage,InImageAttribute* inimage,DisplayAttribute* display);
void ChangeFrameAdd(	INT32S  YAdd,	INT32S  CAdd);
INT32S  CscSet(CSCAttribute* csc);
INT32S  GammaSet(INT32U sw,INT32U* pGamma);

void Pallet16Set(	INT32S * Pallet,	INT32S  TransColor);
void Pallet256Set(INT32S * Pallet);
INT32S  OsdSet(OsdAttribute* osd);

INT32S  DeReset(void);
void EnterFrameOver(OS_CPU_SR* pFlag);
void ExitFrameOver(OS_CPU_SR* pFlag);
void  DisplayClkDisable(void);
void ClearIntFlag(void); 

INT32S  LtiCti( LtiCtiAttribute* lticti);
INT32S  LumHorFilter( FilterAttribute* filter);
INT32S  HistogramGet(	INT32S  channel);
void BleWle(BleWleAttribute* blewle);//1203

void ChangeImageMode(InImageAttribute* inimage,DisplayAttribute* display);
void CursorSet(CursorAttribute* cursor);

//void DeRegRead(DE_REG* de);//for slideshow and se in 1201/1205
//void DeRegWrite(DE_REG* de);//for slideshow and se in 1201/1205

#endif
 
