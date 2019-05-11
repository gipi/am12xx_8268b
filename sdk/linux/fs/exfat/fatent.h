/*
 * Copyright (C) 2008, OGAWA Hirofumi
 * Released under GPL v2.
 */

#ifndef _EXFAT_FATENT_H
#define _EXFAT_FATENT_H

#include <linux/buffer_head.h>
#include "debug.h"

struct exfat_ent {
	struct buffer_head *bh;
	u32 offset;
};

static inline void exfat_ent_init(struct exfat_ent *fatent)
{
	fatent->bh = NULL;
}

static inline void exfat_ent_release(struct exfat_ent *fatent)
{
	brelse(fatent->bh);
}

/* FIXME: should be outline those funcs? must rethink those */
//读物理簇clustnr在fat表中指向的簇号
static inline int exfat_ent_read(struct inode *inode,
				 struct exfat_ent *fatent,
				 u32 clusnr, u32 *result_clusnr)
{
	struct super_block *sb = inode->i_sb;
	struct exfat_sb_info *sbi = EXFAT_SB(sb);
	sector_t blocknr;
	__le32 *ent;

	blocknr = sbi->fat_blocknr + (clusnr >> sbi->fpb_bits);
	if(fatent->bh){
		if(fatent->bh->b_blocknr != blocknr){
			brelse(fatent->bh);
			goto next;
		}
	}else{
next:
		fatent->bh = sb_bread(sb, blocknr);
		if (!fatent->bh)
			return -EIO;
	}		
	
	fatent->offset = clusnr & (sbi->fpb - 1);
	exfat_debug("blocknr:0x%llx, fatent->offset : 0x%x",blocknr,fatent->offset );
	ent = (__le32 *)fatent->bh->b_data;
	*result_clusnr = le32_to_cpu(ent[fatent->offset]);

	return 0;
}

static inline int exfat_ent_prepare(struct inode *inode,
				    struct exfat_ent *fatent,
				    u32 clusnr, u32 *result_clusnr)
{
	struct super_block *sb = inode->i_sb;
	struct exfat_sb_info *sbi = EXFAT_SB(sb);
	__le32 *ent;

	fatent->bh = sb_bread(sb, sbi->fat_blocknr + (clusnr >> sbi->fpb_bits));
	if (!fatent->bh)
		return -EIO;

	fatent->offset = clusnr & (sbi->fpb - 1);
	ent = (__le32 *)fatent->bh->b_data;
	*result_clusnr = le32_to_cpu(ent[fatent->offset]);

	return 0;
}
#ifdef EXFAT_MODIFY

//更新fat表: 将原fat表项用new_clusnr填充
static inline int exfat_ent_write(struct inode *inode,
				   struct exfat_ent *fatent,
				   u32 new_clusnr)
{
	__le32 *ent = (__le32 *)fatent->bh->b_data;
	//struct super_block *sb = inode->i_sb;
	int err = 0;

	ent[fatent->offset] = cpu_to_le32(new_clusnr);

	mark_buffer_dirty(fatent->bh);		
	err = sync_dirty_buffer(fatent->bh);
	return err;
}
#endif

#endif /* !_EXFAT_FATENT_H */
