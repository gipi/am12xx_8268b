/* zconf.h -- configuration of the zlib compression library
 * Copyright (C) 1995-2005 Jean-loup Gailly.
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#ifndef ZCONF_H
#define ZCONF_H

/* No gzip supprot to supress the code size */
#define NO_GZIP

/* Maximum value for windowBits in deflateInit2 and inflateInit2. */
#ifndef MAX_WBITS
#  define MAX_WBITS   15 /* 4K LZ77 window */
#endif

/* Type declarations */
#ifndef OF /* function prototypes */
#  ifdef STDC
#    define OF(args)  args
#  else
#    define OF(args)  ()
#  endif
#endif

#ifndef ZEXTERN
#  define ZEXTERN extern
#endif
#ifndef ZEXPORT
#  define ZEXPORT
#endif
#ifndef ZEXPORTVA
#  define ZEXPORTVA
#endif

#ifndef FAR
#define FAR
#endif

#define z_off_t long

typedef unsigned int   uInt;  /* 16 bits or more */
typedef unsigned long  uLong; /* 32 bits or more */
typedef unsigned char  Byte;  /* 8 bits */

typedef Byte  FAR Bytef;
typedef char  FAR charf;
typedef int   FAR intf;
typedef uInt  FAR uIntf;
typedef uLong FAR uLongf;

typedef void const *voidpc;
typedef void FAR   *voidpf;
typedef void       *voidp;

#endif /* ZCONF_H */

