/*
 * index.c - NTFS kernel index handling.  Part of the Linux-NTFS project.
 *
 * Copyright (c) 2004-2005 Anton Altaparmakov
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

#include <linux/math64.h>
#include "aops.h"
#include "collate.h"
#include "debug.h"
#include "index.h"
#include "ntfs.h"
#include "dir.h"
#include "attrib.h"


/**·ÖÅä²¢³õÊ¼»¯Ò»¸öÐÂµÄindex context
 * ntfs_index_ctx_get - allocate and initialize a new index context
 * @idx_ni:	ntfs index inode with which to initialize the context
 *
 * Allocate a new index context, initialize it with @idx_ni and return it.
 * Return NULL if allocation failed.
 *
 * Locking:  Caller must hold i_mutex on the index inode.
 */
ntfs_index_context *ntfs_index_ctx_get(ntfs_inode *idx_ni)
{
	ntfs_index_context *ictx;

	ictx = kmem_cache_alloc(ntfs_index_ctx_cache, GFP_NOFS);
	if (ictx)
		*ictx = (ntfs_index_context){ .idx_ni = idx_ni,
								 .pindex = 0,
							     };
	return ictx;
}

/**
 * ntfs_index_ctx_put - release an index context
 * @ictx:	index context to free
 *
 * Release the index context @ictx, releasing all associated resources.
 *
 * Locking:  Caller must hold i_mutex on the index inode.
 */
void ntfs_index_ctx_put(ntfs_index_context *ictx)
{
	if (ictx->entry) {
		if (ictx->is_in_root) {
			if (ictx->actx)
				ntfs_attr_put_search_ctx(ictx->actx);
			if (ictx->base_ni)
				unmap_mft_record(ictx->base_ni);
		} else {
			struct page *page = ictx->page;
			if (page) {
				DEBUG0_ON(!PageLocked(page));
				unlock_page(page);
				ntfs_unmap_page(page);				
			}
		}
	}
	kmem_cache_free(ntfs_index_ctx_cache, ictx);
	return;
}


/**ÔÚindexÕÒµ½key£¬²¢·µ»ØËüµÄindex Èë¿Ú
 * ntfs_index_lookup - find a key in an index and return its index entry
 * @key:	[IN] key for which to search in the index
 * @key_len:	[IN] length of @key in bytes
 * @ictx:	[IN/OUT] context describing the index and the returned entryÊäÈëÊä³ö
 *
 * Before calling ntfs_index_lookup(), @ictx must have been obtained from a
 * call to ntfs_index_ctx_get().
 *
 * Look for the @key in the index specified by the index lookup context @ictx.
 * ntfs_index_lookup() walks the contents of the index looking for the @key.
 *
 * If the @key is found in the index, 0 is returned and @ictx is setup to
 * describe the index entry containing the matching @key.  @ictx->entry is the
 * index entry and @ictx->data and @ictx->data_len are the index entry data and
 * its length in bytes, respectively.
 Èç¹ûÕÒµ½ÁË£¬·µ»Ø0£¬ictxÃèÊöÁËÓëkeyÆ¥ÅäµÄindex entry
 *
 * If the @key is not found in the index, -ENOENT is returned and @ictx is
 * setup to describe the index entry whose key collates immediately after the
 * search @key, i.e. this is the position in the index at which an index entry
 * with a key of @key would need to be inserted.
 *Èç¹û@keyÃ»ÕÒµ½£¬·µ»Ø-ENOENT£¬@ictxÇ¡ºÃ¾ÍÊÇÒªÕÒ@keyµÄÏÂÒ»¸ö£¬
´ËÎ»ÖÃ¿ÉÒÔ²åÈëindex entry¡£

 * If an error occurs return the negative error code and @ictx is left
 * untouched.
 *
 * When finished with the entry and its data, call ntfs_index_ctx_put() to free
 * the context and other associated resources.
 *
 * If the index entry was modified, call flush_dcache_index_entry_page()
 * immediately after the modification and either ntfs_index_entry_mark_dirty()
 * or ntfs_index_entry_write() before the call to ntfs_index_ctx_put() to
 * ensure that the changes are written to disk.
 *
 * Locking:  - Caller must hold i_mutex on the index inode.
 *	     - Each page cache page in the index allocation mapping must be
 *	       locked whilst being accessed otherwise we may find a corrupt
 *	       page due to it being under ->writepage at the moment which
 *	       applies the mst protection fixups before writing out and then
 *	       removes them again after the write is complete after which it 
 *	       unlocks the page.
 */
int ntfs_index_lookup(const void *key, const int key_len,
		ntfs_index_context *ictx)
{
	VCN vcn, old_vcn;
	ntfs_inode *idx_ni = ictx->idx_ni;
	ntfs_volume *vol = idx_ni->vol;
	struct super_block *sb = vol->sb;
	ntfs_inode *base_ni = idx_ni->ext.base_ntfs_ino;
	MFT_RECORD *m;
	INDEX_ROOT *ir;
	INDEX_ENTRY *ie;
	INDEX_ALLOCATION *ia;
	u8 *index_end, *kaddr;
	ntfs_attr_search_ctx *actx;
	struct address_space *ia_mapping;
	struct page *page;
	int rc, err = 0;

	ntfs_debug("Entering.----------------------------------------------------------");
	ntfs_debug("mft_no =0x%lx, base_no=0x%lx, idx_ni->nr_extents == %d",idx_ni->mft_no,base_ni->mft_no,idx_ni->nr_extents);
	DEBUG0_ON(!NInoAttr(idx_ni));	//²»ÄÜÊÇÖ÷½Úµã?	
	DEBUG0_ON(idx_ni->type != AT_INDEX_ALLOCATION);
	DEBUG0_ON(idx_ni->nr_extents != -1); //Ò»¶¨ÒªÊÇ´Ó½Úµãå	
	DEBUG0_ON(!base_ni);
	DEBUG0_ON(!key);	
	DEBUG0_ON(key_len <= 0);
		
	if (!ntfs_is_collation_rule_supported(
			idx_ni->itype.index.collation_rule)) {
		ntfs_error(sb, "Index uses unsupported collation rule 0x%x.  "
				"Aborting lookup.", le32_to_cpu(
				idx_ni->itype.index.collation_rule));
		return -EOPNOTSUPP;
	}
	ntfs_debug("Index uses supported collation rule 0x%x. ",le32_to_cpu(idx_ni->itype.index.collation_rule));
	/* Get hold of the mft record for the index inode. */
	m = map_mft_record(base_ni);
	if (IS_ERR(m)) {
		ntfs_error(sb, "map_mft_record() failed with error code %ld.",
				-PTR_ERR(m));
		return PTR_ERR(m);
	}
	actx = ntfs_attr_get_search_ctx(base_ni, m);
	if (unlikely(!actx)) {
		err = -ENOMEM;
		goto err_out;
	}
	old_vcn = VCN_INDEX_ROOT_PARENT; //add by yangli
	
	/* Find the index root attribute in the mft record. */
	err = ntfs_attr_lookup(AT_INDEX_ROOT, idx_ni->name, idx_ni->name_len,
			CASE_SENSITIVE, 0, NULL, 0, actx);
	if (unlikely(err)) {
		if (err == -ENOENT) {
			ntfs_error(sb, "Index root attribute missing in inode "
					"0x%lx.", idx_ni->mft_no);
			err = -EIO;
		}
		goto err_out;
	}
	/* Get to the index root value (it has been verified in read_inode). */
	ir = (INDEX_ROOT*)((u8*)actx->attr +
			le16_to_cpu(actx->attr->data.resident.value_offset));
	index_end = (u8*)&ir->index + le32_to_cpu(ir->index.index_length);
	
	/* The first index entry. */
	ie = (INDEX_ENTRY*)((u8*)&ir->index +
			le32_to_cpu(ir->index.entries_offset));
	/*
	 * Loop until we exceed valid memory (corruption case) or until we
	 * reach the last entry.
	 */
	for (;; ie = (INDEX_ENTRY*)((u8*)ie + le16_to_cpu(ie->length))) {
		/* Bounds checks. */
		if ((u8*)ie < (u8*)actx->mrec || (u8*)ie +
				sizeof(INDEX_ENTRY_HEADER) > index_end ||
				(u8*)ie + le16_to_cpu(ie->length) > index_end)
			goto idx_err_out;
		/*
		 * The last entry cannot contain a key.  It can however contain
		 * a pointer to a child node in the B+tree so we just break out.
		 ×îºóÒ»¸öentry²»°üº¬key£¬µ«ÊÇËüÖ¸ÏòB+Ê÷µÄÒ»¸öº¢×Ó½Úµã
		 */
		if (ie->flags & INDEX_ENTRY_END)
			break;
		/* Further bounds checks. */
		if ((u32)sizeof(INDEX_ENTRY_HEADER) +
				le16_to_cpu(ie->key_length) >
				le16_to_cpu(ie->data.vi.data_offset) ||
				(u32)le16_to_cpu(ie->data.vi.data_offset) +
				le16_to_cpu(ie->data.vi.data_length) >
				le16_to_cpu(ie->length))
			goto idx_err_out;
		/* If the keys match perfectly, we setup @ictx and return 0. */
		if ((key_len == le16_to_cpu(ie->key_length)) && !memcmp(key,
				&ie->key, key_len)) {				
ir_done:
			ictx->is_in_root = true;//Ã»ÓÐ0xa0
			ictx->ir = ir;
			ictx->actx = actx;
			ictx->base_ni = base_ni;//idx_niµÄÖ÷½Úµã
			ictx->ia = NULL;
			ictx->page = NULL;
done:			
			ictx->entry = ie;//Ê×¸öINDEX ENTRY
			ictx->data = (u8*)ie +
					le16_to_cpu(ie->data.vi.data_offset);
			ictx->data_len = le16_to_cpu(ie->data.vi.data_length);
			ntfs_debug("Done.");
			return err;
		}
		/*
		 * Not a perfect match, need to do full blown collation so we
		 * know which way in the B+tree we have to go.
		 */
		rc = ntfs_collate(vol, idx_ni->itype.index.collation_rule, key,
				key_len, &ie->key, le16_to_cpu(ie->key_length));
		/*
		 * If @key collates before the key of the current entry, there
		 * is definitely no such key in this index but we might need to
		 * descend into the B+tree so we just break out of the loop.
		 */
		if (rc == -1)
			break;
		/*
		 * A match should never happen as the memcmp() call should have
		 * cought it, but we still treat it correctly.
		 */
		if (!rc)
			goto ir_done;
		/* The keys are not equal, continue the search. */
	}
	/*
	 * We have finished with this index without success.  Check for the
	 * presence of a child node and if not present setup @ictx and return
	 * -ENOENT.
	 */
	if (!(ie->flags & INDEX_ENTRY_NODE)) {
		ntfs_debug("Entry not found.");
		err = -ENOENT;
		goto ir_done;
	} /* Child node present, descend into it. */
	/* Consistency check: Verify that an index allocation exists. */
	if (!NInoIndexAllocPresent(idx_ni)) { //Èç¹ûÓÐº¢×Ó½Úµã£¬¾ÍÒ»¶¨ÓÐ0XA0ÊôÐÔ
		ntfs_error(sb, "No index allocation attribute but index entry "
				"requires one.  Inode 0x%lx is corrupt or "
				"driver bug.", idx_ni->mft_no);
		goto err_out;
	}
/**/////////////////////////index allocation exists//////////////////////////////**/
	/* Get the starting vcn of the index_block holding the child node. */
	vcn = sle64_to_cpup((sle64*)((u8*)ie + le16_to_cpu(ie->length) - 8)); //º¢×Ó½Úµã¶ÔÓ¦µÄvcn
	ia_mapping = VFS_I(idx_ni)->i_mapping;
	/*
	 * We are done with the index root and the mft record.  Release them,
	 * otherwise we deadlock with ntfs_map_page().
	 */
	ntfs_attr_put_search_ctx(actx);
	unmap_mft_record(base_ni);
	m = NULL;
	actx = NULL;
descend_into_child_node:
	/*
	 * Convert vcn to index into the index allocation attribute in units
	 * of PAGE_CACHE_SIZE and map the page cache page, reading it from
	 * disk if necessary.
	 */
	page = ntfs_map_page(ia_mapping, vcn << //´Ó0xa0ÊôÐÔÖÐ¶ÁÈ¡º¢×Ó½Úµã¶ÔÓ¦µÄpage
			idx_ni->itype.index.vcn_size_bits >> PAGE_CACHE_SHIFT);
	if (IS_ERR(page)) {
		ntfs_error(sb, "Failed to map index page, error %ld.",
				-PTR_ERR(page));
		err = PTR_ERR(page);
		goto err_out;
	}
	lock_page(page);
	kaddr = (u8*)page_address(page);
fast_descend_into_child_node:
	/* Get to the index allocation block. */
	ia = (INDEX_ALLOCATION*)(kaddr + ((vcn <<
			idx_ni->itype.index.vcn_size_bits) & ~PAGE_CACHE_MASK));
	/* Bounds checks. */
	if ((u8*)ia < kaddr || (u8*)ia > kaddr + PAGE_CACHE_SIZE) {
		ntfs_error(sb, "Out of bounds check failed.  Corrupt inode "
				"0x%lx or driver bug.", idx_ni->mft_no);
		goto unm_err_out;
	}
	/* Catch multi sector transfer fixup errors. */
	if (unlikely(!ntfs_is_indx_record(ia->magic))) {
		ntfs_error(sb, "Index record with vcn 0x%llx is corrupt.  "
				"Corrupt inode 0x%lx.  Run chkdsk.",
				(long long)vcn, idx_ni->mft_no);
		goto unm_err_out;
	}
	if (sle64_to_cpu(ia->index_block_vcn) != vcn) {
		ntfs_error(sb, "Actual VCN (0x%llx) of index buffer is "
				"different from expected VCN (0x%llx).  Inode "
				"0x%lx is corrupt or driver bug.",
				(unsigned long long)
				sle64_to_cpu(ia->index_block_vcn),
				(unsigned long long)vcn, idx_ni->mft_no);
		goto unm_err_out;
	}
	if (le32_to_cpu(ia->index.allocated_size) + 0x18 !=
			idx_ni->itype.index.block_size) {
		ntfs_error(sb, "Index buffer (VCN 0x%llx) of inode 0x%lx has "
				"a size (%u) differing from the index "
				"specified size (%u).  Inode is corrupt or "
				"driver bug.", (unsigned long long)vcn,
				idx_ni->mft_no,
				le32_to_cpu(ia->index.allocated_size) + 0x18,
				idx_ni->itype.index.block_size);
		goto unm_err_out;
	}
	index_end = (u8*)ia + idx_ni->itype.index.block_size;
	if (index_end > kaddr + PAGE_CACHE_SIZE) {
		ntfs_error(sb, "Index buffer (VCN 0x%llx) of inode 0x%lx "
				"crosses page boundary.  Impossible!  Cannot "
				"access!  This is probably a bug in the "
				"driver.", (unsigned long long)vcn,
				idx_ni->mft_no);
		goto unm_err_out;
	}
	index_end = (u8*)&ia->index + le32_to_cpu(ia->index.index_length);
	if (index_end > (u8*)ia + idx_ni->itype.index.block_size) {
		ntfs_error(sb, "Size of index buffer (VCN 0x%llx) of inode "
				"0x%lx exceeds maximum size.",
				(unsigned long long)vcn, idx_ni->mft_no);
		goto unm_err_out;
	}
	/* The first index entry. */
	ie = (INDEX_ENTRY*)((u8*)&ia->index +
			le32_to_cpu(ia->index.entries_offset));
	/*
	 * Iterate similar to above big loop but applied to index buffer, thus
	 * loop until we exceed valid memory (corruption case) or until we
	 * reach the last entry.
	 */
	for (;; ie = (INDEX_ENTRY*)((u8*)ie + le16_to_cpu(ie->length))) {
		/* Bounds checks. */
		if ((u8*)ie < (u8*)ia || (u8*)ie +
				sizeof(INDEX_ENTRY_HEADER) > index_end ||
				(u8*)ie + le16_to_cpu(ie->length) > index_end) {
			ntfs_error(sb, "Index entry out of bounds in inode "
					"0x%lx.", idx_ni->mft_no);
			goto unm_err_out;
		}
		/*
		 * The last entry cannot contain a key.  It can however contain
		 * a pointer to a child node in the B+tree so we just break out.
		 */
		if (ie->flags & INDEX_ENTRY_END)
			break;
		/* Further bounds checks. */
		if ((u32)sizeof(INDEX_ENTRY_HEADER) +
				le16_to_cpu(ie->key_length) >
				le16_to_cpu(ie->data.vi.data_offset) ||
				(u32)le16_to_cpu(ie->data.vi.data_offset) +
				le16_to_cpu(ie->data.vi.data_length) >
				le16_to_cpu(ie->length)) {
			ntfs_error(sb, "Index entry out of bounds in inode "
					"0x%lx.", idx_ni->mft_no);
			goto unm_err_out;
		}
		/* If the keys match perfectly, we setup @ictx and return 0. */
		if ((key_len == le16_to_cpu(ie->key_length)) && !memcmp(key,
				&ie->key, key_len)) {			
ia_done:
			ictx->is_in_root = false;
			ictx->actx = NULL;
			ictx->base_ni = NULL;
			ictx->ia = ia;
			ictx->page = page;//´æ´¢iaµÄÊý¾Ý			
			goto done;
		}
		/*
		 * Not a perfect match, need to do full blown collation so we
		 * know which way in the B+tree we have to go.
		 */
		rc = ntfs_collate(vol, idx_ni->itype.index.collation_rule, key,
				key_len, &ie->key, le16_to_cpu(ie->key_length));
		/*
		 * If @key collates before the key of the current entry, there
		 * is definitely no such key in this index but we might need to
		 * descend into the B+tree so we just break out of the loop.
		 */
		if (rc == -1)
			break;
		/*
		 * A match should never happen as the memcmp() call should have
		 * cought it, but we still treat it correctly.
		 */
		if (!rc)
			goto ia_done;
		/* The keys are not equal, continue the search. */
	}
	/*
	 * We have finished with this index buffer without success.  Check for
	 * the presence of a child node and if not present return -ENOENT.
	 */
	if (!(ie->flags & INDEX_ENTRY_NODE)) {
		ntfs_debug("Entry not found.");
		err = -ENOENT;
		goto ia_done;
	}
	if ((ia->index.flags & NODE_MASK) == LEAF_NODE) {
		ntfs_error(sb, "Index entry with child node found in a leaf "
				"node in inode 0x%lx.", idx_ni->mft_no);
		goto unm_err_out;
	}
	/* Child node present, descend into it. */ //º¢×Ó½Úµã´æÔÚ£¬¼ÌÐø½øÈëÏÂÒ»¼¶B+ TREEËÑË÷
	old_vcn = vcn;
	vcn = sle64_to_cpup((sle64*)((u8*)ie + le16_to_cpu(ie->length) - 8));
	if (vcn >= 0) {
		/*
		 * If vcn is in the same page cache page as old_vcn we recycle
		 * the mapped page.
		 */
		if (old_vcn << vol->cluster_size_bits >>
				PAGE_CACHE_SHIFT == vcn <<
				vol->cluster_size_bits >>
				PAGE_CACHE_SHIFT)
			goto fast_descend_into_child_node;
		unlock_page(page);
		ntfs_unmap_page(page);
		goto descend_into_child_node;
	}
	ntfs_error(sb, "Negative child node vcn in inode 0x%lx.",
			idx_ni->mft_no);
unm_err_out:
	unlock_page(page);
	ntfs_unmap_page(page);
err_out:
	if (!err)
		err = -EIO;
	if (actx)
		ntfs_attr_put_search_ctx(actx);
	if (m)
		unmap_mft_record(base_ni);
	return err;
idx_err_out:
	ntfs_error(sb, "Corrupt index.  Aborting lookup.");
	goto err_out;
}

#ifdef NTFS_RW

#ifdef NTFS_MODIFY

 void _Print_Buf(unsigned char * name, const unsigned char * pData, unsigned short inLen)
{
	unsigned short iLoop;
	
	if (!debug_msgs)
		return;
	
	printk("%s \n",name);
	for( iLoop=0; iLoop< inLen; iLoop++)
	{
		printk("%2x ", pData[iLoop]);
		if( 0== ((iLoop+1) &0x0f) )
		{
			printk("  %x\n",iLoop);
		}
	}
       printk("\n");
	
}
void ntfs_index_ctx_reinit(ntfs_index_context *ictx)
{
	ntfs_debug("Entering");
	
	if (ictx->entry) {
		if (ictx->is_in_root) {
			if (ictx->actx)
				ntfs_attr_put_search_ctx(ictx->actx);
			ntfs_debug("next");
			//if (ictx->base_ni)
			//	unmap_mft_record(ictx->base_ni);
		} else {
			struct page *page = ictx->page;
			if (page) {
				ntfs_debug("next1");
				DEBUG0_ON(!PageLocked(page));
				unlock_page(page);
				ntfs_unmap_page(page);				
			}
		}
	}
	
	if (ictx)
		*ictx = (ntfs_index_context){ .idx_ni = ictx->idx_ni,
								 .pindex = 0,
							     };
}

static int ntfs_icx_parent_inc(ntfs_index_context *icx)
{
	icx->pindex++;
	if (icx->pindex >= MAX_PARENT_VCN) {
		//errno = EOPNOTSUPP;
		ntfs_debug("Index is over %d level deep", MAX_PARENT_VCN);
		return STATUS_ERROR;
	}
	return STATUS_OK;
}

static int ntfs_icx_parent_dec(ntfs_index_context *icx)
{
	icx->pindex--;
	ntfs_debug("icx->pindex == 0x%x ",icx->pindex);
	if (icx->pindex < 0) {
		//errno = EINVAL;
		ntfs_debug("Corrupt index pointer (%d)", icx->pindex);
		return STATUS_ERROR;
	}
	return STATUS_OK;
}
/**ÔÚindexÕÒµ½key£¬²¢·µ»ØËüµÄindex Èë¿Ú
 * ntfs_index_lookup - find a key in an index and return its index entry
 * @key:	[IN] key for which to search in the index
 * @key_len:	[IN] length of @key in bytes
 * @ictx:	[IN/OUT] context describing the index and the returned entryÊäÈëÊä³ö
 *
 * Before calling ntfs_index_lookup(), @ictx must have been obtained from a
 * call to ntfs_index_ctx_get().
 *
 * Look for the @key in the index specified by the index lookup context @ictx.
 * ntfs_index_lookup() walks the contents of the index looking for the @key.
 *
 * If the @key is found in the index, 0 is returned and @ictx is setup to
 * describe the index entry containing the matching @key.  @ictx->entry is the
 * index entry and @ictx->data and @ictx->data_len are the index entry data and
 * its length in bytes, respectively.
 Èç¹ûÕÒµ½ÁË£¬·µ»Ø0£¬ictxÃèÊöÁËÓëkeyÆ¥ÅäµÄindex entry
 *
 * If the @key is not found in the index, -ENOENT is returned and @ictx is
 * setup to describe the index entry whose key collates immediately after the
 * search @key, i.e. this is the position in the index at which an index entry
 * with a key of @key would need to be inserted.
 *Èç¹û@keyÃ»ÕÒµ½£¬·µ»Ø-ENOENT£¬@ictxÇ¡ºÃ¾ÍÊÇÒªÕÒ@keyµÄÏÂÒ»¸ö£¬
´ËÎ»ÖÃ¿ÉÒÔ²åÈëindex entry¡£

 * If an error occurs return the negative error code and @ictx is left
 * untouched.
 *
 * When finished with the entry and its data, call ntfs_index_ctx_put() to free
 * the context and other associated resources.
 *
 * If the index entry was modified, call flush_dcache_index_entry_page()
 * immediately after the modification and either ntfs_index_entry_mark_dirty()
 * or ntfs_index_entry_write() before the call to ntfs_index_ctx_put() to
 * ensure that the changes are written to disk.
 *
 * Locking:  - Caller must hold i_mutex on the index inode.
 *	     - Each page cache page in the index allocation mapping must be
 *	       locked whilst being accessed otherwise we may find a corrupt
 *	       page due to it being under ->writepage at the moment which
 *	       applies the mst protection fixups before writing out and then
 *	       removes them again after the write is complete after which it 
 *	       unlocks the page.
 */
int ntfs_index_lookup2(const void *key, const int key_len,
		ntfs_index_context *ictx)
{
	VCN vcn, old_vcn;
	ntfs_inode *idx_ni = ictx->idx_ni;
	ntfs_volume *vol = idx_ni->vol;
	struct super_block *sb = vol->sb;
	ntfs_inode *base_ni ;//= idx_ni->ext.base_ntfs_ino;
	MFT_RECORD *m;
	INDEX_ROOT *ir;
	INDEX_ENTRY *ie;
	INDEX_ALLOCATION *ia;
	u8 *index_end, *kaddr;
	ntfs_attr_search_ctx *actx;
	struct address_space *ia_mapping;
	struct page *page =NULL;
	int rc, err = 0;
	int item = 0;
	int ir_flag = 0;
	
	ntfs_debug("Entering.----------------------------------------------------------");
	ntfs_debug("mft_no =0x%lx, idx_ni->nr_extents == %d",idx_ni->mft_no,idx_ni->nr_extents);
/*	DEBUG0_ON(!NInoAttr(idx_ni));	//²»ÄÜÊÇÖ÷½Úµã?
	ntfs_debug("next1.");
	DEBUG0_ON(idx_ni->type != AT_INDEX_ALLOCATION);
	ntfs_debug("next2.");
	DEBUG0_ON(idx_ni->nr_extents != -1); //Ò»¶¨ÒªÊÇ´Ó½Úµãå
	ntfs_debug("next3.");
	DEBUG0_ON(!base_ni);
	ntfs_debug("next4.");*/
	
	DEBUG0_ON(!key);
	DEBUG0_ON(key_len <= 0);	
	
	if (!NInoAttr(idx_ni))//ÅÐ¶ÏÊÇ·ñÎªÖ÷½Úµã
		base_ni = idx_ni;//Ö÷½Úµã
	else
		base_ni = idx_ni->ext.base_ntfs_ino;//´Î½ÚµãµÄÖ÷½Úµã
	ntfs_debug("base mft_no =0x%lx", base_ni->mft_no);
	
	if (!ntfs_is_collation_rule_supported(
			idx_ni->itype.index.collation_rule)) {
		ntfs_error(sb, "Index uses unsupported collation rule 0x%x.  "
				"Aborting lookup.", le32_to_cpu(
				idx_ni->itype.index.collation_rule));
		return -EOPNOTSUPP;
	}
	ntfs_debug("Index uses supported collation rule 0x%x. ",le32_to_cpu(idx_ni->itype.index.collation_rule));

	/* Get hold of the mft record for the index inode. */
	m = map_mft_record(base_ni);
	if (IS_ERR(m)) {
		ntfs_error(sb, "map_mft_record() failed with error code %ld.",
				-PTR_ERR(m));
		return PTR_ERR(m);
	}
	actx = ntfs_attr_get_search_ctx(base_ni, m);
	if (unlikely(!actx)) {
		err = -ENOMEM;
		goto err_out;
	}
	unmap_mft_record(base_ni);
	old_vcn = VCN_INDEX_ROOT_PARENT; //add by yangli
	
	/* Find the index root attribute in the mft record. */
	err = ntfs_attr_lookup(AT_INDEX_ROOT, I30, 4, CASE_SENSITIVE, 0, NULL, 0, actx);
	if (unlikely(err)) {
		if (err == -ENOENT) {
			ntfs_error(sb, "Index root attribute missing in inode "
					"0x%lx.", idx_ni->mft_no);
			err = -EIO;
		}
		goto err_out;
	}
	ntfs_debug("have found the AT_INDEX_RROT");
	/* Get to the index root value (it has been verified in read_inode). */
	ir = (INDEX_ROOT*)((u8*)actx->attr +
			le16_to_cpu(actx->attr->data.resident.value_offset));
	index_end = (u8*)&ir->index + le32_to_cpu(ir->index.index_length);
	
	/* The first index entry. */
	ie = (INDEX_ENTRY*)((u8*)&ir->index +
			le32_to_cpu(ir->index.entries_offset));
	//ntfs_debug("ir 0x%x , ie 0x%x, ,index_end 0x%x",&ir, &ie, &index_end);

	/*
	 * Loop until we exceed valid memory (corruption case) or until we
	 * reach the last entry.
	 */
	for (;; ie = (INDEX_ENTRY*)((u8*)ie + le16_to_cpu(ie->length))) {
		/* Bounds checks. */
		//ntfs_debug("ir 0x%x , ie 0x%x, ,index_end 0x%x",&ir, &ie, &index_end);

		if ((u8*)ie < (u8*)actx->mrec || (u8*)ie +
				sizeof(INDEX_ENTRY_HEADER) > index_end ||
				(u8*)ie + le16_to_cpu(ie->length) > index_end)
			goto idx_err_out;
		/*
		 * The last entry cannot contain a key.  It can however contain
		 * a pointer to a child node in the B+tree so we just break out.
		 ×îºóÒ»¸öentry²»°üº¬key£¬µ«ÊÇËüÖ¸ÏòB+Ê÷µÄÒ»¸öº¢×Ó½Úµã
		 */
		if (ie->flags & INDEX_ENTRY_END)
			break;
		/* Further bounds checks. */
		/* @ delete by yangli*/
		/* @ ie->data is dir ,not data */
		/*
		if ((u32)sizeof(INDEX_ENTRY_HEADER) +
				le16_to_cpu(ie->key_length) >
				le16_to_cpu(ie->data.vi.data_offset) ||
				(u32)le16_to_cpu(ie->data.vi.data_offset) +
				le16_to_cpu(ie->data.vi.data_length) >
				le16_to_cpu(ie->length))
			goto idx_err_out;
		*/
		/* @ end */
		/* If the keys match perfectly, we setup @ictx and return 0. */
		if ((key_len == le16_to_cpu(ie->key_length)) && !memcmp(key,
				&ie->key, key_len)) {				
ir_done:
			ictx->parent_vcn[ictx->pindex] = old_vcn; //add by yangli
			ntfs_debug("is_in_root = 1");
			ictx->is_in_root = true;//Ã»ÓÐ0xa0
			ictx->ir = ir;
			ictx->actx = actx;
			//ictx->base_ni = base_ni;//idx_niµÄÖ÷½Úµã by yangli
			ictx->base_ni = NULL; //by yangli
			ictx->ia = NULL;
			ictx->page = NULL;
done:
			ictx->block_size = ir->index_block_size;//add by yangli
			if (base_ni->vol->cluster_size <= ictx->block_size)
				ictx->vcn_size_bits = base_ni->vol->cluster_size_bits;//add by yangli
			else
				ictx->vcn_size_bits = base_ni->vol->sector_size_bits;
			ntfs_debug("ictx->block_size = %d, ictx->vcn_size_bits=%d ",ictx->block_size,ictx->vcn_size_bits );
			ictx->entry = ie;//Ê×¸öINDEX ENTRY
			ictx->data = (u8*)ie +
					le16_to_cpu(ie->data.vi.data_offset);
			ictx->data_len = le16_to_cpu(ie->data.vi.data_length);
			if (ntfs_icx_parent_inc(ictx)) //may bug here
				goto unm_err_out;	
			ntfs_debug("Done.");
			return err;
		}
		/*
		 * Not a perfect match, need to do full blown collation so we
		 * know which way in the B+tree we have to go.
		 */
		rc = ntfs_collate(vol, idx_ni->itype.index.collation_rule, key,
				key_len, &ie->key, le16_to_cpu(ie->key_length));
		/*
		 * If @key collates before the key of the current entry, there
		 * is definitely no such key in this index but we might need to
		 * descend into the B+tree so we just break out of the loop.
		 */
		if (rc == -1){
			ntfs_debug("rc == -1");
			break;
		}
		/*
		 * A match should never happen as the memcmp() call should have
		 * cought it, but we still treat it correctly.
		 */
		if (!rc)
			goto ir_done;

		/*@add by yangli*/
		/*
		 * We have finished with this index block without success. Check for the
		 * presence of a child node and if not present return with errno ENOENT,
		 * otherwise we will keep searching in another index block.
		 */
		if (ie->flags & INDEX_ENTRY_NODE) {			
			/* Get the starting vcn of the index_block holding the child node. */
			ictx->parent_vcn[ictx->pindex] = old_vcn;
			vcn = sle64_to_cpup((sle64*)((u8*)ie + le16_to_cpu(ie->length) - 8)); //½øÈëB+ treeµÄÏÂÒ»¼¶			
			if (vcn < 0) {
				//errno = EINVAL;
				ntfs_error(sb,"Negative vcn in inode %llu",
					       	(unsigned long long)base_ni->mft_no);
				return STATUS_ERROR;
			}
			if (ntfs_icx_parent_inc(ictx)) 
				goto unm_err_out;	
			goto next_ia;
		}
next_ir:
		ntfs_debug("old_vcn :%lld",old_vcn);
		/*@ end add*/
		
		/* The keys are not equal, continue the search. */
	}

	ir_flag = 1;
	/*
	 * We have finished with this index without success.  Check for the
	 * presence of a child node and if not present setup @ictx and return
	 * -ENOENT.
	 */
	if (!(ie->flags & INDEX_ENTRY_NODE)) {
		ntfs_debug("Entry not found.");
		err = -ENOENT;
		goto ir_done;
	} /* Child node present, descend into it. */
	/* Consistency check: Verify that an index allocation exists. */
	if (!NInoIndexAllocPresent(idx_ni)) { //Èç¹ûÓÐº¢×Ó½Úµã£¬¾ÍÒ»¶¨ÓÐ0XA0ÊôÐÔ
		ntfs_error(sb, "No index allocation attribute but index entry "
				"requires one.  Inode 0x%lx is corrupt or "
				"driver bug.", idx_ni->mft_no);
		goto err_out;
	}
	
	ictx->parent_vcn[ictx->pindex] = old_vcn; //add by yangli
	
/**/////////////////////////index allocation exists//////////////////////////////**/
	/* Get the starting vcn of the index_block holding the child node. */
	vcn = sle64_to_cpup((sle64*)((u8*)ie + le16_to_cpu(ie->length) - 8)); //º¢×Ó½Úµã¶ÔÓ¦µÄvcn
next_ia:
	ntfs_debug("vcn == %lld \n",vcn);
	ia_mapping = VFS_I(idx_ni)->i_mapping;
	/*
	 * We are done with the index root and the mft record.  Release them,
	 * otherwise we deadlock with ntfs_map_page().
	 */
	ntfs_attr_put_search_ctx(actx);
	//unmap_mft_record(base_ni); fix by yangli
	m = NULL;
	actx = NULL;
descend_into_child_node:
	/*
	 * Convert vcn to index into the index allocation attribute in units
	 * of PAGE_CACHE_SIZE and map the page cache page, reading it from
	 * disk if necessary.
	 */
	page = ntfs_map_page(ia_mapping, vcn << //´Ó0xa0ÊôÐÔÖÐ¶ÁÈ¡º¢×Ó½Úµã¶ÔÓ¦µÄpage
			idx_ni->itype.index.vcn_size_bits >> PAGE_CACHE_SHIFT);
	if (IS_ERR(page)) {
		ntfs_error(sb, "Failed to map index page, error %ld.",
				-PTR_ERR(page));
		err = PTR_ERR(page);
		goto err_out;
	}
	lock_page(page);
	kaddr = (u8*)page_address(page);
fast_descend_into_child_node:
	/* Get to the index allocation block. */
	ia = (INDEX_ALLOCATION*)(kaddr + ((vcn <<
			idx_ni->itype.index.vcn_size_bits) & ~PAGE_CACHE_MASK));
	/* Bounds checks. */
	if ((u8*)ia < kaddr || (u8*)ia > kaddr + PAGE_CACHE_SIZE) {
		ntfs_error(sb, "Out of bounds check failed.  Corrupt inode "
				"0x%lx or driver bug.", idx_ni->mft_no);
		goto unm_err_out;
	}
	/* Catch multi sector transfer fixup errors. */
	if (unlikely(!ntfs_is_indx_record(ia->magic))) {
		ntfs_error(sb, "Index record with vcn 0x%llx is corrupt.  "
				"Corrupt inode 0x%lx.  Run chkdsk.",
				(long long)vcn, idx_ni->mft_no);
		goto unm_err_out;
	}
	ntfs_debug("magic = INDX");
	if (sle64_to_cpu(ia->index_block_vcn) != vcn) {
		ntfs_error(sb, "Actual VCN (0x%llx) of index buffer is "
				"different from expected VCN (0x%llx).  Inode "
				"0x%lx is corrupt or driver bug.",
				(unsigned long long)
				sle64_to_cpu(ia->index_block_vcn),
				(unsigned long long)vcn, idx_ni->mft_no);
		goto unm_err_out;
	}
	ntfs_debug( "Actual VCN (0x%llx) of index buffer is "
				"expected VCN (0x%llx).",
				(unsigned long long)
				sle64_to_cpu(ia->index_block_vcn),
				(unsigned long long)vcn);
	if (le32_to_cpu(ia->index.allocated_size) + 0x18 !=
			idx_ni->itype.index.block_size) {
		ntfs_error(sb, "Index buffer (VCN 0x%llx) of inode 0x%lx has "
				"a size (%u) differing from the index "
				"specified size (%u).  Inode is corrupt or "
				"driver bug.", (unsigned long long)vcn,
				idx_ni->mft_no,
				le32_to_cpu(ia->index.allocated_size) + 0x18,
				idx_ni->itype.index.block_size);
		goto unm_err_out;
	}
	ntfs_debug("allocated_size (%u) == the index "
				"specified size (%u). ", 
				le32_to_cpu(ia->index.allocated_size),
				idx_ni->itype.index.block_size);	
	index_end = (u8*)ia + idx_ni->itype.index.block_size;
	if (index_end > kaddr + PAGE_CACHE_SIZE) {
		ntfs_error(sb, "Index buffer (VCN 0x%llx) of inode 0x%lx "
				"crosses page boundary.  Impossible!  Cannot "
				"access!  This is probably a bug in the "
				"driver.", (unsigned long long)vcn,
				idx_ni->mft_no);
		goto unm_err_out;
	}

	index_end = (u8*)&ia->index + le32_to_cpu(ia->index.index_length);
	if (index_end > (u8*)ia + idx_ni->itype.index.block_size) {
		ntfs_error(sb, "Size of index buffer (VCN 0x%llx) of inode "
				"0x%lx exceeds maximum size.",
				(unsigned long long)vcn, idx_ni->mft_no);
		goto unm_err_out;
	}
	/* The first index entry. */
	ie = (INDEX_ENTRY*)((u8*)&ia->index +
			le32_to_cpu(ia->index.entries_offset));
	ntfs_debug("ia->index.index_length=%d,ia->index.entries_offset=%d ",le32_to_cpu(ia->index.index_length),le32_to_cpu(ia->index.entries_offset));
	/*
	 * Iterate similar to above big loop but applied to index buffer, thus
	 * loop until we exceed valid memory (corruption case) or until we
	 * reach the last entry.
	 */

	item = 0;
	for (;; ie = (INDEX_ENTRY*)((u8*)ie + le16_to_cpu(ie->length))) {
		/* Bounds checks. */
		//ntfs_debug("ie->length =%d",le16_to_cpu(ie->length));
		if ((u8*)ie < (u8*)ia || (u8*)ie +
				sizeof(INDEX_ENTRY_HEADER) > index_end ||
				(u8*)ie + le16_to_cpu(ie->length) > index_end) {
			ntfs_error(sb, "Index entry out of bounds in inode "
					"0x%lx.", idx_ni->mft_no);
			goto unm_err_out;
		}
		
		/*
		 * The last entry cannot contain a key.  It can however contain
		 * a pointer to a child node in the B+tree so we just break out.
		 */
		if (ie->flags & INDEX_ENTRY_END)
			break;
		/* Further bounds checks. */

		/* @ delete by yangli*/
		/* @ ie->data is dir ,not data */
		/*
		if ((u32)sizeof(INDEX_ENTRY_HEADER) +
				le16_to_cpu(ie->key_length) >
				le16_to_cpu(ie->data.vi.data_offset) ||
				(u32)le16_to_cpu(ie->data.vi.data_offset) +
				le16_to_cpu(ie->data.vi.data_length) >
				le16_to_cpu(ie->length)) {
			ntfs_error(sb, "Index entry out of bounds in inode "
					"0x%lx.", idx_ni->mft_no);
			goto unm_err_out;
		}*/
		/* @ end */
		
		/* If the keys match perfectly, we setup @ictx and return 0. */
		if ((key_len == le16_to_cpu(ie->key_length)) && !memcmp(key,
				&ie->key, key_len)) {
	
ia_done:
			if (ntfs_icx_parent_inc(ictx)) 
				goto unm_err_out;	
			/* @ add by yangli */						
			ictx->parent_vcn[ictx->pindex] = vcn;//index entryËùÔÚµÄindex_block¶ÔÓ¦µÄvcn

			ntfs_debug("Parent[0x%x] ,vcn 0x%llx, entry number %d\n",ictx->pindex,vcn, item);
			ictx->parent_pos[ictx->pindex] = item;/* end @ */
			
			ictx->is_in_root = false;
			ictx->actx = NULL;
			ictx->base_ni = NULL;
			ictx->ia = ia;
			ictx->page = page;//´æ´¢iaµÄÊý¾Ý				
			ntfs_debug("is_in_root = 0");
			goto done;
		}
		/*
		 * Not a perfect match, need to do full blown collation so we
		 * know which way in the B+tree we have to go.
		 */
		rc = ntfs_collate(vol, idx_ni->itype.index.collation_rule, key,
				key_len, &ie->key, le16_to_cpu(ie->key_length));
		/*
		 * If @key collates before the key of the current entry, there
		 * is definitely no such key in this index but we might need to
		 * descend into the B+tree so we just break out of the loop.
		 */
		if (rc == -1)
			break;
		/*
		 * A match should never happen as the memcmp() call should have
		 * cought it, but we still treat it correctly.
		 */
		if (!rc)
			goto ia_done;
		item ++;
		/* The keys are not equal, continue the search. */
	}
	/*
	 * We have finished with this index buffer without success.  Check for
	 * the presence of a child node and if not present return -ENOENT.
	 */
	
	if(ir_flag){
		 ntfs_debug("all entry have been searched");
		if (!(ie->flags & INDEX_ENTRY_NODE)) {
			ntfs_debug("Entry not found.");
			err = -ENOENT;
			goto ia_done;
		}
		 ntfs_debug("next1");
		if ((ia->index.flags & NODE_MASK) == LEAF_NODE) {
			ntfs_error(sb, "Index entry with child node found in a leaf "
					"node in inode 0x%lx.", idx_ni->mft_no);
			goto unm_err_out;
		}
		/* Child node present, descend into it. */ //º¢×Ó½Úµã´æÔÚ£¬¼ÌÐø½øÈëÏÂÒ»¼¶B+ TREEËÑË÷
		old_vcn = vcn;
		vcn = sle64_to_cpup((sle64*)((u8*)ie + le16_to_cpu(ie->length) - 8));
		ntfs_debug("child vcn : 0x%llx ",vcn);
		if (vcn >= 0) {
			/*
			 * If vcn is in the same page cache page as old_vcn we recycle
			 * the mapped page.
			 */
			if (old_vcn << vol->cluster_size_bits >>
					PAGE_CACHE_SHIFT == vcn <<
					vol->cluster_size_bits >>
					PAGE_CACHE_SHIFT)
				goto fast_descend_into_child_node;
			unlock_page(page);
			ntfs_unmap_page(page);
			goto descend_into_child_node;
		}
		ntfs_error(sb, "Negative child node vcn in inode 0x%lx.",
				idx_ni->mft_no);
	}else{
		 ntfs_debug("vcn %lld have been searched,go to next ir",vcn);
		 if (ntfs_icx_parent_inc(ictx)) 
				goto unm_err_out;	
			/* @ add by yangli */						
		ictx->parent_vcn[ictx->pindex] = vcn;//index entryËùÔÚµÄindex_block¶ÔÓ¦µÄvcn

		ntfs_debug("Parent[0x%x] ,vcn 0x%llx, entry number %d\n",ictx->pindex,vcn, item);
		ictx->parent_pos[ictx->pindex] = item;/* end @ */
		old_vcn = vcn;
		goto next_ir;
	}
		
unm_err_out:
	unlock_page(page);
	ntfs_unmap_page(page);
err_out:
	if (!err)
		err = -EIO;
	if (actx)
		ntfs_attr_put_search_ctx(actx);
	if (m)
		unmap_mft_record(base_ni);
	return err;
idx_err_out:
	ntfs_error(sb, "Corrupt index.  Aborting lookup.");
	goto err_out;
}


static s64 ntfs_ib_vcn_to_pos(ntfs_index_context *icx, VCN vcn)//VCN×ª»»Îªpos
{
	return vcn << icx->vcn_size_bits;
}

static VCN ntfs_ib_pos_to_vcn(ntfs_index_context *icx, s64 pos)//pos×ª»»ÎªVCN
{
	return pos >> icx->vcn_size_bits;
}

static int ntfs_ib_write(ntfs_index_context *icx, INDEX_BLOCK *ib,struct page *page)
{
	u8 *kaddr;
	s64 ret;
	unsigned long index;
	s64  vcn = sle64_to_cpu(ib->index_block_vcn);	
	ntfs_inode *ni = icx->idx_ni;
	
	index = (ib->index_block_vcn) <<ni->itype.index.vcn_size_bits >> PAGE_CACHE_SHIFT;

	ntfs_debug("vcn: 0x%llx\n", (long long)vcn);

	if(icx->page && (index == icx->page->index))
	{
		ntfs_debug("case 1");
		kaddr = (u8*)page_address(icx->page);
		memcpy(kaddr,ib,PAGE_SIZE);
		ret = check_mst_fixup((NTFS_RECORD*)ib,PAGE_SIZE);
		if(!ret)
			pre_write_mst_fixup((NTFS_RECORD*)ib,PAGE_SIZE);
	
		ntfs_index_entry_mark_dirty(icx);	
	}
	else if(page && (index == page->index)){
		ntfs_debug("case 2");
		kaddr = (u8*)page_address(page);
		memcpy(kaddr,ib,PAGE_SIZE);
		ret = check_mst_fixup((NTFS_RECORD*)ib,PAGE_SIZE);
		if(!ret)
			pre_write_mst_fixup((NTFS_RECORD*)ib,PAGE_SIZE);
		
		flush_dcache_page(page);
		set_page_dirty(page);		
		//write_one_page(page,1);
		//lock_page(page);
	}else{
		ntfs_debug("case 3");
		//NInoSetNonResident(icx->idx_ni);
		ret = check_mst_fixup((NTFS_RECORD*)ib,PAGE_SIZE);
		if(ret)
			post_write_mst_fixup((NTFS_RECORD*)ib);
		
		ret = ntfs_attr_mst_pwrite(icx->idx_ni,AT_INDEX_ALLOCATION, I30,4,ntfs_ib_vcn_to_pos(icx, vcn),
					   1, icx->block_size, ib);
		if (ret != 1) {//Ò»¸öblock
			ntfs_error(ni->vol->sb,"Failed to write index block %lld, inode %llu",
				(long long)vcn, (unsigned long long)ni->mft_no);
			return STATUS_ERROR;
		}
	}
		
#if 0
	s64 ret, vcn = sle64_to_cpu(ib->index_block_vcn);	
	unsigned long index;
	ntfs_inode *ni = icx->idx_ni;
	u8 *kaddr;

	ntfs_debug("vcn: 0x%llx\n", (long long)vcn);//Òò´ËÍ¨¹ýntfs_ibm_getfreeµÃµ½µÄvcnÊÇ±íÃ÷ÔÚËùÓÐINDEX BLOCKÖÐµÄÆ«ÒÆµØÖ·µÄ

	index = (ib->index_block_vcn) <<ni->itype.index.vcn_size_bits >> PAGE_CACHE_SHIFT;
		
	if((icx->page) && (index == icx->page->index))
	{		
		ntfs_debug("index :0x%lx,icx->page->index :0x%lx ",index,icx->page->index);
		kaddr = (u8*)page_address(icx->page);
		//ntfs_debug("map the page ok ");
		memcpy(kaddr,ib,PAGE_SIZE);
		ntfs_index_entry_mark_dirty(icx);	
	}else{
		//NInoSetNonResident(icx->idx_ni);
		ret = ntfs_attr_mst_pwrite(icx->idx_ni,AT_INDEX_ALLOCATION, I30,4,ntfs_ib_vcn_to_pos(icx, vcn),
					   1, icx->block_size, ib);
		if (ret != 1) {//Ò»¸öblock
			ntfs_error("Failed to write index block %lld, inode %llu",
				(long long)vcn, (unsigned long long)ni->mft_no);
			return STATUS_ERROR;
		}
	}

#endif
	ntfs_debug("STATUS_OK\n");
	return STATUS_OK;
}

static int ntfs_icx_ib_write(ntfs_index_context *icx)
{
		if (ntfs_ib_write(icx, icx->ia,icx->page))
			return STATUS_ERROR;
		
		//icx->ia_dirty = FALSE;
		
		return STATUS_OK;
}
#if 0

/**
 * ntfs_index_ctx_reinit - reinitialize an index context
 * @icx:	index context to reinitialize
 *
 * Reinitialize the index context @icx so it can be used for ntfs_index_lookup.
 */
void ntfs_index_ctx_reinit(ntfs_index_context *icx)
{
	ntfs_debug("Entering\n");
	
	ntfs_index_ctx_free(icx);
	
	*icx = (ntfs_index_context) {
		.ni = icx->idx_ni,
		.name = icx->name,
		.name_len = icx->name_len,
	};
}
#endif
static VCN *ntfs_ie_get_vcn_addr(INDEX_ENTRY *ie)
{
	//ntfs_debug("Entering");	
	return (VCN *)((u8 *)ie + le16_to_cpu(ie->length) - sizeof(VCN));
}

/**
 *  Get the subnode vcn to which the index entry refers.
 */
VCN ntfs_ie_get_vcn(INDEX_ENTRY *ie) //µÃµ½¸Ãindex_entry µÄrunlist(datarun)
{
	//ntfs_debug("Entering");
	return sle64_to_cpup(ntfs_ie_get_vcn_addr(ie));
}

/**µÃµ½¸ÃÄ¿Â¼ÏÂµÄµÚÒ»¸öË÷ÒýIndex_Entry*/
static INDEX_ENTRY *ntfs_ie_get_first(INDEX_HEADER *ih)
{
	return (INDEX_ENTRY *)((u8 *)ih + le32_to_cpu(ih->entries_offset));
}

/**µÃµ½µ±Ç°Ë÷ÒýIndex_EntryµÄÏÂÒ»¸öË÷Òý*/
static INDEX_ENTRY *ntfs_ie_get_next(INDEX_ENTRY *ie)
{
	return (INDEX_ENTRY *)((char *)ie + le16_to_cpu(ie->length));
}

/**µÃµ½¸ÃÄ¿Â¼¼ÇÂ¼µÄÎ²*/
static u8 *ntfs_ie_get_end(INDEX_HEADER *ih)
{
	/* FIXME: check if it isn't overflowing the index block size */
	return (u8 *)ih + le32_to_cpu(ih->index_length);
}

/*ÅÐ¶Ï¸ÃË÷ÒýieÊÇ·ñÎªÄ©Î²Ë÷Òý*/
static int ntfs_ie_end(INDEX_ENTRY *ie)
{
	return ie->flags & INDEX_ENTRY_END || !ie->length;
}

/** 
 *  Find the last entry in the index blockÕÒµ½µ±Ç°index blockÖÐµÄ×îºóÒ»¸öindex entry
 */
static INDEX_ENTRY *ntfs_ie_get_last(INDEX_ENTRY *ie, char *ies_end)
{
	ntfs_debug("Entering");
	
	while ((char *)ie < ies_end && !ntfs_ie_end(ie))
		ie = ntfs_ie_get_next(ie);
	
	return ie;
}

/*µÃµ½index blockÖÐµÄµÚpos¸öindex entry*/
static INDEX_ENTRY *ntfs_ie_get_by_pos(INDEX_HEADER *ih, int pos)
{
	INDEX_ENTRY *ie;
	
	ntfs_debug("pos: %d\n", pos);
	
	ie = ntfs_ie_get_first(ih);
	
	while (pos-- > 0)
		ie = ntfs_ie_get_next(ie);
	ntfs_debug("Done");
	return ie;
}
/*µÃµ½µ±Ç°index entryµÄÇ°Ò»¸öindex entry*/
static INDEX_ENTRY *ntfs_ie_prev(INDEX_HEADER *ih, INDEX_ENTRY *ie)
{
	INDEX_ENTRY *ie_prev = NULL;
	INDEX_ENTRY *tmp;
	
	ntfs_debug("Entering\n");
	
	tmp = ntfs_ie_get_first(ih);
	
	while (tmp != ie) {
		ie_prev = tmp;
		tmp = ntfs_ie_get_next(tmp);
	}
	
	return ie_prev;
}
#if 0
/*µÃµ½µ±Ç°index entryÖÐµÄ$filename*/
char *ntfs_ie_filename_get(INDEX_ENTRY *ie)
{
	FILE_NAME_ATTR *fn;

	fn = (FILE_NAME_ATTR *)&ie->key;
	//½«unicodeÎÄ¼þÃû×ªÎªÄÚÂë
	//ntfs_ucstonls();
	return ntfs_attr_name_get(fn->file_name, fn->file_name_length);
}


void ntfs_ie_filename_dump(INDEX_ENTRY *ie)
{
	char *s;

	s = ntfs_ie_filename_get(ie);
	ntfs_debug("'%s' ", s);
	ntfs_attr_name_free(&s);
}

void ntfs_ih_filename_dump(INDEX_HEADER *ih)
{
	INDEX_ENTRY *ie;
	
	ntfs_debug("Entering\n");
	
	ie = ntfs_ie_get_first(ih);
	while (!ntfs_ie_end(ie)) {
		ntfs_ie_filename_dump(ie);
		ie = ntfs_ie_get_next(ie);
	}
}
#endif
static int ntfs_ih_numof_entries(INDEX_HEADER *ih)//¼ÆËãINDEX HEADERÖÐENTRYµÄ¸öÊý
{
	int n;
	INDEX_ENTRY *ie;
	u8 *end;
	
	//ntfs_debug("Entering");
	
	end = ntfs_ie_get_end(ih);
	ie = ntfs_ie_get_first(ih);
	for (n = 0; !ntfs_ie_end(ie) && (u8 *)ie < end; n++)
		ie = ntfs_ie_get_next(ie);
	return n;
}

static int ntfs_ih_one_entry(INDEX_HEADER *ih)//Ö»ÓÐÒ»¸öINDEX ENTRY
{
	return (ntfs_ih_numof_entries(ih) == 1);
}

static int ntfs_ih_zero_entry(INDEX_HEADER *ih)//Ã»ÓÐINDEX ENTRY
{
	return (ntfs_ih_numof_entries(ih) == 0);
}

static void ntfs_ie_delete(INDEX_HEADER *ih, INDEX_ENTRY *ie)
{
	u32 new_size;
	
	ntfs_debug("Entering, mft no =%ld, ie->length = %d",MREF_LE(ie->data.dir.indexed_file) ,ie->length);
	
	new_size = le32_to_cpu(ih->index_length) - le16_to_cpu(ie->length);
	ih->index_length = cpu_to_le32(new_size);//¸Ä±äINDEX HEADERµÄ´óÐ¡
	memmove(ie, (u8 *)ie + le16_to_cpu(ie->length),//ÒÆ³ýINDEX ENTRY
		new_size - ((u8 *)ie - (u8 *)ih));
	ntfs_debug("Done");
}
static void ntfs_ie_set_vcn(INDEX_ENTRY *ie, VCN vcn)
{
	//ntfs_debug("Entering");
	*ntfs_ie_get_vcn_addr(ie) = cpu_to_le64(vcn);
}

/**½«@ie²åÈëµ½@ih£¬Ô­@posÎ»ÖÃ
 *  Insert @ie index entry at @pos entry. Used @ih values should be ok already.
 */
static void ntfs_ie_insert(INDEX_HEADER *ih, INDEX_ENTRY *ie, INDEX_ENTRY *pos)
{
	int ie_size = le16_to_cpu(ie->length);
	
	//ntfs_debug("Entering\n");
	//_Print_Buf("ie->key2",(unsigned char *)ie->key.file_name.file_name,ie->key.file_name.file_name_length*2);
	ih->index_length = cpu_to_le32(le32_to_cpu(ih->index_length) + ie_size);
	memmove((u8 *)pos + ie_size, pos,	//½«posÖ®ºóµÄÒ»ÇÐ¶¼Å²µ½pos+ie_sizeÖ®ºó
		le32_to_cpu(ih->index_length) - ((u8 *)pos - (u8 *)ih) - ie_size);
	memcpy(pos, ie, ie_size);//½«ÍÚ³öµÄ¿Õ°×ÓÃieÌî³ä
}

/** ÔÚINDEX_HEADERÖ¸ÏòµÄINDEX_BLOCKÖÐ²éÕÒ°üº¬keyµÄINDEX_ENTRY
 * Find a key in the index block.
 * 
 * Return values:
 *   STATUS_OK with errno set to ESUCCESS if we know for sure that the 
 *             entry exists and @ie_out points to this entry.
 *   STATUS_NOT_FOUND with errno set to ENOENT if we know for sure the
 *                    entry doesn't exist and @ie_out is the insertion point.
 *   STATUS_KEEP_SEARCHING if we can't answer the above question and
 *                         @vcn will contain the node index block.
 *   STATUS_ERROR with errno set if on unexpected error during lookup.
 */
static int ntfs_ie_lookup(const void *key, const int key_len,
			  ntfs_index_context *icx, INDEX_HEADER *ih,
			  VCN *vcn, INDEX_ENTRY **ie_out)
{
	INDEX_ENTRY *ie;
	u8 *index_end;
	int rc, item = 0;
	ntfs_inode *idx_ni = icx->idx_ni;
	ntfs_volume *vol = idx_ni->vol;
	struct super_block *sb = vol->sb;

	ntfs_debug("Entering++++");
	
	index_end = ntfs_ie_get_end(ih);
	
	/*
	 * Loop until we exceed valid memory (corruption case) or until we
	 * reach the last entry.
	 */
	for (ie = ntfs_ie_get_first(ih); ; ie = ntfs_ie_get_next(ie)) {
		/* Bounds checks. */
		if ((u8 *)ie + sizeof(INDEX_ENTRY_HEADER) > index_end ||
		    (u8 *)ie + le16_to_cpu(ie->length) > index_end) {
			//errno = ERANGE;
			ntfs_error(sb,"Index entry out of bounds in inode "
				       "%llu.",
				       (unsigned long long)icx->idx_ni->mft_no);
			return STATUS_ERROR;
		}
		/*
		 * The last entry cannot contain a key.  It can however contain
		 * a pointer to a child node in the B+tree so we just break out.
		 */
		if (ntfs_ie_end(ie))
			break;
	
		 /*
		 * Not a perfect match, need to do full blown collation so we
		 * know which way in the B+tree we have to go.
		 */
		rc = ntfs_collate(vol, idx_ni->itype.index.collation_rule, key,
				key_len, &ie->key, le16_to_cpu(ie->key_length));
		
		/*
		 * If @key collates before the key of the current entry, there
		 * is definitely no such key in this index but we might need to
		 * descend into the B+tree so we just break out of the loop.
		 */
		if (rc == -1)
			break;
		
		if (!rc) {//ÕÒµ½key
			*ie_out = ie;
			//errno = 0;
			icx->parent_pos[icx->pindex] = item; //¸ÃINDEX_ENTRYÔÚ¸ÃINDEX_BLOCKÖÐµÄÐòºÅ
			ntfs_debug("STATUS_OK++++++++\n");
			return STATUS_OK;
		}
		
		item++;
	}
	/*
	 * We have finished with this index block without success. Check for the
	 * presence of a child node and if not present return with errno ENOENT,
	 * otherwise we will keep searching in another index block.
	 */
	if (!(ie->flags & INDEX_ENTRY_NODE)) {
		ntfs_debug("Index entry wasn't found.");
		*ie_out = ie;
		//errno = ENOENT;
		return STATUS_NOT_FOUND;
	}
	
	/* Get the starting vcn of the index_block holding the child node. */
	*vcn = ntfs_ie_get_vcn(ie); 	//½øÈëB+ treeµÄÏÂÒ»¼¶
	if (*vcn < 0) {
		//errno = EINVAL;
		ntfs_error(sb,"Negative vcn in inode %llu",
			       	(unsigned long long)icx->idx_ni->mft_no);
		return STATUS_ERROR;
	}

	ntfs_debug("[STATUS_KEEP_SEARCHING++++]: icx->pindex:%d, entry number %d,vcn :0x%llx\n",icx->pindex, item,*vcn);
	icx->parent_pos[icx->pindex] = item;
	
	return STATUS_KEEP_SEARCHING;//ÐèÒª¼ÌÐøËÑË÷
}
static INDEX_ENTRY *ntfs_ie_dup(INDEX_ENTRY *ie)
{
	INDEX_ENTRY *dup;
	
	ntfs_debug("Entering\n");
	
	dup = kmalloc(le16_to_cpu(ie->length),GFP_KERNEL);
	if (dup)
		memcpy(dup, ie, le16_to_cpu(ie->length));
	
	return dup;
}
/**¸´ÖÆINDEX ENTRY£¬Èç¹û@ie°üº¬×Ó½Úµã£¬½«Æä¸´ÖÆÎª·Ç¸¸½Úµã?*/
static INDEX_ENTRY *ntfs_ie_dup_novcn(INDEX_ENTRY *ie)
{
	INDEX_ENTRY *dup;
	int size = le16_to_cpu(ie->length);
	
	ntfs_debug("Entering");
	
	if (ie->flags & INDEX_ENTRY_NODE)//°üº¬×Ó½Úµã
		size -= sizeof(VCN);
	
	dup = kmalloc(size,GFP_KERNEL);
	if (dup) {
		memcpy(dup, ie, size);
		dup->flags &= ~INDEX_ENTRY_NODE;
		dup->length = cpu_to_le16(size);
	}
	return dup;
}

static int ntfs_ia_check(ntfs_index_context *icx, INDEX_BLOCK *ib, VCN vcn)
{
	u32 ib_size = (unsigned)le32_to_cpu(ib->index.allocated_size) + 0x18;
	
	ntfs_debug("Entering\n");
	
	if (!ntfs_is_indx_record(ib->magic)) {
		
		ntfs_debug("Corrupt index block signature: vcn %lld inode "
			       "%llu\n", (long long)vcn,
			       (unsigned long long)icx->idx_ni->mft_no);
		return -1;
	}
	
	if (sle64_to_cpu(ib->index_block_vcn) != vcn) {
		
		ntfs_debug("Corrupt index block: VCN (%lld) is different "
			       "from expected VCN (%lld) in inode %llu\n",
			       (long long)sle64_to_cpu(ib->index_block_vcn),
			       (long long)vcn,
			       (unsigned long long)icx->idx_ni->mft_no);
		return -1;
	}
	
	if (ib_size != icx->block_size) {
		
		ntfs_debug("Corrupt index block : VCN (%lld) of inode %llu "
			       "has a size (%u) differing from the index "
			       "specified size (%u)\n", (long long)vcn, 
			       (unsigned long long)icx->idx_ni->mft_no, ib_size,
			       icx->block_size);
		return -1;
	}
	return 0;
}

/**Ñ°ÕÒindex_root*/
static INDEX_ROOT *ntfs_ir_lookup(ntfs_inode *ni, ntfschar *name,
				  u32 name_len, ntfs_attr_search_ctx **ctx)
{
	ATTR_RECORD *a;
	INDEX_ROOT *ir = NULL;
	MFT_RECORD *mrec = NULL;
	int err = 0;
	ntfs_debug("Entering++++++++");
	
	mrec = map_mft_record(ni);
	if (IS_ERR(mrec)) {
		err = PTR_ERR(mrec);
		ntfs_error(VFS_I(ni)->i_sb, "Failed to map mft record for inode 0x%lx "
				"(error code %d).", VFS_I(ni)->i_ino, err);		
		return NULL;
	}
	*ctx= ntfs_attr_get_search_ctx(ni, mrec);
	if (!*ctx){		
		goto err_out;
	}
	
	
	if ((ntfs_attr_lookup(AT_INDEX_ROOT, name, name_len, CASE_SENSITIVE, 
			     0, NULL, 0, *ctx) != 0)) {
		ntfs_debug("Failed to find $INDEX_ROOT");
		goto err_out;
	}
	
	a = (*ctx)->attr;
	if (a->non_resident) {//$INDEX_ROOTÒ»¶¨ÊÇ×¤ÁôµÄ
		//errno = EINVAL;
		ntfs_debug("Non-resident $INDEX_ROOT detected");
		goto err_out;
	}
	
	ir = (INDEX_ROOT *)((char *)a + le16_to_cpu(a->data.resident.value_offset));//ÕÒµ½ÁË$INDEX_ROOTµÄÖµ
	ntfs_debug("Found++++++++\n");
err_out:
	if (!ir) {
		ntfs_attr_put_search_ctx(*ctx);
		*ctx = NULL;
	}
	unmap_mft_record(ni);
	return ir;
}

/*ºÍntfs_ir_lookupµÄÇø±ðÔÚÓÚÊÍ·ÅÁËctx*/
static INDEX_ROOT *ntfs_ir_lookup2(ntfs_inode *ni, ntfschar *name, u32 len)
{
	ntfs_attr_search_ctx *ctx;
	INDEX_ROOT *ir;

	ir = ntfs_ir_lookup(ni, name, len, &ctx);
	if (ir)
		ntfs_attr_put_search_ctx(ctx);
	return ir;
}

#if 0
/**  ÔÚINDEX_HEADERÖ¸ÏòµÄINDEX_BLOCKÖÐ²éÕÒ°üº¬keyµÄINDEX_ENTRY

 * Find a key in the index block.
 * 
 * Return values:
 *   STATUS_OK with errno set to ESUCCESS if we know for sure that the 
 *             entry exists and @ie_out points to this entry.
 *   STATUS_NOT_FOUND with errno set to ENOENT if we know for sure the
 *                    entry doesn't exist and @ie_out is the insertion point.
 *   STATUS_KEEP_SEARCHING if we can't answer the above question and
 *                         @vcn will contain the node index block.
 *   STATUS_ERROR with errno set if on unexpected error during lookup.
 */
static int ntfs_ie_lookup(const void *key, const int key_len,
			  ntfs_index_context *icx, INDEX_HEADER *ih,
			  VCN *vcn, INDEX_ENTRY **ie_out)
{
	INDEX_ENTRY *ie;
	u8 *index_end;
	int rc, item = 0;
	 
	ntfs_debug("Entering\n");
	
	index_end = ntfs_ie_get_end(ih);
	
	/*
	 * Loop until we exceed valid memory (corruption case) or until we
	 * reach the last entry.
	 */
	for (ie = ntfs_ie_get_first(ih); ; ie = ntfs_ie_get_next(ie)) {
		/* Bounds checks. */
		if ((u8 *)ie + sizeof(INDEX_ENTRY_HEADER) > index_end ||
		    (u8 *)ie + le16_to_cpu(ie->length) > index_end) {
			//errno = ERANGE;
			ntfs_debug("Index entry out of bounds in inode "
				       "%llu.\n",
				       (unsigned long long)icx->idx_ni->mft_no);
			return STATUS_ERROR;
		}
		/*
		 * The last entry cannot contain a key.  It can however contain
		 * a pointer to a child node in the B+tree so we just break out.
		 */
		if (ntfs_ie_end(ie))
			break;
		/*
		 * Not a perfect match, need to do full blown collation so we
		 * know which way in the B+tree we have to go.
		 */
		if (!icx->collate) {
			ntfs_debug("Collation function not defined\n");
			errno = EOPNOTSUPP;
			return STATUS_ERROR;
		}
		rc = icx->collate(icx->idx_ni->vol, key, key_len,
					&ie->key, le16_to_cpu(ie->key_length));
		if (rc == NTFS_COLLATION_ERROR) {
			ntfs_debug("Collation error. Perhaps a filename "
				       "contains invalid characters?\n");
			errno = ERANGE;
			return STATUS_ERROR;
		}
		/*
		 * If @key collates before the key of the current entry, there
		 * is definitely no such key in this index but we might need to
		 * descend into the B+tree so we just break out of the loop.
		 */
		if (rc == -1)
			break;
	
		if (!rc) {//ÕÒµ½key
			*ie_out = ie;
			errno = 0;
			icx->parent_pos[icx->pindex] = item; //¸ÃINDEX_ENTRYÔÚ¸ÃINDEX_BLOCKÖÐµÄÐòºÅ
			return STATUS_OK;
		}
		
		item++;
	}
	/*
	 * We have finished with this index block without success. Check for the
	 * presence of a child node and if not present return with errno ENOENT,
	 * otherwise we will keep searching in another index block.
	 */
	if (!(ie->ie_flags & INDEX_ENTRY_NODE)) {
		ntfs_debug("Index entry wasn't found.\n");
		*ie_out = ie;
		errno = ENOENT;
		return STATUS_NOT_FOUND;
	}
	
	/* Get the starting vcn of the index_block holding the child node. */
	*vcn = ntfs_ie_get_vcn(ie); 	//½øÈëB+ treeµÄÏÂÒ»¼¶
	if (*vcn < 0) {
		errno = EINVAL;
		ntfs_debug("Negative vcn in inode %llu",
			       	(unsigned long long)icx->ni->mft_no);
		return STATUS_ERROR;
	}

	ntfs_debug("Parent entry number %d\n", item);
	icx->parent_pos[icx->pindex] = item;
	
	return STATUS_KEEP_SEARCHING;//ÐèÒª¼ÌÐøËÑË÷
}
}

static ntfs_attr_3g *ntfs_ia_open(ntfs_index_context *icx, ntfs_inode *ni)
{
	ntfs_attr_3g *na_3g;
	
	na_3g = ntfs_attr_open(ni, AT_INDEX_ALLOCATION, icx->name, icx->name_len);
	if (!na_3g) {
		ntfs_debug("Failed to open index allocation of inode "
				"%llu", (unsigned long long)ni->mft_no);
		return NULL;
	}
	
	return na_3g;
}
#endif

static struct page*  ntfs_ib_read(ntfs_index_context *icx, VCN vcn, INDEX_BLOCK **ib)
{
	struct address_space *ia_mapping;
	struct  page *page = NULL;
	u8 *index_end, *kaddr;
	INDEX_ALLOCATION *ia =NULL;
	ntfs_inode *idx_ni = icx->idx_ni;
	struct super_block *sb = idx_ni->vol->sb;
	int err = 0;
	int len = 0;
	pgoff_t index;
	
	ntfs_debug("inode 0x%lx, vcn: 0x%llx", idx_ni->mft_no,(long long)vcn);
	ia_mapping = VFS_I(icx->idx_ni)->i_mapping;	
	index  = (vcn <<idx_ni->itype.index.vcn_size_bits >> PAGE_CACHE_SHIFT);

	/*
	 * Convert vcn to index into the index allocation attribute in units
	 * of PAGE_CACHE_SIZE and map the page cache page, reading it from
	 * disk if necessary.
	 */	
	 if(icx->page)
		ntfs_debug("icx->page->index : 0x%lx ",icx->page->index);
	if((icx->page) && (index == icx->page->index))
	{
		ntfs_debug("case1:0x%lx",index);
		page = icx->page;
		ia = icx->ia;	
	}else {
		ntfs_debug("case2:");
		page = ntfs_map_page(ia_mapping, vcn << //´Ó0xa0ÊôÐÔÖÐ¶ÁÈ¡º¢×Ó½Úµã¶ÔÓ¦µÄpage
				idx_ni->itype.index.vcn_size_bits >> PAGE_CACHE_SHIFT);
		if (IS_ERR(page)) {
			ntfs_error(sb, "Failed to map index page, error %ld.",
					-PTR_ERR(page));
			err = PTR_ERR(page);
			goto err_out;
		}
		lock_page(page);
	}
	kaddr = (u8*)page_address(page);
	
	/* Get to the index allocation block. */
	len =  (vcn <<idx_ni->itype.index.vcn_size_bits) & (~PAGE_CACHE_MASK);
	//memcpy(ia,(INDEX_ALLOCATION*)(kaddr +len),4096-len);
	ia = (INDEX_ALLOCATION*)(kaddr + ((vcn <<
			idx_ni->itype.index.vcn_size_bits) & ~PAGE_CACHE_MASK));
	
	/* Bounds checks. */
	if ((u8*)ia < kaddr || (u8*)ia > kaddr + PAGE_CACHE_SIZE) {
		ntfs_error(sb, "Out of bounds check failed.  Corrupt inode "
				"0x%lx or driver bug.", idx_ni->mft_no);
		goto unm_err_out;
	}
	/* Catch multi sector transfer fixup errors. */
	if (unlikely(!ntfs_is_indx_record(ia->magic))) {
		ntfs_error(sb, "Index record with vcn 0x%llx is corrupt.  "
				"Corrupt inode 0x%lx.  Run chkdsk.",
				(long long)vcn, idx_ni->mft_no);
		goto unm_err_out;
	}
	if (sle64_to_cpu(ia->index_block_vcn) != vcn) {
		ntfs_error(sb, "Actual VCN (0x%llx) of index buffer is "
				"different from expected VCN (0x%llx).  Inode "
				"0x%lx is corrupt or driver bug.",
				(unsigned long long)
				sle64_to_cpu(ia->index_block_vcn),
				(unsigned long long)vcn, idx_ni->mft_no);
		goto unm_err_out;
	}
	if (le32_to_cpu(ia->index.allocated_size) + 0x18 !=
			idx_ni->itype.index.block_size) {
		ntfs_error(sb, "Index buffer (VCN 0x%llx) of inode 0x%lx has "
				"a size (%u) differing from the index "
				"specified size (%u).  Inode is corrupt or "
				"driver bug.", (unsigned long long)vcn,
				idx_ni->mft_no,
				le32_to_cpu(ia->index.allocated_size) + 0x18,
				idx_ni->itype.index.block_size);
		goto unm_err_out;
	}
	index_end = (u8*)ia + idx_ni->itype.index.block_size;
	if (index_end > kaddr + PAGE_CACHE_SIZE) {
		ntfs_error(sb, "Index buffer (VCN 0x%llx) of inode 0x%lx "
				"crosses page boundary.  Impossible!  Cannot "
				"access!  This is probably a bug in the "
				"driver.", (unsigned long long)vcn,
				idx_ni->mft_no);
		goto unm_err_out;
	}
	index_end = (u8*)&ia->index + le32_to_cpu(ia->index.index_length);
	if (index_end > (u8*)ia + idx_ni->itype.index.block_size) {
		ntfs_error(sb, "Size of index buffer (VCN 0x%llx) of inode "
				"0x%lx exceeds maximum size.",
				(unsigned long long)vcn, idx_ni->mft_no);
		goto unm_err_out;
	}
err_out:
	*ib = ia;	
	//_Print_Buf("ib",*ib,128);
	err = check_mst_fixup((NTFS_RECORD*)ia,icx->block_size);
	if(err)
		post_read_mst_fixup((NTFS_RECORD*)ia,icx->block_size);
	
	ntfs_debug("Done. map the page ok \n");
	return page;
	
unm_err_out:
	
	if(page){
		unlock_page(page);
		ntfs_unmap_page(page);
	}
	return NULL;
}	
	
	


static INDEX_BLOCK *ntfs_ib_alloc(VCN ib_vcn, u32 ib_size, 
				  INDEX_HEADER_FLAGS node_type)
{
	INDEX_BLOCK *ib;
	int ih_size = sizeof(INDEX_HEADER);
	
	//ntfs_debug("ib_vcn: %lld ib_size: %u\n", (long long)ib_vcn, ib_size);
	
	ib = kcalloc(1,ib_size,GFP_KERNEL);
	if (!ib){
		ntfs_debug("kcalloc err ");
		return NULL;
	}
	
	ib->magic = magic_INDX;
	ib->usa_ofs = cpu_to_le16(sizeof(INDEX_BLOCK));
	ib->usa_count = cpu_to_le16(ib_size / NTFS_BLOCK_SIZE + 1);//ÐÞÕýÐòÁÐµÄ¸öÊý
	/* Set USN to 1 */
	*(u16 *)((char *)ib + le16_to_cpu(ib->usa_ofs)) = cpu_to_le16(1);
	ib->lsn = cpu_to_le64(0);
	
	ib->index_block_vcn = cpu_to_sle64(ib_vcn);//¶ÔÓ¦µÄvcnºÅ£¬´Ó0¿ªÊ¼µÝÔö
	
	ib->index.entries_offset = cpu_to_le32((ih_size +
			le16_to_cpu(ib->usa_count) * 2 + 7) & ~7);
	ib->index.index_length = 0;
	ib->index.allocated_size = cpu_to_le32(ib_size - 
					       (sizeof(INDEX_BLOCK) - ih_size));
	ib->index.flags = node_type;
	
	return ib;
}	

/** 
 *  Find the median by going through all the entries
 ÕÒµ½ÖÐ¼äÎ»ÖÃµÄindex entry
 */
static INDEX_ENTRY *ntfs_ie_get_median(INDEX_HEADER *ih)
{
	INDEX_ENTRY *ie, *ie_start;
	u8 *ie_end;
	int i = 0, median;
	
	ntfs_debug("Entering");
	
	ie = ie_start = ntfs_ie_get_first(ih);
	ie_end   = (u8 *)ntfs_ie_get_end(ih);
	
	while ((u8 *)ie < ie_end && !ntfs_ie_end(ie)) {
		ie = ntfs_ie_get_next(ie);
		i++;
	}
	/*
	 * NOTE: this could be also the entry at the half of the index block.
	 */
	median = i / 2 - 1;
	
	ntfs_debug("Entries: %d  median: %d\n", i, median);
	
	for (i = 0, ie = ie_start; i <= median; i++)
		ie = ntfs_ie_get_next(ie);
	
	return ie;
}

/**ÓÐ¹ØbitmapµÄ²Ù×÷*/
static s64 ntfs_ibm_vcn_to_pos(ntfs_index_context *icx, VCN vcn)
{
	return(div_s64(ntfs_ib_vcn_to_pos(icx, vcn),icx->block_size));	
}

static s64 ntfs_ibm_pos_to_vcn(ntfs_index_context *icx, s64 pos)
{
	return ntfs_ib_pos_to_vcn(icx, pos * icx->block_size);
}

/*¸øÄ¿Â¼½ÚµãÌí¼ÓAT_BITMAP 0XB0*/
static int ntfs_ibm_add(ntfs_index_context *icx)
{
	u8 bmp[8];

	ntfs_debug("Entering");
	
	if (!ntfs_attr_exist(icx->idx_ni, AT_BITMAP, I30,4))
		return STATUS_OK;
	/*
	 * AT_BITMAP must be at least 8 bytes.
	 */
	memset(bmp, 0, sizeof(bmp));
	if (ntfs_attr_add(icx->idx_ni, AT_BITMAP,I30,4,
			  bmp, sizeof(bmp))) {
		ntfs_debug("Failed to add AT_BITMAP");
		return STATUS_ERROR;
	}
	ntfs_debug("Done");
	return STATUS_OK;
}

/**ÐÞ¸Äindex ¶ÔÓ¦µÄbitmap*/
static int ntfs_ibm_modify(ntfs_index_context *icx, VCN vcn, int set)
{
#if 0
	s64 pos = ntfs_ibm_vcn_to_pos(icx, vcn);
	u32 bpos = pos >>3 ; // ³ýÒÔ8
	u32 bit = 1 << (pos & 7);		
	ntfs_inode * ni = icx->idx_ni;
	MFT_RECORD *m =NULL;
	ntfs_attr_search_ctx *ctx;
	ATTR_RECORD *a;
	u8 bm[2] = {0};	
	int ret = 0 ;
	u32 attr_len;	
	char *kattr;
#endif
	s64 pos = ntfs_ibm_vcn_to_pos(icx, vcn);
	//u32 bpos = pos >>3 ; // ³ýÒÔ8
	u32 bit = 1 << (pos & 7);	
	struct address_space  *bmp_mapping;
	struct page *bmp_page = NULL;
	u8 *bmp;
	struct inode *bmp_vi, *vdir; 
	struct super_block *sb ;
	int err =  0;
	
	ntfs_debug(" %s vcn: 0x%llx", set ? "set" : "clear", (long long)vcn);	

	vdir =  VFS_I(icx->idx_ni);
	ntfs_debug("Inode 0x%lx, getting index bitmap.", vdir->i_ino);

	sb =  vdir->i_sb;

	bmp_vi = ntfs_attr_iget(vdir, AT_BITMAP, I30, 4);	//¸ù¾ÝÊôÐÔÀàÐÍ£¬Éú³ÉÐÂµÄfake inode
	if (IS_ERR(bmp_vi)) {
		ntfs_error(sb, "Failed to get bitmap attribute.");
		err = PTR_ERR(bmp_vi);
		goto iput_err_out;
	}
	bmp_mapping = bmp_vi->i_mapping;	//µÃµ½bitmapµÄaddress mapping	
	
	bmp_page = ntfs_map_page(bmp_mapping,	//µÃµ½bitmapÖµ
			pos >> (3 + PAGE_CACHE_SHIFT));
	if (IS_ERR(bmp_page)) {
		ntfs_error(sb, "Reading index bitmap failed.");
		err = PTR_ERR(bmp_page);
		bmp_page = NULL;
		goto err_out;;
	}
	bmp = (u8*)page_address(bmp_page);
	ntfs_debug("before bm[0] = 0x%x ",*(bmp));	
	if (set) 
		*(bmp) |= bit;
	else
		*(bmp) &= ~bit;

	ntfs_debug("after bm[0] = 0x%x ",*(bmp));	
	flush_dcache_page(bmp_page);
	set_page_dirty(bmp_page);	
	lock_page(bmp_page);
	write_one_page(bmp_page,1);
	err  = 0;
err_out:
	if(bmp_page)
		ntfs_unmap_page(bmp_page);
iput_err_out:
	iput(bmp_vi);	
	ntfs_debug("Done");
	return err;
#if 0
	m = map_mft_record(ni);
	if (IS_ERR(m)) {
		ret = PTR_ERR(m);
		goto err_out;
	}
	ctx = ntfs_attr_get_search_ctx(ni, m);
	if (unlikely(!ctx)) {
		ret = -ENOMEM;
		goto unm_err_out;
	}
	ret = ntfs_attr_lookup(AT_BITMAP,I30,4,CASE_SENSITIVE, 0, NULL, 0, ctx);
	if (unlikely(ret)) {
		if (ret == -ENOENT)
			ret = -EIO;
		goto put_unm_err_out;
	}
	a = ctx->attr;
	//ntfs_debug("a->non_resident = %d",a->non_resident);	
	/* The total length of the attribute value. */
	DEBUG0_ON(a->non_resident);
	attr_len = le32_to_cpu(a->data.resident.value_length);
	//ntfs_debug("attr_len = 0x%x ,pos = 0x%llx, bpos = 0x%x, bit = 0x%x",attr_len,pos,bpos,bit);

	DEBUG0_ON(bpos > attr_len);	
	kattr = (u8*)a + le16_to_cpu(a->data.resident.value_offset);//ÊôÐÔµÄÖµ
	/* Copy the received data from the page to the mft record. */
	memcpy(bm, kattr + bpos, 1);	//ÏÈ¶ÁÈ¡bmµÄÖµ

	//ntfs_debug("before bm[0] = 0x%x ",bm[0]);	
	if (set) 
		bm[0] |= bit;
	else
		bm[0] &= ~bit;

	//ntfs_debug("after bm[0] = 0x%x ",bm[0]);	
	memcpy(kattr + bpos,bm,1);	//½«ÐÞ¸ÄºóµÄÄÚÈÝ¿½±´»ØÊôÐÔ

	/* Mark the mft record dirty, so it gets written back. */
	flush_dcache_mft_record_page(ctx->ntfs_ino);
	mark_mft_record_dirty(ctx->ntfs_ino);
	write_mft_record(ni,m,1);
	unmap_mft_record(ni);
	ntfs_attr_put_search_ctx(ctx);	
	ntfs_debug("Done.\n");
	return 0;

put_unm_err_out:
	ntfs_attr_put_search_ctx(ctx);
unm_err_out:
	unmap_mft_record(ni);
err_out:		
	ntfs_debug("Failed.\n");
	return ret;
#endif
}


static int ntfs_ibm_set(ntfs_index_context *icx, VCN vcn) //±ê¼ÇÎªÒÑÕ¼ÓÃ
{
	return ntfs_ibm_modify(icx, vcn, 1);
}

static int ntfs_ibm_clear(ntfs_index_context *icx, VCN vcn)//±ê¼ÇÎª¿ÕÏÐ
{
	return ntfs_ibm_modify(icx, vcn, 0);
}

static VCN ntfs_ibm_get_free(ntfs_index_context *icx)
{
	u8 *bm;
	int bit = 0;
	s64 vcn, byte, size;	
	
	//ntfs_debug("Entering");
	
	bm = ntfs_attr_readall(icx->idx_ni, AT_BITMAP, I30,4,
			       &size);
	if (!bm)
		return (VCN)-1;

	//ntfs_debug("bm[0] = 0x%x ,bm[1] = 0x%x ",bm[0],bm[1]);
	for (byte = 0; byte < size; byte++) {
		
		if (bm[byte] == 255)
			continue;
		
		for (bit = 0; bit < 8; bit++) {
			if (!(bm[byte] & (1 << bit))) {//find the free vcn
				vcn = ntfs_ibm_pos_to_vcn(icx, byte * 8 + bit);
				goto out;
			}
		}
	}	
	vcn = ntfs_ibm_pos_to_vcn(icx, size * 8);
out:	
	//ntfs_debug("bm size = 0x%llx ,byte = 0x%llx ,bit = 0x%x ",size,byte,bit);
	ntfs_debug("allocated vcn: 0x%llx\n", (long long)vcn);
	kfree(bm);	
	if (ntfs_ibm_set(icx, vcn))
		vcn = (VCN)-1;
	
	
	return vcn;
}

/*Éú³ÉÒ»¸öÐÂµÄindex_allocation,½«ÏÖÓÐµÄindex_rootµÄÄÚÈÝ¿½±´µ½index_allocation*/
static INDEX_BLOCK *ntfs_ir_to_ib(INDEX_ROOT *ir, VCN ib_vcn)
{
	INDEX_BLOCK *ib;
	INDEX_ENTRY *ie_last;
	char *ies_start, *ies_end;
	int i;
	ntfs_debug("Entering");

	//allocate a new INDEX_ALLOCATION 0xa0
	ib = ntfs_ib_alloc(ib_vcn, le32_to_cpu(ir->index_block_size), LEAF_NODE);
	if (!ib)
		return NULL;
	
	ies_start = (char *)ntfs_ie_get_first(&ir->index);
	ies_end   = (char *)ntfs_ie_get_end(&ir->index);
	ie_last   = ntfs_ie_get_last((INDEX_ENTRY *)ies_start, ies_end);
	/* 
	 * Copy all entries, including the termination entry
	 * as well, which can never have any data.
	 */
	i = (char *)ie_last - ies_start + le16_to_cpu(ie_last->length);
	memcpy(ntfs_ie_get_first(&ib->index), ies_start, i);
	
	ib->index.flags = ir->index.flags;
	ib->index.index_length = cpu_to_le32(i +
			le32_to_cpu(ib->index.entries_offset));
	//_Print_Buf("ir_ib",(unsigned char *)ib,600);

	ntfs_debug("Done\n");
	return ib;
}

static void ntfs_ir_nill(INDEX_ROOT *ir)
{
	INDEX_ENTRY *ie_last;
	char *ies_start, *ies_end;
	
	ntfs_debug("Entering");
	/*
	 * TODO: This function could be much simpler.
	 */
	ies_start = (char *)ntfs_ie_get_first(&ir->index);// the first INDEX ENTRY
	ies_end   = (char *)ntfs_ie_get_end(&ir->index);
	ie_last   = ntfs_ie_get_last((INDEX_ENTRY *)ies_start, ies_end);// the last INDEX ENTRY
	/* 
	 * Move the index root termination entry forward
	 * INDEX ROOTµÄÊ×Î²INDEX_ENTRY»¥»»,ÒòÎªÎ²INDEX_ENTRYÃ»ÓÐÊµ¼ÊÒâÒå£¬ËùÒÔÏàµ±ÓÚÇå¿ÕINDEX_ROOTµÄINDEX_ENTRYS
	 */
	if ((char *)ie_last > ies_start) {
		memmove(ies_start, (char *)ie_last, le16_to_cpu(ie_last->length));//½«the last INDEX ENTRY ¸²¸Çthe first INDEX ENTRY
		ie_last = (INDEX_ENTRY *)ies_start;//½«the first INDEX ENTRY¸øthe last INDEX ENTRY
	}
	ntfs_debug("Done");
}


/*@src ÊÇÔ­Ê¼µÄINDEX_BLOCK
   @median ÊÇ@srcµÄÖÐ¼äINDEX_ENTRY*/
static int ntfs_ib_copy_tail(ntfs_index_context *icx, INDEX_BLOCK *src,
			     INDEX_ENTRY *median, VCN new_vcn)
{
	u8 *ies_end;
	INDEX_ENTRY *ie_head;		/* first entry after the median */
	int tail_size, ret;
	INDEX_BLOCK *dst;

	ntfs_debug("Entering, new_vcn : 0x%llx,icx->pindex == 0x%x ",new_vcn,icx->pindex);

	dst = ntfs_ib_alloc(new_vcn, icx->block_size, //·ÖÅäÐÂµÄindex allocation,ÆävcnÎªnew_vcn
			    src->index.flags & NODE_MASK);
	if (!dst)
		return STATUS_ERROR;
	//ntfs_debug("ib->index_block_vcn :0x%llx ",dst->index_block_vcn );

	ie_head = ntfs_ie_get_next(median);//µÃµ½medianµÄÏÂÒ»¸öINDEX ENTRY
	_Print_Buf("ie_head",(unsigned char *) ie_head, ie_head->length);

	ies_end = (u8 *)ntfs_ie_get_end(&src->index);
	tail_size = ies_end - (u8 *)ie_head;//ºó°ë²¿·Ö?
	memcpy(ntfs_ie_get_first(&dst->index), ie_head, tail_size);//½«srcµÄºó°ë²¿·Ö¿½±´µ½dstÆðÊ¼´¦
	
	dst->index.index_length = cpu_to_le32(tail_size + 
					      le32_to_cpu(dst->index.entries_offset));
	ntfs_debug("index length: 0x%x",dst->index.index_length);
	ret = ntfs_ib_write(icx, dst,NULL);//½«dstÐ´Èë´ÅÅÌ
	kfree(dst);
	ntfs_debug("Done\n");
	return ret;
}

static int ntfs_ib_cut_tail(ntfs_index_context *icx, INDEX_BLOCK *ib,
			    INDEX_ENTRY *ie)
{
	char *ies_start, *ies_end;
	INDEX_ENTRY *ie_last, *ie_median;
	INDEX_BLOCK *dup;
	int ret = STATUS_ERROR;
	
	ntfs_debug("ib->index_block_vcn :0x%llx ",ib->index_block_vcn );
	ntfs_debug("icx->pindex == 0x%x ",icx->pindex);
	
	//median = ntfs_ie_dup(ie);//¸´ÖÆ
	//if (!median)
	//	return STATUS_ERROR;
	dup = kmalloc(le16_to_cpu(ib->index.index_length)+ 0x18,GFP_KERNEL);
	if (dup)
		memcpy(dup, ib, le16_to_cpu(ib->index.index_length)+ le32_to_cpu(ib->index.entries_offset)+0x18);
	else{
		//kfree(median);
		return STATUS_ERROR;	
	}

	ies_start = (char *)ntfs_ie_get_first(&dup->index);
	ies_end   = (char *)ntfs_ie_get_end(&dup->index);
	
	ie_last   = ntfs_ie_get_last((INDEX_ENTRY *)ies_start, ies_end);
	
	ie_median  = ntfs_ie_get_median(&dup->index);
	//if(memcmp(ie_median,median,median->length))
	//	ntfs_debug("sorry");
	if (ie_last->flags & INDEX_ENTRY_NODE)
		ntfs_ie_set_vcn(ie_last, ntfs_ie_get_vcn(ie_median));


	//_Print_Buf("median",ie_median,ie_median->length);
	
	memcpy(ie_median, ie_last, le16_to_cpu(ie_last->length));
	ntfs_debug("0x%x",((char *)ie_median - ies_start));
	dup->index.index_length = cpu_to_le32(((char *)ie_median - ies_start) + 
		le16_to_cpu(ie->length) + le32_to_cpu(dup->index.entries_offset));

	ntfs_debug("ie_last->length = 0x%x ,ib->index.index_length = 0x%x",ie_median->length,dup->index.index_length );
	ret = ntfs_ib_write(icx, dup,NULL);

	kfree(dup);
	ntfs_debug("ret = %d \n",ret);
	return ret;
}


/*½«0xa0 Ä¿Â¼Ë÷ÒýÊôÐÔÌí¼Óµ½Ä¿Â¼½Úµã**/
static int ntfs_ia_add(ntfs_index_context *icx)
{
	ntfs_debug("Entering");	

	if (ntfs_ibm_add(icx))
		return -1;	

	if (ntfs_attr_exist(icx->idx_ni, AT_INDEX_ALLOCATION, I30, 4)) {//ÊôÐÔ²»´æÔÚ	
		if ((ntfs_attr_add(icx->idx_ni, AT_INDEX_ALLOCATION,I30, 4, NULL, 0)) !=0) {
			ntfs_debug("Failed to add AT_INDEX_ALLOCATION");
			return -1;
		}
	}
	/**FIXEME:still not sure*/
#if 0
	icx->ia_na = ntfs_ia_open(icx, icx->idx_ni);
	if (!icx->ia_na)
		return -1;
#endif
	ntfs_debug("Done.");
	return 0;
}

static int ntfs_ir_reparent(ntfs_index_context *icx)
{
	ntfs_attr_search_ctx *ctx = NULL;
	INDEX_ROOT *ir;
	INDEX_ENTRY *ie;
	INDEX_BLOCK *ib = NULL;
	VCN new_ib_vcn;
	int ret = STATUS_ERROR;

	ntfs_debug("Entering");
	
	ir = ntfs_ir_lookup2(icx->idx_ni, I30, 4);
	if (!ir)
		goto out;
	
	if ((ir->index.flags & NODE_MASK) == SMALL_INDEX)//Ð¡Ä¿Â¼
		if (ntfs_ia_add(icx))//Ìí¼Ó0xa0ÊôÐÔ
			goto out;
	
	new_ib_vcn = ntfs_ibm_get_free(icx);//µÃµ½¿ÕµÄVCN(Í¨¹ý¼ìË÷bitmap)
	if (new_ib_vcn == -1)
		goto out;
		
	ir = ntfs_ir_lookup2(icx->idx_ni,I30,4);
	if (!ir)
		goto clear_bmp;
	
	ib = ntfs_ir_to_ib(ir, new_ib_vcn);//¸ù¾Ýir´´½¨ÐÂµÄib
	if (ib == NULL) {
		ntfs_debug("Failed to move index root to index block");
		goto clear_bmp;
	}
	if (ntfs_ib_write(icx, ib,NULL))//½«index_allocationÐ´Èë´ÅÅÌ?
		goto clear_bmp;
		
	ir = ntfs_ir_lookup(icx->idx_ni, I30, 4, &ctx);
	if (!ir)
		goto clear_bmp;
	
	ntfs_ir_nill(ir);//Çå¿ÕÔ­INDEX_ROOT
	
	ie = ntfs_ie_get_first(&ir->index);
	ie->flags |= INDEX_ENTRY_NODE;
	ie->length = cpu_to_le16(sizeof(INDEX_ENTRY_HEADER) + sizeof(VCN));
	
	ir->index.flags = LARGE_INDEX;
	ir->index.index_length = cpu_to_le32(le32_to_cpu(ir->index.entries_offset)
					     + le16_to_cpu(ie->length));
	ir->index.allocated_size = ir->index.index_length;
	
	if (ntfs_resident_attr_value_resize(ctx->mrec, ctx->attr,//¸Ä±äINDEX_ROOTÔÚmft reordÖÐµÄ¼ÇÂ¼´óÐ¡
			sizeof(INDEX_ROOT) - sizeof(INDEX_HEADER) +
			le32_to_cpu(ir->index.allocated_size)))
		/* FIXME: revert index root */
		goto clear_bmp;
	/*
	 *  FIXME: do it earlier if we have enough space in IR (should always),
	 *  so in error case we wouldn't lose the IB.
	 */
	ntfs_ie_set_vcn(ie, new_ib_vcn);

	ret = STATUS_OK;
err_out:
	if(ib)
		kfree(ib);
	if(ctx)
		ntfs_attr_put_search_ctx(ctx);
out:
	ntfs_debug("DOne");
	return ret;
clear_bmp:
	ntfs_ibm_clear(icx, new_ib_vcn);
	goto err_out;
}


/**¸Ä±äÊôÐÔ0x90µÄ´óÐ¡
 * ntfs_ir_truncate - Truncate index root attribute
 * 
 * Returns STATUS_OK, STATUS_RESIDENT_ATTRIBUTE_FILLED_MFT or STATUS_ERROR.
 */
static int ntfs_ir_truncate(ntfs_index_context *icx, int data_size)
{			  
	//ntfs_attr_3g *na_3g;
	int ret = 0;
	
	ntfs_debug("Entering");
	/*
	 *  INDEX_ROOT must be resident and its entries can be moved to 
	 *  INDEX_BLOCK, so ENOSPC isn't a real error.
	 */
	ret = ntfs_attr_truncate(icx->idx_ni,AT_INDEX_ROOT,I30, 4,data_size + offsetof(INDEX_ROOT, index));
	if (ret == 0) {		
		icx->ir = ntfs_ir_lookup2(icx->idx_ni,I30, 4);
		if (!icx->ir)
			return -1;	
		icx->ir->index.allocated_size = cpu_to_le32(data_size);//¸üÐÂ·ÖÅä´óÐ¡
		ntfs_debug("Done, new_size  =0x%x",le32_to_cpu(icx->ir->index.allocated_size));
	
	} else 
		ntfs_debug("Failed to truncate INDEX_ROOT");	
	return ret;

#if 0
	na_3g = ntfs_attr_open(icx->idx_ni, AT_INDEX_ROOT, icx->name, icx->name_len);
	if (!na_3g) {
		ntfs_debug("Failed to open INDEX_ROOT");
		return STATUS_ERROR;
	}
	/*
	 *  INDEX_ROOT must be resident and its entries can be moved to 
	 *  INDEX_BLOCK, so ENOSPC isn't a real error.
	 */
	ret = ntfs_attr_truncate(na_3g, data_size + offsetof(INDEX_ROOT, index));
	if (ret == STATUS_OK) {
		
		icx->ir = ntfs_ir_lookup2(icx->idx_ni, icx->name, icx->name_len);
		if (!icx->ir)
			return STATUS_ERROR;
	
		icx->ir->index.allocated_size = cpu_to_le32(data_size);//¸üÐÂ·ÖÅä´óÐ¡
		
	} else if (ret == STATUS_ERROR)
		ntfs_debug("Failed to truncate INDEX_ROOT");
	
	ntfs_attr_close(na_3g);
	return ret;
#endif
}
		
/**
 * ntfs_ir_make_space - Make more space for the index root attribute:0x90
 * 
 * On success return STATUS_OK or STATUS_KEEP_SEARCHING.
 * On error return STATUS_ERROR.
 */
static int ntfs_ir_make_space(ntfs_index_context *icx, int data_size)
{			  
	int ret;
	
	ntfs_debug("Entering");

	ret = ntfs_ir_truncate(icx, data_size);
	if (ret == STATUS_RESIDENT_ATTRIBUTE_FILLED_MFT) {//INDEX_root --->index_allocation
	
		ret = ntfs_ir_reparent(icx);
		if (ret == STATUS_OK)
			ret = STATUS_KEEP_SEARCHING;
		else
			ntfs_debug("Failed to nodify INDEX_ROOT");
	}
	ntfs_debug("Done\n");
	return ret;
}

/*
 * NOTE: 'ie' must be a copy of a real index entry.
 * ¸øINDEX ENTRY À©´ósizeof(VCN)µÄ¿Õ¼ä
 */
static int ntfs_ie_add_vcn(INDEX_ENTRY **ie)
{
	INDEX_ENTRY *p, *old = *ie;

	 ntfs_debug("Entering");
	old->length = cpu_to_le16(le16_to_cpu(old->length) + sizeof(VCN));
	p = krealloc(old, le16_to_cpu(old->length),GFP_KERNEL);
	if (!p)
		return STATUS_ERROR;
	
	p->flags |= INDEX_ENTRY_NODE;
	*ie = p;

	return STATUS_OK;
}
/*½«@orig_ieÌí¼Óµ½ihÖÐ*/
static int ntfs_ih_insert(INDEX_HEADER *ih, INDEX_ENTRY *orig_ie, VCN new_vcn, 
			  int pos)
{
	INDEX_ENTRY *ie_node, *ie;
	int ret = STATUS_ERROR;
	VCN old_vcn;
	
	ntfs_debug("Entering, new_vcn  = %lld, pos = %d",new_vcn, pos);
	
	ie = ntfs_ie_dup(orig_ie);//¸´ÖÆ
	if (!ie)
		return STATUS_ERROR;
	
	if (!(ie->flags & INDEX_ENTRY_NODE))
		if (ntfs_ie_add_vcn(&ie))
			goto out;

	ie_node = ntfs_ie_get_by_pos(ih, pos);//µÃµ½µÚpos¸öINDEX_ENTRY
	old_vcn = ntfs_ie_get_vcn(ie_node);//µÃµ½INDEX_ENTRYµÄvcn
	ntfs_ie_set_vcn(ie_node, new_vcn);//½«INDEX_ENTRYµÄvcnÉèÎªnew_vcn
	//_Print_Buf("ie_node",(unsigned char *) ie_node, ie_node->length);
	ntfs_debug("old_vcn : %lld ",old_vcn);
	ntfs_ie_insert(ih, ie, ie_node);//½«´ý²åÈëµÄINDEX_ENTRYÌí¼Óµ½ie_nodeµÄÎ»ÖÃ
	ntfs_ie_set_vcn(ie_node, old_vcn);//»¹Ô­ie_nodeµÄvcn
	//_Print_Buf("ie_node",(unsigned char *) ie_node, ie_node->length);

	ret = STATUS_OK;
out:	
	kfree(ie);
	ntfs_debug("ret= %d",ret);
	return ret;
}

static VCN ntfs_icx_parent_vcn(ntfs_index_context *icx)
{
	ntfs_debug("icx->pindex : 0x%x,parent_vcn :0x%llx, parent_pos :0x%x",icx->pindex,icx->parent_vcn[icx->pindex],icx->parent_pos[icx->pindex]);
	return icx->parent_vcn[icx->pindex];
}

static VCN ntfs_icx_parent_pos(ntfs_index_context *icx)
{
	ntfs_debug("icx->pindex : 0x%x,parent_vcn :0x%llx, parent_pos :0x%x",icx->pindex,icx->parent_vcn[icx->pindex],icx->parent_pos[icx->pindex]);
	return icx->parent_pos[icx->pindex];
}

/*½«@median²åÈëµ½@icxµÄINDEX_ROOTÖÐ£¬new_vcnÊÇmedianµÄrunlist´æ·Åvcn*/
static int ntfs_ir_insert_median(ntfs_index_context *icx, INDEX_ENTRY *median,
				 VCN new_vcn)
{
	u32 new_size;
	int ret;

	ntfs_debug("icx->pindex == 0x%x ",icx->pindex);

	icx->ir = ntfs_ir_lookup2(icx->idx_ni, I30, 4);
	if (!icx->ir)
		return STATUS_ERROR;

	new_size = le32_to_cpu(icx->ir->index.index_length) + 
			le16_to_cpu(median->length);
	if (!(median->flags & INDEX_ENTRY_NODE))
		new_size += sizeof(VCN);

	ret = ntfs_ir_make_space(icx, new_size);//À©Õ¹INDEX_ROOTÊôÐÔ
	if (ret != STATUS_OK)
		return ret;
	
	icx->ir = ntfs_ir_lookup2(icx->idx_ni, I30, 4);
	if (!icx->ir)
		return STATUS_ERROR;

	return ntfs_ih_insert(&icx->ir->index, median, new_vcn, //²åÈëµ½INDEX_HEADER
			      ntfs_icx_parent_pos(icx));
}

/**
 * ntfs_index_lookup - find a key in an index and return its index entry
 * @key:	[IN] key for which to search in the index
 * @key_len:	[IN] length of @key in bytes
 * @icx:	[IN/OUT] context describing the index and the returned entry
 *
 * Before calling ntfs_index_lookup(), @icx must have been obtained from a
 * call to ntfs_index_ctx_get().
 *
 * Look for the @key in the index specified by the index lookup context @icx.
 * ntfs_index_lookup() walks the contents of the index looking for the @key.
 *
 * If the @key is found in the index, 0 is returned and @icx is setup to
 * describe the index entry containing the matching @key.  @icx->entry is the
 * index entry and @icx->data and @icx->data_len are the index entry data and
 * its length in bytes, respectively.
 *
 * If the @key is not found in the index, -1 is returned, errno = ENOENT and
 * @icx is setup to describe the index entry whose key collates immediately
 * after the search @key, i.e. this is the position in the index at which
 * an index entry with a key of @key would need to be inserted.
 *
 * If an error occurs return -1, set errno to error code and @icx is left
 * untouched.
 *
 * When finished with the entry and its data, call ntfs_index_ctx_put() to free
 * the context and other associated resources.
 *
 * If the index entry was modified, call ntfs_index_entry_mark_dirty() before
 * the call to ntfs_index_ctx_put() to ensure that the changes are written
 * to disk.
 */
int ntfs_index_lookup3(const void *key, const int key_len, ntfs_index_context *icx)
{
	VCN old_vcn, vcn;
	ntfs_inode *ni = icx->idx_ni;
	ntfs_volume *vol = ni->vol;
	struct super_block *sb = vol->sb;
	INDEX_ROOT *ir;
	INDEX_ENTRY *ie;
	INDEX_BLOCK *ib = NULL;
	int ret =  0;
	struct page * page =NULL;
	
	ntfs_debug("Entering.----------------------------------------------------------");

	if (!key || key_len <= 0) {		
		ntfs_error(sb,"key: %p  key_len: %d", key, key_len);
		return -1;
	}

	ir = ntfs_ir_lookup(ni,I30, 4,&icx->actx);//find the Index Root
	if (!ir) 	
		return -1;
	
	
	icx->block_size = le32_to_cpu(ir->index_block_size);
	if (icx->block_size < NTFS_BLOCK_SIZE) {		
		ntfs_error(sb,"Index block size (%d) is smaller than the "
				"sector size (%d)", icx->block_size, NTFS_BLOCK_SIZE);
		goto err_out;
	}

	if (ni->vol->cluster_size <= icx->block_size)
		icx->vcn_size_bits = ni->vol->cluster_size_bits;
	else
		icx->vcn_size_bits = ni->vol->sector_size_bits;

	old_vcn = VCN_INDEX_ROOT_PARENT;
	/* 
	 * FIXME: check for both ir and ib that the first index entry is
	 * within the index block.
	 */
	ret = ntfs_ie_lookup(key, key_len, icx, &ir->index, &vcn, &ie);
	if (ret == STATUS_ERROR) {		
		goto err_out;
	}
	
	icx->ir = ir;
	
	if (ret != STATUS_KEEP_SEARCHING) {
		/* STATUS_OK or STATUS_NOT_FOUND */		
		ntfs_debug("is_in_root = 1");
		icx->is_in_root = true;
		icx->parent_vcn[icx->pindex] = old_vcn;
		goto done;
	}
	
	//ÐèÒª½øÈëB+ treeµÄÏÂÒ»¼¶ËÑË÷
	/* Child node present, descend into it. */
/*
	ib = kmalloc(icx->block_size,GFP_KERNEL);
	if (!ib) {		
		goto err_out;
	}
*/	
descend_into_child_node:

	icx->parent_vcn[icx->pindex] = old_vcn;
	if (ntfs_icx_parent_inc(icx)) {		
		goto err_out;
	}
	old_vcn = vcn;//ÐèÒªËÑË÷µÄÏÂÒ»¼¶B+ TREEµÄvcn

	ntfs_debug("Descend into node with VCN :0x%llx", (long long)vcn);

	page = ntfs_ib_read(icx, vcn, &ib);	
	if (!page)//¶ÁÈ¡vcn¶ÔÓ¦µÄindex block
		goto err_out;
	//_Print_Buf("ib2",ib,600);
	

	ret = ntfs_ie_lookup(key, key_len, icx, &ib->index, &vcn, &ie);
	if (ret != STATUS_KEEP_SEARCHING) {		
		if (ret == STATUS_ERROR)
			goto err_out;
		
		/* STATUS_OK or STATUS_NOT_FOUND */
		icx->is_in_root = false;
		icx->ia = ib;
		icx->page = page;
		icx->parent_vcn[icx->pindex] = vcn;//index entryËùÔÚµÄindex_block¶ÔÓ¦µÄvcn
		goto done;
	}

	if ((ib->index.flags & NODE_MASK) == LEAF_NODE) {
		ntfs_error(sb,"Index entry with child node found in a leaf "
			       "node in inode 0x%llx.\n",
			       (unsigned long long)ni->mft_no);
		goto err_out;
	}
	ntfs_debug("unmap page :0x%lx ",page->index);
	unlock_page(page);
	ntfs_unmap_page(page);
	goto descend_into_child_node;
err_out:
	//kfree(ib);	
	return -1;
done:
	icx->entry = ie;
	icx->data = (u8*)ie +
			le16_to_cpu(ie->data.vi.data_offset);
	icx->data_len = le16_to_cpu(ie->data.vi.data_length);
	ntfs_debug("Done.,ret = %d-------------------------------------------------------\n",ret);
	return ret;
	

}
static int ntfs_ib_split(ntfs_index_context *icx, INDEX_BLOCK *ib);
/**½«INDEX_ENTRYÌí¼Óµ½INDEXT_ALLOCATION
 * On success return STATUS_OK or STATUS_KEEP_SEARCHING.
 * On error return STATUS_ERROR.
 */
static int ntfs_ib_insert(ntfs_index_context *icx, INDEX_ENTRY *ie, VCN new_vcn)
{			  
	INDEX_BLOCK *ib;
	u32 idx_size, allocated_size;
	int err = STATUS_ERROR;
	VCN old_vcn;
	struct page *page;
	//u8 *kaddr;

	ntfs_debug("Entering");
/*	ib = kmalloc(icx->block_size,GFP_KERNEL);
	if (!ib)
		return STATUS_ERROR;
	memset(ib,0,icx->block_size);
*/	
	//_Print_Buf("index block",ib,512);
	old_vcn = ntfs_icx_parent_vcn(icx);	 
	page = ntfs_ib_read(icx, old_vcn,&ib);
	if (!page)//¶ÁÈ¡Æäblock 
		goto err_out;

	//_Print_Buf("index block",ib,512);
	ntfs_debug("ib->index_block_vcn :0x%llx ",ib->index_block_vcn );
	
	idx_size       = le32_to_cpu(ib->index.index_length);
	allocated_size = le32_to_cpu(ib->index.allocated_size);
	ntfs_debug("idx_size = 0x%x ,allocated_size = 0x%x ",idx_size,allocated_size);
	/* FIXME: sizeof(VCN) should be included only if ie has no VCN */
	if (idx_size + le16_to_cpu(ie->length) + sizeof(VCN) > allocated_size) {//À©Õ¹0xa0
		ntfs_debug("µÝ¹é");
		err = ntfs_ib_split(icx, ib);
		if (err == STATUS_OK)
			err = STATUS_KEEP_SEARCHING;
		goto err_out;
	}	
	if (ntfs_ih_insert(&ib->index, ie, new_vcn, ntfs_icx_parent_pos(icx)))
		goto err_out;

/*	kaddr = (u8*)page_address(icx->page);	
	memcpy(kaddr,ib,PAGE_SIZE);
	flush_dcache_page(page);
	set_page_dirty(page);
	lock_page(page);
	write_one_page(page,1);*/
	if (ntfs_ib_write(icx, ib,page))//Ìí¼ÓÍêºó£¬Ðë½«¸Ä±äµÄINDEX_BLOCKÐ´»Ø´ÅÅÌ
		goto err_out;

	if(!icx->page || page->index != icx->page->index){
		ntfs_debug("unmap page :0x%lx ",page->index);
		unlock_page(page);
		ntfs_unmap_page(page);
	}
	err = STATUS_OK;
err_out:	
	ntfs_debug("Done, err= %d\n ",err);	
//	if(ib)
//		kfree(ib);	 
	
	return err;
}

/**
 * ntfs_ib_split - Split an index block, À©Õ¹ index block
 * 
 * On success return STATUS_OK or STATUS_KEEP_SEARCHING.
 * On error return is STATUS_ERROR.
 */
static int ntfs_ib_split(ntfs_index_context *icx, INDEX_BLOCK *ib)
{			  
	INDEX_ENTRY *median;
	VCN new_vcn;
	int ret;	
	
	ntfs_debug("Entering");

	if (ntfs_icx_parent_dec(icx))//??
		return STATUS_ERROR;
	
	median  = ntfs_ie_get_median(&ib->index);//µÃµ½@ibÖÐ¼äÎ»ÖÃµÄindex entry	
	//_Print_Buf("median",(unsigned char *) median, median->length);
	new_vcn = ntfs_ibm_get_free(icx);//ÔÚ0xb0 bitmapÖÐÕÒµ½¿ÕÏÐµÄvcn
	if (new_vcn == -1)
		return STATUS_ERROR;
	
	if (ntfs_ib_copy_tail(icx, ib, median, new_vcn)) { //½«@ibµÄºó°ë²¿·Ö¿½±´µ½ÐÂµÄINDEX BLOCK£¬²¢½«ÐÂINDEX BLOCK¸üÐÂÖÁ´ÅÅÌ
		ntfs_ibm_clear(icx, new_vcn);	
		return STATUS_ERROR;
	}
	
	if (ntfs_icx_parent_vcn(icx) == VCN_INDEX_ROOT_PARENT)
		ret = ntfs_ir_insert_median(icx, median, new_vcn);//Ìí¼Óµ½INDEX_ROOT
	else
		ret = ntfs_ib_insert(icx, median, new_vcn);//½«medianÌí¼Óµ½@ib
	
	if (ret != STATUS_OK) {
		ntfs_ibm_clear(icx, new_vcn);		
		return ret;
	}
	
	ret = ntfs_ib_cut_tail(icx, ib, median);//¼ôÇÐµô@ibµÄºó°ë²¿·Ö
	ntfs_debug("Done, ret: %d\n",ret);
	return ret;
}

/*insert the INDEX_ENTRY to the icx*/
int ntfs_ie_add(ntfs_index_context *icx, INDEX_ENTRY *ie)
{
	INDEX_HEADER *ih;
	int allocated_size, new_size;
	int ret = 0;	
	
	ntfs_debug("Entering.");
	
#ifdef DEBUG
/* removed by JPA to make function usable for security indexes
	char *fn;
	fn = ntfs_ie_filename_get(ie);
	ntfs_debug("file: '%s'\n", fn);
	ntfs_attr_name_free(&fn);
*/
#endif
	
	while (1) {
		if ((ret = ntfs_index_lookup3(&ie->key, le16_to_cpu(ie->key_length), icx)) == 0) {
			ret = -EEXIST;
			ntfs_debug("Index already have such entry");
			goto err_out;
		}
		else{
			if (ret != STATUS_NOT_FOUND) {
				ntfs_debug("Failed to find place for new entry");
				goto err_out;
			}
		}
		/**ok,find place for new entry*/		

		/**find the INDEX_HEADER*/
		if (icx->is_in_root)
			ih = &icx->ir->index;//0x90
		else
			ih = &icx->ia->index;//0xa0		

		allocated_size = le32_to_cpu(ih->allocated_size);
		new_size = le32_to_cpu(ih->index_length) + le16_to_cpu(ie->length);
		
		ntfs_debug("index block sizes: allocated: 0x%x  needed: 0x%x\n",
			       allocated_size, new_size);
		if (new_size <= allocated_size)//enough space, donot need to allocate
		{		
			ntfs_debug("make space ok.");
			break;	
		}		

		/**make new room in index_root or index_allocation for the new index entry*/
		if (icx->is_in_root) {
			if (ntfs_ir_make_space(icx, new_size) == STATUS_ERROR)
				goto err_out;
		} else {
			if (ntfs_ib_split(icx, icx->ia) == STATUS_ERROR)
				goto err_out;
		}

		
		/* Finally, mark the mft record dirty, so it gets written back. */
		/**FIXEME: all the NIoSet things */
		//ntfs_inode_mark_dirty(icx->actx->ntfs_ino);			
		ntfs_index_ctx_reinit(icx);
		
	}
	ntfs_ie_insert(ih, ie, icx->entry);//insert the new INDEX_RENTRY--ie to the index tree

	ntfs_index_entry_mark_dirty(icx);
	ret = STATUS_OK;
err_out:
	ntfs_debug("%s\n", ret ? "Failed" : "Done");
	return ret;
}

/**½«ÎÄ¼þÃûÊôÐÔ0x30 Ìí¼Óµ½Ä¿Â¼Ë÷Òý
 * ntfs_index_add_filename - add filename to directory index
 * @ni:		ntfs inode describing directory to which index add filename
 * @fn:		FILE_NAME attribute to add
 * @mref:		reference of the inode which @fn describes
 *
 * Return 0 on success or -1 on error with errno set to the error code.
 */
int ntfs_index_add_filename(ntfs_inode *ni, FILE_NAME_ATTR *fn, MFT_REF mref)
{
	INDEX_ENTRY *ie;
	ntfs_index_context *icx;
	int fn_size, ie_size, ret = -1;

	ntfs_debug("Entering");
	
	if (!ni || !fn) {
		ntfs_debug("Invalid arguments.");		
		return -EINVAL;
	}
	
	/**build the INDEX_ENTRY accroding to @fn*********/
	fn_size = (fn->file_name_length * sizeof(ntfschar)) +
			sizeof(FILE_NAME_ATTR);
	/*INDEX_ENTRY_HEADER+$FileName == INDEX_ENTRY*/
	ie_size = (sizeof(INDEX_ENTRY_HEADER) + fn_size + 7) & ~7;//Ä¿Â¼ÖÐindex entryµÄ´óÐ¡:ÒÔ8×Ö½Ú¶ÔÆë
	
	ie = kcalloc(1,ie_size,GFP_KERNEL);
	if (!ie)
		return -ENOMEM;

	ie->data.dir.indexed_file = cpu_to_le64(mref);//ÎÄ¼þµÄmft ²Î¿¼ºÅ?
	ie->length 	 = cpu_to_le16(ie_size);//Èç$Attrdef : 0x68
	ie->key_length 	 = cpu_to_le16(fn_size);//Èç$Attrdef : 0x52
	memcpy(&ie->key, fn, fn_size);// keyÖÐ´æ·Å$FileName
	
	//icx = ntfs_index_ctx_get(ni, NTFS_INDEX_I30, 4);
	icx = ntfs_index_ctx_get(ni);
	if (!icx)
		goto out;
	//_Print_Buf("ie->key",(unsigned char *)ie->key.file_name.file_name,ie->key.file_name.file_name_length);

	ret = ntfs_ie_add(icx, ie);//½«@fn¶ÔÓ¦µÄindex entry--ieÌí¼Óµ½inode
	//err = errno;

	/* ntfs_index_entry_mark_dirty() has been called in ntfs_ie_add()*/
	ntfs_index_ctx_put(icx);
	//errno = err;
out:
	kfree(ie);	
	ntfs_debug("Done\n");	
	return ret;
}
//Èç¹ûieÎª·ÇÒ¶×Ó½Úµã£¬½«Æä×Ó½ÚµãÇå¿Õ
static int ntfs_ih_takeout(ntfs_index_context *icx, INDEX_HEADER *ih,
			   INDEX_ENTRY *ie, INDEX_BLOCK *ib,struct page *page,char *flag)
{
	INDEX_ENTRY *ie_roam;
	int ret = STATUS_ERROR;
	
	ntfs_debug("Entering");
	
	ie_roam = ntfs_ie_dup_novcn(ie);
	if (!ie_roam)
		return STATUS_ERROR;

	ntfs_ie_delete(ih, ie);//É¾³ý@ie ÒÔ¼°ÆäÏÂ×Ó½ÚµãËùÓÐµÄ¼ÇÂ¼

	if (ntfs_icx_parent_vcn(icx) == VCN_INDEX_ROOT_PARENT)
		ntfs_inode_mark_dirty(icx->actx->ntfs_ino);
	else{
		if(ntfs_ib_write(icx, ib,page))
			goto out;
	}

	if(page){
		if(!icx->page ||(icx->page &&  (page->index != icx->page->index))){
			ntfs_debug("unmap page :0x%lx ",page->index);
			unlock_page(page);
			ntfs_unmap_page(page);
			page = NULL;
			*flag = 1;
		}else if(icx->page && (icx->page &&  (page->index == icx->page->index)))
			*flag = 1;
	}
	ntfs_index_ctx_reinit(icx);

	ret = ntfs_ie_add(icx, ie_roam);//½«ÊÂÏÈ¸´ÖÆºÃµÄ²»°üº¬×Ó½ÚµãµÄINDEX ENTRYÌí¼Óµ½IA
out:
	kfree(ie_roam);
	return ret;
}

/**µ±Ä³index root½öÓÐÎ¨Ò»µÄentry£¬µ«¸Ãentry¶ÔÓ¦µÄindex block±»É¾³ýÁË
 *  Used if an empty index block to be deleted has END entry as the parent
 *  in the INDEX_ROOT which is the only one there.
 */
static void ntfs_ir_leafify(ntfs_index_context *icx, INDEX_HEADER *ih)
{
	INDEX_ENTRY *ie;
	
	ntfs_debug("Entering");
	
	ie = ntfs_ie_get_first(ih);
	ie->flags &= ~INDEX_ENTRY_NODE;
	ie->length = cpu_to_le16(le16_to_cpu(ie->length) - sizeof(VCN));
	
	ih->index_length = cpu_to_le32(le32_to_cpu(ih->index_length) - sizeof(VCN));
	ih->flags &= ~LARGE_INDEX;
	
	/* Not fatal error */
	ntfs_ir_truncate(icx, le32_to_cpu(ih->index_length));
	ntfs_debug("Done");
}
/**µ±Ä³index rootµÄÄ©Î²entry¶ÔÓ¦µÄblock±»É¾³ýºó¡£ÐÒÔËµØÊÇ¸Ãindex root²»Ö¹¸ÃÒ»¸öentry
 * Ôò¸Ãentry¼Ì³ÐÆäÇ°Ò»¸öentryµÄ×Ó½Úµã£¬È»ºó½«Ç°entryÍÚ¿Õ
 *  Used if an empty index block to be deleted has END entry as the parent 
 *  in the INDEX_ROOT which is not the only one there.
 */
static int ntfs_ih_reparent_end(ntfs_index_context *icx, INDEX_HEADER *ih,
				INDEX_BLOCK *ib,struct page *page,char *flag)
{
	INDEX_ENTRY *ie, *ie_prev;
	
	ntfs_debug("Entering\n");
	
	ie = ntfs_ie_get_by_pos(ih, ntfs_icx_parent_pos(icx));
	ie_prev = ntfs_ie_prev(ih, ie);//µÃµ½Ç°Ò»¸öentry
	
	ntfs_ie_set_vcn(ie, ntfs_ie_get_vcn(ie_prev));//ÖØÐÂÉèÖÃ±»É¾³ý×Ó½Úµãenty µÄvcn
	
	return ntfs_ih_takeout(icx, ih, ie_prev, ib,page,flag);//ÔõÃ´°ÑÇ°Ò»¸öentryµÄ×Ó½ÚµãÍÚ¿Õ?
}

static int ntfs_index_rm_leaf(ntfs_index_context *icx)
{
	INDEX_BLOCK *ib = NULL;
	INDEX_HEADER *parent_ih;
	INDEX_ENTRY *ie;
	int ret = STATUS_ERROR;
	struct page *page =NULL;
	char flag = 0;
	
	ntfs_debug("pindex: %d", icx->pindex);
	
	if (ntfs_icx_parent_dec(icx))
		return STATUS_ERROR;

	if (ntfs_ibm_clear(icx, icx->parent_vcn[icx->pindex + 1]))//½«Õû¸öindex blockÉèÖÃ¿ÕÏÐ?
		return STATUS_ERROR;

	//µÃµ½Ç°Ò»¼¶index treeµÄINDEX HEADER
	if (ntfs_icx_parent_vcn(icx) == VCN_INDEX_ROOT_PARENT)//INDEX ROOT
		parent_ih = &icx->ir->index;
	else {											    //INDEX ALLOCATION
	/*	ib = kmalloc(icx->block_size,GFP_KERNEL);
		if (!ib)
			return STATUS_ERROR;
			*/
		page = ntfs_ib_read(icx, ntfs_icx_parent_vcn(icx),&ib);
		if (!page)//¶ÁÈ¡Æäblock 
			goto out;
		
		parent_ih = &ib->index;
	}

	//µÃµ½±»É¾³ýindex blockµÄ¸¸entry
	ie = ntfs_ie_get_by_pos(parent_ih, ntfs_icx_parent_pos(icx));
	if (!ntfs_ie_end(ie)) {				//²»ÊÇÄ©Î²INEX ENTRY
		ret = ntfs_ih_takeout(icx, parent_ih, ie, ib,page,&flag);//Ôò½«Æä×Ó½Úµã¼ÇÂ¼Çå¿Õ
		goto out;
	}

	//±»É¾³ýindex blockµÄ¸¸entry¸ÕºÃÒ²ÊÇÎ¨Ò»µÄentry
	if (ntfs_ih_zero_entry(parent_ih)) {//É¾³ýºó£¬Ç°Ò»¼¶index Ã»ÓÐINDEX ENTRYÁË		
		if (ntfs_icx_parent_vcn(icx) == VCN_INDEX_ROOT_PARENT) {//¸Ã¼¶¸ÕºÃÊÇindex root
			INDEX_HEADER *ih;
			ntfs_debug("cut the index allocation size");			
			ih = &icx->ia->index;
			ih->index_length -=  icx->entry->length; //learn from pc ntfs
			ntfs_index_entry_mark_dirty(icx);
			ntfs_ir_leafify(icx, parent_ih); //½«index rootÇå¿Õ
			goto ok;
		}
		
		ret = ntfs_index_rm_leaf(icx);//µÝ¹é!
		goto out;
	}

	//±»É¾³ýindex blockµÄ¸¸entryÊÇÄ©Î²entryµ«²»ÊÇÎ¨Ò»µÄentry
	if (ntfs_ih_reparent_end(icx, parent_ih, ib,page,&flag))
		goto out;
ok:	
	ret = STATUS_OK;
out:
//	if(ib)
//		kfree(ib);
	if(page && !flag){
		if(!icx->page ||(icx->page &&  (page->index != icx->page->index))){
			ntfs_debug("unmap page :0x%lx ",page->index);
			unlock_page(page);
			ntfs_unmap_page(page);
		}
	}
	if(flag)
		if(page)
			ntfs_debug(" ");
		
	ntfs_debug("Done");
	return ret;
}

/*INDEX ENTRY°üº¬×Ó½Úµã: Æ©ÈçÄ¿Â?µ«²»ÊÇ²»¿ÉÒÔÉ¾³ý·Ç¿ÕÄ¿Â¼Âð*/
static int ntfs_index_rm_node(ntfs_index_context *icx)
{
	int entry_pos, pindex;
	VCN vcn;
	INDEX_BLOCK *ib = NULL;
	INDEX_ENTRY *ie_succ, *ie, *entry = icx->entry;
	INDEX_HEADER *ih;
	u32 new_size;
	int delta, ret = STATUS_ERROR;
	ntfs_attr_search_ctx *ctx;
	MFT_RECORD *mrec;
	ntfs_inode * ni = icx->idx_ni;
	struct page *page;
	
	ntfs_debug("Entering\n");
	
	mrec  = map_mft_record(ni);
	if (IS_ERR(mrec)) {		
		ntfs_error(VFS_I(ni)->i_sb, "Failed to map mft record for inode 0x%lx "
				, VFS_I(ni)->i_ino);
		
		return STATUS_ERROR;;
	}
	ctx = ntfs_attr_get_search_ctx(ni, mrec);
	if (!ctx){
		unmap_mft_record(ni);
		return STATUS_ERROR;
	}
	
	if(ntfs_attr_lookup(AT_INDEX_ALLOCATION, I30,4, 
				CASE_SENSITIVE, 0, NULL, 0, ctx)){
		unmap_mft_record(ni);
		ntfs_attr_put_search_ctx(ctx);
		return STATUS_ERROR;
	}

	unmap_mft_record(ni);
	ntfs_attr_put_search_ctx(ctx);
	
/*	ib = kmalloc(icx->block_size,GFP_KERNEL);
	if (!ib)
		return STATUS_ERROR;
*/	
	ie_succ = ntfs_ie_get_next(icx->entry);//´ýÉ¾³ýµÄINDEX ENTRY µÄÏÂÒ»¸ö
	entry_pos = icx->parent_pos[icx->pindex]++;//´ýÉ¾³ýµÄINDEX ENTRYÔÚ¸¸½ÚµãµÄÎ»ÖÃ +1 
	pindex = icx->pindex;
descend:
	vcn = ntfs_ie_get_vcn(ie_succ);//´ýÉ¾³ýentryµÄnext entry µÄvcn
	page = ntfs_ib_read(icx, vcn,&ib);
	if (!page)//¶ÁÈ¡Æäblock 
		goto out;

	ie_succ = ntfs_ie_get_first(&ib->index);//´ýÉ¾³ýenryµÄnext entryµÄµÚÒ»¸ö×Óentry

	if (ntfs_icx_parent_inc(icx))
		goto out;
	
	icx->parent_vcn[icx->pindex] = vcn;
	icx->parent_pos[icx->pindex] = 0;

	if ((ib->index.flags & NODE_MASK) == INDEX_NODE)
		goto descend;		//Ñ­»·Ö±µ½ÕÒµ½´ýÉ¾³ýentryºóµÄÊ×¸öÒ¶×Óentry
	if (ntfs_ih_zero_entry(&ib->index)) {
		//errno = EIO;
		ntfs_debug("Empty index block");
		goto out;
	}

	ie = ntfs_ie_dup(ie_succ);//¸´ÖÆ´ýÉ¾³ýentryµÄnext entry
	if (!ie)
		goto out;
	
	if (ntfs_ie_add_vcn(&ie))//À©´ósizeof(vcn)
		goto out2;

	ntfs_ie_set_vcn(ie, ntfs_ie_get_vcn(icx->entry)); //½«´ýÉ¾³ýentryµÄvcn±£´æµ½ÕÒµ½µÄÒ¶×Óentry

	if (icx->is_in_root)
		ih = &icx->ir->index;
	else
		ih = &icx->ia->index;

	delta = le16_to_cpu(ie->length) - le16_to_cpu(icx->entry->length);
	new_size = le32_to_cpu(ih->index_length) + delta;//ÐÂindex headerµÄ³¤¶È
	ntfs_debug("delta :%d ,new_size :%d",delta,new_size);
	if (delta > 0) {
		if (icx->is_in_root) {
			ret = ntfs_ir_make_space(icx, new_size);//¸Ä±äindex rootµÄ´óÐ¡
			if (ret != STATUS_OK)
				goto out2;
			
			ih = &icx->ir->index;
			entry = ntfs_ie_get_by_pos(ih, entry_pos);//ÕÒµ½Ô­À´entry
			
		} else if (new_size > le32_to_cpu(ih->allocated_size)) {
			icx->pindex = pindex;
			ret = ntfs_ib_split(icx, icx->ia);	//¸Ä±äindex block
			if (ret == STATUS_OK)
				ret = STATUS_KEEP_SEARCHING; //ÔÚntfs_index_removeÖÐ¼ÌÐøÑ­»·
			goto out2;
		}
	}

	ntfs_ie_delete(ih, entry);//É¾³ýÔ­entry
	ntfs_ie_insert(ih, ie, entry);//ÔÚÔ­entryÎ»ÖÃ²åÈë¸Ä±äÁËµÄÒ¶×Óentry
	
	if (icx->is_in_root) {
		if (ntfs_ir_truncate(icx, new_size))//¸Ä±äINDEX ROOT´óÐ¡
			goto out2;
	} else
		if (ntfs_icx_ib_write(icx))//¸Ä±äINDEX ALLOCATION´óÐ¡
			goto out2;
	
	ntfs_ie_delete(&ib->index, ie_succ);//ÔÙÉ¾³ýÖ®Ç°µÄÒ¶×Óentry
	
	if (ntfs_ih_zero_entry(&ib->index)) {
		if (ntfs_index_rm_leaf(icx))
			goto out2;
	} else 
		if (ntfs_ib_write(icx, ib,page))//·ÂÕÕntfs_ib_readÀ´Ð´£¬½«¶ÔpageµÄÅÐ¶Ï¼Ó½øÈ¥
			goto out2;
	ret = STATUS_OK;
	
out2:
	kfree(ie);
out:
//	if(ib)		
//		kfree(ib);		
	if(!icx->page || page->index != icx->page->index){
		ntfs_debug("unmap page :0x%lx ",page->index);
		unlock_page(page);
		ntfs_unmap_page(page);
	}
	return ret;
}


/**É¾³ýÄ¿Â¼ÖÐµÄ¼ÇÂ¼
 * ntfs_index_rm - remove entry from the index
 * @icx:	index context describing entry to delete
 *
 * Delete entry described by @icx from the index. Index context is always 
 * reinitialized after use of this function, so it can be used for index 
 * lookup once again.
 *
 * Return 0 on success or -1 on error with errno set to the error code.
 */
/*static JPA*/
int ntfs_index_rm(ntfs_index_context *icx)
{
	INDEX_HEADER *ih;
	int err, ret = STATUS_OK;
	unsigned long index = 0;
//	struct page * page;
//	int reuse = 0;
//   struct address_space *ia_mapping;
	
	ntfs_debug("Entering\n");
	
	if (!icx || (!icx->ia && !icx->ir) || ntfs_ie_end(icx->entry)) {
		ntfs_debug("Invalid arguments.");
		//errno = EINVAL;
		goto err_out;
	}
	/*Get the INDEX HEADER*/
	if (icx->is_in_root)
		ih = &icx->ir->index;
	else
		ih = &icx->ia->index;
	
	if (icx->entry->flags & INDEX_ENTRY_NODE) {//case 1: °üº¬×Ó½Úµã	
		ntfs_debug("Case 1 :");
		ret = ntfs_index_rm_node(icx);
	} else if (icx->is_in_root || !ntfs_ih_one_entry(ih)) {//case 2:Ð¡Ä¿Â¼(Ã»ÓÐINDEX_ALLOCATION)»òÕû¸öÄ¿Â¼²»Ö¹Ò»¸öINDEX ENTRY
		ntfs_debug("Case 2 :");
		
		ntfs_ie_delete(ih, icx->entry);//É¾³ý¶ÔÓ¦µÄINDEX ENTRY
		
		if (icx->is_in_root) {//Ð¡Ä¿Â¼£¬ÐèÒª¸Ä±äINDEX ROOT´óÐ¡
			ntfs_debug("small dir");
			err = ntfs_ir_truncate(icx, le32_to_cpu(ih->index_length));
			if (err != STATUS_OK)
				goto err_out;
		} else{
			ntfs_debug("big dir");
			index = (icx->ia->index_block_vcn) <<icx->idx_ni->itype.index.vcn_size_bits >> PAGE_CACHE_SHIFT;
			ntfs_debug("index :%ld,page->index:%ld",index,icx->page->index);
			//if(index == icx->page->index){
				ntfs_index_entry_mark_dirty(icx);
				//write_inode_now(VFS_I(icx->idx_ni), 1); //ÊÇ²»ÊÇ¶¼Ö»ÒªÖ´ÐÐ´Ë¾ä?
			//}
		}
	} else {
		ntfs_debug("Case 3 :"); 
		if (ntfs_index_rm_leaf(icx))//case 3:INDEX ALLOCATION½öÓÐÒ»¸öINDEX ENTRY£¬É¾³ýINDEX ALLOCATION?
			goto err_out;
	}
out:
	ntfs_debug("ret = %d ",ret);
	return ret;
err_out:
	ret = STATUS_ERROR;
	goto out;
}
/**·µ»ØÖµ: STATUS_ERROR,STATUS_OK*/
int ntfs_index_remove(ntfs_inode *dir_ni, ntfs_inode *ni,
		const void *key, const int keylen)
{
	int ret = STATUS_ERROR;
	ntfs_index_context *icx;

	ntfs_debug("Entering.");
	icx = ntfs_index_ctx_get(dir_ni);
	if (!icx)
		return -1;

	while (1) {				
		if (ntfs_index_lookup3(key, keylen, icx))//ÕÒµ½key¶ÔÓ¦µÄicx
			goto err_out;

/**FIXEME; REPARSE*/
#if 0
		if ((((FILE_NAME_ATTR *)icx->data)->file_attributes &
				FILE_ATTR_REPARSE_POINT)
		   && !ntfs_possible_symlink(ni)) {
			errno = EOPNOTSUPP;
			goto err_out;
		}
#endif
		ret = ntfs_index_rm(icx);
		if (ret == STATUS_ERROR)
			goto err_out;
		else if (ret == STATUS_OK)
			break;
		
		ntfs_inode_mark_dirty(icx->actx->ntfs_ino);
		ntfs_index_ctx_reinit(icx);
	}

	//ntfs_inode_mark_dirty(icx->actx->ntfs_ino);
	ntfs_index_entry_mark_dirty(icx);

out:	
	ntfs_index_ctx_put(icx);
	return ret;
err_out:
	ret = STATUS_ERROR;
	ntfs_debug("Delete failed");
	goto out;
}
#endif
#endif
