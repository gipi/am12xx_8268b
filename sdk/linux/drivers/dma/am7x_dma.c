/*
 * Driver for the Synopsys DesignWare DMA Controller (aka DMACA on
 * AVR32 systems.)
 *
 * Copyright (C) 2007-2008 Atmel Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/dmaengine.h>
#include <linux/dma-mapping.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>

#include "am7x_platform_device.h"

#ifdef PLATFORM_DMA_SUPPORT
/* The physical dma channel */
struct phy_dma_chan {
	unsigned int mode;
	unsigned int dest;
	unsigned int src;
	unsigned int count;
	unsigned int cmd;
};

struct dw_dma_chan {
	struct dma_chan chan;
	volatile struct phy_dma_chan *io;
};

struct dw_dma {
	struct dma_device   dma;
	struct dw_dma_chan  chan[6];
};

struct dw_dma_desc {
	int a;
};


static inline struct dw_dma_chan *to_dw_dma_chan(struct dma_chan *chan)
{
	return container_of(chan, struct dw_dma_chan, chan);
}

static inline struct dw_dma *to_dw_dma(struct dma_device *ddev)
{
	return container_of(ddev, struct dw_dma, dma);
}


static struct dma_async_tx_descriptor *
dwc_prep_dma_memcpy(struct dma_chan *chan, dma_addr_t dest, dma_addr_t src,
		size_t len, unsigned long flags)
{
	return NULL;
}

static struct dma_async_tx_descriptor *
dwc_prep_slave_sg(struct dma_chan *chan, struct scatterlist *sgl,
		unsigned int sg_len, enum dma_data_direction direction,
		unsigned long flags)
{
	struct dw_dma_chan	*dwc = to_dw_dma_chan(chan);
	return NULL;
}

static void dwc_terminate_all(struct dma_chan *chan)
{
	struct dw_dma_chan	*dwc = to_dw_dma_chan(chan);
}

static int dwc_alloc_chan_resources(struct dma_chan *chan,
		struct dma_client *client)
{
	return 0;
}

static void dwc_free_chan_resources(struct dma_chan *chan)
{
}

static enum dma_status
dwc_is_tx_complete(struct dma_chan *chan,
		dma_cookie_t cookie,
		dma_cookie_t *done, dma_cookie_t *used)
{
	return DMA_SUCCESS;
}

static void dwc_issue_pending(struct dma_chan *chan)
{
}


/*----------------------------------------------------------------------*/

static int __init dw_probe(struct platform_device *pdev)
{
	struct am7x_dma_platform_data *pdata;
	struct resource		*io;
	static struct dw_dma my_dma;
	struct dw_dma		*dw = &my_dma;
	size_t			size;
	int			irq;
	int			err;
	int			i;

	pdata = pdev->dev.platform_data;
	io = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!io)
		return -EINVAL;

	irq = platform_get_irq(pdev, 0);
	if (irq < 0)
		return irq;

	size = io->end - io->start + 1;
printk("io: %08x, 0x%04x\n", io->start, size);
	if (!request_mem_region(io->start, size, pdev->dev.driver->name)) {
		err = -EBUSY;
		goto err_kfree;
	}

	platform_set_drvdata(pdev, dw);

	INIT_LIST_HEAD(&dw->dma.channels);
	for (i = 0; i < pdata->nr_channels; i++) {
		struct dw_dma_chan *dwc = &dw->chan[i];
		dwc->io = (struct phy_dma_chan *) (DMA_MODE0 + 0x20 * i);
		list_add_tail(&dwc->chan.device_node, &dw->dma.channels);
	}
	dw->dma.chancnt = pdata->nr_channels;

	dma_cap_set(DMA_MEMCPY, dw->dma.cap_mask);
	dma_cap_set(DMA_SLAVE, dw->dma.cap_mask);
	dw->dma.dev = &pdev->dev;
	dw->dma.device_alloc_chan_resources = dwc_alloc_chan_resources;
	dw->dma.device_free_chan_resources = dwc_free_chan_resources;

	dw->dma.device_prep_dma_memcpy = dwc_prep_dma_memcpy;
	dw->dma.device_prep_slave_sg = dwc_prep_slave_sg;
	dw->dma.device_terminate_all = dwc_terminate_all;

	dw->dma.device_is_tx_complete = dwc_is_tx_complete;
	dw->dma.device_issue_pending  = dwc_issue_pending;

	printk(KERN_INFO "%s: DesignWare DMA Controller, %d channels\n",
			pdev->dev.bus_id, dw->dma.chancnt);

	dma_async_device_register(&dw->dma);

	return 0;

err_irq:
err_clk:
err_release_r:
	release_resource(io);
err_kfree:
	kfree(dw);
	return err;
}

static int __exit dw_remove(struct platform_device *pdev)
{
	return 0;
}

static void dw_shutdown(struct platform_device *pdev)
{
}

static int dw_suspend_late(struct platform_device *pdev, pm_message_t mesg)
{
	return 0;
}

static int dw_resume_early(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver dw_driver = {
	.remove		= __exit_p(dw_remove),
	.shutdown	= dw_shutdown,
	.suspend_late	= dw_suspend_late,
	.resume_early	= dw_resume_early,
	.driver = {
		.name	= AM7X_DEV_NAME_DMA,
		.owner  = THIS_MODULE,
	},
};

static int __init dw_init(void)
{
	int retval = platform_driver_probe(&dw_driver, dw_probe);
	if(retval != 0) 
		printk("%s failed, err=%d\n", __func__,retval);
	return retval;
}
module_init(dw_init);

static void __exit dw_exit(void)
{
	platform_driver_unregister(&dw_driver);
}
module_exit(dw_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Synopsys DesignWare DMA Controller driver");
MODULE_AUTHOR("Haavard Skinnemoen <haavard.skinnemoen@atmel.com>");
#endif

