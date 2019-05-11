/*
 * namei.c - NTFS kernel directory inode operations. Part of the Linux-NTFS
 *	     project.
 *
 * Copyright (c) 2001-2006 Anton Altaparmakov
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

#include <linux/dcache.h>
#include <linux/exportfs.h>
#include <linux/security.h>
#include <linux/gfp.h>
#include <linux/kdev_t.h>

#include "attrib.h"
#include "debug.h"
#include "dir.h"
#include "mft.h"
#include "ntfs.h"
#include "param.h"
#include "time.h"
#include "index.h"
#include "lcnalloc.h"

/**
 * ntfs_lookup - find the inode represented by a dentry in a directory inode
 * @dir_ino:	directory inode in which to look for the inode
 * @dent:	dentry representing the inode to look for
 * @nd:		lookup nameidata
 *
 * In short, ntfs_lookup() looks for the inode represented by the dentry @dent
 * in the directory inode @dir_ino and if found attaches the inode to the
 * dentry @dent.
 *
 * In more detail, the dentry @dent specifies which inode to look for by
 * supplying the name of the inode in @dent->d_name.name. ntfs_lookup()
 * converts the name to Unicode and walks the contents of the directory inode
 * @dir_ino looking for the converted Unicode name. If the name is found in the
 * directory, the corresponding inode is loaded by calling ntfs_iget() on its
 * inode number and the inode is associated with the dentry @dent via a call to
 * d_splice_alias().
 *
 * If the name is not found in the directory, a NULL inode is inserted into the
 * dentry @dent via a call to d_add(). The dentry is then termed a negative
 * dentry.
 *
 * Only if an actual error occurs, do we return an error via ERR_PTR().
 *
 * In order to handle the case insensitivity issues of NTFS with regards to the
 * dcache and the dcache requiring only one dentry per directory, we deal with
 * dentry aliases that only differ in case in ->ntfs_lookup() while maintaining
 * a case sensitive dcache. This means that we get the full benefit of dcache
 * speed when the file/directory is looked up with the same case as returned by
 * ->ntfs_readdir() but that a lookup for any other case (or for the short file
 * name) will not find anything in dcache and will enter ->ntfs_lookup()
 * instead, where we search the directory for a fully matching file name
 * (including case) and if that is not found, we search for a file name that
 * matches with different case and if that has non-POSIX semantics we return
 * that. We actually do only one search (case sensitive) and keep tabs on
 * whether we have found a case insensitive match in the process.
 *
 * To simplify matters for us, we do not treat the short vs long filenames as
 * two hard links but instead if the lookup matches a short filename, we
 * return the dentry for the corresponding long filename instead.
 *
 * There are three cases we need to distinguish here:
 *
 * 1) @dent perfectly matches (i.e. including case) a directory entry with a
 *    file name in the WIN32 or POSIX namespaces. In this case
 *    ntfs_lookup_inode_by_name() will return with name set to NULL and we
 *    just d_splice_alias() @dent.
 * 2) @dent matches (not including case) a directory entry with a file name in
 *    the WIN32 namespace. In this case ntfs_lookup_inode_by_name() will return
 *    with name set to point to a kmalloc()ed ntfs_name structure containing
 *    the properly cased little endian Unicode name. We convert the name to the
 *    current NLS code page, search if a dentry with this name already exists
 *    and if so return that instead of @dent.  At this point things are
 *    complicated by the possibility of 'disconnected' dentries due to NFS
 *    which we deal with appropriately (see the code comments).  The VFS will
 *    then destroy the old @dent and use the one we returned.  If a dentry is
 *    not found, we allocate a new one, d_splice_alias() it, and return it as
 *    above.
 * 3) @dent matches either perfectly or not (i.e. we don't care about case) a
 *    directory entry with a file name in the DOS namespace. In this case
 *    ntfs_lookup_inode_by_name() will return with name set to point to a
 *    kmalloc()ed ntfs_name structure containing the mft reference (cpu endian)
 *    of the inode. We use the mft reference to read the inode and to find the
 *    file name in the WIN32 namespace corresponding to the matched short file
 *    name. We then convert the name to the current NLS code page, and proceed
 *    searching for a dentry with this name, etc, as in case 2), above.
 *
 * Locking: Caller must hold i_mutex on the directory.
 */
static struct dentry *ntfs_lookup(struct inode *dir_ino, struct dentry *dent,
		struct nameidata *nd)
{
	ntfs_volume *vol = NTFS_SB(dir_ino->i_sb);
	struct inode *dent_inode;
	ntfschar *uname;
	ntfs_name *name = NULL;
	MFT_REF mref;
	unsigned long dent_ino;
	int uname_len;

	ntfs_debug("Looking up %s in directory inode 0x%lx.",
			dent->d_name.name, dir_ino->i_ino);
	/* Convert the name of the dentry to Unicode. */
	//ntfs_debug(vol->sb,"%s",dent->d_name.name);
	uname_len = ntfs_nlstoucs(vol, dent->d_name.name, dent->d_name.len,
			&uname);
	if (uname_len < 0) {
		if (uname_len != -ENAMETOOLONG)
			ntfs_error(vol->sb, "Failed to convert name to "
					"Unicode.");
		return ERR_PTR(uname_len);
	}
	mref = ntfs_lookup_inode_by_name(NTFS_I(dir_ino), uname, uname_len, //¸ù¾ÝÎÄ¼þ»òÄ¿Â¼ÃûÕÒµ½Æä¶ÔÓ¦ntfs inodeµÄmft record
			&name);
	kmem_cache_free(ntfs_name_cache, uname);
	if (!IS_ERR_MREF(mref)) {
		dent_ino = MREF(mref);
		ntfs_debug("Found inode 0x%lx. Calling ntfs_iget.", dent_ino);
		dent_inode = ntfs_iget(vol->sb, dent_ino);//»ñÈ¡ÐÂµÄinode
		if (likely(!IS_ERR(dent_inode))) {
			/* Consistency check. */
			if (is_bad_inode(dent_inode) || MSEQNO(mref) ==
					NTFS_I(dent_inode)->seq_no ||
					dent_ino == FILE_MFT) {
				/* Perfect WIN32/POSIX match. -- Case 1. */
				if (!name) {
					ntfs_debug("Done.  (Case 1.)");
					return d_splice_alias(dent_inode, dent);//½«ÆäÌí¼Óµ½dent
				}
				/*
				 * We are too indented.  Handle imperfect
				 * matches and short file names further below.
				 */
				goto handle_name;
			}
			ntfs_error(vol->sb, "Found stale reference to inode "
					"0x%lx (reference sequence number = "
					"0x%x, inode sequence number = 0x%x), "
					"returning -EIO. Run chkdsk.",
					dent_ino, MSEQNO(mref),
					NTFS_I(dent_inode)->seq_no);
			iput(dent_inode);
			dent_inode = ERR_PTR(-EIO);
		} else
			ntfs_error(vol->sb, "ntfs_iget(0x%lx) failed with "
					"error code %li.", dent_ino,
					PTR_ERR(dent_inode));
		kfree(name);
		/* Return the error code. */
		return (struct dentry *)dent_inode;
	}
	/* It is guaranteed that @name is no longer allocated at this point. */
	if (MREF_ERR(mref) == -ENOENT) {
		ntfs_debug("Entry was not found, adding negative dentry.");
		/* The dcache will handle negative entries. */
		d_add(dent, NULL);
		ntfs_debug("Done.\n");
		return NULL;
	}
	ntfs_error(vol->sb, "ntfs_lookup_ino_by_name() failed with error "
			"code %i.", -MREF_ERR(mref));
	return ERR_PTR(MREF_ERR(mref));
	// TODO: Consider moving this lot to a separate function! (AIA)
handle_name:
   {
	MFT_RECORD *m;
	ntfs_attr_search_ctx *ctx;
	ntfs_inode *ni = NTFS_I(dent_inode);
	int err;
	struct qstr nls_name;

	nls_name.name = NULL;
	if (name->type != FILE_NAME_DOS) {			/* Case 2. */
		ntfs_debug("Case 2.");
		nls_name.len = (unsigned)ntfs_ucstonls(vol,
				(ntfschar*)&name->name, name->len,
				(unsigned char**)&nls_name.name, 0);
		kfree(name);
	} else /* if (name->type == FILE_NAME_DOS) */ {		/* Case 3. */
		FILE_NAME_ATTR *fn;

		ntfs_debug("Case 3.");
		kfree(name);

		/* Find the WIN32 name corresponding to the matched DOS name. */
		ni = NTFS_I(dent_inode);
		m = map_mft_record(ni);
		if (IS_ERR(m)) {
			err = PTR_ERR(m);
			m = NULL;
			ctx = NULL;
			goto err_out;
		}
		ctx = ntfs_attr_get_search_ctx(ni, m);
		if (unlikely(!ctx)) {
			err = -ENOMEM;
			goto err_out;
		}
		do {
			ATTR_RECORD *a;
			u32 val_len;

			err = ntfs_attr_lookup(AT_FILE_NAME, NULL, 0, 0, 0,
					NULL, 0, ctx);
			if (unlikely(err)) {
				ntfs_error(vol->sb, "Inode corrupt: No WIN32 "
						"namespace counterpart to DOS "
						"file name. Run chkdsk.");
				if (err == -ENOENT)
					err = -EIO;
				goto err_out;
			}
			/* Consistency checks. */
			a = ctx->attr;
			if (a->non_resident || a->flags)
				goto eio_err_out;
			val_len = le32_to_cpu(a->data.resident.value_length);
			if (le16_to_cpu(a->data.resident.value_offset) +
					val_len > le32_to_cpu(a->length))
				goto eio_err_out;
			fn = (FILE_NAME_ATTR*)((u8*)ctx->attr + le16_to_cpu(
					ctx->attr->data.resident.value_offset));
			if ((u32)(fn->file_name_length * sizeof(ntfschar) +
					sizeof(FILE_NAME_ATTR)) > val_len)
				goto eio_err_out;
		} while (fn->file_name_type != FILE_NAME_WIN32);

		/* Convert the found WIN32 name to current NLS code page. */
		nls_name.len = (unsigned)ntfs_ucstonls(vol,
				(ntfschar*)&fn->file_name, fn->file_name_length,
				(unsigned char**)&nls_name.name, 0);

		ntfs_attr_put_search_ctx(ctx);
		unmap_mft_record(ni);
	}
	m = NULL;
	ctx = NULL;

	/* Check if a conversion error occurred. */
	if ((signed)nls_name.len < 0) {
		err = (signed)nls_name.len;
		goto err_out;
	}
	nls_name.hash = full_name_hash(nls_name.name, nls_name.len);

	dent = d_add_ci(dent, dent_inode, &nls_name);
	kfree(nls_name.name);
	return dent;

eio_err_out:
	ntfs_error(vol->sb, "Illegal file name attribute. Run chkdsk.");
	err = -EIO;
err_out:
	if (ctx)
		ntfs_attr_put_search_ctx(ctx);
	if (m)
		unmap_mft_record(ni);
	iput(dent_inode);
	ntfs_error(vol->sb, "Failed, returning error code %i.", err);
	return ERR_PTR(err);
   }
}

#ifdef NTFS_RW

#ifdef NTFS_MODIFY
/*
 void _Print_Buf(unsigned char * name, const unsigned char * pData, unsigned short inLen)
{
	unsigned short iLoop;

	printk("%s \n",name);
	for( iLoop=0; iLoop< inLen; iLoop++)
	{
		printk("%2x ", pData[iLoop]);
		if( 0== ((iLoop+1) &0x1f) )
		{
			printk("  %d\n",iLoop);
		}
	}
       printk("\n");
	
}*/

/**
 * __ntfs_create - create object on ntfs volume
 * @dir_ni:	ntfs inode for directory in which create new object
 * @securid:	id of inheritable security descriptor, 0 if none
 * @name:	unicode name of new object
 * @name_len:	length of the name in unicode characters
 * @type:	type of the object to create
 * @dev:	major and minor device numbers (obtained from makedev())
 * @target:	target in unicode (only for symlinks)
 * @target_len:	length of target in unicode characters
 *
 * Internal, use ntfs_create{,_device,_symlink} wrappers instead.
 *
 * @type can be:
 *	S_IFREG		to create regular file
 *	S_IFDIR		to create directory
 *	S_IFBLK		to create block device
 *	S_IFCHR		to create character device
 *	S_IFLNK		to create symbolic link
 *	S_IFIFO		to create FIFO
 *	S_IFSOCK	to create socket
 * other values are invalid.
 *
 * @dev is used only if @type is S_IFBLK or S_IFCHR, in other cases its value
 * ignored.
 *
 * @target and @target_len are used only if @type is S_IFLNK, in other cases
 * their value ignored.
 *
 * Return opened ntfs inode that describes created object on success or NULL
 * on error with errno set to the error code.
 
 *	EINVAL	- Invalid arguments passed to function.
 *	EEXIST	- Attribute of such type and with same name already exists.
 *	EIO	- I/O error occurred or damaged filesystem. 
 */
static ntfs_inode *   __ntfs_create(ntfs_inode *dir_ni, le32 securid,
		ntfschar *name, u8 name_len, mode_t type, dev_t dev,
		ntfschar *target, int target_len)
{
	int rollback_data = 0, rollback_sd = 0;
	FILE_NAME_ATTR *fn = NULL;
	STANDARD_INFORMATION *si = NULL, *dir_si;
	int err = 0, fn_len, si_len;
	MFT_RECORD *mrec = NULL, *ni_mrec;
	ntfs_attr_search_ctx *ctx=NULL;	
	struct inode * vi = VFS_I(dir_ni);
	MFT_REF  mftr;
	ntfs_inode *  ni;
	ntfs_debug("Entering.");
	
	/* Sanity checks. */
	if (!dir_ni || !name || !name_len) {		
		ntfs_error(vi->i_sb,"Invalid arguments");	
		return ERR_PTR(-EINVAL);	
	}

#if 0		
	if (dir_ni->flags & FILE_ATTR_REPARSE_POINT) {/*discribe in 0X30, fixme: about $reparse and $objild!*/
		errno = EOPNOTSUPP;
		return NULL;
	}	
#endif 	

	ni = ntfs_mft_record_alloc(dir_ni->vol, type,NULL,&ni_mrec);/*allocate a base ntfs inode and it's mft record(ni_mrec)*/
	if (IS_ERR(ni)){
		err = PTR_ERR(ni);		
		ntfs_error(vi->i_sb,"Fail to alloc new mft record");
		goto err_out;
	}
	ntfs_debug("dir mft_no:0x%lx ;allocated a new ntfs inode, mft_no:0x%lx ",dir_ni->mft_no,ni->mft_no);	
	unmap_mft_record(ni);/* release the mft record */
	
	/*get the 0x10 attribute of dir inode*/
	mrec= map_mft_record(dir_ni);
	if (IS_ERR(mrec)) {
		err = PTR_ERR(mrec);
		ntfs_error(vi->i_sb, "Failed to map mft record for inode 0x%lx "
				"(error code %d)", vi->i_ino, err);
		goto err_out;
	}
	ctx = ntfs_attr_get_search_ctx(dir_ni, mrec);/*allocate a search context for dir inode*/
	if (unlikely(!ctx)) {
		ntfs_error(vi->i_sb, "Failed to allocate a search context for "
				"inode 0x%lx (not enough memory)",
				vi->i_ino);
		//unmap_mft_record(dir_ni);	
		err = -ENOMEM;
		goto err_out;
	}
	err = ntfs_attr_lookup(AT_STANDARD_INFORMATION, NULL, 0,/* find the 0x10 attribute*/
			0, 0, NULL, 0, ctx);
	if (unlikely(err)) {
		if (err == -ENOENT) {
			ntfs_error(vi->i_sb, "Open attribute is missing from "
					"mft record.  Inode 0x%lx is corrupt.  "
					"Run chkdsk", vi->i_ino);
			err = -EIO;
		} else
			ntfs_error(vi->i_sb, "Failed to lookup attribute in "
					"inode 0x%lx (error code %d).",
					vi->i_ino, err);
		goto err_out;
	}
	
	si_len = sizeof(STANDARD_INFORMATION);/* ntfs version 3.0 */
	dir_si = (STANDARD_INFORMATION *)((char*)ctx->attr + 	/* 0x10 attribute*/
			le16_to_cpu(ctx->attr->data.resident.value_offset));

	/*
	 * Create STANDARD_INFORMATION attribute.
	 * JPA Depending on available inherited security descriptor,
	 * Write STANDARD_INFORMATION v1.2 (no inheritance) or v3
	 */

	if (!dir_si->ver.v3.security_id)
		si_len = offsetof(STANDARD_INFORMATION, ver.v1.v1_end);/* ntfs version 1.0*/

	si = kcalloc(1,si_len,GFP_KERNEL);
	if (!si) {
		err = -ENOMEM;
		goto err_out;
	}
	
	/* set the current time*/
	si->last_data_change_time = si->last_mft_change_time
						      = si->last_access_time 
						      = si->creation_time 
						      = get_current_ntfs_time();			     
	ntfs_trace("si time :%lld ",si->last_data_change_time);
	
	/* securid = 0, ntfs version:1; else:3 */
	if (dir_si->ver.v3.security_id) {
		//set_nino_flag(ni, v3_Extensions);//????
		si->ver.v3.owner_id = 0;
		si->ver.v3.security_id = dir_si->ver.v3.security_id;
		ntfs_trace("si->ver.v3.security_id = %d\n",si->ver.v3.security_id);
		si->ver.v3.quota_charged = const_cpu_to_le64(0);
		si->ver.v3.usn = const_cpu_to_le64(0);
	} //else
		//clear_nino_flag(ni, v3_Extensions);//????

	//if(S_ISREG(type))
	//	si->file_attributes |= FILE_ATTR_NORMAL;
	if(S_ISDIR(type))
		si->file_attributes |= FILE_ATTR_DIRECTORY;
	//else if(S_ISBLK(type) || S_ISCHR(type))
	//	si->file_attributes |= FILE_ATTR_DEVICE;
	//else if (!S_ISFIFO(type) && !S_ISLNK(type) && !S_ISSOCK(type)) 
	else if (!S_ISDIR(type) && !S_ISREG(type))
		si->file_attributes = FILE_ATTR_SYSTEM;
	
	si->file_attributes |= FILE_ATTR_ARCHIVE;
	
#if 0 /*donot support hidden files */
		if (NVolHideDotFiles(dir_ni->vol)//???
	    && (name_len > 1)
	    && (name[0] == const_cpu_to_le16('.'))
	    && (name[1] != const_cpu_to_le16('.')))
		//ni->flags |= FILE_ATTR_HIDDEN;
		si->file_attributes |= FILE_ATTR_COMPRESSED;
	
		/*
		 * Set compression flag according to parent directory
		 * unless NTFS version < 3.0 or cluster size > 4K
		 * or compression has been disabled
		 */
	
	if ((dir_si->flags & FILE_ATTR_COMPRESSED)
	   && (dir_ni->vol->major_ver >= 3)
	   && NVolCompression(dir_ni->vol)///???
	   && (dir_ni->vol->cluster_size <= MAX_COMPRESSION_CLUSTER_SIZE)
	   && (S_ISREG(type) || S_ISDIR(type)))
		//ni->flags |= FILE_ATTR_COMPRESSED;
		si->file_attributes |= FILE_ATTR_COMPRESSED;
	
#endif 

	/* Add STANDARD_INFORMATION to inode. */
	err = ntfs_attr_add(ni, AT_STANDARD_INFORMATION, AT_UNNAMED, 0,
			(u8*)si, si_len);
	if (err != 0) {		
		ntfs_error(vi->i_sb,"Failed to add STANDARD_INFORMATION "
				"attribute.");
		goto err_out;
	}	
	
#if 0 /* fixme: all the securid problem*/
	/*
	if (!securid) {
		if (ntfs_sd_add_everyone(ni)) {
			err = errno;
			goto err_out;
		}
	}
	*/
#endif 

	rollback_sd = 1;

	if (S_ISDIR(type)) {
		INDEX_ROOT *ir = NULL;
		INDEX_ENTRY *ie;
		int ir_len, index_len;

		/* Create INDEX_ROOT 0x90 attribute. */
		index_len = sizeof(INDEX_HEADER) + sizeof(INDEX_ENTRY_HEADER);
		ir_len = offsetof(INDEX_ROOT, index) + index_len; 
		ir = kcalloc(1,ir_len,GFP_KERNEL);
		if (!ir) {
			err = -ENOMEM;
			goto err_out;
		}
		ir->type = AT_FILE_NAME;
		ir->collation_rule = COLLATION_FILE_NAME;
		ir->index_block_size = cpu_to_le32(ni->vol->index_record_size);
		if (ni->vol->cluster_size <= ni->vol->index_record_size)
			ir->clusters_per_index_block =
					ni->vol->index_record_size >>
					ni->vol->cluster_size_bits;
		else
			ir->clusters_per_index_block = 
					ni->vol->index_record_size >>
					ni->vol->sector_size_bits;
		ir->index.entries_offset = cpu_to_le32(sizeof(INDEX_HEADER));
		ir->index.index_length = cpu_to_le32(index_len);
		ir->index.allocated_size = cpu_to_le32(index_len);
		ie = (INDEX_ENTRY*)((u8*)ir + sizeof(INDEX_ROOT));
		ie->length = cpu_to_le16(sizeof(INDEX_ENTRY_HEADER));
		ie->key_length = 0;
		ie->flags = INDEX_ENTRY_END;	/*the last Index entry*/
		/* Add INDEX_ROOT attribute to inode. */
		err = ntfs_attr_add(ni, AT_INDEX_ROOT, I30, 4,
				(u8*)ir, ir_len);
		if (err != 0) {			
			kfree(ir);
			ntfs_error(vi->i_sb,"Failed to add INDEX_ROOT attribute.");
			goto err_out;
		}
		kfree(ir);
	} else {
		INTX_FILE *data;
		int data_len;

		switch (type) {
			case S_IFBLK:/*creat block device file*/
			case S_IFCHR:/*creat char device file*/
				data_len = offsetof(INTX_FILE, device_end);
				data = kmalloc(data_len,GFP_KERNEL);
				if (!data) {
					err = -ENOMEM;
					goto err_out;
				}
				data->major = cpu_to_le64(MAJOR(dev));
				data->minor = cpu_to_le64(MINOR(dev));
				if (type == S_IFBLK)
					data->magic = INTX_BLOCK_DEVICE;
				if (type == S_IFCHR)
					data->magic = INTX_CHARACTER_DEVICE;
				break;
			case S_IFLNK: /*creat symlink file*/
				data_len = sizeof(INTX_FILE_TYPES) +
						target_len * sizeof(ntfschar);
				data = kmalloc(data_len,GFP_KERNEL);
				if (!data) {
					err = -ENOMEM;
					goto err_out;
				}
				data->magic = INTX_SYMBOLIC_LINK;
				memcpy(data->target, target,
						target_len * sizeof(ntfschar));
				break;
			case S_IFSOCK: /*creat socket file*/
				data = NULL;
				data_len = 1;
				break;
			default: /* FIFO or regular file. */
				ntfs_debug("FIFO or regular file.");
				data = NULL;
				data_len = 0;
				break;
		}
		/* Add DATA attribute 0x80 to inode.*/
		err  = ntfs_attr_add(ni, AT_DATA, AT_UNNAMED, 0, (u8*)data,
				data_len);
		if (err != 0) {			
			ntfs_error(vi->i_sb,"Failed to add DATA attribute.");
			kfree(data);
			goto err_out;
		}
		rollback_data = 1;
		kfree(data);
	}
	/* Create FILE_NAME attribute 0x30. */
	fn_len = sizeof(FILE_NAME_ATTR) + name_len * sizeof(ntfschar);
	fn = kcalloc(1,fn_len,GFP_KERNEL);
	if (!fn) {
		err = -ENOMEM;
		goto err_out;
	}
	
	fn->parent_directory = MK_LE_MREF(dir_ni->mft_no,
			le16_to_cpu(mrec->sequence_number));
	fn->file_name_length = name_len;
	fn->file_name_type = FILE_NAME_POSIX;
	if (S_ISDIR(type))
		fn->file_attributes = FILE_ATTR_DUP_FILE_NAME_INDEX_PRESENT;
	if (!S_ISREG(type) && !S_ISDIR(type))
		fn->file_attributes = FILE_ATTR_SYSTEM;
	/*fixme : all the compression and sparse ,encrption things */
	//else
		//fn->file_attributes |= ni->flags & FILE_ATTR_COMPRESSED;
	fn->file_attributes |= FILE_ATTR_ARCHIVE;
	//fn->file_attributes |= ni->flags & FILE_ATTR_HIDDEN;
	
	fn->creation_time =  fn->last_data_change_time
				     =  fn->last_mft_change_time 
				     =  fn->last_access_time
				     =  get_current_ntfs_time();	
	
	if (mrec->flags & MFT_RECORD_IS_DIRECTORY)
		fn->data_size = fn->allocated_size = const_cpu_to_le64(0);
	else {
		fn->data_size = cpu_to_sle64(ni->initialized_size);
		fn->allocated_size = cpu_to_sle64(ni->allocated_size);
	}
	memcpy(fn->file_name, name, name_len * sizeof(ntfschar));
	/* Add FILE_NAME attribute 0x30 to inode. */
	err  = ntfs_attr_add(ni, AT_FILE_NAME, AT_UNNAMED, 0, (u8*)fn, fn_len);
	if (err != 0) {			
		ntfs_error(vi->i_sb,"Failed to add FILE_NAME attribute.");
		goto err_out;
	}	
	/* Add FILE_NAME attribute to index. */
	ni_mrec = map_mft_record(ni);
	mftr= le16_to_cpu(ni_mrec->sequence_number);	
	unmap_mft_record(dir_ni);
	err  = ntfs_index_add_filename(dir_ni, fn, MK_MREF(ni->mft_no,
			mftr));
	if (err != 0) {		
		ntfs_error(vi->i_sb,"Failed to add entry to the index.");		
		goto err_out;
	}	
	mrec = NULL;
	mrec = map_mft_record(dir_ni);	
	/* Set hard links count and directory flag. */
	ni_mrec->link_count = cpu_to_le16(1);
	if (S_ISDIR(type))
		ni_mrec->flags |= MFT_RECORD_IS_DIRECTORY;
	//ntfs_inode_mark_dirty(ni);
	//flush_dcache_mft_record_page(ni);
	/*mark the page dirty ,in order to write to disk later*/	
	flush_dcache_mft_record_page(ni);
	mark_mft_record_dirty(ni);	
	
	flush_dcache_mft_record_page(dir_ni);
	mark_mft_record_dirty(dir_ni);	
	
	unmap_mft_record(dir_ni);
	unmap_mft_record(ni);
	/* Done! */
	kfree(si);	
	kfree(fn);	
	write_inode_now(VFS_I(ni), 1);
	ntfs_debug("Done.");
	return ni;
	
err_out:	
	if(!ni_mrec)
		ni_mrec = map_mft_record(ni);
	if ((ntfs_extent_mft_record_free(ni,ni_mrec)) != 0)
		ntfs_error(vi->i_sb,"Failed to free MFT record.  "
				"Leaving inconsistent metadata. Run chkdsk.");
	if(ni_mrec)
		unmap_mft_record(ni);
	if(mrec)
		unmap_mft_record(dir_ni);	
	if (ctx)
		ntfs_attr_put_search_ctx(ctx);
	if (rollback_sd)
		ntfs_attr_rm(ni, AT_SECURITY_DESCRIPTOR, AT_UNNAMED, 0);//??
	
	if (rollback_data)
		ntfs_attr_rm(ni, AT_DATA, AT_UNNAMED, 0);
#if 0
	/*
	 * Free extent MFT records (should not exist any with current
	 * ntfs_create implementation, but for any case if something will be
	 * changed in the future).
	 */

	int i =  0;
	MFT_RECORD * extent_mrec = NULL;
	while (ni->nr_extents){
		extent_mrec = map_mft_record(ni->ext.extent_ntfs_inos[i]);
		err  = ntfs_extent_mft_record_free(ni->ext.extent_ntfs_inos[i], extent_mrec);
		if (err != 0 ) {
			unmap_mft_record(ni->ext.extent_ntfs_inos[i]);
			ntfs_error(vi->i_sb,"Failed to free extent MFT record.  "
					"Leaving inconsistent metadata:%i.");
		}		
		unmap_mft_record(ni->ext.extent_ntfs_inos[i]);
		i++;
	}
#endif

	kfree(si);
	kfree(fn);
	write_inode_now(VFS_I(dir_ni), 1);		
out:
	ntfs_debug("Failed.");
	return ERR_PTR(err);

}

/*creat device file*/
static  ntfs_inode *  ntfs_create_device(ntfs_inode *dir_ni, le32 securid,
		ntfschar *name, u8 name_len, mode_t type, dev_t dev)
{
	if (type != S_IFCHR && type != S_IFBLK) {
		ntfs_error((VFS_I(dir_ni))->i_sb,"Invalid arguments.");
		return  ERR_PTR(-EINVAL);
	}
	return __ntfs_create(dir_ni, securid, name, name_len, type, dev, NULL, 0);
}

/*creat symlink file*/
static ntfs_inode *  ntfs_create_symlink(ntfs_inode *dir_ni, le32 securid,
		ntfschar *name, u8 name_len, ntfschar *target, int target_len)
{
	if (!target || !target_len) {
		ntfs_error((VFS_I(dir_ni))->i_sb,"Invalid argument (%p, %d)", target, target_len);
		return ERR_PTR(-EINVAL);
	}
	return __ntfs_create(dir_ni, securid, name, name_len, S_IFLNK, 0,
			target, target_len);
}

/*creat normal FILE or DIR or FIFIO FILE or SOCKET FILE*/
static ntfs_inode *  ntfs_create_base(ntfs_inode *dir_ni, le32 securid, ntfschar *name,
		u8 name_len, mode_t type)
{
	if (type != S_IFREG && type != S_IFIFO && type != S_IFSOCK) {
		ntfs_error((VFS_I(dir_ni))->i_sb,"Invalid arguments.");
		return ERR_PTR(-EINVAL);
	}
	return __ntfs_create(dir_ni, securid, name, name_len, type, 0, NULL, 0);
}
 
 /**
 *  creat normal file or fifo file or socket file, at present ,only test the creating of normal file ;
 *  creating fifo file or socket file havenot been tested.
 *
 * @dir:	directory inode in which to creat the inode
 * @dentry:	dentry representing the inode to creat
 * @nd:		creat nameidata
 * @mode:    creat mode
 */
static int ntfs_create(struct inode *dir, struct dentry *dentry, int mode,
		       struct nameidata *nd)
{
	ntfs_inode *dir_ni = NTFS_I(dir), *ni =NULL;
	ntfs_volume *vol = dir_ni->vol;
	ntfschar *uname = NULL;
	int uname_len = 0;
	mode_t type = mode & ~07777;	
	le32 securid = 1;	
	struct inode *vi ;
	
	ntfs_debug("entering+++++++++++++++++++++++++++++++++++++++++++++++++++++++++=.");	

	//ntfs_debug("dentry name:%s , len:%d",(char *)&dentry->d_name.name,dentry->d_name.len);	
	//ntfs_debug("dir_ni --mft_no:%ld",dir_ni->mft_no);

	/* Generate unicode filename. */
	uname_len = ntfs_nlstoucs(vol,(char *)dentry->d_name.name, dentry->d_name.len,&uname);
	_Print_Buf("file_uname",(unsigned char *)uname,uname_len*2);

	if (uname_len < 0) {
		if (uname_len != -ENAMETOOLONG)
			ntfs_error(vol->sb, "Failed to convert name to "
					"Unicode.");
		return -1;
	}

	/**
	* FIXEME:securid problem !!!
	*/

	/* Create object specified in @type. */
	switch (type) {
	#if 0 /*fixme: creat device*/
		case S_IFCHR:
		case S_IFBLK:
			res = ntfs_create_device(dir_ni, securid,
					uname, uname_len, type, dev);
			break;
	#endif 
		default:
			ni = ntfs_create_base(dir_ni, securid, uname,
					uname_len, type);
			break;
		}	

	/*release the uname buffer*/
	kmem_cache_free(ntfs_name_cache, uname);	
	if(!IS_ERR(ni)){
		vi = VFS_I(ni);
		vi->i_version++;
		vi->i_mtime = vi->i_atime = CURRENT_TIME_SEC;
		vi->i_op = &ntfs_file_inode_ops;
		vi->i_fop = &ntfs_file_ops;	
		if (NInoMstProtected(ni))
			vi->i_mapping->a_ops = &ntfs_mst_aops;
		else
			vi->i_mapping->a_ops = &ntfs_aops;
	
		//mark_inode_dirty(vi);	
		//mark_inode_dirty(dir);	
		dir->i_version++;
		/*write the dir inode to disk*/
		write_inode_now(dir, 1);		
		/*add the created inode to @dentry*/
		d_instantiate(dentry, VFS_I(ni));
		
		/*update the $mft:bitmap to disk */
		down_write(&vol->mftbmp_lock);
		ntfs_commit_inode(vol->mftbmp_ino);
		up_write(&vol->mftbmp_lock);
		return 0;
	}
	else
		return PTR_ERR(ni);

}

/**creat symlink file or directory*/
static int ntfs_link (struct dentry * old_dentry, struct inode * dir,
	struct dentry *dentry)
{
	ntfs_inode *dir_ni = NTFS_I(dir),*ni = NULL;
	ntfs_volume *vol = dir_ni->vol;
	ntfschar *uname = NULL;
	int uname_len = 0;	
	ntfschar *utarget = NULL;	
	le32 securid = 1;
	int utarget_len;
	int res = 0;
	struct inode *vi;
	
	ntfs_debug("Entering");
	/* Generate unicode filename for dentry. */
	uname_len = ntfs_nlstoucs(vol,(char *)dentry->d_name.name, dentry->d_name.len,&uname);
	//_Print_Buf("uname",(unsigned char *)uname,uname_len*2);

	if (uname_len < 0) {
		if (uname_len != -ENAMETOOLONG)
			ntfs_error(vol->sb, "Failed to convert name to "
					"Unicode.");
		res = -1;
		goto out;
	}

	/**
	* FIXEME:securid problem !!!
	*/

	/* Generate unicode filename for old_dentry */
	utarget_len = ntfs_nlstoucs(vol,(char *)old_dentry->d_name.name ,old_dentry->d_name.len, &utarget);
	if (utarget_len < 0) {
		if (uname_len != -ENAMETOOLONG)
			ntfs_error(vol->sb, "Failed to convert name to "
					"Unicode.");
		res =  -1;	
		goto put_uname_out;
	}	
	ni = ntfs_create_symlink(dir_ni, securid,uname, uname_len,utarget, utarget_len);
	
	/*release the name buffer*/
	kmem_cache_free(ntfs_name_cache, utarget);

	if(!IS_ERR(ni))
	{
		vi = VFS_I(ni);
		vi->i_version++;
		vi->i_mtime = vi->i_atime = CURRENT_TIME_SEC;

		vi->i_op = &ntfs_file_inode_ops;
		vi->i_fop = &ntfs_file_ops;	
		if (NInoMstProtected(ni))
			vi->i_mapping->a_ops = &ntfs_mst_aops;
		else
			vi->i_mapping->a_ops = &ntfs_aops;
	
		//mark_inode_dirty(vi);	
		//mark_inode_dirty(dir);	
		dir->i_version++;
		/*write the dir inode to disk*/
		write_inode_now(dir, 1);	
		/*add the created inode to @dentry*/
		d_instantiate(dentry, VFS_I(ni));
		
		down_write(&vol->mftbmp_lock);
		/*update the $mft:bitmap to disk */
		ntfs_commit_inode(vol->mftbmp_ino);
		up_write(&vol->mftbmp_lock);
		res = 0;
	}
	else
		res = PTR_ERR(ni);
	
put_uname_out:
	kmem_cache_free(ntfs_name_cache, uname);
out:	
	return res;
}

/**
 * ntfs_link - create hard link for file or directory
 * @ni:		ntfs inode for object to create hard link
 * @dir_ni:	ntfs inode for directory in which new link should be placed
 * @name:	unicode name of the new link
 * @name_len:	length of the name in unicode characters
 *
 * NOTE: At present we allow creating hardlinks to directories, we use them
 * in a temporary state during rename. But it's defenitely bad idea to have
 * hard links to directories as a result of operation.
 * FIXME: Create internal  __ntfs_link that allows hard links to a directories
 * and external ntfs_link that do not. Write ntfs_rename that uses __ntfs_link.
 *
 * Return 0 on success or -1 on error with errno set to the error code.
 */
static int ntfs_hardlink_i(ntfs_inode *ni, ntfs_inode *dir_ni, ntfschar *name,
			 u8 name_len, FILE_NAME_TYPE_FLAGS nametype)
{
	FILE_NAME_ATTR *fn = NULL;
	int fn_len;
	MFT_RECORD *mrec =NULL, *dir_mrec = NULL;
	struct inode * vi = VFS_I(ni);
	int ret = 0;
	ntfs_debug("Entering.");
	
	if (!ni || !dir_ni || !name || !name_len || 
			ni->mft_no == dir_ni->mft_no) {		
		ntfs_error(vi->i_sb,"ntfs_link wrong arguments");
		goto err_out;
	}
	/*fixme*/
	#if 0 
	if ((ni->flags & FILE_ATTR_REPARSE_POINT)
	   && !ntfs_possible_symlink(ni)) {
		err = EOPNOTSUPP;
		goto err_out;
	}
	#endif 

	dir_mrec = map_mft_record(dir_ni);//$MFT ¼ÇÂ¼Í·
	if (IS_ERR(dir_mrec)) {		
		ntfs_error(dir_ni->vol->sb, "Failed to map mft record for inode 0x%lx ",VFS_I(dir_ni)->i_ino);
		goto err_out;
	}
	mrec = map_mft_record(ni);//$MFT ¼ÇÂ¼Í·
	if (IS_ERR(mrec)) {		
		ntfs_error(vi->i_sb, "Failed to map mft record for inode 0x%lx ", vi->i_ino);
		goto err_out;
	}
	
	/* Create FILE_NAME attribute. */
	fn_len = sizeof(FILE_NAME_ATTR) + name_len * sizeof(ntfschar);
	fn = kcalloc(1,fn_len,GFP_KERNEL);
	if (!fn) {		
		goto err_out;
	}
	fn->parent_directory = MK_LE_MREF(dir_ni->mft_no,
			le16_to_cpu(dir_mrec->sequence_number));
	fn->file_name_length = name_len;
	fn->file_name_type = nametype;
	//fn->file_attributes = ni->flags; fix me
	if (mrec->flags & MFT_RECORD_IS_DIRECTORY) {
		fn->file_attributes |= FILE_ATTR_DUP_FILE_NAME_INDEX_PRESENT;//keep study 
		fn->data_size = fn->allocated_size = const_cpu_to_le64(0);
	} else {
		fn->allocated_size = cpu_to_sle64(ni->allocated_size);
		fn->data_size = cpu_to_sle64(ni->initialized_size);
	}
		
	fn->creation_time =  fn->last_data_change_time
				     =  fn->last_mft_change_time 
				     =  fn->last_access_time
				     =  get_current_ntfs_time();	
	/*
	fn->creation_time = ni->creation_time;
	fn->last_data_change_time = ni->last_data_change_time;
	fn->last_mft_change_time = ni->last_mft_change_time;
	fn->last_access_time = ni->last_access_time;
	*/
	memcpy(fn->file_name, name, name_len * sizeof(ntfschar));

	unmap_mft_record(ni);
	unmap_mft_record(dir_ni);
	/* Add FILE_NAME attribute to index. */
	if (ntfs_index_add_filename(dir_ni, fn, MK_MREF(ni->mft_no,
			le16_to_cpu(mrec->sequence_number)))) {
		ntfs_error(vi->i_sb,"Failed to add filename to the index");
		goto err_out;
	}
	/* Add FILE_NAME attribute to inode. */
	if (ntfs_attr_add(ni, AT_FILE_NAME, AT_UNNAMED, 0, (u8*)fn, fn_len)) {
		ntfs_error(vi->i_sb,"Failed to add FILE_NAME attribute.");
		
		/* Try to remove just added attribute from index. */
		if (ntfs_index_remove(dir_ni, ni, fn, fn_len))
			goto rollback_failed;
		goto err_out;
	}
	mrec = map_mft_record(ni);//$MFT ¼ÇÂ¼Í·
	if (IS_ERR(mrec)) {		
		ntfs_error(vi->i_sb, "Failed to map mft record for inode 0x%lx ", vi->i_ino);
		goto err_out;
	}
	/* Increment hard links count. */
	mrec->link_count = cpu_to_le16(le16_to_cpu(
			mrec->link_count) + 1);
	/* Done! */
	//ntfs_inode_mark_dirty(ni);
	
	#if 0 //do not need?
	/* Finally, mark the mft record dirty, so it gets written back. */
	flush_dcache_mft_record_page(dir_ni);
	mark_mft_record_dirty(dir_ni);
	#endif
	
	
	ntfs_debug("Done.");
	ret =  0;
out:
	if(fn)
		kfree(fn);
	if(ni->page)
		unmap_mft_record(ni);
	if(dir_ni->page)
		unmap_mft_record(dir_ni);	
	return ret;
rollback_failed:
	ntfs_error(vi->i_sb,"Rollback failed. Leaving inconsistent metadata.");
err_out:
	ntfs_debug("Failed.");
	ret = -1;
	goto out;
}

int ntfs_hardlink(ntfs_inode *ni, ntfs_inode *dir_ni, ntfschar *name, u8 name_len)
{
	return (ntfs_hardlink_i(ni, dir_ni, name, name_len, FILE_NAME_POSIX));
}

/**
 * ntfs_delete - delete file or directory from ntfs volume
 * @ni:		ntfs inode for object to delte
 * @dir_ni:	ntfs inode for directory in which delete object
 * @name:	unicode name of the object to delete  ÊäÈëµÄÊÇUNICODE!
 * @name_len:	length of the name in unicode characters
 *
 * @ni is always closed after the call to this function (even if it failed),
 * user does not need to call ntfs_inode_close himself.
 *
 * Return 0 on success or -1 on error with errno set to the error code.
 */
int ntfs_delete(ntfs_volume *vol,  ntfs_inode *dir_ni,ntfs_inode *ni, ntfschar *name, u8 name_len,int flags)
{
	ntfs_attr_search_ctx *actx = NULL;
	FILE_NAME_ATTR *fn = NULL;
	BOOL looking_for_dos_name = false, looking_for_win32_name = false;
	BOOL case_sensitive_match = true;
	int err = 0;
	struct inode * vi =VFS_I(ni);
	MFT_RECORD *m  =NULL;	

	ntfs_debug("Entering.");
	
	if (!ni  ||  !dir_ni  ||  !name  ||  !name_len) {
		ntfs_error(vi->i_sb,"Invalid arguments.");
		err = -EINVAL;
		goto err_out;
	}
	if (ni->nr_extents == -1)
		ni = ni->ext.base_ntfs_ino;
	if (dir_ni->nr_extents == -1)
		dir_ni = dir_ni->ext.base_ntfs_ino;
	/*
	 * Search for FILE_NAME attribute with such name. If it's in POSIX or
	 * WIN32_AND_DOS namespace, then simply remove it from index and inode.
	 * If filename in DOS or in WIN32 namespace, then remove DOS name first,
	 * only then remove WIN32 name.
	 */
	 m = map_mft_record(ni);
	if (IS_ERR(m)) {
		err = PTR_ERR(m);
		ntfs_error(vi->i_sb, "Failed to map mft record for inode 0x%lx "
				"(error code %d)", vi->i_ino, err);	
		goto err_out;
	}
	actx = ntfs_attr_get_search_ctx(ni, m);
	if (!actx)
		goto err_out;
search:
	while (!(err = ntfs_attr_lookup(AT_FILE_NAME, AT_UNNAMED, 0, CASE_SENSITIVE,
			0, NULL, 0, actx))) {
	//	unsigned char *s =NULL;
	//	int s_len = 0;
		BOOL case_sensitive = IGNORE_CASE;
		err = 0;
		
		fn = (FILE_NAME_ATTR*)((u8*)actx->attr +
				le16_to_cpu(actx->attr->data.resident.value_offset));
		//s = ntfs_attr_name_get(fn->file_name, fn->file_name_length);
	/*	s_len = ntfs_ucstonls(vol,(ntfschar *)fn->file_name,  fn->file_name_length,(unsigned char**)&s,0); //½«ÎÄ¼þÃû×ªÎªÄÚÂë?
		if (s_len < 0) {
			ntfs_error(vi->i_sb, "Failed to convert name to "
						"NLS.");
			//res = -1;
			goto err_out;
		}
	*/
		_Print_Buf("name",(unsigned char *)name,20);
		_Print_Buf("fn name",(unsigned char *)fn->file_name,20);
		ntfs_debug("type: %d  dos: %d  win32: %d  "
			       "case: %d",  fn->file_name_type,
			       looking_for_dos_name, looking_for_win32_name,
			       case_sensitive_match);
		//kfree(&s);//å ÕâÀï¾Ífree£¬ÄÇÎªÉ¶Òª×ªÄÚÂë
		if (looking_for_dos_name) {
			if (fn->file_name_type == FILE_NAME_DOS)//?
				break;
			else
				continue;
		}
		if (looking_for_win32_name) {
			if  (fn->file_name_type == FILE_NAME_WIN32)
				break;
			else
				continue;
		}
		
		/* Ignore hard links from other directories */
		if (dir_ni->mft_no != MREF_LE(fn->parent_directory)) {
			ntfs_debug("MFT record numbers don't match "
				       "(%llu != %llu)", 
				       (long long unsigned)dir_ni->mft_no, 
				       (long long unsigned)MREF_LE(fn->parent_directory));
			continue;
		}
		     
		if (fn->file_name_type == FILE_NAME_POSIX || case_sensitive_match)
			case_sensitive = CASE_SENSITIVE;
		err = ntfs_are_names_equal(fn->file_name, fn->file_name_length,//±È½ÏunicodeÎÄ¼þÃû
					 name, name_len, case_sensitive, 
					 ni->vol->upcase, ni->vol->upcase_len);		
		if (err){//unicodeÃû×ÖÆ¥Åä	
			ntfs_debug("unicode names are equal ");
			if (fn->file_name_type == FILE_NAME_WIN32) {//Èç¹ûWIN32(³¤Ãû)Ãû×ÖÆ¥ÅäÁË£¬»¹Ðè±È½ÏDOS(¶ÌÃû)Ãû×Ö
				looking_for_dos_name = true;
				ntfs_attr_reinit_search_ctx(actx);
				continue;
			}
			if (fn->file_name_type == FILE_NAME_DOS)
				looking_for_dos_name = true;
			err = 0;
			break;
		}
		else{
			err  =ENOENT;
			ntfs_debug("unicode names arenot equal ");
		}
	}
	if (err) {
		ntfs_debug("have not found the 0x30 ? ");
		/*
		 * If case sensitive search failed, then try once again
		 * ignoring case.
		 */
		if (err == ENOENT && case_sensitive_match) {
			case_sensitive_match = false;
			ntfs_attr_reinit_search_ctx(actx);
			ntfs_debug("change case_sensitive_match, and goto search ");
			goto search;
		}
		goto err_out;
	}
	ntfs_debug("find the filename attribute");
	if(ni->page)
		unmap_mft_record(ni);
	m = NULL;
	
	if (ntfs_check_unlinkable_dir(ni, fn) < 0)
		goto err_out;
	if (ntfs_index_remove(dir_ni, ni, fn, le32_to_cpu(actx->attr->data.resident.value_length)))//É¾³ýINDEX ¼ÇÂ¼
		goto err_out;
	
	fn = (FILE_NAME_ATTR*)((u8*)actx->attr +
				le16_to_cpu(actx->attr->data.resident.value_offset));
	_Print_Buf("fn name",(unsigned char *)fn->file_name,20);
	if (ntfs_attr_record_rm(actx))//É¾³ýÎÄ¼þÃûÊôÐÔ
		goto err_out;

	m = map_mft_record(ni);
	if (IS_ERR(m)) {
		err = PTR_ERR(m);
		ntfs_error(vi->i_sb, "Failed to map mft record for inode 0x%lx "
				"(error code %d)", vi->i_ino, err);	
		goto err_out;
	}
	/*dec the hard link count*/
	m->link_count = cpu_to_le16(le16_to_cpu(
			m->link_count) - 1);

	unmap_mft_record(ni);
	m =NULL;
	//ntfs_inode_mark_dirty(ni);
	if (looking_for_dos_name) { //»¹ÐèÆ¥Åä¶ÌÃû
		looking_for_dos_name = false;
		looking_for_win32_name = true;
		ntfs_attr_reinit_search_ctx(actx);
		ntfs_debug("goto research ");
		goto search;
	}
#if 0
	/* TODO: Update object id, quota and securiry indexes if required. */
	/*
	 * If hard link count is not equal to zero then we are done. In other
	 * case there are no reference to this inode left, so we should free all
	 * non-resident attributes and mark all MFT record as not in use.
	 */
//	if (ni->mrec->link_count) {
//		ntfs_inode_update_times(ni, NTFS_UPDATE_CTIME);
//		goto ok;
//	}
	if (ntfs_delete_reparse_index(ni)) {
		/*
		 * Failed to remove the reparse index : proceed anyway
		 * This is not a critical error, the entry is useless
		 * because of sequence_number, and stopping file deletion
		 * would be much worse as the file is not referenced now.
		 */
		err = errno;
	}
	if (ntfs_delete_object_id_index(ni)) {
		/*
		 * Failed to remove the object id index : proceed anyway
		 * This is not a critical error.
		 */
		err = errno;
	}
#endif
	if(flags){
		ntfs_attr_reinit_search_ctx(actx);
		while (!(err = ntfs_attrs_walk(actx))) {//±éÀúËùÓÐÊôÐÔ,ÊÍ·ÅËùÓÐ·Ç×¤ÁôÊôÐÔµÄrunlist
			if (actx->attr->non_resident) {
				runlist_element *rl;

				rl = ntfs_mapping_pairs_decompress(ni->vol, actx->attr,//»ñÈ¡runlist
						NULL);
				if (!rl) {
					//err = errno;
					ntfs_error(vi->i_sb,"Failed to decompress runlist.  "
							"Leaving inconsistent metadata.");
					continue;
				}
				if (ntfs_cluster_free_from_rl(ni->vol, rl)) {//ÊÍ·Å¸Ãrunlist
					//err = errno;
					ntfs_error(vi->i_sb,"Failed to free clusters.  "
							"Leaving inconsistent metadata.");
					continue;
				}
				kfree(rl);
			}
		}
		if (err != -ENOENT) {
			//err = errno;
			ntfs_error(vi->i_sb,"Attribute enumeration failed.  "	//may bug
					"Probably leaving inconsistent metadata.");
		}
		/* All extents should be attached after attribute walk. */
		if (ntfs_mft_record_free(ni->vol, ni)) {//ÊÍ·Åmft record
			//err = errno;
			ntfs_error(vi->i_sb,"Failed to free base MFT record.  "
					"Leaving inconsistent metadata.");
		}
		m = NULL;
		ni = NULL;		
	}	
	err = 0;
out:
	if(m)
		unmap_mft_record(ni);
	if (actx)
		ntfs_attr_put_search_ctx(actx);	
	ntfs_debug("err = %d.",err);
	
	return err;
err_out:		
	ntfs_debug("Could not delete file.");
	err = -1;
	goto out;		
	
}

static int ntfs_unlink(struct inode *dir, struct dentry *dentry)
{
	int res = 0;
	struct inode * vi = dentry->d_inode;	
	ntfs_inode *dir_ni = NTFS_I(dir),*ni = NULL;
	ntfs_volume *vol = dir_ni->vol;
	ntfschar *uname = NULL;
	int uname_len = 0;
//	MFT_REF mref;
//	ntfs_name *name =NULL;
	ntfs_debug("Entering");

	ni = NTFS_I(vi);
	ntfs_debug("ni->mft_no = %ld ,dir_ni->mft_no = %ld ",ni->mft_no,dir_ni->mft_no);
	uname_len = ntfs_nlstoucs(vol,(char *)dentry->d_name.name, dentry->d_name.len,&uname);
	if (uname_len < 0){
	 	 if (uname_len != -ENAMETOOLONG)
			ntfs_error(vol->sb, "Failed to convert name to "
					"Unicode.");
		return -1;
	}
/*
	mref = ntfs_lookup_inode_by_name(dir_ni, uname, uname_len, //¸ù¾ÝÎÄ¼þ»òÄ¿Â¼ÃûÕÒµ½Æä¶ÔÓ¦ntfs inodeµÄmft record
			&name);
	if (!IS_ERR_MREF(mref)) {
		ntfs_error(dir_ni->i_sb," path %s isnot exsit",(char *)&new_dentry->d_name.name);
		goto out;
	}
	*/
	res = ntfs_delete(vol, dir_ni,ni, uname, uname_len,1);
		
	if(!res){
		//clear_nlink(vi);
		//vi->i_mtime = vi->i_atime = CURRENT_TIME_SEC;
		//drop_nlink(dir);
		
		write_inode_now(dir, 1);
		write_inode_now(vi, 1);
		
		down_write(&vol->mftbmp_lock);
		ntfs_commit_inode(vol->mftbmp_ino);
		up_write(&vol->mftbmp_lock);
		
		down_write(&vol->lcnbmp_lock);
		ntfs_commit_inode(vol->lcnbmp_ino);
		up_write(&vol->lcnbmp_lock);	
		
		ntfs_debug("Done ");
	}else
		ntfs_debug("Failed ");	
	//clear_nlink(vi);

	//d_delete(dentry);
	//ntfs_clear_big_inode(vi);
//out:
	if (uname_len)
		kfree(uname);
	
	return res;
	
}
static int ntfs_mkdir(struct inode *dir, struct dentry *dentry, int mode)
{
	ntfs_inode *dir_ni = NTFS_I(dir),*ni = NULL;
	ntfs_volume *vol = dir_ni->vol;
	ntfschar *uname = NULL;
	int uname_len = 0;
	mode_t type = mode & ~07777;
	le32 securid =  1;
	int res = 0;
	struct inode *inode ;
	
	ntfs_debug("Entering");
	
/*	if (type != S_IFDIR){
		ntfs_error(dir->i_sb,"Invalid arguments.");
		return -EINVAL;
	}
	*/
	/**filename (ÄÚÂë)*/
	//ntfs_debug("dentry name:%s , len:%d",(char *)&dentry->d_name.name,dentry->d_name.len);	
	//ntfs_debug("dir_ni --mft_no:%ld",dir_ni->mft_no);

	/* Generate unicode filename. */
	uname_len = ntfs_nlstoucs(vol,(char *)dentry->d_name.name, dentry->d_name.len,&uname);
	//_Print_Buf("dir_uname",(unsigned char *)uname,uname_len*2);

	if (uname_len < 0){
	 	 if (uname_len != -ENAMETOOLONG)
			ntfs_error(vol->sb, "Failed to convert name to "
					"Unicode.");
		return -1;
	}

	ni = __ntfs_create(dir_ni, securid, uname, uname_len,  type| S_IFDIR, 0, NULL, 0);

	kmem_cache_free(ntfs_name_cache, uname);
	if(!IS_ERR(ni)){
		inode  =VFS_I(ni);
		inode->i_version++;
		inode->i_nlink = 2;
		inode->i_mtime = inode->i_atime = CURRENT_TIME_SEC;

		inode->i_op = &ntfs_dir_inode_ops;
		inode->i_fop = &ntfs_dir_ops;	
		if (NInoMstProtected(ni))
			inode->i_mapping->a_ops = &ntfs_mst_aops;
		else
			inode->i_mapping->a_ops = &ntfs_aops;

		dir->i_version++;
		dentry->d_time = dentry->d_parent->d_inode->i_version;
		inc_nlink(dir);
		
		//mark_inode_dirty(inode);	
		//mark_inode_dirty(dir);		

		write_inode_now(dir, 1);
		d_instantiate(dentry, inode);
		
		down_write(&vol->mftbmp_lock);
		ntfs_commit_inode(vol->mftbmp_ino);
		up_write(&vol->mftbmp_lock);
		
		res = 0;
	}else
		res =  PTR_ERR(ni);
	
	ntfs_debug("res:%d",res);
	
	return res;
}

static int ntfs_rmdir(struct inode *dir, struct dentry *dentry)
{
	int res = 0;
	struct inode * vi = dentry->d_inode;	
	ntfs_inode *dir_ni = NTFS_I(dir),*ni = NULL;
	ntfs_volume *vol = dir_ni->vol;
	ntfschar *uname = NULL;
	int uname_len = 0;
//	MFT_REF mref;
//	ntfs_name *name =NULL;
	ntfs_debug("Entering");

	ni = NTFS_I(vi);
	ntfs_debug("ni->mft_no = %ld ,dir_ni->mft_no = %ld ",ni->mft_no,dir_ni->mft_no);
	uname_len = ntfs_nlstoucs(vol,(char *)dentry->d_name.name, dentry->d_name.len,&uname);
	if (uname_len < 0){
	 	 if (uname_len != -ENAMETOOLONG)
			ntfs_error(vol->sb, "Failed to convert name to "
					"Unicode.");
		return -1;
	}
	res = ntfs_delete(vol, dir_ni,ni, uname, uname_len,1);


	if(!res){
		drop_nlink(dir);
		//clear_nlink(vi);
		//vi->i_mtime = vi->i_atime = CURRENT_TIME_SEC;
		
		write_inode_now(dir, 1);
		write_inode_now(vi, 1);
		down_write(&vol->mftbmp_lock);
		ntfs_commit_inode(vol->mftbmp_ino);
		up_write(&vol->mftbmp_lock);
		
		down_write(&vol->lcnbmp_lock);
		ntfs_commit_inode(vol->lcnbmp_ino);
		up_write(&vol->lcnbmp_lock);	
		
		ntfs_debug("Done ");
	}else
		ntfs_debug("Failed ");
		
	//clear_nlink(vi);

	//d_delete(dentry);
	//ntfs_clear_big_inode(vi);
//out:
	if (uname_len)
		kfree(uname);
	
	return res;
	
}
static int ntfs_rename(struct inode *old_dir, struct dentry *old_dentry,
		       struct inode *new_dir, struct dentry *new_dentry)
{
	int ret = 0, new_uname_len,old_uname_len;
	ntfschar *new_uname = NULL,*old_uname = NULL;
	ntfs_name *name =NULL;
	ntfs_inode *old_dni, * new_dni, * ni;
	ntfs_volume *old_vol,* new_vol;
//	u64 inum;
//	BOOL same;
	MFT_REF mref;

	ntfs_debug("Entering");
	
	ni = NTFS_I(old_dentry->d_inode);	
	//vi = VFS_I(ni);
	
	old_dni = NTFS_I(old_dir);
	new_dni = NTFS_I(new_dir);
	
	old_vol = old_dni->vol;
	new_vol = new_dni->vol;
	
	ntfs_debug("ni->mft_no = %ld ",ni->mft_no);
	ntfs_debug("old_dni->mft_no = %ld ,new_dni->mft_no = %ld ",old_dni->mft_no,new_dni->mft_no);

	/*
	 *  FIXME: Rename should be atomic.
	 */

	old_uname_len = ntfs_nlstoucs(old_vol,(char *)old_dentry->d_name.name, old_dentry->d_name.len,&old_uname);
	if (old_uname_len < 0){
	 	 if (old_uname_len != -ENAMETOOLONG)
			ntfs_error(old_dir->i_sb, "Failed to convert name to "
					"Unicode.");
		goto err_out;
	}
	
	/**check if the new path exsit already*/
	new_uname_len = ntfs_nlstoucs(new_vol,(char *)new_dentry->d_name.name, new_dentry->d_name.len,&new_uname);
	if (new_uname_len < 0){
	 	 if (new_uname_len != -ENAMETOOLONG)
			ntfs_error(new_dir->i_sb, "Failed to convert name to "
					"Unicode.");
		goto err_out;
	}
	mref = ntfs_lookup_inode_by_name(new_dni, new_uname, new_uname_len, //¸ù¾ÝÎÄ¼þ»òÄ¿Â¼ÃûÕÒµ½Æä¶ÔÓ¦ntfs inodeµÄmft record
			&name);
	if (IS_ERR_MREF(mref)) {
		ntfs_debug("new path  is not  exsiting in new dir %ld",new_dni->mft_no);
		goto rename_non_exsit;
	}
	else
		ntfs_debug("new path is  exsiting in new dir %ld",new_dni->mft_no);		
		goto err_out;
	/**donot need to check if the old path exsit already*/	
/*	mref = ntfs_lookup_inode_by_name(old_ni, old_uname, old_uname_len, //¸ù¾ÝÎÄ¼þ»òÄ¿Â¼ÃûÕÒµ½Æä¶ÔÓ¦ntfs inodeµÄmft record
			&name);
	if (IS_ERR_MREF(mref)) {
		ntfs_error(old_dir->i_sb,"fail to find old path %s",(char *)&old_dentry->d_name.name);
		goto err_out;
	}
*/
rename_non_exsit:
	ret = ntfs_hardlink(ni,new_dni,new_uname, new_uname_len);	
	if(ret){
		ntfs_debug("creat hard link err ");
		goto err_out;
	}
	d_instantiate(new_dentry, VFS_I(ni));//½«niÌí¼Óµ½new_dentry
	inc_nlink(VFS_I(ni));
	VFS_I(ni)->i_version++;
	
	ret =  ntfs_delete(old_vol, old_dni,ni, old_uname, old_uname_len,0);
	drop_nlink(VFS_I(ni));
	if (ret){
		//ntfs_unlink(new_dir,new_dentry);
		ntfs_debug("delete just add name and index entry");
		ntfs_delete(new_vol, new_dni, ni, new_uname, new_uname_len,0);
		drop_nlink(VFS_I(ni));
	}
	write_inode_now(VFS_I(ni), 1);
	write_inode_now(old_dir, 1);
	write_inode_now(new_dir, 1);


out:
	if(old_uname)
		kmem_cache_free(ntfs_name_cache, old_uname);
	if(new_uname)
		kmem_cache_free(ntfs_name_cache, new_uname);
	return ret;
err_out:
	ntfs_debug("Failed.");
	ret = -1;
	goto out;
}

#endif /*NTFS_MODIFY*/
#endif

/**
 * Inode operations for directories.
 */
const struct inode_operations ntfs_dir_inode_ops = {
	.lookup	= ntfs_lookup,	/* VFS: Lookup directory. */
#ifdef NTFS_RW
#ifdef NTFS_MODIFY
	.create		= ntfs_create,/*creat file including normal file and fifo file,socket file, link file*/
	.unlink		= ntfs_unlink,/*delete file*/ 
	.mkdir		= ntfs_mkdir,
	.rmdir		= ntfs_rmdir,
	.link			= ntfs_link,
	.rename		= ntfs_rename,
	.setattr		= ntfs_setattr,
/*	.getattr	= fat_getattr,*/
#endif /*NTFS_MODIFY*/
#endif
};

/**
 * ntfs_get_parent - find the dentry of the parent of a given directory dentry
 * @child_dent:		dentry of the directory whose parent directory to find
 *
 * Find the dentry for the parent directory of the directory specified by the
 * dentry @child_dent.  This function is called from
 * fs/exportfs/expfs.c::find_exported_dentry() which in turn is called from the
 * default ->decode_fh() which is export_decode_fh() in the same file.
 *
 * The code is based on the ext3 ->get_parent() implementation found in
 * fs/ext3/namei.c::ext3_get_parent().
 *
 * Note: ntfs_get_parent() is called with @child_dent->d_inode->i_mutex down.
 *
 * Return the dentry of the parent directory on success or the error code on
 * error (IS_ERR() is true).
 */
static struct dentry *ntfs_get_parent(struct dentry *child_dent)
{
	struct inode *vi = child_dent->d_inode;
	ntfs_inode *ni = NTFS_I(vi);
	MFT_RECORD *mrec;
	ntfs_attr_search_ctx *ctx;
	ATTR_RECORD *attr;
	FILE_NAME_ATTR *fn;
	struct inode *parent_vi;
	struct dentry *parent_dent;
	unsigned long parent_ino;
	int err;

	ntfs_debug("Entering for inode 0x%lx.", vi->i_ino);
	/* Get the mft record of the inode belonging to the child dentry. */
	mrec = map_mft_record(ni);
	if (IS_ERR(mrec))
		return (struct dentry *)mrec;
	/* Find the first file name attribute in the mft record. */
	ctx = ntfs_attr_get_search_ctx(ni, mrec);
	if (unlikely(!ctx)) {
		unmap_mft_record(ni);
		return ERR_PTR(-ENOMEM);
	}
try_next:
	err = ntfs_attr_lookup(AT_FILE_NAME, NULL, 0, CASE_SENSITIVE, 0, NULL,
			0, ctx);
	if (unlikely(err)) {
		ntfs_attr_put_search_ctx(ctx);
		unmap_mft_record(ni);
		if (err == -ENOENT)
			ntfs_error(vi->i_sb, "Inode 0x%lx does not have a "
					"file name attribute.  Run chkdsk.",
					vi->i_ino);
		return ERR_PTR(err);
	}
	attr = ctx->attr;
	if (unlikely(attr->non_resident))
		goto try_next;
	fn = (FILE_NAME_ATTR *)((u8 *)attr +
			le16_to_cpu(attr->data.resident.value_offset));
	if (unlikely((u8 *)fn + le32_to_cpu(attr->data.resident.value_length) >
			(u8*)attr + le32_to_cpu(attr->length)))
		goto try_next;
	/* Get the inode number of the parent directory. */
	parent_ino = MREF_LE(fn->parent_directory);
	/* Release the search context and the mft record of the child. */
	ntfs_attr_put_search_ctx(ctx);
	unmap_mft_record(ni);
	/* Get the inode of the parent directory. */
	parent_vi = ntfs_iget(vi->i_sb, parent_ino);
	if (IS_ERR(parent_vi) || unlikely(is_bad_inode(parent_vi))) {
		if (!IS_ERR(parent_vi))
			iput(parent_vi);
		ntfs_error(vi->i_sb, "Failed to get parent directory inode "
				"0x%lx of child inode 0x%lx.", parent_ino,
				vi->i_ino);
		return ERR_PTR(-EACCES);
	}
	/* Finally get a dentry for the parent directory and return it. */
	parent_dent = d_alloc_anon(parent_vi);
	if (unlikely(!parent_dent)) {
		iput(parent_vi);
		return ERR_PTR(-ENOMEM);
	}
	ntfs_debug("Done for inode 0x%lx.", vi->i_ino);
	return parent_dent;
}

static struct inode *ntfs_nfs_get_inode(struct super_block *sb,
		u64 ino, u32 generation)
{
	struct inode *inode;

	inode = ntfs_iget(sb, ino);
	if (!IS_ERR(inode)) {
		if (is_bad_inode(inode) || inode->i_generation != generation) {
			iput(inode);
			inode = ERR_PTR(-ESTALE);
		}
	}

	return inode;
}

static struct dentry *ntfs_fh_to_dentry(struct super_block *sb, struct fid *fid,
		int fh_len, int fh_type)
{
	return generic_fh_to_dentry(sb, fid, fh_len, fh_type,
				    ntfs_nfs_get_inode);
}

static struct dentry *ntfs_fh_to_parent(struct super_block *sb, struct fid *fid,
		int fh_len, int fh_type)
{
	return generic_fh_to_parent(sb, fid, fh_len, fh_type,
				    ntfs_nfs_get_inode);
}

/**
 * Export operations allowing NFS exporting of mounted NTFS partitions.
 *
 * We use the default ->encode_fh() for now.  Note that they
 * use 32 bits to store the inode number which is an unsigned long so on 64-bit
 * architectures is usually 64 bits so it would all fail horribly on huge
 * volumes.  I guess we need to define our own encode and decode fh functions
 * that store 64-bit inode numbers at some point but for now we will ignore the
 * problem...
 *
 * We also use the default ->get_name() helper (used by ->decode_fh() via
 * fs/exportfs/expfs.c::find_exported_dentry()) as that is completely fs
 * independent.
 *
 * The default ->get_parent() just returns -EACCES so we have to provide our
 * own and the default ->get_dentry() is incompatible with NTFS due to not
 * allowing the inode number 0 which is used in NTFS for the system file $MFT
 * and due to using iget() whereas NTFS needs ntfs_iget().
 */
const struct export_operations ntfs_export_ops = {
	.get_parent	= ntfs_get_parent,	/* Find the parent of a given
						   directory. */
	.fh_to_dentry	= ntfs_fh_to_dentry,
	.fh_to_parent	= ntfs_fh_to_parent,
};
