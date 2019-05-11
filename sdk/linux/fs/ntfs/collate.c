/*
 * collate.c - NTFS kernel collation handling.  Part of the Linux-NTFS project.
 *
 * Copyright (c) 2004 Anton Altaparmakov
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

#include "collate.h"
#include "debug.h"
#include "ntfs.h"


static int ntfs_collate_binary(ntfs_volume *vol,
		const void *data1, const int data1_len,
		const void *data2, const int data2_len)
{
	int rc;

	//ntfs_debug("Entering.");
	rc = memcmp(data1, data2, min(data1_len, data2_len));
	if (!rc && (data1_len != data2_len)) {
		if (data1_len < data2_len)
			rc = -1;
		else
			rc = 1;
	}
	//ntfs_debug("Done, returning %i", rc);
	return rc;
}

static int ntfs_collate_ntofs_ulong(ntfs_volume *vol,
		const void *data1, const int data1_len,
		const void *data2, const int data2_len)
{
	int rc;
	u32 d1, d2;

	//ntfs_debug("Entering.");
	// FIXME:  We don't really want to bug here.
	BUG_ON(data1_len != data2_len);
	BUG_ON(data1_len != 4);
	d1 = le32_to_cpup(data1);
	d2 = le32_to_cpup(data2);
	if (d1 < d2)
		rc = -1;
	else {
		if (d1 == d2)
			rc = 0;
		else
			rc = 1;
	}
	//ntfs_debug("Done, returning %i", rc);
	return rc;
}

#ifdef NTFS_MODIFY
/**
 * ntfs_collate_ntofs_ulongs - Which of two le32 arrays should be listed first
 *
 * Returns: -1, 0 or 1 depending of how the arrays compare
 */

static int ntfs_collate_ntofs_ulongs(ntfs_volume *vol,
		const void *data1, const int data1_len,
		const void *data2, const int data2_len)
{
	int rc;
	int len;
	const le32 *p1, *p2;
	u32 d1, d2;

	//ntfs_debug("Entering.");
	if ((data1_len != data2_len) || (data1_len <= 0) || (data1_len & 3)) {
		ntfs_error(vol->sb,"data1_len or data2_len not valid");
		return NTFS_COLLATION_ERROR;
	}
	p1 = (const le32*)data1;
	p2 = (const le32*)data2;
	len = data1_len;
	do {
		d1 = le32_to_cpup(p1);
		p1++;
		d2 = le32_to_cpup(p2);
		p2++;
	} while ((d1 == d2) && ((len -= 4) > 0));
	if (d1 < d2)
		rc = -1;
	else {
		if (d1 == d2)
			rc = 0;
		else
			rc = 1;
	}
	//ntfs_debug("Done, returning %i.", rc);
	return rc;
}

/**
 * ntfs_collate_ntofs_security_hash - Which of two security descriptors
 *                    should be listed first
 * @vol: unused
 * @data1:
 * @data1_len:
 * @data2:
 * @data2_len:
 *
 * JPA compare two security hash keys made of two unsigned le32
 *
 * Returns: -1, 0 or 1 depending of how the keys compare
 */
static int ntfs_collate_ntofs_security_hash(ntfs_volume *vol ,
		const void *data1, const int data1_len,
		const void *data2, const int data2_len)
{
	int rc;
	u32 d1, d2;
	const le32 *p1, *p2;

	//ntfs_debug("Entering.");
	if (data1_len != data2_len || data1_len != 8) {
		ntfs_error(vol->sb,"data1_len or/and data2_len not equal to 8.");
		return NTFS_COLLATION_ERROR;
	}
	p1 = (const le32*)data1;
	p2 = (const le32*)data2;
	d1 = le32_to_cpup(p1);
	d2 = le32_to_cpup(p2);
	if (d1 < d2)
		rc = -1;
	else {
		if (d1 > d2)
			rc = 1;
		else {
			p1++;
			p2++;
			d1 = le32_to_cpup(p1);
			d2 = le32_to_cpup(p2);
			if (d1 < d2)
				rc = -1;
			else {
				if (d1 > d2)
					rc = 1;
				else
					rc = 0;
			}
		}
	}
	//ntfs_debug("Done, returning %i.", rc);
	return rc;
}

/*
 * ntfs_names_full_collate() fully collate two Unicode names
 *
 * @name1:	first Unicode name to compare
 * @name1_len:	length of first Unicode name to compare
 * @name2:	second Unicode name to compare
 * @name2_len:	length of second Unicode name to compare
 * @ic:		either CASE_SENSITIVE or IGNORE_CASE
 * @upcase:	upcase table (ignored if @ic is CASE_SENSITIVE)
 * @upcase_len:	upcase table size (ignored if @ic is CASE_SENSITIVE)
 *
 *  -1 if the first name collates before the second one,
 *   0 if the names match,
 *   1 if the second name collates before the first one, or
 *
 */
int ntfs_names_full_collate(const ntfschar *name1, const u32 name1_len,
		const ntfschar *name2, const u32 name2_len,
		const IGNORE_CASE_BOOL ic, const ntfschar *upcase,
		const u32 upcase_len)
{
	u32 cnt;
	u16 c1, c2;
	u16 u1, u2;

#ifdef DEBUG
	if (!name1 || !name2 || (ic && (!upcase || !upcase_len))) {
		ntfs_debug("ntfs_names_collate received NULL pointer!");
		return NTFS_COLLATION_ERROR;
	}
#endif
	cnt = min(name1_len, name2_len);
	if (cnt > 0) {
		if (ic == CASE_SENSITIVE) {
			do {
				c1 = le16_to_cpu(*name1);
				name1++;
				c2 = le16_to_cpu(*name2);
				name2++;
			} while (--cnt && (c1 == c2));
			u1 = c1;
			u2 = c2;
			if (u1 < upcase_len)
				u1 = le16_to_cpu(upcase[u1]);
			if (u2 < upcase_len)
				u2 = le16_to_cpu(upcase[u2]);
			if ((u1 == u2) && cnt)
				do {
					u1 = le16_to_cpu(*name1);
					name1++;
					u2 = le16_to_cpu(*name2);
					name2++;
					if (u1 < upcase_len)
						u1 = le16_to_cpu(upcase[u1]);
					if (u2 < upcase_len)
						u2 = le16_to_cpu(upcase[u2]);
				} while ((u1 == u2) && --cnt);
			if (u1 < u2)
				return -1;
			if (u1 > u2)
				return 1;
			if (name1_len < name2_len)
				return -1;
			if (name1_len > name2_len)
				return 1;
			if (c1 < c2)
				return -1;
			if (c1 > c2)
				return 1;
		} else {
			do {
				u1 = c1 = le16_to_cpu(*name1);
				name1++;
				u2 = c2 = le16_to_cpu(*name2);
				name2++;
				if (u1 < upcase_len)
					u1 = le16_to_cpu(upcase[u1]);
				if (u2 < upcase_len)
					u2 = le16_to_cpu(upcase[u2]);
			} while ((u1 == u2) && --cnt);
			if (u1 < u2)
				return -1;
			if (u1 > u2)
				return 1;
			if (name1_len < name2_len)
				return -1;
			if (name1_len > name2_len)
				return 1;
		}
	} else {
		if (name1_len < name2_len)
			return -1;
		if (name1_len > name2_len)
			return 1;
	}
	return 0;
}
/**
 * ntfs_collate_file_name - Which of two filenames should be listed first
 * @vol:
 * @data1:
 * @data1_len: unused
 * @data2:
 * @data2_len: unused
 *
 * Description...
 *
 * Returns:
 */
static int ntfs_collate_file_name(ntfs_volume *vol,
		const void *data1, const int data1_len ,
		const void *data2, const int data2_len )
{
	const FILE_NAME_ATTR *file_name_attr1;
	const FILE_NAME_ATTR *file_name_attr2;
	int rc;

	//ntfs_debug("Entering.");
	file_name_attr1 = (const FILE_NAME_ATTR*)data1;
	file_name_attr2 = (const FILE_NAME_ATTR*)data2;
	rc = ntfs_names_full_collate(
			(ntfschar*)&file_name_attr1->file_name,
			file_name_attr1->file_name_length,
			(ntfschar*)&file_name_attr2->file_name,
			file_name_attr2->file_name_length,
			CASE_SENSITIVE, vol->upcase, vol->upcase_len);
	//ntfs_debug("Done, returning %i.", rc);
	return rc;
}

static int ntfs_collate_unicode_string(ntfs_volume *vol,
		const void *data1, const int data1_len ,
		const void *data2, const int data2_len )
{
	ntfs_debug("sorry , donot support yet ");
	return NTFS_COLLATION_ERROR;
}

static int ntfs_collate_ntofs_sid (ntfs_volume *vol,
		const void *data1, const int data1_len ,
		const void *data2, const int data2_len )
{
	ntfs_debug("sorry , donot support yet ");
	return NTFS_COLLATION_ERROR;
}

#endif
typedef int (*ntfs_collate_func_t)(ntfs_volume *, const void *, const int,
		const void *, const int);

/* @ fix by yangli */
/* @ add all collation functions*/
static ntfs_collate_func_t ntfs_do_collate0x0[3] = {
	ntfs_collate_binary,
	ntfs_collate_file_name,
	ntfs_collate_unicode_string,
};

static ntfs_collate_func_t ntfs_do_collate0x1[4] = {
	ntfs_collate_ntofs_ulong,
	ntfs_collate_ntofs_sid,
	ntfs_collate_ntofs_security_hash,
	ntfs_collate_ntofs_ulongs,
};
/* @ end */

/**
 * ntfs_collate - collate two data items using a specified collation rule
 * @vol:	ntfs volume to which the data items belong
 * @cr:		collation rule to use when comparing the items
 * @data1:	first data item to collate
 * @data1_len:	length in bytes of @data1
 * @data2:	second data item to collate
 * @data2_len:	length in bytes of @data2
 *
 * Collate the two data items @data1 and @data2 using the collation rule @cr
 * and return -1, 0, ir 1 if @data1 is found, respectively, to collate before,
 * to match, or to collate after @data2.
 *
 * For speed we use the collation rule @cr as an index into two tables of
 * function pointers to call the appropriate collation function.
 */

/*return value: 
   0: math
   1: data1 > data2
   -1: data1 <data2
   -2: err
 */
int ntfs_collate(ntfs_volume *vol, COLLATION_RULE cr,
		const void *data1, const int data1_len,
		const void *data2, const int data2_len) {
	int i;

	//ntfs_debug("Entering.");
	/*
	 * FIXME:  At the moment we only support COLLATION_BINARY and
	 * COLLATION_NTOFS_ULONG, so we BUG() for everything else for now.
	 */
	//BUG_ON(cr != COLLATION_BINARY && cr != COLLATION_NTOFS_ULONG); //delete by yangli
	i = le32_to_cpu(cr);
	BUG_ON(i < 0);
	if (i <= 0x02)
		return ntfs_do_collate0x0[i](vol, data1, data1_len,
				data2, data2_len);
	BUG_ON(i < 0x10);
	i -= 0x10;
	if (likely(i <= 3))
		return ntfs_do_collate0x1[i](vol, data1, data1_len,
				data2, data2_len);
	BUG();
	return 0;
}
