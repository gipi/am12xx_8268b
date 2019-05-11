/*
 * Copyright (C) 2008, OGAWA Hirofumi
 * Released under GPL v2.
 */
#include <linux/buffer_head.h>
#include <linux/mpage.h>
#include <linux/hash.h>
#include "exfat.h"
#include "cluster.h"
#include "debug.h"

#ifdef EXFAT_MODIFY

 void exfat_Print_Buf(unsigned char * title,  const unsigned char * pData, unsigned int inLen)
{
	unsigned int iLoop;
#ifndef DEBUG
	return;
#endif
	printk("%s : \n",title);
	for( iLoop=0; iLoop< inLen; iLoop++)
	{
		printk("%2x ", pData[iLoop]);
		if( 0== ((iLoop+1) &0x1f) )
		{
			printk("  %d\n",iLoop);
		}
	}
       printk("\n");
	
}

 int exfat_write_inode(struct inode *inode, int wait)
{

	struct super_block *sb = inode->i_sb;
	//struct exfat_sb_info *sbi = EXFAT_SB(sb);
	struct exfat_inode_info *exi = EXFAT_I(inode);
	struct buffer_head *bh=NULL,*bh_next =NULL;
	struct exfat_chunk_dirent  *dirent;
	struct exfat_chunk_data *data;
	struct exfat_chunk_name *name;
	sector_t blocknr;
	int err=0, i,chunks;	
	u16 csum;
	unsigned long offset,prev_offset;
	struct timespec ctime  = current_kernel_time() ;
	
	exfat_debug("Entering,inode->i_no: %ld",inode->i_ino);
	if (inode->i_ino == EXFAT_ROOT_INO)
		return 0;

	if (!exi->de_blocknr[0]) /*for released inode*/
		return 0;
	
	exfat_debug("exi->de_size: 0x%lx",exi->de_size);

	i = 0;
	blocknr = exi->de_blocknr[i];
	exfat_debug("the 1st blocknr of the entry is 0x%llx",blocknr);
	bh = sb_bread(sb, blocknr);
	if (!bh) {
		exfat_error("unable to read inode block "
		       "for updating (blocknr 0x%llx)\n", blocknr);
		goto err_out;
	}
	//spin_lock(&sbi->inode_hash_lock);
	//exfat_Print_Buf("bh[0]",bh->b_data,512);	
	offset = exi->de_offset;
	dirent = (struct exfat_chunk_dirent *) (bh->b_data+offset);//更新0x85	
	exfat_debug("offset : 0x%lx",offset);
	exfat_debug("dirent->type : 0x%x ,sub entries is 0x%x",dirent->type, dirent->sub_chunks);
	chunks = dirent->sub_chunks;

	exfat_time_unix2exfat(&ctime, &dirent->mtime, &dirent->mdate,&dirent->mtime_cs);
	exfat_time_unix2exfat(&ctime, &dirent->atime, &dirent->adate, NULL);

	/* Skip dirent->checksum field */
	csum = exfat_checksum16(0, dirent, 2);
	csum = exfat_checksum16(csum, (u8 *)dirent + 4, EXFAT_CHUNK_SIZE - 4);
	prev_offset = offset;
/*	if(exi->de_size + exi->de_offset <= sb->s_blocksize){/all entries are in one block/
		offset += EXFAT_CHUNK_SIZE;
		data = (struct exfat_chunk_data *)(bh->b_data+offset);
		 exfat_debug("data->type : 0x%x, data->size : 0x%llx ",data->type, data->size);
		data->size = data->size2 =  i_size_read(inode);			
		data->flag  = exi->data_flag = exi->dataChangeFlag;
		data->clusnr = exi->clusnr;
	 	exfat_debug("data->size : 0x%llx ",data->size);
		exfat_debug("exi->data_flag  :0x%x\n",exi->data_flag);
		csum = exfat_checksum16(csum, data, EXFAT_CHUNK_SIZE*(dirent->sub_chunks));
		goto out;
	}else 
*/
	if(prev_offset >=0x1E0 && exi->de_blocknr[i++] ){/*the 0xc0 entry is in the next block*/
		exfat_debug("case1:");
		offset = 0x0;
		blocknr = exi->de_blocknr[i];
		exfat_debug("the %dst blocknr of the entry is 0x%llx",i,blocknr);
		bh_next = sb_bread(sb, blocknr);
		if (!bh_next) {
			exfat_error("unable to read inode block "
			       "for updating (blocknr 0x%llx)\n", blocknr);
			goto err_out;
		}
		//_Print_Buf("bh[1]",bh_next->b_data,512);
		data = (struct exfat_chunk_data *) (bh_next->b_data);	

	}else{/*the 0xc0 entry is in the same block*/
		exfat_debug("case2:");
		offset += EXFAT_CHUNK_SIZE;
		data = (struct exfat_chunk_data *) (bh->b_data+offset);
		//csum = exfat_checksum16(csum, data, EXFAT_CHUNK_SIZE);
	}
	 exfat_debug("data->type : 0x%x, data->size : 0x%llx ",data->type, data->size);/*update the file size*/
	 data->size = data->size2 =  inode->i_size;
	 data->clusnr = exi->clusnr;
	 data->flag  = exi->data_flag = exi->dataChangeFlag;
	 exfat_debug("data->size : 0x%llx ",data->size);
	 exfat_debug("exi->data_flag  :0x%x\n",exi->data_flag);
	 csum = exfat_checksum16(csum, data, EXFAT_CHUNK_SIZE);
	
	while( --chunks){
		exfat_debug("chunks: %d",chunks);
		prev_offset = offset;		
		if(prev_offset >=0x1E0 && exi->de_blocknr[i++]){
			if(bh_next){
				mark_buffer_dirty(bh_next);			
				if (wait)
					err = sync_dirty_buffer(bh_next);		
				brelse(bh_next);
				if(err)
					goto err_out;
			}
			blocknr = exi->de_blocknr[i];
			exfat_debug("the %dst blocknr of the entry is 0x%llx",i,blocknr);
			bh_next = sb_bread(sb, blocknr);
			if (!bh_next) {
				exfat_error("unable to read inode block "
				       "for updating (blocknr 0x%llx)\n", blocknr);
				goto err_out;
			}
			//exfat_Print_Buf("bh_next",bh_next->b_data,512);
			offset  =0;
			name = (struct exfat_chunk_name *) (bh_next->b_data);

		}else{
			offset += EXFAT_CHUNK_SIZE;			
		}
		if(bh_next)
			name = (struct exfat_chunk_name *) (bh_next->b_data+offset);
		else
			name = (struct exfat_chunk_name *) (bh->b_data+offset);
		csum = exfat_checksum16(csum, name, EXFAT_CHUNK_SIZE);

	}
	exfat_debug("old checksum : 0x%x ,new checksum : 0x%x ",dirent->checksum,csum);
	dirent->checksum  = csum;
	if(bh_next){
		//exfat_debug("next");
		mark_buffer_dirty(bh_next);			
		if (wait)
			err = sync_dirty_buffer(bh_next);		
		if(err)
			goto err_out;	
	}
	//exfat_debug("next");
 	mark_buffer_dirty(bh);			
	if (wait)
		err = sync_dirty_buffer(bh);	
	if(err)
		goto err_out;
	err =0;
err_out:
	//exfat_debug("next");

	if(bh)
		brelse(bh);
	exfat_debug("next");

	if(bh_next)
		brelse(bh_next);
	
	//spin_unlock(&sbi->inode_hash_lock);
	exfat_debug("Done,err : %d\n",err);
	return err;
}

int exfat_sync_inode(struct inode *inode)
{
	return exfat_write_inode(inode, 1);
}

//EXPORT_SYMBOL_GPL(exfat_sync_inode);
/**
 * exfat_unmap_page - release a page that was mapped using ntfs_map_page()
 * @page:	the page to release
 *
 * Unpin, unmap and release a page that was obtained from exfat_map_page().
 */
void exfat_unmap_page(struct page *page)
{
	//exfat_debug("Entering page[%ld]",page->index);
	kunmap(page);
	page_cache_release(page);

}
/**
 * exfat_map_page - map a page into accessible memory, reading it if necessary
 * @mapping:	address space for which to obtain the page
 * @index:	index into the page cache for @mapping of the page to map
 *
 * Read a page from the page cache of the address space @mapping at position
 * @index, where @index is in units of PAGE_CACHE_SIZE, and not in bytes.
 *
 * If the page is not in memory it is loaded from disk first using the readpage
 * method defined in the address space operations of @mapping and the page is
 * added to the page cache of @mapping in the process.
 *
 * If the page belongs to an mst protected attribute and it is marked as such
 * in its ntfs inode (NInoMstProtected()) the mst fixups are applied but no
 * error checking is performed.  This means the caller has to verify whether
 * the ntfs record(s) contained in the page are valid or not using one of the
 * ntfs_is_XXXX_record{,p}() macros, where XXXX is the record type you are
 * expecting to see.  (For details of the macros, see fs/ntfs/layout.h.)
 *
 * If the page is in high memory it is mapped into memory directly addressible
 * by the kernel.
 *
 * Finally the page count is incremented, thus pinning the page into place.
 *
 * The above means that page_address(page) can be used on all pages obtained
 * with ntfs_map_page() to get the kernel virtual address of the page.
 *
 * When finished with the page, the caller has to call ntfs_unmap_page() to
 * unpin, unmap and release the page.
 *
 * Note this does not grant exclusive access. If such is desired, the caller
 * must provide it independently of the ntfs_{un}map_page() calls by using
 * a {rw_}semaphore or other means of serialization. A spin lock cannot be
 * used as ntfs_map_page() can block.
 *
 * The unlocked and uptodate page is returned on success or an encoded error
 * on failure. Caller has to test for error using the IS_ERR() macro on the
 * return value. If that evaluates to 'true', the negative error code can be
 * obtained using PTR_ERR() on the return value of ntfs_map_page().
 */
struct page *exfat_map_page(struct address_space *mapping,
		unsigned long index)
{
	struct page *page = read_mapping_page(mapping, index, NULL);
	//exfat_debug("Entering page[%ld]",page->index);

	if (!IS_ERR(page)) {
		kmap(page);
		if (!PageError(page))
			return page;
		exfat_unmap_page(page);
		return ERR_PTR(-EIO);
	}
	return page;
}

int exfat_add_cluster(struct inode *inode)
{
	int err;
	sector_t cluster;
	struct exfat_inode_info *exi = EXFAT_I(inode);
	
	exfat_debug("Entering,ino:0x%lx",inode->i_ino);
	exfat_debug("exi->data_flag  :0x%x",exi->data_flag);

	err = exfat_alloc_clusters(inode, &cluster, 1);//申请一簇
	if (err)
		return err;
	/* FIXME: this cluster should be added after data of this
	 * cluster is writed */

	if((exi->data_flag & EXFAT_DATA_NORMAL) && !exi->phys_size)
	{	
		exi->data_flag = exi->dataChangeFlag = EXFAT_DATA_CONTIGUOUS;
		exfat_debug("change the normal flag to  contiguous one");
	}	
	err = exfat_chain_add(inode, cluster, 1);
	if (err)
		exfat_free_clusters(inode,1,cluster);
	
	exi->last_clusnr  =cluster;
	
	exfat_debug("Done,exi->data_flag  :0x%x\n",exi->data_flag);
	return err;
}

/*	sector:<输入>逻辑扇区
* 	phys:<输出>物理开始扇区
*	mapped_blocks:<输出>map的扇区总数
*/
int _exfat_bmap(struct inode *inode, sector_t sector, sector_t *phys,
	     unsigned long *mapped_blocks)
{
	struct super_block *sb = inode->i_sb;
	struct exfat_sb_info *sbi = EXFAT_SB(sb);
	struct exfat_inode_info *exi = EXFAT_I(inode);
	sector_t last_iblock;
	int cluster, offset;
	unsigned long blocksize;
	int err = 0;
	struct exfat_clus_map cmap;

	*phys = 0;
	*mapped_blocks = 0;
	
	blocksize = 1 << inode->i_blkbits;;
	last_iblock = (exi->phys_size+ (blocksize - 1))//该节点的最大block数
		>> inode->i_blkbits;
	//exfat_debug("exi->phys_size : 0x%llx, inode->i_size :0x%llx ,i_size_read: 0x%llx , inode->i_blocks: 0x%llx",exi->phys_size ,inode->i_size, i_size_read(inode) ,inode->i_blocks);
	//exfat_debug("last_iblock : 0x%llx ,sector : 0x%llx ",last_iblock,sector);
	if (sector >= last_iblock){
		//exfat_debug("have not enough blocks, need to allocate later");
		return 0;
	}

	cluster = sector >> (sbi->clus_bits - sb->s_blocksize_bits);	//sector对应的簇数
	offset  = sector & (sbi->bpc - 1);							//簇中偏移
	err = exfat_get_cluster(inode, cluster, 1, &cmap);//得到第iclusnr簇
	if (err)
		return err;	

	if (!exfat_ent_valid(sbi, cmap.clusnr)) {
			exfat_error("unexpected exFAT entry (start cluster 0x%08x, entry 0x%08x)",
			       exi->clusnr, cmap.clusnr);
			return -EIO;
		}
	*phys = exfat_clus_to_blknr(sbi, cmap.clusnr) + offset;//簇对应的物理扇区
	*mapped_blocks = (cmap.len << sbi->bpc_bits) - offset;//map的扇区总数
	if (*mapped_blocks > last_iblock - sector)
		*mapped_blocks = last_iblock - sector;	
	return 0;
}

int exfat_get_block2(struct inode *inode, sector_t iblock,
		    struct buffer_head *bh_result, int create)
{
	struct super_block *sb = inode->i_sb;
	struct exfat_sb_info *sbi = EXFAT_SB(sb);
	struct exfat_inode_info *exi = EXFAT_I(inode);
	unsigned long max_blocks, mapped_blocks;	
	int err, offset;
	sector_t phys;
	
	max_blocks = bh_result->b_size >> inode->i_blkbits;//buffer head能存的最大block数:不会超过8
	mapped_blocks = 0;

	//exfat_debug("Entering");
	err = _exfat_bmap(inode, iblock, &phys, &mapped_blocks);
	if (err)
		return err;
	if (phys) {//对于扩展写， phys=0
		map_bh(bh_result, sb, phys);
		mapped_blocks = min(mapped_blocks, max_blocks);
		bh_result->b_size = mapped_blocks << inode->i_blkbits;//共映射的block数
		//exfat_debug("mapped blocks : 0x%lx begin from blocknr 0x%llx ",mapped_blocks,phys);
		return 0;		
	}
	if (!create)
		return 0;
	
	/**extend write, need to alloc clusters*/
	DEBUG_ON(iblock != exi->phys_size >> inode->i_blkbits,
		 "iblock %llu, phys_size %lld\n", (llu)iblock, exi->phys_size);
	DEBUG_ON(mapped_blocks >= sbi->bpc,
		 "mapped_blocks %lu\n", mapped_blocks);

	offset = (unsigned long)iblock & (sbi->bpc- 1);
	if (!offset) {
		/* TODO: multiple cluster allocation would be desirable. */
		err = exfat_add_cluster(inode);
		if (err){
			exfat_debug("err :%d",err);
			return err;
			}
	}
	/* available blocks on this cluster */
	mapped_blocks = sbi->bpc - offset;

	max_blocks= min(mapped_blocks, max_blocks);
	exi->phys_size += max_blocks << sb->s_blocksize_bits;
	//inode->i_size  = exi->phys_size;
	//exfat_sync_inode(inode);

	err = _exfat_bmap(inode, iblock, &phys, &mapped_blocks);
	if (err)
		return err;

	BUG_ON(!phys);
	BUG_ON(max_blocks != mapped_blocks);
	set_buffer_new(bh_result);
	map_bh(bh_result, sb, phys);
	bh_result->b_size = max_blocks << inode->i_blkbits;
	//exfat_debug("mapped blocks : 0x%lx begin from blocknr 0x%llx ",max_blocks,phys);

	return 0;
	
}
#endif

int exfat_get_block(struct inode *inode, sector_t iblock,
		    struct buffer_head *bh_result, int create)
{
	struct super_block *sb = inode->i_sb;
	struct exfat_sb_info *sbi = EXFAT_SB(sb);
	struct exfat_inode_info *exi = EXFAT_I(inode);
	unsigned long blocksize, max_blocks, mapped_blocks;
	sector_t blocknr, last_iblock;
	u32 iclusnr, last_iclusnr;
	int err, offset;

	blocksize = 1 << inode->i_blkbits;;
	last_iblock = (i_size_read(inode) + (blocksize - 1))//该节点的最大block数
		>> inode->i_blkbits;
	if (!create && iblock >= last_iblock)
		return -EIO;

	/*
	 * Find the available blocks already allocating
	 */

	last_iclusnr = (last_iblock + (sbi->bpc - 1)) >> sbi->bpc_bits;//该节点的最大簇数
	iclusnr = iblock >> sbi->bpc_bits;//要获取的簇号
	offset = iblock & (sbi->bpc - 1);

	max_blocks = bh_result->b_size >> inode->i_blkbits;//buffer head能存的最大block数:不会超过8
	mapped_blocks = 0;
	if (iclusnr < last_iclusnr) {
		/* Some clusters are available */
		struct exfat_clus_map cmap;
		u32 clus_len;

		clus_len = (max_blocks + (sbi->bpc - 1)) >> sbi->bpc_bits;//buffer head能存的最大簇长
		err = exfat_get_cluster(inode, iclusnr, clus_len, &cmap);//得到第iclusnr簇
		if (err)
			return err;

		if (!exfat_ent_valid(sbi, cmap.clusnr)) {
			exfat_error("unexpected exFAT entry (start cluster 0x%08x, entry 0x%08x)",
			       exi->clusnr, cmap.clusnr);
			return -EIO;
		}

		blocknr = exfat_clus_to_blknr(sbi, cmap.clusnr) + offset;//将簇转化为block
		mapped_blocks = (cmap.len << sbi->bpc_bits) - offset;
		if (iblock < last_iblock) {
			/* Some blocks are available */
			unsigned long avail_blocks = last_iblock - iblock;

			mapped_blocks = min(mapped_blocks, avail_blocks);
			mapped_blocks = min(mapped_blocks, max_blocks);

			map_bh(bh_result, sb, blocknr);
			bh_result->b_size = mapped_blocks << inode->i_blkbits;//共映射的block数
			//exfat_debug("mapped blocks : 0x%lx from blocknr 0x%llx ",mapped_blocks,blocknr);
			return 0;
		}
		/* We can allocate blocks in this last cluster */
	}
	
	/* FIXME: write is not supported */
	DEBUG0_ON(create);
	/*
	 * Allocate new blocks
	 */
	DEBUG_ON(iblock != exi->phys_size >> inode->i_blkbits,
		 "iblock %llu, phys_size %lld\n", (llu)iblock, exi->phys_size);
	DEBUG_ON(mapped_blocks >= sbi->bpc,
		 "mapped_blocks %lu\n", mapped_blocks);

	return -EIO;

	/* FIXME: multiple cluster allocation would be desirable. */
	if (!mapped_blocks) {
		/* We have to allocate new cluster */
//		err = fat_add_cluster(inode, iclusnr);
		if (err)
			return err;

		mapped_blocks += sbi->bpc;
	} else if (mapped_blocks < max_blocks) {
		/* Try to allocate next cluster of this cluster */
//		err = fat_add_next_cluster(inode, iclusnr);
		if (err)
			return err;

		mapped_blocks += sbi->bpc;
	}

	mapped_blocks = min(mapped_blocks, max_blocks);
	exi->phys_size += mapped_blocks << inode->i_blkbits;

	set_buffer_new(bh_result);
	map_bh(bh_result, sb, blocknr);
	bh_result->b_size = mapped_blocks << inode->i_blkbits;
	exfat_debug("mapped blocks : 0x%lx from blocknr 0x%llx ",mapped_blocks,blocknr);
	return 0;
}

static int exfat_readpage(struct file *file, struct page *page)
{
	return mpage_readpage(page, exfat_get_block2);
}

static int exfat_readpages(struct file *file, struct address_space *mapping,
			   struct list_head *pages, unsigned nr_pages)
{
	return mpage_readpages(mapping, pages, nr_pages, exfat_get_block2);
}

static sector_t exfat_bmap(struct address_space *mapping, sector_t iblock)
{
	sector_t blocknr;

	/* exfat_get_cluster() assumes the requested blocknr isn't truncated. */
	mutex_lock(&mapping->host->i_mutex);
	blocknr = generic_block_bmap(mapping, iblock, exfat_get_block);
	mutex_unlock(&mapping->host->i_mutex);

	return blocknr;
}
#ifdef EXFAT_MODIFY
static int exfat_write_begin(struct file *file, struct address_space *mapping,
			loff_t pos, unsigned len, unsigned flags,
			struct page **pagep, void **fsdata)
{
	*pagep = NULL;
	int err;
	err =  cont_write_begin(file, mapping, pos, len, flags, pagep, fsdata,
				exfat_get_block2,
				&EXFAT_I(mapping->host)->phys_size);
	return err;

}

static int exfat_write_end(struct file *file, struct address_space *mapping,
			loff_t pos, unsigned len, unsigned copied,
			struct page *pagep, void *fsdata)
{
	struct inode *inode = mapping->host;
	int err;

	err = generic_write_end(file, mapping, pos, len, copied, pagep, fsdata);
	if (!(err < 0) && !(EXFAT_I(inode)->attrib & EXFAT_ATTR_ARCH)) {
		inode->i_mtime = inode->i_ctime = CURRENT_TIME_SEC;
		EXFAT_I(inode)->attrib |= EXFAT_ATTR_ARCH;
		mark_inode_dirty(inode);
	}
	return err;
}

static int exfat_writepage(struct page *page, struct writeback_control *wbc)
{
	return(block_write_full_page(page, exfat_get_block2, wbc));
}

static int exfat_writepages(struct address_space *mapping,
			  struct writeback_control *wbc)
{	
	return(mpage_writepages(mapping, wbc, exfat_get_block2));
}
#endif

static const struct address_space_operations exfat_aops = {
	.readpage			= exfat_readpage,
	.readpages		= exfat_readpages,
#ifdef EXFAT_MODIFY
	.writepage		= exfat_writepage,
	.writepages		= exfat_writepages,
	.sync_page		= block_sync_page,
	.write_begin		= exfat_write_begin,
	.write_end		= exfat_write_end,
#endif
	.bmap			= exfat_bmap,
//	.invalidatepage		= ext4_da_invalidatepage,
//	.releasepage		= ext4_releasepage,
//	.direct_IO		= ext4_direct_IO,
//	.migratepage		= buffer_migrate_page,
//	.is_partially_uptodate	= block_is_partially_uptodate,
};

int exfat_getattr(struct vfsmount *mnt, struct dentry *dentry,
		  struct kstat *stat)
{
	struct inode *inode = dentry->d_inode;
	generic_fillattr(inode, stat);
	stat->blksize = EXFAT_SB(inode->i_sb)->clus_size;
	return 0;
}

static const struct file_operations exfat_file_ops = {
	.llseek		= generic_file_llseek,
	.read		= do_sync_read,
	.write		= do_sync_write,
	.aio_read	= generic_file_aio_read,
	.aio_write	= generic_file_aio_write,
//	.unlocked_ioctl	= fat_generic_ioctl,
#ifdef CONFIG_COMPAT
//	.compat_ioctl	= fat_compat_dir_ioctl,
#endif
	.mmap		= generic_file_mmap,
	.open		= generic_file_open,
//	.fsync		= file_fsync,
	.splice_read	= generic_file_splice_read,
	.splice_write	= generic_file_splice_write,
};

static const struct inode_operations exfat_file_inode_ops = {
//	.truncate	= ext4_truncate,
//	.permission	= ext4_permission,
//	.setattr	= ext4_setattr,
	.getattr	= exfat_getattr
#ifdef CONFIG_EXT4DEV_FS_XATTR
//	.setxattr	= generic_setxattr,
//	.getxattr	= generic_getxattr,
//	.listxattr	= ext4_listxattr,
//	.removexattr	= generic_removexattr,
#endif
//	.fallocate	= ext4_fallocate,
//	.fiemap		= ext4_fiemap,
};

/* See comment in fs/fat/inode.c */
/* FIXME: rethink those hash stuff */

/*
 * New FAT inode stuff. We do the following:
 *	a) i_ino is constant and has nothing with on-disk location.
 *	b) FAT manages its own cache of directory entries.
 *	c) *This* cache is indexed by on-disk location.
 *	d) inode has an associated directory entry, all right, but
 *		it may be unhashed.
 *	e) currently entries are stored within struct inode. That should
 *		change.
 *	f) we deal with races in the following way:
 *		1. readdir() and lookup() do FAT-dir-cache lookup.
 *		2. rename() unhashes the F-d-c entry and rehashes it in
 *			a new place.
 *		3. unlink() and rmdir() unhash F-d-c entry.
 *		4. fat_write_inode() checks whether the thing is unhashed.
 *			If it is we silently return. If it isn't we do bread(),
 *			check if the location is still valid and retry if it
 *			isn't. Otherwise we do changes.
 *		5. Spinlock is used to protect hash/unhash/location check/lookup
 *		6. fat_clear_inode() unhashes the F-d-c entry.
 *		7. lookup() and readdir() do igrab() if they find a F-d-c entry
 *			and consider negative result as cache miss.
 */
void exfat_hash_init(struct super_block *sb)
{
	struct exfat_sb_info *sbi = EXFAT_SB(sb);
	int i;

	spin_lock_init(&sbi->inode_hash_lock);
	for (i = 0; i < EXFAT_HASH_SIZE; i++)
		INIT_HLIST_HEAD(&sbi->inode_hashtable[i]);
}

static inline u32 exfat_hash(struct super_block *sb, sector_t blocknr,
			     unsigned long offset)
{
	u32 key = (((loff_t)blocknr << sb->s_blocksize_bits) | offset)
		>> EXFAT_CHUNK_BITS;
	return hash_32(key, EXFAT_HASH_BITS);
}

static void exfat_attach(struct inode *inode, struct exfat_parse_data *pd)
{
	struct super_block *sb = inode->i_sb;
	struct exfat_sb_info *sbi = EXFAT_SB(sb);
	struct exfat_inode_info *exi = EXFAT_I(inode);
	u32 hashval = exfat_hash(sb, pd->bhs[0]->b_blocknr, pd->bh_offset);
	struct hlist_head *head = sbi->inode_hashtable + hashval;
	int i;

	exfat_debug("Entering, inode->i_no :0x%lx",inode->i_ino);
	spin_lock(&sbi->inode_hash_lock);
	for (i = 0; i < pd->nr_bhs; i++){
		exi->de_blocknr[i] = pd->bhs[i]->b_blocknr;
	}
	exfat_debug("pd->nr_bhs : %d ",pd->nr_bhs);
	exfat_Print_Buf("exi->de_blocknr",(unsigned char *)exi->de_blocknr,pd->nr_bhs*sizeof(sector_t));

	exi->de_offset = pd->bh_offset;
	exi->de_size = pd->size;	
	exfat_debug("exi->clusnr : 0x%x, exi->phys_size : 0x%llx, exi->de_offset : 0x%lx ,exi->de_size :0x%lx",exi->clusnr,exi->phys_size,exi->de_offset,exi->de_size);
	hlist_add_head(&exi->i_hash, head);
	spin_unlock(&sbi->inode_hash_lock);
}

static void __exfat_detach(struct exfat_inode_info *exi)
{
	exi->de_blocknr[0] = 0;
	exi->de_offset = -1;
	exi->de_size = -1;
}

void exfat_detach(struct inode *inode)
{
	struct exfat_sb_info *sbi = EXFAT_SB(inode->i_sb);
	struct exfat_inode_info *exi = EXFAT_I(inode);

	spin_lock(&sbi->inode_hash_lock);
	__exfat_detach(exi);
	hlist_del_init(&exi->i_hash);
	spin_unlock(&sbi->inode_hash_lock);
}

struct inode *exfat_ilookup(struct super_block *sb, sector_t blocknr,
			    unsigned long offset)
{
	struct exfat_sb_info *sbi = EXFAT_SB(sb);
	u32 hashval = exfat_hash(sb, blocknr, offset);
	struct hlist_head *head = sbi->inode_hashtable + hashval;
	struct hlist_node *_p;
	struct exfat_inode_info *exi;
	struct inode *inode = NULL;

	spin_lock(&sbi->inode_hash_lock);
	hlist_for_each_entry(exi, _p, head, i_hash) {
		DEBUG_ON(exi->vfs_inode.i_sb != sb,
			 "vfs.sb %p, sb %p\n", exi->vfs_inode.i_sb, sb);
		if (exi->de_blocknr[0] != blocknr)
			continue;
		if (exi->de_offset != offset)
			continue;
		inode = igrab(&exi->vfs_inode);
		if (inode)
			break;
	}
	spin_unlock(&sbi->inode_hash_lock);
	return inode;
}

/*初始化exfat inode
* @inode :待初始化的inode
* @ino : inode number,从1开始
*
*/
static void exfat_fill_inode(struct inode *inode, unsigned long ino,
			     struct exfat_chunk_dirent *dirent,
			     struct exfat_chunk_data *data)
{
	struct exfat_sb_info *sbi = EXFAT_SB(inode->i_sb);
	struct exfat_inode_info *exi = EXFAT_I(inode);

	inode->i_ino = ino;
	inode->i_nlink = 1;	/*  Windows use 1 even if directory.*/
	inode->i_uid = sbi->opts.uid;
	inode->i_gid = sbi->opts.gid;
	inode->i_version = 1;
	/* FIXME: inode->i_size is "signed", have to check overflow? */
	inode->i_size = le64_to_cpu(data->size);
	/* FIXME: sane time conversion is needed */
	exfat_time_exfat2unix(&inode->i_mtime, dirent->mtime, dirent->mdate,
			      dirent->mtime_cs);
	exfat_time_exfat2unix(&inode->i_atime, dirent->atime, dirent->adate, 0);
	inode->i_ctime.tv_sec = 0;
	inode->i_ctime.tv_nsec = 0;
	inode->i_blocks = (inode->i_size + (sbi->clus_size - 1)) >> 9;
/*	inode->i_generation = get_seconds(); FIXME: don't support nfs? */
	if (dirent->attrib & cpu_to_le16(EXFAT_ATTR_DIR)) {
		inode->i_mode = S_IFDIR | sbi->opts.dmode;
		inode->i_op = &exfat_dir_inode_ops;
		inode->i_fop = &exfat_dir_ops;
	} else {
		inode->i_mode = S_IFREG | sbi->opts.fmode;
		inode->i_op = &exfat_file_inode_ops;
		inode->i_fop = &exfat_file_ops;
		inode->i_mapping->a_ops = &exfat_aops;
	}
#if 0
	/* FIXME: separate S_NOCMTIME to S_NOCTIME and S_NOMTIME,
	 * then exFAT/FAT can use S_NOCTIME. */
	inode->i_flags |= S_NOCTIME;

	/* FIXME: how handle these attrib? */
	if (dirent->attrib & cpu_to_le16(EXFAT_ATTR_SYSTEM)) {
		if (sbi->opts.sys_immutable)
			inode->i_flags |= S_IMMUTABLE;
	}
	if (dirent->attrib & cpu_to_le16(EXFAT_ATTR_RO)) {
		if (sbi->opts.sys_immutable)
			inode->i_flags |= S_IMMUTABLE;
	}
#endif
	exi->phys_size = inode->i_size;
	exi->last_clusnr  = exi->clusnr = le32_to_cpu(data->clusnr);/*add by yangli*/
	exi->attrib = dirent->attrib;
	exi->dataChangeFlag = exi->data_flag = data->flag;/*add by yangli*/

	__exfat_detach(exi);
}

#ifdef EXFAT_MODIFY
void exfat_attach2(struct inode *inode, struct exfat_entry_data *ed)
{
	struct super_block *sb = inode->i_sb;
	struct exfat_sb_info *sbi = EXFAT_SB(sb);
	struct exfat_inode_info *exi = EXFAT_I(inode);
	u32 hashval = exfat_hash(sb, ed->bnr[0], ed->bh_offset);
	struct hlist_head *head = sbi->inode_hashtable + hashval;
	int i;

	spin_lock(&sbi->inode_hash_lock);
	for (i = 0; i < ed->nr_bnr; i++){
		exi->de_blocknr[i] =ed->bnr[i];
	}
	exfat_debug("ed->nr_bhs : %d ",ed->nr_bnr);
	exfat_Print_Buf("exi->de_blocknr",(unsigned char *)exi->de_blocknr,ed->nr_bnr*sizeof(sector_t));
	exi->de_offset = ed->bh_offset;
	exi->de_size = ed->size;	
	exfat_debug("exi->clusnr : 0x%x, exi->phys_size : 0x%llx, exi->de_offset : 0x%lx ,exi->de_size :0x%lx",exi->clusnr,exi->phys_size,exi->de_offset,exi->de_size);
	hlist_add_head(&exi->i_hash, head);
	spin_unlock(&sbi->inode_hash_lock);
}

struct inode *exfat_build_inode(struct super_block *sb,struct exfat_entry_data *ed,u16 attrib , u64 size, u32 dclusnr)
{
	struct inode *inode;
	unsigned long ino;
	
	struct exfat_chunk_dirent dirent = { //File Directory Entry: 0x85
		.attrib	= cpu_to_le16(attrib),
	};
	struct exfat_chunk_data data = {		
		.flag	= (attrib & EXFAT_ATTR_DIR)? EXFAT_DATA_CONTIGUOUS: EXFAT_DATA_NORMAL,
		.clusnr	= cpu_to_le32(dclusnr),
		.size	= cpu_to_le64(size),
	};

	exfat_debug("Entering, flag :0x%x",data.flag);
	/* FIXME: this can use ilookup5 (with inode_lock). Which one is fast? */
	inode = exfat_ilookup(sb, ed->bnr[0], ed->bh_offset);//在哈希表中查找inode
	if (inode)
		return inode;
	inode = new_inode(sb);
	if (!inode)
		return ERR_PTR(-ENOMEM);

	ino = iunique(sb, EXFAT_RESERVED_INO);//得到唯一的inode号
	exfat_debug("ino : 0x%lx ",ino);
	exfat_fill_inode(inode, ino, &dirent, &data);

	exfat_attach2(inode, ed);
	insert_inode_hash(inode);
	
	exfat_debug("Done");

	return inode;
}
#endif

struct inode *exfat_iget(struct super_block *sb,
			 struct exfat_parse_data *pd,
			 struct exfat_chunk_dirent *dirent,
			 struct exfat_chunk_data *data)
{
	struct inode *inode;
	unsigned long ino;

	exfat_debug("Entering");
	/* FIXME: this can use ilookup5 (with inode_lock). Which one is fast? */
	inode = exfat_ilookup(sb, pd->bhs[0]->b_blocknr, pd->bh_offset);//在哈希表中查找inode
	if (inode)
		return inode;

	inode = new_inode(sb);
	if (!inode)
		return ERR_PTR(-ENOMEM);

	ino = iunique(sb, EXFAT_RESERVED_INO);//得到唯一的inode号
	exfat_fill_inode(inode, ino, dirent, data);

	exfat_attach(inode, pd);
	insert_inode_hash(inode);
	
	exfat_debug("Done");
	return inode;
}

/*分配新的inode*/
struct inode *exfat_new_internal_inode(struct super_block *sb,
				       unsigned long ino, u16 attrib,
				       u32 clusnr, u64 size)
{
	struct inode *inode;
	struct exfat_chunk_dirent dirent = { //File Directory Entry: 0x85
		.attrib	= cpu_to_le16(attrib),
	};
	struct exfat_chunk_data data = {
		.flag	= EXFAT_DATA_NORMAL,
		.clusnr	= cpu_to_le32(clusnr),
		.size	= cpu_to_le64(size),
	};

	inode = new_inode(sb);//申请新的节点
	if (inode) {
		exfat_fill_inode(inode, ino, &dirent, &data);//初始化inode
		/* The internal inode doesn't have timestamp */
		inode->i_flags |= S_NOATIME | S_NOCMTIME;
	}
	return inode;
}

struct inode *exfat_rootdir_iget(struct super_block *sb)
{
	struct exfat_sb_info *sbi = EXFAT_SB(sb);
	struct inode *inode;
	struct exfat_clus_map cmap;
	int err;

	exfat_debug("Entering");
	err = -ENOMEM;
	inode = exfat_new_internal_inode(sb, EXFAT_ROOT_INO, EXFAT_ATTR_DIR,
					 sbi->rootdir_clusnr, 0);
	if (!inode)
		goto error;

	/* Get last iclusnr to set ->i_size */
	err = exfat_get_cluster(inode, EXFAT_ENT_EOF, 0, &cmap);
	if (err)
		goto error_iput;

	if (!exfat_ent_eof(sbi, cmap.clusnr)) {
		exfat_error( "exFAT: found invalid FAT entry 0x%08x"
		       " for root directory", cmap.clusnr);
		err = -EIO;
		goto error_iput;
	}

	inode->i_size = cmap.iclusnr << sbi->clus_bits;
	EXFAT_I(inode)->phys_size = inode->i_size;
	inode->i_blocks = (inode->i_size + (sbi->clus_size - 1)) >> 9;

	insert_inode_hash(inode);
	exfat_debug("Done");

	return inode;

error_iput:
	iput(inode);
error:
	return ERR_PTR(err);
}
