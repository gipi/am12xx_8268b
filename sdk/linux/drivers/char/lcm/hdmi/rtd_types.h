/*=============================================================
 * Copyright (c)      Realtek Semiconductor Corporation, 2004 *
 * All rights reserved.                                       *
 *============================================================*/
/*======================= Description ========================*/
/**
  * @file
 * We define the usual types in this file.
 * We define the usual types in this file such as INT8,uin8,etc.
 * Please include this file in all source file. And declare your
 * variables with this re-defined type.
 * @Author Moya Yu
 * @date  2004/4/7
 * @version 1.0
 */


#ifndef _RTD_TYPES_H
#define _RTD_TYPES_H

/*
 * Internal names for basic integral types.  Omit the typedef if
 * not possible for a machine/compiler combination.
 */
#ifdef _WIN32 //for platform-acrossing of models, _WIN32: the pre-defined keyword of VC compiler
typedef 	unsigned __int64         UINT64; //!<for unsigned 64 bits integer in VC windows platform
typedef  __int64				INT64;//!<for signed 64 bits integer in VC windows platform
#else
typedef unsigned long long		UINT64; //!< for unsigned 64 bits integer
typedef long long				INT64;     //!< for signed 64 bits integer
#endif
typedef unsigned int			UINT32;   //!< for unsigned integer (32 bits)
typedef int					INT32;    //!< for signed integer (32 bits)
typedef unsigned short			UINT16;  //!< for unsigned short (16 bits)
typedef short					INT16;    //!< for signed short (16 bits)
typedef unsigned char			UINT8;   //!< for unsigned character (8 bits)
typedef char					INT8;    //!< for character (8 bits)
//typedef int BOOL;  //!< for boolean value
//typedef float					FLOAT32;	//!< for float point (32 bits)
//typedef double				FLOAT64;	//!< for float point (64 bits)

enum{FALSE = 0, TRUE};

#if 0
#ifndef bool
//typedef unsigned char	bool ;	//!< For Boolean declaration
#ifndef WIN32
typedef enum{FALSE = 0, TRUE} bool;
#endif
#endif

#ifndef BOOL
#define BOOL bool
#endif
#endif

#ifndef NULL
#define NULL (void *)0
#endif
/*
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
*/
#ifndef SUCCESS
#define SUCCESS 0
#endif

#ifndef FAILED
#define FAILED -1
#endif

#define CLEARBITS(a,b)	((a) &= ~(b))
#define SETBITS(a,b)		((a) |= (b))
#define ISSET(a,b)		(((a) & (b))!=0)
#define ISCLEARED(a,b)	(((a) & (b))==0)


#ifndef MAX
#define MAX(a,b)  (((a) > (b)) ? (a) : (b))
#endif			   /* max */

#ifndef MIN
#define MIN(a,b)  (((a) < (b)) ? (a) : (b))
#endif			   /* min */

#define ASSERT_CSP(x) if (!(x)) {printk("\nAssertion fail at file %s, function %s, line number %d:, expression '%s'", __FILE__, __FUNCTION__, __LINE__, #x); while(1);}
#define ASSERT_ISR(x) if (!(x)) {printk("\nAssertion fail at file %s, function %s, line number %d:, expression '%s'", __FILE__, __FUNCTION__, __LINE__, #x); while(1);}


typedef enum  {
        HD_COMPRESS_MODE = (1<<0),         // mvd always compress HD
        DDR_16BIT_MODE = (1<<1),         // DDR Data Bit
} FLAG_MVD_CONF;



#endif
