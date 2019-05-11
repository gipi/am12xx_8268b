/*
 * dir.h - Defines for directory handling in NTFS Linux kernel driver. Part of
 *	   the Linux-NTFS project.
 *
 * Copyright (c) 2002-2004 Anton Altaparmakov
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

#ifndef _LINUX_NTFS_DIR_H
#define _LINUX_NTFS_DIR_H

#include "layout.h"
#include "inode.h"
#include "types.h"

/*
 * ntfs_name is used to return the file name to the caller of
 * ntfs_lookup_inode_by_name() in order for the caller (namei.c::ntfs_lookup())
 * to be able to deal with dcache aliasing issues.
 */
typedef struct {
	MFT_REF mref;
	FILE_NAME_TYPE_FLAGS type;
	u8 len;
	ntfschar name[0];
} __attribute__ ((__packed__)) ntfs_name;

/* The little endian Unicode string $I30 as a global constant. */
extern ntfschar I30[5];

#if 1 /*add by yangli*/
/*
 * The little endian Unicode strings $I30, $SII, $SDH, $O, $Q, $R
 * as a global constant.
 */
extern ntfschar NTFS_INDEX_SII[5];
extern ntfschar NTFS_INDEX_SDH[5];
extern ntfschar NTFS_INDEX_O[3];
extern ntfschar NTFS_INDEX_Q[3];
extern ntfschar NTFS_INDEX_R[3];
extern int ntfs_check_empty_dir(ntfs_inode *ni);

extern int ntfs_check_unlinkable_dir(ntfs_inode *ni, FILE_NAME_ATTR *fn);
#endif

extern MFT_REF ntfs_lookup_inode_by_name(ntfs_inode *dir_ni,
		const ntfschar *uname, const int uname_len, ntfs_name **res);

#endif /* _LINUX_NTFS_FS_DIR_H */
