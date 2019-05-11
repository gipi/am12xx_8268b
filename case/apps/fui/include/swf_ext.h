#ifndef SWFEXT__H
#define SWFEXT__H

#include "swfdec.h"
#include "fui_list.h"

extern void *__as_handler;
extern int __as_param_num;
extern int __as_return_type;

#define SWFEXT_FUNC_BEGIN(_as) \
	{\
		__as_handler     = _as;\
		__as_param_num   = SWF_GetParamNum(_as);\
		__as_return_type = 0;\
	}

#define SWFEXT_FUNC_END() \
	{\
		if(!__as_return_type)\
		{\
			Swfext_PutNull();\
		}\
		return 0;\
	}

#define SWFEXT_REGISTER(_name, _func) \
	SWF_RegisterExtCall(_name, AS_OBJECT_EXTERNAL, _func)

extern int  Swfext_GetParamNum();
extern int  Swfext_GetParamType();
extern int  Swfext_GetNumber();
extern float Swfext_GetFloat();
extern char *Swfext_GetString();
extern void * Swfext_GetObject();
extern void   Swfext_PutNumber(int n);
extern void   Swfext_PutFloat(float n);
extern void   Swfext_PutString(char * str);
extern void   Swfext_PutObject(void * obj);
extern void   Swfext_PutNull();
extern int  Swfext_Register(void);

#endif
