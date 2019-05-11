#ifndef	CARD_DETECT_H
#define	CARD_DETECT_H 

#include <am_types.h>
/* card support definations*/
#define SUPPORT_CF
#define SUPPORT_XD

/*driver(.ko) file name*/
#define SD_DRV_NAME	"Am7xsd.ko"
#define MS_DRV_NAME	"Am7xms.ko"
#ifdef SUPPORT_XD
#define XD_DRV_NAME	"Am7xxd.ko"
#endif
#ifdef SUPPORT_CF
#define CF_DRV_NAME	"Am7xcf.ko"
#endif

/* gpio definations*/


#define INVALID_GPIO_PIN  0xFF
#define CARD_DET_NOTIN  0x00
#define CARD_DET_IN          0x01






#endif
