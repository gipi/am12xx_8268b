/*
 *  linux/drivers/mmc/sdio.c
 *
 *  Copyright 2006-2007 Pierre Ossman
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 */

#include <linux/err.h>

#include <linux/mmc/host.h>
#include <linux/mmc/card.h>
#include <linux/mmc/sdio.h>
#include <linux/mmc/sdio_func.h>

#include "core.h"
#include "bus.h"
#include "sdio_bus.h"
#include "mmc_ops.h"
#include "sd_ops.h"
#include "sdio_ops.h"
#include "sdio_cis.h"

static int sdio_read_fbr(struct sdio_func *func)
{
	int ret;
	unsigned char data;

	ret = mmc_io_rw_direct(func->card, 0, 0,
		SDIO_FBR_BASE(func->num) + SDIO_FBR_STD_IF, 0, &data);
	if (ret)
		goto out;

	data &= 0x0f;

	if (data == 0x0f) {
		ret = mmc_io_rw_direct(func->card, 0, 0,
			SDIO_FBR_BASE(func->num) + SDIO_FBR_STD_IF_EXT, 0, &data);
		if (ret)
			goto out;
	}

	func->class = data;

out:
	return ret;
}

static int sdio_init_func(struct mmc_card *card, unsigned int fn)
{
	int ret;
	struct sdio_func *func;

	BUG_ON(fn > SDIO_MAX_FUNCS);

	func = sdio_alloc_func(card);
	if (IS_ERR(func))
		return PTR_ERR(func);

	func->num = fn;

	ret = sdio_read_fbr(func);
	if (ret)
		goto fail;

	ret = sdio_read_func_cis(func);
	if (ret)
		goto fail;

	card->sdio_func[fn - 1] = func;

	return 0;

fail:
	/*
	 * It is okay to remove the function here even though we hold
	 * the host lock as we haven't registered the device yet.
	 */
	sdio_remove_func(func);
	return ret;
}

static int sdio_read_cccr(struct mmc_card *card)
{
	int ret;
	int cccr_vsn;
	unsigned char data;

	memset(&card->cccr, 0, sizeof(struct sdio_cccr));

	ret = mmc_io_rw_direct(card, 0, 0, SDIO_CCCR_CCCR, 0, &data);
	if (ret)
		goto out;

	cccr_vsn = data & 0x0f;

	if (cccr_vsn > SDIO_CCCR_REV_1_20) {
		printk(KERN_ERR "%s: unrecognised CCCR structure version %d\n",
			mmc_hostname(card->host), cccr_vsn);
		return -EINVAL;
	}

	card->cccr.sdio_vsn = (data & 0xf0) >> 4;

	ret = mmc_io_rw_direct(card, 0, 0, SDIO_CCCR_CAPS, 0, &data);
	if (ret)
		goto out;

	if (data & SDIO_CCCR_CAP_SMB)
		card->cccr.multi_block = 1;
	if (data & SDIO_CCCR_CAP_LSC)
		card->cccr.low_speed = 1;
	if (data & SDIO_CCCR_CAP_4BLS)
		card->cccr.wide_bus = 1;

	if (cccr_vsn >= SDIO_CCCR_REV_1_10) {
		ret = mmc_io_rw_direct(card, 0, 0, SDIO_CCCR_POWER, 0, &data);
		if (ret)
			goto out;

		if (data & SDIO_POWER_SMPC)
			card->cccr.high_power = 1;
	}

	if (cccr_vsn >= SDIO_CCCR_REV_1_20) {
		ret = mmc_io_rw_direct(card, 0, 0, SDIO_CCCR_SPEED, 0, &data);
		if (ret)
			goto out;

		if (data & SDIO_SPEED_SHS)
			card->cccr.high_speed = 1;
	}

out:
	return ret;
}




static int sdio_disable_cd(struct mmc_card *card)
{
	int ret;
	u8 ctrl;

	
	ret = mmc_io_rw_direct(card, 0, 0, SDIO_CCCR_IF, 0, &ctrl);
	if (ret)
		return ret;

	ctrl |= SDIO_BUS_CD_DISABLE;

	return mmc_io_rw_direct(card, 1, 0, SDIO_CCCR_IF, ctrl, &ctrl);
}



static int mmc_sdio_switch_hs(struct mmc_card *card, int enable)
{
	int ret;
	u8 speed;
	u8 tmp;

#if 0
	if (!(card->host->caps & MMC_CAP_SD_HIGHSPEED))
		return 0;
#endif

	if (!card->cccr.high_speed)
		return 0;

	ret = mmc_io_rw_direct(card, 0, 0, SDIO_CCCR_SPEED, 0, &speed);
	if (ret)
		return ret;

	if (enable)
		speed |= SDIO_SPEED_EHS;
	else
		speed &= ~SDIO_SPEED_EHS;

	ret = mmc_io_rw_direct(card, 1, 0, SDIO_CCCR_SPEED, speed, NULL);
	if (ret)
		return ret;


	ret = mmc_io_rw_direct(card, 0, 0, SDIO_CCCR_SPEED, 0, &tmp);
	if (ret)
		return ret;


	if(tmp !=speed)
		printk("mmc_sdio_switch_hs fail,0x%x 0x%x!\n",tmp,speed);
    else
        printk("mmc_sdio_switch_hs OK,0x%x 0x%x!\n",tmp,speed);    

	return 0;
}




static int sdio_enable_wide(struct mmc_card *card)
{
	int ret;
	u8 ctrl;

	if (!(card->host->caps & MMC_CAP_4_BIT_DATA))
		return 0;

	if (card->cccr.low_speed && !card->cccr.wide_bus)
		return 0;

	ret = mmc_io_rw_direct(card, 0, 0, SDIO_CCCR_IF, 0, &ctrl);
	if (ret)
		return ret;

	ctrl |= SDIO_BUS_WIDTH_4BIT;

	ret = mmc_io_rw_direct(card, 1, 0, SDIO_CCCR_IF, ctrl, NULL);
	if (ret)
		return ret;

	mmc_set_bus_width(card->host, MMC_BUS_WIDTH_4);

	return 0;
}

/*
 * Host is being removed. Free up the current card.
 */
static void mmc_sdio_remove(struct mmc_host *host)
{
	int i;

	BUG_ON(!host);
	BUG_ON(!host->card);

	for (i = 0;i < host->card->sdio_funcs;i++) {
		if (host->card->sdio_func[i]) {
			sdio_remove_func(host->card->sdio_func[i]);
			host->card->sdio_func[i] = NULL;
		}
	}

	mmc_remove_card(host->card);
	host->card = NULL;
}

/*
 * Card detection callback from host.
 */
static void mmc_sdio_detect(struct mmc_host *host)
{
	int err;

	BUG_ON(!host);
	BUG_ON(!host->card);

	mmc_claim_host(host);

	/*
	 * Just check if our card has been removed.
	 */
	err = mmc_select_card(host->card);

	mmc_release_host(host);

	if (err) {
		mmc_sdio_remove(host);

		mmc_claim_host(host);
		mmc_detach_bus(host);
		mmc_release_host(host);
	}
}


static const struct mmc_bus_ops mmc_sdio_ops = {
	.remove = mmc_sdio_remove,
	.detect = mmc_sdio_detect,
};


/*
 * Starting point for SDIO card init.
 */
int mmc_attach_sdio(struct mmc_host *host, u32 ocr)
{
	int err;
	int i, funcs;
	unsigned char data;
	unsigned blksz;
	struct mmc_card *card;
    int data_buf[256];
//	unsigned char data_buf1[256];

	BUG_ON(!host);
	WARN_ON(!host->claimed);

	mmc_attach_bus(host, &mmc_sdio_ops);

	/*
	 * Sanity check the voltages that the card claims to
	 * support.
	 */
	if (ocr & 0x7F) {
		printk(KERN_WARNING "%s: card claims to support voltages "
		       "below the defined range. These will be ignored.\n",
		       mmc_hostname(host));
		ocr &= ~0x7F;
	}

	if (ocr & MMC_VDD_165_195) {
		printk(KERN_WARNING "%s: SDIO card claims to support the "
		       "incompletely defined 'low voltage range'. This "
		       "will be ignored.\n", mmc_hostname(host));
		ocr &= ~MMC_VDD_165_195;
	}

	host->ocr = mmc_select_voltage(host, ocr);

	/*
	 * Can we support the voltage(s) of the card(s)?
	 */
	if (!host->ocr) {
		err = -EINVAL;
		goto err;
	}

	/*
	 * Inform the card of the voltage
	 */
	err = mmc_send_io_op_cond(host, host->ocr, &ocr);
	if (err)
		goto err;

	/*
	 * For SPI, enable CRC as appropriate.
	 */
	if (mmc_host_is_spi(host)) {
		err = mmc_spi_set_crc(host, use_spi_crc);
		if (err)
			goto err;
	}

	/*
	 * The number of functions on the card is encoded inside
	 * the ocr.
	 */
	funcs = (ocr & 0x70000000) >> 28;

	/*
	 * Allocate card structure.
	 */
	card = mmc_alloc_card(host, NULL);
	if (IS_ERR(card)) {
		err = PTR_ERR(card);
		goto err;
	}

	card->type = MMC_TYPE_SDIO;
	card->sdio_funcs = funcs;

	host->card = card;

	/*
	 * For native busses:  set card RCA and quit open drain mode.
	 */
	if (!mmc_host_is_spi(host)) {
		err = mmc_send_relative_addr(host, &card->rca);
		if (err)
			goto remove;

		mmc_set_bus_mode(host, MMC_BUSMODE_PUSHPULL);
	}

	/*
	 * Select card, as all following commands rely on that.
	 */
	if (!mmc_host_is_spi(host)) {
		err = mmc_select_card(card);
		if (err)
			goto remove;
	}

	/*
	 * Read the common registers.
	 */
	err = sdio_read_cccr(card);
	if (err)
		goto remove;

	/*
	 * Read the common CIS tuples.
	 */
	err = sdio_read_common_cis(card);
	if (err)
		goto remove;

	/*
	 * No support for high-speed yet, so just set
	 * the card's maximum speed.
	 */

	/*
	 * If needed, disconnect card detection pull-up resistor.
	 */



#if 1
	err = sdio_disable_cd(card);
	if (err)
		goto remove;

	mmc_delay(10);
	
#endif

#if 1
	err = mmc_sdio_switch_hs(card, 1);
    if (err)
		goto remove;
    printk("%s %d !\n",__func__,__LINE__);
	
	mmc_set_timing(card->host, MMC_TIMING_SD_HS);
	printk("%s %d !\n",__func__,__LINE__);
    mmc_set_clock(host, 80000000);
#endif
	
	
	

#if 1
	/*
	 * Switch to wider bus (if supported).
	 */
	err = sdio_enable_wide(card);
	printk("%s %d !\n",__func__,__LINE__);
	if (err > 0)
		mmc_set_bus_width(card->host, MMC_BUS_WIDTH_4);
	else if (err)
		goto remove;
	printk("%s %d !\n",__func__,__LINE__);
#endif

   

	/*
	 * Initialize (but don't add) all present functions.
	 */
	for (i = 0;i < funcs;i++) {
		err = sdio_init_func(host->card, i + 1);
		if (err)
			goto remove;
	}

    
	mmc_release_host(host);

	/*
	 * First add the card to the driver model...
	 */
	err = mmc_add_card(host->card);
	if (err)
		goto remove_added;

	/*
	 * ...then the SDIO functions.
	 */
	for (i = 0;i < funcs;i++) {
		err = sdio_add_func(host->card->sdio_func[i]);
#if 0
	    mmc_claim_host(host);
        sdio_enable_func(host->card->sdio_func[i]);
        sdio_set_block_size(host->card->sdio_func[i],512);

        data_buf[0]=4;
        data_buf[1]=5;
        data_buf[2]=6;
        data_buf[3]=7;
        mmc_io_rw_extended(host->card,1,1,0x10200,1,data_buf,1,4);
        
        mmc_io_rw_extended(host->card,0,1,0x10200,1,data_buf,1,4);
        

        mmc_release_host(host);
#endif
        
        if (err)
		goto remove_added;
	}
#if 0
    mmc_claim_host(host);
    mmc_io_rw_extended(host->card,0,0,0,1,data_buf,1,256);
    mmc_release_host(host);
#endif

    #if 0
	 mmc_claim_host(host);



	blksz=512;

	err = mmc_io_rw_direct(card, 1, 0,
	SDIO_FBR_BLKSIZE,
	blksz & 0xff, NULL);
	if (err)
		return err;
	err = mmc_io_rw_direct(card, 1, 0,
	SDIO_FBR_BLKSIZE + 1,
	(blksz >> 8) & 0xff, NULL);
	if (err)
		return err;


	
	err = mmc_io_rw_direct(card, 0, 0, SDIO_FBR_BLKSIZE,0, &data);
	printk("%s %d i=%d,data:0x%x\n",__func__,__LINE__,0,data);
	err = mmc_io_rw_direct(card, 0, 0,SDIO_FBR_BLKSIZE + 1,0, &data);
	printk("%s %d i=%d,data:0x%x\n",__func__,__LINE__,1,data);
	err = mmc_io_rw_direct(card, 0, 0, 0x100 + SDIO_FBR_BLKSIZE,0, &data);
	printk("%s %d i=%d,data:0x%x\n",__func__,__LINE__,0,data);
	err = mmc_io_rw_direct(card, 0, 0,0x100 + SDIO_FBR_BLKSIZE + 1,0, &data);
	printk("%s %d i=%d,data:0x%x\n",__func__,__LINE__,1,data);


	for(i=0;i<0x20;i++)
	{
		mmc_io_rw_direct(host->card, 0, 0, i, 0, &data_buf1[i]);	
	}
	printk("CCCR is:\n");
	for(i=0;i<0x20;i++)
	{
		printk("  %02x",data_buf1[i]);
		if((i+1)%10 == 0)
			printk("\n");
	}
	for(i=0;i<0x20;i++)
	{
		mmc_io_rw_direct(host->card, 0, 0, SDIO_FBR_BASE(1)+i, 0, &data_buf1[i]);	
	}
	printk("FBR1 is:\n");
	for(i=0;i<0x20;i++)
	{
		printk("  %02x",data_buf1[i]);
		if((i+1)%10 == 0)
			printk("\n");
	}



	while(1)
	{
#if 1
		data_buf[0]=0x5a;
		data_buf[1]=0xa5;
		mmc_io_rw_extended(host->card,1,0,0xf0,1,data_buf,1,1);
		mmc_delay(10);	
		//sdio_write_func(card,0,SDIO_CCCR_ABORT,0);
		mmc_io_rw_direct(host->card, 0, 0, 0x0C, NULL, &data_buf1[0]);
		mmc_io_rw_direct(host->card, 0, 0, 0xf0, NULL, &data_buf1[1]);
		printk("The read data:0x%x 0x%x\n",data_buf1[0],data_buf1[1]);
		
#else
		mmc_io_rw_extended(host->card,0,0,0,0,data_buf,1,1);
		sdio_write_func(card,0,SDIO_CCCR_ABORT,0);
		mmc_delay(10);
		printk("%s %d \n",__func__,__LINE__);
		
		mmc_io_rw_extended(host->card,0,0,0,1,data_buf,1,1);
		sdio_write_func(card,0,SDIO_CCCR_ABORT,0);
		printk("%s %d \n",__func__,__LINE__);
		mmc_delay(10);

		mmc_io_rw_extended(host->card,0,1,0,0,data_buf,1,1);
		sdio_write_func(card,0,SDIO_CCCR_ABORT,1);
		printk("%s %d \n",__func__,__LINE__);
		mmc_delay(10);

		mmc_io_rw_extended(host->card,0,1,0,1,data_buf,1,1);
		sdio_write_func(card,0,SDIO_CCCR_ABORT,1);
		printk("%s %d \n",__func__,__LINE__);
		mmc_delay(10);
		#endif
	}

    mmc_release_host(host);
    for(i=0;i<512/16;i++)
        printk("CCCR:i=%d 0x%x 0x%x 0x%x 0x%x \n",i*4,data_buf[i*4],data_buf[i*4+1],data_buf[i*4+2],data_buf[i*4+3]);



    
#endif
    printk("mmc_attach_sdio ok!\n");
	return 0;


remove_added:
	/* Remove without lock if the device has been added. */
	mmc_sdio_remove(host);
	mmc_claim_host(host);
remove:
	/* And with lock if it hasn't been added. */
	if (host->card)
		mmc_sdio_remove(host);
err:
	mmc_detach_bus(host);
	mmc_release_host(host);

	printk(KERN_ERR "%s: error %d whilst initialising SDIO card\n",
		mmc_hostname(host), err);

	return err;
}

