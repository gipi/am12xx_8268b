#if (!defined(__KERNEL__)) 
#include "mfp_config.h"
#include "actions_regs.h"

#if CONFIG_AM_CHIP_ID == 1213
const struct MFP_CONFIG am7051_nand_mfp_configs[] = {
	{ GPIO_MFCTL3, ~0x73300000, 0x11100000 },
	{ GPIO_MFCTL4, ~0xfff00733, 0x11100111 },
	{ 0,           ~0xffffffff, 0x00000000 },
};
#else
const struct MFP_CONFIG am7051_nand_mfp_configs[] = {
	{ GPIO_MFCTL3, ~0x73300000, 0x11100000 },
	{ GPIO_MFCTL4, ~0xfff00733, 0x11100111 },
	{ GPIO_MFCTL5, ~0x00000777, 0x00000111 },
	{ 0,           ~0xffffffff, 0x00000000 },
};
#endif

#endif
