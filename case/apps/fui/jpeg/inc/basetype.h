/*------------------------------------------------------------------------------
--																			  --
--		 This software is confidential and proprietary and may be used		  --
--		  only as expressly authorized by a licensing agreement from		  --
--																			  --
--							  Hantro Products Oy.							  --
--																			  --
--					 (C) COPYRIGHT 2006 HANTRO PRODUCTS OY					  --
--							  ALL RIGHTS RESERVED							  --
--																			  --
--				   The entire notice above must be reproduced				  --
--					on all copies and should not be removed.				  --
--																			  --
--------------------------------------------------------------------------------
--
--	Description : Basic type definitions.
--
--------------------------------------------------------------------------------
--
--	Version control information, please leave untouched.
--
--	$RCSfile: basetype.h,v $
--	$Revision: 1.1 $
--	$Date: 2006/12/20 14:21:31 $
--
------------------------------------------------------------------------------*/

#ifndef __BASETYPE_H__
#define __BASETYPE_H__

#ifndef NULL
#ifdef	__cplusplus
#define NULL	0
#else
#define NULL	((void *)0)
#endif
#endif

typedef unsigned char		u8;
typedef signed char 		i8;
typedef unsigned short		u16;
typedef signed short		i16;
typedef unsigned int		u32;
typedef signed int			i32;

#ifdef WIN32
typedef unsigned __int64	u64;
typedef __int64 			i64;
#else
typedef unsigned long long	u64;
typedef long long			i64;
#endif


#endif /* __BASETYPE_H__ */
