#ifndef SWF_CONTEXT_H
#define SWF_CONTEXT_H

#include "swf_base.h"
#include "swf_geometry.h"
#include "swf_action.h"
#include "swf_gzio.h"
#include "act_plugin.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LINKMEM_SIZE		2048
#define DICTIONARY_SIZE		8192//1024
#define STACK_SIZE			32
#define MASK_STACK_SIZE		256
#define AS_HASH_TABLE_SIZE	1024
#define SWF_MSG_QUEUE_SIZE	128
#define MAX_FONT_NUM		16

#ifdef EN_SOFT_KEYBOARD	
typedef enum keyboard_state
{
	FREE,
	CLICKED,
	CLICKED_DRAG,
	DRAG
}keyboard_state;

typedef struct KEYBOARD_DRAG
{
	int x;
	int y;
	keyboard_state state;
}KEYBOARD_DRAG;

typedef struct EDIT_CURSOR
{
	int cur;
	int total;
	SWF_RECT cursor;
}EDIT_CURSOR;
#endif

typedef struct _SwfDecCommon
{
	SWF_MATRIX		MxStack[STACK_SIZE];
	int				MxStackTop;
	SWF_CXFORM		CxStack[STACK_SIZE];
	int				CxStackTop;
	DISPLAYNODE*	MaskStack[MASK_STACK_SIZE];
	int				MaskStackTop;
	AA_LINE			AALine;

	AS_VALUE		AsStack[AS_MAX_STACK];
	//char			AsTextPool[AS_MAX_STACK][AS_MAX_TEXT]; /*STACK STRING LEN IS N TIMES OF AS_MAX_TEXT*/
	int				AsStackPtr;

	int				currentFrame;
	unsigned char*			maskBuffer;
	unsigned char*			zBuffer;
	int				zBufferEnable;

	SWF_RECT		currentWindow;
	SWF_RECT		nextWindow;

	DISPLAYNODE*	focusNode;
	DISPLAYNODE*	firstMask;
	int				focusX;
	int				focusY;
	BLIST			Listener;
	DISPLAYNODE*	focusEditNode;
	DISPLAYNODE*	LastEditNode;

#ifdef EN_SOFT_KEYBOARD		
	int				imeEn;//input method
	int				KeyBoardEn;//soft key board
	int				English;//soft key board
	SWF_RECT		imeRect;
	SWF_RECT		keyBoardRect;
	int				cursor_flag;
	int				ime_flag;
	KEYBOARD_DRAG	keyboarddrag;
	KEYBOARD_DRAG	freepydrag;
	EDIT_CURSOR 	edit_cur;

	int 			keyboard_stride;
	char *			keyboard_buf;
	char *			keyboard_buf1;
	char *			keyboard_buf2;
	int				KeyBoardshow;
#endif
} SWF_DEC_COMMON;

typedef struct _SwfMsg
{
	SWF_DEC * swfInst;
	int       msgId;
	void *    param;
}SWF_MSG;

typedef struct _SharedObjList
{
	BLIST BList;
	AS_SHAREDOBJECT *shareObj;
}SO_LIST;

struct _SwfDec
{
	BLIST				BList;
	SWF_DEC*			parent;
	SWF_DEC*			RootInst;
	SWF_CONTEXT*		context;
	SWF_DEC_COMMON*		common;
	int					state;
	
	DICTIONARY			dictionary[DICTIONARY_SIZE];
	DISPLAYNODE*		root;
	TAG					currTag;
	int					headOffset;
	AS_MCLOADER*		loader;
	
	gzFile				file;
	char*				filename;
	char				signature[3];
	SWF_RECT			frameRect;
	unsigned short				frameRate;
	unsigned int				fileLength;
	int					frameLoaded;
	int					frameRendered;

	SWF_RECT			window_rect;
	int					output_type;
	int					output_width;
	int					output_height;
			
	unsigned int				bg_color;
	unsigned char				version;
	AS_NUMBER			startTime;
	
	int					enableNavigate;
	int					enableFocusRect;
	
	unsigned char*				JPEGTable;
	int					JPEGTableLength;

	BLIST				VarHeap;
	BLIST				ASHeap;
	BLIST				Trash;
	BLIST				ConstantPool;
	BLIST				ExportAssets;
#if EN_EDIT_EMBEDFONT
	BLIST				FontList;
#endif
	BLIST				SharedObjList;

	SWF_DEV_FUNC		devFunc;
};

#define FRAME_FLAG_INTERNAL 1
#define FRAME_FLAG_FIRST    2

struct _SwfContext
{
	BLIST			SwfInst;
	SWF_DEC*		DefaultInst;
	SWF_DEC*		ActiveInst;

	int				frameBuffer[2];
	int				frameWidth;
	int				frameHeight;
	int				frameFlag;

	SWF_RECT *		ignoreWin;
	unsigned char*  zBuffer;
	unsigned char*  maskBuffer[2];

	SWF_HASH		HashTable[HASH_SIZE];
	SWF_MSG			msgQueue[SWF_MSG_QUEUE_SIZE];
	int				msgHead, msgTail;
	
	SWF_DEV_FUNC	devFunc;

	SWF_FONTINFO	fonts[MAX_FONT_NUM];
	int				fontNum;
	int				memLeakCheck;

	SWF_CONTEXTINFO *info;
};

SWF_DEC * swf_dec_create(char * filename, DISPLAYNODE * target);
void      swf_dec_set_state(SWF_DEC * s, int state);
void      swf_dec_set_window(SWF_DEC * s, SWF_RECT * window, int flag);
int       swf_dec_parse_frame(SWF_DEC * s);
int       swf_dec_process(SWF_DEC * s);
int       swf_dec_dispatch(SWF_DEC * s, int msg, void * param);
void      swf_dec_destory(SWF_DEC * s);

void swf_context_buffer_alloc(SWF_CONTEXT * ctx, int width, int height, int format);
void swf_context_buffer_clear(SWF_CONTEXT * ctx);
SWF_CONTEXT * swf_context_create(SWF_CONTEXTINFO * info);
void swf_context_destroy(SWF_CONTEXT * ctx);
SWF_DEC * swf_context_addInst(SWF_CONTEXT * ctx, char * filename, SWF_RECT * window, DISPLAYNODE * target, SWF_DEC * parent, int flag);
void swf_context_removeInst(SWF_DEC * s);
int  swf_context_message(SWF_CONTEXT * ctx, SWF_DEC * s, int msg, void * param);
int  swf_context_dispatch(SWF_CONTEXT * ctx);
int swf_context_next_msg_needs_play(SWF_CONTEXT * ctx);

#ifdef __cplusplus
}
#endif

#endif
