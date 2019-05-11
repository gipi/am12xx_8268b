/*------------------------------------------------------------------------------
--                                                                            --
--       This software is confidential and proprietary and may be used        --
--        only as expressly authorized by a licensing agreement from          --
--                                                                            --
--                            Hantro Products Oy.                             --
--                                                                            --
--                   (C) COPYRIGHT 2006 HANTRO PRODUCTS OY                    --
--                            ALL RIGHTS RESERVED                             --
--                                                                            --
--                 The entire notice above must be reproduced                 --
--                  on all copies and should not be removed.                  --
--                                                                            --
--------------------------------------------------------------------------------
--
--  Description :  dwl common part header file
--
------------------------------------------------------------------------------
--
--  Version control information, please leave untouched.
--
--  $RCSfile: dwl_linux.h,v $
--  $Revision: 1.5 $
--  $Date: 2007/04/03 17:06:35 $
--
------------------------------------------------------------------------------*/

#include "basetype.h"
#include "dwl.h"

#include "memalloc.h"
#include "dwl_linux_lock.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/timeb.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef USE_EFENCE
#include "efence.h"
#endif

#ifdef _DWL_DEBUG
//#define DWL_DEBUG(fmt, args...) printf(__FILE__ ":%d: " fmt, __LINE__ , ## args)
#define DWL_DEBUG(fmt, args...) printf(fmt, ## args)
#define REG_DUMP_FILE "swregdump.log"
#else
#define DWL_DEBUG(fmt, args...) /* not debugging: nothing */
#endif

#ifndef HX170DEC_IO_BASE
#define HX170DEC_IO_BASE   0x10080000U
#endif

#define HX170PP_REG_START       0xF0
#define HX170DEC_REG_START      0x4
#define HX170DEC_REG_AMOUNT     150

#define HX170DEC_REG_SIZE       ((HX170DEC_REG_AMOUNT+1)*4)
#define HX170PP_SYNTH_CFG       100
#define HX170DEC_SYNTH_CFG       50

#define DWL_DECODER_INT ((DWLReadReg(dec_dwl, HX170DEC_REG_START)>>12) & 0xFF)
#define DWL_PP_INT ((DWLReadReg(dec_dwl, HX170PP_REG_START)>>12) & 0xFF)

#define DWL_STREAM_ERROR_BIT 0x10000 /* 16th bit*/
#define DWL_HW_TIMEOUT_BIT 0x40000 /* 18th bit*/

/* Function prototypes */

/* wrapper information */
typedef struct hX170dwl
{
    u32 clientType;
    int fd; /* decoder device file */
    int fd_mem; /* /dev/mem for mapping */
    int fd_memalloc;    /* linear memory allocator */
    volatile u32 *pRegBase; /* IO mem base */
    u32 regSize;    /* IO mem size */
    u32 freeLinMem; /* Start address of free linear memory */
    u32 freeRefFrmMem;/* Start address of free reference frame memory */
    int semid;
#ifdef _DWL_DEBUG
    FILE *regDump;
#endif
	volatile u32 *pCMURegs;
}
hX170dwl_t;

