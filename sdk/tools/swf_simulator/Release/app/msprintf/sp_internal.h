#ifndef __SP_INTERNAL_H__
#define __SP_INTERNAL_H__

/*
#ifdef MIPS_VERSION
#include "console.h"
#else
#include <stdarg.h>
#endif
*/

#include <math.h>
#include <string.h>

#ifdef WIN32
#define	__inline__ 
#endif

#ifdef WIN32
typedef unsigned __int64    UINT64;
typedef __int64 			INT64;
#else
typedef unsigned long long	UINT64;
typedef long long			INT64;
#endif

#define is_digit(c) ((c) >= '0' && (c) <= '9')

#ifndef _FILE_DEFINED
typedef struct {
  char *_ptr;
  int   _cnt;
  char *_base;
  int   _flag;
  int   _file;
  int   _ungotchar;
  int   _bufsiz;
  char *_name_to_remove;
} FILE;

#define _FILE_DEFINED
#endif

#ifndef NULL
#define NULL (void*)0
#endif

typedef struct {
  unsigned int mantissa:23;
  unsigned int exponent:8;
  unsigned int sign:1;
} float_t;

typedef struct {
  unsigned int mantissal:32;
  unsigned int mantissah:20;
  unsigned int exponent:11;
  unsigned int sign:1;
} double_t;

typedef struct {
  unsigned int mantissal:32;
  unsigned int mantissah:32;
  unsigned int exponent:15;
  unsigned int sign:1;
  unsigned int empty:16;
} long_double_t;


//int isnan(double x);
//int isinf(double x);


//double mmodf(double __x, double *__i);


#endif
