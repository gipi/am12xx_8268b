#ifndef SWF_ACTION_H
#define SWF_ACTION_H

#define ACTION_PRINT   printf

#include "swf_types.h"
#include "swf_system.h"

#ifdef __cplusplus
extern "C" {
#endif

/* all known actions */
typedef enum {
  SWFDEC_AS_ACTION_NEXT_FRAME = 0x04,
  SWFDEC_AS_ACTION_PREVIOUS_FRAME = 0x05,
  SWFDEC_AS_ACTION_PLAY = 0x06,
  SWFDEC_AS_ACTION_STOP = 0x07,
  SWFDEC_AS_ACTION_TOGGLE_QUALITY = 0x08,
  SWFDEC_AS_ACTION_STOP_SOUNDS = 0x09,
  SWFDEC_AS_ACTION_ADD = 0x0A,
  SWFDEC_AS_ACTION_SUBTRACT = 0x0B,
  SWFDEC_AS_ACTION_MULTIPLY = 0x0C,
  SWFDEC_AS_ACTION_DIVIDE = 0x0D,
  SWFDEC_AS_ACTION_EQUALS = 0x0E,
  SWFDEC_AS_ACTION_LESS = 0x0F,
  SWFDEC_AS_ACTION_AND = 0x10,
  SWFDEC_AS_ACTION_OR = 0x11,
  SWFDEC_AS_ACTION_NOT = 0x12,
  SWFDEC_AS_ACTION_STRING_EQUALS = 0x13,
  SWFDEC_AS_ACTION_STRING_LENGTH = 0x14,
  SWFDEC_AS_ACTION_STRING_EXTRACT = 0x15,
  SWFDEC_AS_ACTION_POP = 0x17,
  SWFDEC_AS_ACTION_TO_INTEGER = 0x18,
  SWFDEC_AS_ACTION_GET_VARIABLE = 0x1C,
  SWFDEC_AS_ACTION_SET_VARIABLE = 0x1D,
  SWFDEC_AS_ACTION_SET_TARGET2 = 0x20,
  SWFDEC_AS_ACTION_STRING_ADD = 0x21,
  SWFDEC_AS_ACTION_GET_PROPERTY = 0x22,
  SWFDEC_AS_ACTION_SET_PROPERTY = 0x23,
  SWFDEC_AS_ACTION_CLONE_SPRITE = 0x24,
  SWFDEC_AS_ACTION_REMOVE_SPRITE = 0x25,
  SWFDEC_AS_ACTION_TRACE = 0x26,
  SWFDEC_AS_ACTION_START_DRAG = 0x27,
  SWFDEC_AS_ACTION_END_DRAG = 0x28,
  SWFDEC_AS_ACTION_STRING_LESS = 0x29,
  SWFDEC_AS_ACTION_THROW = 0x2A,
  SWFDEC_AS_ACTION_CAST = 0x2B,
  SWFDEC_AS_ACTION_IMPLEMENTS = 0x2C,
  SWFDEC_AS_ACTION_RANDOM = 0x30,
  SWFDEC_AS_ACTION_MB_STRING_LENGTH = 0x31,
  SWFDEC_AS_ACTION_CHAR_TO_ASCII = 0x32,
  SWFDEC_AS_ACTION_ASCII_TO_CHAR = 0x33,
  SWFDEC_AS_ACTION_GET_TIME = 0x34,
  SWFDEC_AS_ACTION_MB_STRING_EXTRACT = 0x35,
  SWFDEC_AS_ACTION_MB_CHAR_TO_ASCII = 0x36,
  SWFDEC_AS_ACTION_MB_ASCII_TO_CHAR = 0x37,
  SWFDEC_AS_ACTION_DELETE = 0x3A,
  SWFDEC_AS_ACTION_DELETE2 = 0x3B,
  SWFDEC_AS_ACTION_DEFINE_LOCAL = 0x3C,
  SWFDEC_AS_ACTION_CALL_FUNCTION = 0x3D,
  SWFDEC_AS_ACTION_RETURN = 0x3E,
  SWFDEC_AS_ACTION_MODULO = 0x3F,
  SWFDEC_AS_ACTION_NEW_OBJECT = 0x40,
  SWFDEC_AS_ACTION_DEFINE_LOCAL2 = 0x41,
  SWFDEC_AS_ACTION_INIT_ARRAY = 0x42,
  SWFDEC_AS_ACTION_INIT_OBJECT = 0x43,
  SWFDEC_AS_ACTION_TYPE_OF = 0x44,
  SWFDEC_AS_ACTION_TARGET_PATH = 0x45,
  SWFDEC_AS_ACTION_ENUMERATE = 0x46,
  SWFDEC_AS_ACTION_ADD2 = 0x47,
  SWFDEC_AS_ACTION_LESS2 = 0x48,
  SWFDEC_AS_ACTION_EQUALS2 = 0x49,
  SWFDEC_AS_ACTION_TO_NUMBER = 0x4A,
  SWFDEC_AS_ACTION_TO_STRING = 0x4B,
  SWFDEC_AS_ACTION_PUSH_DUPLICATE = 0x4C,
  SWFDEC_AS_ACTION_SWAP = 0x4D,
  SWFDEC_AS_ACTION_GET_MEMBER = 0x4E,
  SWFDEC_AS_ACTION_SET_MEMBER = 0x4F,
  SWFDEC_AS_ACTION_INCREMENT = 0x50,
  SWFDEC_AS_ACTION_DECREMENT = 0x51,
  SWFDEC_AS_ACTION_CALL_METHOD = 0x52,
  SWFDEC_AS_ACTION_NEW_METHOD = 0x53,
  SWFDEC_AS_ACTION_INSTANCE_OF = 0x54,
  SWFDEC_AS_ACTION_ENUMERATE2 = 0x55,
  SWFDEC_AS_ACTION_BREAKPOINT = 0x5F,
  SWFDEC_AS_ACTION_BIT_AND = 0x60,
  SWFDEC_AS_ACTION_BIT_OR = 0x61,
  SWFDEC_AS_ACTION_BIT_XOR = 0x62,
  SWFDEC_AS_ACTION_BIT_LSHIFT = 0x63,
  SWFDEC_AS_ACTION_BIT_RSHIFT = 0x64,
  SWFDEC_AS_ACTION_BIT_URSHIFT = 0x65,
  SWFDEC_AS_ACTION_STRICT_EQUALS = 0x66,
  SWFDEC_AS_ACTION_GREATER = 0x67,
  SWFDEC_AS_ACTION_STRING_GREATER = 0x68,
  SWFDEC_AS_ACTION_EXTENDS = 0x69,
  SWFDEC_AS_ACTION_GOTO_FRAME = 0x81,
  SWFDEC_AS_ACTION_GET_URL = 0x83,
  SWFDEC_AS_ACTION_STORE_REGISTER = 0x87,
  SWFDEC_AS_ACTION_CONSTANT_POOL = 0x88,
  SWFDEC_AS_ACTION_STRICT_MODE = 0x89,
  SWFDEC_AS_ACTION_WAIT_FOR_FRAME = 0x8A,
  SWFDEC_AS_ACTION_SET_TARGET = 0x8B,
  SWFDEC_AS_ACTION_GOTO_LABEL = 0x8C,
  SWFDEC_AS_ACTION_WAIT_FOR_FRAME2 = 0x8D,
  SWFDEC_AS_ACTION_DEFINE_FUNCTION2 = 0x8E,
  SWFDEC_AS_ACTION_TRY = 0x8F,
  SWFDEC_AS_ACTION_WITH = 0x94,
  SWFDEC_AS_ACTION_PUSH = 0x96,
  SWFDEC_AS_ACTION_JUMP = 0x99,
  SWFDEC_AS_ACTION_GET_URL2 = 0x9A,
  SWFDEC_AS_ACTION_DEFINE_FUNCTION = 0x9B,
  SWFDEC_AS_ACTION_IF = 0x9D,
  SWFDEC_AS_ACTION_CALL = 0x9E,
  SWFDEC_AS_ACTION_GOTO_FRAME2 = 0x9F,
  SWFDEC_AS_ACTION_NUM = 0x100
} SwfdecAsAction;

typedef struct _AsConstant {
	BLIST	BList;
	unsigned char*	StartName;
	unsigned char** Constant;
}AS_CONSTANT;

typedef struct _AsString {
	char*	ptr;
#ifdef DOUBLE_POINT
	int		stuff;
#endif
	int		size;
	char*	inner;
}AS_STRING;

typedef struct _AsValue {
	int	type;
	union 
	{
		int			boolean;
		AS_NUMBER	number;
		AS_OBJECT*	object;
		AS_STRING	string;
	};
}AS_VALUE;

typedef struct _AsVariable {
	SLIST       SList;
	BLIST		BList;
	char*       Name;
	int         hash;
	AS_VALUE    Val;
}AS_VAR;

typedef enum {
	AS_RET_INVALID_SET = -9,
	AS_RET_INVALID_GET = -8,
	AS_RET_INVALID_NODE = -7,
	AS_RET_INVALID_TYPE = -6,
	AS_RET_INVALID_FUNCTION = -5,
	AS_RET_INVALID_INSTRUCTION = -4,
	AS_RET_INVALID_PROPERTY = -3,
	AS_RET_INVALID_LABEL = -2,
	AS_RET_INVALID_HANDLER = -1,
	AS_RET_OK = 0,
	AS_RET_RETURN,
} AsRetType;

typedef struct _AsInstruction
{
	char * Name;
	int    (*Func)(ASCONTEXT*);
	int    (*Dump)(ASCONTEXT*);
}AS_INSTRUCTION;

typedef enum {
	AS_EVENT_SKIP = -2,
	AS_EVENT_NULL = -1,
	AS_EVENT_KEY_UP = 0,
	AS_EVENT_KEY_DOWN,
	AS_EVENT_UNLOAD,
	AS_EVENT_LOAD,
	AS_EVENT_KEY_PRESS,
	AS_EVENT_ENTER_FRAME,
	AS_EVENT_ROLL_OVER,
	AS_EVENT_ROLL_OUT,
	AS_EVENT_RELEASE,
	AS_EVENT_ENTER,
	AS_EVENT_CONSTRUCT,
	AS_EVENT_MOUSE_DOWN,
	AS_EVENT_MOUSE_MOVE,
	AS_EVENT_MOUSE_UP,
	AS_EVENT_DRAG_OUT,
	AS_EVENT_DRAG_OVER,
	AS_EVENT_RELEASE_OUTSIDE,
	AS_EVENT_MOUSE_WHEEL,
	AS_EVENT_CHANGED,
	AS_EVENT_SETFOCUS,
	AS_EVENT_KILLFOCUS,
	AS_ENENT_SCROLLER,
	AS_EVENT_TYPE_NUM
} AsEventType;

#define AS_STACK_PUSH(as)\
	(&(as)->SwfInst->common->AsStack[(as)->SwfInst->common->AsStackPtr++])

#define AS_STACK_POP(as)\
	(&(as)->SwfInst->common->AsStack[--(as)->SwfInst->common->AsStackPtr])

#define AS_STACK_PEEK(as, n)\
	(&(as)->SwfInst->common->AsStack[(as)->SwfInst->common->AsStackPtr - (n)])

#define AS_GET_STRING(str, data)\
{\
	int _i;\
	for(_i = 0; *(data) != 0; _i++, (data)++)\
	{\
		(str)[_i] = *(data);\
	}\
	(str)[_i] = 0;\
	(data)++;\
}

typedef struct {
	char * name;
	void (* get) (DISPLAYNODE * node, AS_VALUE * value);
	void (* set) (DISPLAYNODE * node, AS_VALUE * value);
} AS_PROPERTY;

typedef struct {
	int    eventID;
	void (* set) (DISPLAYNODE * node, AS_VALUE * value);
} AS_EVENT;

typedef struct _AsTimer{
	unsigned int        timer;
	SWF_DEC     * inst;
	AS_OBJECT   * obj;
	AS_FUNCTION * func;
	int		      argc;
	AS_VALUE	* argv;
	int	          intv;
} AS_TIMER;

extern const AS_INSTRUCTION	swf_as_actions[];
extern const AS_PROPERTY    swf_as_properties[];
extern const char *			swf_as_error_code[];
extern const char *			swf_as_event_name[];
extern const int			Action2Condition[];
extern AS_CLASS *		AsObjectClassTable[];
extern AS_TIMER			AsTimerTable[];

/************************************************************************/
/* Define Object                                                        */
/************************************************************************/

#define DEFINE_AS_FUNC(_class, _func)\
	{#_func,swf_as_##_class##_##_func}

#define DEF_SWF_OBJECT \
	BLIST		BList; \
	SWF_DEC *	SwfInst; \
	SHEAD		UserVars; \
	SHEAD		Getters; \
	SHEAD		Setters; \
	int			Class; \
	AS_OBJECT * Prototype; \
	AS_OBJECT * Super; \
	AS_OBJECT*	SubClassObject; \
	int			Valid;

struct _AsObject{
	DEF_SWF_OBJECT
};

struct _AsArray{
	DEF_SWF_OBJECT
	int			length;
	int			size;
	AS_VALUE*	elem;
};

struct _AsMcLoader{
	DEF_SWF_OBJECT
	AS_OBJECT * Listerner;
};

struct _AsFunction{
	DEF_SWF_OBJECT
	ASCODE		code;
};

struct _AsColor{
	DEF_SWF_OBJECT
	DISPLAYNODE * target;
};

struct _AsDate{
	DEF_SWF_OBJECT
	SWF_DATE	date;
	SWF_TIME	time;
	unsigned int		millisec;
};

struct _AsSound{
	DEF_SWF_OBJECT
	DISPLAYNODE * target;
	char		  url[AS_MAX_TEXT];
	int        type;// 0:单一音频文件，1:打包文件
	int	sndId;
};

struct _AsMatrix{
	DEF_SWF_OBJECT
	AS_NUMBER a, b, c, d, tx, ty;
};

struct _AsBitmapData{
	DEF_SWF_OBJECT
	int	bmpId;
};

struct _AsSharedObject{
	DEF_SWF_OBJECT
	AS_OBJECT * data;
	int	size;
	char  url[AS_MAX_TEXT];
	char  *fileName;
};

struct _AsTextFormat{
	DEF_SWF_OBJECT
	int flag;
	char font[AS_MAX_FONTNAME];
	AS_NUMBER size;
	AS_NUMBER color;
	char bold;//是否为粗体
	char italic;
	char underline;
	char align;//指示段落的对齐方式
/*
	char url[AS_MAX_TEXT];
	char target[AS_MAX_TEXT];
*/
	AS_NUMBER leftMargin;
	AS_NUMBER rightMargin;
	AS_NUMBER indent;
	AS_NUMBER leading;
};

struct _AsTextSnapshot{
	DEF_SWF_OBJECT
	int count;
	DISPLAYNODE * target;
};

struct _AsPoint{
	DEF_SWF_OBJECT
	AS_NUMBER x;
	AS_NUMBER y;
};

struct _AsRectangle{
	DEF_SWF_OBJECT
	AS_NUMBER x;
	AS_NUMBER y;
	AS_NUMBER width;
	AS_NUMBER height;
};

struct _AsLoadVars{
	DEF_SWF_OBJECT
	int loaded;
};

struct _AsColorTransform{
	DEF_SWF_OBJECT
	AS_NUMBER alphaMultiplier, alphaOffset;
	AS_NUMBER blueMultiplier, blueOffset;
	AS_NUMBER greenMultiplier, greenOffset;
	AS_NUMBER redMultiplier, redOffset;
};


typedef struct _SwfKeyListener {
	BLIST BList;
	AS_OBJECT * obj;
} SWF_KEY_LISTENER;

typedef struct _SwfKey {
	int   key;
	int   lastKey;
	BLIST listener;
} SWF_KEY;

typedef struct _SwfMouseListener {
	BLIST BList;
	AS_OBJECT * obj;
} SWF_MOUSE_LISTENER;

typedef struct _SwfMouse {
	int state;
	BLIST listener;
} SWF_MOUSE;


struct _AsContext
{
	DEF_SWF_OBJECT
	struct _AsContext * ParantCxt;		//parent context
	ASCODE				AsCode;			//running code
	int					CurrPtr;		//pc
	int					NextPtr;		//next pc
	int					StackPtr;		//sp
	int					CurrAction;		//current action
	AS_VALUE			Register[AS_MAX_REGS];
	int					RegisterFlag[AS_MAX_REGS];
	int					Arguments;		//arguments number
	AS_OBJECT *			OriginalTarget; //used when reseting target (like this pointer)
};


#define SWF_GET_OBJECT(o) ((AS_OBJECT*)(o))
#define SWF_IS_OBJECT_CLASS(o,c) (SWF_GET_OBJECT(o)->Class == c)
#define SWF_GET_OBJECT_CLASS(o) (SWF_GET_OBJECT(o)->Class)
#define SWF_GET_FUNCTION(o) ((AS_FUNCTION*)o)

#define SWF_ALLOC_OBJECT(_s)\
	(AS_OBJECT*)swf_as_object_alloc(_s,AS_OBJECT_OBJECT,sizeof(AS_OBJECT),NULL)
#define SWF_ALLOC_ARRAY(_s)\
	(AS_ARRAY*)swf_as_object_alloc(_s,AS_OBJECT_ARRAY,sizeof(AS_ARRAY),NULL)
#define SWF_ALLOC_MCLOADER(_s)\
	(AS_MCLOADER*)swf_as_object_alloc(_s,AS_OBJECT_MCLOADER,sizeof(AS_MCLOADER),NULL)
#define SWF_ALLOC_COLOR(_s)\
	(AS_COLOR*)swf_as_object_alloc(_s,AS_OBJECT_COLOR,sizeof(AS_COLOR),NULL)
#define SWF_ALLOC_EXT_OBJECT(_s,_proto)\
	(AS_OBJECT*)swf_as_object_alloc(_s,AS_OBJECT_OBJECT,sizeof(AS_OBJECT),_proto)
#define SWF_ALLOC_FUNCTION(_s)\
	swf_as_function_alloc(_s)

#define SWF_IS_ILLEGAL_VALUE(val)\
	((val)->type == SWFDEC_AS_TYPE_NULL || ((val)->type == SWFDEC_AS_TYPE_NUMBER && !((val)->number == (val)->number)))

/************************************************************************/
/* Define Function                                                      */
/************************************************************************/

#ifdef __cplusplus
}
#endif

#endif
