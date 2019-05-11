#ifndef _TYPEDEF_H
#define _TYPEDEF_H

//#define MIPS
//#define WIN32

#ifdef WIN32
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif

#ifdef WIN32
typedef int BOOL;
#define TRUE	1
#define FALSE	0

typedef unsigned long		DWORD;
typedef unsigned short		WORD;
typedef unsigned char		BYTE;

typedef unsigned long		*LPDWORD;
typedef unsigned short		*LPWORD;
typedef unsigned char		*LPBYTE;
typedef unsigned __int64	UINT64;
typedef __int64				INT64;

typedef unsigned char       U8;
typedef signed char         I8;
typedef unsigned short      U16;
typedef signed short        I16;
typedef unsigned long       U32;
typedef signed long         I32;
typedef unsigned __int64	U64;
typedef signed __int64		I64;
typedef float               F32;
typedef double              F64;

typedef unsigned char       u8;
typedef signed char         i8;
typedef unsigned short      u16;
typedef signed short        i16;
typedef unsigned long       u32;
typedef signed long         i32;
typedef unsigned __int64	u64;
typedef signed __int64		i64;
typedef float               f32;
typedef double              f64;
#endif	//WIN32

#ifdef MIPS

#define TRUE	1
#define FALSE	0

typedef unsigned long	DWORD;
typedef unsigned short	WORD;
typedef unsigned char	BYTE;

typedef unsigned long		*LPDWORD;
typedef unsigned short		*LPWORD;
typedef unsigned char		*LPBYTE;



typedef unsigned char       U8;
typedef signed char         I8;
typedef unsigned short      U16;
typedef signed short        I16;
typedef unsigned long       U32;
typedef signed long         I32;
typedef unsigned long long 	U64;
typedef signed long long	I64;
typedef float               F32;
typedef double              F64;

typedef unsigned char       u8;
typedef signed char         i8;
typedef unsigned short      u16;
typedef signed short        i16;
typedef unsigned long       u32;
typedef signed long         i32;
typedef unsigned long long 	u64;
typedef signed long long	i64;
typedef float               f32;
typedef double              f64;
#endif	//MIPS

#ifndef NULL
#define NULL ((void*)0)
#endif

//#ifndef WIN32
//#include "misc.h"
//#endif

#endif //_TYPEDEF_H

