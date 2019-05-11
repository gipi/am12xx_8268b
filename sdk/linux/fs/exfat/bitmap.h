/*
 * Copyright (C) 2008, OGAWA Hirofumi
 * Released under GPL v2.
 */

#ifndef _EXFAT_BITMAP_H
#define _EXFAT_BITMAP_H

int exfat_setup_bitmap(struct super_block *sb, u32 clusnr, u64 i_size);
void exfat_free_bitmap(struct exfat_sb_info *sbi);
//int find_bit_and_set(uint8_t* bitmap, int start,int end);

#ifdef EXFAT_MODIFY
#define BMAP_GET(bitmap, index) ((bitmap)[(index) / 8] & (1u << ((index) % 8)))
#define BMAP_SET(bitmap, index) (bitmap)[(index) / 8] |= (1u << ((index) % 8))
#define BMAP_CLR(bitmap, index) (bitmap)[(index) / 8] &= ~(1u << ((index) % 8))


extern  unsigned long find_bit_and_set(uint8_t* bitmap, unsigned long start,unsigned long end);
#endif
#endif /* !_EXFAT_BITMAP_H */
