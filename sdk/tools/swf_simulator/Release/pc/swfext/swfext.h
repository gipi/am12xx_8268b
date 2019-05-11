#ifndef SWFEXT__H
#define SWFEXT__H

#include "swfdec.h"

extern void*  __as_handler;
extern INT32S __as_param_num;
extern INT32S __as_return_type;

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

INT32S  Swfext_GetParamNum();
INT32S  Swfext_GetParamType();
INT32S  Swfext_GetNumber();
FLOAT32 Swfext_GetFloat();
INT8S * Swfext_GetString();
void *  Swfext_GetObject();
void    Swfext_PutNumber(AS_NUMBER n);
void    Swfext_PutFloat(FLOAT32 n);
void    Swfext_PutString(INT8S * str);
void    Swfext_PutObject(void * obj);
void    Swfext_PutNull();
INT32S	Swfext_Register(void);
#endif
