#ifndef CARDDET_API_H
#define CARDDET_API_H

#include "am_types.h"

INT32U Get_CardType(INT8U slot_No);
INT32U SD_Power_CTRL(INT8U bFlag);
int card_powerreset(INT16U delay1,INT16U delay2);
INT32U card_detecting(void);
int CFcard_detecting(void);
INT32U Card_SwitchClock(INT8U clock_freq);
INT32U Get_CardClock(void);
INT32U Card_Reader_Enable(void);
INT32U SD_Detect_WP(void);
int CF_Power_CTRL(INT8U bFlag);
//struct card_init_type * Get_CardDet_Info(void);
struct card_Com_info  * Get_ComomInfo(void);
#endif
