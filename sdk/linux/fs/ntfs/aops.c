/**
 * aops.c - NTFS kernel address space operations and page cache handling.
 *	    Part of the Linux-NTFS project.
 *
 * Copyright (c) 2001-2007 Anton Altaparmakov
 * Copyright (c) 2002 Richard Russon
 *
 * This program/include file is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program/include file is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in the main directory of the Linux-NTFS
 * distribution in the file COPYING); if not, write to the Free Software
 * Foundation,Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/pagemap.h>
#include <linux/swap.h>
#include <linux/buffer_head.h>
#include <linux/writeback.h>
#include <linux/bit_spinlock.h>
#include <linux/mpage.h>
#include <linux/bio.h>

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/blkdev.h>
#include <linux/highmem.h>
#include <linux/prefetch.h>
#include <linux/writeback.h>
#include <linux/backing-dev.h>
#include <linux/pagevec.h>

#include "aops.h"
#include "attrib.h"
#include "debug.h"
#include "inode.h"
#include "mft.h"
#include "runlist.h"
#include "types.h"
#include "ntfs.h"

#ifdef NTFS_MODIFY
static int check_continue(struct buffer_head *bh[],int nr)
{	
	int i =0;
	while(i<nr-1){
		if((bh[i]->b_blocknr+1) !=  bh[i+1]->b_blocknr)
			return 0;		
		i++;
	}
	return 1;		
}
static void ntfs_end_bio_async_read(struct bio *bio, int uptodate)
{
	return;
}

static inline int __ntfs_get_block(struct inode *inode, sector_t iblock,
				  unsigned long *max_blocks,
				  struct buffer_head *bh_result, int create)
{
	loff_t i_size;
	VCN vcn;
	LCN lcn;
	s64 init_size;
	ntfs_inode *ni;
	unsigned long flags;
	ntfs_volume *vol;
	runlist_element *rl;
	sector_t  lblock, zblock;
	unsigned int blocksize, vcn_ofs;
	unsigned char blocksize_bits;
	bool is_retry = false;
	int err = 0;

	ni = NTFS_I(inode);
	vol = ni->vol;

	/* $MFT/$DATA must have its complete runlist in memory at all times. */
	DEBUG0_ON(!ni->runlist.rl && !ni->mft_no && !NInoAttr(ni));

	blocksize = vol->sb->s_blocksize;//块大小==512
	blocksize_bits = vol->sb->s_blocksize_bits;//块大小位数==9

	read_lock_irqsave(&ni->size_lock, flags);
	lblock = (ni->allocated_size + blocksize - 1) >> blocksize_bits;//属性分配大小所占的总block数
	init_size = ni->initialized_size;//属性初始化的大小
	i_size = i_size_read(inode);//ntfs属性大小	
	read_unlock_irqrestore(&ni->size_lock, flags);
	
	if (unlikely(init_size > i_size)) {
		/* Race with shrinking truncate. */
		init_size = i_size;
	}
	zblock = (init_size + blocksize - 1) >> blocksize_bits;//属性初始化大小所占的总blocks
	//printk("iblock==%lld ,lblock ==%lld, init_size ==%lld ,i_size ==%lld ,zblock==%lld\n",iblock,lblock,init_size,i_size,zblock);

	rl = NULL;

	if(iblock>lblock )/* Is the block within the allowed limits? */
		err = -EIO;
	
	bh_result->b_bdev = vol->sb->s_bdev;
		
	/* Convert iblock into corresponding vcn and offset. */
	vcn = (VCN)iblock << blocksize_bits >>
			vol->cluster_size_bits;
	vcn_ofs = ((VCN)iblock << blocksize_bits) &
			vol->cluster_size_mask;
			
	if (!rl) {
lock_retry_remap:
		 if(ni->mft_no == 0)
			 ntfs_debug("down_read");
		down_read(&ni->runlist.lock);
		rl = ni->runlist.rl;
	}
	if (likely(rl != NULL)) {
		/* Seek to element containing target vcn. */
		while (rl->length && rl[1].vcn <= vcn)
			rl++;
			lcn = ntfs_rl_vcn_to_lcn(rl, vcn);
	} else
		lcn = LCN_RL_NOT_MAPPED;

			
	/* Successful remap. */
	if (lcn >= 0) {
		/* Setup buffer head to correct block. */
		bh_result->b_blocknr = ((lcn << vol->cluster_size_bits)//将簇号转为物理扇区号
		+ vcn_ofs) >> blocksize_bits;
		set_buffer_mapped(bh_result);

		/*printk("bh->b_blocknr ==%lld ",bh_result->b_blocknr);
		printk("vcn ==%lld ,vcn_ofs ==%d,lcn ==%lld ",vcn,vcn_ofs,lcn);
		printk("iblock ==%lld \n",iblock);*/
		
		/* Only read initialized data blocks. */
		if (iblock >= zblock) 
			/* Fully non-initialized data block, zero it. */
			goto err_out;
		else 
			goto out;
	}

	/* If first try and runlist unmapped, map and retry. */
	if (!is_retry && lcn == LCN_RL_NOT_MAPPED) {
		is_retry = true;
		/*
		 * Attempt to map runlist, dropping lock for
		 * the duration.
		 */
		 if(ni->mft_no == 0)
			 ntfs_debug("up_read");
		up_read(&ni->runlist.lock);
		err = ntfs_map_runlist(ni, vcn);
		if (likely(!err))
			goto lock_retry_remap;
			rl = NULL;
		} 
	else if (!rl)
		 if(ni->mft_no == 0)
			 ntfs_debug("up_read");
		up_read(&ni->runlist.lock);
	/*
	 * If buffer is outside the runlist, treat it as a
	 * hole.  This can happen due to concurrent truncate
	 * for example.
		 */
	
	/* Hard error, zero out region. */
	if (!err){
		err = -EIO;
		bh_result->b_blocknr = -1;
		ntfs_error(vol->sb, "Failed to read from inode 0x%lx, "
			"attribute type 0x%x, vcn 0x%llx, "
			"offset 0x%x because its location on "
			"disk could not be determined%s "
			"(error code %i).", ni->mft_no,
			ni->type, (unsigned long long)vcn,
			vcn_ofs, is_retry ? " even after "
			"retrying" : "", err);
	}
	/*
	 * Either iblock was outside lblock limits or
	 * ntfs_rl_vcn_to_lcn() returned error.  Just zero that portion
	 * of the page and set the buffer uptodate.
	 */
err_out:
	bh_result->b_blocknr = -1UL;
	clear_buffer_mapped(bh_result);	
out:	
	/* Release the lock if we took it. */
	if (rl){
		 if(ni->mft_no == 0)
			 ntfs_debug("up_read");
		up_read(&ni->runlist.lock);
		}

	*max_blocks = 8;
	return err;
	
}

//哈哈，fat_get_block每次最多也是获取8个blocks
static int ntfs_get_block(struct inode *inode, sector_t iblock,
			 struct buffer_head *bh_result, int create)
{
	struct super_block *sb = inode->i_sb;
	unsigned long max_blocks = bh_result->b_size >> inode->i_blkbits;//申请的最大块号
	int err;

	//iblock: 逻辑起始块号?
	err = __ntfs_get_block(inode, iblock, &max_blocks, bh_result, create);
	//printk("err is %d \n",err);
	if (err)
		return err;
	bh_result->b_size = max_blocks << sb->s_blocksize_bits;
	//printk("ntfs_get_block : %d \n",bh_result->b_size);
	return 0;
}
#endif	
/**
 * ntfs_end_buffer_async_read - async io completion for reading attributes
 * @bh:		buffer head on which io is completed
 * @uptodate:	whether @bh is now uptodate or not
 *
 * Asynchronous I/O completion handler for reading pages belonging to the
 * attribute address space of an inode.  The inodes can either be files or
 * directories or they can be fake inodes describing some attribute.
 *
 * If NInoMstProtected(), perform the post read mst fixups when all IO on the
 * page has been completed and mark the page uptodate or set the error bit on
 * the page.  To determine the size of the records that need fixing up, we
 * cheat a little bit by setting the index_block_size in ntfs_inode to the ntfs
 * record size, and index_block_size_bits, to the log(base 2) of the ntfs
 * record size.
 */
static void ntfs_end_buffer_async_read(struct buffer_head *bh, int uptodate)
{
	unsigned long flags;
	struct buffer_head *first, *tmp;
	struct page *page;
	struct inode *vi;
	ntfs_inode *ni;
	int page_uptodate = 1;

	//printk("enter ntfs_end_buffer...\n");
	page = bh->b_page;
	vi = page->mapping->host;
	ni = NTFS_I(vi);

	if (likely(uptodate)) {
		loff_t i_size;
		s64 file_ofs, init_size;

		set_buffer_uptodate(bh);

		file_ofs = ((s64)page->index << PAGE_CACHE_SHIFT) +		//当前文件指针，指向文件的当前位置
				bh_offset(bh);
		read_lock_irqsave(&ni->size_lock, flags);
		init_size = ni->initialized_size;
		i_size = i_size_read(vi);
		read_unlock_irqrestore(&ni->size_lock, flags);
		if (unlikely(init_size > i_size)) {
			/* Race with shrinking truncate. */
			init_size = i_size;
		}
		//printk("2 bh b_blocknr ==%lld,b_size==%d,file_ops ==%lld \n",bh->b_blocknr,bh->b_size,file_ofs);

		/* Check for the current buffer head overflowing. */
		if (unlikely(file_ofs + bh->b_size > init_size)) {
			int ofs;
			void *kaddr;

			ofs = 0;
			if (file_ofs < init_size)
				ofs = init_size - file_ofs;
			local_irq_save(flags);
			kaddr = kmap_atomic(page, KM_BIO_SRC_IRQ);
			memset(kaddr + bh_offset(bh) + ofs, 0,
					bh->b_size - ofs);
			flush_dcache_page(page);
			kunmap_atomic(kaddr, KM_BIO_SRC_IRQ);
			local_irq_restore(flags);
		}
	} else {
		clear_buffer_uptodate(bh);
		SetPageError(page);
		ntfs_error(ni->vol->sb, "Buffer I/O error, logical block "
				"0x%llx.", (unsigned long long)bh->b_blocknr);
	}
	first = page_buffers(page);
	local_irq_save(flags);
	bit_spin_lock(BH_Uptodate_Lock, &first->b_state);
	clear_buffer_async_read(bh);
	unlock_buffer(bh);
	tmp = bh;	
	
	do {		
		//printk("enter do_while \n");
		if (!buffer_uptodate(tmp))
			page_uptodate = 0;
		if (buffer_async_read(tmp)) {
			if (likely(buffer_locked(tmp)))//被锁住了
				goto still_busy;
			/* Async buffers must be locked. */
			BUG();
		}	
		//printk("tmp size :%d ,state :%ld--",tmp->b_size,tmp->b_state);
		tmp = tmp->b_this_page;
		//printk("tmp next size :%d ,state :%ld\n",tmp->b_size,tmp->b_state);
		
		if(bh->b_size == 4096){
			clear_buffer_async_read(tmp);
			unlock_buffer(tmp);	
			set_buffer_uptodate(tmp);
		}
		
	} while (tmp != bh);
	
	bit_spin_unlock(BH_Uptodate_Lock, &first->b_state);
	local_irq_restore(flags);
	/*
	 * If none of the buffers had errors then we can set the page uptodate,
	 * but we first have to perform the post read mst fixups, if the
	 * attribute is mst protected, i.e. if NInoMstProteced(ni) is true.
	 * Note we ignore fixup errors as those are detected when
	 * map_mft_record() is called which gives us per record granularity
	 * rather than per page granularity.
	 */
	if (!NInoMstProtected(ni)) {	
		if (likely(page_uptodate && !PageError(page))){
			SetPageUptodate(page);
			//printk("setpage up todate 1 \n");
		}
	} else {
		u8 *kaddr;
		unsigned int i, recs;
		u32 rec_size;

		rec_size = ni->itype.index.block_size;//MFT记录项的大小=1024
		recs = PAGE_CACHE_SIZE / rec_size;
		//printk("recs == %d \n",recs);
		/* Should have been verified before we got here... */
		DEBUG0_ON(!recs);
		local_irq_save(flags);
		kaddr = kmap_atomic(page, KM_BIO_SRC_IRQ);
		for (i = 0; i < recs; i++)
			post_read_mst_fixup((NTFS_RECORD*)(kaddr +
					i * rec_size), rec_size);
		kunmap_atomic(kaddr, KM_BIO_SRC_IRQ);
		local_irq_restore(flags);
		flush_dcache_page(page);
		if (likely(page_uptodate && !PageError(page))){			
			SetPageUptodate(page);
			//printk("setpage up todate 2 \n");
		}
	}
	//printk("unlock page \n");
	unlock_page(page);
	return;
still_busy:
	//printk("still busy \n");
	bit_spin_unlock(BH_Uptodate_Lock, &first->b_state);
	local_irq_restore(flags);
	return;
}

/**
 * ntfs_read_block - fill a @page of an address space with data
 * @page:	page cache page to fill with data
 *
 * Fill the page @page of the address space belonging to the @page->host inode.
 * We read each buffer asynchronously and when all buffers are read in, our io
 * completion handler ntfs_end_buffer_read_async(), if required, automatically
 * applies the mst fixups to the page before finally marking it uptodate and
 * unlocking it.
 *
 * We only enforce allocated_size limit because i_size is checked for in
 * generic_file_read().
 *
 * Return 0 on success and -errno on error.
 *
 * Contains an adapted version of fs/buffer.c::block_read_full_page().
 */
static int ntfs_read_block(struct page *page)
{
	loff_t i_size;
	VCN vcn;
	LCN lcn;
	s64 init_size;
	struct inode *vi;
	ntfs_inode *ni;
	ntfs_volume *vol;
	runlist_element *rl;
	struct buffer_head *bh, *head, *arr[MAX_BUF_PER_PAGE];
	sector_t iblock, lblock, zblock;
	unsigned long flags;
	unsigned int blocksize, vcn_ofs;
	int i, nr;
	unsigned char blocksize_bits;

	vi = page->mapping->host;
	ni = NTFS_I(vi);
	vol = ni->vol;

	/* $MFT/$DATA must have its complete runlist in memory at all times. */
	DEBUG0_ON(!ni->runlist.rl && !ni->mft_no && !NInoAttr(ni));

	blocksize = vol->sb->s_blocksize;//块大小==512
	blocksize_bits = vol->sb->s_blocksize_bits;//块大小位数==9

	if (!page_has_buffers(page)) {
		create_empty_buffers(page, blocksize, 0);
		if (unlikely(!page_has_buffers(page))) {
			unlock_page(page);
			return -ENOMEM;
		}
	}
	bh = head = page_buffers(page);//块缓冲区描述符的首部
	DEBUG0_ON(!bh);

	/*
	 * We may be racing with truncate.  To avoid some of the problems we
	 * now take a snapshot of the various sizes and use those for the whole
	 * of the function.  In case of an extending truncate it just means we
	 * may leave some buffers unmapped which are now allocated.  This is
	 * not a problem since these buffers will just get mapped when a write
	 * occurs.  In case of a shrinking truncate, we will detect this later
	 * on due to the runlist being incomplete and if the page is being
	 * fully truncated, truncate will throw it away as soon as we unlock
	 * it so no need to worry what we do with it.
	 */
	ntfs_trace("ni->mft_no :0x%lx, page index = %ld",ni->mft_no,page->index);
	iblock = (s64)page->index << (PAGE_CACHE_SHIFT - blocksize_bits);//页中的起始块号(0->8->16->24)
	read_lock_irqsave(&ni->size_lock, flags);
	lblock = (ni->allocated_size + blocksize - 1) >> blocksize_bits;//属性分配大小所占的总block数
	init_size = ni->initialized_size;//属性初始化的大小
	i_size = i_size_read(vi);//ntfs属性大小
	
	read_unlock_irqrestore(&ni->size_lock, flags);
	if (unlikely(init_size > i_size)) {
		/* Race with shrinking truncate. */
		init_size = i_size;
	}
	zblock = (init_size + blocksize - 1) >> blocksize_bits;//属性初始化大小所占的总blocks
	//printk("iblock==%lld ,lblock ==%lld, init_size ==%lld ,i_size ==%lld ,zblock==%lld\n",iblock,lblock,init_size,i_size,zblock);

	/* Loop through all the buffers in the page. */
	rl = NULL;
	nr = i = 0;
	do {
		int err = 0;

		if (unlikely(buffer_uptodate(bh)))//已经是最新的了，不需要提交，继续下一个
			continue;
		if (unlikely(buffer_mapped(bh))) {//缓冲区没有被映射
			arr[nr++] = bh;
			continue;
		}
		

		bh->b_bdev = vol->sb->s_bdev;
		/* Is the block within the allowed limits? */
		if (iblock < lblock) {
			bool is_retry = false;

			/* Convert iblock into corresponding vcn and offset. */
			vcn = (VCN)iblock << blocksize_bits >>
					vol->cluster_size_bits;
			vcn_ofs = ((VCN)iblock << blocksize_bits) &
					vol->cluster_size_mask;	
			
			//ntfs_debug("i==%d ,iblock ==%lld ,vcn==0x%llx\n",i,iblock,vcn);

			if (!rl) {
lock_retry_remap:
			 if(ni->mft_no == 0)
				ntfs_debug("down_read");
				down_read(&ni->runlist.lock);
				rl = ni->runlist.rl;
			}

			if (likely(rl != NULL)) {
				/* Seek to element containing target vcn. */
				while (rl->length && rl[1].vcn <= vcn){
					//ntfs_debug("rl->length ==0x%llx,rl[1].vcn = =0x%llx ",rl->length,rl[1].vcn);
					rl++;
				}
				lcn = ntfs_rl_vcn_to_lcn(rl, vcn);
			} else
				lcn = LCN_RL_NOT_MAPPED;

			//ntfs_debug("lcn ==0x%llx ",lcn);

			/* Successful remap. */
			if (lcn >= 0) {
				/* Setup buffer head to correct block. */
				bh->b_blocknr = ((lcn << vol->cluster_size_bits)//将簇号转为物理扇区号
						+ vcn_ofs) >> blocksize_bits;
				set_buffer_mapped(bh);
				//printk("bh->b_blocknr ==%lld ",bh->b_blocknr);
				//printk("vcn ==%lld ,vcn_ofs ==%d,lcn ==%lld ",vcn,vcn_ofs,lcn);
				//printk("i==%d ,iblock ==%lld \n",i,iblock);
				/* Only read initialized data blocks. */
				if (iblock < zblock) {
					arr[nr++] = bh;//提交的bh保存到arr数组中
					continue;
				}
				/* Fully non-initialized data block, zero it. */
				goto handle_zblock;
			}

			/* It is a hole, need to zero it. */
			if (lcn == LCN_HOLE)
				goto handle_hole;
			/* If first try and runlist unmapped, map and retry. */
			if (!is_retry && lcn == LCN_RL_NOT_MAPPED) {
				is_retry = true;
				/*
				 * Attempt to map runlist, dropping lock for
				 * the duration.
				 */
				 if(ni->mft_no == 0)
					 ntfs_debug("up_read");
				up_read(&ni->runlist.lock);
				err = ntfs_map_runlist(ni, vcn);
				if (likely(!err))
					goto lock_retry_remap;
				rl = NULL;
			} else if (!rl){
				 if(ni->mft_no == 0)
					 ntfs_debug("up_read");
				up_read(&ni->runlist.lock);
				}
			/*
			 * If buffer is outside the runlist, treat it as a
			 * hole.  This can happen due to concurrent truncate
			 * for example.
			 */
			if (err == -ENOENT || lcn == LCN_ENOENT) {
				err = 0;
				goto handle_hole;
			}
			/* Hard error, zero out region. */
			if (!err)
				err = -EIO;
			bh->b_blocknr = -1;
			SetPageError(page);
			ntfs_error(vol->sb, "Failed to read from inode 0x%lx, "
					"attribute type 0x%x, vcn 0x%llx, "
					"offset 0x%x because its location on "
					"disk could not be determined%s "
					"(error code %i).", ni->mft_no,
					ni->type, (unsigned long long)vcn,
					vcn_ofs, is_retry ? " even after "
					"retrying" : "", err);
		}
		/*
		 * Either iblock was outside lblock limits or
		 * ntfs_rl_vcn_to_lcn() returned error.  Just zero that portion
		 * of the page and set the buffer uptodate.
		 */
handle_hole:
		bh->b_blocknr = -1UL;
		clear_buffer_mapped(bh);
handle_zblock:
		zero_user(page, i * blocksize, blocksize);
		if (likely(!err))
			set_buffer_uptodate(bh);
	} while (i++, iblock++, (bh = bh->b_this_page) != head);

	//ntfs_debug(" ");
	
	/* Release the lock if we took it. */
	if (rl){
		 if(ni->mft_no == 0)
			 ntfs_debug("up_read");
		up_read(&ni->runlist.lock);
		}

	/* Check we have at least one buffer ready for i/o. */
	if (nr) {
		struct buffer_head *tbh;

		/* Lock the buffers. */
		for (i = 0; i < nr; i++) {
			tbh = arr[i];
			lock_buffer(tbh);//对每一个提交的bh进行锁定
			tbh->b_end_io = ntfs_end_buffer_async_read;//ntfs_end_bio_async_read;
			set_buffer_async_read(tbh);
		}
		
		/* Finally, start i/o on the buffers. */
		/*if(check_continue(arr,nr)){			
			//printk("continue :1,nr :%d ",nr);
			tbh =arr[0];
			//printk(" blcoknr:%lld\n",tbh->b_blocknr);
			tbh->b_size = 4096;
			if (likely(!buffer_uptodate(tbh)))					
				submit_bh_queue(READ, tbh,0,0,0);			
			else				
				ntfs_end_bio_async_read(tbh, 1);	//已经更新了
				
			return 0;
		}

		printk(" continue :0 \n");	*/
		
		for (i = 0; i < nr; i++) {
			tbh = arr[i];			
			if (likely(!buffer_uptodate(tbh)))					
				submit_bh(READ, tbh);			
			else 
				ntfs_end_buffer_async_read(tbh, 1);		
		}
		//printk("i==%d, b_blocknr ==%lld,b_size==%d\n",i,tbh->b_blocknr,tbh->b_size);

		return 0;
	}
	/* No i/o was scheduled on any of the buffers. */
	if (likely(!PageError(page)))
		SetPageUptodate(page);
	else /* Signal synchronous i/o error. */
		nr = -EIO;
	unlock_page(page);
	ntfs_debug(" ");

	return nr;
}

/**
 * ntfs_readpage - fill a @page of a @file with data from the device
 * @file:	open file to which the page @page belongs or NULL
 * @page:	page cache page to fill with data
 *
 * For non-resident attributes, ntfs_readpage() fills the @page of the open
 * file @file by calling the ntfs version of the generic block_read_full_page()
 * function, ntfs_read_block(), which in turn creates and reads in the buffers
 * associated with the page asynchronously.
 *
 * For resident attributes, OTOH, ntfs_readpage() fills @page by copying the
 * data from the mft record (which at this stage is most likely in memory) and
 * fills the remainder with zeroes. Thus, in this case, I/O is synchronous, as
 * even if the mft record is not cached at this point in time, we need to wait
 * for it to be read in before we can do the copy.
 *
 * Return 0 on success and -errno on error.
 */
static int ntfs_readpage(struct file *file, struct page *page)
{
	loff_t i_size;
	struct inode *vi;
	ntfs_inode *ni, *base_ni;
	u8 *addr;
	ntfs_attr_search_ctx *ctx;
	MFT_RECORD *mrec;
	unsigned long flags;
	u32 attr_len;
	int err = 0;
	int result = 0;

	ntfs_trace("Entering");
retry_readpage:
	DEBUG0_ON(!PageLocked(page));
	vi = page->mapping->host;
	i_size = i_size_read(vi);

	//printk("\n i_size ==%lld \n",i_size);
	
	/* Is the page fully outside i_size? (truncate in progress) */
	if (unlikely(page->index >= (i_size + PAGE_CACHE_SIZE - 1) >>
			PAGE_CACHE_SHIFT)) {
		zero_user(page, 0, PAGE_CACHE_SIZE);
		ntfs_debug("Read outside i_size - truncated?");
		goto done;
	}
	/*
	 * This can potentially happen because we clear PageUptodate() during
	 * ntfs_writepage() of MstProtected() attributes.
	 写等操作时可能会清uptodate
	 */
	if (PageUptodate(page)) {
		unlock_page(page);
		return 0;
	}
	ni = NTFS_I(vi);//inode信息中提取出ntfs_node的信息
	ntfs_trace("ni->type ==0x%x ,ni->name ==%s \n",ni->type,(char *)ni->name);

	//printk("\n\n ntfs_node info: mft_no==%ld, initialized_size==%llu,type ==0x%x\n",ni->mft_no,ni->initialized_size,ni->type);
	/*
	 * Only $DATA attributes can be encrypted and only unnamed $DATA
	 * attributes can be compressed.  Index root can have the flags set but
	 * this means to create compressed/encrypted files, not that the
	 * attribute is compressed/encrypted.  Note we need to check for
	 * AT_INDEX_ALLOCATION since this is the type of both directory and
	 * index inodes.
	 */
	if (ni->type != AT_INDEX_ALLOCATION) {//索引分配(大目录使用)
		/* If attribute is encrypted, deny access, just like NT4. */
		if (NInoEncrypted(ni)) {//只有数据属性才能被加密
			DEBUG0_ON(ni->type != AT_DATA);
			err = -EACCES;
			goto err_out;
		}
		/* Compressed data streams are handled in compress.c. */
		if (NInoNonResident(ni) && NInoCompressed(ni)) {//非驻留且压缩
			DEBUG0_ON(ni->type != AT_DATA);
			DEBUG0_ON(ni->name_len);
			printk("begin ntfs_read_compressed_block \n");
			return ntfs_read_compressed_block(page);
		}
	}
	/* NInoNonResident() == NInoIndexAllocPresent() */
	if (NInoNonResident(ni)) {
		/* Normal, non-resident data stream. *///非驻留
		result =ntfs_read_block(page);
		//printk("result is %d \n",result);
		ntfs_trace("Done\n");			
		return result;
	}
	/*
	 * Attribute is resident, implying it is not compressed or encrypted.
	 * This also means the attribute is smaller than an mft record and
	 * hence smaller than a page, so can simply zero out any pages with
	 * index above 0.  Note the attribute can actually be marked compressed
	 * but if it is resident the actual data is not compressed so we are
	 * ok to ignore the compressed flag here.
	 属性是驻留的，表明没被压缩或者加密。
	 也就是说属性比一个MTF记录小，当然也小于1page。
	 因此能简单地将index>0的page清零
	 注意:属性能被标记为压缩的，但是如果是驻留属性，
	 我们将忽视该压缩标志。
	 */
	if (unlikely(page->index > 0)) {//将index>0的page清零
		zero_user(page, 0, PAGE_CACHE_SIZE);
		goto done;
	}
	if (!NInoAttr(ni))
		base_ni = ni;
	else
		base_ni = ni->ext.base_ntfs_ino;
	/* Map, pin, and lock the mft record. */
	mrec = map_mft_record(base_ni);//MTF记录项
	if (IS_ERR(mrec)) {
		err = PTR_ERR(mrec);
		goto err_out;
	}
	/*
	 * If a parallel write made the attribute non-resident, drop the mft
	 * record and retry the readpage.
	 如果有操作使得属性非主驻留了，丢弃MFT记录，重新读
	 */
	if (unlikely(NInoNonResident(ni))) {
		unmap_mft_record(base_ni);
		goto retry_readpage;
	}
	ctx = ntfs_attr_get_search_ctx(base_ni, mrec); // Allocate a new attribute search context, initialize it with @ni and @mrec,
	
	if (unlikely(!ctx)) {
		err = -ENOMEM;
		goto unm_err_out;
	}
	
	
	err = ntfs_attr_lookup(ni->type, ni->name, ni->name_len,//查找驻留属性
			CASE_SENSITIVE, 0, NULL, 0, ctx);
	//printk("ctx->attr.type ==0x%x \n",ctx->attr->type); //$Data =0x80
	
	if (unlikely(err))
		goto put_unm_err_out;
	attr_len = le32_to_cpu(ctx->attr->data.resident.value_length);
	
	
	read_lock_irqsave(&ni->size_lock, flags);
	if (unlikely(attr_len > ni->initialized_size))
		attr_len = ni->initialized_size;
	i_size = i_size_read(vi);
	//printk("attr_len ==%d,i_size ==%lld \n",attr_len,i_size);//实际数据的长度
	
	read_unlock_irqrestore(&ni->size_lock, flags);
	if (unlikely(attr_len > i_size)) {
		/* Race with shrinking truncate. */
		attr_len = i_size;
	}
	addr = kmap_atomic(page, KM_USER0);//建立临时映射
	/* Copy the data to the page. */
	memcpy(addr, (u8*)ctx->attr +//将驻留属性的值拷贝到page
			le16_to_cpu(ctx->attr->data.resident.value_offset),
			attr_len);
	/* Zero the remainder of the page. */
	memset(addr + attr_len, 0, PAGE_CACHE_SIZE - attr_len);
	flush_dcache_page(page);
	kunmap_atomic(addr, KM_USER0);
put_unm_err_out:
	ntfs_attr_put_search_ctx(ctx);
unm_err_out:
	unmap_mft_record(base_ni);
done:
	SetPageUptodate(page);
err_out:
	unlock_page(page);
	ntfs_debug("Done\n");
	return err;
}

#define list_to_page(head) (list_entry((head)->prev, struct page, lru))

static int ntfs_readpages(struct file *file, struct address_space *mapping,
			 struct list_head *pages, unsigned nr_pages)
{
	struct inode *vi;
	ntfs_inode *ni;

	//printk("ntfs_readpages ,nr_pages=%d \n",nr_pages);

	vi = mapping->host;
	ni = NTFS_I(vi);//inode信息中提取出ntfs_node的信息
	ntfs_trace("ni->type ==0x%x ,ni->name ==%s ",ni->type,(char *)ni->name);
	if (NInoNonResident(ni)){
		//printk("enter read nonresident data \n");
		/* Normal, non-resident data stream. *///非驻留
		return mpage_readpages(mapping, pages, nr_pages, ntfs_get_block);
	}
	else{
		ntfs_debug("enter ntfs_readpages-->readpage ");
		struct page *page = list_to_page(pages);
		list_del(&page->lru);
		if (!add_to_page_cache_lru(page, mapping,
					page->index, GFP_KERNEL)) {
			ntfs_readpage(file, page);
		}
		page_cache_release(page);
		}
	return 0;
}

static ssize_t ntfs_direct_IO(int rw, struct kiocb *iocb,
			     const struct iovec *iov,
			     loff_t offset, unsigned long nr_segs)
{
	struct file *file = iocb->ki_filp;
	struct inode *inode = file->f_mapping->host;
	ntfs_inode *ni;

	if (rw == WRITE) 		
		return 0;

	/*
	 * FAT need to use the DIO_LOCKING for avoiding the race
	 * condition of fat_get_block() and ->truncate().
	 */
	// printk("dio\n");
	ni = NTFS_I(inode);//inode信息中提取出ntfs_node的信息
	if (NInoNonResident(ni)){
		return blockdev_direct_IO(rw, iocb, inode, inode->i_sb->s_bdev, iov,
				  offset, nr_segs, ntfs_get_block, NULL);
		}
	else{
		ntfs_debug("sorry ,donot support yet \n");
		return 0;
		}
}


#ifdef NTFS_RW

/**
 * ntfs_write_block - write a @page to the backing store
 * @page:	page cache page to write out
 * @wbc:	writeback control structure
 *
 * This function is for writing pages belonging to non-resident, non-mst
 * protected attributes to their backing store.
 *只支持非驻留的，非MST保护属性的数据写
 * For a page with buffers, map and write the dirty buffers asynchronously
 * under page writeback. For a page without buffers, create buffers for the
 * page, then proceed as above.
 *对于有buffer的page，在页写回的时候将脏buffer异步写回；
 对于没有buffer的page，创建buffers，再按上面的步骤处理。
 
 * If a page doesn't have buffers the page dirty state is definitive. If a page
 * does have buffers, the page dirty state is just a hint, and the buffer dirty
 * state is definitive. (A hint which has rules: dirty buffers against a clean
 * page is illegal. Other combinations are legal and need to be handled. In
 * particular a dirty page containing clean buffers for example.)
 *脏buffer_head+ clean page是非法的，其他组合都是合法的。
 * Return 0 on success and -errno on error.
 *
 * Based on ntfs_read_block() and __block_write_full_page().
 */
static int ntfs_write_block(struct page *page, struct writeback_control *wbc)
{
	VCN vcn;
	LCN lcn;
	s64 initialized_size;
	loff_t i_size;
	sector_t block, dblock, iblock;
	struct inode *vi;
	ntfs_inode *ni;
	ntfs_volume *vol;
	runlist_element *rl;
	struct buffer_head *bh, *head;
	unsigned long flags;
	unsigned int blocksize, vcn_ofs;
	int err;
	bool need_end_writeback;
	unsigned char blocksize_bits;

	vi = page->mapping->host;
	ni = NTFS_I(vi);
	vol = ni->vol;

	ntfs_trace("Entering for inode 0x%lx, attribute type 0x%x, page index "
			"0x%lx.", ni->mft_no, ni->type, page->index);

	DEBUG0_ON(!NInoNonResident(ni));
	DEBUG0_ON(NInoMstProtected(ni));
	blocksize = vol->sb->s_blocksize;
	blocksize_bits = vol->sb->s_blocksize_bits;
	if (!page_has_buffers(page)) {
		DEBUG0_ON(!PageUptodate(page));
		create_empty_buffers(page, blocksize,
				(1 << BH_Uptodate) | (1 << BH_Dirty));
		if (unlikely(!page_has_buffers(page))) {	//给page创建buffers
			ntfs_warning(vol->sb, "Error allocating page "
					"buffers.  Redirtying page so we try "
					"again later.");
			/*
			 * Put the page back on mapping->dirty_pages, but leave
			 * its buffers' dirty state as-is.
			 */
			redirty_page_for_writepage(wbc, page);	//创建失败，解锁但保持其dirty
			unlock_page(page);
			return 0;
		}
	}
	bh = head = page_buffers(page);
	DEBUG0_ON(!bh);

	/* NOTE: Different naming scheme to ntfs_read_block()! */

	/* The first block in the page. */
	block = (s64)page->index << (PAGE_CACHE_SHIFT - blocksize_bits);

	read_lock_irqsave(&ni->size_lock, flags);
	i_size = i_size_read(vi);
	initialized_size = ni->initialized_size;
	read_unlock_irqrestore(&ni->size_lock, flags);

	/* The first out of bounds block for the data size. */
	dblock = (i_size + blocksize - 1) >> blocksize_bits;

	/* The last (fully or partially) initialized block. */
	iblock = initialized_size >> blocksize_bits;

	/*
	 * Be very careful.  We have no exclusion from __set_page_dirty_buffers
	 * here, and the (potentially unmapped) buffers may become dirty at
	 * any time.  If a buffer becomes dirty here after we've inspected it
	 * then we just miss that fact, and the page stays dirty.
	 *
	 * Buffers outside i_size may be dirtied by __set_page_dirty_buffers;
	 * handle that here by just cleaning them.
	 */

	/*
	 * Loop through all the buffers in the page, mapping all the dirty
	 * buffers to disk addresses and handling any aliases from the
	 * underlying block device's mapping.
	 浏览page中的所有buffers，将脏的buffers映射到磁盘地址并处理。
	 */
	rl = NULL;
	err = 0;
	do {
		bool is_retry = false;

		if (unlikely(block >= dblock)) {
			/*
			 * Mapped buffers outside i_size will occur, because
			 * this page can be outside i_size when there is a
			 * truncate in progress. The contents of such buffers
			 * were zeroed by ntfs_writepage().
			 *
			 * FIXME: What about the small race window where
			 * ntfs_writepage() has not done any clearing because
			 * the page was within i_size but before we get here,
			 * vmtruncate() modifies i_size?
			 */
			clear_buffer_dirty(bh);
			set_buffer_uptodate(bh);
			continue;
		}

		/* Clean buffers are not written out, so no need to map them. */
		if (!buffer_dirty(bh))
			continue;

		/* Make sure we have enough initialized size. */
		if (unlikely((block >= iblock) &&
				(initialized_size < i_size))) {
			/*
			 * If this page is fully outside initialized size, zero
			 * out all pages between the current initialized size
			 * and the current page. Just use ntfs_readpage() to do
			 * the zeroing transparently.
			 */
			if (block > iblock) {
				// TODO:
				// For each page do:
				// - read_cache_page()
				// Again for each page do:
				// - wait_on_page_locked()
				// - Check (PageUptodate(page) &&
				//			!PageError(page))
				// Update initialized size in the attribute and
				// in the inode.
				// Again, for each page do:
				//	__set_page_dirty_buffers();
				// page_cache_release()
				// We don't need to wait on the writes.
				// Update iblock.
			}
			/*
			 * The current page straddles initialized size. Zero
			 * all non-uptodate buffers and set them uptodate (and
			 * dirty?). Note, there aren't any non-uptodate buffers
			 * if the page is uptodate.
			 * FIXME: For an uptodate page, the buffers may need to
			 * be written out because they were not initialized on
			 * disk before.
			 */
			if (!PageUptodate(page)) {
				// TODO:
				// Zero any non-uptodate buffers up to i_size.
				// Set them uptodate and dirty.
			}
			// TODO:
			// Update initialized size in the attribute and in the
			// inode (up to i_size).
			// Update iblock.
			// FIXME: This is inefficient. Try to batch the two
			// size changes to happen in one go.
			ntfs_error(vol->sb, "Writing beyond initialized size "
					"is not supported yet. Sorry.");
			err = -EOPNOTSUPP;
			break;
			// Do NOT set_buffer_new() BUT DO clear buffer range
			// outside write request range.
			// set_buffer_uptodate() on complete buffers as well as
			// set_buffer_dirty().
		}

		/* No need to map buffers that are already mapped. */
		if (buffer_mapped(bh))
			continue;

		/* Unmapped, dirty buffer. Need to map it. */
		bh->b_bdev = vol->sb->s_bdev;

		/* Convert block into corresponding vcn and offset. */
		vcn = (VCN)block << blocksize_bits;
		vcn_ofs = vcn & vol->cluster_size_mask;
		vcn >>= vol->cluster_size_bits;
		if (!rl) {
lock_retry_remap:
			 if(ni->mft_no == 0)
				 ntfs_debug("down_read");
			down_read(&ni->runlist.lock);
			rl = ni->runlist.rl;
		}
		if (likely(rl != NULL)) {
			/* Seek to element containing target vcn. */
			while (rl->length && rl[1].vcn <= vcn)
				rl++;
			lcn = ntfs_rl_vcn_to_lcn(rl, vcn);
		} else
			lcn = LCN_RL_NOT_MAPPED;
		/* Successful remap. */
		if (lcn >= 0) {
			/* Setup buffer head to point to correct block. */
			bh->b_blocknr = ((lcn << vol->cluster_size_bits) +
					vcn_ofs) >> blocksize_bits;
			set_buffer_mapped(bh);
			continue;
		}
		/* It is a hole, need to instantiate it. */
		if (lcn == LCN_HOLE) {
			u8 *kaddr;
			unsigned long *bpos, *bend;

			/* Check if the buffer is zero. */
			kaddr = kmap_atomic(page, KM_USER0);
			bpos = (unsigned long *)(kaddr + bh_offset(bh));
			bend = (unsigned long *)((u8*)bpos + blocksize);
			do {
				if (unlikely(*bpos))
					break;
			} while (likely(++bpos < bend));
			kunmap_atomic(kaddr, KM_USER0);
			if (bpos == bend) {
				/*
				 * Buffer is zero and sparse, no need to write
				 * it.
				 */
				bh->b_blocknr = -1;
				clear_buffer_dirty(bh);
				continue;
			}
			// TODO: Instantiate the hole.
			// clear_buffer_new(bh);
			// unmap_underlying_metadata(bh->b_bdev, bh->b_blocknr);
			ntfs_error(vol->sb, "Writing into sparse regions is "
					"not supported yet. Sorry.");
			err = -EOPNOTSUPP;
			break;
		}
		/* If first try and runlist unmapped, map and retry. */
		if (!is_retry && lcn == LCN_RL_NOT_MAPPED) {
			is_retry = true;
			/*
			 * Attempt to map runlist, dropping lock for
			 * the duration.
			 */
			 if(ni->mft_no == 0)
				 ntfs_debug("up_read");
			up_read(&ni->runlist.lock);
			err = ntfs_map_runlist(ni, vcn);
			if (likely(!err))
				goto lock_retry_remap;
			rl = NULL;
		} else if (!rl){
			 if(ni->mft_no == 0)
				 ntfs_debug("up_read");
			up_read(&ni->runlist.lock);
			}
		/*
		 * If buffer is outside the runlist, truncate has cut it out
		 * of the runlist.  Just clean and clear the buffer and set it
		 * uptodate so it can get discarded by the VM.
		 */
		if (err == -ENOENT || lcn == LCN_ENOENT) {
			bh->b_blocknr = -1;
			clear_buffer_dirty(bh);
			zero_user(page, bh_offset(bh), blocksize);
			set_buffer_uptodate(bh);
			err = 0;
			continue;
		}
		/* Failed to map the buffer, even after retrying. */
		if (!err)
			err = -EIO;
		bh->b_blocknr = -1;
		ntfs_error(vol->sb, "Failed to write to inode 0x%lx, "
				"attribute type 0x%x, vcn 0x%llx, offset 0x%x "
				"because its location on disk could not be "
				"determined%s (error code %i).", ni->mft_no,
				ni->type, (unsigned long long)vcn,
				vcn_ofs, is_retry ? " even after "
				"retrying" : "", err);
		break;
	} while (block++, (bh = bh->b_this_page) != head);

	/* Release the lock if we took it. */
	if (rl){
		 if(ni->mft_no == 0)
			 ntfs_debug("up_read");
		up_read(&ni->runlist.lock);
		}

	/* For the error case, need to reset bh to the beginning. */
	bh = head;

	/* Just an optimization, so ->readpage() is not called later. */
	if (unlikely(!PageUptodate(page))) {
		int uptodate = 1;
		do {
			if (!buffer_uptodate(bh)) {
				uptodate = 0;
				bh = head;
				break;
			}
		} while ((bh = bh->b_this_page) != head);
		if (uptodate)
			SetPageUptodate(page);
	}

	/* Setup all mapped, dirty buffers for async write i/o. */
	do {
		if (buffer_mapped(bh) && buffer_dirty(bh)) {
			lock_buffer(bh);
			if (test_clear_buffer_dirty(bh)) {
				DEBUG0_ON(!buffer_uptodate(bh));
				mark_buffer_async_write(bh);
			} else
				unlock_buffer(bh);
		} else if (unlikely(err)) {
			/*
			 * For the error case. The buffer may have been set
			 * dirty during attachment to a dirty page.
			 */
			if (err != -ENOMEM)
				clear_buffer_dirty(bh);
		}
	} while ((bh = bh->b_this_page) != head);

	if (unlikely(err)) {
		// TODO: Remove the -EOPNOTSUPP check later on...
		if (unlikely(err == -EOPNOTSUPP))
			err = 0;
		else if (err == -ENOMEM) {
			ntfs_warning(vol->sb, "Error allocating memory. "
					"Redirtying page so we try again "
					"later.");
			/*
			 * Put the page back on mapping->dirty_pages, but
			 * leave its buffer's dirty state as-is.
			 */
			redirty_page_for_writepage(wbc, page);
			err = 0;
		} else
			SetPageError(page);
	}

	DEBUG0_ON(PageWriteback(page));
	set_page_writeback(page);	/* Keeps try_to_free_buffers() away. */

	/* Submit the prepared buffers for i/o. */
	need_end_writeback = true;
	do {
		struct buffer_head *next = bh->b_this_page;
		if (buffer_async_write(bh)) {
			submit_bh(WRITE, bh);
			need_end_writeback = false;
		}
		bh = next;
	} while (bh != head);
	unlock_page(page);

	/* If no i/o was started, need to end_page_writeback(). */
	if (unlikely(need_end_writeback))
		end_page_writeback(page);

	ntfs_trace("Done.");
	return err;
}

/**
 * ntfs_write_mst_block - write a @page to the backing store
 * @page:	page cache page to write out
 * @wbc:	writeback control structure
 *
 * This function is for writing pages belonging to non-resident, mst protected
 * attributes to their backing store.  The only supported attributes are index
 * allocation and $MFT/$DATA.  Both directory inodes and index inodes are
 * supported for the index allocation case.
 *
 * The page must remain locked for the duration of the write because we apply
 * the mst fixups, write, and then undo the fixups, so if we were to unlock the
 * page before undoing the fixups, any other user of the page will see the
 * page contents as corrupt.
 *
 * We clear the page uptodate flag for the duration of the function to ensure
 * exclusion for the $MFT/$DATA case against someone mapping an mft record we
 * are about to apply the mst fixups to.
 *
 * Return 0 on success and -errno on error.
 *
 * Based on ntfs_write_block(), ntfs_mft_writepage(), and
 * write_mft_record_nolock().
 */
static int ntfs_write_mst_block(struct page *page,
		struct writeback_control *wbc)
{
	sector_t block, dblock, rec_block;
	struct inode *vi = page->mapping->host;
	ntfs_inode *ni = NTFS_I(vi);
	ntfs_volume *vol = ni->vol;
	u8 *kaddr;
	unsigned int rec_size = ni->itype.index.block_size;
	ntfs_inode *locked_nis[PAGE_CACHE_SIZE / rec_size];
	struct buffer_head *bh, *head, *tbh, *rec_start_bh;
	struct buffer_head *bhs[MAX_BUF_PER_PAGE];
	runlist_element *rl;
	int i, nr_locked_nis, nr_recs, nr_bhs, max_bhs, bhs_per_rec, err, err2;
	unsigned bh_size, rec_size_bits;
	bool sync, is_mft, page_is_dirty, rec_is_dirty;
	unsigned char bh_size_bits;

	ntfs_trace("Entering for inode 0x%lx, attribute type 0x%x, page index "
			"0x%lx.", vi->i_ino, ni->type, page->index);
	DEBUG0_ON(!NInoNonResident(ni));
	DEBUG0_ON(!NInoMstProtected(ni));
	is_mft = (S_ISREG(vi->i_mode) && !vi->i_ino);
	/*
	 * NOTE: ntfs_write_mst_block() would be called for $MFTMirr if a page
	 * in its page cache were to be marked dirty.  However this should
	 * never happen with the current driver and considering we do not
	 * handle this case here we do want to BUG(), at least for now.
	 */
	DEBUG0_ON(!(is_mft || S_ISDIR(vi->i_mode) ||
			(NInoAttr(ni) && ni->type == AT_INDEX_ALLOCATION)));
	bh_size = vol->sb->s_blocksize;
	bh_size_bits = vol->sb->s_blocksize_bits;
	max_bhs = PAGE_CACHE_SIZE / bh_size;
	DEBUG0_ON(!max_bhs);
	DEBUG0_ON(max_bhs > MAX_BUF_PER_PAGE);

	/* Were we called for sync purposes? */
	sync = (wbc->sync_mode == WB_SYNC_ALL);

	/* Make sure we have mapped buffers. */
	bh = head = page_buffers(page);
	DEBUG0_ON(!bh);

	rec_size_bits = ni->itype.index.block_size_bits;
	DEBUG0_ON(!(PAGE_CACHE_SIZE >> rec_size_bits));
	bhs_per_rec = rec_size >> bh_size_bits;
	DEBUG0_ON(!bhs_per_rec);

	/* The first block in the page. */
	rec_block = block = (sector_t)page->index <<
			(PAGE_CACHE_SHIFT - bh_size_bits);

	/* The first out of bounds block for the data size. */
	dblock = (i_size_read(vi) + bh_size - 1) >> bh_size_bits;

	rl = NULL;
	err = err2 = nr_bhs = nr_recs = nr_locked_nis = 0;
	page_is_dirty = rec_is_dirty = false;
	rec_start_bh = NULL;
	do {
		bool is_retry = false;

		if (likely(block < rec_block)) {
			if (unlikely(block >= dblock)) {
				clear_buffer_dirty(bh);
				set_buffer_uptodate(bh);
				continue;
			}
			/*
			 * This block is not the first one in the record.  We
			 * ignore the buffer's dirty state because we could
			 * have raced with a parallel mark_ntfs_record_dirty().
			 */
			if (!rec_is_dirty)
				continue;
			if (unlikely(err2)) {
				if (err2 != -ENOMEM)
					clear_buffer_dirty(bh);
				continue;
			}
		} else /* if (block == rec_block) */ {
			DEBUG0_ON(block > rec_block);
			/* This block is the first one in the record. */
			rec_block += bhs_per_rec;
			err2 = 0;
			if (unlikely(block >= dblock)) {
				clear_buffer_dirty(bh);
				continue;
			}
			if (!buffer_dirty(bh)) {
				/* Clean records are not written out. */
				rec_is_dirty = false;
				continue;
			}
			rec_is_dirty = true;
			rec_start_bh = bh;
		}
		/* Need to map the buffer if it is not mapped already. */
		if (unlikely(!buffer_mapped(bh))) {
			VCN vcn;
			LCN lcn;
			unsigned int vcn_ofs;

			bh->b_bdev = vol->sb->s_bdev;
			/* Obtain the vcn and offset of the current block. */
			vcn = (VCN)block << bh_size_bits;
			vcn_ofs = vcn & vol->cluster_size_mask;
			vcn >>= vol->cluster_size_bits;
			if (!rl) {
lock_retry_remap:
				 if(ni->mft_no == 0)
					 ntfs_debug("down_read");
				down_read(&ni->runlist.lock);
				rl = ni->runlist.rl;
			}
			if (likely(rl != NULL)) {
				/* Seek to element containing target vcn. */
				while (rl->length && rl[1].vcn <= vcn)
					rl++;
				lcn = ntfs_rl_vcn_to_lcn(rl, vcn);
			} else
				lcn = LCN_RL_NOT_MAPPED;
			/* Successful remap. */
			if (likely(lcn >= 0)) {
				/* Setup buffer head to correct block. */
				bh->b_blocknr = ((lcn <<
						vol->cluster_size_bits) +
						vcn_ofs) >> bh_size_bits;
				set_buffer_mapped(bh);
			} else {
				/*
				 * Remap failed.  Retry to map the runlist once
				 * unless we are working on $MFT which always
				 * has the whole of its runlist in memory.
				 */
				if (!is_mft && !is_retry &&
						lcn == LCN_RL_NOT_MAPPED) {
					is_retry = true;
					/*
					 * Attempt to map runlist, dropping
					 * lock for the duration.
					 */
					 if(ni->mft_no == 0)
						 ntfs_debug("up_read");
					up_read(&ni->runlist.lock);
					err2 = ntfs_map_runlist(ni, vcn);
					if (likely(!err2))
						goto lock_retry_remap;
					if (err2 == -ENOMEM)
						page_is_dirty = true;
					lcn = err2;
				} else {
					err2 = -EIO;
					if (!rl){
						 if(ni->mft_no == 0)
							 ntfs_debug("up_read");
						up_read(&ni->runlist.lock);
						}
				}
				/* Hard error.  Abort writing this record. */
				if (!err || err == -ENOMEM)
					err = err2;
				bh->b_blocknr = -1;
				ntfs_error(vol->sb, "Cannot write ntfs record "
						"0x%llx (inode 0x%lx, "
						"attribute type 0x%x) because "
						"its location on disk could "
						"not be determined (error "
						"code %lli).",
						(long long)block <<
						bh_size_bits >>
						vol->mft_record_size_bits,
						ni->mft_no, ni->type,
						(long long)lcn);
				/*
				 * If this is not the first buffer, remove the
				 * buffers in this record from the list of
				 * buffers to write and clear their dirty bit
				 * if not error -ENOMEM.
				 */
				if (rec_start_bh != bh) {
					while (bhs[--nr_bhs] != rec_start_bh)
						;
					if (err2 != -ENOMEM) {
						do {
							clear_buffer_dirty(
								rec_start_bh);
						} while ((rec_start_bh =
								rec_start_bh->
								b_this_page) !=
								bh);
					}
				}
				continue;
			}
		}
		DEBUG0_ON(!buffer_uptodate(bh));
		DEBUG0_ON(nr_bhs >= max_bhs);
		bhs[nr_bhs++] = bh;
	} while (block++, (bh = bh->b_this_page) != head);
	if (unlikely(rl)){
		 if(ni->mft_no == 0)
			 ntfs_debug("up_read");
		up_read(&ni->runlist.lock);
		}
	/* If there were no dirty buffers, we are done. */
	if (!nr_bhs)
		goto done;
	/* Map the page so we can access its contents. */
	kaddr = kmap(page);
	/* Clear the page uptodate flag whilst the mst fixups are applied. */
	DEBUG0_ON(!PageUptodate(page));
	ClearPageUptodate(page);
	for (i = 0; i < nr_bhs; i++) {
		unsigned int ofs;

		/* Skip buffers which are not at the beginning of records. */
		if (i % bhs_per_rec)
			continue;
		tbh = bhs[i];
		ofs = bh_offset(tbh);
		if (is_mft) {
			ntfs_inode *tni;
			unsigned long mft_no;

			/* Get the mft record number. */
			mft_no = (((s64)page->index << PAGE_CACHE_SHIFT) + ofs)
					>> rec_size_bits;
			/* Check whether to write this mft record. */
			tni = NULL;
			if (!ntfs_may_write_mft_record(vol, mft_no,
					(MFT_RECORD*)(kaddr + ofs), &tni)) {
				/*
				 * The record should not be written.  This
				 * means we need to redirty the page before
				 * returning.
				 */
				page_is_dirty = true;
				/*
				 * Remove the buffers in this mft record from
				 * the list of buffers to write.
				 */
				do {
					bhs[i] = NULL;
				} while (++i % bhs_per_rec);
				continue;
			}
			/*
			 * The record should be written.  If a locked ntfs
			 * inode was returned, add it to the array of locked
			 * ntfs inodes.
			 */
			if (tni)
				locked_nis[nr_locked_nis++] = tni;
		}
		/* Apply the mst protection fixups. */
		err2 = pre_write_mst_fixup((NTFS_RECORD*)(kaddr + ofs),
				rec_size);
		if (unlikely(err2)) {
			if (!err || err == -ENOMEM)
				err = -EIO;
			ntfs_error(vol->sb, "Failed to apply mst fixups "
					"(inode 0x%lx, attribute type 0x%x, "
					"page index 0x%lx, page offset 0x%x)!"
					"  Unmount and run chkdsk.", vi->i_ino,
					ni->type, page->index, ofs);
			/*
			 * Mark all the buffers in this record clean as we do
			 * not want to write corrupt data to disk.
			 */
			do {
				clear_buffer_dirty(bhs[i]);
				bhs[i] = NULL;
			} while (++i % bhs_per_rec);
			continue;
		}
		nr_recs++;
	}
	/* If no records are to be written out, we are done. */
	if (!nr_recs)
		goto unm_done;
	flush_dcache_page(page);
	/* Lock buffers and start synchronous write i/o on them. */
	for (i = 0; i < nr_bhs; i++) {
		tbh = bhs[i];
		if (!tbh)
			continue;
		if (!trylock_buffer(tbh)){
			ntfs_debug(" ");
			BUG();
		}
		/* The buffer dirty state is now irrelevant, just clean it. */
		clear_buffer_dirty(tbh);
		//ntfs_debug(" ");
		DEBUG0_ON(!buffer_uptodate(tbh));
		//ntfs_debug(" ");
		DEBUG0_ON(!buffer_mapped(tbh));
		get_bh(tbh);
		tbh->b_end_io = end_buffer_write_sync;
		submit_bh(WRITE, tbh);
	}
	/* Synchronize the mft mirror now if not @sync. */
	if (is_mft && !sync)
		goto do_mirror;
do_wait:
	/* Wait on i/o completion of buffers. */
	for (i = 0; i < nr_bhs; i++) {
		tbh = bhs[i];
		if (!tbh)
			continue;
		wait_on_buffer(tbh);
		if (unlikely(!buffer_uptodate(tbh))) {
			ntfs_error(vol->sb, "I/O error while writing ntfs "
					"record buffer (inode 0x%lx, "
					"attribute type 0x%x, page index "
					"0x%lx, page offset 0x%lx)!  Unmount "
					"and run chkdsk.", vi->i_ino, ni->type,
					page->index, bh_offset(tbh));
			if (!err || err == -ENOMEM)
				err = -EIO;
			/*
			 * Set the buffer uptodate so the page and buffer
			 * states do not become out of sync.
			 */
			set_buffer_uptodate(tbh);
		}
	}
	/* If @sync, now synchronize the mft mirror. */
	if (is_mft && sync) {
do_mirror:
		for (i = 0; i < nr_bhs; i++) {
			unsigned long mft_no;
			unsigned int ofs;

			/*
			 * Skip buffers which are not at the beginning of
			 * records.
			 */
			if (i % bhs_per_rec)
				continue;
			tbh = bhs[i];
			/* Skip removed buffers (and hence records). */
			if (!tbh)
				continue;
			ofs = bh_offset(tbh);
			/* Get the mft record number. */
			mft_no = (((s64)page->index << PAGE_CACHE_SHIFT) + ofs)
					>> rec_size_bits;
			if (mft_no < vol->mftmirr_size)
				ntfs_sync_mft_mirror(vol, mft_no,
						(MFT_RECORD*)(kaddr + ofs),
						sync);
		}
		if (!sync)
			goto do_wait;
	}
	/* Remove the mst protection fixups again. */
	for (i = 0; i < nr_bhs; i++) {
		if (!(i % bhs_per_rec)) {
			tbh = bhs[i];
			if (!tbh)
				continue;
			post_write_mst_fixup((NTFS_RECORD*)(kaddr +
					bh_offset(tbh)));
		}
	}
	flush_dcache_page(page);
unm_done:
	/* Unlock any locked inodes. */
	while (nr_locked_nis-- > 0) {
		ntfs_inode *tni, *base_tni;
		
		tni = locked_nis[nr_locked_nis];
		/* Get the base inode. */
		mutex_lock(&tni->extent_lock);
		if (tni->nr_extents >= 0)
			base_tni = tni;
		else {
			base_tni = tni->ext.base_ntfs_ino;
			DEBUG0_ON(!base_tni);
		}
		mutex_unlock(&tni->extent_lock);
		ntfs_debug("Unlocking %s inode 0x%lx.",
				tni == base_tni ? "base" : "extent",
				tni->mft_no);
		mutex_unlock(&tni->mrec_lock);
		atomic_dec(&tni->count);
		iput(VFS_I(base_tni));
	}
	SetPageUptodate(page);
	kunmap(page);
done:
	if (unlikely(err && err != -ENOMEM)) {
		/*
		 * Set page error if there is only one ntfs record in the page.
		 * Otherwise we would loose per-record granularity.
		 */
		if (ni->itype.index.block_size == PAGE_CACHE_SIZE)
			SetPageError(page);
		NVolSetErrors(vol);
	}
	if (page_is_dirty) {
		ntfs_debug("Page still contains one or more dirty ntfs "
				"records.  Redirtying the page starting at "
				"record 0x%lx.", page->index <<
				(PAGE_CACHE_SHIFT - rec_size_bits));
		redirty_page_for_writepage(wbc, page);
		unlock_page(page);
	} else {
		/*
		 * Keep the VM happy.  This must be done otherwise the
		 * radix-tree tag PAGECACHE_TAG_DIRTY remains set even though
		 * the page is clean.
		 */
		DEBUG0_ON(PageWriteback(page));
		set_page_writeback(page);
		unlock_page(page);
		end_page_writeback(page);
	}
	if (likely(!err))
		ntfs_trace("Done.");
	return err;
}

/**
 * ntfs_writepage - write a @page to the backing store
 * @page:	page cache page to write out
 * @wbc:	writeback control structure
 *
 * This is called from the VM when it wants to have a dirty ntfs page cache
 * page cleaned.  The VM has already locked the page and marked it clean.
 *
 * For non-resident attributes, ntfs_writepage() writes the @page by calling
 * the ntfs version of the generic block_write_full_page() function,
 * ntfs_write_block(), which in turn if necessary creates and writes the
 * buffers associated with the page asynchronously.
 *对于非驻留属性，调用ntfs_write_block；
 
 * For resident attributes, OTOH, ntfs_writepage() writes the @page by copying
 * the data to the mft record (which at this stage is most likely in memory).
 * The mft record is then marked dirty and written out asynchronously via the
 * vfs inode dirty code path for the inode the mft record belongs to or via the
 * vm page dirty code path for the page the mft record is in.
 *对于驻留属性，先将数据拷贝到mft record，然后将其标记
 为脏，最后通过vfs inode 或vm page的脏写回机制写到磁盘
 *
 Based on ntfs_readpage() and fs/buffer.c::block_write_full_page().
 *
 * Return 0 on success and -errno on error.
 */
static int ntfs_writepage(struct page *page, struct writeback_control *wbc)
{
	loff_t i_size;
	struct inode *vi = page->mapping->host;
	ntfs_inode *base_ni = NULL, *ni = NTFS_I(vi);
	char *addr;
	ntfs_attr_search_ctx *ctx = NULL;
	MFT_RECORD *m = NULL;
	u32 attr_len;
	int err;

	//printk("enter ntfs_writepage \n");
	ntfs_trace("mft_no == 0x%lx ,ni->type ==0x%x ,ni->name ==%s",ni->mft_no,ni->type,(char *)ni->name);
retry_writepage:
	DEBUG0_ON(!PageLocked(page));
	i_size = i_size_read(vi);
	/* Is the page fully outside i_size? (truncate in progress) */
	if (unlikely(page->index >= (i_size + PAGE_CACHE_SIZE - 1) >>
			PAGE_CACHE_SHIFT)) {
		/*
		 * The page may have dirty, unmapped buffers.  Make them
		 * freeable here, so the page does not leak.
		 */
		block_invalidatepage(page, 0);
		unlock_page(page);
		ntfs_debug("Write outside i_size - truncated?");
		return 0;
	}
	/*
	 * Only $DATA attributes can be encrypted and only unnamed $DATA
	 * attributes can be compressed.  Index root can have the flags set but
	 * this means to create compressed/encrypted files, not that the
	 * attribute is compressed/encrypted.  Note we need to check for
	 * AT_INDEX_ALLOCATION since this is the type of both directory and
	 * index inodes.
	 只有$data才能被加密和压缩，Index root如果为加密或压缩，
	 则表示其要被设置为加密或压缩，而不是其本身就是加密或压缩。
	 首先要检测AT_INDEX_ALLOCATION标志，因为directory和index共有
	 */

	/**暂不支持加密、压缩、稀疏文件*/
	if (ni->type != AT_INDEX_ALLOCATION) {
		/* If file is encrypted, deny access, just like NT4. */
		if (NInoEncrypted(ni)) {
			unlock_page(page);
			DEBUG0_ON(ni->type != AT_DATA);
			ntfs_error(vi->i_sb,"Denying write access to encrypted file.");
			return -EACCES;
		}
		/* Compressed data streams are handled in compress.c. */
		if (NInoNonResident(ni) && NInoCompressed(ni)) {
			DEBUG0_ON(ni->type != AT_DATA);
			DEBUG0_ON(ni->name_len);
			// TODO: Implement and replace this with
			// return ntfs_write_compressed_block(page);
			unlock_page(page);
			ntfs_error(vi->i_sb, "Writing to compressed files is "
					"not supported yet.  Sorry.");
			return -EOPNOTSUPP;
		}
		// TODO: Implement and remove this check.
		if (NInoNonResident(ni) && NInoSparse(ni)) {
			unlock_page(page);
			ntfs_error(vi->i_sb, "Writing to sparse files is not "
					"supported yet.  Sorry.");
			return -EOPNOTSUPP;
		}
	}
	/* NInoNonResident() == NInoIndexAllocPresent() */
	if (NInoNonResident(ni)) {//非驻留属性
		/* We have to zero every time due to mmap-at-end-of-file. */
		if (page->index >= (i_size >> PAGE_CACHE_SHIFT)) {//先清零page中多余的部分
			/* The page straddles i_size. */
			unsigned int ofs = i_size & ~PAGE_CACHE_MASK;
			zero_user_segment(page, ofs, PAGE_CACHE_SIZE);
		}
		/* Handle mst protected attributes. */
		if (NInoMstProtected(ni))
			return ntfs_write_mst_block(page, wbc);
		/* Normal, non-resident data stream. 非驻留的数据流*/
		return ntfs_write_block(page, wbc);
	}
	/*
	 * Attribute is resident, implying it is not compressed, encrypted, or
	 * mst protected.  This also means the attribute is smaller than an mft
	 * record and hence smaller than a page, so can simply return error on
	 * any pages with index above 0.  Note the attribute can actually be
	 * marked compressed but if it is resident the actual data is not
	 * compressed so we are ok to ignore the compressed flag here.
	 驻留属性是非压缩/非加密/非mst保护的,
	并且小于一个mft record 1024，当然也小于1 page。
	对于压缩标志也直接忽略。
	 */
	DEBUG0_ON(page_has_buffers(page));
	DEBUG0_ON(!PageUptodate(page));
	if (unlikely(page->index > 0)) {
		ntfs_error(vi->i_sb, "BUG()! page->index (0x%lx) > 0.  "
				"Aborting write.", page->index);
		DEBUG0_ON(PageWriteback(page));
		set_page_writeback(page);	//设置page写回标志PG_writeback =1
		unlock_page(page);
		end_page_writeback(page);
		return -EIO;
	}
	if (!NInoAttr(ni))//判断是否为主节点
		base_ni = ni;//主节点
	else
		base_ni = ni->ext.base_ntfs_ino;//次节点的主节点
	/* Map, pin, and lock the mft record. */
	m = map_mft_record(base_ni);
	if (IS_ERR(m)) {
		err = PTR_ERR(m);
		m = NULL;
		ctx = NULL;
		goto err_out;
	}
	/*
	 * If a parallel write made the attribute non-resident, drop the mft
	 * record and retry the writepage.
	 如果有操作并行地使得属性变为非驻留，
	 则舍弃mft record并返回
	 */
	if (unlikely(NInoNonResident(ni))) {
		unmap_mft_record(base_ni);
		goto retry_writepage;
	}
	ctx = ntfs_attr_get_search_ctx(base_ni, m);
	if (unlikely(!ctx)) {
		err = -ENOMEM;
		goto err_out;
	}
	err = ntfs_attr_lookup(ni->type, ni->name, ni->name_len,//查找待写入的属性
			CASE_SENSITIVE, 0, NULL, 0, ctx);
	if (unlikely(err))
		goto err_out;
	/*
	 * Keep the VM happy.  This must be done otherwise the radix-tree tag
	 * PAGECACHE_TAG_DIRTY remains set even though the page is clean.
	 */
	DEBUG0_ON(PageWriteback(page));
	set_page_writeback(page);
	unlock_page(page);
	attr_len = le32_to_cpu(ctx->attr->data.resident.value_length);//属性长度
	i_size = i_size_read(vi);//inode节点大小
	if (unlikely(attr_len > i_size)) {
		/* Race with shrinking truncate or a failed truncate. */
		attr_len = i_size;
		/*
		 * If the truncate failed, fix it up now.  If a concurrent
		 * truncate, we do its job, so it does not have to do anything.
		 */
		err = ntfs_resident_attr_value_resize(ctx->mrec, ctx->attr,//改变该驻留属性的大小
				attr_len);
		/* Shrinking cannot fail. */
		DEBUG0_ON(err);
	}
	addr = kmap_atomic(page, KM_USER0);//建立临时映射
	/* Copy the data from the page to the mft record. */
	memcpy((u8*)ctx->attr +//将数据写入磁盘上属性的位置
			le16_to_cpu(ctx->attr->data.resident.value_offset),
			addr, attr_len);
	/* Zero out of bounds area in the page cache page. */
	memset(addr + attr_len, 0, PAGE_CACHE_SIZE - attr_len);//将页中的剩余部分清零
	kunmap_atomic(addr, KM_USER0);
	flush_dcache_page(page);
	flush_dcache_mft_record_page(ctx->ntfs_ino);
	/* We are done with the page. */
	end_page_writeback(page);
	/* Finally, mark the mft record dirty, so it gets written back. */
	mark_mft_record_dirty(ctx->ntfs_ino);
	ntfs_attr_put_search_ctx(ctx);
	unmap_mft_record(base_ni);
	ntfs_debug("Done\n");
	return 0;
err_out:
	if (err == -ENOMEM) {
		ntfs_warning(vi->i_sb, "Error allocating memory. Redirtying "
				"page so we try again later.");
		/*
		 * Put the page back on mapping->dirty_pages, but leave its
		 * buffers' dirty state as-is.
		 */
		redirty_page_for_writepage(wbc, page);
		err = 0;
	} else {
		ntfs_error(vi->i_sb, "Resident attribute write failed with "
				"error %i.", err);
		SetPageError(page);
		NVolSetErrors(ni->vol);
	}
	unlock_page(page);
	if (ctx)
		ntfs_attr_put_search_ctx(ctx);
	if (m)
		unmap_mft_record(base_ni);
	return err;
}

#endif	/* NTFS_RW */

/**
 * ntfs_aops - general address space operations for inodes and attributes
 */
const struct address_space_operations ntfs_aops = {
	.readpage	= ntfs_readpage,		/* Fill page with data. */
	.readpages	= ntfs_readpages,	/* add by yangli */
	.sync_page	= block_sync_page,	/* Currently, just unplugs the
						   				disk request queue. */
	.direct_IO	=ntfs_direct_IO,		/* add by yangli */
#ifdef NTFS_RW
	.writepage	= ntfs_writepage,	/* Write dirty page to disk. */
#endif /* NTFS_RW */
	.migratepage	= buffer_migrate_page,	/* Move a page cache page from
						   one physical page to an
						   other. */
};

/**
 * ntfs_mst_aops - general address space operations for mst protecteed inodes
 *		   and attributes
 */
const struct address_space_operations ntfs_mst_aops = {
	.readpage	= ntfs_readpage,	/* Fill page with data. */
	.sync_page	= block_sync_page,	/* Currently, just unplugs the
						   disk request queue. */
#ifdef NTFS_RW
	.writepage	= ntfs_writepage,	/* Write dirty page to disk. */
	.set_page_dirty	= __set_page_dirty_nobuffers,	/* Set the page dirty
						   without touching the buffers
						   belonging to the page. */
#endif /* NTFS_RW */
	.migratepage	= buffer_migrate_page,	/* Move a page cache page from
						   one physical page to an
						   other. */
};

#ifdef NTFS_RW

/**
 * mark_ntfs_record_dirty - mark an ntfs record dirty
 * @page:	page containing the ntfs record to mark dirty
 * @ofs:	byte offset within @page at which the ntfs record begins
 *
 * Set the buffers and the page in which the ntfs record is located dirty.
 *
 * The latter also marks the vfs inode the ntfs record belongs to dirty
 * (I_DIRTY_PAGES only).
 *
 * If the page does not have buffers, we create them and set them uptodate.
 * The page may not be locked which is why we need to handle the buffers under
 * the mapping->private_lock.  Once the buffers are marked dirty we no longer
 * need the lock since try_to_free_buffers() does not free dirty buffers.
 */
void mark_ntfs_record_dirty(struct page *page, const unsigned int ofs) {
	struct address_space *mapping = page->mapping;
	ntfs_inode *ni = NTFS_I(mapping->host);
	struct buffer_head *bh, *head, *buffers_to_free = NULL;
	unsigned int end, bh_size, bh_ofs;

	DEBUG0_ON(!PageUptodate(page));
	end = ofs + ni->itype.index.block_size;
	bh_size = VFS_I(ni)->i_sb->s_blocksize;
	spin_lock(&mapping->private_lock);
	if (unlikely(!page_has_buffers(page))) {
		spin_unlock(&mapping->private_lock);
		bh = head = alloc_page_buffers(page, bh_size, 1);
		spin_lock(&mapping->private_lock);
		if (likely(!page_has_buffers(page))) {
			struct buffer_head *tail;

			do {
				set_buffer_uptodate(bh);
				tail = bh;
				bh = bh->b_this_page;
			} while (bh);
			tail->b_this_page = head;
			attach_page_buffers(page, head);
		} else
			buffers_to_free = bh;
	}
	bh = head = page_buffers(page);
	DEBUG0_ON(!bh);
	do {
		bh_ofs = bh_offset(bh);
		if (bh_ofs + bh_size <= ofs)
			continue;
		if (unlikely(bh_ofs >= end))
			break;
		set_buffer_dirty(bh);
	} while ((bh = bh->b_this_page) != head);
	spin_unlock(&mapping->private_lock);
	__set_page_dirty_nobuffers(page);
	if (unlikely(buffers_to_free)) {
		do {
			bh = buffers_to_free->b_this_page;
			free_buffer_head(buffers_to_free);
			buffers_to_free = bh;
		} while (buffers_to_free);
	}
}

#endif /* NTFS_RW */

