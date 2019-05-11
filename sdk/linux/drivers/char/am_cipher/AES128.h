
#include <actions_regs.h>

#define AES_CTL			0xb0208000+0x00000000
#define AES_TEXT_IN		0xb0208000+0x00000008
#define AES_TEXT_OUT	0xb0208000+0x0000000c
#define AES_KEY_P0		0xb0208000+0x00000010
#define AES_KEY_P1		0xb0208000+0x00000014
#define AES_KEY_P2		0xb0208000+0x00000018
#define AES_KEY_P3		0xb0208000+0x0000001c
#define AES_CTR_LOW		0xB0208000+0x00000020  
#define AES_CTR_HIG		0xB0208000+0x00000024
#define AES_RIV_LOW		0xB0208000+0x00000028  
#define AES_RIV_HIG		0xB0208000+0x0000002c

#define MASK_COST_Read  0x30000000

#define act_writel(val,reg)  (*(volatile u32 *)(reg) = (val))
#define act_readl(port)  (*(volatile u32 *)(port))

