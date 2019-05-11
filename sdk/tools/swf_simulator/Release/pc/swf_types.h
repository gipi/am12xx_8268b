#ifndef _SWF_TYPES_H_
#define _SWF_TYPES_H_

//#include "amTypedef.h"
#include "am_types.h"

typedef struct _SwfContext		SWF_CONTEXT;
typedef struct _SwfDec			SWF_DEC;
typedef struct _AsContext		ASCONTEXT;
typedef struct _AsObject		AS_OBJECT;
typedef struct _AsArray			AS_ARRAY;
typedef struct _AsMcLoader		AS_MCLOADER;
typedef struct _AsFunction		AS_FUNCTION;
typedef struct _AsColor			AS_COLOR;
typedef struct _AsDate			AS_DATE;
typedef struct _AsSound			AS_SOUND;
typedef struct _AsMatrix		AS_MATRIX;
typedef struct _AsBitmapData	AS_BITMAPDATA;
typedef struct _AsSharedObject	AS_SHAREDOBJECT;
typedef struct _AsTextFormat	AS_TEXTFORMAT;
typedef struct _AsTextSnapshot	AS_TEXTSNAPSHOT;
typedef struct _AsPoint			AS_POINT;
typedef struct _AsRectangle		AS_RECTANGLE;
typedef struct _AsLoadVars		AS_LOADVARS;
typedef struct _AsColorTransform	AS_COLORTRANSFORM;

#define DOUBLE_POINT

#ifdef FIX_POINT
typedef INT32S AS_NUMBER;
#define ITOF(_n) ((_n) << 16)
#define FTOI(_n) ((_n) >> 16)
#else
#ifdef DOUBLE_POINT
typedef double AS_NUMBER;
#define ITOF(_n) ((double)(_n))
#else
typedef float AS_NUMBER;
#define ITOF(_n) ((float)(_n))
#endif
#define FTOI(_n) ((int)(_n))
#endif

/************************************************************************/
/* Decode Bitmap Flags */

#define SWF_BMP_FMT_COMPACT		0x02
#define SWF_BMP_FMT_ARGB        0x00
#define SWF_BMP_FMT_XRGB        0x01
#define SWF_BMP_FMT_RGB565		0x02
#define SWF_BMP_FMT_YUV422      0x03
#define SWF_BMP_MODE_IGNORE     0x00
#define SWF_BMP_MODE_REPEAT     0x01
#define SWF_BMP_MODE_EXTEND     0x02
#define SWF_BMP_MODE_BGCOLOR    0x03

/************************************************************************/
/* Window Flags */
#define SWF_WINDOW_MASK		0x00FF
#define SWF_SIZE_MASK		0x000F
#define SWF_SIZE_HEIGHT		0x0001
#define SWF_SIZE_WIDTH		0x0002
#define SWF_SIZE_AUTO		0x0004
#define SWF_SIZE_FILL		0x0008
#define SWF_ALIGN_H_MASK	0x0030
#define SWF_ALIGN_H_LEFT	0x0010
#define SWF_ALIGN_H_RIGHT	0x0020
#define SWF_ALIGN_H_CENTER	0x0030
#define SWF_ALIGN_V_MASK	0x00C0
#define SWF_ALIGN_V_UP		0x0040
#define SWF_ALIGN_V_DOWN	0x0080
#define SWF_ALIGN_V_CENTER	0x00C0

/* Root SWF instance Flags */
#define SWF_STANDALONE		0x0100
#define SWF_NON_RELOAD		0x0200

/* Debug Flags */
#define SWF_DEBUG_MASK		0x0c00
#define SWF_DEBUG			0x0400
#define SWF_ECHO			0x0800

/* Active Status Flags */
#define SWF_STATE_MASK		0x3000
#define SWF_STATE_SLEEP		0x0000
#define SWF_STATE_ACTIVE	0x1000
#define SWF_STATE_PAUSE		0x3000

/* Focus Navigation Flags */
#define SWF_FOCUS_AUTO		0x4000
#define SWF_KEY_NAVIGATE	0x8000

/* Format Flag */
#define SWF_FORMAT_MASK		0xF0000
#define SWF_FORMAT_YUV422	0x00000
#define SWF_FORMAT_RGB565	0x10000
#define SWF_FORMAT_RGB888	0x20000

/* Transparent Flag Mask */
#define SWF_TRANS_MASK		(SWF_DEBUG_MASK | SWF_FORMAT_MASK)

/* Default Flags */
#define SWF_DEFAULT_FLAG    (SWF_SIZE_AUTO | SWF_ALIGN_H_CENTER | SWF_ALIGN_V_CENTER | SWF_SIZE_FILL | SWF_STATE_ACTIVE | SWF_KEY_NAVIGATE)
#define SWF_FULLSCREEN_FLAG (SWF_SIZE_HEIGHT | SWF_SIZE_WIDTH | SWF_SIZE_FILL | SWF_STATE_ACTIVE | SWF_KEY_NAVIGATE | SWF_FOCUS_AUTO)

/************************************************************************/
/* Messages */

#define SWF_MSG_NULL		0x0000
#define SWF_MSG_PLAY		0x0001
#define SWF_MSG_SLEEP		0x0002
#define SWF_MSG_ACTIVE		0x0003
#define SWF_MSG_PAUSE		0x0004
#define SWF_MSG_TIMER		0x0005
#define SWF_MSG_SOUND_END	0x0006
#define SWF_MSG_RELOAD		0x0007
#define SWF_MSG_EDITCHANGED		0x0008

#define SWF_MSG_KEY			0x0100
#define SWF_MSG_ENTER		(SWF_MSG_KEY|13)
#define SWF_MSG_EXIT		0x0113
#define SWF_MSG_KEY_LEFT	(SWF_MSG_KEY|37)
#define SWF_MSG_KEY_RIGHT	(SWF_MSG_KEY|39)
#define SWF_MSG_KEY_UP	(SWF_MSG_KEY|38)
#define SWF_MSG_KEY_DOWN	(SWF_MSG_KEY|40)

#define SWF_MSG_MOUSE			0x0200
#define SWF_MSG_LBUTTONDOWN		(SWF_MSG_MOUSE|0x1)
#define SWF_MSG_MOVE			(SWF_MSG_MOUSE|0x2)
#define SWF_MSG_LBUTTONUP		(SWF_MSG_MOUSE|0x3)
#define SWF_MSG_RBUTTONDOWN		(SWF_MSG_MOUSE|0x4)
#define SWF_MSG_RBUTTONUP		(SWF_MSG_MOUSE|0x5)
#define SWF_MSG_MBUTTONDOWN		(SWF_MSG_MOUSE|0x6)
#define SWF_MSG_MBUTTONUP		(SWF_MSG_MOUSE|0x7)
#define SWF_MSG_WHEEL		(SWF_MSG_MOUSE|0x8)
#define SWF_MSG_LDRAG		(SWF_MSG_MOUSE|0x9)
#define SWF_MSG_RDRAG		(SWF_MSG_MOUSE|0xA)

/************************************************************************/
/* Font Flags */

#define SWF_FONT_UTF8		0x0000
#define SWF_FONT_UTF16		0x0001
#define SWF_FONT_SINGLE		0x0002

/************************************************************************/
/* Basic types */

typedef struct _Rect
{
	int Xmin;
	int Ymin;
	int Xmax;
	int Ymax;
}SWF_RECT;

typedef struct _SwfMatrix
{
	int ScaleX;
	int ScaleY;	
	int RotateSkew0;
	int RotateSkew1;	
	int TranslateX;
	int TranslateY;
}SWF_MATRIX;

typedef struct _SwfCxForm
{
	int RedMultTerm,GreenMultTerm,BlueMultTerm,AlphaMultTerm;
	int RedAddTerm,GreenAddTerm,BlueAddTerm,AlphaAddTerm;
}SWF_CXFORM;

/************************************************************************/
/* Initializing structure */

typedef enum {
	SWFDEC_AS_TYPE_UNDEFINED = 0,
	SWFDEC_AS_TYPE_NULL = 0,
	SWFDEC_AS_TYPE_BOOLEAN,
	SWFDEC_AS_TYPE_INT,
	SWFDEC_AS_TYPE_NUMBER,
	SWFDEC_AS_TYPE_STRING,
	SWFDEC_AS_TYPE_OBJECT,
} SwfdecAsValueType;

typedef enum {
	AS_OBJECT_SPRITE = 0,
	AS_OBJECT_MATH,
	AS_OBJECT_STRING,
	AS_OBJECT_ARRAY,
	AS_OBJECT_KEY,
	AS_OBJECT_EXTERNAL,
	AS_OBJECT_OBJECT,
	AS_OBJECT_GLOBAL,
	AS_OBJECT_DEFAULT,
	AS_OBJECT_MCLOADER,
	AS_OBJECT_FUNCTION,
	AS_OBJECT_COLOR,
	AS_OBJECT_DATE,
	AS_OBJECT_SELECTION,
	AS_OBJECT_SOUND,
	AS_OBJECT_MATRIX,
	AS_OBJECT_BITMAPDATA,
	AS_OBJECT_STAGE,
	AS_OBJECT_MOUSE,
	AS_OBJECT_SHAREDOBJECT,
	AS_OBJECT_TEXTFORMAT,
	AS_OBJECT_TEXTSNAPSHOT,
	AS_OBJECT_POINT,
	AS_OBJECT_RECTANGLE,
	AS_OBJECT_ASBROADCASTER,
	AS_OBJECT_BOOLEAN,
	AS_OBJECT_NUMBER,
	AS_OBJECT_LOADVARS,
	AS_OBJECT_COLORTRANSFORM,
	AS_OBJECT_TYPE_NUM,
	AS_CLASS_SYSTEM,
	AS_CLASS_USER
} AsObjectType;

#define HEAP_TYPE_BASIC		0
#define HEAP_TYPE_SHARE		1

typedef struct _SwfHeapInfo
{
	int heap_addr;
	int heap_size;
	int heap_type;
	/** the base virtual and physical address for this heap */
	int heap_base_virtual_addr;
	int heap_base_phy_addr;
} SWF_HEAPINFO;

typedef struct _SwfFontInfo
{
	char * font_path;
	int    font_size;
} SWF_FONTINFO;

typedef struct _SwfImageInfo
{
	void * swf_inst;
	INT32S id;
	INT8U* jpeg_data;
	INT32S jpeg_size;
	INT8U* pixel;
	INT32S width;
	INT32S height;
	INT32S stride;
	INT32S flag;
	INT32S (*GetStatus)(INT32S id);
} SWF_IMAGE_INFO;

#define SOUND_SYNC_STOP			2
#define SOUND_SYNC_NOMULTIPLE	1
#define SWF_GET_SOUND_SYNC(x)   (((x) >> 16) & 0x3)

#define SOUND_FMT_UNCOMP_BE		0
#define SOUND_FMT_ADPCM			1
#define SOUND_FMT_MP3			2
#define SOUND_FMT_UNCOMP_LE		3
#define SOUND_FMT_NELLYMOSER	6
#define SWF_GET_SOUND_FMT(x)	(((x) >> 4) & 0xF)

#define SOUND_RATE_5_5KHZ		0
#define SOUND_RATE_11KHZ		1
#define SOUND_RATE_22KHZ		2
#define SOUND_RATE_44KHZ		3
#define SWF_GET_SOUND_RATE(x)	(((x) >> 2) & 0x3)

#define SOUND_BPS_SND8BIT		0
#define SOUND_BPS_SND16BIT		1
#define SWF_GET_SOUND_BPS(x)	(((x) >> 1) & 0x1)

#define SOUND_TYPE_MONO			0
#define SOUND_TYPE_STEREO		1
#define SWF_GET_SOUND_TYPE(x)	((x) & 0x1)

typedef struct _SwfAudioInfo
{
	void * swf_inst;
	INT32S id;
	INT32S frame_count;
	INT8U* audio_data;
	INT32S audio_size;
	INT32S loop;
	INT32S flag;
} SWF_AUDIO_INFO;

typedef struct _SwfDeviceFunction
{
	int (*decode_bitmap)(SWF_IMAGE_INFO * info);
	int (*play_audio)(SWF_AUDIO_INFO * info);
	int (*release)(SWF_DEC * s);
	int (*KeySound_Trigger)();
} SWF_DEV_FUNC;

typedef struct _SwfContextInfo
{
	SWF_HEAPINFO * heaps;
	SWF_DEV_FUNC defaultFunc;
	SWF_FONTINFO * fonts;
	
	int    frameBuffer[2];
	int    frameWidth;
	int    frameHeight;

	char	reload[256];
} SWF_CONTEXTINFO;

typedef struct _SwfInstInfo
{
	INT8S  signature[3];
	INT8U  version;
	INT32U file_length;
	INT16U width;
	INT16U height;
	INT16U frame_rate;
	INT16U frame_count;
} SWF_INSTINFO;

#endif

