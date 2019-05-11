/*
 * MTD SPI driver for ST M25Pxx (and similar) serial flash chips
 *
 * Author: Mike Lavender, mike@steroidmicros.com
 *
 * Copyright (c) 2005, Intec Automation Inc.
 *
 * Some parts are based on lart.c by Abraham Van Der Merwe
 *
 * Cleaned up and generalized based on mtd_dataflash.c
 *
 * This code is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/mutex.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>

#include <linux/spi/spi.h>
#include <linux/spi/flash.h>


#define FLASH_PAGESIZE		256

/* Flash opcodes. */
#define	OPCODE_WREN		0x06	/* Write enable */
#define	OPCODE_RDSR		0x05	/* Read status register */
#define	OPCODE_WRSR		0x01	/* Write status register 1 byte */
#define	OPCODE_NORM_READ	0x03	/* Read data bytes (low frequency) */
#define	OPCODE_FAST_READ	0x0b	/* Read data bytes (high frequency) */
#define	OPCODE_PP		0x02	/* Page program (up to 256 bytes) */
#define	OPCODE_BE_4K 		0x20	/* Erase 4KiB block */
#define	OPCODE_BE_32K		0x52	/* Erase 32KiB block */
#define	OPCODE_SE		0xd8	/* Sector erase (usually 64KiB) */
#define	OPCODE_RDID		0x9f	/* Read JEDEC ID */
#define   OPCODE_SFDP           0x5A
#define   DUMMY_DATA             0xFF
#define CONFIG_SUPPORT_SFDP



/* Status Register bits. */
#define	SR_WIP			1	/* Write in progress */
#define	SR_WEL			2	/* Write enable latch */
/* meaning of other SR_* bits may differ between vendors */
#define	SR_BP0			4	/* Block protect 0 */
#define	SR_BP1			8	/* Block protect 1 */
#define	SR_BP2			0x10	/* Block protect 2 */
#define	SR_SRWD			0x80	/* SR write protect */

/* Define max times to check status register before we give up. */
#define	MAX_READY_WAIT_COUNT	10000000
#define	CMD_SIZE		4

#ifdef CONFIG_M25PXX_USE_FAST_READ
#define OPCODE_READ 	OPCODE_FAST_READ
#define FAST_READ_DUMMY_BYTE 1
#else
#define OPCODE_READ 	OPCODE_NORM_READ
#define FAST_READ_DUMMY_BYTE 0
#endif

#ifdef CONFIG_MTD_PARTITIONS
#define	mtd_has_partitions()	(1)
#else
#define	mtd_has_partitions()	(0)
#endif

/****************************************************************************/
/*
 * SPI device driver setup and teardown
 */

struct flash_info {
	char		*name;

	/* JEDEC id zero means "no ID" (most older chips); otherwise it has
	 * a high byte of zero plus three data bytes: the manufacturer id,
	 * then a two byte device id.
	 */
	u32		jedec_id;

	/* The size listed here is what works with OPCODE_SE, which isn't
	 * necessarily called a "sector" by the vendor.
	 */
	unsigned	 sector_size;
        
	u16		n_sectors;
        u16		flags;
        u8	        erase_opcode;
#define	SECT_4K		0x01		/* OPCODE_BE_4K works uniformly */
};

struct m25p {
	struct spi_device	*spi;
	struct mutex		lock;
	struct mtd_info		mtd;
	struct mtd_info		mtd_dummy;
	unsigned		partitioned:1;
	u8			erase_opcode;
	u8			command[CMD_SIZE + FAST_READ_DUMMY_BYTE];
};

#ifdef CONFIG_SUPPORT_SFDP


#define msg_cspew(fmt, args...) printk(KERN_INFO "M25p80:" fmt  , ## args)
#define msg_cdbg(fmt, args...) printk(KERN_INFO "M25p80:" fmt  , ## args)
#define msg_cinfo(fmt, args...) printk(KERN_INFO "M25p80:" fmt  , ## args)
#define msg_cerr(fmt, args...) printk(KERN_INFO "M25p80:" fmt  , ## args)
#define msg_gerr(fmt, args...) printk(KERN_INFO "M25p80:" fmt  , ## args)
#define msg_tst(fmt, args...)    printk(KERN_INFO fmt  , ## args) 

struct sfdp_tbl_hdr {
       uint8_t id;
       uint8_t v_minor;
       uint8_t v_major;
       uint8_t len;
       uint32_t ptp; /* 24b pointer */
};
int spi_sfdp(struct spi_device *spi, uint32_t addr, uint8_t *read_buf, size_t size) {
    int retval;
    uint8_t cmd[5];
    int read_size;
    uint8_t *buf=read_buf;

    /*one time ,read data no longer than 32*/
    do{
        if(size>8)
            read_size=8;
        else
            read_size=size;
        cmd[0] = OPCODE_SFDP;
        cmd[1] = (addr >> 16) & 0xFF;
        cmd[2] = (addr >> 8) & 0xFF;
        cmd[3] = (addr >> 0) & 0xFF;
        cmd[4] = DUMMY_DATA;

        retval = spi_write_then_read(spi, cmd, 5, buf, read_size);
        if (retval < 0) {
            dev_err(&spi->dev, "error %d : read_sfdp_data\n",
                    (int) retval);
            return retval;
        }
        size -= read_size;
        buf += read_size;
        addr += read_size;
    }while(size>0);

    return retval;
}
static int sfdp_fetch_pt(struct spi_device *spi,uint32_t addr, uint8_t *buf, uint16_t len)
{
	uint16_t i;
	if (spi_sfdp(spi,addr, buf, len)) {
		msg_cerr("Receiving SFDP parameter table failed.\n");
		return 1;
	}
	msg_cspew("  Parameter table contents:\n");
	for(i = 0; i < len; i++) {
		if ((i % 8) == 0) {
			msg_tst("    0x%03x: ", i);
		}
		msg_tst("%02x", buf[i]);
		if ((i % 8) == 7) {
			msg_tst("\n");
			continue;
		}
		if ((i % 8) == 3) {
			msg_tst(" ");
			continue;
		}
	}
	msg_tst("\n");
	return 0;
}
#if 0
static int (*get_erasefn_from_opcode(uint8_t opcode)) (struct flashchip *flash, unsigned int blockaddr, unsigned int blocklen)
{
	switch(opcode){
	case 0x00:
	case 0xff:
		/* Not specified, assuming "not supported". */
		return NULL;
	case 0x20:
		return &spi_block_erase_20;
		break;
	case 0x52:
		return &spi_block_erase_52;
		break;
	case 0x60:
		return &spi_block_erase_60;
		break;
	case 0xc7:
		return &spi_block_erase_c7;
		break;
	case 0xd7:
		return &spi_block_erase_d7;
		break;
	case 0xd8:
		return &spi_block_erase_d8;
	default:
		msg_cinfo("%s: unknown opcode (0x%02x) in SFDP table. "
			 "Please report this at "
			 "flashrom at flashrom.org\n",
			 __func__, opcode);
		return NULL;
	}
}
#endif
static int sfdp_fill_flash(struct flash_info*f, uint8_t *buf, uint16_t len)
{
	uint32_t tmp32;
	uint8_t tmp8;
	uint32_t total_size; /* in bytes */
	uint32_t bsize;
	uint8_t opcode_4k = 0xFF;
	int dw,j;

        msg_cspew("Parsing JEDEC SFDP parameter table...\n");
	if (len != 9 * 4) {
		msg_cerr("%s: len out of spec\n", __func__);
		return 1;
	}

	/* 1. double word */
	dw = 0;
	tmp32 = buf[(4 * dw) + 0];
	tmp32 |= ((unsigned int)buf[(4 * dw) + 1]) << 8;
	tmp32 |= ((unsigned int)buf[(4 * dw) + 2]) << 16;
	tmp32 |= ((unsigned int)buf[(4 * dw) + 3]) << 24;
#if 0

	tmp8 = (tmp32 >> 17) & 0x3;
        switch (tmp8) {
	case 0x0:
		msg_cspew("  3-Byte only addressing.\n");
		break;
	case 0x1:
		msg_cspew("  3-Byte (and optionally 4-Byte) addressing.\n");
		break;
	case 0x2:
		msg_cdbg("  4-Byte only addressing not supported.\n");
		return 1;
	default:
		msg_cdbg("  Required addressing mode (0x%x) not supported.\n",
			 tmp8);
		return 1;
	}

	msg_cspew("  Writes to the status register have ");
	if (tmp32 & (1 << 3)) {
		f->unlock = spi_disable_blockprotect;
		msg_cspew("to be enabled with ");
		if (tmp32 & (1 << 4)) {
			f->feature_bits = FEATURE_WRSR_WREN;
			msg_cspew("WREN (0x06).\n");
		} else {
			f->feature_bits = FEATURE_WRSR_EWSR;
			msg_cspew("EWSR (0x50).\n");
		}
	} else
		msg_cspew("not to be especially enabled.\n");
    

	msg_cspew("  Write granularity is ");
	if (tmp32 & (1 << 2)) {
		msg_cspew(" at least 64 B.\n");
		f->page_size = 64;
		f->write = spi_chip_write_256;
	} else {
		msg_cspew(" 1 B only.\n");
		f->page_size = 265; /* ? */
		f->write = spi_chip_write_1;
	}
#endif

	if ((tmp32 & 0x3) == 0x1) {
		opcode_4k = (tmp32 >> 8) & 0xFF; /* will be dealt with later */
               f->sector_size = 4096;
               f->erase_opcode=opcode_4k;
               f->flags=1;
       }

	/* 2. double word */
	dw = 1;
	tmp32 = buf[(4 * dw) + 0];
	tmp32 |= ((unsigned int)buf[(4 * dw) + 1]) << 8;
	tmp32 |= ((unsigned int)buf[(4 * dw) + 2]) << 16;
	tmp32 |= ((unsigned int)buf[(4 * dw) + 3]) << 24;

	if (tmp32 & (1 << 31)) {
		msg_cdbg("  Flash chip size >= 4 Gb/500 MB not supported.\n");
		return 1;
	}
	total_size = ((tmp32+1) & 0x7FFFFFFF) / 8;
        if(opcode_4k != 0xff)
            f->n_sectors = total_size/4096;
       

	dw = 8;
	for(j = 0; j < 4; j++) {
		/* 8 double words from the start + 2 words for every eraser */
		tmp32 = buf[(4 * dw) + (2 * j)];
		if (tmp32 == 0) {
			msg_cspew("  Block eraser %d is unused.\n", j);
			continue;
		}
		if (tmp32 >= 31) {
			msg_cspew("  Block size of eraser %d (2^%d) is too big."
				  "\n", j, tmp32);
			continue;
		}
		bsize = 1 << (tmp32); /* bsize = 2 ^ field */
               tmp8 = buf[(4 * dw) + (2 * j) + 1];
               if(f->sector_size>bsize){
                    f->sector_size=bsize;
                    f->erase_opcode=tmp8;
                    f->n_sectors = total_size/bsize;
               }

               msg_cspew("  Block eraser %d: %d x %d B with opcode 0x%02x\n",
			  j, total_size/bsize, bsize, tmp8);
	}
        msg_cspew("  Flash chip size is %d kB.\n", total_size/1024);
        msg_cspew("  Flash chip sector size is %d \n", f->sector_size);
        msg_cspew("  Flash chip sector num is %d \n", f->n_sectors);
        msg_cspew("  Flash chip 4k sector supoort is %d\n", f->flags);
        msg_cspew("  Flash chip name is %s\n", f->name);
        msg_cspew("  Flash chip jedec id is %x\n", f->jedec_id);
        return 0;
}
                                                              

                                                                

int probe_spi_sfdp(struct flash_info*info,struct spi_device *spi)
{
	int ret = 0;
	uint8_t buf[8];
	uint16_t tmp16;
	uint32_t tmp32;
	uint8_t nph;
	uint8_t i;
	struct sfdp_tbl_hdr *hdrs;
	uint8_t *hbuf;
	uint8_t *tbuf;

        
	if (spi_sfdp(spi,0x0, buf, 4)) {
		msg_cerr("Receiving SFDP signature failed.\n");
		return 0;
	}
	tmp32 = buf[0];
	tmp32 |= ((unsigned int)buf[1]) << 8;
	tmp32 |= ((unsigned int)buf[2]) << 16;
	tmp32 |= ((unsigned int)buf[3]) << 24;

	msg_cspew("SFDP signature = 0x%08x (should be 0x50444653)\n", tmp32);
	if (tmp32 != 0x50444653) {
		msg_cdbg("No SFDP signature found.\n");
		return 0;
	}
	if (spi_sfdp(spi,0x4, buf, 3)) {
		msg_cerr("Receiving SFDP revision and number of parameter "
			 "headers (NPH) failed. ");
		return 0;
	}
	msg_cspew("SFDP revision = %d.%d\n", buf[1], buf[0]);
	nph = buf[2];
	msg_cspew("SFDP number of parameter headers (NPH) = %d (+ 1 mandatory)"
		  "\n", nph);

	/* Fetch all parameter headers, even if we don't use them all (yet). */
	hbuf = kmalloc(sizeof(struct sfdp_tbl_hdr) * (nph + 1),GFP_KERNEL);
	hdrs = kmalloc((nph + 1) * 8,GFP_KERNEL);
	if (hbuf == NULL || hdrs == NULL ) {
		msg_gerr("Out of memory!\n");
		return 0;
	}
	if (spi_sfdp(spi,0x8, hbuf, (nph + 1) * 8)) {
		msg_cerr("Receiving SFDP parameter table headers failed.\n");
		goto cleanup_hdrs;
	}

	i = 0;
	do{
		hdrs[i].id = hbuf[(8 * i) + 0];
		hdrs[i].v_minor = hbuf[(8 * i) + 1];
		hdrs[i].v_major = hbuf[(8 * i) + 2];
		hdrs[i].len = hbuf[(8 * i) + 3];
		hdrs[i].ptp = hbuf[(8 * i) + 4];
		hdrs[i].ptp |= ((unsigned int)hbuf[(8 * i) + 5]) << 8;
		hdrs[i].ptp |= ((unsigned int)hbuf[(8 * i) + 6]) << 16;
		msg_cspew("SFDP parameter table header %d:\n", i);
		msg_cspew("ID 0x%02x, version %d.%d\n", hdrs[i].id,
			  hdrs[i].v_major, hdrs[i].v_minor);
		tmp16 = hdrs[i].len * 4;
		tmp32 = hdrs[i].ptp;
		msg_cspew("Length %d B, Parameter Table Pointer 0x%06x\n",
			  tmp16, tmp32);

		tbuf = kmalloc(tmp16,GFP_KERNEL);
		if (tbuf == NULL) {
			msg_gerr("Out of memory!\n");
			return 0;
		}
               
		if (sfdp_fetch_pt(spi,tmp32, tbuf, tmp16)){
			msg_cerr("Fetching SFDP parameter table %d failed.\n",
				 i);
			kfree(tbuf);
			break;
		}
		if (i == 0) { /* Mandatory JEDEC SFDP parameter table */
			if (hdrs[i].id != 0)
				msg_cdbg("ID of the mandatory JEDEC SFDP "
					 "parameter table is not 0 as demanded "
					 "by JESD216 (warning only).\n");

			if (hdrs[i].len != (9 )) {
				msg_cdbg("Length of the mandatory JEDEC SFDP "
					 "parameter table is not 24 B as "
					 "demanded by JESD216.\n");
				if (hdrs[i].len == (4 ))
					msg_cdbg("It seems like it is the "
						 "preliminary Intel version of "
						 "SFDP, which we don't support."
						 "\n");
			} else if (sfdp_fill_flash(info, tbuf, tmp16) == 0)
				ret = 1;
		}
		
		kfree(tbuf);
		i++;
	} while(i < nph);

cleanup_hdrs:
	kfree(hdrs);
	kfree(hbuf);
	return ret;
}
#endif


static inline struct m25p *mtd_to_m25p(struct mtd_info *mtd)
{
	// container_of(mtd, struct m25p, mtd);

	 return (struct m25p *)mtd->priv;
}

/****************************************************************************/

/*
 * Internal helper functions
 */

/*
 * Read the status register, returning its value in the location
 * Return the status register value.
 * Returns negative if error occurred.
 */
static int read_sr(struct m25p *flash)
{
	ssize_t retval;
	u8 code = OPCODE_RDSR;
	u8 val;

	retval = spi_write_then_read(flash->spi, &code, 1, &val, 1);

	if (retval < 0) {
		dev_err(&flash->spi->dev, "error %d reading SR\n",
				(int) retval);
		return retval;
	}

	return val;
}

/*
 * Write status register 1 byte
 * Returns negative if error occurred.
 */
static int write_sr(struct m25p *flash, u8 val)
{
	flash->command[0] = OPCODE_WRSR;
	flash->command[1] = val;

	return spi_write(flash->spi, flash->command, 2);
}

/*
 * Set write enable latch with Write Enable command.
 * Returns negative if error occurred.
 */
static inline int write_enable(struct m25p *flash)
{
	u8	code = OPCODE_WREN;

	return spi_write_then_read(flash->spi, &code, 1, NULL, 0);
}


/*
 * Service routine to read status register until ready, or timeout occurs.
 * Returns non-zero if error.
 */
static int wait_till_ready(struct m25p *flash)
{
	int count;
	int sr;

	/* one chip guarantees max 5 msec wait here after page writes,
	 * but potentially three seconds (!) after page erase.
	 */
	for (count = 0; count < MAX_READY_WAIT_COUNT; count++) {
		if ((sr = read_sr(flash)) < 0)
			break;
		else if (!(sr & SR_WIP))
			return 0;

		/* REVISIT sometimes sleeping would be best */
	}

        dev_err(&flash->spi->dev, "wait_till_ready error ,the snor is busy,status is: %x \n",sr);
	return 1;
}


/*
 * Erase one sector of flash memory at offset ``offset'' which is any
 * address within the sector which should be erased.
 *
 * Returns 0 if successful, non-zero otherwise.
 */
static int erase_sector(struct m25p *flash, u32 offset)
{
	DEBUG(MTD_DEBUG_LEVEL3, "%s: %s %dKiB at 0x%08x\n",
			flash->spi->dev.bus_id, __func__,
			flash->mtd.erasesize / 1024, offset);

	/* Wait until finished previous write command. */
	if (wait_till_ready(flash))
		return 1;

	/* Send write enable, then erase commands. */
	write_enable(flash);

	/* Set up command buffer. */
	flash->command[0] = flash->erase_opcode;
	flash->command[1] = offset >> 16;
	flash->command[2] = offset >> 8;
	flash->command[3] = offset;

	spi_write(flash->spi, flash->command, CMD_SIZE);

	return 0;
}

/****************************************************************************/

/*
 * MTD implementation
 */

/*
 * Erase an address range on the flash chip.  The address range may extend
 * one or more erase sectors.  Return an error is there is a problem erasing.
 */
static int m25p80_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	struct m25p *flash = mtd_to_m25p(mtd);
	u32 addr,len;

	DEBUG(MTD_DEBUG_LEVEL2, "%s: %s %s 0x%08x, len %d\n",
			flash->spi->dev.bus_id, __func__, "at",
			(u32)instr->addr, instr->len);

	/* sanity checks */
	if (instr->addr + instr->len > flash->mtd.size)
		return -EINVAL;
	if ((instr->addr % mtd->erasesize) != 0
			|| (instr->len % mtd->erasesize) != 0) {
		return -EINVAL;
	}

	addr = instr->addr;
	len = instr->len;

	mutex_lock(&flash->lock);

	/* REVISIT in some cases we could speed up erasing large regions
	 * by using OPCODE_SE instead of OPCODE_BE_4K
	 */

	/* now erase those sectors */
	while (len) {
		if (erase_sector(flash, addr)) {
			instr->state = MTD_ERASE_FAILED;
			mutex_unlock(&flash->lock);
			return -EIO;
		}

		addr += mtd->erasesize;
		len -= mtd->erasesize;
	}

	mutex_unlock(&flash->lock);

	instr->state = MTD_ERASE_DONE;
	mtd_erase_callback(instr);

	return 0;
}

/*
 * Read an address range from the flash chip.  The address range
 * may be any size provided it is within the physical boundaries.
 */
static int m25p80_read(struct mtd_info *mtd, loff_t from, size_t len,
	size_t *retlen, u_char *buf)
{
	struct m25p *flash = mtd_to_m25p(mtd);
	struct spi_transfer t[2];
	struct spi_message m;

	DEBUG(MTD_DEBUG_LEVEL2, "%s: %s %s 0x%08x, len %zd\n",
			flash->spi->dev.bus_id, __func__, "from",
			(u32)from, len);

	/* sanity checks */
	if (!len)
		return 0;

	if (from + len > flash->mtd.size)
		return -EINVAL;

	spi_message_init(&m);
	memset(t, 0, (sizeof t));

	/* NOTE:
	 * OPCODE_FAST_READ (if available) is faster.
	 * Should add 1 byte DUMMY_BYTE.
	 */
	t[0].tx_buf = flash->command;
	t[0].len = CMD_SIZE + FAST_READ_DUMMY_BYTE;
	spi_message_add_tail(&t[0], &m);

	t[1].rx_buf = buf;
	t[1].len = len;
	spi_message_add_tail(&t[1], &m);

	/* Byte count starts at zero. */
	if (retlen)
		*retlen = 0;

	mutex_lock(&flash->lock);

	/* Wait till previous write/erase is done. */
	if (wait_till_ready(flash)) {
		/* REVISIT status return?? */
		mutex_unlock(&flash->lock);
		return 1;
	}

	/* FIXME switch to OPCODE_FAST_READ.  It's required for higher
	 * clocks; and at this writing, every chip this driver handles
	 * supports that opcode.
	 */

	/* Set up the write data buffer. */
	flash->command[0] = OPCODE_READ;
	flash->command[1] = from >> 16;
	flash->command[2] = from >> 8;
	flash->command[3] = from;

	spi_sync(flash->spi, &m);

	*retlen = m.actual_length - CMD_SIZE - FAST_READ_DUMMY_BYTE;

	mutex_unlock(&flash->lock);

	return 0;
}

/*
 * Write an address range to the flash chip.  Data must be written in
 * FLASH_PAGESIZE chunks.  The address range may be any size provided
 * it is within the physical boundaries.
 */
static int m25p80_write(struct mtd_info *mtd, loff_t to, size_t len,
	size_t *retlen, const u_char *buf)
{
	struct m25p *flash = mtd_to_m25p(mtd);
	u32 page_offset, page_size;
	struct spi_transfer t[2];
	struct spi_message m;

	DEBUG(MTD_DEBUG_LEVEL2, "%s: %s %s 0x%08x, len %zd\n",
			flash->spi->dev.bus_id, __func__, "to",
			(u32)to, len);

	if (retlen)
		*retlen = 0;

	/* sanity checks */
	if (!len)
		return(0);

	if (to + len > flash->mtd.size)
		return -EINVAL;

	spi_message_init(&m);
	memset(t, 0, (sizeof t));

	t[0].tx_buf = flash->command;
	t[0].len = CMD_SIZE;
	spi_message_add_tail(&t[0], &m);

	t[1].tx_buf = buf;
	spi_message_add_tail(&t[1], &m);

	mutex_lock(&flash->lock);

	/* Wait until finished previous write command. */
	if (wait_till_ready(flash)) {
		mutex_unlock(&flash->lock);
		return 1;
	}

	write_enable(flash);

	/* Set up the opcode in the write buffer. */
	flash->command[0] = OPCODE_PP;
	flash->command[1] = to >> 16;
	flash->command[2] = to >> 8;
	flash->command[3] = to;

	/* what page do we start with? */
	page_offset = to % FLASH_PAGESIZE;

	/* do all the bytes fit onto one page? */
	if (page_offset + len <= FLASH_PAGESIZE) {
		t[1].len = len;

		spi_sync(flash->spi, &m);

		*retlen = m.actual_length - CMD_SIZE;
	} else {
		u32 i;

		/* the size of data remaining on the first page */
		page_size = FLASH_PAGESIZE - page_offset;

		t[1].len = page_size;
		spi_sync(flash->spi, &m);

		*retlen = m.actual_length - CMD_SIZE;

		/* write everything in PAGESIZE chunks */
		for (i = page_size; i < len; i += page_size) {
			page_size = len - i;
			if (page_size > FLASH_PAGESIZE)
				page_size = FLASH_PAGESIZE;

			/* write the next page to flash */
			flash->command[1] = (to + i) >> 16;
			flash->command[2] = (to + i) >> 8;
			flash->command[3] = (to + i);

			t[1].tx_buf = buf + i;
			t[1].len = page_size;

			wait_till_ready(flash);

			write_enable(flash);

			spi_sync(flash->spi, &m);

			if (retlen)
				*retlen += m.actual_length - CMD_SIZE;
		}
	}

	mutex_unlock(&flash->lock);

	return 0;
}


/****************************************************************************/




/* NOTE: double check command sets and memory organization when you add
 * more flash chips.  This current list focusses on newer chips, which
 * have been converging on command sets which including JEDEC ID.
 */
static struct flash_info __devinitdata m25p_data [] = {

	/* Atmel -- some are (confusingly) marketed as "DataFlash" */
	{ "at25fs010",  0x1f6601, 32 * 1024, 4, SECT_4K, },
	{ "at25fs040",  0x1f6604, 64 * 1024, 8, SECT_4K, },

	{ "at25df041a", 0x1f4401, 64 * 1024, 8, SECT_4K, },
	{ "at25df641",  0x1f4800, 64 * 1024, 128, SECT_4K, },

	{ "at26f004",   0x1f0400, 64 * 1024, 8, SECT_4K, },
	{ "at26df081a", 0x1f4501, 64 * 1024, 16, SECT_4K, },
	{ "at26df161a", 0x1f4601, 64 * 1024, 32, SECT_4K, },
	{ "at26df321",  0x1f4701, 64 * 1024, 64, SECT_4K, },

	/* Spansion -- single (large) sector size only, at least
	 * for the chips listed here (without boot sectors).
	 */
	{ "s25sl004a", 0x010212, 64 * 1024, 8, },
	{ "s25sl008a", 0x010213, 64 * 1024, 16, },
	{ "s25sl016a", 0x010214, 64 * 1024, 32, },
	{ "s25sl032a", 0x010215, 64 * 1024, 64, },
	{ "s25sl064a", 0x010216, 64 * 1024, 128, },

	/* SST -- large erase sizes are "overlays", "sectors" are 4K */
	{ "sst25vf040b", 0xbf258d, 64 * 1024, 8, SECT_4K, },
	{ "sst25vf080b", 0xbf258e, 64 * 1024, 16, SECT_4K, },
	{ "sst25vf016b", 0xbf2541, 64 * 1024, 32, SECT_4K, },
	{ "sst25vf032b", 0xbf254a, 64 * 1024, 64, SECT_4K, },

	/* ST Microelectronics -- newer production may have feature updates */
	{ "m25p05",  0x202010,  32 * 1024, 2, },
	{ "m25p10",  0x202011,  32 * 1024, 4, },
	{ "m25p20",  0x202012,  64 * 1024, 4, },
	{ "m25p40",  0x202013,  64 * 1024, 8, },
	{ "m25p80",         0,  64 * 1024, 16, },
	{ "m25p16",  0x202015,  64 * 1024, 32, },
	{ "m25p32",  0x202016,  64 * 1024, 64, },
	{ "m25p64",  0x202017,  64 * 1024, 128, },
	{ "m25p128", 0x202018, 256 * 1024, 64, },

	{ "m45pe80", 0x204014,  64 * 1024, 16, },
	{ "m45pe16", 0x204015,  64 * 1024, 32, },

	{ "m25pe80", 0x208014,  64 * 1024, 16, },
	{ "m25pe16", 0x208015,  64 * 1024, 32, SECT_4K, },

	/* Winbond -- w25x "blocks" are 64K, "sectors" are 4KiB */
	{ "w25x10", 0xef3011, 64 * 1024, 2, SECT_4K, },
	{ "w25x20", 0xef3012, 64 * 1024, 4, SECT_4K, },
	{ "w25x40", 0xef3013, 64 * 1024, 8, SECT_4K, },
	{ "w25x80", 0xef3014, 64 * 1024, 16, SECT_4K, },
	{ "w25x16", 0xef3015, 64 * 1024, 32, SECT_4K, },
	{ "w25x32", 0xef3016, 64 * 1024, 64, SECT_4K, },
	{ "w25x64", 0xef3017, 64 * 1024, 128, SECT_4K, },
        { "w25q32fv", 0xef4016, 4096, 1024, SECT_4K, },
        { "w25q32jv", 0xef7016, 4096, 1024, SECT_4K, },
        { "w25q128fw", 0xef6018, 4096, 4096, SECT_4K, },


	/*GigaDevice -- */
	{"ZB25VQ32 ",0x5E4016,4096, 1024, SECT_4K},
	{"ZB25VQ128 ",0x5E4018,4096, 4096, SECT_4K},
	{"GD25Q128C",0xC84018,4096, 4096, SECT_4K},
	{"GD25LQ128",0xC86018,4096, 4096, SECT_4K},
        {"GX25Q128C",0xd84018,4096, 4096, SECT_4K},
        {"BY25Q128AS",0x684018,4096, 4096, SECT_4K},
        {"LR25Q32S",0x684016,4096, 1024, SECT_4K},
        {"GD25Q32C ",0xC84016,4096, 1024, SECT_4K},
	{"GD25Q128C",0xEF4018,4096, 4096, SECT_4K},
	{"GD25Q64C",0xC84017,4096, 2048, SECT_4K},
	{"GD25Q256C",0xC84019,4096,8192,SECT_4K},
	{"PN25F64B",0x1C7017,4096, 2048, SECT_4K},
        {"PN25F128S",0x1C7018,4096, 4096, SECT_4K},
        {"EN25Q32C ",0x1C3016,4096, 1024, SECT_4K},
	{"W25Q64FV",0xEF4017,4096, 2048, SECT_4K},
	{"MX25L3206E",0xC22016,4096, 1024, SECT_4K},
	{"KH25L6406E",0xC22017,4096, 2048, SECT_4K},
         {"KH25L12835F",0xC22018,4096, 4096, SECT_4K},
         {"ZB25Q64",0x1C7017,4096, 2048, SECT_4K},
          {"XT25F128A",0x207018,4096, 4096, SECT_4K},
          {"XM25QH64A",0x207017,4096, 2048, SECT_4K},
          {"XM25QH32B",0x204016,4096, 1024, SECT_4K},
          {"PN25F32B",0x1C3116,4096, 1024, SECT_4K},
          {"IS25CQ032",0x7f9d46,4096, 1024, SECT_4K},
          {"FT25H32",0x0e4016,4096, 1024, SECT_4K},
          {"S25FL128P",0x012018,65536, 256, 0},
          {"JESD216B",0xFFFFFF,0xFFFFFFFF, 0xFF, 0},
};

static struct flash_info *__devinit jedec_probe(struct spi_device *spi)
{
	int			tmp;
	u8			code = OPCODE_RDID;
	u8			id[3];
	u32			jedec;
	struct flash_info	*info;
        int ret;

	/* JEDEC also defines an optional "extended device information"
	 * string for after vendor-specific data, after the three bytes
	 * we use here.  Supporting some chips might require using it.
	 */
	tmp = spi_write_then_read(spi, &code, 1, id, 3);
	if (tmp < 0) {
		DEBUG(MTD_DEBUG_LEVEL0, "%s: error %d reading JEDEC ID\n",
			spi->dev.bus_id, tmp);
		return NULL;
	}
	jedec = id[0];
	jedec = jedec << 8;
	jedec |= id[1];
	jedec = jedec << 8;
	jedec |= id[2];

	for (tmp = 0, info = m25p_data;
			tmp < ARRAY_SIZE(m25p_data);
			tmp++, info++) {
		if (info->jedec_id == jedec)
			return info;
               if(strcmp(info->name,"JESD216B")==0)
                    break;
	}

#ifdef CONFIG_SUPPORT_SFDP
        info->jedec_id=jedec;
        ret = probe_spi_sfdp(info,spi);
        if(ret){
            return info;
        }else{
            dev_err(&spi->dev, "probe_spi_sfdp error, JEDEC id %06x\n", jedec);
        }
#endif
	dev_err(&spi->dev, "unrecognized JEDEC id %06x\n", jedec);
	return NULL;
}


/*
 * board specific setup should have ensured the SPI clock used here
 * matches what the READ command supports, at least until this driver
 * understands FAST_READ (for clocks over 25 MHz).
 */
static int __devinit m25p_probe(struct spi_device *spi)
{
	struct flash_platform_data	*data;
	struct m25p			*flash;
	struct flash_info		*info;
	unsigned			i;

	/* Platform data helps sort out which chip type we have, as
	 * well as how this board partitions it.  If we don't have
	 * a chip ID, try the JEDEC id commands; they'll work for most
	 * newer chips, even if we don't recognize the particular chip.
	 */
	data = spi->dev.platform_data;
	if (data && data->type) {
		for (i = 0, info = m25p_data;
				i < ARRAY_SIZE(m25p_data);
				i++, info++) {
			if (strcmp(data->type, info->name) == 0)
				break;
		}

		/* unrecognized chip? */
		if (i == ARRAY_SIZE(m25p_data)) {
			DEBUG(MTD_DEBUG_LEVEL0, "%s: unrecognized id %s\n",
					spi->dev.bus_id, data->type);
			info = NULL;

		/* recognized; is that chip really what's there? */
		} else if (info->jedec_id) {
			struct flash_info	*chip = jedec_probe(spi);

			if (!chip || chip != info) {
				dev_warn(&spi->dev, "found %s, expected %s\n",
						chip ? chip->name : "UNKNOWN",
						info->name);
				info = NULL;
			}
		}
	} else
		info = jedec_probe(spi);

	if (!info)
		return -ENODEV;

	flash = kzalloc(sizeof *flash, GFP_KERNEL);
	if (!flash)
		return -ENOMEM;

	flash->spi = spi;
	mutex_init(&flash->lock);
	dev_set_drvdata(&spi->dev, flash);

	/*
	 * Atmel serial flash tend to power up
	 * with the software protection bits set
	 */

	if (info->jedec_id >> 16 == 0x1f) {
		write_enable(flash);
		write_sr(flash, 0);
	}

	if (data && data->name)
		flash->mtd.name = data->name;
	else
		flash->mtd.name = spi->dev.bus_id;

	flash->mtd.type = MTD_NORFLASH;
	flash->mtd.writesize = 1;
	flash->mtd.flags = MTD_CAP_NORFLASH;
	flash->mtd.size = info->sector_size * info->n_sectors;
	flash->mtd.erase = m25p80_erase;
	flash->mtd.read = m25p80_read;
	flash->mtd.write = m25p80_write;

	/* prefer "small sector" erase if possible */
        if(strcmp(info->name, "JESD216B") == 0){
            flash->erase_opcode = info->erase_opcode;
        }else{
            if (info->flags & SECT_4K) {
                flash->erase_opcode = OPCODE_BE_4K;
            } else {
                flash->erase_opcode = OPCODE_SE;
            }
        }
        flash->mtd.erasesize = info->sector_size;
    
        dev_info(&spi->dev, "%s (%d Kbytes) sector_size:%d opcode:%x %d\n", info->name,
			flash->mtd.size /1024,flash->mtd.erasesize,flash->erase_opcode,sizeof(info->sector_size));

	DEBUG(MTD_DEBUG_LEVEL2,
		"mtd .name = %s, .size = 0x%.8x (%uMiB) "
			".erasesize = 0x%.8x (%uKiB) .numeraseregions = %d\n",
		flash->mtd.name,
		flash->mtd.size, flash->mtd.size / (1024*1024),
		flash->mtd.erasesize, flash->mtd.erasesize / 1024,
		flash->mtd.numeraseregions);

	if (flash->mtd.numeraseregions)
		for (i = 0; i < flash->mtd.numeraseregions; i++)
			DEBUG(MTD_DEBUG_LEVEL2,
				"mtd.eraseregions[%d] = { .offset = 0x%.8x, "
				".erasesize = 0x%.8x (%uKiB), "
				".numblocks = %d }\n",
				i, flash->mtd.eraseregions[i].offset,
				flash->mtd.eraseregions[i].erasesize,
				flash->mtd.eraseregions[i].erasesize / 1024,
				flash->mtd.eraseregions[i].numblocks);


	/* partitions should match sector boundaries; and it may be good to
	 * use readonly partitions for writeprotected sectors (BP2..BP0).
	 */
	if (mtd_has_partitions()) {
		struct mtd_partition	*parts = NULL;
		int			nr_parts = 0;

#ifdef CONFIG_MTD_CMDLINE_PARTS
		static const char *part_probes[] = { "cmdlinepart", NULL, };

		nr_parts = parse_mtd_partitions(&flash->mtd,
				part_probes, &parts, 0);
#endif

		if (nr_parts <= 0 && data && data->parts) {
			parts = data->parts;
			nr_parts = data->nr_parts;
		}

		if (nr_parts > 0) {
			for (i = 0; i < nr_parts; i++) {
				DEBUG(MTD_DEBUG_LEVEL2, "partitions[%d] = "
					"{.name = %s, .offset = 0x%.8x, "
						".size = 0x%.8x (%uKiB) }\n",
					i, parts[i].name,
					parts[i].offset,
					parts[i].size,
					parts[i].size / 1024);
			}
			flash->partitioned = 1;
			return add_mtd_partitions(&flash->mtd, parts, nr_parts);
		}
	} else if (data->nr_parts)
		dev_warn(&spi->dev, "ignoring %d default partitions on %s\n",
				data->nr_parts, data->name);

	flash->mtd.priv = flash;
		memcpy(&flash->mtd_dummy,&flash->mtd,sizeof(flash->mtd));
	if(add_mtd_device(&flash->mtd))
		return -ENODEV;
	return add_mtd_device(&flash->mtd_dummy) == 1 ? -ENODEV : 0;
}


static int __devexit m25p_remove(struct spi_device *spi)
{
	struct m25p	*flash = dev_get_drvdata(&spi->dev);
	int		status;

	/* Clean up MTD stuff. */
	if (mtd_has_partitions() && flash->partitioned)
		status = del_mtd_partitions(&flash->mtd);
	else
		status = del_mtd_device(&flash->mtd);
	if (status == 0)
		kfree(flash);
	return 0;
}


static struct spi_driver m25p80_driver = {
	.driver = {
		.name	= "m25p80",
		.bus	= &spi_bus_type,
		.owner	= THIS_MODULE,
	},
	.probe	= m25p_probe,
	.remove	= __devexit_p(m25p_remove),

	/* REVISIT: many of these chips have deep power-down modes, which
	 * should clearly be entered on suspend() to minimize power use.
	 * And also when they're otherwise idle...
	 */
};


static int m25p80_init(void)
{
	return spi_register_driver(&m25p80_driver);
}


static void m25p80_exit(void)
{
	spi_unregister_driver(&m25p80_driver);
}


module_init(m25p80_init);
module_exit(m25p80_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mike Lavender");
MODULE_DESCRIPTION("MTD SPI driver for ST M25Pxx flash chips");
