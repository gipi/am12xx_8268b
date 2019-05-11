#ifndef AM_TYPEDEF_H
#define AM_TYPEDEF_H
#if ! defined(_ASSEMBLER_)

#ifndef __cplusplus
#ifndef bool
typedef enum{F=0, T} bool;
#endif
#endif

#ifndef BOOL
#define BOOL bool
#endif

#ifndef NULL
#define NULL ((void*)0)
#endif

#ifndef TRUE
#define TRUE	1
#endif

#ifndef FALSE
#define FALSE	0
#endif

typedef unsigned char	INT8U;
typedef char			INT8S;
typedef unsigned short	INT16U;
typedef short			INT16S;
typedef unsigned int	INT32U;
typedef int			INT32S;

typedef unsigned long long	INT64U;
typedef long long			INT64S;
typedef float				FLOAT32;
typedef double				FLOAT64;

#endif
#endif //_TYPEDEF_H

