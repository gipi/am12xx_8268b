/**
 * attrib.c - NTFS attribute operations.  Part of the Linux-NTFS project.
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

#include <linux/buffer_head.h>
#include <linux/sched.h>
#include <linux/swap.h>
#include <linux/writeback.h>
#include <linux/math64.h>

#include "attrib.h"
#include "debug.h"
#include "layout.h"
#include "lcnalloc.h"
#include "malloc.h"
#include "mft.h"
#include "ntfs.h"
#include "types.h"
#include "dir.h"
#include "param.h"

/**
 * ntfs_map_runlist_nolock - map (a part of) a runlist of an ntfs inode
 * @ni:		ntfs inode for which to map (part of) a runlist
 * @vcn:	map runlist part containing this vcn
 * @ctx:	active attribute search context if present or NULL if not
 *
 * Map the part of a runlist containing the @vcn of the ntfs inode @ni.
 *映射包含vcn(在ni中)的部分runlist
 
 * If @ctx is specified, it is an active search context of @ni and its base mft
 * record.  This is needed when ntfs_map_runlist_nolock() encounters unmapped
 * runlist fragments and allows their mapping.  If you do not have the mft
 * record mapped, you can specify @ctx as NULL and ntfs_map_runlist_nolock()
 * will perform the necessary mapping and unmapping.
 *如果指定@ctx，则其是@ni的搜索内容，当该函数遇到
 没映射的runlist 碎片时，可以用来允许映射。

 * Note, ntfs_map_runlist_nolock() saves the state of @ctx on entry and
 * restores it before returning.  Thus, @ctx will be left pointing to the same
 * attribute on return as on entry.  However, the actual pointers in @ctx may
 * point to different memory locations on return, so you must remember to reset
 * any cached pointers from the @ctx, i.e. after the call to
 * ntfs_map_runlist_nolock(), you will probably want to do:
 *	m = ctx->mrec;
 *	a = ctx->attr;
 * Assuming you cache ctx->attr in a variable @a of type ATTR_RECORD * and that
 * you cache ctx->mrec in a variable @m of type MFT_RECORD *.
 *
 * Return 0 on success and -errno on error.  There is one special error code
 * which is not an error as such.  This is -ENOENT.  It means that @vcn is out
 * of bounds of the runlist.
 *
 * Note the runlist can be NULL after this function returns if @vcn is zero and
 * the attribute has zero allocated size, i.e. there simply is no runlist.
 *当@vcn=0且属性的分配大小为0时，runlist为NULL
 
 * WARNING: If @ctx is supplied, regardless of whether success or failure is
 *	    returned, you need to check IS_ERR(@ctx->mrec) and if 'true' the @ctx
 *	    is no longer valid, i.e. you need to either call
 *	    ntfs_attr_reinit_search_ctx() or ntfs_attr_put_search_ctx() on it.
 *	    In that case PTR_ERR(@ctx->mrec) will give you the error code for
 *	    why the mapping of the old inode failed.
 当指定@ctx时，不管返回是否成功，都需要检测IS_ERR(@ctx->mrec) ，
 如果为真，证明@ctx无效了，需要执行 ntfs_attr_reinit_search_ctx() or ntfs_attr_put_search_ctx()。
 *
 * Locking: - The runlist described by @ni must be locked for writing on entry
 *	      and is locked on return.  Note the runlist will be modified.
 *	    - If @ctx is NULL, the base mft record of @ni must not be mapped on
 *	      entry and it will be left unmapped on return.
 *	    - If @ctx is not NULL, the base mft record must be mapped on entry
 *	      and it will be left mapped on return.
 @ni描述的runlist在写操作的入口和出口都必须上锁，注意runlist可能被修改。
 如果@ctx为NULL，ni的主mft record在函数入和出时不能被映射；
 如果@ctx非空，ni的主mft record在函数入和出时都被映射；

 */
int ntfs_map_runlist_nolock(ntfs_inode *ni, VCN vcn, ntfs_attr_search_ctx *ctx)
{
	VCN end_vcn;
	unsigned long flags;
	ntfs_inode *base_ni;
	MFT_RECORD *m;
	ATTR_RECORD *a;
	runlist_element *rl;
	struct page *put_this_page = NULL;
	int err = 0;
	bool ctx_is_temporary, ctx_needs_reset;
	ntfs_attr_search_ctx old_ctx = { NULL, };

	ntfs_debug("Mapping runlist part containing vcn 0x%llx.",
			(unsigned long long)vcn);
	if (!NInoAttr(ni))
		base_ni = ni;
	else
		base_ni = ni->ext.base_ntfs_ino;
	if (!ctx) {
		ctx_is_temporary = ctx_needs_reset = true;
		m = map_mft_record(base_ni);
		if (IS_ERR(m))
			return PTR_ERR(m);
		ctx = ntfs_attr_get_search_ctx(base_ni, m);
		if (unlikely(!ctx)) {
			err = -ENOMEM;
			goto err_out;
		}
	} else {
		VCN allocated_size_vcn;

		DEBUG0_ON(IS_ERR(ctx->mrec));
		a = ctx->attr;
		DEBUG0_ON(!a->non_resident);
		ctx_is_temporary = false;
		end_vcn = sle64_to_cpu(a->data.non_resident.highest_vcn);
		read_lock_irqsave(&ni->size_lock, flags);
		allocated_size_vcn = ni->allocated_size >>
				ni->vol->cluster_size_bits;
		read_unlock_irqrestore(&ni->size_lock, flags);
		if (!a->data.non_resident.lowest_vcn && end_vcn <= 0)
			end_vcn = allocated_size_vcn - 1;
		/*
		 * If we already have the attribute extent containing @vcn in
		 * @ctx, no need to look it up again.  We slightly cheat in
		 * that if vcn exceeds the allocated size, we will refuse to
		 * map the runlist below, so there is definitely no need to get
		 * the right attribute extent.
		 */
		if (vcn >= allocated_size_vcn || (a->type == ni->type &&
				a->name_length == ni->name_len &&
				!memcmp((u8*)a + le16_to_cpu(a->name_offset),
				ni->name, ni->name_len) &&
				sle64_to_cpu(a->data.non_resident.lowest_vcn)
				<= vcn && end_vcn >= vcn))
			ctx_needs_reset = false;
		else {
			/* Save the old search context. */
			old_ctx = *ctx;
			/*
			 * If the currently mapped (extent) inode is not the
			 * base inode we will unmap it when we reinitialize the
			 * search context which means we need to get a
			 * reference to the page containing the mapped mft
			 * record so we do not accidentally drop changes to the
			 * mft record when it has not been marked dirty yet.
			 */
			if (old_ctx.base_ntfs_ino && old_ctx.ntfs_ino !=
					old_ctx.base_ntfs_ino) {
				put_this_page = old_ctx.ntfs_ino->page;
				page_cache_get(put_this_page);
			}
			/*
			 * Reinitialize the search context so we can lookup the
			 * needed attribute extent.
			 */
			ntfs_attr_reinit_search_ctx(ctx);
			ctx_needs_reset = true;
		}
	}
	if (ctx_needs_reset) {
		err = ntfs_attr_lookup(ni->type, ni->name, ni->name_len,
				CASE_SENSITIVE, vcn, NULL, 0, ctx);
		if (unlikely(err)) {
			if (err == -ENOENT)
				err = -EIO;
			goto err_out;
		}
		DEBUG0_ON(!ctx->attr->non_resident);
	}
	a = ctx->attr;
	/*
	 * Only decompress the mapping pairs if @vcn is inside it.  Otherwise
	 * we get into problems when we try to map an out of bounds vcn because
	 * we then try to map the already mapped runlist fragment and
	 * ntfs_mapping_pairs_decompress() fails.
	 */
	end_vcn = sle64_to_cpu(a->data.non_resident.highest_vcn) + 1;
	if (unlikely(vcn && vcn >= end_vcn)) {
		err = -ENOENT;
		goto err_out;
	}
	rl = ntfs_mapping_pairs_decompress(ni->vol, a, ni->runlist.rl);
	if (IS_ERR(rl))
		err = PTR_ERR(rl);
	else
		ni->runlist.rl = rl;
err_out:
	if (ctx_is_temporary) {
		if (likely(ctx))
			ntfs_attr_put_search_ctx(ctx);
		unmap_mft_record(base_ni);
	} else if (ctx_needs_reset) {
		/*
		 * If there is no attribute list, restoring the search context
		 * is acomplished simply by copying the saved context back over
		 * the caller supplied context.  If there is an attribute list,
		 * things are more complicated as we need to deal with mapping
		 * of mft records and resulting potential changes in pointers.
		 */
		if (NInoAttrList(base_ni)) {
			/*
			 * If the currently mapped (extent) inode is not the
			 * one we had before, we need to unmap it and map the
			 * old one.
			 */
			if (ctx->ntfs_ino != old_ctx.ntfs_ino) {
				/*
				 * If the currently mapped inode is not the
				 * base inode, unmap it.
				 */
				if (ctx->base_ntfs_ino && ctx->ntfs_ino !=
						ctx->base_ntfs_ino) {
					unmap_extent_mft_record(ctx->ntfs_ino);
					ctx->mrec = ctx->base_mrec;
					DEBUG0_ON(!ctx->mrec);
				}
				/*
				 * If the old mapped inode is not the base
				 * inode, map it.
				 */
				if (old_ctx.base_ntfs_ino &&
						old_ctx.ntfs_ino !=
						old_ctx.base_ntfs_ino) {
retry_map:
					ctx->mrec = map_mft_record(
							old_ctx.ntfs_ino);
					/*
					 * Something bad has happened.  If out
					 * of memory retry till it succeeds.
					 * Any other errors are fatal and we
					 * return the error code in ctx->mrec.
					 * Let the caller deal with it...  We
					 * just need to fudge things so the
					 * caller can reinit and/or put the
					 * search context safely.
					 */
					if (IS_ERR(ctx->mrec)) {
						if (PTR_ERR(ctx->mrec) ==
								-ENOMEM) {
							schedule();
							goto retry_map;
						} else
							old_ctx.ntfs_ino =
								old_ctx.
								base_ntfs_ino;
					}
				}
			}
			/* Update the changed pointers in the saved context. */
			if (ctx->mrec != old_ctx.mrec) {
				if (!IS_ERR(ctx->mrec))
					old_ctx.attr = (ATTR_RECORD*)(
							(u8*)ctx->mrec +
							((u8*)old_ctx.attr -
							(u8*)old_ctx.mrec));
				old_ctx.mrec = ctx->mrec;
			}
		}
		/* Restore the search context to the saved one. */
		*ctx = old_ctx;
		/*
		 * We drop the reference on the page we took earlier.  In the
		 * case that IS_ERR(ctx->mrec) is true this means we might lose
		 * some changes to the mft record that had been made between
		 * the last time it was marked dirty/written out and now.  This
		 * at this stage is not a problem as the mapping error is fatal
		 * enough that the mft record cannot be written out anyway and
		 * the caller is very likely to shutdown the whole inode
		 * immediately and mark the volume dirty for chkdsk to pick up
		 * the pieces anyway.
		 */
		if (put_this_page)
			page_cache_release(put_this_page);
	}
	return err;
}

/**
 * ntfs_map_runlist - map (a part of) a runlist of an ntfs inode
 * @ni:		ntfs inode for which to map (part of) a runlist
 * @vcn:	map runlist part containing this vcn
 *
 * Map the part of a runlist containing the @vcn of the ntfs inode @ni.
 *
 * Return 0 on success and -errno on error.  There is one special error code
 * which is not an error as such.  This is -ENOENT.  It means that @vcn is out
 * of bounds of the runlist.
 *
 * Locking: - The runlist must be unlocked on entry and is unlocked on return.
 *	    - This function takes the runlist lock for writing and may modify
 *	      the runlist.
 */
int ntfs_map_runlist(ntfs_inode *ni, VCN vcn)
{
	int err = 0;
	
	 if(ni->mft_no == 0)
		 ntfs_debug("down_write");

	down_write(&ni->runlist.lock);
	/* Make sure someone else didn't do the work while we were sleeping. */
	if (likely(ntfs_rl_vcn_to_lcn(ni->runlist.rl, vcn) <=
			LCN_RL_NOT_MAPPED))
		err = ntfs_map_runlist_nolock(ni, vcn, NULL);
	 if(ni->mft_no == 0)
		ntfs_debug("up_write");
	up_write(&ni->runlist.lock);
	return err;
}

/**根据给定的ntfs inode(拥有runlist)将vcn转换为lcn
 * ntfs_attr_vcn_to_lcn_nolock - convert a vcn into a lcn given an ntfs inode
 * @ni:			ntfs inode of the attribute whose runlist to search
 * @vcn:		vcn to convert
 * @write_locked:	true if the runlist is locked for writing
 *
 * Find the virtual cluster number @vcn in the runlist of the ntfs attribute
 * described by the ntfs inode @ni and return the corresponding logical cluster
 * number (lcn).
 *
 * If the @vcn is not mapped yet, the attempt is made to map the attribute
 * extent containing the @vcn and the vcn to lcn conversion is retried.
 *
 * If @write_locked is true the caller has locked the runlist for writing and
 * if false for reading.
 *
 * Since lcns must be >= 0, we use negative return codes with special meaning:
 *
 * Return code	Meaning / Description
 * ==========================================
 *  LCN_HOLE	Hole / not allocated on disk.
 *  LCN_ENOENT	There is no such vcn in the runlist, i.e. @vcn is out of bounds.
 *  LCN_ENOMEM	Not enough memory to map runlist.
 *  LCN_EIO	Critical error (runlist/file is corrupt, i/o error, etc).
 *
 * Locking: - The runlist must be locked on entry and is left locked on return.
 *	    - If @write_locked is 'false', i.e. the runlist is locked for reading,
 *	      the lock may be dropped inside the function so you cannot rely on
 *	      the runlist still being the same when this function returns.
 */
LCN ntfs_attr_vcn_to_lcn_nolock(ntfs_inode *ni, const VCN vcn,
		const bool write_locked)
{
	LCN lcn;
	unsigned long flags;
	bool is_retry = false;

	ntfs_debug("Entering for i_ino 0x%lx, vcn 0x%llx, %s_locked.",
			ni->mft_no, (unsigned long long)vcn,
			write_locked ? "write" : "read");
	DEBUG0_ON(!ni);
	DEBUG0_ON(!NInoNonResident(ni));
	DEBUG0_ON(vcn < 0);
	if (!ni->runlist.rl) {
		read_lock_irqsave(&ni->size_lock, flags);
		if (!ni->allocated_size) {
			read_unlock_irqrestore(&ni->size_lock, flags);
			return LCN_ENOENT;
		}
		read_unlock_irqrestore(&ni->size_lock, flags);
	}
retry_remap:
	/* Convert vcn to lcn.  If that fails map the runlist and retry once. */
	lcn = ntfs_rl_vcn_to_lcn(ni->runlist.rl, vcn);
	if (likely(lcn >= LCN_HOLE)) {
		ntfs_debug("Done, lcn 0x%llx.", (long long)lcn);
		return lcn;
	}
	if (lcn != LCN_RL_NOT_MAPPED) {
		if (lcn != LCN_ENOENT)
			lcn = LCN_EIO;
	} else if (!is_retry) {
		int err;

		if (!write_locked) {
			 if(ni->mft_no == 0)
				 ntfs_debug("up_read");
			up_read(&ni->runlist.lock);
			 if(ni->mft_no == 0)
				 ntfs_debug("down_write");
			down_write(&ni->runlist.lock);
			if (unlikely(ntfs_rl_vcn_to_lcn(ni->runlist.rl, vcn) !=
					LCN_RL_NOT_MAPPED)) {
				 if(ni->mft_no == 0)
					ntfs_debug("up_write");

				up_write(&ni->runlist.lock);
				 if(ni->mft_no == 0)
					 ntfs_debug("down_read");
				down_read(&ni->runlist.lock);
				goto retry_remap;
			}
		}
		err = ntfs_map_runlist_nolock(ni, vcn, NULL);
		if (!write_locked) {
			 if(ni->mft_no == 0)
				ntfs_debug("up_write");

			up_write(&ni->runlist.lock);
			 if(ni->mft_no == 0)
				 ntfs_debug("down_read");
			down_read(&ni->runlist.lock);
		}
		if (likely(!err)) {
			is_retry = true;
			goto retry_remap;
		}
		if (err == -ENOENT)
			lcn = LCN_ENOENT;
		else if (err == -ENOMEM)
			lcn = LCN_ENOMEM;
		else
			lcn = LCN_EIO;
	}
	if (lcn != LCN_ENOENT)
		ntfs_error(ni->vol->sb, "Failed with error code %lli.",
				(long long)lcn);
	return lcn;
}

/**在给定的ntfs inode的runlist中寻找vcn
 * ntfs_attr_find_vcn_nolock - find a vcn in the runlist of an ntfs inode
 * @ni:		ntfs inode describing the runlist to search
 * @vcn:	vcn to find
 * @ctx:	active attribute search context if present or NULL if not
 *
 * Find the virtual cluster number @vcn in the runlist described by the ntfs
 * inode @ni and return the address of the runlist element containing the @vcn.
 *
 * If the @vcn is not mapped yet, the attempt is made to map the attribute
 * extent containing the @vcn and the vcn to lcn conversion is retried.
 *
 * If @ctx is specified, it is an active search context of @ni and its base mft
 * record.  This is needed when ntfs_attr_find_vcn_nolock() encounters unmapped
 * runlist fragments and allows their mapping.  If you do not have the mft
 * record mapped, you can specify @ctx as NULL and ntfs_attr_find_vcn_nolock()
 * will perform the necessary mapping and unmapping.
 *
 * Note, ntfs_attr_find_vcn_nolock() saves the state of @ctx on entry and
 * restores it before returning.  Thus, @ctx will be left pointing to the same
 * attribute on return as on entry.  However, the actual pointers in @ctx may
 * point to different memory locations on return, so you must remember to reset
 * any cached pointers from the @ctx, i.e. after the call to
 * ntfs_attr_find_vcn_nolock(), you will probably want to do:
 *	m = ctx->mrec;
 *	a = ctx->attr;
 * Assuming you cache ctx->attr in a variable @a of type ATTR_RECORD * and that
 * you cache ctx->mrec in a variable @m of type MFT_RECORD *.
 * Note you need to distinguish between the lcn of the returned runlist element
 * being >= 0 and LCN_HOLE.  In the later case you have to return zeroes on
 * read and allocate clusters on write.
 *
 * Return the runlist element containing the @vcn on success and
 * ERR_PTR(-errno) on error.  You need to test the return value with IS_ERR()
 * to decide if the return is success or failure and PTR_ERR() to get to the
 * error code if IS_ERR() is true.
 *
 * The possible error return codes are:
 *	-ENOENT - No such vcn in the runlist, i.e. @vcn is out of bounds.
 *	-ENOMEM - Not enough memory to map runlist.
 *	-EIO	- Critical error (runlist/file is corrupt, i/o error, etc).
 *
 * WARNING: If @ctx is supplied, regardless of whether success or failure is
 *	    returned, you need to check IS_ERR(@ctx->mrec) and if 'true' the @ctx
 *	    is no longer valid, i.e. you need to either call
 *	    ntfs_attr_reinit_search_ctx() or ntfs_attr_put_search_ctx() on it.
 *	    In that case PTR_ERR(@ctx->mrec) will give you the error code for
 *	    why the mapping of the old inode failed.
 *
 * Locking: - The runlist described by @ni must be locked for writing on entry
 *	      and is locked on return.  Note the runlist may be modified when
 *	      needed runlist fragments need to be mapped.
 *	    - If @ctx is NULL, the base mft record of @ni must not be mapped on
 *	      entry and it will be left unmapped on return.
 *	    - If @ctx is not NULL, the base mft record must be mapped on entry
 *	      and it will be left mapped on return.
 */
runlist_element *ntfs_attr_find_vcn_nolock(ntfs_inode *ni, const VCN vcn,
		ntfs_attr_search_ctx *ctx)
{
	unsigned long flags;
	runlist_element *rl;
	int err = 0;
	bool is_retry = false;

	ntfs_debug("Entering for i_ino 0x%lx, vcn 0x%llx, with%s ctx.",
			ni->mft_no, (unsigned long long)vcn, ctx ? "" : "out");
	DEBUG0_ON(!ni);
	DEBUG0_ON(!NInoNonResident(ni));
	DEBUG0_ON(vcn < 0);
	if (!ni->runlist.rl) {
		read_lock_irqsave(&ni->size_lock, flags);
		if (!ni->allocated_size) {
			read_unlock_irqrestore(&ni->size_lock, flags);
			return ERR_PTR(-ENOENT);
		}
		read_unlock_irqrestore(&ni->size_lock, flags);
	}
retry_remap:
	rl = ni->runlist.rl;
	if (likely(rl && vcn >= rl[0].vcn)) {
		while (likely(rl->length)) {
			if (unlikely(vcn < rl[1].vcn)) {
				if (likely(rl->lcn >= LCN_HOLE)) {
					ntfs_debug("Done.");
					return rl;
				}
				break;
			}
			rl++;
		}
		if (likely(rl->lcn != LCN_RL_NOT_MAPPED)) {
			if (likely(rl->lcn == LCN_ENOENT))
				err = -ENOENT;
			else
				err = -EIO;
		}
	}
	if (!err && !is_retry) {
		/*
		 * If the search context is invalid we cannot map the unmapped
		 * region.
		 */
		if (IS_ERR(ctx->mrec))
			err = PTR_ERR(ctx->mrec);
		else {
			/*
			 * The @vcn is in an unmapped region, map the runlist
			 * and retry.
			 */
			err = ntfs_map_runlist_nolock(ni, vcn, ctx);
			if (likely(!err)) {
				is_retry = true;
				goto retry_remap;
			}
		}
		if (err == -EINVAL)
			err = -EIO;
	} else if (!err)
		err = -EIO;
	if (err != -ENOENT)
		ntfs_error(ni->vol->sb, "Failed with error code %i.", err);
	return ERR_PTR(err);
}

/**在mft record中找到(下一个)属性
 * ntfs_attr_find - find (next) attribute in mft record
 * @type:	attribute type to find
 * @name:	attribute name to find (optional, i.e. NULL means don't care)
 * @name_len:	attribute name length (only needed if @name present)
 * @ic:		IGNORE_CASE or CASE_SENSITIVE (ignored if @name not present)
 * @val:	attribute value to find (optional, resident attributes only)
 * @val_len:	attribute value length
 * @ctx:	search context with mft record and attribute to search from
 *
 * You should not need to call this function directly.  Use ntfs_attr_lookup()
 * instead.
 不要直接调用该函数，使用ntfs_attr_lookup()代替.
 *
 * ntfs_attr_find() takes a search context @ctx as parameter and searches the
 * mft record specified by @ctx->mrec, beginning at @ctx->attr, for an
 * attribute of @type, optionally @name and @val.
 *
 * If the attribute is found, ntfs_attr_find() returns 0 and @ctx->attr will
 * point to the found attribute.
 成功时，返回0，ctx->attr指向找到的属性
 *
 * If the attribute is not found, ntfs_attr_find() returns -ENOENT and
 * @ctx->attr will point to the attribute before which the attribute being
 * searched for would need to be inserted if such an action were to be desired.
 *如果返回-ENOENT,ctx->attr指向所找属性的下一个属性，
 这是要添加属性的正确位置。
 
 * On actual error, ntfs_attr_find() returns -EIO.  In this case @ctx->attr is
 * undefined and in particular do not rely on it not changing.
 *
 * If @ctx->is_first is 'true', the search begins with @ctx->attr itself.  If it
 * is 'false', the search begins after @ctx->attr.
 *如果ctx->is_first为1，从ctx->attr本身开始搜索；
 否则从其后开始搜索。
 
 * If @ic is IGNORE_CASE, the @name comparisson is not case sensitive and
 * @ctx->ntfs_ino must be set to the ntfs inode to which the mft record
 * @ctx->mrec belongs.  This is so we can get at the ntfs volume and hence at
 * the upcase table.  If @ic is CASE_SENSITIVE, the comparison is case
 * sensitive.  When @name is present, @name_len is the @name length in Unicode
 * characters.
 *当ic为IGNORE_CASE时，名字间的比较不区分大小写，
 此时ctx->ntfs_ino必须是ctx->mrec属于的节点。因为我们要
 通过ntfs volume得到upcase table.
 
 * If @name is not present (NULL), we assume that the unnamed attribute is
 * being searched for.
 *如果name为空，则表明搜索未命名的属性。
 
 * Finally, the resident attribute value @val is looked for, if present.  If
 * @val is not present (NULL), @val_len is ignored.
 *
 * ntfs_attr_find() only searches the specified mft record and it ignores the
 * presence of an attribute list attribute (unless it is the one being searched
 * for, obviously).  If you need to take attribute lists into consideration,
 * use ntfs_attr_lookup() instead (see below).  This also means that you cannot
 * use ntfs_attr_find() to search for extent records of non-resident
 * attributes, as extents with lowest_vcn != 0 are usually described by the
 * attribute list attribute only. - Note that it is possible that the first
 * extent is only in the attribute list while the last extent is in the base
 * mft record, so do not rely on being able to find the first extent in the
 * base mft record.
 该函数只搜索给定的mft record，而忽略存在的属性列表
 (当然搜索属性列表时例外)。如果要想搜索是不忽略属性
 列表，请使用ntfs_attr_lookup() .
 这也就是说你不能使用该函数来搜索位于扩展记录的非驻留属性。
 对于扩展记录，lowest_vcn!=0(仅由属性列表来描述)；

注意: 有可能第一个extent在属性列表中，而最后的extent
是在主mft record中，所以别依赖在主mft record中找到第一个extent。

 *
 * Warning: Never use @val when looking for attribute types which can be
 *	    non-resident as this most likely will result in a crash!
 * 注意当搜索非驻留属性时，@val一定要NULL，否则可能崩溃!
 */

static int ntfs_attr_find(const ATTR_TYPE type, const ntfschar *name,
		const u32 name_len, const IGNORE_CASE_BOOL ic,
		const u8 *val, const u32 val_len, ntfs_attr_search_ctx *ctx)
{
	ATTR_RECORD *a;
	ntfs_volume *vol = ctx->ntfs_ino->vol;
	ntfschar *upcase = vol->upcase;
	u32 upcase_len = vol->upcase_len;

	ntfs_trace("Entering. type ==0x%x",type);	
	/*
	 * Iterate over attributes in mft record starting at @ctx->attr, or the
	 * attribute following that, if @ctx->is_first is 'true'.
	 */
	if (ctx->is_first) {
		a = ctx->attr;
		ctx->is_first = false;
	} else
		a = (ATTR_RECORD*)((u8*)ctx->attr +
				le32_to_cpu(ctx->attr->length));
	for (;;	a = (ATTR_RECORD*)((u8*)a + le32_to_cpu(a->length))) {
		if ((u8*)a < (u8*)ctx->mrec || (u8*)a > (u8*)ctx->mrec +
				le32_to_cpu(ctx->mrec->bytes_allocated))
			break;		
		ctx->attr = a;
		//ntfs_debug("a->type = 0x%x ,a->length = 0x%x, value_lenth=0x%x",a->type,a->length,a->data.resident.value_length);
		/*note:the codes about AT_UNUSED were add by yangli, please check it*/
		/* @ modify by yangli */
		if (((type != AT_UNUSED) && unlikely(le32_to_cpu(a->type) > le32_to_cpu(type))) ||
				a->type == AT_END)
		/* @ end */
			return -ENOENT;
		if (unlikely(!a->length))
			break;

		/* @ add by yangli */
		/* If this is an enumeration return this attribute. */
		if (type == AT_UNUSED)
			return 0;
		/* @ end */
		if (a->type != type)
			continue;		
		/*
		 * If @name is present, compare the two names.  If @name is
		 * missing, assume we want an unnamed attribute.
		 */
		if (!name) {
			/* The search failed if the found attribute is named. */
			if (a->name_length)
				return -ENOENT;
		} else if (!ntfs_are_names_equal(name, name_len,
			    (ntfschar*)((u8*)a + le16_to_cpu(a->name_offset)),
			    a->name_length, ic, upcase, upcase_len)) {
			register int rc;
			rc = ntfs_collate_names(name, name_len,
					(ntfschar*)((u8*)a +
					le16_to_cpu(a->name_offset)),
					a->name_length, 1, IGNORE_CASE,
					upcase, upcase_len);
			/*
			 * If @name collates before a->name, there is no
			 * matching attribute.
			 */
			if (rc == -1)
				return -ENOENT;
			/* If the strings are not equal, continue search. */
			if (rc)
				continue;
			}
		
		/*
		 * The names match or @name not present and attribute is
		 * unnamed.  If no @val specified, we have found the attribute
		 * and are done.
		 */
		if (!val){
			ntfs_trace("Done1");
			return 0;
			}
		/* @val is present; compare values. */
		else {
			register int rc;

			rc = memcmp(val, (u8*)a + le16_to_cpu(
					a->data.resident.value_offset),
					min_t(u32, val_len, le32_to_cpu(
					a->data.resident.value_length)));
			/*
			 * If @val collates before the current attribute's
			 * value, there is no matching attribute.
			 */
			if (!rc) {
				register u32 avl;

				avl = le32_to_cpu(
						a->data.resident.value_length);
				if (val_len == avl){
					ntfs_trace("Done2");
					return 0;
				}
				if (val_len < avl)
					return -ENOENT;
			} else if (rc < 0)
				return -ENOENT;
		}
	}
	ntfs_error(vol->sb, "Inode is corrupt.  Run chkdsk.");
	NVolSetErrors(vol);
	return -EIO;
}

/**加载属性列表到内存中
 * load_attribute_list - load an attribute list into memory
 * @vol:		ntfs volume from which to read
 * @runlist:		runlist of the attribute list
 * @al_start:		destination buffer
 * @size:		size of the destination buffer in bytes
 * @initialized_size:	initialized size of the attribute list
 *
 * Walk the runlist @runlist and load all clusters from it copying them into
 * the linear buffer @al. The maximum number of bytes copied to @al is @size
 * bytes. Note, @size does not need to be a multiple of the cluster size. If
 * @initialized_size is less than @size, the region in @al between
 * @initialized_size and @size will be zeroed and not read from disk.
 *
 * Return 0 on success or -errno on error.
 */
int load_attribute_list(ntfs_volume *vol, runlist *runlist, u8 *al_start,
		const s64 size, const s64 initialized_size)
{
	LCN lcn;
	u8 *al = al_start;
	u8 *al_end = al + initialized_size;
	runlist_element *rl;
	struct buffer_head *bh;
	struct super_block *sb;
	unsigned long block_size;
	unsigned long block, max_block;
	int err = 0;
	unsigned char block_size_bits;

	ntfs_debug("Entering.");
	if (!vol || !runlist || !al || size <= 0 || initialized_size < 0 ||
			initialized_size > size)
		return -EINVAL;
	if (!initialized_size) {
		memset(al, 0, size);
		return 0;
	}
	sb = vol->sb;
	block_size = sb->s_blocksize;
	block_size_bits = sb->s_blocksize_bits;
	ntfs_debug("down_read");
	down_read(&runlist->lock);
	rl = runlist->rl;
	if (!rl) {
		ntfs_error(sb, "Cannot read attribute list since runlist is "
				"missing.");
		goto err_out;	
	}
	/* Read all clusters specified by the runlist one run at a time. */
	while (rl->length) {
		lcn = ntfs_rl_vcn_to_lcn(rl, rl->vcn);
		ntfs_debug("Reading vcn = 0x%llx, lcn = 0x%llx.",
				(unsigned long long)rl->vcn,
				(unsigned long long)lcn);
		/* The attribute list cannot be sparse. */
		if (lcn < 0) {
			ntfs_error(sb, "ntfs_rl_vcn_to_lcn() failed.  Cannot "
					"read attribute list.");
			goto err_out;
		}
		block = lcn << vol->cluster_size_bits >> block_size_bits;
		/* Read the run from device in chunks of block_size bytes. */
		max_block = block + (rl->length << vol->cluster_size_bits >>
				block_size_bits);
		ntfs_debug("max_block = 0x%lx.", max_block);
		do {
			ntfs_debug("Reading block = 0x%lx.", block);
			bh = sb_bread(sb, block);
			if (!bh) {
				ntfs_error(sb, "sb_bread() failed. Cannot "
						"read attribute list.");
				goto err_out;
			}
			if (al + block_size >= al_end)
				goto do_final;
			memcpy(al, bh->b_data, block_size);
			brelse(bh);
			al += block_size;
		} while (++block < max_block);
		rl++;
	}
	if (initialized_size < size) {
initialize:
		memset(al_start + initialized_size, 0, size - initialized_size);
	}
done:
	ntfs_debug("up_read");
	up_read(&runlist->lock);
	return err;
do_final:
	if (al < al_end) {
		/*
		 * Partial block.
		 *
		 * Note: The attribute list can be smaller than its allocation
		 * by multiple clusters.  This has been encountered by at least
		 * two people running Windows XP, thus we cannot do any
		 * truncation sanity checking here. (AIA)
		 */
		memcpy(al, bh->b_data, al_end - al);
		brelse(bh);
		if (initialized_size < size)
			goto initialize;
		goto done;
	}
	brelse(bh);
	/* Real overflow! */
	ntfs_error(sb, "Attribute list buffer overflow. Read attribute list "
			"is truncated.");
err_out:
	err = -EIO;
	goto done;
}

/**在inode的属性列表中查找属性attribute
 * ntfs_external_attr_find - find an attribute in the attribute list of an inode
 * @type:	attribute type to find
 * @name:	attribute name to find (optional, i.e. NULL means don't care)
 * @name_len:	attribute name length (only needed if @name present)
 * @ic:		IGNORE_CASE or CASE_SENSITIVE (ignored if @name not present)
 * @lowest_vcn:	lowest vcn to find (optional, non-resident attributes only)
 * @val:	attribute value to find (optional, resident attributes only)
 * @val_len:	attribute value length
 * @ctx:	search context with mft record and attribute to search from
 *
 * You should not need to call this function directly.  Use ntfs_attr_lookup()
 * instead.
 *
 * Find an attribute by searching the attribute list for the corresponding
 * attribute list entry.  Having found the entry, map the mft record if the
 * attribute is in a different mft record/inode, ntfs_attr_find() the attribute
 * in there and return it.
 *
 * On first search @ctx->ntfs_ino must be the base mft record and @ctx must
 * have been obtained from a call to ntfs_attr_get_search_ctx().  On subsequent
 * calls @ctx->ntfs_ino can be any extent inode, too (@ctx->base_ntfs_ino is
 * then the base inode).
 *
 * After finishing with the attribute/mft record you need to call
 * ntfs_attr_put_search_ctx() to cleanup the search context (unmapping any
 * mapped inodes, etc).
 *
 * If the attribute is found, ntfs_external_attr_find() returns 0 and
 * @ctx->attr will point to the found attribute.  @ctx->mrec will point to the
 * mft record in which @ctx->attr is located and @ctx->al_entry will point to
 * the attribute list entry for the attribute.
 *
 * If the attribute is not found, ntfs_external_attr_find() returns -ENOENT and
 * @ctx->attr will point to the attribute in the base mft record before which
 * the attribute being searched for would need to be inserted if such an action
 * were to be desired.  @ctx->mrec will point to the mft record in which
 * @ctx->attr is located and @ctx->al_entry will point to the attribute list
 * entry of the attribute before which the attribute being searched for would
 * need to be inserted if such an action were to be desired.
 *
 * Thus to insert the not found attribute, one wants to add the attribute to
 * @ctx->mrec (the base mft record) and if there is not enough space, the
 * attribute should be placed in a newly allocated extent mft record.  The
 * attribute list entry for the inserted attribute should be inserted in the
 * attribute list attribute at @ctx->al_entry.
 *
 * On actual error, ntfs_external_attr_find() returns -EIO.  In this case
 * @ctx->attr is undefined and in particular do not rely on it not changing.
 */
static int ntfs_external_attr_find(const ATTR_TYPE type,
		const ntfschar *name, const u32 name_len,
		const IGNORE_CASE_BOOL ic, const VCN lowest_vcn,
		const u8 *val, const u32 val_len, ntfs_attr_search_ctx *ctx)
{
	ntfs_inode *base_ni, *ni;
	ntfs_volume *vol;
	ATTR_LIST_ENTRY *al_entry, *next_al_entry;
	u8 *al_start, *al_end;
	ATTR_RECORD *a;
	ntfschar *al_name;
	u32 al_name_len;
	int err = 0;
	static const char *es = " Unmount and run chkdsk.";
	BOOL is_first_search = false;

	ni = ctx->ntfs_ino;
	base_ni = ctx->base_ntfs_ino;
	ntfs_debug("Entering for inode 0x%lx, type 0x%x.", ni->mft_no, type);
	if (!base_ni) {
		/* First call happens with the base mft record. */
		base_ni = ctx->base_ntfs_ino = ctx->ntfs_ino;
		ctx->base_mrec = ctx->mrec;
	}
	if (ni == base_ni)
		ctx->base_attr = ctx->attr;
	if (type == AT_END)
		goto not_found;
	vol = base_ni->vol;
	al_start = base_ni->attr_list;
	al_end = al_start + base_ni->attr_list_size;
	if (!ctx->al_entry){
		ctx->al_entry = (ATTR_LIST_ENTRY*)al_start;
		 is_first_search = true;
	}
	/*
	 * Iterate over entries in attribute list starting at @ctx->al_entry,
	 * or the entry following that, if @ctx->is_first is 'true'.
	 */
	if (ctx->is_first) {
		al_entry = ctx->al_entry;
		ctx->is_first = false;
		if ((type == AT_UNUSED) && is_first_search &&
				le32_to_cpu(al_entry->type) >
				le32_to_cpu(AT_ATTRIBUTE_LIST))
			goto find_attr_list_attr;
	} else{
		al_entry = (ATTR_LIST_ENTRY*)((u8*)ctx->al_entry +
				le16_to_cpu(ctx->al_entry->length));

		/* @ add by yangli*/
		/*
		 * If this is an enumeration and the attribute list attribute
		 * is the next one in the enumeration sequence, just return the
		 * attribute list attribute from the base mft record as it is
		 * not listed in the attribute list itself.
		 */
		if ((type == AT_UNUSED) && le32_to_cpu(ctx->al_entry->type) <
				le32_to_cpu(AT_ATTRIBUTE_LIST) &&
				le32_to_cpu(al_entry->type) >
				le32_to_cpu(AT_ATTRIBUTE_LIST)) {
			int rc;
find_attr_list_attr:

			/* Check for bogus calls. */
			if (name || name_len || val || val_len || lowest_vcn) {
				//errno = EINVAL;
				ntfs_error(vol->sb,"invalid argurement");
				return -1;
			}

			/* We want the base record. */
			ctx->ntfs_ino = base_ni;
			ctx->mrec = ctx->base_mrec;
			ctx->is_first = true;
			/* Sanity checks are performed elsewhere. */
			ctx->attr = (ATTR_RECORD*)((u8*)ctx->mrec +
					le16_to_cpu(ctx->mrec->attrs_offset));

			/* Find the attribute list attribute. */
			rc = ntfs_attr_find(AT_ATTRIBUTE_LIST, NULL, 0,
					IGNORE_CASE, NULL, 0, ctx);

			/*
			 * Setup the search context so the correct
			 * attribute is returned next time round.
			 */
			ctx->al_entry = al_entry;
			ctx->is_first = true;

			/* Got it. Done. */
			if (!rc)
				return 0;

			/* Error! If other than not found return it. */
			if (rc != -ENOENT)
				return rc;

			/* Not found?!? Absurd! */
			//errno = EIO;
			ntfs_error(vol->sb,"Attribute list wasn't found");
			return -1;
		}
		/* @ end */
	}
	
	for (;; al_entry = next_al_entry) {
		/* Out of bounds check. */
		if ((u8*)al_entry < base_ni->attr_list ||
				(u8*)al_entry > al_end)
			break;	/* Inode is corrupt. */
		ctx->al_entry = al_entry;
		/* Catch the end of the attribute list. */
		if ((u8*)al_entry == al_end)
			goto not_found;
		if (!al_entry->length)
			break;
		if ((u8*)al_entry + 6 > al_end || (u8*)al_entry +
				le16_to_cpu(al_entry->length) > al_end)
			break;
		next_al_entry = (ATTR_LIST_ENTRY*)((u8*)al_entry +
				le16_to_cpu(al_entry->length));
/*		if (le32_to_cpu(al_entry->type) > le32_to_cpu(type))
			goto not_found;
		if (type != al_entry->type)
			continue;
*/
		if (type != AT_UNUSED) {//先匹配类型fix by yangli
			if (le32_to_cpu(al_entry->type) > le32_to_cpu(type))
				goto not_found;
			if (type != al_entry->type)
				continue;
		}
	
		/*
		 * If @name is present, compare the two names.  If @name is
		 * missing, assume we want an unnamed attribute.
		 */
		al_name_len = al_entry->name_length;
		al_name = (ntfschar*)((u8*)al_entry + al_entry->name_offset);
		/*
		 * If !@type we want the attribute represented by this
		 * attribute list entry.
		 */
		if (type == AT_UNUSED)
			goto is_enumeration;
		
		if (!name) {
			if (al_name_len)
				goto not_found;
		} else if (!ntfs_are_names_equal(al_name, al_name_len, name,//再匹配名字
				name_len, ic, vol->upcase, vol->upcase_len)) {
			register int rc;

			rc = ntfs_collate_names(name, name_len, al_name,
					al_name_len, 1, IGNORE_CASE,
					vol->upcase, vol->upcase_len);
			/*
			 * If @name collates before al_name, there is no
			 * matching attribute.
			 */
			if (rc == -1)
				goto not_found;
			/* If the strings are not equal, continue search. */
			if (rc)
				continue;
			/*
			 * FIXME: Reverse engineering showed 0, IGNORE_CASE but
			 * that is inconsistent with ntfs_attr_find().  The
			 * subsequent rc checks were also different.  Perhaps I
			 * made a mistake in one of the two.  Need to recheck
			 * which is correct or at least see what is going on...
			 * (AIA)
			 */
			rc = ntfs_collate_names(name, name_len, al_name,
					al_name_len, 1, CASE_SENSITIVE,
					vol->upcase, vol->upcase_len);
			if (rc == -1)
				goto not_found;
			if (rc)
				continue;
		}
		/*
		 * The names match or @name not present and attribute is
		 * unnamed.  Now check @lowest_vcn.  Continue search if the
		 * next attribute list entry still fits @lowest_vcn.  Otherwise
		 * we have reached the right one or the search has failed.
		 */
		if (lowest_vcn && (u8*)next_al_entry >= al_start	    &&//再匹配lowest_vcn
				(u8*)next_al_entry + 6 < al_end		    &&
				(u8*)next_al_entry + le16_to_cpu(
					next_al_entry->length) <= al_end    &&
				sle64_to_cpu(next_al_entry->lowest_vcn) <=
					lowest_vcn			    &&
				next_al_entry->type == al_entry->type	    &&
				next_al_entry->name_length == al_name_len   &&
				ntfs_are_names_equal((ntfschar*)((u8*)
					next_al_entry +
					next_al_entry->name_offset),
					next_al_entry->name_length,
					al_name, al_name_len, CASE_SENSITIVE,
					vol->upcase, vol->upcase_len))
			continue;
		
is_enumeration://fix by yangli
		if (MREF_LE(al_entry->mft_reference) == ni->mft_no) {
			if (MSEQNO_LE(al_entry->mft_reference) != ni->seq_no) {
				ntfs_error(vol->sb, "Found stale mft "
						"reference in attribute list "
						"of base inode 0x%lx.%s",
						base_ni->mft_no, es);
				err = -EIO;
				break;
			}
		} else { /* Mft references do not match. */
			/* If there is a mapped record unmap it first. */
			if (ni != base_ni)
				unmap_extent_mft_record(ni);
			/* Do we want the base record back? */
			if (MREF_LE(al_entry->mft_reference) ==
					base_ni->mft_no) {
				ni = ctx->ntfs_ino = base_ni;
				ctx->mrec = ctx->base_mrec;
			} else {
				/* We want an extent record. */
				ctx->mrec = map_extent_mft_record(base_ni,
						le64_to_cpu(
						al_entry->mft_reference), &ni);
				if (IS_ERR(ctx->mrec)) {
					ntfs_error(vol->sb, "Failed to map "
							"extent mft record "
							"0x%lx of base inode "
							"0x%lx.%s",
							MREF_LE(al_entry->
							mft_reference),
							base_ni->mft_no, es);
					err = PTR_ERR(ctx->mrec);
					if (err == -ENOENT)
						err = -EIO;
					/* Cause @ctx to be sanitized below. */
					ni = NULL;
					break;
				}
				ctx->ntfs_ino = ni;
			}
			ctx->attr = (ATTR_RECORD*)((u8*)ctx->mrec +
					le16_to_cpu(ctx->mrec->attrs_offset));
		}
		/*
		 * ctx->vfs_ino, ctx->mrec, and ctx->attr now point to the
		 * mft record containing the attribute represented by the
		 * current al_entry.
		 */
		/*
		 * We could call into ntfs_attr_find() to find the right
		 * attribute in this mft record but this would be less
		 * efficient and not quite accurate as ntfs_attr_find() ignores
		 * the attribute instance numbers for example which become
		 * important when one plays with attribute lists.  Also,
		 * because a proper match has been found in the attribute list
		 * entry above, the comparison can now be optimized.  So it is
		 * worth re-implementing a simplified ntfs_attr_find() here.
		 */
		a = ctx->attr;
		/*
		 * Use a manual loop so we can still use break and continue
		 * with the same meanings as above.
		 */
do_next_attr_loop:
		if ((u8*)a < (u8*)ctx->mrec || (u8*)a > (u8*)ctx->mrec +
				le32_to_cpu(ctx->mrec->bytes_allocated))
			break;
		if (a->type == AT_END)
			break;
		if (!a->length)
			break;
		if (al_entry->instance != a->instance)
			goto do_next_attr;
		/*
		 * If the type and/or the name are mismatched between the
		 * attribute list entry and the attribute record, there is
		 * corruption so we break and return error EIO.
		 */
		if (al_entry->type != a->type)
			break;
		if (!ntfs_are_names_equal((ntfschar*)((u8*)a +
				le16_to_cpu(a->name_offset)), a->name_length,
				al_name, al_name_len, CASE_SENSITIVE,
				vol->upcase, vol->upcase_len))
			break;
		ctx->attr = a;

		/*
		 * If no @val specified or @val specified and it matches, we
		 * have found it! Also, if !@type, it is an enumeration, so we
		 * want the current attribute.
		 */			
		if ((type == AT_UNUSED) || !val || (!a->non_resident && le32_to_cpu(// fix by yangli
				a->data.resident.value_length) == val_len &&
				!memcmp((u8*)a +
				le16_to_cpu(a->data.resident.value_offset),
				val, val_len))) {
			ntfs_debug("Done, found.");
			return 0;
		}
do_next_attr:
		/* Proceed to the next attribute in the current mft record. */
		a = (ATTR_RECORD*)((u8*)a + le32_to_cpu(a->length));
		goto do_next_attr_loop;
	}
	if (!err) {
		ntfs_error(vol->sb, "Base inode 0x%lx contains corrupt "
				"attribute list attribute.%s", base_ni->mft_no,
				es);
		err = -EIO;
	}
	if (ni != base_ni) {
		if (ni)
			unmap_extent_mft_record(ni);
		ctx->ntfs_ino = base_ni;
		ctx->mrec = ctx->base_mrec;
		ctx->attr = ctx->base_attr;
	}
	if (err != -ENOMEM)
		NVolSetErrors(vol);
	return err;
not_found:
	/*
	 * If we were looking for AT_END, we reset the search context @ctx and
	 * use ntfs_attr_find() to seek to the end of the base mft record.
	 */
	if (type == AT_UNUSED || type == AT_END) {//fix by yangli
		ntfs_attr_reinit_search_ctx(ctx);
		return ntfs_attr_find(AT_END, name, name_len, ic, val, val_len,
				ctx);
	}
	/*
	 * The attribute was not found.  Before we return, we want to ensure
	 * @ctx->mrec and @ctx->attr indicate the position at which the
	 * attribute should be inserted in the base mft record.  Since we also
	 * want to preserve @ctx->al_entry we cannot reinitialize the search
	 * context using ntfs_attr_reinit_search_ctx() as this would set
	 * @ctx->al_entry to NULL.  Thus we do the necessary bits manually (see
	 * ntfs_attr_init_search_ctx() below).  Note, we _only_ preserve
	 * @ctx->al_entry as the remaining fields (base_*) are identical to
	 * their non base_ counterparts and we cannot set @ctx->base_attr
	 * correctly yet as we do not know what @ctx->attr will be set to by
	 * the call to ntfs_attr_find() below.
	 */
	if (ni != base_ni)
		unmap_extent_mft_record(ni);
	ctx->mrec = ctx->base_mrec;
	ctx->attr = (ATTR_RECORD*)((u8*)ctx->mrec +
			le16_to_cpu(ctx->mrec->attrs_offset));
	ctx->is_first = true;
	ctx->ntfs_ino = base_ni;
	ctx->base_ntfs_ino = NULL;
	ctx->base_mrec = NULL;
	ctx->base_attr = NULL;
	/*
	 * In case there are multiple matches in the base mft record, need to
	 * keep enumerating until we get an attribute not found response (or
	 * another error), otherwise we would keep returning the same attribute
	 * over and over again and all programs using us for enumeration would
	 * lock up in a tight loop.
	 */
	do {
		err = ntfs_attr_find(type, name, name_len, ic, val, val_len,
				ctx);
	} while (!err);
	ntfs_debug("Done, not found.");
	return err;
}

/**在ntfs inode中查找属性attribute
 * ntfs_attr_lookup - find an attribute in an ntfs inode
 * @type:	attribute type to find
 * @name:	attribute name to find (optional, i.e. NULL means don't care)
 * @name_len:	attribute name length (only needed if @name present)
 * @ic:		IGNORE_CASE or CASE_SENSITIVE (ignored if @name not present)
 * @lowest_vcn:	lowest vcn to find (optional, non-resident attributes only)
 * @val:	attribute value to find (optional, resident attributes only)
 * @val_len:	attribute value length
 * @ctx:	search context with mft record and attribute to search from
 *
 * Find an attribute in an ntfs inode.  On first search @ctx->ntfs_ino must
 * be the base mft record and @ctx must have been obtained from a call to
 * ntfs_attr_get_search_ctx().
 首先ctx->ntfs_ino必须是主节点,ctx必须是由  ntfs_attr_get_search_ctx()
 获得的。
 *
 * This function transparently handles attribute lists and @ctx is used to
 * continue searches where they were left off at.
 该函数透明地处理属性列表attribute lists。
 *
 * After finishing with the attribute/mft record you need to call
 * ntfs_attr_put_search_ctx() to cleanup the search context (unmapping any
 * mapped inodes, etc).
 完了要调用ntfs_attr_put_search_ctx()。
 *
 * Return 0 if the search was successful and -errno if not.
 *成功返回0，否则返回-errno
 
 * When 0, @ctx->attr is the found attribute and it is in mft record
 * @ctx->mrec.  If an attribute list attribute is present, @ctx->al_entry is
 * the attribute list entry of the found attribute.
 *当返回0时，ctx->attr即要找的属性，并且位于ctx->mrec中。
 如果存在属性列表，ctx->al_entry是要找属性的属性列表入口.
 
 * When -ENOENT, @ctx->attr is the attribute which collates just after the
 * attribute being searched for, i.e. if one wants to add the attribute to the
 * mft record this is the correct place to insert it into.  If an attribute
 * list attribute is present, @ctx->al_entry is the attribute list entry which
 * collates just after the attribute list entry of the attribute being searched
 * for, i.e. if one wants to add the attribute to the mft record this is the
 * correct place to insert its attribute list entry into.
 *
 1、如果返回-ENOENT，ctx->attr是要找属性的下一个属性，
 如果想往mft record中添加属性,此时的ctx->attr就是正确的位置.

 2、如果存在属性列表,此时的ctx->al_entry就是要查找的属性所在的列表的下一 列表，
 如果想往mft record中添加属性,ctx->al_entry就是要插入对应属性列表的正确位置。
 
 * When -errno != -ENOENT, an error occured during the lookup.  @ctx->attr is
 * then undefined and in particular you should not rely on it not changing.
 当 -errno 1= -ENOENT时，出错了!!
 */
int ntfs_attr_lookup(const ATTR_TYPE type, const ntfschar *name,
		const u32 name_len, const IGNORE_CASE_BOOL ic,
		const VCN lowest_vcn, const u8 *val, const u32 val_len,
		ntfs_attr_search_ctx *ctx)
{
	ntfs_inode *base_ni;

	//ntfs_debug("Entering.");

	DEBUG0_ON(IS_ERR(ctx->mrec));	
	if (ctx->base_ntfs_ino)
		base_ni = ctx->base_ntfs_ino;
	else
		base_ni = ctx->ntfs_ino;
	/* Sanity check, just for debugging really. */
	DEBUG0_ON(!base_ni);
	if (!NInoAttrList(base_ni) || type == AT_ATTRIBUTE_LIST)
		return ntfs_attr_find(type, name, name_len, ic, val, val_len,
				ctx);
	return ntfs_external_attr_find(type, name, name_len, ic, lowest_vcn,//find in Attribute list
			val, val_len, ctx);
}

/**
 * ntfs_attr_init_search_ctx - initialize an attribute search context
 * @ctx:	attribute search context to initialize
 * @ni:		ntfs inode with which to initialize the search context
 * @mrec:	mft record with which to initialize the search context
 *
 * Initialize the attribute search context @ctx with @ni and @mrec.
 */
static inline void ntfs_attr_init_search_ctx(ntfs_attr_search_ctx *ctx,
		ntfs_inode *ni, MFT_RECORD *mrec)
{
	*ctx = (ntfs_attr_search_ctx) {
		.mrec = mrec,
		/* Sanity checks are performed elsewhere. */
		.attr = (ATTR_RECORD*)((u8*)mrec +	//$mft 中的第一个属性
				le16_to_cpu(mrec->attrs_offset)),
		.is_first = true,//搜索时跳过attr(也就是$standard_information)
		.ntfs_ino = ni,
	};
}

/**
 * ntfs_attr_reinit_search_ctx - reinitialize an attribute search context
 * @ctx:	attribute search context to reinitialize
 *
 * Reinitialize the attribute search context @ctx, unmapping an associated
 * extent mft record if present, and initialize the search context again.
 *
 * This is used when a search for a new attribute is being started to reset
 * the search context to the beginning.
 */
void ntfs_attr_reinit_search_ctx(ntfs_attr_search_ctx *ctx)
{
	if (likely(!ctx->base_ntfs_ino)) {
		/* No attribute list. */
		ctx->is_first = true;
		/* Sanity checks are performed elsewhere. */
		ctx->attr = (ATTR_RECORD*)((u8*)ctx->mrec +
				le16_to_cpu(ctx->mrec->attrs_offset));
		/*
		 * This needs resetting due to ntfs_external_attr_find() which
		 * can leave it set despite having zeroed ctx->base_ntfs_ino.
		 */
		ctx->al_entry = NULL;
		return;
	} /* Attribute list. */
	if (ctx->ntfs_ino != ctx->base_ntfs_ino)
		unmap_extent_mft_record(ctx->ntfs_ino);
	ntfs_attr_init_search_ctx(ctx, ctx->base_ntfs_ino, ctx->base_mrec);
	return;
}

/**分配或初始化一个属性搜索内容
 * ntfs_attr_get_search_ctx - allocate/initialize a new attribute search context
 * @ni:		ntfs inode with which to initialize the search context
 * @mrec:	mft record with which to initialize the search context
 *
 * Allocate a new attribute search context, initialize it with @ni and @mrec,
 * and return it. Return NULL if allocation failed.
 */
ntfs_attr_search_ctx *ntfs_attr_get_search_ctx(ntfs_inode *ni, MFT_RECORD *mrec)
{
	ntfs_attr_search_ctx *ctx;

	ctx = kmem_cache_alloc(ntfs_attr_ctx_cache, GFP_NOFS);
	if (ctx)
		ntfs_attr_init_search_ctx(ctx, ni, mrec);
	return ctx;
}

/**
 * ntfs_attr_put_search_ctx - release an attribute search context
 * @ctx:	attribute search context to free
 *
 * Release the attribute search context @ctx, unmapping an associated extent
 * mft record if present.
 */
void ntfs_attr_put_search_ctx(ntfs_attr_search_ctx *ctx)
{
	if (ctx->base_ntfs_ino && ctx->ntfs_ino != ctx->base_ntfs_ino)
		unmap_extent_mft_record(ctx->ntfs_ino);
	kmem_cache_free(ntfs_attr_ctx_cache, ctx);
	return;
}

#ifdef NTFS_RW
/**在$Attrdef 系统文件中查找属性type
 * ntfs_attr_find_in_attrdef - find an attribute in the $AttrDef system file
 * @vol:	ntfs volume to which the attribute belongs
 * @type:	attribute type which to find
 *
 * Search for the attribute definition record corresponding to the attribute
 * @type in the $AttrDef system file.
 *
 * Return the attribute type definition record if found and NULL if not found.
 */
static ATTR_DEF *ntfs_attr_find_in_attrdef(const ntfs_volume *vol,
		const ATTR_TYPE type)
{
	ATTR_DEF *ad;

	DEBUG0_ON(!vol->attrdef);
	DEBUG0_ON(!type);
	for (ad = vol->attrdef; (u8*)ad - (u8*)vol->attrdef <
			vol->attrdef_size && ad->type; ++ad) {
		/* We have not found it yet, carry on searching. */
		if (likely(le32_to_cpu(ad->type) < le32_to_cpu(type)))
			continue;
		/* We found the attribute; return it. */
		if (likely(ad->type == type))
			return ad;
		/* We have gone too far already.  No point in continuing. */
		break;
	}
	/* Attribute not found. */
	ntfs_debug("Attribute type 0x%x not found in $AttrDef.",
			le32_to_cpu(type));
	return NULL;
}

/**检测属性type的合法性
 * ntfs_attr_size_bounds_check - check a size of an attribute type for validity
 * @vol:	ntfs volume to which the attribute belongs
 * @type:	attribute type which to check
 * @size:	size which to check
 *
 * Check whether the @size in bytes is valid for an attribute of @type on the
 * ntfs volume @vol.  This information is obtained from $AttrDef system file.
 *
 * Return 0 if valid, -ERANGE if not valid, or -ENOENT if the attribute is not
 * listed in $AttrDef.
 */
int ntfs_attr_size_bounds_check(const ntfs_volume *vol, const ATTR_TYPE type,
		const s64 size)
{
	ATTR_DEF *ad;

	DEBUG0_ON(size < 0);
	/*
	 * $ATTRIBUTE_LIST has a maximum size of 256kiB, but this is not
	 * listed in $AttrDef.
	 */
	if (unlikely(type == AT_ATTRIBUTE_LIST && size > 256 * 1024))
		return -ERANGE;
	/* Get the $AttrDef entry for the attribute @type. */
	ad = ntfs_attr_find_in_attrdef(vol, type);
	if (unlikely(!ad))
		return -ENOENT;
	/* Do the bounds check. */
	if (((sle64_to_cpu(ad->min_size) > 0) &&
			size < sle64_to_cpu(ad->min_size)) ||
			((sle64_to_cpu(ad->max_size) > 0) && size >
			sle64_to_cpu(ad->max_size)))
		return -ERANGE;
	return 0;
}

/**检测属性是否可以为非驻留
 * ntfs_attr_can_be_non_resident - check if an attribute can be non-resident
 * @vol:	ntfs volume to which the attribute belongs
 * @type:	attribute type which to check
 *
 * Check whether the attribute of @type on the ntfs volume @vol is allowed to
 * be non-resident.  This information is obtained from $AttrDef system file.
 *
 * Return 0 if the attribute is allowed to be non-resident, -EPERM if not, and
 * -ENOENT if the attribute is not listed in $AttrDef.
 */
int ntfs_attr_can_be_non_resident(const ntfs_volume *vol, const ATTR_TYPE type)
{
	ATTR_DEF *ad;

	/* Find the attribute definition record in $AttrDef. */
	ad = ntfs_attr_find_in_attrdef(vol, type);
	if (unlikely(!ad))//不存在该属性
		return -ENOENT;
	/* Check the flags and return the result. */
	if (ad->flags & ATTR_DEF_RESIDENT)//属性为驻留
		return -EPERM;
	return 0;
}

/**检测属性是否为驻留
 * ntfs_attr_can_be_resident - check if an attribute can be resident
 * @vol:	ntfs volume to which the attribute belongs
 * @type:	attribute type which to check
 *
 * Check whether the attribute of @type on the ntfs volume @vol is allowed to
 * be resident.  This information is derived from our ntfs knowledge and may
 * not be completely accurate, especially when user defined attributes are
 * present.  Basically we allow everything to be resident except for index
 * allocation and $EA attributes.
 *
 * Return 0 if the attribute is allowed to be non-resident and -EPERM if not.
 *
 * Warning: In the system file $MFT the attribute $Bitmap must be non-resident
 *	    otherwise windows will not boot (blue screen of death)!  We cannot
 *	    check for this here as we do not know which inode's $Bitmap is
 *	    being asked about so the caller needs to special case this.
 */
int ntfs_attr_can_be_resident(const ntfs_volume *vol, const ATTR_TYPE type)
{
	if (type == AT_INDEX_ALLOCATION)
		return -EPERM;
	return 0;
}

/**调整属性记录的大小
 * ntfs_attr_record_resize - resize an attribute record
 * @m:		mft record containing attribute record
 * @a:		attribute record to resize
 * @new_size:	new size in bytes to which to resize the attribute record @a
 *
 * Resize the attribute record @a, i.e. the resident part of the attribute, in
 * the mft record @m to @new_size bytes.
 *
 * Return 0 on success and -errno on error.  The following error codes are
 * defined:
 *	-ENOSPC	- Not enough space in the mft record @m to perform the resize.
 *
 * Note: On error, no modifications have been performed whatsoever.
 *
 * Warning: If you make a record smaller without having copied all the data you
 *	    are interested in the data may be overwritten.
 注意:如果你将记录变小了，但是没初始化，记录将被覆盖
 */
int ntfs_attr_record_resize(MFT_RECORD *m, ATTR_RECORD *a, u32 new_size)
{
	ntfs_trace("Entering for new_size :0x%x.", new_size);
	/* Align to 8 bytes if it is not already done. */
	if (new_size & 7)//8字节对齐
		new_size = (new_size + 7) & ~7;
	/* If the actual attribute length has changed, move things around. */
	if (new_size != le32_to_cpu(a->length)) {
		u32 new_muse = le32_to_cpu(m->bytes_in_use) -
				le32_to_cpu(a->length) + new_size;
		/* Not enough space in this mft record. */
		if (new_muse > le32_to_cpu(m->bytes_allocated))//mft record分配的空间不够
			return -ENOSPC;
		
#ifdef NTFS_MODIFY
		if (a->type == AT_INDEX_ROOT  &&  new_size > le32_to_cpu(a->length)  &&
		    new_muse + 120 > le32_to_cpu(m->bytes_allocated)  &&  le32_to_cpu(m->bytes_in_use) + 120 <=  le32_to_cpu(m->bytes_allocated)) {
			//errno = ENOSPC;
			ntfs_debug("Too big INDEX_ROOT (0x%x > 0x%x)\n",
					new_muse,  le32_to_cpu(m->bytes_allocated));
			return STATUS_RESIDENT_ATTRIBUTE_FILLED_MFT; //INDEX_ROOT大小不够，需转换为INDEX_ALLOCATION
		}
		
#endif
		/* Move attributes following @a to their new location. */
		memmove((u8*)a + new_size, (u8*)a + le32_to_cpu(a->length),//把a之后的一切都移动到a+new_size之后
				le32_to_cpu(m->bytes_in_use) - ((u8*)a -
				(u8*)m) - le32_to_cpu(a->length));
		/* Adjust @m to reflect the change in used space. */
		m->bytes_in_use = cpu_to_le32(new_muse);
		/* Adjust @a to reflect the new size. */
		if (new_size >= offsetof(ATTR_REC, length) + sizeof(a->length))
			a->length = cpu_to_le32(new_size);
	}
	ntfs_trace("Done");
	return 0;
}

/**改变驻留属性的值
 * ntfs_resident_attr_value_resize - resize the value of a resident attribute
 * @m:		mft record containing attribute record
 * @a:		attribute record whose value to resize
 * @new_size:	new size in bytes to which to resize the attribute value of @a
 *
 * Resize the value of the attribute @a in the mft record @m to @new_size bytes.
 * If the value is made bigger, the newly allocated space is cleared.
 *
 * Return 0 on success and -errno on error.  The following error codes are
 * defined:
 *	-ENOSPC	- Not enough space in the mft record @m to perform the resize.
 *
 * Note: On error, no modifications have been performed whatsoever.
 *
 * Warning: If you make a record smaller without having copied all the data you
 *	    are interested in the data may be overwritten.
 返回值: STATUS_RESIDENT_ATTRIBUTE_FILLED_MFT; //INDEX_ROOT大小不够，需转换为INDEX_ALLOCATION
 */
int ntfs_resident_attr_value_resize(MFT_RECORD *m, ATTR_RECORD *a,
		const u32 new_size)
{
	u32 old_size;
	int ret = 0;
	
	ntfs_trace("Entering");
	/* Resize the resident part of the attribute record. */
	ret = ntfs_attr_record_resize(m, a,
			le16_to_cpu(a->data.resident.value_offset) + new_size);
	if (ret  != 0 )
		return ret;
	/*
	 * The resize succeeded!  If we made the attribute value bigger, clear
	 * the area between the old size and @new_size.
	 */
	old_size = le32_to_cpu(a->data.resident.value_length);
	if (new_size > old_size)
		memset((u8*)a + le16_to_cpu(a->data.resident.value_offset) +
				old_size, 0, new_size - old_size);
	/* Finally update the length of the attribute value. */
	a->data.resident.value_length = cpu_to_le32(new_size);
	return ret;
}

/**将驻留属性转化为非驻留属性
 * ntfs_attr_make_non_resident - convert a resident to a non-resident attribute
 * @ni:		ntfs inode describing the attribute to convert
 * @data_size:	size of the resident data to copy to the non-resident attribute
 *
 * Convert the resident ntfs attribute described by the ntfs inode @ni to a
 * non-resident one.
 *
 * @data_size must be equal to the attribute value size.  This is needed since
 * we need to know the size before we can map the mft record and our callers
 * always know it.  The reason we cannot simply read the size from the vfs
 * inode i_size is that this is not necessarily uptodate.  This happens when
 * ntfs_attr_make_non_resident() is called in the ->truncate call path(s).
 *
 * Return 0 on success and -errno on error.  The following error return codes
 * are defined:
 *	-EPERM	- The attribute is not allowed to be non-resident.
 *	-ENOMEM	- Not enough memory.
 *	-ENOSPC	- Not enough disk space.
 *	-EINVAL	- Attribute not defined on the volume.
 *	-EIO	- I/o error or other error.
 * Note that -ENOSPC is also returned in the case that there is not enough
 * space in the mft record to do the conversion.  This can happen when the mft
 * record is already very full.  The caller is responsible for trying to make
 * space in the mft record and trying again.  FIXME: Do we need a separate
 * error return code for this kind of -ENOSPC or is it always worth trying
 * again in case the attribute may then fit in a resident state so no need to
 * make it non-resident at all?  Ho-hum...  (AIA)
 *
 * NOTE to self: No changes in the attribute list are required to move from
 *		 a resident to a non-resident attribute.
 *
 * Locking: - The caller must hold i_mutex on the inode.
 */
int ntfs_attr_make_non_resident(ntfs_inode *ni, const u32 data_size)
{
	s64 new_size;
	struct inode *vi = VFS_I(ni);
	ntfs_volume *vol = ni->vol;
	ntfs_inode *base_ni;
	MFT_RECORD *m;
	ATTR_RECORD *a;
	ntfs_attr_search_ctx *ctx;
	struct page *page;
	runlist_element *rl;
	u8 *kaddr;
	unsigned long flags;
	int mp_size, mp_ofs, name_ofs, arec_size, err, err2;
	u32 attr_size;
	u8 old_res_attr_flags;

	/* Check that the attribute is allowed to be non-resident. */
	err = ntfs_attr_can_be_non_resident(vol, ni->type);
	if (unlikely(err)) {
		if (err == -EPERM)
			ntfs_debug("Attribute is not allowed to be "
					"non-resident.");
		else
			ntfs_debug("Attribute not defined on the NTFS "
					"volume!");
		return err;
	}
	/*
	 * FIXME: Compressed and encrypted attributes are not supported when
	 * writing and we should never have gotten here for them.
	 */
	DEBUG0_ON(NInoCompressed(ni));
	DEBUG0_ON(NInoEncrypted(ni));
	/*
	 * The size needs to be aligned to a cluster boundary for allocation
	 * purposes.
	 */
	new_size = (data_size + vol->cluster_size - 1) &
			~(vol->cluster_size - 1);
	if (new_size > 0) {
		/*
		 * Will need the page later and since the page lock nests
		 * outside all ntfs locks, we need to get the page now.
		 */
		page = find_or_create_page(vi->i_mapping, 0,
				mapping_gfp_mask(vi->i_mapping));
		if (unlikely(!page))
			return -ENOMEM;
		/* Start by allocating clusters to hold the attribute value. */
		rl = ntfs_cluster_alloc(vol, 0, new_size >>
				vol->cluster_size_bits, -1, DATA_ZONE, true);
		if (IS_ERR(rl)) {
			err = PTR_ERR(rl);
			ntfs_debug("Failed to allocate cluster%s, error code "
					"%i.", (new_size >>
					vol->cluster_size_bits) > 1 ? "s" : "",
					err);
			goto page_err_out;
		}
	} else {
		rl = NULL;
		page = NULL;
	}
	/* Determine the size of the mapping pairs array. */
	mp_size = ntfs_get_size_for_mapping_pairs(vol, rl, 0, -1);
	if (unlikely(mp_size < 0)) {
		err = mp_size;
		ntfs_debug("Failed to get size for mapping pairs array, error "
				"code %i.", err);
		goto rl_err_out;
	}
 	if(ni->mft_no == 0)
		 ntfs_debug("down_write");
	 down_write(&ni->runlist.lock);
	if (!NInoAttr(ni))
		base_ni = ni;
	else
		base_ni = ni->ext.base_ntfs_ino;
	m = map_mft_record(base_ni);
	if (IS_ERR(m)) {
		err = PTR_ERR(m);
		m = NULL;
		ctx = NULL;
		goto err_out;
	}
	ctx = ntfs_attr_get_search_ctx(base_ni, m);
	if (unlikely(!ctx)) {
		err = -ENOMEM;
		goto err_out;
	}
	err = ntfs_attr_lookup(ni->type, ni->name, ni->name_len,
			CASE_SENSITIVE, 0, NULL, 0, ctx);
	if (unlikely(err)) {
		if (err == -ENOENT)
			err = -EIO;
		goto err_out;
	}
	m = ctx->mrec;
	a = ctx->attr;
	DEBUG0_ON(NInoNonResident(ni));
	DEBUG0_ON(a->non_resident);
	/*
	 * Calculate new offsets for the name and the mapping pairs array.
	 */
	if (NInoSparse(ni) || NInoCompressed(ni))
		name_ofs = (offsetof(ATTR_REC,
				data.non_resident.compressed_size) +
				sizeof(a->data.non_resident.compressed_size) +
				7) & ~7;
	else
		name_ofs = (offsetof(ATTR_REC,
				data.non_resident.compressed_size) + 7) & ~7;
	mp_ofs = (name_ofs + a->name_length * sizeof(ntfschar) + 7) & ~7;
	/*
	 * Determine the size of the resident part of the now non-resident
	 * attribute record.
	 */
	arec_size = (mp_ofs + mp_size + 7) & ~7;
	/*
	 * If the page is not uptodate bring it uptodate by copying from the
	 * attribute value.
	 */
	attr_size = le32_to_cpu(a->data.resident.value_length);
	DEBUG0_ON(attr_size != data_size);
	if (page && !PageUptodate(page)) {
		kaddr = kmap_atomic(page, KM_USER0);
		memcpy(kaddr, (u8*)a +
				le16_to_cpu(a->data.resident.value_offset),
				attr_size);
		memset(kaddr + attr_size, 0, PAGE_CACHE_SIZE - attr_size);
		kunmap_atomic(kaddr, KM_USER0);
		flush_dcache_page(page);
		SetPageUptodate(page);
	}
	/* Backup the attribute flag. */
	old_res_attr_flags = a->data.resident.flags;
	/* Resize the resident part of the attribute record. */
	err = ntfs_attr_record_resize(m, a, arec_size);
	if (unlikely(err))
		goto err_out;
	/*
	 * Convert the resident part of the attribute record to describe a
	 * non-resident attribute.
	 */
	a->non_resident = 1;
	/* Move the attribute name if it exists and update the offset. */
	if (a->name_length)
		memmove((u8*)a + name_ofs, (u8*)a + le16_to_cpu(a->name_offset),
				a->name_length * sizeof(ntfschar));
	a->name_offset = cpu_to_le16(name_ofs);
	/* Setup the fields specific to non-resident attributes. */
	a->data.non_resident.lowest_vcn = 0;
	a->data.non_resident.highest_vcn = cpu_to_sle64((new_size - 1) >>
			vol->cluster_size_bits);
	a->data.non_resident.mapping_pairs_offset = cpu_to_le16(mp_ofs);
	memset(&a->data.non_resident.reserved, 0,
			sizeof(a->data.non_resident.reserved));
	a->data.non_resident.allocated_size = cpu_to_sle64(new_size);
	a->data.non_resident.data_size =
			a->data.non_resident.initialized_size =
			cpu_to_sle64(attr_size);
	if (NInoSparse(ni) || NInoCompressed(ni)) {
		a->data.non_resident.compression_unit = 0;
		if (NInoCompressed(ni) || vol->major_ver < 3)
			a->data.non_resident.compression_unit = 4;
		a->data.non_resident.compressed_size =
				a->data.non_resident.allocated_size;
	} else
		a->data.non_resident.compression_unit = 0;
	/* Generate the mapping pairs array into the attribute record. */
	err = ntfs_mapping_pairs_build(vol, (u8*)a + mp_ofs,
			arec_size - mp_ofs, rl, 0, -1, NULL);
	if (unlikely(err)) {
		ntfs_debug("Failed to build mapping pairs, error code %i.",
				err);
		goto undo_err_out;
	}
	/* Setup the in-memory attribute structure to be non-resident. */
	ni->runlist.rl = rl;
	write_lock_irqsave(&ni->size_lock, flags);
	ni->allocated_size = new_size;
	if (NInoSparse(ni) || NInoCompressed(ni)) {
		ni->itype.compressed.size = ni->allocated_size;
		if (a->data.non_resident.compression_unit) {
			ni->itype.compressed.block_size = 1U << (a->data.
					non_resident.compression_unit +
					vol->cluster_size_bits);
			ni->itype.compressed.block_size_bits =
					ffs(ni->itype.compressed.block_size) -
					1;
			ni->itype.compressed.block_clusters = 1U <<
					a->data.non_resident.compression_unit;
		} else {
			ni->itype.compressed.block_size = 0;
			ni->itype.compressed.block_size_bits = 0;
			ni->itype.compressed.block_clusters = 0;
		}
		vi->i_blocks = ni->itype.compressed.size >> 9;
	} else
		vi->i_blocks = ni->allocated_size >> 9;
	write_unlock_irqrestore(&ni->size_lock, flags);
	/*
	 * This needs to be last since the address space operations ->readpage
	 * and ->writepage can run concurrently with us as they are not
	 * serialized on i_mutex.  Note, we are not allowed to fail once we flip
	 * this switch, which is another reason to do this last.
	 */
	NInoSetNonResident(ni);
	/* Mark the mft record dirty, so it gets written back. */
	flush_dcache_mft_record_page(ctx->ntfs_ino);
	mark_mft_record_dirty(ctx->ntfs_ino);
	ntfs_attr_put_search_ctx(ctx);
	unmap_mft_record(base_ni);
	 if(ni->mft_no == 0)
		ntfs_debug("up_write");

	up_write(&ni->runlist.lock);
	if (page) {
		set_page_dirty(page);
		unlock_page(page);
		mark_page_accessed(page);
		page_cache_release(page);
	}
	ntfs_debug("Done.");
	return 0;
undo_err_out:
	/* Convert the attribute back into a resident attribute. */
	a->non_resident = 0;
	/* Move the attribute name if it exists and update the offset. */
	name_ofs = (offsetof(ATTR_RECORD, data.resident.reserved) +
			sizeof(a->data.resident.reserved) + 7) & ~7;
	if (a->name_length)
		memmove((u8*)a + name_ofs, (u8*)a + le16_to_cpu(a->name_offset),
				a->name_length * sizeof(ntfschar));
	mp_ofs = (name_ofs + a->name_length * sizeof(ntfschar) + 7) & ~7;
	a->name_offset = cpu_to_le16(name_ofs);
	arec_size = (mp_ofs + attr_size + 7) & ~7;
	/* Resize the resident part of the attribute record. */
	err2 = ntfs_attr_record_resize(m, a, arec_size);
	if (unlikely(err2)) {
		/*
		 * This cannot happen (well if memory corruption is at work it
		 * could happen in theory), but deal with it as well as we can.
		 * If the old size is too small, truncate the attribute,
		 * otherwise simply give it a larger allocated size.
		 * FIXME: Should check whether chkdsk complains when the
		 * allocated size is much bigger than the resident value size.
		 */
		arec_size = le32_to_cpu(a->length);
		if ((mp_ofs + attr_size) > arec_size) {
			err2 = attr_size;
			attr_size = arec_size - mp_ofs;
			ntfs_error(vol->sb, "Failed to undo partial resident "
					"to non-resident attribute "
					"conversion.  Truncating inode 0x%lx, "
					"attribute type 0x%x from %i bytes to "
					"%i bytes to maintain metadata "
					"consistency.  THIS MEANS YOU ARE "
					"LOSING %i BYTES DATA FROM THIS %s.",
					vi->i_ino,
					(unsigned)le32_to_cpu(ni->type),
					err2, attr_size, err2 - attr_size,
					((ni->type == AT_DATA) &&
					!ni->name_len) ? "FILE": "ATTRIBUTE");
			write_lock_irqsave(&ni->size_lock, flags);
			ni->initialized_size = attr_size;
			i_size_write(vi, attr_size);
			write_unlock_irqrestore(&ni->size_lock, flags);
		}
	}
	/* Setup the fields specific to resident attributes. */
	a->data.resident.value_length = cpu_to_le32(attr_size);
	a->data.resident.value_offset = cpu_to_le16(mp_ofs);
	a->data.resident.flags = old_res_attr_flags;
	memset(&a->data.resident.reserved, 0,
			sizeof(a->data.resident.reserved));
	/* Copy the data from the page back to the attribute value. */
	if (page) {
		kaddr = kmap_atomic(page, KM_USER0);
		memcpy((u8*)a + mp_ofs, kaddr, attr_size);
		kunmap_atomic(kaddr, KM_USER0);
	}
	/* Setup the allocated size in the ntfs inode in case it changed. */
	write_lock_irqsave(&ni->size_lock, flags);
	ni->allocated_size = arec_size - mp_ofs;
	write_unlock_irqrestore(&ni->size_lock, flags);
	/* Mark the mft record dirty, so it gets written back. */
	flush_dcache_mft_record_page(ctx->ntfs_ino);
	mark_mft_record_dirty(ctx->ntfs_ino);
err_out:
	if (ctx)
		ntfs_attr_put_search_ctx(ctx);
	if (m)
		unmap_mft_record(base_ni);
	ni->runlist.rl = NULL;
	 if(ni->mft_no == 0)
		ntfs_debug("up_write");

	up_write(&ni->runlist.lock);
rl_err_out:
	if (rl) {
		if (ntfs_cluster_free_from_rl(vol, rl) < 0) {
			ntfs_error(vol->sb, "Failed to release allocated "
					"cluster(s) in error code path.  Run "
					"chkdsk to recover the lost "
					"cluster(s).");
			NVolSetErrors(vol);
		}
		ntfs_free(rl);
page_err_out:
		unlock_page(page);
		page_cache_release(page);
	}
	if (err == -EINVAL)
		err = -EIO;
	return err;
}

/**将ni对应的mft record中的属性tpye转换为非驻留的*/
int ntfs_attr_make_non_resident2(ntfs_inode *ni, const ATTR_TYPE type,
		ntfschar *name, u32 name_len, const u32 data_size)
{
	s64 new_size;
	struct inode *vi = VFS_I(ni);
	ntfs_volume *vol = ni->vol;
	ntfs_inode *base_ni;
	MFT_RECORD *m;
	ATTR_RECORD *a;
	ntfs_attr_search_ctx *ctx;
	struct page *page;
	runlist_element *rl;
	u8 *kaddr;
	unsigned long flags;
	int mp_size, mp_ofs, name_ofs, arec_size, err, err2;
	u32 attr_size;
	u8 old_res_attr_flags;

	/* Check that the attribute is allowed to be non-resident. */
	err = ntfs_attr_can_be_non_resident(vol, type);
	if (unlikely(err)) {
		if (err == -EPERM)
			ntfs_debug("Attribute is not allowed to be "
					"non-resident.");
		else
			ntfs_debug("Attribute not defined on the NTFS "
					"volume!");
		return err;
	}
	/*
	 * FIXME: Compressed and encrypted attributes are not supported when
	 * writing and we should never have gotten here for them.
	 */
	DEBUG0_ON(NInoCompressed(ni));
	DEBUG0_ON(NInoEncrypted(ni));
	/*
	 * The size needs to be aligned to a cluster boundary for allocation
	 * purposes.
	 */
	new_size = (data_size + vol->cluster_size - 1) &
			~(vol->cluster_size - 1);
	if (new_size > 0) {
		/*
		 * Will need the page later and since the page lock nests
		 * outside all ntfs locks, we need to get the page now.
		 */
		page = find_or_create_page(vi->i_mapping, 0,
				mapping_gfp_mask(vi->i_mapping));
		if (unlikely(!page))
			return -ENOMEM;
		/* Start by allocating clusters to hold the attribute value. */
		rl = ntfs_cluster_alloc(vol, 0, new_size >>
				vol->cluster_size_bits, -1, DATA_ZONE, true);
		if (IS_ERR(rl)) {
			err = PTR_ERR(rl);
			ntfs_debug("Failed to allocate cluster%s, error code "
					"%i.", (new_size >>
					vol->cluster_size_bits) > 1 ? "s" : "",
					err);
			goto page_err_out;
		}
	} else {
		rl = NULL;
		page = NULL;
	}
	/* Determine the size of the mapping pairs array. */
	mp_size = ntfs_get_size_for_mapping_pairs(vol, rl, 0, -1);
	if (unlikely(mp_size < 0)) {
		err = mp_size;
		ntfs_debug("Failed to get size for mapping pairs array, error "
				"code %i.", err);
		goto rl_err_out;
	}
 	if(ni->mft_no == 0)
		 ntfs_debug("down_write");
 	down_write(&ni->runlist.lock);
	if (!NInoAttr(ni))
		base_ni = ni;
	else
		base_ni = ni->ext.base_ntfs_ino;
	m = map_mft_record(base_ni);
	if (IS_ERR(m)) {
		err = PTR_ERR(m);
		m = NULL;
		ctx = NULL;
		goto err_out;
	}
	ctx = ntfs_attr_get_search_ctx(base_ni, m);
	if (unlikely(!ctx)) {
		err = -ENOMEM;
		goto err_out;
	}
	err = ntfs_attr_lookup(type, name, name_len,
			CASE_SENSITIVE, 0, NULL, 0, ctx);
	if (unlikely(err)) {
		if (err == -ENOENT)
			err = -EIO;
		goto err_out;
	}
	m = ctx->mrec;
	a = ctx->attr;
	DEBUG0_ON(NInoNonResident(ni));
	DEBUG0_ON(a->non_resident);
	/*
	 * Calculate new offsets for the name and the mapping pairs array.
	 */
	if (NInoSparse(ni) || NInoCompressed(ni))
		name_ofs = (offsetof(ATTR_REC,
				data.non_resident.compressed_size) +
				sizeof(a->data.non_resident.compressed_size) +
				7) & ~7;
	else
		name_ofs = (offsetof(ATTR_REC,
				data.non_resident.compressed_size) + 7) & ~7;
	mp_ofs = (name_ofs + a->name_length * sizeof(ntfschar) + 7) & ~7;
	/*
	 * Determine the size of the resident part of the now non-resident
	 * attribute record.
	 */
	arec_size = (mp_ofs + mp_size + 7) & ~7;
	/*
	 * If the page is not uptodate bring it uptodate by copying from the
	 * attribute value.
	 */
	attr_size = le32_to_cpu(a->data.resident.value_length);
	DEBUG0_ON(attr_size != data_size);
	if (page && !PageUptodate(page)) {
		kaddr = kmap_atomic(page, KM_USER0);
		memcpy(kaddr, (u8*)a +
				le16_to_cpu(a->data.resident.value_offset),
				attr_size);
		memset(kaddr + attr_size, 0, PAGE_CACHE_SIZE - attr_size);
		kunmap_atomic(kaddr, KM_USER0);
		flush_dcache_page(page);
		SetPageUptodate(page);
	}
	/* Backup the attribute flag. */
	old_res_attr_flags = a->data.resident.flags;
	/* Resize the resident part of the attribute record. */
	err = ntfs_attr_record_resize(m, a, arec_size);
	if (unlikely(err))
		goto err_out;
	/*
	 * Convert the resident part of the attribute record to describe a
	 * non-resident attribute.
	 */
	a->non_resident = 1;
	/* Move the attribute name if it exists and update the offset. */
	if (a->name_length)
		memmove((u8*)a + name_ofs, (u8*)a + le16_to_cpu(a->name_offset),
				a->name_length * sizeof(ntfschar));
	a->name_offset = cpu_to_le16(name_ofs);
	/* Setup the fields specific to non-resident attributes. */
	a->data.non_resident.lowest_vcn = 0;
	a->data.non_resident.highest_vcn = cpu_to_sle64((new_size - 1) >>
			vol->cluster_size_bits);
	a->data.non_resident.mapping_pairs_offset = cpu_to_le16(mp_ofs);
	memset(&a->data.non_resident.reserved, 0,
			sizeof(a->data.non_resident.reserved));
	a->data.non_resident.allocated_size = cpu_to_sle64(new_size);
	a->data.non_resident.data_size =
			a->data.non_resident.initialized_size =
			cpu_to_sle64(attr_size);
	if (NInoSparse(ni) || NInoCompressed(ni)) {
		a->data.non_resident.compression_unit = 0;
		if (NInoCompressed(ni) || vol->major_ver < 3)
			a->data.non_resident.compression_unit = 4;
		a->data.non_resident.compressed_size =
				a->data.non_resident.allocated_size;
	} else
		a->data.non_resident.compression_unit = 0;
	/* Generate the mapping pairs array into the attribute record. */
	err = ntfs_mapping_pairs_build(vol, (u8*)a + mp_ofs,
			arec_size - mp_ofs, rl, 0, -1, NULL);
	if (unlikely(err)) {
		ntfs_debug("Failed to build mapping pairs, error code %i.",
				err);
		goto undo_err_out;
	}
	/* Setup the in-memory attribute structure to be non-resident. */
	ni->runlist.rl = rl;
	write_lock_irqsave(&ni->size_lock, flags);
	ni->allocated_size = new_size;
	if (NInoSparse(ni) || NInoCompressed(ni)) {
		ni->itype.compressed.size = ni->allocated_size;
		if (a->data.non_resident.compression_unit) {
			ni->itype.compressed.block_size = 1U << (a->data.
					non_resident.compression_unit +
					vol->cluster_size_bits);
			ni->itype.compressed.block_size_bits =
					ffs(ni->itype.compressed.block_size) -
					1;
			ni->itype.compressed.block_clusters = 1U <<
					a->data.non_resident.compression_unit;
		} else {
			ni->itype.compressed.block_size = 0;
			ni->itype.compressed.block_size_bits = 0;
			ni->itype.compressed.block_clusters = 0;
		}
		vi->i_blocks = ni->itype.compressed.size >> 9;
	} else
		vi->i_blocks = ni->allocated_size >> 9;
	write_unlock_irqrestore(&ni->size_lock, flags);
	/*
	 * This needs to be last since the address space operations ->readpage
	 * and ->writepage can run concurrently with us as they are not
	 * serialized on i_mutex.  Note, we are not allowed to fail once we flip
	 * this switch, which is another reason to do this last.
	 */
	NInoSetNonResident(ni);
	/* Mark the mft record dirty, so it gets written back. */
	flush_dcache_mft_record_page(ctx->ntfs_ino);
	mark_mft_record_dirty(ctx->ntfs_ino);
	ntfs_attr_put_search_ctx(ctx);
	unmap_mft_record(base_ni);
	 if(ni->mft_no == 0)
		ntfs_debug("up_write");

	up_write(&ni->runlist.lock);
	if (page) {
		set_page_dirty(page);
		unlock_page(page);
		mark_page_accessed(page);
		page_cache_release(page);
	}
	ntfs_debug("Done.");
	return 0;
undo_err_out:
	/* Convert the attribute back into a resident attribute. */
	a->non_resident = 0;
	/* Move the attribute name if it exists and update the offset. */
	name_ofs = (offsetof(ATTR_RECORD, data.resident.reserved) +
			sizeof(a->data.resident.reserved) + 7) & ~7;
	if (a->name_length)
		memmove((u8*)a + name_ofs, (u8*)a + le16_to_cpu(a->name_offset),
				a->name_length * sizeof(ntfschar));
	mp_ofs = (name_ofs + a->name_length * sizeof(ntfschar) + 7) & ~7;
	a->name_offset = cpu_to_le16(name_ofs);
	arec_size = (mp_ofs + attr_size + 7) & ~7;
	/* Resize the resident part of the attribute record. */
	err2 = ntfs_attr_record_resize(m, a, arec_size);
	if (unlikely(err2)) {
		/*
		 * This cannot happen (well if memory corruption is at work it
		 * could happen in theory), but deal with it as well as we can.
		 * If the old size is too small, truncate the attribute,
		 * otherwise simply give it a larger allocated size.
		 * FIXME: Should check whether chkdsk complains when the
		 * allocated size is much bigger than the resident value size.
		 */
		arec_size = le32_to_cpu(a->length);
		if ((mp_ofs + attr_size) > arec_size) {
			err2 = attr_size;
			attr_size = arec_size - mp_ofs;
			ntfs_error(vol->sb, "Failed to undo partial resident "
					"to non-resident attribute "
					"conversion.  Truncating inode 0x%lx, "
					"attribute type 0x%x from %i bytes to "
					"%i bytes to maintain metadata "
					"consistency.  THIS MEANS YOU ARE "
					"LOSING %i BYTES DATA FROM THIS %s.",
					vi->i_ino,
					(unsigned)le32_to_cpu(type),
					err2, attr_size, err2 - attr_size,
					((type == AT_DATA) &&
					!name_len) ? "FILE": "ATTRIBUTE");
			write_lock_irqsave(&ni->size_lock, flags);
			ni->initialized_size = attr_size;
			i_size_write(vi, attr_size);
			write_unlock_irqrestore(&ni->size_lock, flags);
		}
	}
	/* Setup the fields specific to resident attributes. */
	a->data.resident.value_length = cpu_to_le32(attr_size);
	a->data.resident.value_offset = cpu_to_le16(mp_ofs);
	a->data.resident.flags = old_res_attr_flags;
	memset(&a->data.resident.reserved, 0,
			sizeof(a->data.resident.reserved));
	/* Copy the data from the page back to the attribute value. */
	if (page) {
		kaddr = kmap_atomic(page, KM_USER0);
		memcpy((u8*)a + mp_ofs, kaddr, attr_size);
		kunmap_atomic(kaddr, KM_USER0);
	}
	/* Setup the allocated size in the ntfs inode in case it changed. */
	write_lock_irqsave(&ni->size_lock, flags);
	ni->allocated_size = arec_size - mp_ofs;
	write_unlock_irqrestore(&ni->size_lock, flags);
	/* Mark the mft record dirty, so it gets written back. */
	flush_dcache_mft_record_page(ctx->ntfs_ino);
	mark_mft_record_dirty(ctx->ntfs_ino);
err_out:
	if (ctx)
		ntfs_attr_put_search_ctx(ctx);
	if (m)
		unmap_mft_record(base_ni);
	ni->runlist.rl = NULL;
	 if(ni->mft_no == 0)
		ntfs_debug("up_write");

	up_write(&ni->runlist.lock);
rl_err_out:
	if (rl) {
		if (ntfs_cluster_free_from_rl(vol, rl) < 0) {
			ntfs_error(vol->sb, "Failed to release allocated "
					"cluster(s) in error code path.  Run "
					"chkdsk to recover the lost "
					"cluster(s).");
			NVolSetErrors(vol);
		}
		ntfs_free(rl);
page_err_out:
		unlock_page(page);
		page_cache_release(page);
	}
	if (err == -EINVAL)
		err = -EIO;
	return err;
}
/**扩展@ni属性的分配空间为 @new_alloc_size
 * ntfs_attr_extend_allocation - extend the allocated space of an attribute
 * @ni:			ntfs inode of the attribute whose allocation to extend
 * @new_alloc_size:	new size in bytes to which to extend the allocation to
 * @new_data_size:	new size in bytes to which to extend the data to
 * @data_start:		beginning of region which is required to be non-sparse
 *
 * Extend the allocated space of an attribute described by the ntfs inode @ni
 * to @new_alloc_size bytes.  If @data_start is -1, the whole extension may be
 * implemented as a hole in the file (as long as both the volume and the ntfs
 * inode @ni have sparse support enabled).  If @data_start is >= 0, then the
 * region between the old allocated size and @data_start - 1 may be made sparse
 * but the regions between @data_start and @new_alloc_size must be backed by
 * actual clusters.
 当@data_start=-1时，整个扩展可能是形成文件洞，当然需要volume和
 ntfs inode支持稀疏文件；
 当@data_start is >= 0，老分配大小和 @data_start - 1之间的区间可以为稀疏的，
 但是@data_start and @new_alloc_size之间的区间一定是实际的簇。
 *
 * If @new_data_size is -1, it is ignored.  If it is >= 0, then the data size
 * of the attribute is extended to @new_data_size.  Note that the i_size of the
 * vfs inode is not updated.  Only the data size in the base attribute record
 * is updated.  The caller has to update i_size separately if this is required.
 * WARNING: It is a BUG() for @new_data_size to be smaller than the old data
 * size as well as for @new_data_size to be greater than @new_alloc_size.
 *如果@new_data_size = -1，忽略它；反之，属性的数据大小被扩展为@new_data_size。
 注意:只有主属性记录的数据大小被更新，vfs inode的i_size并没有被更新。
 		如果需要，caller必须自己更新i_size。
 bug: @new_data_size 小于old data size 或者 @new_data_size 大于 @new_alloc_size.
 
 * For resident attributes this involves resizing the attribute record and if
 * necessary moving it and/or other attributes into extent mft records and/or
 * converting the attribute to a non-resident attribute which in turn involves
 * extending the allocation of a non-resident attribute as described below.
 *对于驻留属性，涉及的操作有:改变属性记录的大小；
 将本身或其他属性添加到extent mft record；将属性转换为非驻留属性，
 这就是变成了扩展非驻留属性分配大小了。
 
 * For non-resident attributes this involves allocating clusters in the data
 * zone on the volume (except for regions that are being made sparse) and
 * extending the run list to describe the allocated clusters as well as
 * updating the mapping pairs array of the attribute.  This in turn involves
 * resizing the attribute record and if necessary moving it and/or other
 * attributes into extent mft records and/or splitting the attribute record
 * into multiple extent attribute records.
 *对于非驻留属性，涉及的操作有:在volume上的data zone(除开稀疏区间)分配簇；
 扩展runlist；更新属性的mapping pairs array；这些操作都会导致:改变属性记录大小，
 将其本身或其他属性添加到extent mft record；将属性记录分割添加到不同的扩展属性记录。
 
 * Also, the attribute list attribute is updated if present and in some of the
 * above cases (the ones where extent mft records/attributes come into play),
 * an attribute list attribute is created if not already present.
 如果存在属性列表，在上面的情况下都将更新属性列表属性；
 如果不存在属性列表，将会创建；
 
 * Return the new allocated size on success and -errno on error.  In the case
 * that an error is encountered but a partial extension at least up to
 * @data_start (if present) is possible, the allocation is partially extended
 * and this is returned.  This means the caller must check the returned size to
 * determine if the extension was partial.  If @data_start is -1 then partial
 * allocations are not performed.
 *成功时返回新的分配大小；反之返回-errno；
 当@data_start 不等于 -1时，可能扩展部分，此时返回的新分配大小<@new_alloc_size
 *
 WARNING: Do not call ntfs_attr_extend_allocation() for $MFT/$DATA.
 *注意；对于系统文件$MFT/$DATA不能调用该函数
 
 * Locking: This function takes the runlist lock of @ni for writing as well as
 * locking the mft record of the base ntfs inode.  These locks are maintained
 * throughout execution of the function.  These locks are required so that the
 * attribute can be resized safely and so that it can for example be converted
 * from resident to non-resident safely.
 锁:该函数执行期间，持有 @ni 的runlist锁和mft record锁，
 这使得操作变得安全。
 *
 * TODO: At present attribute list attribute handling is not implemented.
 *目前$attribute list属性还不支持；
 
 * TODO: At present it is not safe to call this function for anything other
 * than the $DATA attribute(s) of an uncompressed and unencrypted file.
 目前，不支持没解压或没解密的数据属扩展

 my views: 1. 该函数仅用来扩展属性的分配空间，不能用来缩小..
 		  2.不能扩展不存在的属性
 */
s64 ntfs_attr_extend_allocation(ntfs_inode *ni, s64 new_alloc_size,
		const s64 new_data_size, const s64 data_start)
{
	VCN vcn;
	s64 ll, allocated_size, start = data_start;
	struct inode *vi = VFS_I(ni);
	ntfs_volume *vol = ni->vol;
	ntfs_inode *base_ni;
	MFT_RECORD *m;
	ATTR_RECORD *a;
	ntfs_attr_search_ctx *ctx;
	runlist_element *rl, *rl2;
	unsigned long flags;
	int err, mp_size;
	u32 attr_len = 0; /* Silence stupid gcc warning. */
	bool mp_rebuilt;

#ifdef DEBUG
	read_lock_irqsave(&ni->size_lock, flags);
	allocated_size = ni->allocated_size;
	read_unlock_irqrestore(&ni->size_lock, flags);
	ntfs_trace("Entering for i_ino 0x%lx, attribute type 0x%x, "
			"old_allocated_size 0x%llx, "
			"new_allocated_size 0x%llx, new_data_size 0x%llx, "
			"data_start 0x%llx.", vi->i_ino,
			(unsigned)le32_to_cpu(ni->type),
			(unsigned long long)allocated_size,
			(unsigned long long)new_alloc_size,
			(unsigned long long)new_data_size,
			(unsigned long long)start);

#endif
retry_extend:
	/*
	 * For non-resident attributes, @start and @new_size need to be aligned
	 * to cluster boundaries for allocation purposes.
	 */
	if (NInoNonResident(ni)) {//必须以簇大小对齐
		if (start > 0)	
			start &= ~(s64)vol->cluster_size_mask;
		new_alloc_size = (new_alloc_size + vol->cluster_size - 1) &
				~(s64)vol->cluster_size_mask;
	}
	DEBUG0_ON(new_data_size >= 0 && new_data_size > new_alloc_size);	//bug:新数据大小大于新分配大小
	/* Check if new size is allowed in $AttrDef. */
	err = ntfs_attr_size_bounds_check(vol, ni->type, new_alloc_size);
	if (unlikely(err)) {
		/* Only emit errors when the write will fail completely. */
		read_lock_irqsave(&ni->size_lock, flags);
		allocated_size = ni->allocated_size;
		read_unlock_irqrestore(&ni->size_lock, flags);
		if (start < 0 || start >= allocated_size) {
			if (err == -ERANGE) {
				ntfs_error(vol->sb, "Cannot extend allocation "
						"of inode 0x%lx, attribute "
						"type 0x%x, because the new "
						"allocation would exceed the "
						"maximum allowed size for "
						"this attribute type.",
						vi->i_ino, (unsigned)
						le32_to_cpu(ni->type));
			} else {
				ntfs_error(vol->sb, "Cannot extend allocation "
						"of inode 0x%lx, attribute "
						"type 0x%x, because this "
						"attribute type is not "
						"defined on the NTFS volume.  "
						"Possible corruption!  You "
						"should run chkdsk!",
						vi->i_ino, (unsigned)
						le32_to_cpu(ni->type));
			}
		}
		/* Translate error code to be POSIX conformant for write(2). */
		if (err == -ERANGE)
			err = -EFBIG;
		else
			err = -EIO;
		return err;
	}
	if (!NInoAttr(ni))	
		base_ni = ni;	//主节点
	else
		base_ni = ni->ext.base_ntfs_ino;	//次节点的主节点
	/*
	 * We will be modifying both the runlist (if non-resident) and the mft
	 * record so lock them both down.
	 */
	if(ni->mft_no == 0)
		 ntfs_debug("down_write");
	ntfs_debug(" ");
	down_write(&ni->runlist.lock);	//给runlist上锁
	m = map_mft_record(base_ni);
	if (IS_ERR(m)) {
		err = PTR_ERR(m);
		m = NULL;
		ctx = NULL;
		goto err_out;
	}
	ctx = ntfs_attr_get_search_ctx(base_ni, m);
	if (unlikely(!ctx)) {
		err = -ENOMEM;
		goto err_out;
	}
	read_lock_irqsave(&ni->size_lock, flags);	//每次读取ni参数时都要先获取读锁
	allocated_size = ni->allocated_size;
	read_unlock_irqrestore(&ni->size_lock, flags);//释放读锁
	/*
	 * If non-resident, seek to the last extent.  If resident, there is
	 * only one extent, so seek to that.
	 */
	vcn = NInoNonResident(ni) ? allocated_size >> vol->cluster_size_bits ://最后一个VCN
			0;
	ntfs_debug("vcn : 0x%llx",vcn);
	/*
	 * Abort if someone did the work whilst we waited for the locks.  If we
	 * just converted the attribute from resident to non-resident it is
	 * likely that exactly this has happened already.  We cannot quite
	 * abort if we need to update the data size.
	 */
	if (unlikely(new_alloc_size <= allocated_size)) {// 如果有人已经把分配大小扩展了
		ntfs_debug("Allocated size already exceeds requested size.");
		new_alloc_size = allocated_size;
		if (new_data_size < 0)//且不需要更新data size-->退出
			goto done;
		/*
		 * We want the first attribute extent so that we can update the
		 * data size.
		 */
		vcn = 0;
	}
	err = ntfs_attr_lookup(ni->type, ni->name, ni->name_len,
			CASE_SENSITIVE, vcn, NULL, 0, ctx);
	if (unlikely(err)) {//也就说该属性一定要事先存在，不能creat
		if (err == -ENOENT)
			err = -EIO;
		goto err_out;
	}
	m = ctx->mrec;
	a = ctx->attr;
	/* Use goto to reduce indentation. */
	ntfs_debug("a->non_resident  = %d",a->non_resident);
	if(NInoNonResident(ni))
		ntfs_trace("(NInoNonResident !");
	if (a->non_resident)
		goto do_non_resident_extend;

	/**扩展驻留属性*/
	DEBUG0_ON(NInoNonResident(ni));
	/* The total length of the attribute value. */
	attr_len = le32_to_cpu(a->data.resident.value_length);	//属性值长度
	/*
	 * Extend the attribute record to be able to store the new attribute
	 * size.  ntfs_attr_record_resize() will not do anything if the size is
	 * not changing.
	 先扩展属性记录的大小，直到可以存储新的属性大小
	 */
	if (new_alloc_size < vol->mft_record_size &&
			!ntfs_attr_record_resize(m, a,	
			le16_to_cpu(a->data.resident.value_offset) +
			new_alloc_size)) {
		/* The resize succeeded! */
		write_lock_irqsave(&ni->size_lock, flags);		//先获取写锁
		ni->allocated_size = le32_to_cpu(a->length) -	//更新分配大小
				le16_to_cpu(a->data.resident.value_offset);
		write_unlock_irqrestore(&ni->size_lock, flags);	//释放写锁
		if (new_data_size >= 0) {
			DEBUG0_ON(new_data_size < attr_len);
			a->data.resident.value_length =
					cpu_to_le32((u32)new_data_size);
		}
		goto flush_done;
	}
	/*
	 * We have to drop all the locks so we can call
	 * ntfs_attr_make_non_resident().  This could be optimised by try-
	 * locking the first page cache page and only if that fails dropping
	 * the locks, locking the page, and redoing all the locking and
	 * lookups.  While this would be a huge optimisation, it is not worth
	 * it as this is definitely a slow code path.
	 */
	 /**在ntfs_attr_make_non_resident之前要释放所有的锁*/
	ntfs_attr_put_search_ctx(ctx);
	unmap_mft_record(base_ni);
	 if(ni->mft_no == 0)
		ntfs_debug("up_write");

	ntfs_debug(" ");
	up_write(&ni->runlist.lock);
	/*
	 * Not enough space in the mft record, try to make the attribute
	 * non-resident and if successful restart the extension process.
	 */
	err = ntfs_attr_make_non_resident(ni, attr_len);	//mft record的空间不足，将属性转为非驻留
	if (likely(!err))
		goto retry_extend;
	/*
	 * Could not make non-resident.  If this is due to this not being
	 * permitted for this attribute type or there not being enough space,
	 * try to make other attributes non-resident.  Otherwise fail.
	 */
	if (unlikely(err != -EPERM && err != -ENOSPC)) {	//转换失败
		/* Only emit errors when the write will fail completely. */
		read_lock_irqsave(&ni->size_lock, flags);
		allocated_size = ni->allocated_size;
		read_unlock_irqrestore(&ni->size_lock, flags);
		if (start < 0 || start >= allocated_size)
			ntfs_error(vol->sb, "Cannot extend allocation of "
					"inode 0x%lx, attribute type 0x%x, "
					"because the conversion from resident "
					"to non-resident attribute failed "
					"with error code %i.", vi->i_ino,
					(unsigned)le32_to_cpu(ni->type), err);
		if (err != -ENOMEM)
			err = -EIO;
		goto conv_err_out;
	}

	/* TODO: Not implemented from here, abort. */
	read_lock_irqsave(&ni->size_lock, flags);
	allocated_size = ni->allocated_size;
	read_unlock_irqrestore(&ni->size_lock, flags);
	if (start < 0 || start >= allocated_size) {
		if (err == -ENOSPC)			//空间不足
			ntfs_error(vol->sb, "Not enough space in the mft "
					"record/on disk for the non-resident "
					"attribute value.  This case is not "
					"implemented yet.");
		else /* if (err == -EPERM) */		//属性不能转换为非驻留
			ntfs_error(vol->sb, "This attribute type may not be "
					"non-resident.  This case is not "
					"implemented yet.");
	}
	err = -EOPNOTSUPP;
	goto conv_err_out;
#if 0
	// TODO: Attempt to make other attributes non-resident.
	if (!err)
		goto do_resident_extend;
	/*
	 * Both the attribute list attribute and the standard information
	 * attribute must remain in the base inode.  Thus, if this is one of
	 * these attributes, we have to try to move other attributes out into
	 * extent mft records instead.
	 */
	if (ni->type == AT_ATTRIBUTE_LIST ||
			ni->type == AT_STANDARD_INFORMATION) {
		// TODO: Attempt to move other attributes into extent mft
		// records.
		err = -EOPNOTSUPP;
		if (!err)
			goto do_resident_extend;
		goto err_out;
	}
	// TODO: Attempt to move this attribute to an extent mft record, but
	// only if it is not already the only attribute in an mft record in
	// which case there would be nothing to gain.
	err = -EOPNOTSUPP;
	if (!err)
		goto do_resident_extend;
	/* There is nothing we can do to make enough space. )-: */
	goto err_out;
#endif

/**扩展非驻留属性*/
do_non_resident_extend:
	//DEBUG0_ON(!NInoNonResident(ni));
	if (new_alloc_size == allocated_size) {
		DEBUG0_ON(vcn);
		goto alloc_done;
	}
	/*
	 * If the data starts after the end of the old allocation, this is a
	 * $DATA attribute and sparse attributes are enabled on the volume and
	 * for this inode, then create a sparse region between the old
	 * allocated size and the start of the data.  Otherwise simply proceed
	 * with filling the whole space between the old allocated size and the
	 * new allocated size with clusters.
	 */
	if ((start >= 0 && start <= allocated_size) || ni->type != AT_DATA ||
			!NVolSparseEnabled(vol) || NInoSparseDisabled(ni))
		goto skip_sparse;
	// TODO: This is not implemented yet.  We just fill in with real
	// clusters for now...
	ntfs_debug("Inserting holes is not-implemented yet.  Falling back to "
			"allocating real clusters instead.");
skip_sparse:

	rl = ni->runlist.rl;
	if (likely(rl)) {
		/* Seek to the end of the runlist. */		
		while (rl->length)		//rl: runlist的末尾
			rl++;

	}
	
	/* If this attribute extent is not mapped, map it now. */
	if (unlikely(!rl || rl->lcn == LCN_RL_NOT_MAPPED ||
			(rl->lcn == LCN_ENOENT && rl > ni->runlist.rl &&
			(rl-1)->lcn == LCN_RL_NOT_MAPPED))) {
		if (!rl && !allocated_size)
			goto first_alloc;
		rl = ntfs_mapping_pairs_decompress(vol, a, ni->runlist.rl);
		if (IS_ERR(rl)) {
			err = PTR_ERR(rl);
			if (start < 0 || start >= allocated_size)
				ntfs_error(vol->sb, "Cannot extend allocation "
						"of inode 0x%lx, attribute "
						"type 0x%x, because the "
						"mapping of a runlist "
						"fragment failed with error "
						"code %i.", vi->i_ino,
						(unsigned)le32_to_cpu(ni->type),
						err);
			if (err != -ENOMEM)
				err = -EIO;
			goto err_out;
		}
		ni->runlist.rl = rl;		
		/* Seek to the end of the runlist. */
		while (rl->length){
			ntfs_debug("rl->length : %lld",rl->length);
			rl++;
		}
	}

	/*
	 * We now know the runlist of the last extent is mapped and @rl is at
	 * the end of the runlist.  We want to begin allocating clusters
	 * starting at the last allocated cluster to reduce fragmentation.  If
	 * there are no valid LCNs in the attribute we let the cluster
	 * allocator choose the starting cluster.
	 好了，我们已经将last extent的runlist 映射了，
	 @ri也位于runlist的末尾。现在需要分配簇了。
	 */
	/* If the last LCN is a hole or simillar seek back tolast  real LCN. */
	while (rl->lcn < 0 && rl > ni->runlist.rl)
		rl--;

first_alloc:
	// FIXME: Need to implement partial allocations so at least part of the
	// write can be performed when start >= 0.  (Needed for POSIX write(2)
	// conformance.)
	ntfs_trace("allocated_size :%lld,new_alloc_size :%lld",allocated_size,new_alloc_size);
	rl2 = ntfs_cluster_alloc(vol, allocated_size >> vol->cluster_size_bits,	//根据new_alloc_size从runlist的末尾分配新的runlist
			(new_alloc_size - allocated_size) >>
			vol->cluster_size_bits, (rl && (rl->lcn >= 0)) ?
			rl->lcn + rl->length : -1, DATA_ZONE, true);
	if (IS_ERR(rl2)) {
		err = PTR_ERR(rl2);
		if (start < 0 || start >= allocated_size)
			ntfs_error(vol->sb, "Cannot extend allocation of "
					"inode 0x%lx, attribute type 0x%x, "
					"because the allocation of clusters "
					"failed with error code %i.", vi->i_ino,
					(unsigned)le32_to_cpu(ni->type), err);
		if (err != -ENOMEM && err != -ENOSPC)
			err = -EIO;
		goto err_out;
	}
	rl = ntfs_runlists_merge(ni->runlist.rl, rl2);	//将新生成的rl2合并到ni->runlist.rl
	if (IS_ERR(rl)) {
		err = PTR_ERR(rl);
		if (start < 0 || start >= allocated_size)
			ntfs_error(vol->sb, "Cannot extend allocation of "
					"inode 0x%lx, attribute type 0x%x, "
					"because the runlist merge failed "
					"with error code %i.", vi->i_ino,
					(unsigned)le32_to_cpu(ni->type), err);
		if (err != -ENOMEM)
			err = -EIO;
		if (ntfs_cluster_free_from_rl(vol, rl2)) {
			ntfs_error(vol->sb, "Failed to release allocated "
					"cluster(s) in error code path.  Run "
					"chkdsk to recover the lost "
					"cluster(s).");
			NVolSetErrors(vol);
		}
		ntfs_free(rl2);
		goto err_out;
	}
	ni->runlist.rl = rl;
	ntfs_trace("Allocated 0x%llx clusters.", (long long)(new_alloc_size -
			allocated_size) >> vol->cluster_size_bits);
	/* Find the runlist element with which the attribute extent starts. */
	ll = sle64_to_cpu(a->data.non_resident.lowest_vcn);
	rl2 = ntfs_rl_find_vcn_nolock(rl, ll);
	DEBUG0_ON(!rl2);
	DEBUG0_ON(!rl2->length);
	DEBUG0_ON(rl2->lcn < LCN_HOLE);
	mp_rebuilt = false;
	/* Get the size for the new mapping pairs array for this extent. */
	mp_size = ntfs_get_size_for_mapping_pairs(vol, rl2, ll, -1);
	if (unlikely(mp_size <= 0)) {
		err = mp_size;
		if (start < 0 || start >= allocated_size)
			ntfs_error(vol->sb, "Cannot extend allocation of "
					"inode 0x%lx, attribute type 0x%x, "
					"because determining the size for the "
					"mapping pairs failed with error code "
					"%i.", vi->i_ino,
					(unsigned)le32_to_cpu(ni->type), err);
		err = -EIO;
		goto undo_alloc;
	}
	/* Extend the attribute record to fit the bigger mapping pairs array. */
	/*改变属性记录的大小以存储新的mapping pairs array*/
	attr_len = le32_to_cpu(a->length);
	err = ntfs_attr_record_resize(m, a, mp_size +
			le16_to_cpu(a->data.non_resident.mapping_pairs_offset));
	if (unlikely(err)) {
		DEBUG0_ON(err != -ENOSPC);
		// TODO: Deal with this by moving this extent to a new mft
		// record or by starting a new extent in a new mft record,
		// possibly by extending this extent partially and filling it
		// and creating a new extent for the remainder, or by making
		// other attributes non-resident and/or by moving other
		// attributes out of this mft record.
		if (start < 0 || start >= allocated_size)
			ntfs_error(vol->sb, "Not enough space in the mft "
					"record for the extended attribute "
					"record.  This case is not "
					"implemented yet.");
		err = -EOPNOTSUPP;
		goto undo_alloc;
	}
	mp_rebuilt = true;
	/* Generate the mapping pairs array directly into the attr record. */
	err = ntfs_mapping_pairs_build(vol, (u8*)a +
			le16_to_cpu(a->data.non_resident.mapping_pairs_offset),
			mp_size, rl2, ll, -1, NULL);
	if (unlikely(err)) {
		if (start < 0 || start >= allocated_size)
			ntfs_error(vol->sb, "Cannot extend allocation of "
					"inode 0x%lx, attribute type 0x%x, "
					"because building the mapping pairs "
					"failed with error code %i.", vi->i_ino,
					(unsigned)le32_to_cpu(ni->type), err);
		err = -EIO;
		goto undo_alloc;
	}
	/* Update the highest_vcn. */
	a->data.non_resident.highest_vcn = cpu_to_sle64((new_alloc_size >>
			vol->cluster_size_bits) - 1);
	/*
	 * We now have extended the allocated size of the attribute.  Reflect
	 * this in the ntfs_inode structure and the attribute record.
	 */
	if (a->data.non_resident.lowest_vcn) {
		/*
		 * We are not in the first attribute extent, switch to it, but
		 * first ensure the changes will make it to disk later.
		 */
		flush_dcache_mft_record_page(ctx->ntfs_ino);
		mark_mft_record_dirty(ctx->ntfs_ino);
		ntfs_attr_reinit_search_ctx(ctx);
		err = ntfs_attr_lookup(ni->type, ni->name, ni->name_len,
				CASE_SENSITIVE, 0, NULL, 0, ctx);
		if (unlikely(err))
			goto restore_undo_alloc;
		/* @m is not used any more so no need to set it. */
		a = ctx->attr;
	}
	write_lock_irqsave(&ni->size_lock, flags);
	ni->allocated_size = new_alloc_size;
	a->data.non_resident.allocated_size = cpu_to_sle64(new_alloc_size);
	/*
	 * FIXME: This would fail if @ni is a directory, $MFT, or an index,
	 * since those can have sparse/compressed set.  For example can be
	 * set compressed even though it is not compressed itself and in that
	 * case the bit means that files are to be created compressed in the
	 * directory...  At present this is ok as this code is only called for
	 * regular files, and only for their $DATA attribute(s).
	 * FIXME: The calculation is wrong if we created a hole above.  For now
	 * it does not matter as we never create holes.
	 */
	if (NInoSparse(ni) || NInoCompressed(ni)) {
		ni->itype.compressed.size += new_alloc_size - allocated_size;
		a->data.non_resident.compressed_size =
				cpu_to_sle64(ni->itype.compressed.size);
		vi->i_blocks = ni->itype.compressed.size >> 9;
	} else
		vi->i_blocks = new_alloc_size >> 9;
	write_unlock_irqrestore(&ni->size_lock, flags);
alloc_done:
	if (new_data_size >= 0) {
		DEBUG0_ON(new_data_size <
				sle64_to_cpu(a->data.non_resident.data_size));
		a->data.non_resident.data_size = cpu_to_sle64(new_data_size);
	}
flush_done:
	/* Ensure the changes make it to disk. */
	flush_dcache_mft_record_page(ctx->ntfs_ino);	//确保改变写入磁盘
	mark_mft_record_dirty(ctx->ntfs_ino);
done:
	ntfs_attr_put_search_ctx(ctx);
	unmap_mft_record(base_ni);
	 if(ni->mft_no == 0)
		ntfs_debug("up_write");

	up_write(&ni->runlist.lock);
	ntfs_trace("Done, new_allocated_size 0x%llx.",
			(unsigned long long)new_alloc_size);
	return new_alloc_size;
restore_undo_alloc:
	if (start < 0 || start >= allocated_size)
		ntfs_error(vol->sb, "Cannot complete extension of allocation "
				"of inode 0x%lx, attribute type 0x%x, because "
				"lookup of first attribute extent failed with "
				"error code %i.", vi->i_ino,
				(unsigned)le32_to_cpu(ni->type), err);
	if (err == -ENOENT)
		err = -EIO;
	ntfs_attr_reinit_search_ctx(ctx);
	if (ntfs_attr_lookup(ni->type, ni->name, ni->name_len, CASE_SENSITIVE,
			allocated_size >> vol->cluster_size_bits, NULL, 0,
			ctx)) {
		ntfs_error(vol->sb, "Failed to find last attribute extent of "
				"attribute in error code path.  Run chkdsk to "
				"recover.");
		write_lock_irqsave(&ni->size_lock, flags);
		ni->allocated_size = new_alloc_size;
		/*
		 * FIXME: This would fail if @ni is a directory...  See above.
		 * FIXME: The calculation is wrong if we created a hole above.
		 * For now it does not matter as we never create holes.
		 */
		if (NInoSparse(ni) || NInoCompressed(ni)) {
			ni->itype.compressed.size += new_alloc_size -
					allocated_size;
			vi->i_blocks = ni->itype.compressed.size >> 9;
		} else
			vi->i_blocks = new_alloc_size >> 9;
		write_unlock_irqrestore(&ni->size_lock, flags);
		ntfs_attr_put_search_ctx(ctx);
		unmap_mft_record(base_ni);
		 if(ni->mft_no == 0)
			ntfs_debug("up_write");

		up_write(&ni->runlist.lock);
		/*
		 * The only thing that is now wrong is the allocated size of the
		 * base attribute extent which chkdsk should be able to fix.
		 */
		NVolSetErrors(vol);
		return err;
	}
	ctx->attr->data.non_resident.highest_vcn = cpu_to_sle64(
			(allocated_size >> vol->cluster_size_bits) - 1);
undo_alloc:
	ll = allocated_size >> vol->cluster_size_bits;
	if (ntfs_cluster_free(ni, ll, -1, ctx) < 0) {
		ntfs_error(vol->sb, "Failed to release allocated cluster(s) "
				"in error code path.  Run chkdsk to recover "
				"the lost cluster(s).");
		NVolSetErrors(vol);
	}
	m = ctx->mrec;
	a = ctx->attr;
	/*
	 * If the runlist truncation fails and/or the search context is no
	 * longer valid, we cannot resize the attribute record or build the
	 * mapping pairs array thus we mark the inode bad so that no access to
	 * the freed clusters can happen.
	 */
	if (ntfs_rl_truncate_nolock(vol, &ni->runlist, ll) || IS_ERR(m)) {
		ntfs_error(vol->sb, "Failed to %s in error code path.  Run "
				"chkdsk to recover.", IS_ERR(m) ?
				"restore attribute search context" :
				"truncate attribute runlist");
		NVolSetErrors(vol);
	} else if (mp_rebuilt) {
		if (ntfs_attr_record_resize(m, a, attr_len)) {
			ntfs_error(vol->sb, "Failed to restore attribute "
					"record in error code path.  Run "
					"chkdsk to recover.");
			NVolSetErrors(vol);
		} else /* if (success) */ {
			if (ntfs_mapping_pairs_build(vol, (u8*)a + le16_to_cpu(
					a->data.non_resident.
					mapping_pairs_offset), attr_len -
					le16_to_cpu(a->data.non_resident.
					mapping_pairs_offset), rl2, ll, -1,
					NULL)) {
				ntfs_error(vol->sb, "Failed to restore "
						"mapping pairs array in error "
						"code path.  Run chkdsk to "
						"recover.");
				NVolSetErrors(vol);
			}
			flush_dcache_mft_record_page(ctx->ntfs_ino);
			mark_mft_record_dirty(ctx->ntfs_ino);
		}
	}
err_out:
	if (ctx)
		ntfs_attr_put_search_ctx(ctx);
	if (m)
		unmap_mft_record(base_ni);
	 if(ni->mft_no == 0)
		ntfs_debug("up_write");

	up_write(&ni->runlist.lock);
conv_err_out:
	ntfs_debug("Failed.  Returning error code %i.", err);
	return err;
}

/**填充属性
 * ntfs_attr_set - fill (a part of) an attribute with a byte
 * @ni:		ntfs inode describing the attribute to fill
 * @ofs:	offset inside the attribute at which to start to fill
 * @cnt:	number of bytes to fill
 * @val:	the unsigned 8-bit value with which to fill the attribute
 *
 * Fill @cnt bytes of the attribute described by the ntfs inode @ni starting at
 * byte offset @ofs inside the attribute with the constant byte @val.
 *
 * This function is effectively like memset() applied to an ntfs attribute.
 * Note thie function actually only operates on the page cache pages belonging
 * to the ntfs attribute and it marks them dirty after doing the memset().
 * Thus it relies on the vm dirty page write code paths to cause the modified
 * pages to be written to the mft record/disk.
 *
 * Return 0 on success and -errno on error.  An error code of -ESPIPE means
 * that @ofs + @cnt were outside the end of the attribute and no write was
 * performed.
 */
int ntfs_attr_set(ntfs_inode *ni, const s64 ofs, const s64 cnt, const u8 val)
{
	ntfs_volume *vol = ni->vol;
	struct address_space *mapping;
	struct page *page;
	u8 *kaddr;
	pgoff_t idx, end;
	unsigned start_ofs, end_ofs, size;

	ntfs_debug("Entering for ofs 0x%llx, cnt 0x%llx, val 0x%hx.",
			(long long)ofs, (long long)cnt, val);
	DEBUG0_ON(ofs < 0);
	DEBUG0_ON(cnt < 0);
	if (!cnt)
		goto done;
	/*
	 * FIXME: Compressed and encrypted attributes are not supported when
	 * writing and we should never have gotten here for them.
	 */
	DEBUG0_ON(NInoCompressed(ni));
	DEBUG0_ON(NInoEncrypted(ni));
	mapping = VFS_I(ni)->i_mapping;
	/* Work out the starting index and page offset. */
	idx = ofs >> PAGE_CACHE_SHIFT;
	start_ofs = ofs & ~PAGE_CACHE_MASK;
	/* Work out the ending index and page offset. */
	end = ofs + cnt;
	end_ofs = end & ~PAGE_CACHE_MASK;
	/* If the end is outside the inode size return -ESPIPE. */
	if (unlikely(end > i_size_read(VFS_I(ni)))) {
		ntfs_error(vol->sb, "Request exceeds end of attribute.");
		return -ESPIPE;
	}
	end >>= PAGE_CACHE_SHIFT;
	/* If there is a first partial page, need to do it the slow way. */
	if (start_ofs) {
		page = read_mapping_page(mapping, idx, NULL);
		if (IS_ERR(page)) {
			ntfs_error(vol->sb, "Failed to read first partial "
					"page (error, index 0x%lx).", idx);
			return PTR_ERR(page);
		}
		/*
		 * If the last page is the same as the first page, need to
		 * limit the write to the end offset.
		 */
		size = PAGE_CACHE_SIZE;
		if (idx == end)
			size = end_ofs;
		kaddr = kmap_atomic(page, KM_USER0);
		memset(kaddr + start_ofs, val, size - start_ofs);
		flush_dcache_page(page);
		kunmap_atomic(kaddr, KM_USER0);
		set_page_dirty(page);
		page_cache_release(page);
		balance_dirty_pages_ratelimited(mapping);
		cond_resched();
		if (idx == end)
			goto done;
		idx++;
	}
	/* Do the whole pages the fast way. */
	for (; idx < end; idx++) {
		/* Find or create the current page.  (The page is locked.) */
		page = grab_cache_page(mapping, idx);
		if (unlikely(!page)) {
			ntfs_error(vol->sb, "Insufficient memory to grab "
					"page (index 0x%lx).", idx);
			return -ENOMEM;
		}
		kaddr = kmap_atomic(page, KM_USER0);
		memset(kaddr, val, PAGE_CACHE_SIZE);
		flush_dcache_page(page);
		kunmap_atomic(kaddr, KM_USER0);
		/*
		 * If the page has buffers, mark them uptodate since buffer
		 * state and not page state is definitive in 2.6 kernels.
		 */
		if (page_has_buffers(page)) {
			struct buffer_head *bh, *head;

			bh = head = page_buffers(page);
			do {
				set_buffer_uptodate(bh);
			} while ((bh = bh->b_this_page) != head);
		}
		/* Now that buffers are uptodate, set the page uptodate, too. */
		SetPageUptodate(page);
		/*
		 * Set the page and all its buffers dirty and mark the inode
		 * dirty, too.  The VM will write the page later on.
		 */
		set_page_dirty(page);
		/* Finally unlock and release the page. */
		unlock_page(page);
		page_cache_release(page);
		balance_dirty_pages_ratelimited(mapping);
		cond_resched();
	}
	/* If there is a last partial page, need to do it the slow way. */
	if (end_ofs) {
		page = read_mapping_page(mapping, idx, NULL);
		if (IS_ERR(page)) {
			ntfs_error(vol->sb, "Failed to read last partial page "
					"(error, index 0x%lx).", idx);
			return PTR_ERR(page);
		}
		kaddr = kmap_atomic(page, KM_USER0);
		memset(kaddr, val, end_ofs);
		flush_dcache_page(page);
		kunmap_atomic(kaddr, KM_USER0);
		set_page_dirty(page);
		page_cache_release(page);
		balance_dirty_pages_ratelimited(mapping);
		cond_resched();
	}
done:
	ntfs_debug("Done.");
	return 0;
}

#ifdef  NTFS_MODIFY

ntfschar AT_UNNAMED[] = { const_cpu_to_le16('\0') };
ntfschar STREAM_SDS[] = { const_cpu_to_le16('$'),
			const_cpu_to_le16('S'),
			const_cpu_to_le16('D'),
			const_cpu_to_le16('S'),
			const_cpu_to_le16('\0') };

ntfschar TXF_DATA[] = { const_cpu_to_le16('$'),
			const_cpu_to_le16('T'),
			const_cpu_to_le16('X'),
			const_cpu_to_le16('F'),
			const_cpu_to_le16('_'),
			const_cpu_to_le16('D'),
			const_cpu_to_le16('A'),
			const_cpu_to_le16('T'),
			const_cpu_to_le16('A'),
			const_cpu_to_le16('\0') };

static const char *es = "  Leaving inconsistent metadata.  Unmount and run "
		"chkdsk.";

/* Already cleaned up code below, but still look for FIXME:... */
#if 0

/**用其他参数初始化ntfs 非驻留属性结构na
 * __ntfs_attr_init - primary initialization of an ntfs attribute structure
 * @na:		ntfs attribute to initialize
 * @ni:		ntfs inode with which to initialize the ntfs attribute
 * @type:		attribute type
 * @name:	attribute name in little endian Unicode or NULL
 * @name_len:	length of attribute @name in Unicode characters (if @name given)
 *
 * Initialize the ntfs attribute @na with @ni, @type, @name, and @name_len.
 */
static void __ntfs_attr_init(ntfs_attr_3g *na_3g, ntfs_inode *ni,
		const ATTR_TYPE type, ntfschar *name, const u32 name_len)
{
	na_3g->rl = NULL;
	na_3g->ni = ni;
	na_3g->type = type;
	na_3g->name = name;
	if (name)
		na_3g->name_len = name_len;
	else
		na_3g->name_len = 0;
}

/**
 * ntfs_attr_init - initialize an ntfs_attr with data sizes and status
 * @na:
 * @non_resident:
 * @data_flags: compressed or encrypted,sparse....
 * @allocated_size:
 * @data_size:
 * @initialized_size:
 * @compressed_size:
 * @compression_unit:
 *
 * Final initialization for an ntfs attribute.
 */
void ntfs_attr_init(ntfs_attr_3g *na_3g, const BOOL non_resident,
		const ATTR_FLAGS data_flags,
		const BOOL encrypted, const BOOL sparse,
		const s64 allocated_size, const s64 data_size,
		const s64 initialized_size, const s64 compressed_size,
		const u8 compression_unit)
{
	if (!NAttrInitialized(na_3g)) {
		na_3g->data_flags = data_flags;
		if (non_resident)
			NAttrSetNonResident(na_3g);
		if (data_flags & ATTR_COMPRESSION_MASK)
			NAttrSetCompressed(na_3g);
		if (encrypted)
			NAttrSetEncrypted(na_3g);
		if (sparse)
			NAttrSetSparse(na_3g);
		na_3g->allocated_size = allocated_size;
		na_3g->data_size = data_size;
		na_3g->initialized_size = initialized_size;
		if ((data_flags & ATTR_COMPRESSION_MASK) || sparse) {
			ntfs_volume *vol = na_3g->ni->vol;

			na_3g->compressed_size = compressed_size;
			na_3g->compression_block_clusters = 1 << compression_unit;
			na_3g->compression_block_size = 1 << (compression_unit +
					vol->cluster_size_bits);
			na_3g->compression_block_size_bits = ffs(
					na_3g->compression_block_size) - 1;
		}
		NAttrSetInitialized(na_3g);
	}
}

/**
 * ntfs_attr_open - open an ntfs attribute for access
 * @ni:		open ntfs inode in which the ntfs attribute resides
 * @type:	attribute type
 * @name:	attribute name in little endian Unicode or AT_UNNAMED or NULL
 * @name_len:	length of attribute @name in Unicode characters (if @name given)
 *
 * Allocate a new ntfs attribute structure, initialize it with @ni, @type,
 * @name, and @name_len, then return it. Return NULL on error with
 * errno set to the error code.
 *
 * If @name is AT_UNNAMED look specifically for an unnamed attribute.  If you
 * do not care whether the attribute is named or not set @name to NULL.  In
 * both those cases @name_len is not used at all.
 */
ntfs_attr_3g *ntfs_attr_open(ntfs_inode *ni, const ATTR_TYPE type,//该函数中na_3g->rl没初始化
		ntfschar *name, u32 name_len)
{
	ntfs_attr_search_ctx *ctx;
	ntfs_attr_3g *na_3g = NULL;
	ntfschar *newname = NULL;
	ATTR_RECORD *a;
	MFT_RECORD *m = NULL;
	BOOL cs;

	ntfs_debug("Entering for inode %lld, attr 0x%x.",
		       (unsigned long long)ni->mft_no, type);

	m = map_mft_record(ni);
	if(!m)
		goto out;
	unmap_mft_record(ni);
	
	if (!ni || !ni->vol ) {
		//errno = EINVAL;
		goto out;
	}
	na_3g = kcalloc(1,sizeof(ntfs_attr_3g),GFP_KERNEL);
	if (!na_3g)
		goto out;
	if (name && name != AT_UNNAMED && name != I30) {
		name = ntfs_ucsndup(name, name_len);
		if (!name)
			goto err_out;
		newname = name;
	}

	ctx = ntfs_attr_get_search_ctx(ni, NULL);
	if (!ctx)
		goto err_out;

	if (ntfs_attr_lookup(type, name, name_len, 0, 0, NULL, 0, ctx))
		goto put_err_out;

	a = ctx->attr;
	
	if (!name) {
		if (a->name_length) {
			name = ntfs_ucsndup((ntfschar*)((u8*)a + le16_to_cpu(
					a->name_offset)), a->name_length);
			if (!name)
				goto put_err_out;
			newname = name;
			name_len = a->name_length;
		} else {
			name = AT_UNNAMED;
			name_len = 0;
		}
	}
	
	__ntfs_attr_init(na_3g, ni, type, name, name_len);
	
	/*
	 * Wipe the flags in case they are not zero for an attribute list
	 * attribute.  Windows does not complain about invalid flags and chkdsk
	 * does not detect or fix them so we need to cope with it, too.
	 */
	if (type == AT_ATTRIBUTE_LIST)
		a->flags = 0;

	if ((type == AT_DATA) && !a->data.non_resident.initialized_size) {
		/*
		 * Define/redefine the compression state if stream is
		 * empty, based on the compression mark on parent
		 * directory (for unnamed data streams) or on current
		 * inode (for named data streams). The compression mark
		 * may change any time, the compression state can only
		 * change when stream is wiped out.
		 * 
		 * Also prevent compression on NTFS version < 3.0
		 * or cluster size > 4K or compression is disabled
		 */
		a->flags &= ~ATTR_COMPRESSION_MASK;
		if ((ni->flags & FILE_ATTR_COMPRESSED)
		    && (ni->vol->major_ver >= 3)
		    && NVolCompression(ni->vol)
		    && (ni->vol->cluster_size <= MAX_COMPRESSION_CLUSTER_SIZE))
			a->flags |= ATTR_IS_COMPRESSED;
	}
	
	cs = a->flags & (ATTR_IS_COMPRESSED | ATTR_IS_SPARSE);
	
	if (na_3g->type == AT_DATA && na_3g->name == AT_UNNAMED &&
	    ((!(a->flags & ATTR_IS_SPARSE)     != !NAttrSparse(na_3g)) ||
	     (!(a->flags & ATTR_IS_ENCRYPTED)  != !NAttrEncrypted(na_3g)))) {
		//errno = EIO;
		ntfs_debug("Inode %lld has corrupt attribute flags "
				"(0x%x <> 0x%x)",(unsigned long long)ni->mft_no,
				a->flags, na_3g->ni->flags);
		goto put_err_out;
	}

	if (a->non_resident) {//非驻留
		if ((a->flags & ATTR_COMPRESSION_MASK)
				 && !a->data.non_resident.compression_unit) {
			//errno = EIO;
			ntfs_debug("Compressed inode %lld attr 0x%x has "
					"no compression unit",
					(unsigned long long)ni->mft_no, type);
			goto put_err_out;
		}
		ntfs_attr_init(na_3g, TRUE, a->flags,
				a->flags & ATTR_IS_ENCRYPTED,
				a->flags & ATTR_IS_SPARSE,
				sle64_to_cpu(a->data.non_resident.allocated_size),
				sle64_to_cpu(a->data.non_resident.data_size),
				sle64_to_cpu(a->data.non_resident.initialized_size),
				cs ? sle64_to_cpu(a->data.non_resident.compressed_size) : 0,
				cs ? a->data.non_resident.compression_unit : 0);
	} else {//驻留
		s64 l = le32_to_cpu(a->data.resident.value_length);
		ntfs_attr_init(na_3g, FALSE, a->flags,
				a->flags & ATTR_IS_ENCRYPTED,
				a->flags & ATTR_IS_SPARSE, (l + 7) & ~7, l, l,
				cs ? (l + 7) & ~7 : 0, 0);
	}
	ntfs_attr_put_search_ctx(ctx);
out:
	ntfs_debug("Done.");	
	return na_3g;

put_err_out:
	ntfs_attr_put_search_ctx(ctx);
err_out:
	free(newname);
	free(na_3g);
	na_3g = NULL;
	ntfs_debug("Failed.");
	goto out;
}

/**
 * ntfs_attr_close - free an ntfs no_resident attribute structure
 * @na:		ntfs attribute structure to free
 *
 * Release all memory associated with the ntfs attribute @na and then release
 * @na itself.
 */
void ntfs_attr_close(ntfs_attr_3g *na_3g)
{
	if (!na_3g)
		return;
	if (NAttrNonResident(na_3g) && na_3g->rl)
		free(na_3g->rl);
	/* Don't release if using an internal constant. */
	if (na_3g->name != AT_UNNAMED && na_3g->name != NTFS_INDEX_I30
				&& na_3g->name != STREAM_SDS)
		free(na_3g->name);
	free(na_3g);
}


static int ntfs_resident_attr_resize(ntfs_attr *na, const s64 newsize);

/**
 * ntfs_resident_attr_resize - resize a resident, open ntfs attribute
 * @na:		resident ntfs attribute to resize
 * @newsize:	new size (in bytes) to which to resize the attribute
 *
 * Change the size of a resident, open ntfs attribute @na to @newsize bytes.
 * Can also be used to force an attribute non-resident. In this case, the
 * size cannot be changed.
 *
 * On success return 0 
 * On error return values are:
 * 	STATUS_RESIDENT_ATTRIBUTE_FILLED_MFT
 * 	STATUS_ERROR - otherwise
 * The following error codes are defined:
 *	ENOMEM - Not enough memory to complete operation.
 *	ERANGE - @newsize is not valid for the attribute type of @na.
 *	ENOSPC - There is no enough space in base mft to resize $ATTRIBUTE_LIST.
 */
static int ntfs_resident_attr_resize_i(ntfs_attr *na, const s64 newsize,
			BOOL force_non_resident)
{
	ntfs_attr_search_ctx *ctx;
	ntfs_volume *vol;
	ntfs_inode *ni;
	int err, ret = STATUS_ERROR;

	ntfs_debug("Inode 0x%llx attr 0x%x new size %lld\n", 
		       (unsigned long long)na->ni->mft_no, na->type, 
		       (long long)newsize);

	/* Get the attribute record that needs modification. */
	ctx = ntfs_attr_get_search_ctx(na->ni, NULL);
	if (!ctx)
		return -1;
	if (ntfs_attr_lookup(na->type, na->name, na->name_len, 0, 0, NULL, 0,
			ctx)) {
		err = errno;
		ntfs_log_perror("ntfs_attr_lookup failed");
		goto put_err_out;
	}
	vol = na->ni->vol;
	/*
	 * Check the attribute type and the corresponding minimum and maximum
	 * sizes against @newsize and fail if @newsize is out of bounds.
	 */
	if (ntfs_attr_size_bounds_check(vol, na->type, newsize) < 0) {
		err = errno;
		if (err == ENOENT)
			err = EIO;
		ntfs_log_perror("%s: bounds check failed", __FUNCTION__);
		goto put_err_out;
	}
	/*
	 * If @newsize is bigger than the mft record we need to make the
	 * attribute non-resident if the attribute type supports it. If it is
	 * smaller we can go ahead and attempt the resize.
	 */
	if ((newsize < vol->mft_record_size) && !force_non_resident) {
		/* Perform the resize of the attribute record. */
		if (!(ret = ntfs_resident_attr_value_resize(ctx->mrec, ctx->attr,
				newsize))) {
			/* Update attribute size everywhere. */
			na->data_size = na->initialized_size = newsize;
			na->allocated_size = (newsize + 7) & ~7;
			if ((na->data_flags & ATTR_COMPRESSION_MASK)
			    || NAttrSparse(na))
				na->compressed_size = na->allocated_size;
			if (na->ni->mrec->flags & MFT_RECORD_IS_DIRECTORY
			    ? na->type == AT_INDEX_ROOT && na->name == NTFS_INDEX_I30
			    : na->type == AT_DATA && na->name == AT_UNNAMED) {
				na->ni->data_size = na->data_size;
				if (((na->data_flags & ATTR_COMPRESSION_MASK)
					|| NAttrSparse(na))
						&& NAttrNonResident(na))
					na->ni->allocated_size
						= na->compressed_size;
				else
					na->ni->allocated_size
						= na->allocated_size;
				set_nino_flag(na->ni,KnownSize);
				if (na->type == AT_DATA)
					NInoFileNameSetDirty(na->ni);
			}
			goto resize_done;
		}
		/* Prefer AT_INDEX_ALLOCATION instead of AT_ATTRIBUTE_LIST */
		if (ret == STATUS_RESIDENT_ATTRIBUTE_FILLED_MFT) {
			err = errno;
			goto put_err_out;
		}
	}
	/* There is not enough space in the mft record to perform the resize. */

	/* Make the attribute non-resident if possible. */
	if (!ntfs_attr_make_non_resident(na, ctx)) {
		ntfs_inode_mark_dirty(ctx->ntfs_ino);
		ntfs_attr_put_search_ctx(ctx);
		/*
		 * do not truncate when forcing non-resident, this
		 * could cause the attribute to be made resident again,
		 * so size changes are not allowed.
		 */
		if (force_non_resident) {
			ret = 0;
			if (newsize != na->data_size) {
				ntfs_log_error("Cannot change size when"
					" forcing non-resident\n");
				errno = EIO;
				ret = STATUS_ERROR;
			}
			return (ret);
		}
		/* Resize non-resident attribute */
		return ntfs_attr_truncate(na, newsize);
	} else if (errno != ENOSPC && errno != EPERM) {
		err = errno;
		ntfs_log_perror("Failed to make attribute non-resident");
		goto put_err_out;
	}

	/* Try to make other attributes non-resident and retry each time. */
	ntfs_attr_init_search_ctx(ctx, NULL, na->ni->mrec);
	while (!ntfs_attr_lookup(AT_UNUSED, NULL, 0, 0, 0, NULL, 0, ctx)) {
		ntfs_attr *tna;
		ATTR_RECORD *a;

		a = ctx->attr;
		if (a->non_resident)
			continue;

		/*
		 * Check out whether convert is reasonable. Assume that mapping
		 * pairs will take 8 bytes.
		 */
		if (le32_to_cpu(a->length) <= offsetof(ATTR_RECORD,
				compressed_size) + ((a->name_length *
				sizeof(ntfschar) + 7) & ~7) + 8)
			continue;

		tna = ntfs_attr_open(na->ni, a->type, (ntfschar*)((u8*)a +
				le16_to_cpu(a->name_offset)), a->name_length);
		if (!tna) {
			err = errno;
			ntfs_log_perror("Couldn't open attribute");
			goto put_err_out;
		}
		if (ntfs_attr_make_non_resident(tna, ctx)) {
			ntfs_attr_close(tna);
			continue;
		}
		if (((tna->data_flags & ATTR_COMPRESSION_MASK)
						== ATTR_IS_COMPRESSED)
		   && ntfs_attr_pclose(tna)) {
			err = errno;
			ntfs_attr_close(tna);
			goto put_err_out;
		}
		ntfs_inode_mark_dirty(tna->ni);
		ntfs_attr_close(tna);
		ntfs_attr_put_search_ctx(ctx);
		return ntfs_resident_attr_resize_i(na, newsize, force_non_resident);
	}
	/* Check whether error occurred. */
	if (errno != ENOENT) {
		err = errno;
		ntfs_log_perror("%s: Attribute lookup failed 1", __FUNCTION__);
		goto put_err_out;
	}
	
	/* 
	 * The standard information and attribute list attributes can't be
	 * moved out from the base MFT record, so try to move out others. 
	 */
	if (na->type==AT_STANDARD_INFORMATION || na->type==AT_ATTRIBUTE_LIST) {
		ntfs_attr_put_search_ctx(ctx);
		if (ntfs_inode_free_space(na->ni, offsetof(ATTR_RECORD,
				non_resident_end) + 8)) {
			ntfs_log_perror("Could not free space in MFT record");
			return -1;
		}
		return ntfs_resident_attr_resize_i(na, newsize, force_non_resident);
	}

	/*
	 * Move the attribute to a new mft record, creating an attribute list
	 * attribute or modifying it if it is already present.
	 */

	/* Point search context back to attribute which we need resize. */
	ntfs_attr_init_search_ctx(ctx, na->ni, NULL);
	if (ntfs_attr_lookup(na->type, na->name, na->name_len, CASE_SENSITIVE,
			0, NULL, 0, ctx)) {
		ntfs_log_perror("%s: Attribute lookup failed 2", __FUNCTION__);
		err = errno;
		goto put_err_out;
	}

	/*
	 * Check whether attribute is already single in this MFT record.
	 * 8 added for the attribute terminator.
	 */
	if (le32_to_cpu(ctx->mrec->bytes_in_use) ==
			le16_to_cpu(ctx->mrec->attrs_offset) +
			le32_to_cpu(ctx->attr->length) + 8) {
		err = ENOSPC;
		ntfs_debug("MFT record is filled with one attribute\n");
		ret = STATUS_RESIDENT_ATTRIBUTE_FILLED_MFT;
		goto put_err_out;
	}

	/* Add attribute list if not present. */
	if (na->ni->nr_extents == -1)
		ni = na->ni->base_ni;
	else
		ni = na->ni;
	if (!NInoAttrList(ni)) {
		ntfs_attr_put_search_ctx(ctx);
		if (ntfs_inode_add_attrlist(ni))
			return -1;
		return ntfs_resident_attr_resize_i(na, newsize, force_non_resident);
	}
	/* Allocate new mft record. */
	ni = ntfs_mft_record_alloc(vol, ni);
	if (!ni) {
		err = errno;
		ntfs_log_perror("Couldn't allocate new MFT record");
		goto put_err_out;
	}
	/* Move attribute to it. */
	if (ntfs_attr_record_move_to(ctx, ni)) {
		err = errno;
		ntfs_log_perror("Couldn't move attribute to new MFT record");
		goto put_err_out;
	}
	/* Update ntfs attribute. */
	if (na->ni->nr_extents == -1)
		na->ni = ni;

	ntfs_attr_put_search_ctx(ctx);
	/* Try to perform resize once again. */
	return ntfs_resident_attr_resize_i(na, newsize, force_non_resident);

resize_done:
	/*
	 * Set the inode (and its base inode if it exists) dirty so it is
	 * written out later.
	 */
	ntfs_inode_mark_dirty(ctx->ntfs_ino);
	ntfs_attr_put_search_ctx(ctx);
	return 0;
put_err_out:
	ntfs_attr_put_search_ctx(ctx);
	errno = err;
	return ret;
}

static int ntfs_resident_attr_resize(ntfs_attr *na, const s64 newsize)
{
	int ret; 
	
	ntfs_log_enter("Entering\n");
	ret = ntfs_resident_attr_resize_i(na, newsize, FALSE);
	ntfs_log_leave("\n");
	return ret;
}
/**
 * ntfs_non_resident_attr_shrink - shrink a non-resident, open ntfs attribute
 * @na:		non-resident ntfs attribute to shrink
 * @newsize:	new size (in bytes) to which to shrink the attribute
 *
 * Reduce the size of a non-resident, open ntfs attribute @na to @newsize bytes.
 *
 * On success return 0 and on error return -1 with errno set to the error code.
 * The following error codes are defined:
 *	ENOMEM	- Not enough memory to complete operation.
 *	ERANGE	- @newsize is not valid for the attribute type of @na.
 */
static int ntfs_non_resident_attr_shrink(ntfs_attr *na, const s64 newsize)
{
	ntfs_volume *vol;
	ntfs_attr_search_ctx *ctx;
	VCN first_free_vcn;
	s64 nr_freed_clusters;
	int err;

	ntfs_debug("Inode 0x%llx attr 0x%x new size %lld", (unsigned long long)
		       na->ni->mft_no, na->type, (long long)newsize);

	vol = na->ni->vol;

	/*
	 * Check the attribute type and the corresponding minimum size
	 * against @newsize and fail if @newsize is too small.
	 */
	if (ntfs_attr_size_bounds_check(vol, na->type, newsize) < 0) {
		//if (errno == ERANGE) {
			ntfs_debug("Eeek! Size bounds check failed. "
					"Aborting...");
		//} else if (errno == ENOENT)
		//	errno = EIO;
		return -1;
	}

	/* The first cluster outside the new allocation. */
	if (na->data_flags & ATTR_COMPRESSION_MASK)
		/*
		 * For compressed files we must keep full compressions blocks,
		 * but currently we do not decompress/recompress the last
		 * block to truncate the data, so we may leave more allocated
		 * clusters than really needed.
		 */
		first_free_vcn = (((newsize - 1)
				 | (na->compression_block_size - 1)) + 1)
				   >> vol->cluster_size_bits;
	else
		first_free_vcn = (newsize + vol->cluster_size - 1) >>
				vol->cluster_size_bits;
	/*
	 * Compare the new allocation with the old one and only deallocate
	 * clusters if there is a change.
	 */
	if ((na->allocated_size >> vol->cluster_size_bits) != first_free_vcn) {
		if (ntfs_attr_map_whole_runlist(na)) {
			ntfs_debug("Eeek! ntfs_attr_map_whole_runlist "
					"failed.");
			return -1;
		}
		/* Deallocate all clusters starting with the first free one. */
		nr_freed_clusters = ntfs_cluster_free(vol, na, first_free_vcn,
				-1);
		if (nr_freed_clusters < 0) {
			ntfs_debug("Eeek! Freeing of clusters failed. "
					"Aborting...");
			return -1;
		}

		/* Truncate the runlist itself. */
		if (ntfs_rl_truncate(&na->rl, first_free_vcn)) {
			/*
			 * Failed to truncate the runlist, so just throw it
			 * away, it will be mapped afresh on next use.
			 */
			free(na->rl);
			na->rl = NULL;
			ntfs_debug("Eeek! Run list truncation failed.");
			return -1;
		}

		/* Prepare to mapping pairs update. */
		na->allocated_size = first_free_vcn << vol->cluster_size_bits;
		/* Write mapping pairs for new runlist. */
		if (ntfs_attr_update_mapping_pairs(na, 0 /*first_free_vcn*/)) {
			ntfs_debug("Eeek! Mapping pairs update failed. "
					"Leaving inconstant metadata. "
					"Run chkdsk.");
			return -1;
		}
	}

	/* Get the first attribute record. */
	ctx = ntfs_attr_get_search_ctx(na->ni, NULL);
	if (!ctx)
		return -1;

	if (ntfs_attr_lookup(na->type, na->name, na->name_len, CASE_SENSITIVE,
			0, NULL, 0, ctx)) {
		//err = errno;
		//if (err == ENOENT)
		//	err = EIO;
		ntfs_debug("Eeek! Lookup of first attribute extent failed. "
				"Leaving inconstant metadata.");
		goto put_err_out;
	}

	/* Update data and initialized size. */
	na->data_size = newsize;
	ctx->attr->data_size = cpu_to_sle64(newsize);
	if (newsize < na->initialized_size) {
		na->initialized_size = newsize;
		ctx->attr->initialized_size = cpu_to_sle64(newsize);
	}
	/* Update data size in the index. */
	if (na->ni->mrec->flags & MFT_RECORD_IS_DIRECTORY) {
		if (na->type == AT_INDEX_ROOT && na->name == NTFS_INDEX_I30) {
			na->ni->data_size = na->data_size;
			na->ni->allocated_size = na->allocated_size;
			set_nino_flag(na->ni,KnownSize);
		}
	} else {
		if (na->type == AT_DATA && na->name == AT_UNNAMED) {
			na->ni->data_size = na->data_size;
			NInoFileNameSetDirty(na->ni);
		}
	}

	/* If the attribute now has zero size, make it resident. */
	if (!newsize) {
		if (ntfs_attr_make_resident(na, ctx)) {
			/* If couldn't make resident, just continue. */
			if (errno != EPERM)
				ntfs_debug("Failed to make attribute "
						"resident. Leaving as is...");
		}
	}

	/* Set the inode dirty so it is written out later. */
	ntfs_inode_mark_dirty(ctx->ntfs_ino);
	/* Done! */
	ntfs_attr_put_search_ctx(ctx);
	return 0;
put_err_out:
	ntfs_attr_put_search_ctx(ctx);
	errno = err;
	return -1;
}

/**扩展非驻留属性的大小
 * ntfs_non_resident_attr_expand - expand a non-resident, open ntfs attribute
 * @na:		non-resident ntfs attribute to expand
 * @newsize:	new size (in bytes) to which to expand the attribute
 *
 * Expand the size of a non-resident, open ntfs attribute @na to @newsize bytes,
 * by allocating new clusters.
 *
 * On success return 0 and on error return -1 with errno set to the error code.
 * The following error codes are defined:
 *	ENOMEM - Not enough memory to complete operation.
 *	ERANGE - @newsize is not valid for the attribute type of @na.
 *	ENOSPC - There is no enough space in base mft to resize $ATTRIBUTE_LIST.
 */
static int ntfs_non_resident_attr_expand_i(ntfs_attr_3g *na_3g, const s64 newsize)
{
	LCN lcn_seek_from;
	VCN first_free_vcn;
	ntfs_volume *vol;
	ntfs_attr_search_ctx *ctx;
	runlist *rl, *rln;
	s64 org_alloc_size;
	int err;

	ntfs_debug("Inode %lld, attr 0x%x, new size %lld old size %lld\n",
			(unsigned long long)na_3g->ni->mft_no, na_3g->type,
			(long long)newsize, (long long)na_3g->data_size);

	vol = na_3g->ni->vol;

	/*
	 * Check the attribute type and the corresponding maximum size
	 * against @newsize and fail if @newsize is too big.
	 */
	if (ntfs_attr_size_bounds_check(vol, na_3g->type, newsize) < 0) {//检测属性的合法性
		//if (errno == ENOENT)
		//	errno = EIO;
		ntfs_debug(" bounds check failed");
		return -1;
	}

	/* Save for future use. */
	org_alloc_size = na_3g->allocated_size;
	/* The first cluster outside the new allocation. */
	first_free_vcn = (newsize + vol->cluster_size - 1) >>		//新大小的首簇
			vol->cluster_size_bits;
	/*
	 * Compare the new allocation with the old one and only allocate
	 * clusters if there is a change.
	 仅当新首簇号大于旧首簇号时才分配新的..
	 */
	if ((na_3g->allocated_size >> vol->cluster_size_bits) < first_free_vcn) {
		if (ntfs_attr_map_whole_runlist(na_3g)) {
			ntfs_debug("ntfs_attr_map_whole_runlist failed");
			return -1;
		}

		/*
		 * If we extend $DATA attribute on NTFS 3+ volume, we can add
		 * sparse runs instead of real allocation of clusters.
		 添加稀疏数据流而不是真正分配簇
		 */
		if (na_3g->type == AT_DATA && vol->major_ver >= 3) {
			rl = kmalloc(0x1000);
			if (!rl)
				return -1;
			
			rl[0].vcn = (na_3g->allocated_size >>		//rl[0]: the first runlist
					vol->cluster_size_bits);
			rl[0].lcn = LCN_HOLE;
			rl[0].length = first_free_vcn -
				(na_3g->allocated_size >> vol->cluster_size_bits);
			rl[1].vcn = first_free_vcn;			//rl[1]: the second runlist
			rl[1].lcn = LCN_ENOENT;
			rl[1].length = 0;
		} else {
			/*
			 * Determine first after last LCN of attribute.
			 * We will start seek clusters from this LCN to avoid
			 * fragmentation.  If there are no valid LCNs in the
			 * attribute let the cluster allocator choose the
			 * starting LCN.
			 首先从该属性的末LCN开始搜索有效的LCN,
			 如果该属性中无有效的LCN了，就从起始LCN开始搜索。
			  */
			lcn_seek_from = -1;
			if (na_3g->rl->length) {
				/* Seek to the last run list element. */
				for (rl = na_3g->rl; (rl + 1)->length; rl++)		//找到末runlist
					;
				/*
				 * If the last LCN is a hole or similar seek
				 * back to last valid LCN.
				 */
				while (rl->lcn < 0 && rl != na_3g->rl)
					rl--;
				/*
				 * Only set lcn_seek_from it the LCN is valid.
				 */
				if (rl->lcn >= 0)
					lcn_seek_from = rl->lcn + rl->length;
			}

			rl = ntfs_cluster_alloc(vol, na_3g->allocated_size >>	//多一个参数 
					vol->cluster_size_bits, first_free_vcn -
					(na_3g->allocated_size >>
					vol->cluster_size_bits), lcn_seek_from,
					DATA_ZONE,1);
			if (!rl) {
				ntfs_debug("Cluster allocation failed "
						"(%lld)",
						(long long)first_free_vcn -
						((long long)na_3g->allocated_size >>
						 vol->cluster_size_bits));
				return -1;
			}
		}

		/* Append new clusters to attribute runlist. */
		rln = ntfs_runlists_merge(na_3g->rl, rl);		//将rl添加到na->rl
		if (!rln) {
			/* Failed, free just allocated clusters. */
			//err = errno;
			ntfs_debug("Run list merge failed");
			ntfs_cluster_free_from_rl(vol, rl);
			free(rl);
			//errno = err;
			return -1;
		}
		na_3g->rl = rln;

		/* Prepare to mapping pairs update. */
		na_3g->allocated_size = first_free_vcn << vol->cluster_size_bits;
		/* Write mapping pairs for new runlist. */
		if (ntfs_attr_update_mapping_pairs(na_3g, 0 /*na->allocated_size >>
				vol->cluster_size_bits*/)) {
			//err = errno;
			ntfs_debug("Mapping pairs update failed");
			goto rollback;
		}
	}

	ctx = ntfs_attr_get_search_ctx(na_3g->ni, NULL);
	if (!ctx) {
		//err = errno;
		if (na_3g->allocated_size == org_alloc_size) {
		//	errno = err;
			return -1;
		} else
			goto rollback;
	}

	if (ntfs_attr_lookup(na_3g->type, na_3g->name, na_3g->name_len, CASE_SENSITIVE,
			0, NULL, 0, ctx)) {
		//err = errno;
		ntfs_debug("Lookup of first attribute extent failed");
		//if (err == ENOENT)
		//	err = EIO;
		if (na_3g->allocated_size != org_alloc_size) {
			ntfs_attr_put_search_ctx(ctx);
			goto rollback;
		} else
			goto put_err_out;
	}

	/* Update data size. */
	na_3g->data_size = newsize;
	ctx->attr->data_size = cpu_to_sle64(newsize);
	/* Update data size in the index. */
	if (na_3g->ni->mrec->flags & MFT_RECORD_IS_DIRECTORY) {
		if (na_3g->type == AT_INDEX_ROOT && na_3g->name == NTFS_INDEX_I30) {
			na_3g->ni->data_size = na_3g->data_size;
			na_3g->ni->allocated_size = na_3g->allocated_size;
			set_nino_flag(na_3g->ni,KnownSize);
		}
	} else {
		if (na_3g->type == AT_DATA && na_3g->name == AT_UNNAMED) {
			na_3g->ni->data_size = na_3g->data_size;
			NInoFileNameSetDirty(na_3g->ni);
		}
	}
	/* Set the inode dirty so it is written out later. */
	ntfs_inode_mark_dirty(ctx->ntfs_ino);
	/* Done! */
	ntfs_attr_put_search_ctx(ctx);
	return 0;
rollback:
	/* Free allocated clusters. */
	if (ntfs_cluster_free(vol, na_3g, org_alloc_size >>
			vol->cluster_size_bits, -1) < 0) {
		err = EIO;
		ntfs_debug("Leaking clusters");
	}
	/* Now, truncate the runlist itself. */
	if (ntfs_rl_truncate(&na_3g->rl, org_alloc_size >>
			vol->cluster_size_bits)) {
		/*
		 * Failed to truncate the runlist, so just throw it away, it
		 * will be mapped afresh on next use.
		 */
		free(na_3g->rl);
		na_3g->rl = NULL;
		ntfs_debug("Couldn't truncate runlist. Rollback failed");
	} else {
		/* Prepare to mapping pairs update. */
		na_3g->allocated_size = org_alloc_size;
		/* Restore mapping pairs. */
		if (ntfs_attr_update_mapping_pairs(na_3g, 0 /*na->allocated_size >>
					vol->cluster_size_bits*/)) {
			ntfs_debug("Failed to restore old mapping pairs");
		}
	}
	//errno = err;
	return -1;
put_err_out:
	ntfs_attr_put_search_ctx(ctx);
	//errno = err;
	return -1;
}


static int ntfs_non_resident_attr_expand(ntfs_attr_3g *na_3g, const s64 newsize)
{
	int ret; 
	
	ntfs_debug("Entering");
	ret = ntfs_non_resident_attr_expand_i(na_3g, newsize);
	ntfs_debug("Done");
	return ret;
}


/**改变ntfs 属性的大小
 * ntfs_attr_truncate - resize an ntfs attribute
 * @na:		open ntfs attribute to resize
 * @newsize:	new size (in bytes) to which to resize the attribute
 *
 * Change the size of an open ntfs attribute @na to @newsize bytes. If the
 * attribute is made bigger and the attribute is resident the newly
 * "allocated" space is cleared and if the attribute is non-resident the
 * newly allocated space is marked as not initialised and no real allocation
 * on disk is performed.
 如果属性变大，当属性为驻留时，其新分配的空间将被清空；
 当属性为非驻留时，其分配的空间会标记为未初始化，且
 不会改变磁盘上的分配大小；
 *
 * On success return 0.
 * On error return values are:
 * 	STATUS_RESIDENT_ATTRIBUTE_FILLED_MFT
 * 	STATUS_ERROR - otherwise
 * The following error codes are defined:
 *	EINVAL	   - Invalid arguments were passed to the function.
 *	EOPNOTSUPP - The desired resize is not implemented yet.
 * 	EACCES     - Encrypted attribute.
 */
int ntfs_attr_truncate(ntfs_attr_3g *na_3g, const s64 newsize)
{
	int ret = STATUS_ERROR;
	s64 fullsize;
	BOOL compressed;

	if (!na_3g || newsize < 0 ||
			(na_3g->ni->mft_no == FILE_MFT && na_3g->type == AT_DATA)) {
		ntfs_debug("Invalid arguments passed.");
		//errno = EINVAL;
		return STATUS_ERROR;
	}

	ntfs_debug("Entering for inode %lld, attr 0x%x, size %lld\n",
		       (unsigned long long)na_3g->ni->mft_no, na_3g->type, 
		       (long long)newsize);

	if (na_3g->data_size == newsize) {//大小没改变
		ntfs_debug("Size is already ok\n");
		ret = STATUS_OK;
		goto out;
	}
	/*
	 * Encrypted attributes are not supported. We return access denied,
	 * which is what Windows NT4 does, too.
	 */
	if (na_3g->data_flags & ATTR_IS_ENCRYPTED) {//加密属性不能改变大小
		//errno = EACCES;
		ntfs_debug("Cannot truncate encrypted attribute\n");
		goto out;
	}
	/*
	 * TODO: Implement making handling of compressed attributes.
	 * Currently we can only expand the attribute or delete it,
	 * and only for ATTR_IS_COMPRESSED. This is however possible
	 * for resident attributes when there is no open fuse context
	 * (important case : $INDEX_ROOT:$I30)
	 */
	compressed = (na_3g->data_flags & ATTR_COMPRESSION_MASK)
			 != const_cpu_to_le16(0);
	if (compressed
	   && NAttrNonResident(na_3g)
	   && ((na_3g->data_flags & ATTR_COMPRESSION_MASK) != ATTR_IS_COMPRESSED)) {
		//errno = EOPNOTSUPP;
		ntfs_debug("Failed to truncate compressed attribute");
		goto out;
	}
	if (NAttrNonResident(na_3g)) {//非驻留
		/*
		 * For compressed data, the last block must be fully
		 * allocated, and we do not know the size of compression
		 * block until the attribute has been made non-resident.
		 * Moreover we can only process a single compression
		 * block at a time (from where we are about to write),
		 * so we silently do not allocate more.
		 *
		 * Note : do not request upsizing of compressed files
		 * unless being able to face the consequences !
		 */
		if (compressed && newsize && (newsize > na_3g->data_size))
			fullsize = (na_3g->initialized_size
				 | (na_3g->compression_block_size - 1)) + 1;
		else
			fullsize = newsize;
		if (fullsize > na_3g->data_size)
			ret = ntfs_non_resident_attr_expand(na_3g, fullsize);
		else
			ret = ntfs_non_resident_attr_shrink(na_3g, fullsize);
	} else
		ret = ntfs_resident_attr_resize(na_3g, newsize);
out:	
	ntfs_debug("Return status %d\n", ret);
	return ret;
}
int ntfs_attr_write(ntfs_inode *ni,u8 *val, s64 size,int  pos)
{
	s64 end, initialized_size;
	loff_t i_size;
	struct inode *vi;
	ntfs_inode *base_ni;
	struct page *page;
	ntfs_attr_search_ctx *ctx;
	MFT_RECORD *m;
	ATTR_RECORD *a;
	char *kattr, *kaddr;
	unsigned long flags;
	u32 attr_len;
	int err;

	vi = VFS_I(ni)
	/*
	 * Attribute is resident, implying it is not compressed, encrypted, or
	 * sparse.驻留属性
	 */
	if (!NInoAttr(ni))//判断其是否为主节点
		base_ni = ni;
	else
		base_ni = ni->ext.base_ntfs_ino;//次节点的主节点
		
	DEBUG0_ON(NInoNonResident(ni));
	/* Map, pin, and lock the mft record. */
	m = map_mft_record(base_ni);
	if (IS_ERR(m)) {
		err = PTR_ERR(m);
		m = NULL;
		ctx = NULL;
		goto err_out;
	}
	ctx = ntfs_attr_get_search_ctx(base_ni, m);
	if (unlikely(!ctx)) {
		err = -ENOMEM;
		goto err_out;
	}
	err = ntfs_attr_lookup(ni->type, ni->name, ni->name_len,
			CASE_SENSITIVE, 0, NULL, 0, ctx);
	if (unlikely(err)) {
		if (err == -ENOENT)
			err = -EIO;
		goto err_out;
	}
	a = ctx->attr;
	DEBUG0_ON(a->non_resident);
	/* The total length of the attribute value. */
	attr_len = le32_to_cpu(a->data.resident.value_length);//属性长度
	i_size = i_size_read(vi);
	DEBUG0_ON(attr_len != i_size);
	DEBUG0_ON(pos > attr_len);
	end = pos + size;
	DEBUG0_ON(end > le32_to_cpu(a->length) -
			le16_to_cpu(a->data.resident.value_offset));
	kattr = (u8*)a + le16_to_cpu(a->data.resident.value_offset);//属性de 地址
	kaddr = kmap_atomic(val, KM_USER0);//待写入值de地址
	/* Copy the received data from the page to the mft record. */
	memcpy(kattr + pos, kaddr + pos, size);//将page的内容拷贝到mft record
	/* Update the attribute length if necessary. 更新属性的长度*/
	if (end > attr_len) {
		attr_len = end;
		a->data.resident.value_length = cpu_to_le32(attr_len);
	}
	
	kunmap_atomic(val, KM_USER0);
	/* Update initialized_size/i_size if necessary. */
	read_lock_irqsave(&ni->size_lock, flags);
	initialized_size = ni->initialized_size;
	DEBUG0_ON(end > ni->allocated_size);
	read_unlock_irqrestore(&ni->size_lock, flags);
	DEBUG0_ON(initialized_size != i_size);
	if (end > initialized_size) {
		write_lock_irqsave(&ni->size_lock, flags);
		ni->initialized_size = end;
		i_size_write(vi, end);
		write_unlock_irqrestore(&ni->size_lock, flags);
	}
	/* Mark the mft record dirty, so it gets written back. */
	flush_dcache_mft_record_page(ctx->ntfs_ino);
	mark_mft_record_dirty(ctx->ntfs_ino);
	ntfs_attr_put_search_ctx(ctx);
	unmap_mft_record(base_ni);
	ntfs_debug("Done.");
	return 0;
err_out:
	if (err == -ENOMEM) {
		ntfs_warning(vi->i_sb, "Error allocating memory required to "
				"commit the write.");		
	} else {
		ntfs_error(vi->i_sb, "Resident attribute commit write failed "
				"with error %i.", err);
		NVolSetErrors(ni->vol);
	}
	if (ctx)
		ntfs_attr_put_search_ctx(ctx);
	if (m)
		unmap_mft_record(base_ni);
	return err;
}
#endif

/**遍历节点中的属性
 * ntfs_attrs_walk - syntactic sugar for walking all attributes in an inode
 * @ctx:	initialised attribute search context
 *
 * Syntactic sugar for walking attributes in an inode.
 *
 * Return 0 on success and -1 on error with errno set to the error code from
 * ntfs_attr_lookup().
 *
 * Example: When you want to enumerate all attributes in an open ntfs inode
 *	    @ni, you can simply do:
 *
 *	int err;
 *	ntfs_attr_search_ctx *ctx = ntfs_attr_get_search_ctx(ni, NULL);
 *	if (!ctx)
 *		// Error code is in errno. Handle this case.
 *	while (!(err = ntfs_attrs_walk(ctx))) {
 *		ATTR_RECORD *attr = ctx->attr;
 *		// attr now contains the next attribute. Do whatever you want
 *		// with it and then just continue with the while loop.
 *	}
 *	if (err && errno != ENOENT)
 *		// Ooops. An error occurred! You should handle this case.
 *	// Now finished with all attributes in the inode.
 */
 int ntfs_attrs_walk(ntfs_attr_search_ctx *ctx)
{
	return ntfs_attr_lookup(AT_UNUSED, NULL, 0, CASE_SENSITIVE, 0,
			NULL, 0, ctx);
}


/**判断0x20属性是否还需要
 * ntfs_attrlist_need - check whether inode need attribute list
 * @ni:		opened ntfs inode for which perform check
 *
 * Check whether all are attributes belong to one MFT record, in that case
 * attribute list is not needed.
 如果所有的属性都在mft record上，那么就不需存在了
 *
 * Return 1 if inode need attribute list, 0 if not, -1 on error with errno set
 * to the error code. If function succeed errno set to 0. The following error
 * codes are defined:
 如果需要返回1，不需要返回0，错误返回-1
 *	EINVAL	- Invalid arguments passed to function or attribute haven't got
 *		  attribute list.
 */
int ntfs_attrlist_need(ntfs_inode *ni)
{
	ATTR_LIST_ENTRY *ale;

	if (!ni) {
		ntfs_debug("Invalid arguments.\n");
		//errno = EINVAL;
		return -1;
	}

	ntfs_debug("Entering for inode 0x%llx.\n", (long long) ni->mft_no);

	if (!NInoAttrList(ni)) {
		ntfs_debug("Inode haven't got attribute list.\n");
		//errno = EINVAL;
		return -1;
	}

	if (!ni->attr_list) {
		ntfs_debug("Corrupt in-memory struct.\n");
		//errno = EINVAL;
		return -1;
	}

	//errno = 0;
	ale = (ATTR_LIST_ENTRY *)ni->attr_list;
	while ((u8*)ale < ni->attr_list + ni->attr_list_size) {
		if (MREF_LE(ale->mft_reference) != ni->mft_no)
			return 1;
		ale = (ATTR_LIST_ENTRY *)((u8*)ale + le16_to_cpu(ale->length));
	}
	return 0;
}


/**将属性attr添加到ni的$Attribute_List--扩展属性列表中
 * ntfs_attrlist_entry_add - add an attribute list attribute entry
 * @ni:		opened ntfs inode, which contains that attribute
 * @attr:	attribute record to add to attribute list
 *
 * Return 0 on success and -1 on error with errno set to the error code. The
 * following error codes are defined:
 *	EINVAL	- Invalid arguments passed to function.
 *	ENOMEM	- Not enough memory to allocate necessary buffers.
 *	EIO	- I/O error occurred or damaged filesystem.
 *	EEXIST	- Such attribute already present in attribute list.
 */
int ntfs_attrlist_entry_add(ntfs_inode *ni, ATTR_RECORD *attr)
{
	ATTR_LIST_ENTRY *ale;//0x20 $Attribute_List
	MFT_REF mref;
	//ntfs_attr_3g *na_3g = NULL;
	ntfs_attr_search_ctx *ctx;
	u8 *new_al;		/**attribute value itself*/
	int entry_len, entry_offset, err;
	MFT_RECORD *mrec = NULL;
	struct inode * vi =VFS_I(ni);
	
	ntfs_debug("Entering for inode 0x%llx, attr 0x%x.",
			(long long) ni->mft_no,
			(unsigned) le32_to_cpu(attr->type));

	if (!ni || !attr) {
		ntfs_debug("Invalid arguments.");
		//errno = EINVAL;
		return -1;
	}

	mrec  =  map_mft_record(ni);
	if (IS_ERR(mrec)) {
		err = PTR_ERR(mrec);
		ntfs_error(vi->i_sb, "Failed to map mft record for inode 0x%lx "
				"(error code %d)", vi->i_ino, err);
		return err;
	}
	//获得MFT ?
	mref = MK_LE_MREF(ni->mft_no, le16_to_cpu(mrec->sequence_number));
	
	if (ni->nr_extents == -1)
		ni = ni->ext.base_ntfs_ino;	//扩展属性列表一定是主节点的属性

	if (!NInoAttrList(ni)) {
		ntfs_debug("Attribute list isn't present.");
		//errno = ENOENT;
		return -1;
	}

	/* Determine size and allocate memory for new attribute list. */
	entry_len = (sizeof(ATTR_LIST_ENTRY) + sizeof(ntfschar) * //$Attribute_List的长度
			attr->name_length + 7) & ~7;
	new_al = kcalloc(1,ni->attr_list_size + entry_len,GFP_KERNEL);
	if (!new_al)
		return -1;

	/* Find place for the new entry. */
	ctx = ntfs_attr_get_search_ctx(ni, mrec);
	if (!ctx) {
		//err = errno;
		goto err_out;
	}
	unmap_mft_record(ni);

	err = ntfs_attr_lookup(attr->type, (attr->name_length) ? (ntfschar*)
			((u8*)attr + le16_to_cpu(attr->name_offset)) :
			AT_UNNAMED, attr->name_length, CASE_SENSITIVE,
			(attr->non_resident) ? le64_to_cpu(attr->data.non_resident.lowest_vcn) :
			0, (attr->non_resident) ? NULL : ((u8*)attr +
			le16_to_cpu(attr->data.resident.value_offset)), (attr->non_resident) ?
			0 : le32_to_cpu(attr->data.resident.value_length), ctx);
			
	if (err == 0) {
		/* Found some extent, check it to be before new extent. */
		if (ctx->al_entry->lowest_vcn == attr->data.non_resident.lowest_vcn) {//属性列表中已存在该属性
			//err = EEXIST;
			ntfs_debug("Such attribute already present in the "
					"attribute list.");			
			goto err_out;
		}
		/* Add new entry after this extent.紧挨着该ATTR_LIST_ENTRY后新建 ale*/
		ale = (ATTR_LIST_ENTRY*)((u8*)ctx->al_entry +
				le16_to_cpu(ctx->al_entry->length));
	} else {
		/* Check for real errors. */
		if (err != -ENOENT) {
			//err = errno;
			ntfs_debug("Attribute lookup failed.\n");			
			goto err_out;
		}
		/* No previous extents found. */
		ale = ctx->al_entry;//直接利用该属性列表
	}
	/* Don't need it anymore, @ctx->al_entry points to @ni->attr_list. */
	ntfs_attr_put_search_ctx(ctx);

	/* Determine new entry offset. 新属性列表的在attr_list中的偏移*/
	entry_offset = ((u8 *)ale - ni->attr_list);
	/* Set pointer to new entry. */
	ale = (ATTR_LIST_ENTRY *)(new_al + entry_offset);
	/* Zero it to fix valgrind warning. */
	memset(ale, 0, entry_len);
	/* Form new entry. */
	ale->type = attr->type;
	ale->length = cpu_to_le16(entry_len);
	ale->name_length = attr->name_length;
	ale->name_offset = offsetof(ATTR_LIST_ENTRY, name);
	if (attr->non_resident)
		ale->lowest_vcn = attr->data.non_resident.lowest_vcn;
	else
		ale->lowest_vcn = 0;
	ale->mft_reference = mref;
	ale->instance = attr->instance;
	/**may bug here*/
	memcpy(ale->name, (u8 *)attr + le16_to_cpu(attr->name_offset),
			attr->name_length * sizeof(ntfschar));

	/* Resize $ATTRIBUTE_LIST to new length. */
#if 0
	na_3g = ntfs_attr_open(ni, AT_ATTRIBUTE_LIST, AT_UNNAMED, 0);
	if (!na_3g) {
		err = errno;
		ntfs_debug("Failed to open $ATTRIBUTE_LIST attribute.\n");
		goto err_out;
	}
	if (ntfs_attr_truncate(na_3g, ni->attr_list_size + entry_len)) {
		err = errno;
		ntfs_debug("$ATTRIBUTE_LIST resize failed.\n");
		goto err_out;
	}
#endif
	if((ntfs_attr_truncate(ni, AT_ATTRIBUTE_LIST, AT_UNNAMED, 0,ni->attr_list_size + entry_len)) != 0)//by yangli
	{
		ntfs_error(vi->i_sb,"Failed to initialize just added attribute");
		goto err_out;
	}
	/* Copy entries from old attribute list to new. */
	memcpy(new_al, ni->attr_list, entry_offset);
	memcpy(new_al + entry_offset + entry_len, ni->attr_list +
			entry_offset, ni->attr_list_size - entry_offset);

	/* Set new runlist. */
	kfree(ni->attr_list);
	ni->attr_list = new_al;
	ni->attr_list_size = ni->attr_list_size + entry_len;
	
	/**FIXEME: all the NIoSet things */
	//NInoAttrListSetDirty(ni);
	
	/* Done! */
	//ntfs_attr_close(na_3g);
	ntfs_debug("Done.");
	return 0;
err_out:
	//if (na_3g)
	//	ntfs_attr_close(na_3g);
	if(mrec)
		unmap_mft_record(ni);
	if(ctx)
		ntfs_attr_put_search_ctx(ctx);
	if(new_al)
		kfree(new_al);
//	errno = err;
	ntfs_debug("Failed.");

	return -1;
}

/**
 * ntfs_attrlist_entry_rm - remove an attribute list attribute entry
 * @ctx:	attribute search context describing the attribute list entry
 *
 * Remove the attribute list entry @ctx->al_entry from the attribute list.
 *将ctx->al_entry 从属性列表中移除
 
 * Return 0 on success and -1 on error with errno set to the error code.
 */
int ntfs_attrlist_entry_rm(ntfs_attr_search_ctx *ctx)
{
	u8 *new_al;
	int new_al_len;
	ntfs_inode *base_ni;	
	ATTR_LIST_ENTRY *ale;
	
	struct inode * vi =VFS_I(ctx->ntfs_ino);
	
	if (!ctx || !ctx->ntfs_ino || !ctx->al_entry) {
		ntfs_debug("Invalid arguments.\n");
		//errno = EINVAL;
		return -1;
	}

	if (ctx->base_ntfs_ino)
		base_ni = ctx->base_ntfs_ino;
	else
		base_ni = ctx->ntfs_ino;
	ale = ctx->al_entry;

	ntfs_debug("Entering for inode 0x%llx, attr 0x%x, lowest_vcn %lld.\n",
			(long long) ctx->ntfs_ino->mft_no,
			(unsigned) le32_to_cpu(ctx->al_entry->type),
			(long long) le64_to_cpu(ctx->al_entry->lowest_vcn));

	if (!NInoAttrList(base_ni)) {
		ntfs_debug("Attribute list isn't present.\n");
		//errno = ENOENT;
		return -1;
	}

	/* Allocate memory for new attribute list. */
	new_al_len = base_ni->attr_list_size - le16_to_cpu(ale->length);
	new_al = kcalloc(1,new_al_len,GFP_KERNEL);
	if (!new_al)
		return -1;
#if 0
	/* Reisze $ATTRIBUTE_LIST to new length. */
	na = ntfs_attr_open(base_ni, AT_ATTRIBUTE_LIST, AT_UNNAMED, 0);
	if (!na) {
		err = errno;
		ntfs_debug("Failed to open $ATTRIBUTE_LIST attribute.\n");
		goto err_out;
	}
	if (ntfs_attr_truncate(na, new_al_len)) {
		err = errno;
		ntfs_debug("$ATTRIBUTE_LIST resize failed.\n");
		goto err_out;
	}
#endif
	if((ntfs_attr_truncate(base_ni, AT_ATTRIBUTE_LIST, AT_UNNAMED, 0,new_al_len)) != 0)//by yangli
	{
		ntfs_error(vi->i_sb,"Failed to initialize just added attribute");
		goto err_out;
	}
	/* Copy entries from old attribute list to new. */
	memcpy(new_al, base_ni->attr_list, (u8*)ale - base_ni->attr_list);
	memcpy(new_al + ((u8*)ale - base_ni->attr_list), (u8*)ale + le16_to_cpu(
		ale->length), new_al_len - ((u8*)ale - base_ni->attr_list));

	/* Set new runlist. */
	kfree(base_ni->attr_list);
	base_ni->attr_list = new_al;
	base_ni->attr_list_size = new_al_len;
	//NInoAttrListSetDirty(base_ni);
	/* Done! */
	//ntfs_attr_close(na);
	return 0;
err_out:
	//if (na)
	//	ntfs_attr_close(na);
	kfree(new_al);
	//errno = err;
	return -1;
}


/**
 * ntfs_attr_record_rm - remove attribute extent
 * @ctx:	search context describing the attribute which should be removed
 *
 * If this function succeed, user should reinit search context if he/she wants
 * use it anymore.
 *
 * Return 0 on success and -1 on error. On error the error code is stored in
 * errno. Possible error codes are:
 *	EINVAL	- Invalid arguments passed to function.
 *	EIO	- I/O error occurred or damaged filesystem.
 */
int ntfs_attr_record_rm(ntfs_attr_search_ctx *ctx)
{
	ntfs_inode *base_ni, *ni;
	ATTR_TYPE type;
	MFT_RECORD *m = NULL;
	int err = 0;
	ntfs_debug("Entering for inode 0x%llx, attr 0x%x.",
			(long long) ctx->ntfs_ino->mft_no,
			(unsigned) le32_to_cpu(ctx->attr->type));
	
	if (!ctx || !ctx->ntfs_ino || !ctx->mrec || !ctx->attr) {
		//errno = EINVAL;
		return -EINVAL;
	}	

	
	type = ctx->attr->type;
	ni = ctx->ntfs_ino;
	if (ctx->base_ntfs_ino)
		base_ni = ctx->base_ntfs_ino;
	else
		base_ni = ctx->ntfs_ino;

	/* Remove attribute itself. */
	if ((ntfs_attr_record_resize(ctx->mrec, ctx->attr, 0)) != 0) {  //将属性记录清零
		ntfs_debug("Couldn't remove attribute record. Bug or damaged MFT "
				"record.");
		if (NInoAttrList(base_ni) && type != AT_ATTRIBUTE_LIST)
			if ((ntfs_attrlist_entry_add(ni, ctx->attr)) !=0)	//?? 添加??
				ntfs_debug("Rollback failed. Leaving inconstant "
						"metadata.\n");
		return -EIO;
	}

	//ntfs_inode_mark_dirty(ni);
	 m = map_mft_record(ni);
	if (IS_ERR(m)) {
		err = PTR_ERR(m);
		ntfs_error(VFS_I(ni)->i_sb, "Failed to map mft record for inode 0x%lx "
				"(error code %d)", VFS_I(ni)->i_ino, err);	
		return err;
	}
	flush_dcache_mft_record_page(ctx->ntfs_ino);
	ntfs_debug("next2");
	mark_mft_record_dirty(ctx->ntfs_ino);
	ntfs_debug("next3");
	unmap_mft_record(ctx->ntfs_ino);
	/*
	 * Remove record from $ATTRIBUTE_LIST if present and we don't want
	 * delete $ATTRIBUTE_LIST itself.
	 */
	if (NInoAttrList(base_ni) && type != AT_ATTRIBUTE_LIST) { 
		if (ntfs_attrlist_entry_rm(ctx)) {	//将该属性从属性列表中删除
			ntfs_debug("Couldn't delete record from "
					"$ATTRIBUTE_LIST.\n");
			return -1;
		}
	}

	/* Post $ATTRIBUTE_LIST delete setup. */
	if (type == AT_ATTRIBUTE_LIST) {	//如果属性恰好是0x20属性列表，直接释放
		if (NInoAttrList(base_ni) && base_ni->attr_list)
			kfree(base_ni->attr_list);	
		base_ni->attr_list = NULL;
		NInoClearAttrList(base_ni);
		//NInoAttrListClearDirty(base_ni);
	}

	/* Free MFT record, if it doesn't contain attributes. */
	if (le32_to_cpu(ctx->mrec->bytes_in_use) -
			le16_to_cpu(ctx->mrec->attrs_offset) == 8) {
		if (ntfs_mft_record_free(ni->vol, ni)) {	//将该mft record从卷标中移除
			// FIXME: We need rollback here.
			ntfs_debug("Couldn't free MFT record.\n");			
			return -EIO;
		}
		/* Remove done if we freed base inode. */
		if (ni == base_ni)
			return 0;
	}

	if (type == AT_ATTRIBUTE_LIST || !NInoAttrList(base_ni))
		return 0;

	/* Remove attribute list if we don't need it any more. */
	if (!ntfs_attrlist_need(base_ni)) { //判断0x20是否还需要
		ntfs_attr_reinit_search_ctx(ctx);
		if ((ntfs_attr_lookup(AT_ATTRIBUTE_LIST, NULL, 0, CASE_SENSITIVE,
				0, NULL, 0, ctx)) != 0) {
			/*
			 * FIXME: Should we succeed here? Definitely something
			 * goes wrong because NInoAttrList(base_ni) returned
			 * that we have got attribute list.
			 */
			ntfs_debug("Couldn't find attribute list. Succeed "
					"anyway.\n");
			return 0;
		}
		/* Deallocate clusters. */
		if (ctx->attr->non_resident) { //非驻留属性: 释放簇
			runlist_element *al_rl;

			al_rl = ntfs_mapping_pairs_decompress(base_ni->vol,
					ctx->attr, NULL);
			if (IS_ERR(al_rl)){
				err = PTR_ERR(al_rl);
				ntfs_debug("Couldn't decompress attribute list "
						"runlist. Succeed anyway.\n");
				return err;
			}
			if (ntfs_cluster_free_from_rl(base_ni->vol, al_rl)) {
				ntfs_debug("Leaking clusters! Run chkdsk. "
						"Couldn't free clusters from "
						"attribute list runlist.\n");
			}
			kfree(al_rl);
		}
		/* Remove attribute record itself. */
		if (ntfs_attr_record_rm(ctx)) {
			/*
			 * FIXME: Should we succeed here? BTW, chkdsk doesn't
			 * complain if it find MFT record with attribute list,
			 * but without extents.
			 */
			ntfs_debug("Couldn't remove attribute list. Succeed "
					"anyway.\n");
			return 0;
		}
	}
	ntfs_debug("Done");
	return 0;
}
/**
 * ntfs_attr_rm - remove attribute from ntfs inode
 * @na:		opened ntfs attribute to delete
 *
 * Remove attribute and all it's extents from ntfs inode. If attribute was non
 * resident also free all clusters allocated by attribute.
 *
 * Return 0 on success or -1 on error with errno set to the error code.
 */
int ntfs_attr_rm(ntfs_inode * ni, ATTR_TYPE type, ntfschar *name, u8 name_len)
{
	ntfs_attr_search_ctx *ctx;
	MFT_RECORD *mrec =NULL;
	int ret = 0;
	struct inode * vi = VFS_I(ni);	
	int find = 0;
	
	mrec = map_mft_record(ni);//$MFT 记录头
	if (IS_ERR(mrec)) {
		ret= PTR_ERR(mrec);
		ntfs_error(vi->i_sb, "Failed to map mft record for inode 0x%lx "
				"(error code %d).", vi->i_ino, ret);
		return ret;
	}
	/* Locate place where record should be. */
	ctx = ntfs_attr_get_search_ctx(ni, mrec);
	if (!ctx){
		unmap_mft_record(ni);
		return -1;
	}

	ntfs_debug("Entering for inode 0x%llx, attr 0x%x.\n",
		(long long) ni->mft_no, type);

	/* Free cluster allocation. */
	ret = ntfs_attr_can_be_non_resident(ni->vol, type);
	if(ret == 0){	
	//if (NAttrNonResident(ni)) {
		//if (ntfs_attr_map_whole_runlist(na)) ?????
		//	return -1;
		if (ntfs_cluster_free(ni, 0, -1,ctx) < 0) { //释放所有的runlist
			ntfs_debug("Failed to free cluster allocation. Leaving "
					"inconstant metadata.\n");
			ret = -1;
		}
	}
	
	/* Search for attribute extents and remove them all. */
	while (!ntfs_attr_lookup(type, name, name_len,
				CASE_SENSITIVE, 0, NULL, 0, ctx)) {
		find = 1;
		if (ntfs_attr_record_rm(ctx)) {
			ntfs_debug("Failed to remove attribute extent. Leaving "
					"inconstant metadata.\n");
			ret = -1;
		}
		ntfs_attr_reinit_search_ctx(ctx);		
	}
	ntfs_attr_put_search_ctx(ctx);
	if(find == 0){
		ntfs_debug("Attribute lookup failed. Probably leaving inconstant "
				"metadata.\n");
		ret = -1;
	}
	return ret;
}
/**
 * ntfs_make_room_for_attr - make room for an attribute inside an mft record
 * @m:		mft record
 * @pos:	position at which to make space
 * @size:	byte size to make available at this position
 *
 * @pos points to the attribute in front of which we want to make space.
 *
 * Return 0 on success or on error. On error the error code is stored in
 * err. Possible error codes are:
 *	ENOSPC	- There is not enough space available to complete operation. The
 *		  caller has to make space before calling this.
 *	EINVAL	- Input parameters were faulty.
 */
int ntfs_make_room_for_attr(MFT_RECORD *m, u8 *pos, u32 size)
{
	u32 biu;
		
	ntfs_trace("Entering for pos 0x%x, size %u.",
		(int)(pos - (u8*)m), (unsigned) size);

	/* Make size 8-byte alignment. */
	size = (size + 7) & ~7;

	/* Rigorous consistency checks. */
	if (!m || !pos || pos < (u8*)m) {		
		ntfs_debug("pos=%p  m=%p",pos, m);
		return  -EINVAL;
	}
	/* The -8 is for the attribute terminator. */
	if (pos - (u8*)m > (int)le32_to_cpu(m->bytes_in_use) - 8) 
		return -EINVAL ;
	
	/* Nothing to do. */
	if (!size)
		return 0;

	biu = le32_to_cpu(m->bytes_in_use);
	/* Do we have enough space? */ 	/*so we donot extend mft record size here!*/
	if (biu + size > le32_to_cpu(m->bytes_allocated) ||
	    pos + size > (u8*)m + le32_to_cpu(m->bytes_allocated)) {	
		ntfs_debug("No enough space in the MFT record");
		return -ENOSPC ;
	}
	/* Move everything after pos to pos + size. */
	memmove(pos + size, pos, biu - (pos - (u8*)m));
	/* Update mft record. */
	m->bytes_in_use = cpu_to_le32(biu + size);
	return 0;
}
/**将原inode中的属性记录move到目标inode
 * ntfs_attr_record_move_to - move attribute record to target inode
 * @ctx:	attribute search context describing the attribute record
 * @ni:		opened ntfs inode to which move attribute record
 *
 * If this function succeed, user should reinit search context if he/she wants
 * use it anymore.
 *
 * Return 0 on success and -1 on error with errno set to the error code.
 */
int ntfs_attr_record_move_to(ntfs_attr_search_ctx *ctx, ntfs_inode *ni)
{
	ntfs_attr_search_ctx *nctx;
	ATTR_RECORD *a;
	int err;
	MFT_RECORD *m = NULL;
	struct inode *vi = VFS_I(ctx->ntfs_ino);

	if (!ctx || !ctx->attr || !ctx->ntfs_ino || !ni) {
		ntfs_debug("Invalid arguments passed.");		
		return -EINVAL;
	}

	ntfs_debug("Entering for ctx->attr->type 0x%x, ctx->ntfs_ino->mft_no "
			"0x%llx, ni->mft_no 0x%llx.",
			(unsigned) le32_to_cpu(ctx->attr->type),
			(long long) ctx->ntfs_ino->mft_no,
			(long long) ni->mft_no);

	if (ctx->ntfs_ino == ni)//不能是本身节点
		return 0;

	if (!ctx->al_entry) {
		ntfs_debug("Inode should contain attribute list to use this "
				"function.");		
		return -EINVAL;
	}

	/* Find place in MFT record where attribute will be moved. */
	a = ctx->attr;
	m = map_mft_record(ni);
		if (IS_ERR(m)) {
			err = PTR_ERR(m);
			ntfs_error(vi->i_sb, "Failed to map mft record for inode 0x%lx "
					"(error code %d).", vi->i_ino,err);		
			return err;
		}
	nctx = ntfs_attr_get_search_ctx(ni, m);
	if (!nctx){
		unmap_mft_record(ni);
		return -1;
		}
	//unmap_mft_record(ni);
	/*
	 * Use ntfs_attr_find instead of ntfs_attr_lookup to find place for
	 * attribute in @ni->mrec, not any extent inode in case if @ni is base
	 * file record.
	 */
	 err = ntfs_attr_find(a->type, (ntfschar*)((u8*)a + le16_to_cpu(
			a->name_offset)), a->name_length, CASE_SENSITIVE, NULL,
			0, nctx);
	if (err == 0) {
		ntfs_error(vi->i_sb, "Attribute of such type, with same name already "
				"present in this MFT record.");
		err = -EEXIST;
		goto put_err_out;
	}
	if (err != -ENOENT) {
		ntfs_error(vi->i_sb,"Attribute lookup failed.");
		goto put_err_out;
	}

	/* Make space and move attribute. */
	err = ntfs_make_room_for_attr(m, (u8*) nctx->attr,
					le32_to_cpu(a->length));
	if (err != 0) {		
		ntfs_error(vi->i_sb,"Couldn't make space for attribute.");
		goto put_err_out;
	}
	memcpy(nctx->attr, a, le32_to_cpu(a->length));
	nctx->attr->instance = nctx->mrec->next_attr_instance;
	m->next_attr_instance = cpu_to_le16(
		(le16_to_cpu(m->next_attr_instance) + 1) & 0xffff);
	ntfs_attr_record_resize(m, a, 0);

	/*FIXEME: all the NIoSet things*/
	ntfs_inode_mark_dirty(ctx->ntfs_ino); //NIoSetDirty 
	ntfs_inode_mark_dirty(ni);

	/* Update attribute list. */
	ctx->al_entry->mft_reference =
		MK_LE_MREF(ni->mft_no, le16_to_cpu(m->sequence_number));
	ctx->al_entry->instance = nctx->attr->instance;
	
	/*FIXEME: all the NIoSet things*/
	//ntfs_attrlist_mark_dirty(ni);
	unmap_mft_record(ni);
	ntfs_attr_put_search_ctx(nctx);
	return 0;
put_err_out:
	unmap_mft_record(ni);
	ntfs_attr_put_search_ctx(nctx);	
	return err;
}

/**目前只支持驻留属性
 * ntfs_attr_readall - read the entire data from an ntfs attribute
 * @ni:		open ntfs inode in which the ntfs attribute resides
 * @type:	attribute type
 * @name:	attribute name in little endian Unicode or AT_UNNAMED or NULL
 * @name_len:	length of attribute @name in Unicode characters (if @name given)
 * @data_size:	if non-NULL then store here the data size 
 *
 * This function will read the entire content of an ntfs attribute.
 * If @name is AT_UNNAMED then look specifically for an unnamed attribute.
 * If @name is NULL then the attribute could be either named or not. 
 * In both those cases @name_len is not used at all.
 *
 * On success a buffer is allocated with the content of the attribute 
 * and which needs to be freed when it's not needed anymore. If the
 * @data_size parameter is non-NULL then the data size is set there.
 *
 * On error NULL is returned with errno set to the error code.
 */
void *ntfs_attr_readall(ntfs_inode *ni, const ATTR_TYPE type,
			ntfschar *name, u32 name_len, s64 *data_size)
{
	loff_t i_size;	
	ntfs_inode *base_ni;	
	ntfs_attr_search_ctx *ctx;
	MFT_RECORD *mrec;
	unsigned long flags;
	u32 attr_len;		
	void *data ;
	struct inode * vi;
	int err = 0;

	ntfs_trace("Entering. attribute type = 0x%x, name = %s",type,(char *)name);
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
	
	if (!NInoAttr(ni))
		base_ni = ni;
	else
		base_ni = ni->ext.base_ntfs_ino;
	vi  = VFS_I(base_ni);
	
	/* Map, pin, and lock the mft record. */
	mrec = map_mft_record(base_ni);//MTF记录项
	if (IS_ERR(mrec)) {
		//err = PTR_ERR(mrec);
		goto err_out;
	}
	/*
	 * If a parallel write made the attribute non-resident, drop the mft
	 * record and retry the readpage.
	 如果有操作使得属性非主驻留了，丢弃MFT记录，重新读
	 */
	/*fixme: */
/*	if (unlikely(NInoNonResident(ni))) {
		unmap_mft_record(base_ni);
		ntfs_debug("the attribute 0x%x is non-resident",type);
		goto err_out;
	}
*/
	ctx = ntfs_attr_get_search_ctx(base_ni, mrec); // Allocate a new attribute search context, initialize it with @ni and @mrec,
	
	if (unlikely(!ctx)) {
		//err = -ENOMEM;
		goto unm_err_out;
	}
	
	err = ntfs_attr_lookup(type, name, name_len,//查找驻留属性
			CASE_SENSITIVE, 0, NULL, 0, ctx);
	
	if (unlikely(err)){
		ntfs_debug("didnot find the 0x%x ",type);
		goto put_unm_err_out;
	}
	attr_len = le32_to_cpu(ctx->attr->data.resident.value_length); //AT_BITMAP是驻留的?
	
	
	read_lock_irqsave(&ni->size_lock, flags);//节点大小锁
	if (unlikely(attr_len > ni->initialized_size))
		attr_len = ni->initialized_size;
	i_size = i_size_read(vi);	
	ntfs_trace("attr_len = 0x%x , i_size = 0x%llx ",attr_len,i_size);
	
	read_unlock_irqrestore(&ni->size_lock, flags);
	if (unlikely(attr_len > i_size)) {
		/* Race with shrinking truncate. */
		attr_len = i_size;
	}
	
	data = kcalloc(1,1024,GFP_KERNEL);
	//addr = kmap_atomic(data, KM_USER0);//建立临时映射
	/* Copy the data to the page. */
	memcpy(data, (u8*)ctx->attr +//将驻留属性的值拷贝到page
			le16_to_cpu(ctx->attr->data.resident.value_offset),
			attr_len);
	/* Zero the remainder of the page. */
	memset(data + attr_len, 0, 1024 - attr_len);
	if (data_size)
		*data_size =  attr_len;
	//flush_dcache_page(page);
	//kunmap_atomic(addr, KM_USER0);
	ntfs_attr_put_search_ctx(ctx);
	unmap_mft_record(base_ni);
	ntfs_trace("Done");
	return data;
	
put_unm_err_out:
	ntfs_attr_put_search_ctx(ctx);
unm_err_out:
	unmap_mft_record(base_ni);
err_out:	
	ntfs_debug("Failed");
	return NULL;
}
#if 0 
ssize_t ntfs_attr_write2(ntfs_inode * ni, const ATTR_TYPE type,
		ntfschar *name, u32 name_len,loff_t pos, size_t count, const  unsigned char * buf)
{
	struct inode  *vi = VFS_I(ni);
	ntfs_volume *vol = ni->vol;
	struct address_space *mapping = vi->i_mapping;
	struct page *pages[NTFS_MAX_PAGES_PER_CLUSTER];
	struct page *cached_page = NULL;
	s64 end, ll;
	VCN last_vcn;
	LCN lcn;
	unsigned long flags;
	size_t bytes ;
	ssize_t status = 0, written;
	unsigned nr_pages;
	int err;
	struct pagevec lru_pvec;
	MFT_RECORD *m = NULL;
	ntfs_attr_search_ctx *ctx;
	ATTR_RECORD *a ;
	
	if (NInoMstProtected(ni))
		vi->i_mapping->a_ops = &ntfs_mst_aops;
	else
		vi->i_mapping->a_ops = &ntfs_aops;

	ntfs_debug("Entering for i_ino 0x%lx, attribute type 0x%x, "
			"pos 0x%llx, count 0x%lx.",
			vi->i_ino, (unsigned)le32_to_cpu(type),
			(unsigned long long)pos, (unsigned long)count);
	if (unlikely(!count))
		return 0;
	//DEBUG0_ON(NInoMstProtected(ni));
	/*
	 * If the attribute is not an index root and it is encrypted or
	 * compressed, we cannot write to it yet.  Note we need to check for
	 * AT_INDEX_ALLOCATION since this is the type of both directory and
	 * index inodes.
	 不支持的属性:非index root;加密的;压缩的;
	 首先先检查AT_INDEX_ALLOCATION,因为目录和索引节点都包含
	 */
	
	if (type != AT_INDEX_ALLOCATION) {
		/* If file is encrypted, deny access, just like NT4. */
		if (NInoEncrypted(ni)) {
			/*
			 * Reminder for later: Encrypted files are _always_
			 * non-resident so that the content can always be
			 * encrypted.
			 */
			ntfs_debug("Denying write access to encrypted file.");
			return -EACCES;
		}
		if (NInoCompressed(ni)) {
			/* Only unnamed $DATA attribute can be compressed. */
			DEBUG0_ON(type != AT_DATA);
			DEBUG0_ON(name_len);
			/*
			 * Reminder for later: If resident, the data is not
			 * actually compressed.  Only on the switch to non-
			 * resident does compression kick in.  This is in
			 * contrast to encrypted files (see above).
			 */
			ntfs_error(vi->i_sb, "Writing to compressed files is "
					"not implemented yet.  Sorry.");
			return -EOPNOTSUPP;
		}
	}

	
	m = map_mft_record(ni);
	if (IS_ERR(m)) {
		err = PTR_ERR(m);
		ntfs_error(vi->i_sb, "Failed to map mft record for inode 0x%lx "
				"(error code %d)", vi->i_ino, err);
		ctx = NULL;
		m = NULL;
		goto old_bad_out;
	}
	ctx = ntfs_attr_get_search_ctx(ni, m);
	if (unlikely(!ctx)) {
		ntfs_error(vi->i_sb, "Failed to allocate a search context for "
				"inode 0x%lx (not enough memory)",
				vi->i_ino);
		err = -ENOMEM;
		goto old_bad_out;
	}
	err = ntfs_attr_lookup(type, name, name_len,
			CASE_SENSITIVE, 0, NULL, 0, ctx);
	if (unlikely(err)) {
		if (err == -ENOENT) {
			ntfs_error(vi->i_sb, "Open attribute is missing from "
					"mft record.  Inode 0x%lx is corrupt. "
					"Run chkdsk.", vi->i_ino);
			err = -EIO;
		} else
			ntfs_error(vi->i_sb, "Failed to lookup attribute in "
					"inode 0x%lx (error code %d).",
					vi->i_ino, err);
		goto old_bad_out;
	}
	m = ctx->mrec;
	a = ctx->attr;
	unmap_mft_record(ni);
	ntfs_attr_put_search_ctx(ctx);	
	
	/* The first byte after the write. */
	end = pos + count;
	/*
	 * If the write goes beyond the allocated size, extend the allocation
	 * to cover the whole of the write, rounded up to the nearest cluster.
	 如果写的字节大小超出了节点分配的大小，
	 扩大allocation，但是不改变文件大小?
	 */
	read_lock_irqsave(&ni->size_lock, flags);
	ll = ni->allocated_size;
	read_unlock_irqrestore(&ni->size_lock, flags);
	ntfs_debug("ll = 0x%llx ,end = 0x%llx ",ll,end);
	if (end > ll) {//写大小大于节点大小
		/* Extend the allocation without changing the data size. */
		ll = ntfs_attr_extend_allocation(ni, end, -1, pos);//扩展属性的分配空间
		if (likely(ll >= 0)) {
			DEBUG0_ON(pos >= ll);
			/* If the extension was partial truncate the write. */
			if (end > ll) {
				ntfs_debug("Truncating write to inode 0x%lx, "
						"attribute type 0x%x, because "
						"the allocation was only "
						"partially extended.",
						vi->i_ino, (unsigned)
						le32_to_cpu(type));
				end = ll;
				count = ll - pos;
			}
		} else {
			err = ll;
			read_lock_irqsave(&ni->size_lock, flags);
			ll = ni->allocated_size;
			read_unlock_irqrestore(&ni->size_lock, flags);
			/* Perform a partial write if possible or fail. */
			if (pos < ll) {
				ntfs_debug("Truncating write to inode 0x%lx, "
						"attribute type 0x%x, because "
						"extending the allocation "
						"failed (error code %i).",
						vi->i_ino, (unsigned)
						le32_to_cpu(type), err);
				end = ll;
				count = ll - pos;
			} else {
				ntfs_error(vol->sb, "Cannot perform write to "
						"inode 0x%lx, attribute type "
						"0x%x, because extending the "
						"allocation failed (error "
						"code %i).", vi->i_ino,
						(unsigned)
						le32_to_cpu(type), err);
				return err;
			}
		}
	}
	pagevec_init(&lru_pvec, 0);
	written = 0;
	ntfs_debug("extend allocation ok ");
	/*
	 * If the write starts beyond the initialized size, extend it up to the
	 * beginning of the write and initialize all non-sparse space between
	 * the old initialized size and the new one.  This automatically also
	 * increments the vfs inode->i_size to keep it above or equal to the
	 * initialized_size.
	 */
	read_lock_irqsave(&ni->size_lock, flags);
	ll = ni->initialized_size;
	read_unlock_irqrestore(&ni->size_lock, flags);
	ntfs_debug("pos = 0x%lld ,ni->initialized_size = 0x%llx",pos,ni->initialized_size);
	if (pos > ll) {
		err = ntfs_attr_extend_initialized(ni, pos, &cached_page,//扩展属性的初始化大小
				&lru_pvec);
		if (err < 0) {
			ntfs_error(vol->sb, "Cannot perform write to inode "
					"0x%lx, attribute type 0x%x, because "
					"extending the initialized size "
					"failed (error code %i).", vi->i_ino,
					(unsigned)le32_to_cpu(type), err);
			status = err;
			goto err_out;
		}
	}
	ntfs_debug("extend initialized_size ok ");
	/*
	 * Determine the number of pages per cluster for non-resident
	 * attributes.
	 计算非驻留属性的每簇占的页数
	 */
	 
	nr_pages = 1;
	if (vol->cluster_size > PAGE_CACHE_SIZE && NInoNonResident(ni))
		nr_pages = vol->cluster_size >> PAGE_CACHE_SHIFT;
	/* Finally, perform the actual write. */
	last_vcn = -1;
	do {
		VCN vcn;
		pgoff_t idx, start_idx;
		unsigned ofs, do_pages, u;
		size_t copied;

		start_idx = idx = pos >> PAGE_CACHE_SHIFT;
		ofs = pos & ~PAGE_CACHE_MASK;
		bytes = PAGE_CACHE_SIZE - ofs;
		do_pages = 1;
		ntfs_debug("start_idx= %ld ,ofs = %d ,bytes = %d",start_idx,ofs,bytes);
		if (nr_pages > 1) {
			vcn = pos >> vol->cluster_size_bits;
			if (vcn != last_vcn) {
				last_vcn = vcn;
				/*
				 * Get the lcn of the vcn the write is in.  If
				 * it is a hole, need to lock down all pages in
				 * the cluster.
				 */
				down_read(&ni->runlist.lock);
				lcn = ntfs_attr_vcn_to_lcn_nolock(ni, pos >>
						vol->cluster_size_bits, false);
				up_read(&ni->runlist.lock);
				if (unlikely(lcn < LCN_HOLE)) {
					status = -EIO;
					if (lcn == LCN_ENOMEM)
						status = -ENOMEM;
					else
						ntfs_error(vol->sb, "Cannot "
							"perform write to "
							"inode 0x%lx, "
							"attribute type 0x%x, "
							"because the attribute "
							"is corrupt.",
							vi->i_ino, (unsigned)
							le32_to_cpu(type));
					break;
				}
				if (lcn == LCN_HOLE) {
					start_idx = (pos & ~(s64)
							vol->cluster_size_mask)
							>> PAGE_CACHE_SHIFT;
					bytes = vol->cluster_size - (pos &
							vol->cluster_size_mask);
					do_pages = nr_pages;
				}
			}
		}
		if (bytes > count)
			bytes = count;
		/*
		 * Bring in the user page(s) that we will copy from _first_.
		 * Otherwise there is a nasty deadlock on copying from the same
		 * page(s) as we are writing to, without it/them being marked
		 * up-to-date.  Note, at present there is nothing to stop the
		 * pages being swapped out between us bringing them into memory
		 * and doing the actual copying.
		 */
		ntfs_debug("next");
		ntfs_fault_in_pages_readable(buf, bytes);//j将用户地址buf转换为page结构
		ntfs_debug("next2");
		/* Get and lock @do_pages starting at index @start_idx. */
		status = __ntfs_grab_cache_pages(mapping, start_idx, do_pages,
				pages, &cached_page, &lru_pvec);
		 ntfs_debug("next3");
		if (unlikely(status))
			break;
		/*
		 * For non-resident attributes, we need to fill any holes with
		 * actual clusters and ensure all bufferes are mapped.  We also
		 * need to bring uptodate any buffers that are only partially
		 * being written to.
		 */
		if (NInoNonResident(ni)) {//非驻留属性
			status = ntfs_prepare_pages_for_non_resident_write(
					pages, do_pages, pos, bytes);
			if (unlikely(status)) {
				loff_t i_size;

				do {
					unlock_page(pages[--do_pages]);
					page_cache_release(pages[do_pages]);
				} while (do_pages);
				/*
				 * The write preparation may have instantiated
				 * allocated space outside i_size.  Trim this
				 * off again.  We can ignore any errors in this
				 * case as we will just be waisting a bit of
				 * allocated space, which is not a disaster.
				 */
				i_size = i_size_read(vi);
				if (pos + bytes > i_size)
					vmtruncate(vi, i_size);
				break;
			}
		}

		ntfs_debug("prepare for commit page ok");
		/**驻留属性**/
		u = (pos >> PAGE_CACHE_SHIFT) - pages[0]->index;
		copied = ntfs_copy_from_user(pages + u, do_pages - u,
					ofs, buf, bytes);
		buf += copied;
		
		ntfs_flush_dcache_pages(pages + u, do_pages - u);
		status = ntfs_commit_pages_after_write(pages, do_pages, pos,//提交page后
				bytes);
		if (likely(!status)) {
			written += copied;
			count -= copied;
			pos += copied;
			if (unlikely(copied != bytes))
				status = -EFAULT;
		}
		do {
			unlock_page(pages[--do_pages]);//释放申请的page
			mark_page_accessed(pages[do_pages]);
			page_cache_release(pages[do_pages]);
		} while (do_pages);
		if (unlikely(status))
			break;
		balance_dirty_pages_ratelimited(mapping);
		cond_resched();
	} while (count);
err_out:
	//*ppos = pos;
	if (cached_page)
		page_cache_release(cached_page);
	/* For now, when the user asks for O_SYNC, we actually give O_DSYNC. */
	if (likely(!status)) {
		//if (unlikely((file->f_flags & O_SYNC) || IS_SYNC(vi))) {
		if (!mapping->a_ops->writepage){ //|| !is_sync_kiocb(iocb))
			status = generic_osync_inode(vi, mapping,
					OSYNC_METADATA|OSYNC_DATA);
			ntfs_debug("sync inode to disk.");
		}
  	}
  	
	pagevec_lru_add(&lru_pvec);
	ntfs_debug("Done.  Returning %s (written 0x%lx, status %li).",
			written ? "written" : "status", (unsigned long)written,
			(long)status);
	return written ? written : status;
old_bad_out:
	//fixme 
	if(m)
		unmap_mft_record(ni);
	if(ctx)
		ntfs_attr_put_search_ctx(ctx);	
	return status;
}
#endif
/**
 * ntfs_attr_mst_pwrite - multi sector transfer protected ntfs attribute write
 * @na:		multi sector transfer protected ntfs attribute to write to
 * @pos:	position in the attribute to write to
 * @bk_cnt:	number of mst protected blocks to write
 * @bk_size:	size of each mst protected block in bytes
 * @src:	data buffer to write to disk
 *
 * This function will write @bk_cnt blocks of size @bk_size bytes each from
 * data buffer @b to multi sector transfer (mst) protected ntfs attribute @na
 * at position @pos.
 *
 * On success, return the number of successfully written blocks. If this number
 * is lower than @bk_cnt this means that an error was encountered during the
 * write so that the write is partial. 0 means nothing was written (also
 * return 0 when @bk_cnt or @bk_size are 0).
 *
 * On error and nothing has been written, return -1 with errno set
 * appropriately to the return code of ntfs_attr_pwrite(), or to EINVAL in case
 * of invalid arguments.
 *
 * NOTE: We mst protect the data, write it, then mst deprotect it using a quick
 * deprotect algorithm (no checking). This saves us from making a copy before
 * the write and at the same time causes the usn to be incremented in the
 * buffer. This conceptually fits in better with the idea that cached data is
 * always deprotected and protection is performed when the data is actually
 * going to hit the disk and the cache is immediately deprotected again
 * simulating an mst read on the written data. This way cache coherency is
 * achieved.
 */
s64 ntfs_attr_mst_pwrite(ntfs_inode *ni,  const ATTR_TYPE type,
		ntfschar *name, u32 name_len, const s64 pos, s64 bk_cnt,const u32 bk_size, void *src)
{
	s64 written, i;

	ntfs_debug("Entering for inode 0x%llx, attr type 0x%x, pos 0x%llx.",
			(unsigned long long)ni->mft_no, type,
			(long long)pos);
	if (bk_cnt < 0 || bk_size % NTFS_BLOCK_SIZE) {
		//errno = EINVAL;
		return -1;
	}
	if (!bk_cnt)
		return 0;
	/* Prepare data for writing. */
	for (i = 0; i < bk_cnt; ++i) {
		int err;
		err = pre_write_mst_fixup((NTFS_RECORD*)//应用修正值技术,每次最多bk_size
				((u8*)src + i * bk_size), bk_size);		
		if (unlikely(err)) {
			if (!err || err == -ENOMEM)
				err = -EIO;
			ntfs_error(ni->vol->sb, "Failed to apply mst fixups "
					"(inode 0x%lx, attribute type 0x%x, "				
					"  Unmount and run chkdsk.", VFS_I(ni)->i_ino,
					AT_INDEX_ALLOCATION);
			if (!i)
				return err;
			bk_cnt = i;
			break;
		}	
	}
	/* Write the prepared data. */
	written = ntfs_attr_write(ni,type,name,name_len, pos, bk_cnt * bk_size, src);//将修正好了的数据写入磁盘
	if (written <= 0) {
		ntfs_error(ni->vol->sb,"written=%lld",(long long)written);
	}
	/* Quickly deprotect the data again. */
	for (i = 0; i < bk_cnt; ++i)
		post_write_mst_fixup((NTFS_RECORD*)((u8*)src + i * //写完后，对数据解除修正值技术
				bk_size));
	if (written <= 0)
		return written;
	/* Finally, return the number of complete blocks written. */
	return div_s64(written,bk_size);	//返回成功写回的块数
}
/**
 * ntfs_attr_position - find given or next attribute type in an ntfs inode
 * @type:	attribute type to start lookup
 * @ctx:	search context with mft record and attribute to search from
 *
 * Find an attribute type in an ntfs inode or the next attribute which is not
 * the AT_END attribute. Please see more details at ntfs_attr_lookup.
 *
 * Return 0 if the search was successful and -1 if not, with errno set to the
 * error code.
 *
 * The following error codes are defined:
 *	EINVAL	Invalid arguments.
 *	EIO	I/O error or corrupt data structures found.
 *	ENOMEM	Not enough memory to allocate necessary buffers.
 * 	ENOSPC  No attribute was found after 'type', only AT_END.
 */
int ntfs_attr_position(const ATTR_TYPE type, ntfs_attr_search_ctx *ctx)
{	
	int err = 0;
	err = ntfs_attr_lookup(type, NULL, 0, CASE_SENSITIVE, 0, NULL, 0, ctx);
	if (err != 0) {
		if (err != -ENOENT)
			return -EIO;
		if (ctx->attr->type == AT_END) {		
			return -ENOSPC;
		}
	}
	return 0;
}
/**返回0，表示属性存在*/
int ntfs_attr_exist(ntfs_inode *ni, const ATTR_TYPE type, ntfschar *name,
		    u32 name_len)
{
	ntfs_attr_search_ctx *ctx;
	int ret;
	MFT_RECORD *mrec;
	
	//ntfs_debug("Entering");
	
	mrec  = map_mft_record(ni);
	if (IS_ERR(mrec)) {
		ret = PTR_ERR(mrec);
		ntfs_error(VFS_I(ni)->i_sb, "Failed to map mft record for inode 0x%lx "
				"(error code %d).", VFS_I(ni)->i_ino, ret);
		
		return -1;;
	}
	ctx = ntfs_attr_get_search_ctx(ni, mrec);
	if (!ctx){
		unmap_mft_record(ni);
		return -1;
	}
	
	ret = ntfs_attr_lookup(type, name, name_len, CASE_SENSITIVE, 0, NULL, 0,
			       ctx);

	unmap_mft_record(ni);
	ntfs_attr_put_search_ctx(ctx);
	
	return ret;
}

int ntfs_attr_truncate_i(ntfs_inode *ni, const ATTR_TYPE type,
		ntfschar *name, u32 name_len,s64 new_size)
{	
	s64 old_size, nr_freed, new_alloc_size, old_alloc_size;
	VCN highest_vcn;
	unsigned long flags;
	ntfs_inode *base_ni;
	ntfs_volume *vol = ni->vol;
	ntfs_attr_search_ctx *ctx;
	MFT_RECORD *m;
	ATTR_RECORD *a;
	const char *te = "  Leaving file length out of sync with i_size.";
	int err, mp_size, size_change, alloc_change;
	u32 attr_len;
	struct inode * vi = VFS_I(ni) ;
	DEBUG0_ON(NInoAttr(ni));//is fake inode
	//DEBUG0_ON(S_ISDIR(vi->i_mode));
	//DEBUG0_ON(NInoMstProtected(ni));  目录节点为真
	DEBUG0_ON(ni->nr_extents < 0);

	ntfs_trace("Entering: type= 0x%x, new_size=0x%llx",type,new_size);
retry_truncate:
	/*
	 * Lock the runlist for writing and map the mft record to ensure it is
	 * safe to mess with the attribute runlist and sizes.
	 */
	if(ni->mft_no == 0)
		 ntfs_debug("down_write");
	down_write(&ni->runlist.lock);
	if (!NInoAttr(ni))
		base_ni = ni;
	else
		base_ni = ni->ext.base_ntfs_ino;
	
	m = map_mft_record(base_ni);
	if (IS_ERR(m)) {
		err = PTR_ERR(m);
		ntfs_error(vi->i_sb, "Failed to map mft record for inode 0x%lx "
				"(error code %d).%s", vi->i_ino, err, te);
		ctx = NULL;
		m = NULL;
		goto old_bad_out;
	}
	ctx = ntfs_attr_get_search_ctx(base_ni, m);
	if (unlikely(!ctx)) {
		ntfs_error(vi->i_sb, "Failed to allocate a search context for "
				"inode 0x%lx (not enough memory).%s",
				vi->i_ino, te);
		err = -ENOMEM;
		goto old_bad_out;
	}
	err = ntfs_attr_lookup(type, name, name_len,//找到目的type
			CASE_SENSITIVE, 0, NULL, 0, ctx);
	if (unlikely(err)) {
		if (err == -ENOENT) {
			ntfs_error(vi->i_sb, "Open attribute is missing from "
					"mft record.  Inode 0x%lx is corrupt.  "
					"Run chkdsk.%s", vi->i_ino, te);
			err = -EIO;
		} else
			ntfs_error(vi->i_sb, "Failed to lookup attribute in "
					"inode 0x%lx (error code %d).%s",
					vi->i_ino, err, te);
		goto old_bad_out;
	}
	m = ctx->mrec;
	a = ctx->attr;

	//新大小: newsize
	/* The current size of the attribute value is the old size. */
	old_size = ntfs_attr_size(a);		//旧大小
	/* Calculate the new allocated size. */
	if (NInoNonResident(ni))
		new_alloc_size = (new_size + vol->cluster_size - 1) & //非驻留属性新分配大小
				~(s64)vol->cluster_size_mask;
	else
		new_alloc_size = (new_size + 7) & ~7;	//驻留属性新分配大小
	/* The current allocated size is the old allocated size. */
	read_lock_irqsave(&ni->size_lock, flags);
	old_alloc_size = ni->allocated_size;		//旧分配大小
	ntfs_trace("old_alloc_size : 0x%llx ,new_alloc_size :0x%llx",old_alloc_size,new_alloc_size);
	read_unlock_irqrestore(&ni->size_lock, flags);
	/*
	 * The change in the file size.  This will be 0 if no change, >0 if the
	 * size is growing, and <0 if the size is shrinking.
	 */
	 ntfs_trace("new_size :0x%llx, old_size: 0x%llx ",new_size,old_size);
	size_change = -1;	//缩小
	if (new_size - old_size >= 0) {
		size_change = 1;	//增大
		if (new_size == old_size)
			size_change = 0;	//没变
	}
	/* As above for the allocated size. */
	alloc_change = -1;	//缩小
	if (new_alloc_size - old_alloc_size >= 0) {
		alloc_change = 1;	//增大
		if (new_alloc_size == old_alloc_size)
			alloc_change = 0;	//没变
	}
	/*
	 * If neither the size nor the allocation are being changed there is
	 * nothing to do.
	 */
	if (!size_change && !alloc_change)
		goto unm_done;
	/* If the size is changing, check if new size is allowed in $AttrDef. */
	if (size_change) {
		err = ntfs_attr_size_bounds_check(vol, type, new_size);//检测newsize的合法性
		if (unlikely(err)) {
			if (err == -ERANGE) {
				ntfs_error(vol->sb, "Truncate would cause the "
						"inode 0x%lx to %simum size "
						"for its attribute type "
						"(0x%x).  Aborting truncate.",
						vi->i_ino,
						new_size > old_size ? "exceed "
						"the max" : "go under the min",
						le32_to_cpu(type));
				err = -EFBIG;
			} else {
				ntfs_error(vol->sb, "Inode 0x%lx has unknown "
						"attribute type 0x%x.  "
						"Aborting truncate.",
						vi->i_ino,
						le32_to_cpu(type));
				err = -EIO;
			}
			/* Reset the vfs inode size to the old size. */
			i_size_write(vi, old_size);	//改变vfs_inode大小??
			goto err_out;
		}
	}
	if (NInoCompressed(ni) || NInoEncrypted(ni)) {
		ntfs_warning(vi->i_sb, "Changes in inode size are not "
				"supported yet for %s files, ignoring.",
				NInoCompressed(ni) ? "compressed" :
				"encrypted");
		err = -EOPNOTSUPP;
		goto bad_out;
	}
	if (a->non_resident)
		goto do_non_resident_truncate;
	//DEBUG0_ON(NInoNonResident(ni));

//	ntfs_debug("next1");
	/* Resize the attribute record to best fit the new attribute size. */
	if (new_size < vol->mft_record_size ){
	//	ntfs_debug("next2");
		err = ntfs_resident_attr_value_resize(m, a, new_size);//改变mft record中属性记录的大小
		if(!err) {
			/* The resize succeeded! */
			flush_dcache_mft_record_page(ctx->ntfs_ino);//把包含mft record的页同步回内存。
			mark_mft_record_dirty(ctx->ntfs_ino);//将mft record和相关的page标记为脏
			
			/* Update the sizes in the ntfs inode and all is done. */
			if(!NInoNonResident(ni)){
			ntfs_trace("change the ni->allocated_size ");
			write_lock_irqsave(&ni->size_lock, flags);//给ntfs inode上锁
			ni->allocated_size = le32_to_cpu(a->length) -
					le16_to_cpu(a->data.resident.value_offset);
			/*
			 * Note ntfs_resident_attr_value_resize() has already done any
			 * necessary data clearing in the attribute record.  When the
			 * file is being shrunk vmtruncate() will already have cleared
			 * the top part of the last partial page, i.e. since this is
			 * the resident case this is the page with index 0.  However,
			 * when the file is being expanded, the page cache page data
			 * between the old data_size, i.e. old_size, and the new_size
			 * has not been zeroed.  Fortunately, we do not need to zero it
			 * either since on one hand it will either already be zero due
			 * to both readpage and writepage clearing partial page data
			 * beyond i_size in which case there is nothing to do or in the
			 * case of the file being mmap()ped at the same time, POSIX
			 * specifies that the behaviour is unspecified thus we do not
			 * have to do anything.  This means that in our implementation
			 * in the rare case that the file is mmap()ped and a write
			 * occured into the mmap()ped region just beyond the file size
			 * and writepage has not yet been called to write out the page
			 * (which would clear the area beyond the file size) and we now
			 * extend the file size to incorporate this dirty region
			 * outside the file size, a write of the page would result in
			 * this data being written to disk instead of being cleared.
			 * Given both POSIX and the Linux mmap(2) man page specify that
			 * this corner case is undefined, we choose to leave it like
			 * that as this is much simpler for us as we cannot lock the
			 * relevant page now since we are holding too many ntfs locks
			 * which would result in a lock reversal deadlock.
			 */
			ni->initialized_size = new_size;
			write_unlock_irqrestore(&ni->size_lock, flags);
			}
			goto unm_done;
		}
		else if(err == STATUS_RESIDENT_ATTRIBUTE_FILLED_MFT) //modify by yangli
			goto unm_done;
	}
	/* If the above resize failed, this must be an attribute extension. */
	//上面失败了，证明是扩展属性!
	ntfs_debug("next3");
	DEBUG0_ON(size_change < 0);
	/*
	 * We have to drop all the locks so we can call
	 * ntfs_attr_make_non_resident().  This could be optimised by try-
	 * locking the first page cache page and only if that fails dropping
	 * the locks, locking the page, and redoing all the locking and
	 * lookups.  While this would be a huge optimisation, it is not worth
	 * it as this is definitely a slow code path as it only ever can happen
	 * once for any given file.
	 */
	ntfs_attr_put_search_ctx(ctx);
	unmap_mft_record(base_ni);
	 if(ni->mft_no == 0)
		ntfs_debug("up_write");

	up_write(&ni->runlist.lock);
	/*
	 * Not enough space in the mft record, try to make the attribute
	 * non-resident and if successful restart the truncation process.
	 */
	err = ntfs_attr_make_non_resident2(ni,type,name,name_len, old_size);//将驻留属性转为非驻留
	if (likely(!err))
		goto retry_truncate;
	/*
	 * Could not make non-resident.  If this is due to this not being
	 * permitted for this attribute type or there not being enough space,
	 * try to make other attributes non-resident.  Otherwise fail.
	 */
	if (unlikely(err != -EPERM && err != -ENOSPC)) {
		ntfs_error(vol->sb, "Cannot truncate inode 0x%lx, attribute "
				"type 0x%x, because the conversion from "
				"resident to non-resident attribute failed "
				"with error code %i.", vi->i_ino,
				(unsigned)le32_to_cpu(type), err);
		if (err != -ENOMEM)
			err = -EIO;
		goto conv_err_out;
	}
	/* TODO: Not implemented from here, abort. */
	if (err == -ENOSPC)
		ntfs_error(vol->sb, "Not enough space in the mft record/on "
				"disk for the non-resident attribute value.  "
				"This case is not implemented yet.");
	else /* if (err == -EPERM) */
		ntfs_error(vol->sb, "This attribute type may not be "
				"non-resident.  This case is not implemented "
				"yet.");
	err = -EOPNOTSUPP;
	goto conv_err_out;
#if 0 //(by kernel author)
	//以下内容在ntfs-3g中已经实现了，请参考
	// TODO: Attempt to make other attributes non-resident.
	if (!err)
		goto do_resident_extend;
	/*
	 * Both the attribute list attribute and the standard information
	 * attribute must remain in the base inode.  Thus, if this is one of
	 * these attributes, we have to try to move other attributes out into
	 * extent mft records instead.
	 */
	if (ni->type == AT_ATTRIBUTE_LIST ||
			ni->type == AT_STANDARD_INFORMATION) {
		// TODO: Attempt to move other attributes into extent mft
		// records.
		err = -EOPNOTSUPP;
		if (!err)
			goto do_resident_extend;
		goto err_out;
	}
	// TODO: Attempt to move this attribute to an extent mft record, but
	// only if it is not already the only attribute in an mft record in
	// which case there would be nothing to gain.
	err = -EOPNOTSUPP;
	if (!err)
		goto do_resident_extend;
	/* There is nothing we can do to make enough space. )-: */
	goto err_out;
#endif
do_non_resident_truncate:
	DEBUG0_ON(!NInoNonResident(ni));
	if (alloc_change < 0) {//allocation size 变小了
		highest_vcn = sle64_to_cpu(a->data.non_resident.highest_vcn);
		if (highest_vcn > 0 &&
				old_alloc_size >> vol->cluster_size_bits >
				highest_vcn + 1) {
			/*
			 * This attribute has multiple extents.  Not yet
			 * supported.
			 */
			ntfs_error(vol->sb, "Cannot truncate inode 0x%lx, "
					"attribute type 0x%x, because the "
					"attribute is highly fragmented (it "
					"consists of multiple extents) and "
					"this case is not implemented yet.",
					vi->i_ino,
					(unsigned)le32_to_cpu(type));
			err = -EOPNOTSUPP;
			goto bad_out;
		}
	}
	/*
	 * If the size is shrinking, need to reduce the initialized_size and
	 * the data_size before reducing the allocation.
	 文件大小变小时，allocation只能变小或不变
	 */
	if (size_change < 0) {//size 变小了,需要缩减initialized_size和data_size
		/*
		 * Make the valid size smaller (i_size is already up-to-date).
		 */
		write_lock_irqsave(&ni->size_lock, flags);
		if (new_size < ni->initialized_size) {
			ni->initialized_size = new_size;	//更新属性的大小
			a->data.non_resident.initialized_size =
					cpu_to_sle64(new_size);
		}
		a->data.non_resident.data_size = cpu_to_sle64(new_size);
		write_unlock_irqrestore(&ni->size_lock, flags);
		flush_dcache_mft_record_page(ctx->ntfs_ino);
		mark_mft_record_dirty(ctx->ntfs_ino);
		/* If the allocated size is not changing, we are done. */
		if (!alloc_change)//此时如果allocation size 不要变化，则done
			goto unm_done;
		/*
		 * If the size is shrinking it makes no sense for the
		 * allocation to be growing.
		 */
		DEBUG0_ON(alloc_change > 0);//此时不允许allocation size 增大
	} else /* if (size_change >= 0) */ {
		/*
		 * The file size is growing or staying the same but the
		 * allocation can be shrinking, growing or staying the same.
		 文件大小变大或不变时，allocation size可以任意变化
		 */
		if (alloc_change > 0) {
			/*
			 * We need to extend the allocation and possibly update
			 * the data size.  If we are updating the data size,
			 * since we are not touching the initialized_size we do
			 * not need to worry about the actual data on disk.
			 * And as far as the page cache is concerned, there
			 * will be no pages beyond the old data size and any
			 * partial region in the last page between the old and
			 * new data size (or the end of the page if the new
			 * data size is outside the page) does not need to be
			 * modified as explained above for the resident
			 * attribute truncate case.  To do this, we simply drop
			 * the locks we hold and leave all the work to our
			 * friendly helper ntfs_attr_extend_allocation().
			 */
			ntfs_attr_put_search_ctx(ctx);
			unmap_mft_record(base_ni);
			 if(ni->mft_no == 0)
				ntfs_debug("up_write");
			up_write(&ni->runlist.lock);
			err = ntfs_attr_extend_allocation(ni, new_size,//扩展属性的分配空间
					size_change > 0 ? new_size : -1, -1);
			/*
			 * ntfs_attr_extend_allocation() will have done error
			 * output already.
			 */
			goto done;
		}
		if (!alloc_change)
			goto alloc_done;
	}
	
	/* alloc_change < 0 */  //包括file size 变大或变小两种情况
	/* Free the clusters. */
	nr_freed = ntfs_cluster_free(ni, new_alloc_size >>//释放多余的簇
			vol->cluster_size_bits, -1, ctx);
	m = ctx->mrec;
	a = ctx->attr;
	if (unlikely(nr_freed < 0)) {
		ntfs_error(vol->sb, "Failed to release cluster(s) (error code "
				"%lli).  Unmount and run chkdsk to recover "
				"the lost cluster(s).", (long long)nr_freed);
		NVolSetErrors(vol);
		nr_freed = 0;
	}
	/* Truncate the runlist. */
	err = ntfs_rl_truncate_nolock(vol, &ni->runlist,//改变runlist的长度
			new_alloc_size >> vol->cluster_size_bits);
	/*
	 * If the runlist truncation failed and/or the search context is no
	 * longer valid, we cannot resize the attribute record or build the
	 * mapping pairs array thus we mark the inode bad so that no access to
	 * the freed clusters can happen.
	 */
	if (unlikely(err || IS_ERR(m))) {
		ntfs_error(vol->sb, "Failed to %s (error code %li).%s",
				IS_ERR(m) ?
				"restore attribute search context" :
				"truncate attribute runlist",
				IS_ERR(m) ? PTR_ERR(m) : err, es);
		err = -EIO;
		goto bad_out;
	}
	/* Get the size for the shrunk mapping pairs array for the runlist. */
	mp_size = ntfs_get_size_for_mapping_pairs(vol, ni->runlist.rl, 0, -1);
	if (unlikely(mp_size <= 0)) {
		ntfs_error(vol->sb, "Cannot shrink allocation of inode 0x%lx, "
				"attribute type 0x%x, because determining the "
				"size for the mapping pairs failed with error "
				"code %i.%s", vi->i_ino,
				(unsigned)le32_to_cpu(type), mp_size, es);
		err = -EIO;
		goto bad_out;
	}
	/*
	 * Shrink the attribute record for the new mapping pairs array.  Note,
	 * this cannot fail since we are making the attribute smaller thus by
	 * definition there is enough space to do so.
	 */
	attr_len = le32_to_cpu(a->length);
	err = ntfs_attr_record_resize(m, a, mp_size +	//改变属性记录的大小
			le16_to_cpu(a->data.non_resident.mapping_pairs_offset));
	DEBUG0_ON(err);
	/*
	 * Generate the mapping pairs array directly into the attribute record.
	 */
	err = ntfs_mapping_pairs_build(vol, (u8*)a +	//生成新的mapping pairs
			le16_to_cpu(a->data.non_resident.mapping_pairs_offset),
			mp_size, ni->runlist.rl, 0, -1, NULL);
	if (unlikely(err)) {
		ntfs_error(vol->sb, "Cannot shrink allocation of inode 0x%lx, "
				"attribute type 0x%x, because building the "
				"mapping pairs failed with error code %i.%s",
				vi->i_ino, (unsigned)le32_to_cpu(type),
				err, es);
		err = -EIO;
		goto bad_out;
	}
	/* Update the allocated/compressed size as well as the highest vcn. */
	//更新属性大小
	a->data.non_resident.highest_vcn = cpu_to_sle64((new_alloc_size >>
			vol->cluster_size_bits) - 1);
	write_lock_irqsave(&ni->size_lock, flags);
	ni->allocated_size = new_alloc_size;
	a->data.non_resident.allocated_size = cpu_to_sle64(new_alloc_size);
	if (NInoSparse(ni) || NInoCompressed(ni)) {
		if (nr_freed) {
			ni->itype.compressed.size -= nr_freed <<
					vol->cluster_size_bits;
			DEBUG0_ON(ni->itype.compressed.size < 0);
			a->data.non_resident.compressed_size = cpu_to_sle64(
					ni->itype.compressed.size);
			vi->i_blocks = ni->itype.compressed.size >> 9;
		}
	} else
		vi->i_blocks = new_alloc_size >> 9;
	write_unlock_irqrestore(&ni->size_lock, flags);
	/*
	 * We have shrunk the allocation.  If this is a shrinking truncate we
	 * have already dealt with the initialized_size and the data_size above
	 * and we are done.  If the truncate is only changing the allocation
	 * and not the data_size, we are also done.  If this is an extending
	 * truncate, need to extend the data_size now which is ensured by the
	 * fact that @size_change is positive.
	 */
alloc_done:
	/*
	 * If the size is growing, need to update it now.  If it is shrinking,
	 * we have already updated it above (before the allocation change).
	 */
	if (size_change > 0)
		a->data.non_resident.data_size = cpu_to_sle64(new_size);
	/* Ensure the modified mft record is written out. */
	flush_dcache_mft_record_page(ctx->ntfs_ino);
	mark_mft_record_dirty(ctx->ntfs_ino);
unm_done:
	ntfs_attr_put_search_ctx(ctx);
	unmap_mft_record(base_ni);
	 if(ni->mft_no == 0)
		ntfs_debug("up_write");

	up_write(&ni->runlist.lock);
done:
	/* Update the mtime and ctime on the base inode. */
	/* normally ->truncate shouldn't update ctime or mtime,
	 * but ntfs did before so it got a copy & paste version
	 * of file_update_time.  one day someone should fix this
	 * for real.
	 */
	if (!IS_NOCMTIME(VFS_I(base_ni)) && !IS_RDONLY(VFS_I(base_ni))) {
		struct timespec now = current_fs_time(VFS_I(base_ni)->i_sb);
		int sync_it = 0;

		if (!timespec_equal(&VFS_I(base_ni)->i_mtime, &now) ||
		    !timespec_equal(&VFS_I(base_ni)->i_ctime, &now))
			sync_it = 1;
		VFS_I(base_ni)->i_mtime = now;
		VFS_I(base_ni)->i_ctime = now;

		if (sync_it)
			mark_inode_dirty_sync(VFS_I(base_ni));
	}

	if (likely(!err)) {
		NInoClearTruncateFailed(ni);
		ntfs_debug("Done.");
	}
	if(err  == STATUS_RESIDENT_ATTRIBUTE_FILLED_MFT){
		NInoClearTruncateFailed(ni);
		ntfs_debug("Index_root needs to be Index_allocation.");
	}
		
	return err;
old_bad_out:
	old_size = -1;
bad_out:
	if (err != -ENOMEM && err != -EOPNOTSUPP)
		NVolSetErrors(vol);
	if (err != -EOPNOTSUPP)
		NInoSetTruncateFailed(ni);//设置truncate失败标志，外部函数根据该标志会再一次调用该函数
	else if (old_size >= 0)
		i_size_write(vi, old_size);
err_out:
	if (ctx)
		ntfs_attr_put_search_ctx(ctx);
	if (m)
		unmap_mft_record(base_ni);
	 if(ni->mft_no == 0)
		ntfs_debug("up_write");

	up_write(&ni->runlist.lock);
out:
	ntfs_debug("Failed.  Returning error code %i.", err);
	return err;
conv_err_out:
	if (err != -ENOMEM && err != -EOPNOTSUPP)
		NVolSetErrors(vol);
	if (err != -EOPNOTSUPP)
		NInoSetTruncateFailed(ni);
	else
		i_size_write(vi, old_size);
	goto out;
}

/**改变ni对应的mft record中属性type的大小，而不是ni->type的大小*/
int ntfs_attr_truncate(ntfs_inode *ni,const ATTR_TYPE type,
		ntfschar *name, u32 name_len,s64 new_size)
{
	int err = 0;
	struct inode * vi =VFS_I(ni);
	if(ni->mft_no == 0)
		 ntfs_debug("down_write");
	down_write(&vi->i_alloc_sem); //读写锁!
	err = ntfs_attr_truncate_i(ni,type,name,name_len,new_size);
	 if(ni->mft_no == 0)
		ntfs_debug("up_write");

	up_write(&vi->i_alloc_sem);
	return err;
}



/**为ntfs inode @ni 添加驻留属性
 * ntfs_resident_attr_record_add - add resident attribute to inode
 * @ni:		opened ntfs inode to which MFT record add attribute
 * @type:	type of the new attribute
 * @name:	name of the new attribute
 * @name_len:	name length of the new attribute
 * @val:	value of the new attribute
 * @size:	size of new attribute (length of @val, if @val != NULL)
 * @flags:	flags of the new attribute
 *
 * Return offset to attribute from the beginning of the mft record on success
 * and -1 on error. On error the error code is stored in errno.
 * Possible error codes are:
 *	EINVAL	- Invalid arguments passed to function.
 *	EEXIST	- Attribute of such type and with same name already exists.
 *	EIO	- I/O error occurred or damaged filesystem.
 */
int ntfs_resident_attr_record_add(ntfs_inode *ni, ATTR_TYPE type,
			ntfschar *name, u8 name_len, u8 *val, u32 size,
			ATTR_FLAGS data_flags)
{
	ntfs_attr_search_ctx *ctx;
	u32 length;
	ATTR_RECORD *a;
	MFT_RECORD *m;
	MFT_RECORD *mrec = NULL;
	int err, offset;
	ntfs_inode *base_ni;
	char *kattr;
	struct inode *vi = VFS_I(ni);
	unsigned long flags;
	
	ntfs_trace("Entering for inode: 0x%llx, attr: 0x%x, flags: 0x%x",
		(long long) ni->mft_no, (unsigned) type, (unsigned) data_flags);
/*
	FIXME:压缩、稀疏、加密?
	if(data_flags & ATTR_IS_COMPRESSED)
		ntfs_error(vi->i_sb,"sorry,compression is not supported yet ");
*/
	
	if (!ni || (!name && name_len)) {
		return -EINVAL;
	}
	err = ntfs_attr_can_be_resident(ni->vol, type);
	if (err != 0) {
		if (err == -EPERM)
			ntfs_debug("Attribute can't be resident.");
		else
			ntfs_debug("ntfs_attr_can_be_resident failed.");
		return -EPERM;
	}

	mrec = map_mft_record(ni);
	if (IS_ERR(mrec)) {
		err = PTR_ERR(mrec);
		ntfs_error(vi->i_sb, "Failed to map mft record for inode 0x%lx "
				"(error code %d).", vi->i_ino, err);
		return err;
	}
	/* Locate place where record should be. */
	ctx = ntfs_attr_get_search_ctx(ni, mrec);
	if (!ctx){
		unmap_mft_record(ni);
		return -1;
	}
	/*
	 * Use ntfs_attr_find instead of ntfs_attr_lookup to find place for
	 * attribute in @ni->mrec, not any extent inode in case if @ni is base
	 * file record.
	 */
	 err = ntfs_attr_find(type, name, name_len, CASE_SENSITIVE, val, size,
			ctx);
	if (err  == 0) { /*find the attribute*/
		err  = -EEXIST;
		ntfs_debug("Attribute already present.");
		goto put_err_out;
	}
	else if (err != -ENOENT) {
		err = -EIO;	/*oh, unfortunately error occurs*/
		ntfs_debug("ntfs_attr_find err ");
		goto put_err_out;
	}
	/*err = -ENOENT*/
	ntfs_trace("find the right place to add the attribute");
	a = ctx->attr;
	m = ctx->mrec;

	/* Make room for attribute. */
	length = offsetof(ATTR_RECORD, data.resident.resident_end) +/*the length of the resident attribute*/
				((name_len * sizeof(ntfschar) + 7) & ~7) +
				((size + 7) & ~7);
	if ((ntfs_make_room_for_attr(ctx->mrec, (u8*) ctx->attr, length))<0) {
		ntfs_debug("Failed to make room for attribute.");
		goto put_err_out;
	}

	/* Setup record fields. */
	offset = ((u8*)a - (u8*)m);		/*the offset of the new attribute in mft record*/
	a->type = type;
	a->length = cpu_to_le32(length);
	a->non_resident = 0;
	a->name_length = name_len;	/*the length of the new attribute name*/
	a->name_offset = (name_len	/*the offset of the new name , just follow data.resident.resident_end*/
		? cpu_to_le16(offsetof(ATTR_RECORD,  data.resident.resident_end))
		: const_cpu_to_le16(0));
	a->flags = data_flags;			/*such as: compressed ,sparse ....*/
	a->instance = m->next_attr_instance;
	a->data.resident.value_length = cpu_to_le32(size);
	a->data.resident.value_offset = cpu_to_le16(length - ((size + 7) & ~7));

	/* Copy the received data to the mft record. */
	kattr = (u8*)a + le16_to_cpu(a->data.resident.value_offset);	
	if (val)		
		memcpy(kattr, val, size);	
	else
		memset(kattr, 0, size);
	
	if (type == AT_FILE_NAME)
		a->data.resident.flags = RESIDENT_ATTR_IS_INDEXED;
	else
		a->data.resident.flags = 0;
	
	/* Copy the received @name to the mft record. */
	if (name_len){
		kattr = (u8*)a + le16_to_cpu(a->name_offset);					
		memcpy(kattr, name,  sizeof(ntfschar) * name_len);	
	}
	m->next_attr_instance =
		cpu_to_le16((le16_to_cpu(m->next_attr_instance) + 1) & 0xffff);
	if (!NInoAttr(ni))	
		base_ni = ni;
	else
		base_ni = ni->ext.base_ntfs_ino;	;	/*the base ntfs inode*/

	/*if type is not AT_ATTRIBUTE_LIST and the base inode has the 0X20 attribute list */
	if (type != AT_ATTRIBUTE_LIST && NInoAttrList(base_ni)) {
		if ((err = ntfs_attrlist_entry_add(ni, a)) != 0) {/*add the new attribute to the 0x20 attribute list*/
			ntfs_attr_record_resize(m, a, 0);
			ntfs_debug("Failed add attribute entry to "
					"ATTRIBUTE_LIST.");
			goto put_err_out;
		}
		ntfs_debug("add the new attribute to the 0x20 attribute list ok ");
	}

	
	if (mrec->flags & MFT_RECORD_IS_DIRECTORY
	    ? type == AT_INDEX_ROOT && name == I30
	    : type == AT_DATA && name == AT_UNNAMED) {
		write_lock_irqsave(&ni->size_lock, flags);		/*get the wirte lock*/
		ni->initialized_size= size;
		ni->allocated_size = (size + 7) & ~7;
		write_unlock_irqrestore(&ni->size_lock, flags);	/*put the wirte lock*/
		//set_nino_flag(ni,KnownSize);		/**FIXEME!*/?
	}
	/* Finally, mark the mft record dirty, so it gets written back. */
	flush_dcache_mft_record_page(ctx->ntfs_ino);
	mark_mft_record_dirty(ctx->ntfs_ino);
	
	/**FIXEME: all the NIoSet things */
	//ntfs_inode_mark_dirty(ni);
	
	ntfs_attr_put_search_ctx(ctx);
	unmap_mft_record(ni);
	ntfs_trace("Done,offset is %d",offset);
	return offset;
put_err_out:
	ntfs_attr_put_search_ctx(ctx);
	unmap_mft_record(ni);
	ntfs_debug("Failed.");
	//errno = err;
	return -1;
}

/**为ntfs inode @ni 添加非驻留属性
 * ntfs_non_resident_attr_record_add - add extent of non-resident attribute
 * @ni:			opened ntfs inode to which MFT record add attribute
 * @type:		type of the new attribute extent
 * @name:		name of the new attribute extent
 * @name_len:		name length of the new attribute extent
 * @lowest_vcn:		lowest vcn of the new attribute extent
 * @dataruns_size:	dataruns size of the new attribute extent
 * @flags:		flags of the new attribute extent
 *
 * Return offset to attribute from the beginning of the mft record on success
 * and -1 on error. On error the error code is stored in errno.
 * Possible error codes are:
 *	EINVAL	- Invalid arguments passed to function.
 *	EEXIST	- Attribute of such type, with same lowest vcn and with same
 *		  name already exists.
 *	EIO	- I/O error occurred or damaged filesystem.
 */
int ntfs_non_resident_attr_record_add(ntfs_inode *ni, ATTR_TYPE type,
		ntfschar *name, u8 name_len, VCN lowest_vcn, int dataruns_size,
		ATTR_FLAGS flags)
{
	ntfs_attr_search_ctx *ctx;
	u32 length;
	ATTR_RECORD *a;
	MFT_RECORD *m;
	MFT_RECORD *mrec = NULL;
	ntfs_inode *base_ni;
	int err, offset;
	char *kattr;
	struct inode * vi = VFS_I(ni);

	ntfs_debug("Entering for inode 0x%llx, attr 0x%x, lowest_vcn %lld, "
			"dataruns_size %d, flags 0x%x.",
			(long long) ni->mft_no, (unsigned) type,
			(long long) lowest_vcn, dataruns_size, (unsigned) flags);

	if (!ni || dataruns_size <= 0 || (!name && name_len)) {
		//errno = EINVAL;
		return -1;
	}
	
	err = ntfs_attr_can_be_non_resident(ni->vol, type);
	if (err != 0) {
		if (err == -EPERM)
			ntfs_debug("Attribute can't be non resident.");
		else
			ntfs_debug("ntfs_attr_can_be_non_resident failed.");
		return -1;
	}

	mrec = map_mft_record(ni);
	if (IS_ERR(mrec)) {
		err = PTR_ERR(mrec);
		ntfs_error(vi->i_sb, "Failed to map mft record for inode 0x%lx "
				"(error code %d).", vi->i_ino, err);
		return err;
	}
	/* Locate place where record should be. */
	ctx = ntfs_attr_get_search_ctx(ni, mrec);
	if (!ctx){
		unmap_mft_record(ni);
		return -1;
	}
	/*
	 * Use ntfs_attr_find instead of ntfs_attr_lookup to find place for
	 * attribute in @ni->mrec, not any extent inode in case if @ni is base
	 * file record.
	 */
	 err = ntfs_attr_find(type, name, name_len, CASE_SENSITIVE, NULL, 0,
			ctx);
	if (err == 0) {
		err = -EEXIST;
		ntfs_debug("Attribute 0x%x already present",type);
		goto put_err_out;
	}
	
	if (err != -ENOENT) {
		ntfs_debug("ntfs_attr_find failed.");
		err = -EIO;
		goto put_err_out;
	}
	/*err = -ENOENT*/
	ntfs_trace("find the right place to add the attribute");
	a = ctx->attr;
	m= ctx->mrec;

	/* Make room for attribute. */
	dataruns_size = (dataruns_size + 7) & ~7;/*assignment in  8 bytes*/
	length = offsetof(ATTR_RECORD, data.non_resident.non_resident_end) + ((sizeof(ntfschar) * /*non-resident attribute length*/
			name_len + 7) & ~7) + dataruns_size +
			((flags & (ATTR_IS_COMPRESSED | ATTR_IS_SPARSE)) ?
			sizeof(a->data.non_resident.compressed_size) : 0);
	err = ntfs_make_room_for_attr(ctx->mrec, (u8*) ctx->attr, length);
	if (err != 0 ) {
		ntfs_debug("Failed to make room for attribute.");
		goto put_err_out;
	}

	/* Setup record fields. */
	a->type = type;
	a->length = cpu_to_le32(length);
	a->non_resident = 1;
	a->name_length = name_len;
	a->name_offset = cpu_to_le16(offsetof(ATTR_RECORD,data.non_resident.compressed_size) +
			((flags & (ATTR_IS_COMPRESSED | ATTR_IS_SPARSE)) ?
			sizeof(a->data.non_resident.compressed_size) : 0));
	a->flags = flags;
	a->instance = m->next_attr_instance;
	a->data.non_resident.lowest_vcn = cpu_to_sle64(lowest_vcn);
	a->data.non_resident.mapping_pairs_offset = cpu_to_le16(length - dataruns_size);/*the relations of lcn and vcn ?*/
	a->data.non_resident.compression_unit = (flags & ATTR_IS_COMPRESSED)
			? STANDARD_COMPRESSION_UNIT : 0;
	/* If @lowest_vcn == 0, than setup empty attribute. */
	if (!lowest_vcn) {
		a->data.non_resident.highest_vcn = cpu_to_sle64(-1);
		a->data.non_resident.allocated_size = 0;
		a->data.non_resident.data_size = 0;
		a->data.non_resident.initialized_size = 0;
		/* Set empty mapping pairs. */
		*((u8*)a + le16_to_cpu(a->data.non_resident.mapping_pairs_offset)) = 0;
		ntfs_debug("mapping_pairs_offset : 0x%x ", *((u8*)a + le16_to_cpu(a->data.non_resident.mapping_pairs_offset)) );
	}
	if (name_len){
		kattr = (u8*)a + le16_to_cpu(a->name_offset);	
		memcpy(kattr, name,  sizeof(ntfschar) * name_len); /*copy the @name to mft record */	
	}
	m->next_attr_instance =
		cpu_to_le16((le16_to_cpu(m->next_attr_instance) + 1) & 0xffff);
	if (!NInoAttr(ni))	
		base_ni = ni;
	else
		base_ni = ni->ext.base_ntfs_ino;	;	

	/*if type is not AT_ATTRIBUTE_LIST and the base inode has the 0X20 attribute list */
	if (type != AT_ATTRIBUTE_LIST && NInoAttrList(base_ni)) {
		if (ntfs_attrlist_entry_add(ni, a)) {
			//err = errno;
			ntfs_debug("Failed add attr entry to attrlist"); /*add the new attribute to the 0x20 attribute list*/
			ntfs_attr_record_resize(m, a, 0);
			goto put_err_out;
		}
	}
	/*Fixme!*/
	//ntfs_inode_mark_dirty(ni);
	
	/*
	 * Locate offset from start of the MFT record where new attribute is
	 * placed. We need relookup it, because record maybe moved during
	 * update of attribute list.
	 */
	ntfs_attr_reinit_search_ctx(ctx);
	if (ntfs_attr_lookup(type, name, name_len, CASE_SENSITIVE, /*find the just added attribute*/
					lowest_vcn, NULL, 0, ctx)) {
		ntfs_debug("attribute lookup failed");
		goto put_err_out;
	}
	offset = (u8*)ctx->attr - (u8*)ctx->mrec;/*the offset of the new attribute in the mft record*/

	write_lock_irqsave(&ni->size_lock, flags);		/*get the wirte lock*/
	ni->initialized_size= 0;
	ni->allocated_size = 0;	
	write_unlock_irqrestore(&ni->size_lock, flags);	/*put the wirte lock*/
	
	/* Finally, mark the mft record dirty, so it gets written back. */
	
	flush_dcache_mft_record_page(ctx->ntfs_ino);
	mark_mft_record_dirty(ctx->ntfs_ino);

	/**FIXEME: all the NIoSet things */
	//ntfs_inode_mark_dirty(ni);
	
	ntfs_attr_put_search_ctx(ctx);
	unmap_mft_record(ni);
	NInoSetNonResident(ni);
	ntfs_debug("Done. offset = %d",offset);
	return offset;
put_err_out:
	ntfs_attr_put_search_ctx(ctx);
	unmap_mft_record(ni);
	ntfs_debug("Failed.");
	//errno = err;
	return -1;
}
/**
 * ntfs_attr_add - add attribute to inode
 * @ni:		opened ntfs inode to which add attribute
 * @type:	type of the new attribute
 * @name:	name in unicode of the new attribute
 * @name_len:	name length in unicode characters of the new attribute
 * @val:	value of new attribute 
 * @size:	size of the new attribute / length of @val (if specified)
 *
 * @val should always be specified for always resident attributes (eg. FILE_NAME
 * attribute), for attributes that can become non-resident @val can be NULL
 * (eg. DATA attribute). @size can be specified even if @val is NULL, in this
 * case data size will be equal to @size and initialized size will be equal
 * to 0.
 *
 * If inode haven't got enough space to add attribute, add attribute to one of
 * it extents, if no extents present or no one of them have enough space, than
 * allocate new extent and add attribute to it.
 如果节点空间不够，就将属性添加到其extents inoodes；
 如果无extents inodes 或空间也不够，就创建extent inode后再添加属性
 *
 * If on one of this steps attribute list is needed but not present, than it is
 * added transparently to caller. So, this function should not be called with
 * @type == AT_ATTRIBUTE_LIST, if you really need to add attribute list call
 * ntfs_inode_add_attrlist instead.
 当type为AT_ATTRIBUTE_LIST时，请调用ntfs_inode_add_attrlist
 *
 * On success return 0. On error return -1 with errno set to the error code.
 */
int ntfs_attr_add(ntfs_inode *ni, ATTR_TYPE type,
		ntfschar *name, u8 name_len, u8 *val, s64 size)
{
	u32 attr_rec_size;
	int err, i, offset;
	BOOL is_resident;
	BOOL can_be_non_resident = 0;
	ntfs_inode *attr_ni;	
	ATTR_FLAGS data_flags;
	MFT_RECORD *m = NULL;
	MFT_RECORD *attr_m = NULL;	
	ntfs_volume *vol = ni->vol;
	struct super_block *sb = vol->sb;
	int res = 0;
	
	if (!ni || size < 0 || type == AT_ATTRIBUTE_LIST) {
		ntfs_error(sb,"size=%lld",(long long)size);
		return -EINVAL;
	}

	ntfs_trace("Entering for mft reocrd num 0x%llx, attr 0x%x, size :0x%llx.",
			(long long)ni->mft_no, type, (long long)size);

	if (NInoAttr(ni))	
		ni = ni->ext.base_ntfs_ino;/*get the base ntfs inode*/	

	/* Check the attribute type and the size. */
	if ((ntfs_attr_size_bounds_check(ni->vol, type, size)) !=0) {/*accroding to the system file $attrdef to check the validity of the attribute*/
		ntfs_error(sb,"attribute is invalid,size =%lld",size);
		return -EIO;
	}

	res = ntfs_attr_can_be_non_resident(ni->vol, type);
	/* Sanity checks for always resident attributes. */
	if (res !=0) {
		if(res == -ENOENT){
			ntfs_error(sb,"Fail to find the attirbute in $Attrdef :0x%x",type);
			return -ENOENT;
		}	
		/*find the attirbute, but can not be non-resident */
		/* @val is mandatory. */
		if (!val) {		/*for resident attributes , @val cannot be NULL*/
			ntfs_error(sb,"val is mandatory for always resident "
					"attributes:%d",res);
			return -EINVAL; 
		}
		if (size > ni->vol->mft_record_size) {	/*for resident attributes , the size must be less than the mft record size(usually 1024)*/
			ntfs_error(sb,"Attribute is too big:%d",res);
			return-ERANGE ;
		}
	}
	 else{/*the attribute can be non-resident*/
		can_be_non_resident = 1;
		ntfs_trace("the attribute can be non-resident");
	}
	/*
	 * Determine resident or not will be new attribute. We add 8 to size in
	 * non resident case for mapping pairs.
	 */
	 res =ntfs_attr_can_be_resident(ni->vol, type);
	if (res == 0) {
		is_resident = 1;
		ntfs_trace("the attribute can be resident");
	} else {	
		if(res != -EPERM){
			ntfs_error(sb,"ntfs_attr_can_be_resident failed:%d",res);
			return -EPERM;
		}
		is_resident = 0;
	}
	
	/* Calculate attribute record size. */
	if (is_resident) /*for resident attribute*/
		attr_rec_size = offsetof(ATTR_RECORD, data.resident.resident_end) +
				((name_len * sizeof(ntfschar) + 7) & ~7) +
				((size + 7) & ~7);
	else			/* note: (sizeof(VCN)==8 ))*/
		attr_rec_size = offsetof(ATTR_RECORD, data.non_resident.non_resident_end) +
				((name_len * sizeof(ntfschar) + 7) & ~7) + 8;

	/*
	 * If we have enough free space for the new attribute in the base MFT
	 * record, then add attribute to it.
	case 1:  如果主MFT记录中有足够空间，就添加新属性
	 */	
	m = map_mft_record(ni);
	if (IS_ERR(m)) {
		err = PTR_ERR(m);
		ntfs_error(sb, "Failed to map mft record for inode 0x%lx "
				"(error code %d)", VFS_I(ni)->i_ino, err);		
		return err;
	}
	if (le32_to_cpu(m->bytes_allocated) -
			le32_to_cpu(m->bytes_in_use) >= attr_rec_size) {
		attr_ni = ni;
		unmap_mft_record(ni);
		ntfs_trace("base mft record has enough space");
		goto add_attr_record;
	}
	unmap_mft_record(ni);

	/* Try to add to extent inodes. case 2: 添加到次节点*/
	ntfs_debug("Try to add to extent inodes, extent inode numbers =%d ",ni->nr_extents);	
	res =ntfs_inode_attach_all_extents(ni);  /* attach all extent inodes to @ni */
	if (res != 0) {		
		ntfs_error(sb,"Failed to attach all extents to inode:%d",res);
		goto err_out;
	}
	for (i = 0; i < ni->nr_extents; i++) {
		attr_ni = ni->ext.extent_ntfs_inos[i];
		attr_m= map_mft_record(attr_ni);/*get the mft record for extent inode -->attr_ni */
		if (le32_to_cpu(attr_m->bytes_allocated) -
				le32_to_cpu(attr_m->bytes_in_use) >=
				attr_rec_size){
			unmap_mft_record(attr_ni); /*please remember to release the mft record ,when you donot need it*/
			ntfs_debug("NO[%d] extent mft record has enough space",i);
			goto add_attr_record;
		}
		unmap_mft_record(attr_ni);
	}

	/* There is no extent that contain enough space for new attribute.*/
	if (!NInoAttrList(ni)) {
		/* Add attribute list 0x20 not present, add it and retry. */
		if ((res = (ntfs_inode_add_attrlist(ni))) != 0) {
			ntfs_error(sb,"Failed to add attribute list:%d",res);
			goto err_out;
		}
		ntfs_debug("add attrlist to inode :recursion!");
		return ntfs_attr_add(ni, type, name, name_len, val, size);/* recursion !!*/
	}
	/* Allocate new extent inodes., and get its mft record -->attr_m*/
	attr_ni = ntfs_mft_record_alloc(ni->vol,0, ni,&attr_m); 
	if (IS_ERR(attr_ni)) {
		res = PTR_ERR(attr_ni);	
		ntfs_error(sb,"Failed to allocate extent record,%d",res);
		goto err_out;
	}
	unmap_mft_record(attr_ni); /*donot need the mft record for the moment*/
add_attr_record:
	/**FIXEME: COMPRESSED*/
	/*
	if ((ctx->attr.flags & ATTR_IS_COMPRESSED)
	    && (ni->vol->major_ver >= 3)
	    //&& NVolCompression(ni->vol)//?
	    && (ni->vol->cluster_size <= MAX_COMPRESSION_CLUSTER_SIZE)
	    && ((type == AT_DATA)//只有AT_DATA 和I30属性才能被压缩
	       || ((type == AT_INDEX_ROOT) && (name == I30))))
		data_flags = ATTR_IS_COMPRESSED;
	else
	*/

	data_flags = const_cpu_to_le16(0);
	if (is_resident) {
		/* Add resident attribute. */
		offset = ntfs_resident_attr_record_add(attr_ni, type, name,
				name_len, val, size, data_flags);
		if (offset < 0) {
			if (can_be_non_resident)	//FIXEME: err ==ENOSPC
				goto add_non_resident;			
			ntfs_error(sb,"Failed to add resident attribute");
			goto free_err_out;
		}
		ntfs_debug("Done1\n");
		return 0;
	}

add_non_resident:
	/* Add non resident attribute. */
	offset = ntfs_non_resident_attr_record_add(attr_ni, type, name,
				name_len, 0, 8, data_flags);
	if (offset < 0) {		
		ntfs_error(sb,"Failed to add non resident attribute");
		goto free_err_out;
	}

	/* If @size == 0, we are done. */
	if (!size){
		ntfs_debug("Done2\n");
		return 0;
	}

#if 0
	/* Open new attribute and resize it. */
	na_3g = ntfs_attr_open(ni, type, name, name_len);
	if (!na_3g) {
		err = errno;
		ntfs_log_perror("Failed to open just added attribute");
		goto rm_attr_err_out;
	}
	/* Resize and set attribute value. */
	if (ntfs_attr_truncate(na_3g, size) ||
			(val && (ntfs_attr_pwrite(na_3g, 0, size, val) != size))) {
		err = errno;
		ntfs_log_perror("Failed to initialize just added attribute");
		if (ntfs_attr_rm(na))
			ntfs_log_perror("Failed to remove just added attribute");
		ntfs_attr_close(na_3g);
		goto err_out;
	}
	ntfs_attr_close(na_3g);
	return 0
#endif
	/* Resize and set attribute value. */
	if(ntfs_attr_truncate(attr_ni,type,name,name_len,size) || (val && (ntfs_attr_write(attr_ni,type,name,name_len,offset,size,val)!= size)))
	{	
		ntfs_error(sb,"Failed to initialize just added attribute");
		if ((ntfs_attr_rm(attr_ni,type,name,name_len) != 0)) /*remove just added attribute*/
			ntfs_error(sb,"Failed to remove just added attribute");
		goto rm_attr_err_out;
	}
	ntfs_debug("Done3\n");
	return 0;
	
	
rm_attr_err_out:
	/* Remove just added attribute in the mft record. */
	attr_m = map_mft_record(attr_ni);
	if ((ntfs_attr_record_resize(attr_m,
			(ATTR_RECORD*)((u8*)attr_m + offset), 0)) != 0)
		ntfs_error(sb,"Failed to remove just added attribute #2");
free_err_out:
	/* Free MFT record, if it doesn't contain attributes. */
	if (le32_to_cpu(attr_m->bytes_in_use) -
			le16_to_cpu(attr_m->attrs_offset) == 8)
		if ((ntfs_extent_mft_record_free(attr_ni, attr_m) != 0))	
			ntfs_error(sb,"Failed to free MFT record");
		
		
err_out:	
	if(attr_m)
		unmap_mft_record(attr_ni);
	ntfs_debug("Failed\n");
	return -1;
}


#endif /*NTFS_MODIFY*/

#endif /* NTFS_RW */
