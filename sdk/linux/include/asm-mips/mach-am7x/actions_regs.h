#ifndef  __ACTIONS_REGISTER_H__
#define  __ACTIONS_REGISTER_H__
/**
*@file actions_regs.h
*
*@brief This file is the register entry of actions chip am7x31
*
*@author yekai
*@date 2010-07-22
*@version 0.1
*/

#if CONFIG_AM_CHIP_ID ==1203
#include "actions_1203_regs.h"
#elif CONFIG_AM_CHIP_ID == 1211
#include "actions_1211_regs.h"
#elif CONFIG_AM_CHIP_ID == 1220
#include "actions_1220_regs.h"
#elif CONFIG_AM_CHIP_ID == 1213
#include "actions_1213_regs.h"
#else
# error -------------- 
#endif
/* Remove P_ style stuff ... */

#endif

