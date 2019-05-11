/* arch/mips/am-mips/am7x-platform.c
 *
 * Copyright (C) 2009 Actions MicroEletronics Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Declare platform resources and register platform devices for
 * AM7XXX.
 *
 * Changelog:
 *
 *  2009/11/04: Pan Ruochen <reachpan@actions-micro.com>
 *				- Create the initial version
 *
 */
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/rtc.h>
#include <linux/types.h>
#include <linux/bcd.h>
#include <linux/delay.h>
#include <linux/spi/spi.h>
#include <linux/spi/flash.h>
#include "am7x_platform_device.h"

static unsigned int rtc_platdata;
static unsigned int sd_platdata;
static unsigned int lcm_platdata;
static unsigned int cec_platdata;
static unsigned int key_platdata;

#if CONFIG_AM_CHIP_ID == 1211 ||CONFIG_AM_CHIP_ID == 1220
static struct am7x_usb_platform_data init_param0 = {
	.vbus_check_gpio = 28,
	.is_otg = 1,
};
#elif CONFIG_AM_CHIP_ID == 1213
static struct am7x_usb_platform_data init_param0 = {
#ifdef CONFIG_AM_8251
	.vbus_check_gpio = 26,
#else
	.vbus_check_gpio = 96,
#endif
	.is_otg = 1,
};
static struct am7x_usb_platform_data init_param1 = {
#ifdef CONFIG_AM_8251
		.vbus_check_gpio = 26,
#else
		.vbus_check_gpio = 95,
#endif

	.is_otg = 1,
};
#endif

struct flash_platform_data am7x_flash_data={
	.type = NULL,
	.name="AH1216",
};


static struct resource rtc_resource[] = {
	[0]	= {
		.start	= KVA_TO_PA(RTC_CTL),
	#if CONFIG_AM_CHIP_ID == 1203
		.end	= KVA_TO_PA(RTC_STATUS) + 3,
	#elif CONFIG_AM_CHIP_ID == 1211 || CONFIG_AM_CHIP_ID == 1220 || CONFIG_AM_CHIP_ID == 1213
		.end	= KVA_TO_PA(RTC_ALARM) + 3,
	#else
	#error "no chip type!"
	#endif
		.flags	= IORESOURCE_MEM,
	},
};
static struct resource sd_resource[] = {
	[0]	= {
		.start	= KVA_TO_PA(CRC_CTL),
		.end	= KVA_TO_PA(CRC_CTL) + 20 - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1]	= { 
		.start	= IRQ_Card, 
		.end	= IRQ_Card, 
		.flags	= IORESOURCE_IRQ, 
	}, 
};

static struct resource usb_resource[] = {
	[0]	= { 
		.start	= KVA_TO_PA(USB_OTG_BASE), 
		.end	= KVA_TO_PA(USB_OTG_BASE) + 0x40F,
		.flags	= IORESOURCE_MEM, 
	}, 
	[1]	= { 
		.start	= IRQ_USB, 
		.end	= IRQ_USB, 
		.flags	= IORESOURCE_IRQ, 
	}, 
};
#if CONFIG_AM_CHIP_ID==1213
static struct resource usb_resource1[] = {
	[0]	= { 
		.start	= KVA_TO_PA(USB_OTG_BASE1), 
		.end	= KVA_TO_PA(USB_OTG_BASE1) + 0x40F,
		.flags	= IORESOURCE_MEM, 
	}, 
	[1]	= { 
		.start	= IRQ_USB2, 
		.end	= IRQ_USB2, 
		.flags	= IORESOURCE_IRQ, 
	}, 
};
#endif

static struct resource lcm_resource[] = {
	[0]	= { 
		.start = 0,
		.end = 10,	
		.flags	= IORESOURCE_MEM, 
	}, 

};

static struct resource cec_resource[] = {
	[0]	= { 
		.start = 0,
		.end = 10,	
		.flags	= IORESOURCE_MEM, 
	}, 

};

static struct resource key_resource[] = {
	[0]	= {
		.start	= KVA_TO_PA(IR_REMOTE_CTRL),
		.end	= KVA_TO_PA(IR_REMOTE_CTRL) + 0x34,
		.flags	= IORESOURCE_MEM,
	},
};

static struct platform_device am7x_device_rtc = {
	.name		= AM7X_DEV_NAME_RTC,
	.id		= 0,
	.num_resources	= ARRAY_SIZE(rtc_resource),
	.resource	= rtc_resource,
	.dev		= {
		.platform_data = &rtc_platdata,
		.coherent_dma_mask = ~0,
	}

};

#if CONFIG_AM_CHIP_ID==1213
static struct resource spi_resource1[] = {
	[0]	= {
		.start	= KVA_TO_PA(SPI1_CTL),
		.end	= KVA_TO_PA(SPI1_CTL) + 0x14,
		.flags	= IORESOURCE_MEM,
	},
};
#endif

static struct resource spi_resource[] = {
	[0]	= {
		.start	= KVA_TO_PA(SPI_CTL),
		.end	= KVA_TO_PA(SPI_CTL) + 0x14,
		.flags	= IORESOURCE_MEM,
	},
};


#ifdef PLATFORM_DMA_SUPPORT
static struct am7x_dma_platform_data dma_platdata = {
	.nr_channels = 4,
};

static struct resource dma_resource[] = {
	[0]	= { 
		.start	= KVA_TO_PA(DMA_CTL), 
		.end	= KVA_TO_PA(DMA_CTL) + 0x200 - 1,
		.flags	= IORESOURCE_MEM, 
	}, 
	[1]	= { 
		.start	= IRQ_DMA, 
		.end	= IRQ_DMA, 
		.flags	= IORESOURCE_IRQ, 
	}, 
};


static struct platform_device am7x_device_dma = {
	.name		= AM7X_DEV_NAME_DMA,
	.id		= 1,
	.num_resources	= ARRAY_SIZE(dma_resource),
	.resource	= dma_resource,
	.dev		= {
		.platform_data = &dma_platdata,
		.coherent_dma_mask = ~0,
	}

};
#endif

static struct platform_device am7x_device_sd = {
	.name		= AM7X_DEV_NAME_SD,
	.id		= 1,
	.num_resources	= ARRAY_SIZE(sd_resource),
	.resource	= sd_resource,
	.dev		= {
		.platform_data = &sd_platdata,
		.coherent_dma_mask = ~0,
	}

};

static struct platform_device am7x_device_usb = {
	.name		= AM7X_DEV_NAME_USB,
	.id		= 2,
	.num_resources	= ARRAY_SIZE(usb_resource),
	.resource	= usb_resource,
	.dev		= {
		.platform_data = &init_param0,
		.coherent_dma_mask = ~0,
	}
};
#if CONFIG_AM_CHIP_ID == 1213
static struct platform_device am7x_device_usb1 = {
	.name		= AM7X_DEV_NAME_USB1,
	.id		= 2,
	.num_resources	= ARRAY_SIZE(usb_resource1),
	.resource	= usb_resource1,
	.dev		= {
		.platform_data = &init_param1,
		.coherent_dma_mask = ~0,
	}
};
#endif

static struct platform_device am7x_device_lcm = {
	.name		= AM7X_DEV_NAME_LCM,
	.id		= 2,
	.num_resources	= ARRAY_SIZE(lcm_resource),
	.resource	= lcm_resource,
	.dev		= {
		.platform_data = &lcm_platdata,
		.coherent_dma_mask = ~0,
	}
};

static struct platform_device am7x_device_cec = {
	.name		= AM7X_DEV_NAME_CEC,
	.id		= 2,
	.num_resources	= ARRAY_SIZE(cec_resource),
	.resource	= cec_resource,
	.dev		= {
		.platform_data = &cec_platdata,
		.coherent_dma_mask = ~0,
	}
};

#ifdef CONFIG_KS884X
static struct resource net_ksz884x_resource[] = {
	[0]	= { 
		.start	= LAN_DEVICE_BASE, 
		.end	= LAN_DEVICE_BASE + 0x200 - 1,
		.flags	= IORESOURCE_MEM, 
	}, 
	[1]	= { 
		.start	= IRQ_EXT, 
		.end	= IRQ_EXT, 
		.flags	= IORESOURCE_IRQ, 
	}, 
};

static struct platform_device am7x_device_net_ksz884x = {
	.name           = AM7X_DEV_NAME_NET_KSZ884X,
	.id		        = -1,
	.num_resources	= ARRAY_SIZE(net_ksz884x_resource),
	.resource	    = net_ksz884x_resource,
	.dev		    = {
		.platform_data     = NULL,
		.coherent_dma_mask = ~0,
	}
};
#endif

static struct platform_device am7x_device_key = {
	.name		= AM7X_DEV_NAME_KBD,
	.id		= 1,
	.num_resources	= ARRAY_SIZE(key_resource),
	.resource	= key_resource,
	.dev		= {
		.platform_data = &key_platdata,
		.coherent_dma_mask = ~0,
	}
};

static struct am7x_spi_info{
	u8 num_cs;
	u8 bus_num;
	void (*set_cs)(u8 cs,int value);
};


static struct am7x_spi_info am7x_spi_platdata[]={
	[0]={
		.num_cs = 3,
		.bus_num = 0,
		.set_cs = NULL,   //&setcs,
	},
	[1]={
		.num_cs = 3,
		.bus_num = 1,
		.set_cs = NULL,   //&setcs,
	},

};

static struct platform_device am7x_device_spi = {
	.name		= "am7x_spi",
	.id		= 1,
	.num_resources	= ARRAY_SIZE(spi_resource),
	.resource	= spi_resource,
	.dev		= {
		.platform_data = &am7x_spi_platdata[0],
	}
};

#if CONFIG_AM_CHIP_ID == 1213
static struct platform_device am7x_device_spi1 = {
	.name		= "am7x_spi_1",
	.id		= 2,
	.num_resources	= ARRAY_SIZE(spi_resource1),
	.resource	= spi_resource1,
	.dev		= {
		.platform_data = &am7x_spi_platdata[1],
	}
};
#endif

static struct resource net_resource[] = {
	[0]	= {
		.start	= 0,
		.end	=0,
		.flags	= IORESOURCE_MEM,
	},
};

static struct platform_device am7x_device_net = {
	.name		= AM7X_DEV_NAME_NET,
	.id		= 0,
	.num_resources = ARRAY_SIZE(net_resource),
	.resource = net_resource,
	.dev		    = {
		.platform_data     = NULL,
		.coherent_dma_mask = ~0,
	}
};

static struct platform_device *am7x_platform_devices[] __initdata = {
	&am7x_device_rtc,
#ifdef PLATFORM_DMA_SUPPORT
	&am7x_device_dma,
#endif
	&am7x_device_sd,
	&am7x_device_usb,
#if CONFIG_AM_CHIP_ID==1213
	&am7x_device_usb1,
#endif
	&am7x_device_lcm,
	&am7x_device_key,
	&am7x_device_spi,
#ifdef CONFIG_KS884X
	&am7x_device_net_ksz884x,
#endif
	&am7x_device_net,
	&am7x_device_cec,
};


static struct spi_board_info spi_user_info[]={
	[0] = {
		.modalias = "spidev",
		.mode = SPI_MODE_0,
		.max_speed_hz = 1,
		.bus_num = 0,
		.chip_select = 3,
		.irq  = IRQ_EXT23,
	},

	[1] = {
		.modalias = "m25p80",
		.mode = SPI_MODE_0,
		.max_speed_hz = 20,
		.bus_num = 0,
		.chip_select = 1,
		.irq  = IRQ_EXT23,
		.platform_data = &am7x_flash_data,
	}

};


static __init int am7x_platform_devices_init(void)
{
	int retval;

	spi_register_board_info(spi_user_info,ARRAY_SIZE(spi_user_info));

	retval = platform_add_devices(am7x_platform_devices,
		    ARRAY_SIZE(am7x_platform_devices));
	if(retval < 0)
		printk(KERN_CRIT"platform_add_devices failed, err=%d\n", retval);
	return retval;
}

fs_initcall(am7x_platform_devices_init);

