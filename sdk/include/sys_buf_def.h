#ifndef __SYS_BUFFER_DEF_H__
#define __SYS_BUFFER_DEF_H__

#include "syscfg.h"

#if CONFIG_AM_CHIP_ID==1203 || CONFIG_AM_CHIP_ID == 1211 || CONFIG_AM_CHIP_ID == 1220 || CONFIG_AM_CHIP_ID == 1213
/*---------------------------------*/
#define VMA_SNOR_BREC       0x80800000
#define LMA_SNOR_BREC       0xa0800000
#define VMA_NAND_BREC       0x80800000
#define LMA_NAND_BREC       0xa0800000

#define NAND_BREC_SIZE  0x20000  // 0xf000 60K   11800 70K  0x10000 64K

/*---------------------------------*/
#endif

/*---------------------------------*/
#define VMA_KERNEL     0x80003000
#define LMA_KERNEL     0xa0003000
/*---------------------------------*/


#define SNOR_BREC_OFFSET  	0x2000
#define SNOR_BREC_SIZE  	0xf000

#endif

