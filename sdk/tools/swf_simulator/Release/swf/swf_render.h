#ifndef SWF_RENDER_H
#define SWF_RENDER_H

#include "swf_types.h"
#include "swf_base.h"
#include "swf_2d.h"

#define CHANGE_LINE  0x80000000

typedef struct {
	INT8U *		ZBuffer;
	INT32S		Value;
	SWF_MATRIX	Matrix;
	SWF_CXFORM	CxForm;
	SWF_TEXT *	Text;
}TEXTINFO;

typedef struct {
	INT8U *		ZBuffer;
	INT32S		Value;
	SWF_MATRIX	Matrix;
//	INT32U		Color;
	SWF_EDIT *	Edit;
//	char		Text[AS_MAX_TEXT];
	SWF_CXFORM	CxForm;
	EditPrivate *EditPriv;
}EDITINFO;

typedef struct {
	INT8U *		ZBuffer;
	INT32S		Value;
	SWF_MATRIX	Matrix;
	SWF_CXFORM	CxForm;
	INT32S		Ratio;
	INT32S		Id;
	SUBSHAPE *	SubShape;
}LINEINFO;

typedef struct {
	INT8U *		ZBuffer;
	INT32S		Value;
	SWF_MATRIX	Matrix;
	SHEAD		ScanlineList;
}MASKINFO;

typedef struct {
	INT8U *		ZBuffer;
	INT32S		Value;
	INT32U		Color;
	SWF_RECT	Box;
}BOXINFO;

typedef enum {
	RENDER_FILL = 0,
	RENDER_TEXT,
	RENDER_EDIT,
	RENDER_LINE,
	RENDER_MASK,
	RENDER_BOX,
	RENDER_TYPE_NUM
}RENDRE_TYPE;

typedef struct {
	INT32S	  type;
	SWF_DEC * SwfInst;
	union 
	{
		FILLINFO fill;
		TEXTINFO text;
		EDITINFO edit;
		LINEINFO line;
		MASKINFO mask;
		BOXINFO  box;
	};
}RENDER_INFO;

typedef struct {
	void (*solid_rect)(FILLINFO *);
	void (*solid_pair)(FILLINFO *);
	void (*bitmap_rect)(FILLINFO *);
	void (*bitmap_pair)(FILLINFO *);
}FILL_HANDLE;

typedef struct {
	INT32U (*color)(INT32U color);
	void (*point)(SWF_DEC * s, INT32U color, int x, int y, INT8U * z_buffer, int depth);
	void (*line)(SWF_DEC * s, INT32U color, int x0, int y0, int x1, int y1, SWF_RECT * boundary, INT8U * z_buffer, int depth);
	void (*text)(SWF_DEC * s, INT32U color, EditPrivate * editPriv, int x,  int y, int x1, INT32U height, SWF_RECT * boundary, INT8U * z_buffer, int depth, int wordWrap, int align, int leading, int multiline);
}DRAW_HANDLE;

extern FILL_HANDLE swf_fill_handle_422;
extern FILL_HANDLE swf_fill_handle_888;
extern FILL_HANDLE swf_fill_handle_565;
extern FILL_HANDLE swf_fill_handle_2d_422;
extern FILL_HANDLE swf_fill_handle_2d_888;
extern FILL_HANDLE swf_fill_handle_2d_565;

extern DRAW_HANDLE swf_draw_handle_422;
extern DRAW_HANDLE swf_draw_handle_888;
extern DRAW_HANDLE swf_draw_handle_565;


#if ANTI_ALIASING == 1
/*for anti-aliasing*/ 
typedef struct {
	int flag;
	int alpha;
	int x;
}POINT_INFO;

extern int swf_cal_aera_mid_quality(FILLINFO * Fillinfo, POINT_INFO * pi, int start_edge, int end_edge);
extern int swf_cal_aera_low_quality(FILLINFO * Fillinfo, POINT_INFO * pi, int start_edge, int end_edge);
#endif

#endif //<SWF_RENDER_H

