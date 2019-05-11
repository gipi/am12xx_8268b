#ifndef SWF_BASE_H
#define SWF_BASE_H

#include "swf_types.h"
#include "swf_util.h"
#include "swf_list.h"
#include "swf_opt.h"
#include "am_types.h"

typedef struct _DisplayList DISPLAYLIST;
typedef struct _DisplayNode DISPLAYNODE;

#define AS_MAX_TEXT		224	//240
#define AS_MAX_NAME		240
#define AS_MAX_STACK	128
#define AS_MAX_REGS		24

#define AS_MAX_NODENAME	100
#define AS_MAX_TEXTNAME	48
#define AS_MAX_FONTNAME	60

#define SWF_MAX_EDGE_NUM		36
#define SWF_MAX_AA_EDGE_NUM		128
#define SWF_MAX_INTERVAL_NUM	36

#define SWF_SMALL_PIC_WIDTH		GET_SPACE_COOR(400)
#define SWF_SMALL_PIC_HEIGHT	GET_SPACE_COOR(240)

#define SWF_PIC_INTERNAL	0x0001
#define SWF_PIC_EXTERNAL	0x0002
#define SWF_PIC_FULL_SIZE	0x0004
#define SWF_PIC_HALF_SIZE	0x0008
#define SWF_PIC_AUTO_SIZE	0x0010

#define DYNAMIC_AS_CODE 0x8000
#define INITIAL_AS_CODE 0x4000
#define WITH_AS_CODE	0x2000 //special for "with()" function

#define PRELOAD_PARENT  0x0080
#define PRELOAD_ROOT    0x0040
#define SUPPRESS_SUPER  0x0020
#define PRELOAD_SUPER   0x0010
#define SUPPRESS_AUGS   0x0008
#define PRELOAD_AUGS    0x0004
#define SUPPRESS_THIS   0x0002
#define PRELOAD_THIS    0x0001
#define PRELOAD_GLOBAL  0x0100

#define BUFFER_LAYER_OP_OVERLAY 0
#define BUFFER_LAYER_OP_ADD     1

typedef struct _Actions
{
	SLIST			SList;
	char**			Constant;
	unsigned short	ConstantNum;
	unsigned short	ParamNum;
	struct 
	{
		char * name;
		int    reg;
	}Param[AS_MAX_REGS];
	unsigned char *	Data;
	int				Length;
	int				Flag;	 //dynamic or static
	AS_OBJECT *		Definer; //who define the function code
	AS_FUNCTION*	thisFun; //the function object that includes the code
}ASCODE;

/************************************************************************/
/* Fill style - original*/

typedef struct _GradientRecord
{
	int   Ratio;
	unsigned int Color;
}GRADIENTRECORD;

typedef struct _GradientFill
{
	unsigned char NumGradients;
	GRADIENTRECORD * GradientRecords;
	SWF_MATRIX GradientMatrix;
}GRADIENTFILL;

typedef struct _BitmapFill
{
	unsigned short   BitmapId;
	SWF_MATRIX BitmapMatrix;
}BITMAPFILL;

typedef union Fill
{
	unsigned int	SolidFill;
	GRADIENTFILL	GradientFill;
	BITMAPFILL		BitmapFill;
}FILL;

/************************************************************************/
/* Shape style */

typedef struct _FillStyle
{
	unsigned char	FillStyleType;
	FILL		Fill;
}FILLSTYLE;

typedef struct _FillStyleArray
{
	unsigned short		FillStyleCount;
	unsigned short		FillBegin;
	FILLSTYLE * FillStyle;
}FILLSTYLEARRAY;

typedef struct _LineStyle
{
	unsigned short  Width;
	unsigned int Color;
	FILLSTYLE * FillStyle;
	int		hasTrans;//是否已经进行LineToPoly
}LINESTYLE;

typedef struct _LineStyleArray
{
	unsigned short	LineStyleCount;
	LINESTYLE * LineStyle;
}LINESTYLEARRAY;

/************************************************************************/
/* Morph style */

typedef struct _MorphFillStyle
{
	unsigned char	MorphFillStyleType;
	FILL	StartFill;
	FILL	EndFill;
}MORPHFILLSTYLE;

typedef struct _MorphFillStyleArray
{
	unsigned short		MorphFillStyleCount;
	MORPHFILLSTYLE *	MorphFillStyle;
}MORPHFILLSTYLEARRAY;

typedef struct _MorphLineStyle
{
	unsigned short	StartWidth;
	unsigned short	EndWidth;
	unsigned int	StartColor;
	unsigned int	EndColor;
}MORPHLINESTYLE;

typedef struct _MorphLineStyleArray
{
	unsigned short	MorphLineStyleCount;
	MORPHLINESTYLE *MorphLineStyle;
}MORPHLINESTYLEARRAY;

/************************************************************************/
/* Characters */

typedef struct _SubShape
{
	BLIST BList;
	FILLSTYLEARRAY  FillStyleArray;
	LINESTYLEARRAY  LineStyleArray;	
	BLIST*			FillPoly;//每种样式的多边形链表
	BLIST*			LinePath;//每种样式的线条链表
	BLIST*			LinePoly;//线条转换为多边形
}SUBSHAPE;

typedef struct _Shape
{
	SWF_RECT	ShapeRect;
	BLIST		SubShape;
	int			LineFlag;
}SHAPE;

typedef struct _BitMap
{
	int		BitmapWidth;		//decoded width
	int		BitmapHeight;		//decoded height
	unsigned int *Pixel;				//decoded data
	int		Format;				//ARGB,YUV422
	int		Stride;				//decoded stride

	int		OriginalWidth;		//defined width
	int		OriginalHeight;		//defined height
	int		ScaleX;				//defined width / decoded width
	int		ScaleY;				//defined height / decoded width
	int		TranslateX;			//x offset
	int		TranslateY;			//y offset
	int		ExtendMode;			//0: no action 1: repeat 2: extend

	unsigned char *JPEGData;			//jpeg data
	int		JPEGSize;			//jpeg size

	int		LifeTime;			//0~15
	int		(*GetStatus)(int id);//get decode status
	int		Status;				//current decode status

	// added by simon for mcloader
	int 	AutoZoom;		
}SWF_BITMAP;


#define FONT_FLAGS_LAYOUT	0x80
#define FONT_FLAGS_SHIFTJIS	0x40
#define FONT_FLAGS_SMALL	0x20
#define FONT_FLAGS_ANSI		0x10
#define FONT_FLAGS_WIDEOFF	0x08
#define FONT_FLAGS_WIDECODE	0x04
#define FONT_FLAGS_ITALIC	0x02
#define FONT_FLAGS_BOLD		0x01

typedef struct _FontAsset
{
	BLIST BList;
	INT16U FontId;
	SWF_DEC * Inst;
}FONTASSET;

typedef struct _UniTable
{
	union {
		INT8U Code;
		INT16U WideCode;
	};
	int GlyphIndex;
}UNITABLE;

typedef struct _Font
{
	int			FontFlags;
	int			LangCode;
	int			nGlyphs;
	BLIST *		GlyphShape;
	char *		FontName;
	int			FontNameLen;
	union
	{
		unsigned short *OffsetTable;
		unsigned int  *WideOffsetTable;
	};
	union
	{
		unsigned char *	CodeTable;
		unsigned short *WideCodeTable;
	};
#if	EN_EDIT_EMBEDFONT
//	UNITABLE *	UniTable;
	INT16S		FontLeading;
	INT16S		FontAscent;
	INT16S		FontDescent;
	INT16S *	FontAdvanceTable;
#endif
}FONT;

typedef struct _GlyphEntry
{
	int GlyphIndex;
	int GlyphAdvance;
}GLYPHENTRY;

typedef struct _TextRecord
{
	BLIST BList;
	unsigned short FontId;
	unsigned int TextColor;
	short XOffset;
	short YOffset;
	unsigned short TextHeight;
	unsigned char GlyphCount;
	GLYPHENTRY * GlyphEntries;

	int startIndex;
}TEXTRECORD;

typedef struct _Text
{
	SWF_RECT	TextBounds;
	SWF_MATRIX		TextMatrix;
	BLIST		TextRecords;

	INT32U	selectColor;
	int selectStart;
	int selectLen;
	int nGlyphNum;
}SWF_TEXT;

#define EDIT_FLAGS_HAS_TEXT			0x0080
#define EDIT_FLAGS_WORD_WRAP		0x0040
#define EDIT_FLAGS_MULTLINE			0x0020
#define EDIT_FLAGS_PASSWORD			0x0010
#define EDIT_FLAGS_READONLY			0x0008
#define EDIT_FLAGS_HAS_TEXT_COLOR	0x0004
#define EDIT_FLAGS_HAS_MAX_LENGTH	0x0002
#define EDIT_FLAGS_HAS_FONT			0x0001
#define EDIT_FLAGS_HAS_FONT_CLASS	0x8000
#define EDIT_FLAGS_AUTO_SIZE		0x4000
#define EDIT_FLAGS_HAS_LAYOUT		0x2000
#define EDIT_FLAGS_NO_SELECT		0x1000
#define EDIT_FLAGS_BORDER			0x0800
#define EDIT_FLAGS_WAS_STATIC		0x0400
#define EDIT_FLAGS_HTML				0x0200
#define EDIT_FLAGS_USE_OUTLINES		0x0100

enum {
	EDIT_ALIGN_LEFT = 0,
	EDIT_ALIGN_RIGHT,
	EDIT_ALIGN_CENTER,
	EDIT_ALIGN_JUSTIFY
};

typedef struct _Edit
{
	SWF_RECT	Bounds;
	int			Flags;
	int			FontId;

	int			FontHeight;
	unsigned int TextColor;
	int			MaxLength;
	int			Align;
	int			LeftMargin;
	int			RightMargin;
	int			Indent;
	int			Leading;

	char		VarName[AS_MAX_TEXTNAME]; //AS_MAX_NAME(240)->AS_MAX_TEXTNAME(40)
	//char		InitText[AS_MAX_TEXT];
	
	/*
	比较特殊，在edit tag解析时可能有html格式信息，导致超过240 bytes，改为动态malloc。
	最终如果cpu速度够快，将全部改为动态malloc.
	*/
	char		*InitText;
}SWF_EDIT;

#define FORMAT_FLAG_MASK	0x3FFFF

#define FORMAT_FLAG_FONT	0x1
#define FORMAT_FLAG_SIZE	0x2
#define FORMAT_FLAG_COLOR	0x4
#define FORMAT_FLAG_BOLD	0x8
#define FORMAT_FLAG_ITALIC	0x10
#define FORMAT_FLAG_UNDERLINE	0x20
#define FORMAT_FLAG_URL	0x40
#define FORMAT_FLAG_TARGET	0x80
#define FORMAT_FLAG_ALIGN	0x100
#define FORMAT_FLAG_LEFTMARGIN	0x200
#define FORMAT_FLAG_RIGHTMARGIN	0x400
#define FORMAT_FLAG_INDENT	0x800
#define FORMAT_FLAG_LEADING	0x1000
#define FORMAT_FLAG_BLOCKINDENT	0x2000
#define FORMAT_FLAG_BULLET	0x4000
#define FORMAT_FLAG_KERNING	0x8000
#define FORMAT_FLAG_LETTERSPACING	0x10000
#define FORMAT_FLAG_TABSTOPS	0x20000


#define TEXTFIELD_FLAG_BACKGROUND			0x1
#define TEXTFIELD_FLAG_BORDER				0x2			
#define TEXTFIELD_FLAG_EMBEDFONTS			0x4
#define TEXTFIELD_FLAG_MULTILINE			0x8
#define TEXTFIELD_FLAG_HTML					0x10
#define TEXTFIELD_FLAG_PASSWORD				0x20
#define TEXTFIELD_FLAG_WORDWRAP				0x40
#define TEXTFIELD_FLAG_SELECTABLE			0x80
#define TEXTFIELD_FLAG_CONDENSEWHITE		0x100
#define TEXTFIELD_FLAG_MOUSEWHEELENABLED	0x200

#define TEXTFIELD_FLAG_NOT_USEDEFALT	0x8000

typedef struct {
	char font[AS_MAX_FONTNAME];
	AS_NUMBER size;
	AS_NUMBER color;
	int bold;
	int italic;
	int underline;
	int align;
/*
	char url[AS_MAX_TEXT];
	char target[AS_MAX_TEXT];
*/
	AS_NUMBER leftMargin;
	AS_NUMBER rightMargin;
	AS_NUMBER indent;
	AS_NUMBER leading;

}SWF_TEXTFORMAT;

typedef struct {
	BLIST BList;
	SWF_TEXTFORMAT txtFmt;
	INT16U beginIndex;
	INT16U endIndex;
}TEXTFMT;

typedef struct {
	BLIST BList;
	INT32U color;
	INT16U beginIndex;
	INT16U endIndex;
}TextColorList;

typedef struct {
	BLIST BList;
	char underline;
	INT16U beginIndex;
	INT16U endIndex;
}TextUnderlineList;

/*For the sake of compatibility*/
typedef struct {
	SWF_TEXTFORMAT* txtFmt;
#ifdef USE_MULTI_TEXTFORMAT
	//BLIST txtFmtList;
	INT32U*	textColor;
	int lenColor;
	char * underline;
	int lenUnder;
#endif
	int FontId;
	/* Fields excluding SWF_TEXTFORMAT. */
	INT32U	BackGroundColor;
	INT32U	BorderColor;
	int		autoSize;	/* 0: "false"/"none", 1:"left", 2:"right", 3:"center", to automatic coordinate the width and height of textfield */
	int		type;		/*1: "dynamic", 0: "input"*/
	int		length;
	int		scroll;
	int		hscroll;
	int		maxscroll;
	/* Boolean flags, embedFonts, Password, WordWrap, BackGround, Border, multiline, html, password, selectable, condensewhite, mouseWheelEnabled.*/
	int		Flags;
}SWF_TextFieldFormat;

typedef struct {
	char *	text/*[AS_MAX_TEXT]*/;	//CORRENT TEXT
	int textLen;
	INT32U	color;				//CORRENT TEXT COLOR, THE SAME WITH TEXTFORMAT->COLOR WHEN TextFieldFormat HAS BEEN SET.
	SWF_TextFieldFormat *TextFieldFormat;	//CORRENT TEXT FORMAT
}EditPrivate;

typedef struct _Morph
{
	SWF_RECT StartRect;
	SWF_RECT EndRect;  
	
	MORPHFILLSTYLEARRAY  MorphFillStyleArray;
	MORPHLINESTYLEARRAY  MorphLineStyleArray;

	BLIST	* PolyPath;
	BLIST	* LinePath;
}MORPH;

typedef struct _Frame
{
	SHEAD   Tags;
	SHEAD	Codes;
	char *  Label;
	int		hash;
}FRAME;

typedef struct _Sprite
{
	FRAME*			Frame;
	INT16U			FrameCount;
	AS_OBJECT*		ClassObj;
	int				HasConstructed;//避免movieclip派生类重复执行构造函数，或申请EventCode的代码空间而没有释放
}SPRITE;

typedef struct _Tag
{
	SLIST	SList;
	int		TagType;
	int		TagLength;
	
	unsigned char*		data;
	unsigned int		hold;
	unsigned int		bits;
	unsigned int		ptr;
}TAG;

typedef enum {
	SWF_BUTTON_UP       = 1,
	SWF_BUTTON_OVER     = 2,
	SWF_BUTTON_DOWN     = 4,
	SWF_BUTTON_HIT = 8
}ButtonState;

typedef struct _ButtonRecord {
	BLIST		BList;
	int			State;
	unsigned short		Id;
	unsigned short		Depth;
	SWF_MATRIX		Matrix;
	SWF_CXFORM		CxForm;
	int			BlendMode;
}BUTTON_RECORD;

typedef enum {
	SWF_BUTTON_OVERDOWN2IDLE = 0x0100,
	SWF_BUTTON_IDLE2OVERDOWN = 0x0080,
	SWF_BUTTON_OUTDOWN2IDLE  = 0x0040,
	SWF_BUTTON_OUTDOWN2OVERDOWN = 0x0020,
	SWF_BUTTON_OVERDOWN2OUTDOWN = 0x0010,
	SWF_BUTTON_OVERDOWN2OVERUP = 0x0008,
	SWF_BUTTON_OVERUP2OVERDOWN = 0x0004,
	SWF_BUTTON_OVERUP2IDEL = 0x0002,
	SWF_BUTTON_IDLE2OVERUP = 0x0001
}ButtonCondition;

typedef struct _ButtonCondition {
	BLIST		BList;
	ASCODE		AsCode;
	int			Condition;
	int			Key;
}BUTTON_CONDITION;

typedef struct
{
	int prevState;
	int currState;
} ButtonPriv;

typedef struct _SoundInfo
{
	int		SyncFlag;
	int		InPoint;
	int		OutPoint;
	int		LoopCount;
} SOUND_INFO;

typedef struct _ButtonSound
{
	int        SoundID;
	SOUND_INFO SoundInfo;
} BUTTON_SOUND;

typedef struct _Button
{
	int			TotalStates;
	BLIST		ButtonRecords;
	BLIST		ButtonConditions;
	BUTTON_SOUND ButtonSound[4];
	int			TrackAsMenu;
} BUTTON;

typedef struct _Sound
{
	int		SoundInfo;
	int		SampleCount;
	int		SoundSize;
	unsigned char *	SoundData;
} SWF_SOUND;

/************************************************************************/
/* Geometry structure */

typedef enum _PointFlag
{
	MOVETO  = 0,
	LINETO  = 1,
	CURVETO = 2
}POINTFLAG;

typedef struct _Point
{
#if 1
	int flag;// : 2;
	int x ;//   : 30;
	int y;
#else
	INT16S x;
	INT16S y;
	INT8U  flag;
#endif
}SWF_POINT;

typedef struct _Point2
{
	INT32S x;
	INT32S y;
	INT8U  flag;
}SWF_POINT2;

typedef struct _Path
{
	BLIST		BList;
	int		PointNum;
	SWF_POINT * Point;
}PATH;

typedef struct _MorphPoint
{
	short startx,starty;
	short endx,endy;
	int flag;
}MORPHPOINT;

typedef struct _MorphPath
{
	BLIST		BList;
	int			PointNum;
	MORPHPOINT* Point;
}MORPHPATH;

typedef struct _Interval
{
#if ANTI_ALIASING_QUALITY == AA_LOW_QUALITY || ANTI_ALIASING_QUALITY == AA_NONE
	short	x;
#elif ANTI_ALIASING_QUALITY == AA_MID_QUALITY
	short	x; //x0
	short	x1;// : 4;	
	unsigned short	a;
#endif
}INTERVAL;

typedef struct _MaskInterval
{
	int			ColNum;
	INTERVAL	point[SWF_MAX_INTERVAL_NUM];
}MINTERVAL;

#define SWF_SL_NEED_BLEND   0x10
#define SWF_SL_INVISIBLE	0x20

typedef struct _ScanLine
{
	SLIST		SList;
	int			Mode;
	FILLSTYLE	FillStyle;
	SWF_RECT	Rect;
	MINTERVAL *	Interval;
	int			RowNum;
	int			PairNum;
	int			PairMax;
	int	*		Pair;
}SCANLINE;

typedef struct  
{
	SLIST	SList;
	SHEAD	ScanLineList;
} SUBSCALLINELIST;

/************************************************************************/

typedef struct _ExprotAsset
{
	BLIST		BList;
	SWF_DEC*	Inst;
	int			Id;
	char		Name[40];
}EXPORT_ASSET;

/************************************************************************/

#define DICT_FLAG_STATIC  0
#define DICT_FLAG_DYNAMIC 1
#define DICT_FLAG_ATTACH  2

typedef struct _Dictionary
{
	int				Flag;
	int				Type;
	union
	{
		SHAPE*      Shape;
		SPRITE*     Sprite;
		SWF_BITMAP* BitMap;
		MORPH*      Morph;
		FONT*       Font;
		SWF_TEXT*   Text;
		SWF_EDIT*   Edit;
		BUTTON*     Button;
		SWF_SOUND*	Sound;
	};
}DICTIONARY;

/************************************************************************/

#define HASH_SIZE 1024 //457

typedef struct _Hash
{
	int    count;
	int    base;
	int    full;
	char*  string;
	void*  item;
}SWF_HASH;

/************************************************************************/

typedef int (*AsFunc)(ASCONTEXT *);

typedef struct _AsClass
{
	char*		ClassName;
	int			ClassNo;
	void		(*Init)(SWF_CONTEXT*);
	AsFunc		Call;
	AsFunc		New;
	AsFunc		Get;
	AsFunc		Set;
	AS_OBJECT*  Prototype;
} AS_CLASS;

#define EXTERN_AS_CLASS(x) \
extern AS_CLASS swf_##x##_class;

#define DEFINE_AS_CLASS(x, _init, _call, _new, _get, _set, _proto) \
	AS_CLASS swf_##x##_class = {#x,AS_OBJECT_EXTERNAL,_init,_call,_new,_get,_set,_proto};

/************************************************************************/

typedef enum {
		SWFDEC_TAG_ERROR                = -1,
		SWFDEC_TAG_END                  = 0,
		SWFDEC_TAG_SHOWFRAME            = 1,
		SWFDEC_TAG_DEFINESHAPE          = 2,
		SWFDEC_TAG_FREECHARACTER        = 3,
		SWFDEC_TAG_PLACEOBJECT          = 4,
		SWFDEC_TAG_REMOVEOBJECT         = 5,
		SWFDEC_TAG_DEFINEBITSJPEG       = 6,
		SWFDEC_TAG_DEFINEBUTTON         = 7,
		SWFDEC_TAG_JPEGTABLES           = 8,
		SWFDEC_TAG_SETBACKGROUNDCOLOR   = 9,
		SWFDEC_TAG_DEFINEFONT           = 10,
		SWFDEC_TAG_DEFINETEXT           = 11,
		SWFDEC_TAG_DOACTION             = 12,
		SWFDEC_TAG_DEFINEFONTINFO       = 13,
		SWFDEC_TAG_DEFINESOUND          = 14,      /* Event sound tags. */
		SWFDEC_TAG_STARTSOUND           = 15,
		SWFDEC_TAG_DEFINEBUTTONSOUND    = 17,
		SWFDEC_TAG_SOUNDSTREAMHEAD      = 18,
		SWFDEC_TAG_SOUNDSTREAMBLOCK     = 19,
		SWFDEC_TAG_DEFINEBITSLOSSLESS   = 20,      /* A bitmap using lossless zlib compression. */
		SWFDEC_TAG_DEFINEBITSJPEG2      = 21,      /* A bitmap using an internal JPEG compression table. */
		SWFDEC_TAG_DEFINESHAPE2         = 22,
		SWFDEC_TAG_DEFINEBUTTONCXFORM   = 23,
		SWFDEC_TAG_PROTECT              = 24,      /* This file should not be importable for editing. */
		SWFDEC_TAG_PLACEOBJECT2         = 26,      /* The new style place w/ alpha color transform and name. */
		SWFDEC_TAG_REMOVEOBJECT2        = 28,      /* A more compact remove object that omits the character tag (just depth). */
		SWFDEC_TAG_DEFINESHAPE3         = 32,      /* A shape V3 includes alpha values. */
		SWFDEC_TAG_DEFINETEXT2          = 33,      /* A text V2 includes alpha values. */
		SWFDEC_TAG_DEFINEBUTTON2        = 34,      /* A button V2 includes color transform, alpha and multiple actions */
		SWFDEC_TAG_DEFINEBITSJPEG3      = 35,      /* A JPEG bitmap with alpha info. */
		SWFDEC_TAG_DEFINEBITSLOSSLESS2  = 36,      /* A lossless bitmap with alpha info. */
		SWFDEC_TAG_DEFINEEDITTEXT       = 37,
		SWFDEC_TAG_DEFINEMOVIE          = 38,
		SWFDEC_TAG_DEFINESPRITE         = 39,      /* Define a sequence of tags that describe the behavior of a sprite. */
		SWFDEC_TAG_NAMECHARACTER        = 40,      /* Name a character definition, character id and a string, (used for buttons, bitmaps, sprites and sounds). */
		SWFDEC_TAG_SERIALNUMBER         = 41,
		SWFDEC_TAG_GENERATORTEXT        = 42,      /* contains an id */
		SWFDEC_TAG_FRAMELABEL           = 43,      /* A string label for the current frame. */
		SWFDEC_TAG_SOUNDSTREAMHEAD2     = 45,      /* For lossless streaming sound, should not have needed this... */
		SWFDEC_TAG_DEFINEMORPHSHAPE     = 46,      /* A morph shape definition */
		SWFDEC_TAG_DEFINEFONT2          = 48,
		SWFDEC_TAG_TEMPLATECOMMAND      = 49,
		SWFDEC_TAG_GENERATOR3           = 51,
		SWFDEC_TAG_EXTERNALFONT         = 52,
		SWFDEC_TAG_EXPORTASSETS			= 56,
		SWFDEC_TAG_IMPORTASSETS			= 57,
		SWFDEC_TAG_ENABLEDEBUGGER		= 58,
		SWFDEC_TAG_DOINITACTION			= 59,
		SWFDEC_TAG_DEFINEVIDEOSTREAM	= 60,
		SWFDEC_TAG_VIDEOFRAME			= 61,
		SWFDEC_TAG_DEFINEFONTINFO2		= 62,
		SWFDEC_TAG_DEBUGID				= 63,
		SWFDEC_TAG_ENABLEDEBUGGER2		= 64,
		SWFDEC_TAG_SCRIPTLIMITS			= 65,
		SWFDEC_TAG_SETTABINDEX			= 66,
#if 0
		/* magic tags that seem to be similar to FILEATTRIBUTES */
		SWFDEC_TAG_						= 67,
		SWFDEC_TAG_						= 68,
#endif
		SWFDEC_TAG_FILEATTRIBUTES	  = 69,
		SWFDEC_TAG_PLACEOBJECT3		  = 70,
		SWFDEC_TAG_IMPORTASSETS2	  = 71,
#if 0
		/* seems similar to SWFDEC_TAG_AVM2DECL */
		SWFDEC_TAG_			  = 72, /* allowed with DefineSprite */
#endif
		SWFDEC_TAG_DEFINEFONTALIGNZONES		= 73,
		SWFDEC_TAG_CSMTEXTSETTINGS			= 74,
		SWFDEC_TAG_DEFINEFONT3				= 75,
		SWFDEC_TAG_AVM2DECL					= 76,
		SWFDEC_TAG_METADATA					= 77,
		SWFDEC_TAG_DEFINESCALINGGRID		= 78,
#if 0
		/* more magic tags that seem to be similar to FILEATTRIBUTES */
		SWFDEC_TAG_			  = 80,
		SWFDEC_TAG_			  = 81,
#endif
		SWFDEC_TAG_AVM2ACTION			  = 82,
		SWFDEC_TAG_DEFINESHAPE4			  = 83,
		SWFDEC_TAG_DEFINEMORPHSHAPE2      = 84,
		SWFDEC_TAG_PRIVATE_IMAGE    	  = 85,
		SWFDEC_TAG_DEFINESCENEDATA  	  = 86,
		SWFDEC_TAG_DEFINEBINARYDATA 	  = 87,
		SWFDEC_TAG_DEFINEFONTNAME		  = 88,
		SWFDEC_TAG_STARTSOUND2			  = 89
} SwfdecTag;

typedef enum {
	/* tag is allowe inside DefineSprite */
	SWFDEC_TAG_DEFINE_SPRITE = (1 << 0),
	/* tag must be first tag */
	SWFDEC_TAG_FIRST_ONLY = (1 << 1)
} SwfdecTagFlag;

typedef struct tag_func_struct
{
	const char *name;
	union
	{
		void * pointer;
		void (*define)(SWF_DEC *);
		void (*control)(SWF_DEC *, TAG *, DISPLAYLIST *);
	};
	int flag;
} TAG_FUNC;

#define TAG_FUNCTION_NUMBER 90

extern TAG_FUNC tag_funcs[TAG_FUNCTION_NUMBER];


#endif
