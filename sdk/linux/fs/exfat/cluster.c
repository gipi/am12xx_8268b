/*
 * Copyright (C) 2008, OGAWA Hirofumi
 * Released under GPL v2.
 */

#include "exfat.h"
#include "cache.h"
#include "fatent.h"
#include "cluster.h"
#include "debug.h"
#include "bitmap.h"

#ifdef EXFAT_MODIFY
/*
int exfat_get_next_cluster(struct inode *inode,u32 prev_iclus,u32 next_dclus)
{
	int err = 0;
	struct exfat_clus_map cmap;
	struct exfat_inode_info *exi = EXFAT_I(inode);

	DEBUG0_ON(!exi->clusnr);

	err = exfat_get_cluster(inode,prev_iclus+1,1,&cmap);
	if(err)
		return err;
	next_dclus  = cmp->clusnr;
	return 0;
}
*/
/*得到inode的末尾物理簇号*/
int exfat_get_last_cluster(struct inode *inode,u32 *last_iclus,u32 *last_dclus)
{
	struct super_block *sb = inode->i_sb;
	struct exfat_sb_info *sbi = EXFAT_SB(sb);
	struct exfat_inode_info *exi = EXFAT_I(inode);
/*	unsigned long blocksize;
	sector_t  last_iblock;
	u32 last_iclusnr;
	*/
	struct exfat_clus_map cmap;
	int err =0;
	
	*last_dclus = 0;
	*last_iclus = 0;
	DEBUG0_ON(!exi->clusnr);
#if 0
	blocksize = 1 << inode->i_blkbits;;
	last_iblock = (i_size_read(inode) + (blocksize - 1))//该节点的最大block数
		>> inode->i_blkbits;
	last_iclusnr = (last_iblock + (sbi->bpc - 1)) >> sbi->bpc_bits;//该节点的最大簇数
	err = exfat_get_cluster(inode,last_iclusnr,1,&cmap);
	if(err)
		return err;
	if (!exfat_ent_eof(sbi, cmap.clusnr)) {
		exfat_error( "exFAT: found invalid FAT entry 0x%08x"
		       " for root directory", cmap.clusnr);
		err = -EIO;
		return err;
	}
	*last_dclus = cmap.clusnr;
	*last_iclus = last_iclusnr;
#endif
	/* Get last iclusnr and dlusnr*/
	if(exi->data_flag == EXFAT_DATA_CONTIGUOUS)
	{
		exfat_debug("exi->phys_size: 0x%llx",exi->phys_size);
		*last_iclus =exfat_div_roundup(exi->phys_size,sbi->clus_size);
		*last_dclus = exi->clusnr + *last_iclus-1;
	}
	else{		
		err = exfat_get_cluster(inode, EXFAT_ENT_EOF, 0, &cmap);
		if (err)
			return err;
		if (!exfat_ent_eof(sbi, cmap.clusnr)) {
			exfat_error( "exFAT: found invalid FAT entry 0x%08x"
			      , cmap.clusnr);
			err = -EIO;
			return err;
		}

		*last_dclus = cmap.clusnr;
		*last_iclus =cmap.iclusnr;
	}
	return 0;
	
}
#endif
static void calc_cmap(struct exfat_clus_map *cmap, struct exfat_cache_id *cid,
		      u32 iclusnr)
{
	u32 offset = iclusnr - cid->iclusnr;
	cmap->iclusnr = iclusnr;
	cmap->clusnr = cid->clusnr + offset;
	cmap->len = cid->len - offset;
}

/*
 * Caller must be guarantee it have no race with truncate().  I.e. the
 * range of "iclusnr" and "iclusnr + len" must not be truncated.
 */
int exfat_get_cluster(struct inode *inode, u32 iclusnr, u32 clus_len,
		      struct exfat_clus_map *cmap)
{
	struct super_block *sb = inode->i_sb;
	struct exfat_sb_info *sbi = EXFAT_SB(sb);
	struct exfat_inode_info *exi = EXFAT_I(inode);
	struct exfat_cache_id cid;
	struct exfat_ent fatent;
	u32 clusnr;
	int err = 0, found_target;

	DEBUG0_ON(!exi->clusnr);

	if (exi->data_flag == EXFAT_DATA_CONTIGUOUS) {//数据是连续的
		/*DEBUG_ON(clus_len >
		  (i_size_read(inode) + (sbi->clus_size - 1)) >> sbi->clus_bits,
			 "clus_len %u, i_size %lld, clus_size %lu\n",
			 clus_len, i_size_read(inode), sbi->clus_size);
			 */
		/* FIXME: should check clus_len limit from i_size? */
		cmap->iclusnr = iclusnr;
		cmap->clusnr = exi->clusnr + iclusnr;//物理簇号
		cmap->len = clus_len;
		//exfat_debug("contig: iclusnr %u, clusnr %u, len %u", 
		//       cmap->iclusnr, cmap->clusnr, cmap->len);
		return 0;
	}

	/*
	 * FIXME: use rw_semaphore (r: here, w: truncate), then we can
	 * do readahead of contiguous clusters and cache it. (This
	 * should optimize disk-seek very much if clusters is not
	 * fragmented heavily)
	 */

	/* Setup the start cluster to walk */
	if (!exfat_cache_lookup(inode, iclusnr, &cid))
		cache_init(&cid, 0, exi->clusnr);//如果没找到iclusnr对应的cache，用物理起始簇号来初始化cid
	clusnr = cid.clusnr + cid.len - 1;//与iclusnr最近的物理簇号

	/* Walk the cluster-chain */
	found_target = 0;
	exfat_ent_init(&fatent);
	/*exfat_debug("iclusnr %u, clus_len %u: cid.iclusnr %u, cid.clusnr %u,"
	       " cid.len %u, search start clusnr %u",
	        iclusnr, clus_len, cid.iclusnr, cid.clusnr, cid.len,
	       clusnr);
	       */
	while (cid.iclusnr + cid.len < iclusnr + clus_len) {
		/* Target iclusnr was found, so find contiguous clusters */
		if (!found_target && (iclusnr < cid.iclusnr + cid.len))
			found_target = 1;//找到起点簇号

		/* FIXME: should be readahead FAT blocks */
#if 0
		err = exfat_ent_read(inode, &fatent, clusnr, &clusnr);
#else
		{
			u32 prev = clusnr;
			err = exfat_ent_read(inode, &fatent, prev, &clusnr);//读FAT表获取下一簇号
			exfat_debug("ent_read: clusnr ox%x, next clusnr 0x%x",
			        prev, clusnr);
		}
#endif
		if (err < 0)
			goto out;

		if (exfat_ent_eof(sbi, clusnr)) {//是否为结束:0xffffffff
			exfat_cache_add(inode, &cid);//将cid添加到inode的cache中去

			/* Return special cmap */
			cmap->iclusnr = cid.iclusnr + cid.len;
			cmap->clusnr = clusnr;
			cmap->len = 0;//不连续
			goto out;
		} else if (exfat_ent_bad(sbi, clusnr)) {//坏簇
			exfat_error(KERN_ERR "exFAT: found bad cluster entry"
			       " (start cluster 0x%08x)\n", exi->clusnr);
			err = -EIO;
			goto out;
		} else if (!exfat_ent_valid(sbi, clusnr)) {//无效
			exfat_fs_panic(sb, "found invalid cluster chain"
				       " (start cluster 0x%08x, entry 0x%08x)",
				       exi->clusnr, clusnr);
			err = -EIO;
			goto out;
		}

		if (cache_contiguous(&cid, clusnr))//判断是否连续?
			cid.len++;
		else {
			if (found_target) {
				/* Found a discontiguous cluster */
				calc_cmap(cmap, &cid, iclusnr);
				exfat_cache_add(inode, &cid);

				cache_init(&cid, cid.iclusnr + cid.len, clusnr);
				exfat_cache_add(inode, &cid);
				goto out;
			}
			cache_init(&cid, cid.iclusnr + cid.len, clusnr);
		}

		/* Prevent the infinite loop of cluster chain */
		if (cid.iclusnr + cid.len > sbi->total_clusters) {
			exfat_fs_panic(sb, "detected the cluster chain loop"
				       " (start cluster 0x%08x)", exi->clusnr);
			err = -EIO;
			goto out;
		}
	}

	calc_cmap(cmap, &cid, iclusnr);
	exfat_cache_add(inode, &cid);
out:
	exfat_ent_release(&fatent);
	//exfat_debug("result cmap: iclusnr %u, clusnr %u, len %u\n",
	//       cmap->iclusnr, cmap->clusnr, cmap->len);

	return err;
}

#ifdef EXFAT_MODIFY
/*erase a  phsic cluster*/

static int add_FatTable(struct inode *inode,struct exfat_ent *exfat_ent,unsigned long num, u32 start_clus)
{
	int ret = 0;
	unsigned int i ;
	u32 last_dclus;
	u32 dclus = start_clus;

	exfat_debug("entering");
	exfat_ent_init(exfat_ent);
	for(i=0;i<num;i++)
	{
		ret = exfat_ent_read(inode, exfat_ent,dclus , &last_dclus);
		if (ret >= 0) {			
			exfat_debug("last_dclus :0x%x",last_dclus);
			ret = exfat_ent_write(inode, exfat_ent, dclus+1);			
		}
		dclus++;
	}
	exfat_ent_release(exfat_ent);
	return 0;
}
static int erase_cluster(struct super_block *sb, sector_t cluster)
{
	struct buffer_head* bh;
	sector_t blocknr;
	int i = 0,err;
	struct exfat_sb_info *sbi = EXFAT_SB(sb);

	exfat_debug("Entering");

	blocknr = exfat_clus_to_blknr(sbi, cluster);
	for(i=0;i<sbi->bpc;i++){
		bh = sb_bread(sb, blocknr);
		if (!bh) {
			exfat_error("unable to read inode block "
			       "for updating (blocknr 0x%llx)\n", blocknr);
			return -EIO;
		}
		memset(bh->b_data,0,bh->b_size);
		mark_buffer_dirty(bh);			
		err = sync_dirty_buffer(bh);		
		brelse(bh);
		blocknr++;
	}
	exfat_debug("Done");
	return 0;	
}
/*actually alloc one cluster*/
int exfat_alloc_clusters(struct inode *inode, sector_t  *cluster, int nr_cluster)
{
	struct super_block *sb = inode->i_sb;
	struct exfat_sb_info *sbi = EXFAT_SB(sb);
	struct exfat_inode_info *exi = EXFAT_I(inode);
	struct inode * bitmap_inode = sbi->bitmap_inode;
	pgoff_t index, end;
	struct address_space *mapping;
	struct page *page;
	u8 *kaddr;
	int err = 0;
	sector_t dec = 0;
	loff_t left_size ;
	
	exfat_debug("Entering");
	//空簇不够
	if (sbi->free_clusters < nr_cluster) {		
		err =  -ENOSPC;
		exfat_error("no enough space,insert another card");
		goto err_out;
	}

/*	cluster -= EXFAT_START_ENT;
	if (cluster >= bitmap_inode->i_size){
		exfat_error("cluster[%d] > bitmp_size[%d] ",cluster, bitmap_inode->i_size);
		cluster = 0;
	}
*/	
	mapping = bitmap_inode->i_mapping;
	for(index=0;index < exfat_div_roundup(bitmap_inode->i_size,PAGE_SIZE); index++)
	{	
		/* Get the page containing the first bit (@start_bit). */
		page = exfat_map_page(mapping, index);
		if (IS_ERR(page)) {
			exfat_error("Failed to map %ld page (error "
						"%li), aborting.",index, PTR_ERR(page));
			err =  PTR_ERR(page);
			goto err_out;
		}
		kaddr = page_address(page);

		left_size  = (index+1)*PAGE_SIZE - bitmap_inode->i_size;
		if(left_size > 0){
			exfat_debug("reach the last bitmap page");
			end  = (bitmap_inode->i_size-index*PAGE_SIZE)*8;
		}else			
			end = PAGE_SIZE*8;

		dec = find_bit_and_set((uint8_t *)kaddr,0, end);
		if (dec == EXFAT_ENT_EOF)
			exfat_unmap_page(page);
		else{
			sbi->free_clusters--; /* update percentage of used space */	
			flush_dcache_page(page);
			set_page_dirty(page);
			lock_page(page);
			write_one_page(page,1);
			exfat_unmap_page(page);

			exfat_debug("index:0x%lx,dec:0x%llx",index,dec);

			dec += index*PAGE_SIZE*8;
			exfat_debug("Done, allocated cluster : 0x%llx",dec);
			cluster[0] = dec;
			if(exi->attrib & EXFAT_ATTR_DIR)
				erase_cluster(sb, dec);
			return 0;
		}
	}	
	err =  -ENOSPC;
	exfat_error("not free space");
	exfat_debug("bitmap_inode->i_size:0x%llx",bitmap_inode->i_size);
	exfat_debug("index:0x%lx",index);

err_out:
	exfat_debug("Failed");
	return err;
	
}

/*
 * exfat_chain_add() adds a new cluster to the chain of clusters represented
 * by inode.
 * 将新簇添加到inode的簇链中去
 */
int exfat_chain_add(struct inode *inode, int new_dclus, int nr_cluster)
{	
	struct super_block *sb = inode->i_sb;
	struct exfat_sb_info *sbi = EXFAT_SB(sb);
	struct exfat_inode_info *exi = EXFAT_I(inode);
	int ret = 0, new_fclus, last_iclus,last_dclus;

	/*
	 * We must locate the last cluster of the file to add this new
	 * one (new_dclus) to the end of the link list (the FAT).
	 * 首先需找到该簇链的末端
	 */
	exfat_debug("Entering, new_dclus : 0x%x, nr_cluster :0x%x", new_dclus,nr_cluster);

	last_dclus = new_fclus = 0;
	if(exi->clusnr) {//该节点已经分配过簇
		exfat_debug("to get the last cluster number of exfat inode ");		
		ret = exfat_get_last_cluster(inode, &last_iclus,&last_dclus);
		if(ret)
			return ret;
		new_fclus = last_iclus + 1;	//last_iclus begin with 0 not 1	
		//last_dclus++;				//last_dclus begin with 0 not 1	
		exfat_debug("last_iclus :0x%d,last_dclus:0x%d",last_iclus,last_dclus);
	}


	if (last_dclus) {/* add new one to the last of the cluster chain */
		struct exfat_ent exfat_ent;
		struct exfat_cache_id cid;
		int prev = last_dclus;

		/*not contiguous any more?*/
		if(exi->data_flag == EXFAT_DATA_CONTIGUOUS) {
			exfat_debug("last_dclus :0x%x,exi->clusnr:0x%x",last_dclus,exi->clusnr);
			if(last_dclus!= new_dclus -1){
				exfat_debug("change the contiguous flag to normal one");
				exi->data_flag = exi->dataChangeFlag = EXFAT_DATA_NORMAL;
				add_FatTable(inode,&exfat_ent,last_iclus-1,exi->clusnr);
			}
		}

		if(exi->data_flag == EXFAT_DATA_CONTIGUOUS)
			return 0;

		exfat_debug("data is not contiguous ,to update FAT table");
		if (!exfat_cache_lookup(inode, last_iclus,&cid))
			cache_init(&cid, last_iclus, last_dclus);
		
		exfat_ent_init(&exfat_ent);
		ret = exfat_ent_read(inode, &exfat_ent, prev, &last_dclus);
		if (ret >= 0) {		
			exfat_debug("last_dclus :0x%x",last_dclus);
			ret = exfat_ent_write(inode, &exfat_ent, new_dclus);			
		}	
		ret = exfat_ent_read(inode, &exfat_ent, new_dclus, &last_dclus);
		if (ret >= 0) {			
			exfat_debug("last_dclus :0x%x\n",last_dclus);
			ret = exfat_ent_write(inode, &exfat_ent, EXFAT_ENT_EOF);
			exfat_ent_release(&exfat_ent);
		}
		if (ret < 0)
			return ret;
		/* check if the cache exsit*/
		if (cache_contiguous(&cid, new_dclus))//判断是否连续?
			cid.len++;
		else{
			cache_init(&cid, last_iclus+1, new_dclus);
			exfat_cache_add(inode, &cid);
		}
		exfat_cache_add(inode, &cid);
		
	} else {/*add new one to the inode without clusters*/
		//exfat_debug("exi->phys_size : 0x%llx, inode->i_size :0x%llx ,i_size_read: 0x%llx , inode->i_blocks: 0x%llx",exi->phys_size ,inode->i_size, i_size_read(inode) ,inode->i_blocks);
		exi->last_clusnr = exi->clusnr= new_dclus;		
	
		/*
		 * Since generic_osync_inode() synchronize later if
		 * this is not directory, we don't here.
		 */ 
		if (S_ISDIR(inode->i_mode) && IS_DIRSYNC(inode)) {
			ret = exfat_sync_inode(inode);
			if (ret)
				return ret;
		} else
			mark_inode_dirty(inode);
			
	}
	/*if (new_fclus != (inode->i_blocks >> (sbi->clus_bits - 9))) {
		exfat_fs_panic(sb,"clusters badly computed (%d != %llu)",
			new_fclus, inode->i_blocks >> (sbi->clus_bits - 9));
		exfat_cache_inval(inode);
	}*/
	
	inode->i_blocks += nr_cluster << (sbi->clus_bits - 9);

	exfat_debug("exi->phys_size : 0x%llx, inode->i_size :0x%llx ,i_size_read: 0x%llx , inode->i_blocks: 0x%llx",exi->phys_size ,inode->i_size, i_size_read(inode) ,inode->i_blocks);
	exfat_debug("Done");
	return 0;
}

/*free @num clusters from @dcluster*/
/*所有的簇号必须物理连续!*/
int exfat_free_clusters(struct inode *inode, u32 num,sector_t dcluster)
{
	struct super_block *sb = inode->i_sb;
	struct exfat_sb_info *sbi = EXFAT_SB(sb);
	struct inode * bitmap_inode = sbi->bitmap_inode;
	pgoff_t index,index_tmp;
	struct address_space *mapping;
	struct page *page;
	sector_t dcluster_tmp;
	u8 *kaddr;
	int err = 0;

	exfat_debug("entering, num :0x%x,dcluster :0x%llx",num,dcluster);

	mapping = bitmap_inode->i_mapping;	
	dcluster_tmp = dcluster;
	do{
		index  = (dcluster_tmp -EXFAT_START_ENT)/(8*PAGE_SIZE);					
		/* Get the page containing the first bit (@start_bit). */
		page = exfat_map_page(mapping, index);
		if (IS_ERR(page)) {
			exfat_error("Failed to map %ld page (error "
						"%li), aborting.",index, PTR_ERR(page));
			err =  PTR_ERR(page);
			goto err_out;
		}
		kaddr = page_address(page);
		
		do{
			if(exfat_ent_eof(sbi, dcluster_tmp) || exfat_ent_bad(sbi, dcluster_tmp) || !exfat_ent_valid(sbi, dcluster_tmp))
			{	
				exfat_debug("attempting to free invalid cluster : 0x%llx",dcluster_tmp);
				break;
			}
			index_tmp  = (dcluster_tmp -EXFAT_START_ENT)/(8*PAGE_SIZE);			
			BMAP_CLR(kaddr, (dcluster_tmp - EXFAT_START_ENT)%(8*PAGE_SIZE));		
			sbi->free_clusters++; /* update percentage of used space */	
			dcluster_tmp++;
			num--;
		}while((index == index_tmp) && num);
		
		flush_dcache_page(page);
		set_page_dirty(page);
		lock_page(page);
		write_one_page(page,1);
		exfat_debug("next");
		exfat_unmap_page(page);
	}while(num);
	
	exfat_debug("Done");
	return 0;
	
err_out:
	exfat_debug("Failed");
	return err;	
}
#endif


