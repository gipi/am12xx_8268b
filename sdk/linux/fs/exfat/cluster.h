/*
 * Copyright (C) 2008, OGAWA Hirofumi
 * Released under GPL v2.
 */

#ifndef _EXFAT_CLUSTER_H
#define _EXFAT_CLUSTER_H

/*用于获取文件簇*/
struct exfat_clus_map {
	u32	iclusnr;		/* cluster number on data of inode. */
	u32	clusnr;			/* cluster number on disk. */
	u32	len;			/* number of contiguous clusters 当前连续的簇数*/
};

extern int exfat_get_cluster(struct inode *inode, u32 iclusnr, u32 clus_len,
		      struct exfat_clus_map *cmap);

#ifdef EXFAT_MODIFY
extern int exfat_alloc_clusters(struct inode *inode, sector_t  *cluster, int nr_cluster);
extern int exfat_chain_add(struct inode *inode, int new_dclus, int nr_cluster);
extern int exfat_free_clusters(struct inode *inode, u32 num,sector_t dcluster);
extern int exfat_get_last_cluster(struct inode *inode,u32 *last_iclus,u32 *last_dclus);

#endif

#endif /* !_EXFAT_CLUSTER_H */
