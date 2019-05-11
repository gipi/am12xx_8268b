/*
 * fs/partitions/amfs.c
 *
 * Copyright (C) 2010 Actions MicroEletronics Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Support the native `MBR' on Actions-MicroEletronics NandFlash system,
 * which maximum contains 16 primary partitions.
 *
 * Changelog:
 *
 *  2010/3/22: Pan Ruochen <reachpan@actions-micro.com>
 *				- The first available version
 */

#include <linux/msdos_fs.h>

#include "check.h"
#include "msdos.h"
#include "efi.h"

#if 1
#define DMSG(format,args...)  printk(KERN_INFO format,##args)
#else
#define DMSG(format,args...)  do {} while(0)
#endif

static inline int
amflash_magic_present(unsigned char *p)
{
	return (p[0] == 0x4c && p[1] == 0x59);
}

#include <asm/unaligned.h>
#define SYS_IND(p)	(get_unaligned(&p->sys_ind))
#define NR_SECTS(p)	({ __le32 __a =	get_unaligned(&p->nr_sects);	\
				le32_to_cpu(__a); \
			})

#define START_SECT(p)	({ __le32 __a =	get_unaligned(&p->start_sect);	\
				le32_to_cpu(__a); \
			})


struct priv_ptr {
    uint8_t  system_id;
    char     id[6];
    uint8_t  status;
    uint32_t part_off;   /* LBA of first sector in the partition */
    uint32_t part_size;  /* number of blocks in partition */
} __attribute__((packed));

#define MAX_PRIV_PARTS    16

struct priv_mbr {
	uint8_t          data[254];
	struct priv_ptr  parts[MAX_PRIV_PARTS];
	uint16_t         signature;
};

int amflash_partition(struct parsed_partitions *state, struct block_device *bdev)
{
	int sector_size = bdev_hardsect_size(bdev) / 512;
	Sector sect;
	unsigned char *data;
	int slot;

	DMSG("Enter My PARTITION PARSER\n");
	data = read_dev_sector(bdev, 0, &sect);
	if (!data)
		return -1;
	if (!amflash_magic_present(data + 510)) {
		put_dev_sector(sect);
		return 0;
	}

	DMSG("sector_size=%d\n", sector_size);
	struct priv_ptr *p = ((struct priv_mbr *)data)->parts;
	state->next = 1;
	for (slot = 1 ; slot <= MAX_PRIV_PARTS ; slot++, p++) {
		u32 start = get_unaligned(&p->part_off)*sector_size;
		u32 size  = get_unaligned(&p->part_size)*sector_size;
		DMSG("start: 0x%08x, size:0x%08x\n", start,size);
		if (!size)
			break;
		put_partition(state, slot, start, size);
		state->next++;
	}
	DMSG("\n");
	put_dev_sector(sect);
	return 1;
}


