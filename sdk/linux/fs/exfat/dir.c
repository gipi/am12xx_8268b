/*
 * Copyright (C) 2008, OGAWA Hirofumi
 * Released under GPL v2.
 */

#include <linux/buffer_head.h>
#include "exfat.h"
#include "bitmap.h"
#include "upcase.h"
#include "debug.h"
#include "cluster.h"
#include "fatent.h"

static void pd_init(struct exfat_parse_data *pd)
{
	pd->nr_bhs = 0;
#ifdef DEBUG
	pd->bh_cnt = 0;
#endif
}

static void pd_brelse(struct exfat_parse_data *pd)
{
	while (pd->nr_bhs) {
		pd->nr_bhs--;
		brelse(pd->bhs[pd->nr_bhs]);
#ifdef DEBUG
		pd->bh_cnt--;
#endif
	}
	pd_init(pd);
}

/*得到?*/
static void *pd_get_chunk(struct exfat_parse_data *pd)
{
	return pd->bhs[0]->b_data + pd->bh_offset;
}

/*记录exfat_parse_data的状态*/
struct exfat_parse_iter_data {
	unsigned long	left;		//exfat_parse_data未被取出的数大小
	unsigned long	bh_offset;
	unsigned int	bh_index;
};

/*get the first chunk of the dentry (pd) */
static void *pd_iter_first_de(struct exfat_parse_data *pd,
			      struct exfat_parse_iter_data *pd_iter)
{
	DEBUG_ON(pd->size < EXFAT_CHUNK_SIZE, "pd->size %lu\n", pd->size);
	pd_iter->left = pd->size - EXFAT_CHUNK_SIZE;
	pd_iter->bh_offset = pd->bh_offset;
	pd_iter->bh_index = 0;
	return pd_get_chunk(pd);
}
/*get the next chunk of the dentry (pd) */
static void *pd_iter_next_de(struct exfat_parse_data *pd,
			     struct exfat_parse_iter_data *pd_iter)
{
	if (pd_iter->left < EXFAT_CHUNK_SIZE)
		return NULL;
	pd_iter->left -= EXFAT_CHUNK_SIZE;
	pd_iter->bh_offset += EXFAT_CHUNK_SIZE;
	if (pd_iter->bh_offset >= pd->bhs[pd_iter->bh_index]->b_size) {
		pd_iter->bh_index++;
		pd_iter->bh_offset = 0;
	}
	return pd->bhs[pd_iter->bh_index]->b_data + pd_iter->bh_offset;
}

enum { PARSE_NEXT, PARSE_STOP, };

struct exfat_parse_callback {
	int		(*parse)(struct inode *, loff_t,
				 struct exfat_parse_data *,
				 struct exfat_parse_callback *);
	void		*data;
};

/*解析目录*/
static int exfat_parse_dir(struct file *filp, struct inode *dir, loff_t *ppos,
			   struct exfat_parse_callback *pcb)
{
	struct super_block *sb = dir->i_sb;
	struct exfat_parse_data pd;
	struct buffer_head map_bh, *bh;
	sector_t uninitialized_var(blocknr), last_iblock;
	unsigned long blocksize, blocks, offset, de_left;
	loff_t pos;
	int ret, err;

	exfat_debug("Entering");

	blocksize = 1 << dir->i_blkbits;
	last_iblock = dir->i_size >> dir->i_blkbits;

	/* FIXME: there is this limitation? Or limitation is cluster size? */
	if (dir->i_size & (blocksize - 1)) {//目录大小与扇区大小对齐?应该是与簇对齐吧
		exfat_fs_panic(sb, "invalid directory size (size %lld)",
			       dir->i_size);
		return -EIO;
	}

	pos = *ppos;

	/* FIXME: can cache the free space for next create()/mkdir()/etc? */
	pd_init(&pd);
	err = ret = 0;
	blocks = de_left = 0;
	while (!ret && pos < dir->i_size) {		
		if (!blocks) {
			/* FIXME: readahead, probably last_iblock is too big */
			sector_t iblock;

			iblock = pos >> dir->i_blkbits;

			map_bh.b_state = 0;
			map_bh.b_blocknr = 0;
			map_bh.b_size = (last_iblock-iblock) << dir->i_blkbits;

			err = exfat_get_block(dir, iblock, &map_bh, 0);
			if (err)
				break;
			DEBUG0_ON(!buffer_mapped(&map_bh));

			blocknr = map_bh.b_blocknr;				//起始block号
			blocks = map_bh.b_size >> dir->i_blkbits;	//map的block数
		}

		offset = pos & (blocksize - 1);
		bh = sb_bread(sb, blocknr);//读取blocknr的数据
		if (!bh) {
			err = -EIO;
			break;
		}
#ifdef DEBUG
		pd.bh_cnt++;//buffer head个数
#endif
		if (de_left) {
			unsigned long len;
#ifdef DEBUG
			pd.bh_cnt++;
#endif
			get_bh(bh);
			pd.bhs[pd.nr_bhs] = bh;
			pd.nr_bhs++;

			len = min(de_left, blocksize);
			de_left -= len;
			offset += len;
			pos += len;

			//exfat_debug("%s: blocknr %llu, de_left %lu, offset %lu,"
			//       " pos %llu, len %lu\n", __func__,
			//       (llu)bh->b_blocknr, de_left, offset, pos, len);
			if (!de_left)
				goto dirent_complete;

			goto next_block;
		}

		while (offset < bh->b_size) {
			struct exfat_chunk_dirent *dirent;
			unsigned long len, chunks;

			dirent = (void *)bh->b_data + offset;//得到目录的entry

			if (dirent->type == EXFAT_TYPE_EOD) {
				pos = dir->i_size;
				break;
			} else if (!(dirent->type & EXFAT_TYPE_VALID)) {
				offset += EXFAT_CHUNK_SIZE;
				pos += EXFAT_CHUNK_SIZE;
				continue;
			}

			if (dirent->type == EXFAT_TYPE_DIRENT)
				chunks = 1 + dirent->sub_chunks;
			else
				chunks = 1;//total chunks number

			de_left = chunks << EXFAT_CHUNK_BITS;//entry总大小

			pd.size = de_left;
#ifdef DEBUG
			pd.bh_cnt++;
#endif
			get_bh(bh);
			pd.bhs[0] = bh;
			pd.nr_bhs = 1;
			pd.bh_offset = offset;

			len = min(de_left, blocksize - offset);
			de_left -= len;
			offset += len;
			pos += len;

			//exfat_debug("blocknr %llu, de_left %lu, offset %lu,"
			 //      " pos %llu, len %lu", 
			 //      (llu)bh->b_blocknr, de_left, offset, pos, len);
			if (!de_left) {
dirent_complete:
				ret = pcb->parse(dir, pos - pd.size, &pd, pcb);
				pd_brelse(&pd);
				if (ret != PARSE_NEXT) {
					if (ret < 0)
						err = ret;
					pos -= pd.size;
					break;
				}
			}
		}
next_block:
		*ppos = pos;
		brelse(bh);
#ifdef DEBUG
		pd.bh_cnt--;
#endif
		blocknr++;
		blocks--;
	}
	pd_brelse(&pd);
#ifdef DEBUG
	DEBUG_ON(pd.bh_cnt, "pd.bh_cnt %d\n", pd.bh_cnt);
#endif
	exfat_debug("Done,err:%d",err);
	return err;
}

/* FIXME: unicode string should be cached? */
static int exfat_d_revalidate(struct dentry *dentry, struct nameidata *nd)
{
	return 1;
}

static int exfat_d_hash(struct dentry *dentry, struct qstr *name)
{
	/* FIXME: can't cache converted name/hash? */
	return 0;
}

static int exfat_d_compare(struct dentry *dentry, struct qstr *a,
			   struct qstr *b)
{
	/* FIXME: can't use converted name/hash? */
	if (a->len != b->len)
		return 1;
	return memcmp(a->name, b->name, a->len);
}

static int exfat_d_delete(struct dentry *dentry)
{
	return 0;
}

static void exfat_d_release(struct dentry *dentry)
{
}

static void exfat_d_iput(struct dentry *dentry, struct inode *inode)
{
	iput(inode);
}

struct dentry_operations exfat_dentry_ops = {
	.d_revalidate	= exfat_d_revalidate,
	.d_hash		= exfat_d_hash,
	.d_compare	= exfat_d_compare,
	.d_delete	= exfat_d_delete,
	.d_release	= exfat_d_release,
	.d_iput		= exfat_d_iput,
};

static int exfat_match(struct exfat_sb_info *sbi, wchar_t *a, __le16 *b, u8 len)
{
	u8 i;
	//exfat_debug("Entering");
	for (i = 0; i < len; i++) {
		/* FIXME: assume a[i] is upper-case */
		wchar_t tmp;
		//exfat_debug("%d -- %d",a[i],le16_to_cpu(b[i]));
		if(a[i] != le16_to_cpu(b[i])){
			tmp = a[i];
			if(exfat_towupper(sbi, tmp) != le16_to_cpu(b[i])){
				if (a[i] != exfat_towupper(sbi, le16_to_cpu(b[i])))
					return 0;
			}
		}
	}
	return -1;
}

struct lookup_parse_data {
	struct inode *inode;
	wchar_t *wstr;
	u8 wlen;
	u16 hash;
};
static int ChangeName(struct exfat_sb_info *sbi ,struct qstr *qname,struct lookup_parse_data *pcb_data);

static int lookup_parse(struct inode *dir, loff_t pos,
			struct exfat_parse_data *pd,
			struct exfat_parse_callback *pcb)
{
	struct super_block *sb = dir->i_sb;
	struct exfat_sb_info *sbi = EXFAT_SB(sb);
	struct lookup_parse_data *pcb_data = pcb->data;
	struct exfat_parse_iter_data pd_iter;
	struct exfat_chunk_dirent *dirent;
	struct exfat_chunk_data *data;
	struct exfat_chunk_name *name;
	wchar_t *wstr;
	u16 csum;
	u8 len, de_namelen;

	exfat_debug("Entering");

	dirent = pd_iter_first_de(pd, &pd_iter);
	if (dirent->type != EXFAT_TYPE_DIRENT)
		return PARSE_NEXT;
	/* ->sub_chunk was already checked by caller. */
	DEBUG_ON((1 + dirent->sub_chunks) << EXFAT_CHUNK_BITS != pd->size,//目录总大小
		 "sub_chunks %u, pd->size %lu\n", dirent->sub_chunks, pd->size);
	//exfat_debug("exFAT: 0x%02x found", dirent->type);
	/* Skip dirent->checksum field */
	csum = exfat_checksum16(0, dirent, 2);//跳过checksum(0x02-0x03) 本身
	csum = exfat_checksum16(csum, (u8 *)dirent + 4, EXFAT_CHUNK_SIZE - 4);

	data = pd_iter_next_de(pd, &pd_iter);//下一个chunck:0xc0
	if (!data)
		return PARSE_NEXT;
	if (data->type != EXFAT_TYPE_DATA)
		return PARSE_NEXT;
	//exfat_debug("wlen %u, name_len %u", 
	 //      pcb_data->wlen, data->name_len);
	if (pcb_data->wlen != data->name_len)//文件或目录名长度不等
		return PARSE_NEXT;
	//exfat_debug("hash 0x%04x, checksum 0x%04x",
	//       pcb_data->hash, le16_to_cpu(data->hash));
	if (pcb_data->hash != le16_to_cpu(data->hash))//哈希值不等
		return PARSE_NEXT;
	de_namelen = data->name_len;
	csum = exfat_checksum16(csum, data, EXFAT_CHUNK_SIZE);

	wstr = pcb_data->wstr;
	while (de_namelen) {
		name = pd_iter_next_de(pd, &pd_iter);//文件名:0xc1
		if (!name)
			return PARSE_NEXT;
		if (name->type != EXFAT_TYPE_NAME)
			return PARSE_NEXT;

		len = min_t(u8, de_namelen, EXFAT_CHUNK_NAME_SIZE);
		if (!exfat_match(sbi, wstr, name->name, len))//文件名不匹配
			return PARSE_NEXT;
		wstr += len;
		de_namelen -= len;

		csum = exfat_checksum16(csum, name, EXFAT_CHUNK_SIZE);
	}

	/* Checksum of the remaining entries (may not be exfat_chunk_name) */
	while ((name = pd_iter_next_de(pd, &pd_iter)) != NULL) {
		if (!(name->type & EXFAT_TYPE_SUBCHUNK))
			return PARSE_NEXT;
		csum = exfat_checksum16(csum, name, EXFAT_CHUNK_SIZE);
	}

	if (le16_to_cpu(dirent->checksum) != csum) {
		exfat_fs_panic(sb, "checksum failed for directory entry"
			       " in lookup (0x%04x != 0x%04x)",
			       le16_to_cpu(dirent->checksum), csum);
		return -EIO;
	}

	exfat_debug("found\n");

	pcb_data->inode = exfat_iget(sb, pd, dirent, data);
	if (IS_ERR(pcb_data->inode))
		return PTR_ERR(pcb_data->inode);

	return PARSE_STOP;
}

static struct dentry *exfat_lookup(struct inode *dir, struct dentry *dentry,
				   struct nameidata *nd)
{
	struct exfat_sb_info *sbi = EXFAT_SB(dir->i_sb);
	struct exfat_parse_callback pcb;
	struct lookup_parse_data pcb_data;
	int err;
	loff_t pos;

	exfat_debug("Entering");
	
	/* FIXME: can't use converted name/hash? */
	err = ChangeName(sbi,&dentry->d_name,&pcb_data);
	if(err)
		goto error;

	pcb.parse = lookup_parse;
	pcb.data = &pcb_data;//存储文件或目录名

	/* FIXME: pos can be previous lookuped position. */
	pos = 0;
	err = exfat_parse_dir(NULL, dir, &pos, &pcb);
	if (err)
		goto error;

	kfree(pcb_data.wstr);

	/* FIXME: can cache the position for next lookup()?
	 * Assume sequential locality (stat() for all files after readdir()) */

	dentry->d_op = &exfat_dentry_ops;
	dentry = d_splice_alias(pcb_data.inode, dentry);
	if (dentry)
		dentry->d_op = &exfat_dentry_ops;
	
	exfat_debug("Done");
	return dentry;

error:
	kfree(pcb_data.wstr);
	exfat_debug("Failed");

	return ERR_PTR(err);
}

struct readdir_parse_data {
	void *dirbuf;
	filldir_t filldir;
	void *str;
};

#define EXFAT_MAX_NAME_BUFSIZE	(EXFAT_MAX_NAMELEN * NLS_MAX_CHARSET_SIZE)
#define DO_READDIR_CHECKSUM

static int readdir_parse(struct inode *dir, loff_t pos,
			 struct exfat_parse_data *pd,
			 struct exfat_parse_callback *pcb)
{
	struct super_block *sb = dir->i_sb;
	struct nls_table *nls = EXFAT_SB(sb)->opts.nls;
	struct readdir_parse_data *pcb_data = pcb->data;
	struct exfat_parse_iter_data pd_iter;
	struct exfat_chunk_dirent *dirent;
	struct exfat_chunk_data *data;
	struct exfat_chunk_name *name;
	struct inode *inode;
	unsigned long ino;
	unsigned int d_type;
#ifdef DO_READDIR_CHECKSUM
	u16 csum;
#endif
	u8 de_namelen;
	const __le16 *ustr;
	unsigned char *str;
	unsigned int uleft, str_left, str_len;
	int err;

	/* FIXME: should cache whether checksum was done */

	dirent = pd_iter_first_de(pd, &pd_iter);
	if (dirent->type != EXFAT_TYPE_DIRENT)
		return PARSE_NEXT;
	/* ->sub_chunk was already checked by caller. */
	DEBUG_ON((1 + dirent->sub_chunks) << EXFAT_CHUNK_BITS != pd->size,
		 "sub_chunks %u, pd->size %lu\n", dirent->sub_chunks, pd->size);
	//exfat_debug("0x%02x found", dirent->type);
#ifdef DO_READDIR_CHECKSUM
	/* Skip dirent->checksum field */
	csum = exfat_checksum16(0, dirent, 2);
	csum = exfat_checksum16(csum, (u8 *)dirent + 4, EXFAT_CHUNK_SIZE - 4);
#endif

	data = pd_iter_next_de(pd, &pd_iter);
	if (!data)
		return PARSE_NEXT;
	if (data->type != EXFAT_TYPE_DATA)
		return PARSE_NEXT;
	de_namelen = data->name_len;
#ifdef DO_READDIR_CHECKSUM
	csum = exfat_checksum16(csum, data, EXFAT_CHUNK_SIZE);
#endif

	if (!pcb_data->str) {
		/* FIXME: too big, should be own kmem_cachep */
		pcb_data->str = kmalloc(EXFAT_MAX_NAME_BUFSIZE, GFP_KERNEL);
		if (!pcb_data->str)
			return -ENOMEM;
	}

	str = pcb_data->str;
	str_left = EXFAT_MAX_NAME_BUFSIZE;
	while (de_namelen) {
		name = pd_iter_next_de(pd, &pd_iter);
		if (!name)
			return PARSE_NEXT;
		if (name->type != EXFAT_TYPE_NAME)
			return PARSE_NEXT;

		ustr = name->name;
		uleft = min_t(u8, de_namelen, EXFAT_CHUNK_NAME_SIZE);
		de_namelen -= uleft;

		err = exfat_conv_u2c(nls, &ustr, &uleft, &str, &str_left);
		if (err) {
			/* FIXME: if char was invalid, what to do */
			if (err == -EINVAL) {
				exfat_error("invalid char 0x%04x in"
				       " file name", le16_to_cpu(*ustr));
			}
			/* FIXME: shouldn't return -ENAMETOOLONG */
			return err;
		}
#ifdef DO_READDIR_CHECKSUM
		csum = exfat_checksum16(csum, name, EXFAT_CHUNK_SIZE);
#endif
	}

	/* Checksum of the remaining entries (may not be exfat_chunk_name) */
	while ((name = pd_iter_next_de(pd, &pd_iter)) != NULL) {
		if (!(name->type & EXFAT_TYPE_SUBCHUNK))
			return PARSE_NEXT;
#ifdef DO_READDIR_CHECKSUM
		csum = exfat_checksum16(csum, name, EXFAT_CHUNK_SIZE);
#endif
	}
#ifdef DO_READDIR_CHECKSUM
	if (le16_to_cpu(dirent->checksum) != csum) {
		exfat_fs_panic(sb, "checksum failed for directory entry"
			       " in readdir (0x%04x != 0x%04x)",
			       le16_to_cpu(dirent->checksum), csum);
		return -EIO;
	}
#endif

	inode = exfat_ilookup(sb, pd->bhs[0]->b_blocknr, pd->bh_offset);
	if (inode) {
		ino = inode->i_ino;
		iput(inode);
	} else
		ino = iunique(sb, EXFAT_RESERVED_INO);

	if (dirent->attrib & cpu_to_le16(EXFAT_ATTR_DIR))
		d_type = DT_DIR;
	else
		d_type = DT_REG;

	str_len = EXFAT_MAX_NAME_BUFSIZE - str_left;

	str[0] = '\0';
	//exfat_debug("ent %p, str %s, len %u, pos %lld, ino %lu, type 0x%x\n",
	 //       pcb_data->dirbuf, (char *)pcb_data->str, str_len,
	//       pos, ino, d_type);

	if (pcb_data->filldir(pcb_data->dirbuf, pcb_data->str, str_len, pos,
			      ino, d_type))
		return PARSE_STOP;
	//exfat_debug("Done");
	return PARSE_NEXT;
}

static int exfat_readdir(struct file *filp, void *dirbuf, filldir_t filldir)
{
	struct inode *dir = filp->f_path.dentry->d_inode;
	struct exfat_parse_callback pcb;
	struct readdir_parse_data pcb_data;
	loff_t pos;
	int err;

	//exfat_debug("Entering");
	/* exFAT doesn't have "." and "..", so returns fake entries */
	while (filp->f_pos < 2) {
		unsigned long ino;
		int len = filp->f_pos + 1;

		if (filp->f_pos == 0)
			ino = dir->i_ino;
		else
			ino = parent_ino(filp->f_path.dentry);

		exfat_debug("ent %p, .., len %u, pos %lld, ino %lu, type 0x%x\n",
		        dirbuf, len, filp->f_pos, ino, DT_DIR);
		if (filldir(dirbuf, "..", len, filp->f_pos, ino, DT_DIR))
			return 0;

		filp->f_pos++;
	}
	if (filp->f_pos > 2 && (filp->f_pos & (EXFAT_CHUNK_SIZE - 1)))
		return -ENOENT;

	pcb_data.dirbuf = dirbuf;
	pcb_data.filldir = filldir;
	pcb_data.str = NULL;
	pcb.parse = readdir_parse;
	pcb.data = &pcb_data;

	pos = filp->f_pos & ~(loff_t)(EXFAT_CHUNK_SIZE - 1);
	err = exfat_parse_dir(filp, dir, &pos, &pcb);
	if (pos < 2)
		filp->f_pos = 2;
	else
		filp->f_pos = pos;

	kfree(pcb_data.str);

	return err;
}

struct rootdir_parse_data {
	u32 bitmap_clusnr;	//bitmap起始簇号
	u64 bitmap_size;		//bitmap总大小
	u32 upcase_checksum;
	u32 upcase_clusnr;	//upcase起始簇号
	u64 upcase_size;		//upcase总大小
};

static int rootdir_parse(struct inode *dir, loff_t pos,
			 struct exfat_parse_data *pd,
			 struct exfat_parse_callback *pcb)
{
	struct exfat_chunk_bitmap *bitmap;
	struct exfat_chunk_upcase *upcase;
	struct rootdir_parse_data *pcb_data = pcb->data;

	bitmap = pd_get_chunk(pd);//获取该entry的数据
	switch (bitmap->type) {
	case EXFAT_TYPE_BITMAP:
		if (pcb_data->bitmap_clusnr) {//之前就有bitmap了?
			/* FIXME: for this, what do we do */
			exfat_waring("exFAT: found another"
			       " free space bitmap, ignored");
			break;
		}
		pcb_data->bitmap_clusnr = le32_to_cpu(bitmap->clusnr);
		pcb_data->bitmap_size = le64_to_cpu(bitmap->size);
		break;
	case EXFAT_TYPE_UPCASE:
		if (pcb_data->upcase_clusnr) {
			/* FIXME: for this, what do we do */
			exfat_waring("exFAT: found another"
			       " upper-case table, ignored");
			break;
		}
		upcase = pd_get_chunk(pd);
		pcb_data->upcase_checksum = le32_to_cpu(upcase->checksum);
		pcb_data->upcase_clusnr = le32_to_cpu(upcase->clusnr);
		pcb_data->upcase_size = le64_to_cpu(upcase->size);
		break;
	}

	return PARSE_NEXT;
}

int exfat_read_rootdir(struct inode *dir)
{
	struct super_block *sb = dir->i_sb;
	struct exfat_parse_callback pcb;
	struct rootdir_parse_data pcb_data;
	loff_t pos;
	int err;

	pcb_data.bitmap_clusnr = 0;
	pcb_data.upcase_clusnr = 0;
	pcb.parse = rootdir_parse;//解析根目录
	pcb.data = &pcb_data;//解析出的数据

	pos = 0;
	err = exfat_parse_dir(NULL, dir, &pos, &pcb);
	if (err)
		return err;

	err = exfat_setup_bitmap(sb, pcb_data.bitmap_clusnr,
				 pcb_data.bitmap_size);
	if (err)
		return err;

	err = exfat_setup_upcase(sb, pcb_data.upcase_checksum,
				 pcb_data.upcase_clusnr,
				 pcb_data.upcase_size);
	if (err) {
		exfat_free_bitmap(EXFAT_SB(sb));
		return err;
	}

	return 0;
}

#ifdef EXFAT_MODIFY

/* on-disk nodes iterator */
struct iterator
{
	sector_t blocknr;
	sector_t  cluster;
	sector_t  iclustnr;
	off_t offset;
	int contiguous;
	char* chunk;
};

struct erase_entry
{
	sector_t blocknr;
	off_t offset;
};
int  init_it(struct inode  *inode, struct iterator *it)
{
	struct exfat_inode_info *exi = EXFAT_I(inode);
	struct exfat_sb_info *sbi = EXFAT_SB(inode->i_sb);
	struct buffer_head *bh;
	
	it->cluster = exi->clusnr;
	it->blocknr = exfat_clus_to_blknr(sbi, it->cluster) ;
	it->iclustnr = 0;
	it->offset = 0;
	it->contiguous = 0;
	it->chunk = kmalloc(inode->i_sb->s_blocksize,GFP_KERNEL);
	if(!it->chunk)
		exfat_error("kmalloc failed ");
	bh = sb_bread(inode->i_sb, it->blocknr);
	if (!bh) {
		exfat_error("unable to read inode block "
		       "for updating (blocknr 0x%llx)\n",  it->blocknr);
		return -EIO;
	}
	memcpy(it->chunk,bh->b_data,bh->b_size);
	brelse(bh);
	return 0;
}

void free_it(struct iterator *it)
{
	it->cluster = 0;
	it->offset = 0;
	it->contiguous = 0;
	it->iclustnr = 0;
	kfree(it->chunk);
	it->chunk = NULL;
}
static void ere_init(struct erase_entry *ere)
{
	ere->blocknr = 0;
	ere->offset = 0;
}
static void ed_init(struct exfat_entry_data *ed)
{	
	ed->nr_bnr= 0;
	ed->size = 0;	
	memset(ed->bnr,0, EXFAT_DE_MAX_BH*sizeof(sector_t));
	ed->bh_offset = 0;
}

static void ed_reflesh(struct exfat_entry_data *ed,sector_t blocknr)
{	
	ed->bnr[ed->nr_bnr] = blocknr;
	ed->nr_bnr++;			
}

static void ed_brelse(struct exfat_entry_data *ed)
{
	ed->size = 0;
	ed->bh_offset = 0;
}

/*  dividend divisor result
 *    0           512     1
 *    1           512     1
 *    512       512     1
 *    513       512     2
*/
u64 exfat_div2(u64 dividend, u32 divisor)
{
	u32 remainder;
	u64 result;
	
	result  = div_u64_rem(dividend,divisor, &remainder);
	if(remainder)
		result ++;
	return result;
}
/*  dividend divisor result
 *    0           512     0
 *    1           512     1
 *    512       512     1
 *    513       512     2
*/
u64 exfat_div_roundup(u64 dividend, u64 divisor)
{
	return div64_u64(dividend+divisor-1,divisor);
}

static int ChangeName(struct exfat_sb_info *sbi ,struct qstr *qname,struct lookup_parse_data *pcb_data)
{
	wchar_t *wstr;
	unsigned int i, name_left, wlen, wleft;
	const unsigned char *name;
	struct nls_table *nls = sbi->opts.nls;
	int err = 0;
	
	//exfat_debug("qname->len : %d",qname->len);
	//exfat_Print_Buf("qname->name",(unsigned char *)qname->name,qname->len*2);

	wlen = wleft = min(qname->len, EXFAT_MAX_NAMELEN);
	pcb_data->wstr = wstr = kmalloc(wlen * sizeof(*wstr), GFP_KERNEL);
	if (!wstr) 	
		return-ENOMEM;	

	name = qname->name;
	name_left = qname->len;
	/* FIXME: maybe toupper and hash should be computed at same time */
//	err = exfat_conv_c2u_for_cmp(nls, &name, &name_left, &wstr, &wleft);
	err = exfat_conv_c2u(nls, &name, &name_left, &wstr, &wleft);//将文件名转为unicode码
	/* FIXME: shouldn't return -ENAMETOOLONG? */
	if (err)
		return err;
	pcb_data->wlen = wlen - wleft;
	pcb_data->hash = 0;
	for (i = 0; i < pcb_data->wlen; i++) {
		__le16 uc;
		wchar_t tmp;
		tmp = pcb_data->wstr[i];
		tmp = exfat_towupper(sbi,tmp);
		//pcb_data->wstr[i] = exfat_towupper(sbi, pcb_data->wstr[i]);//转为大写字母?
		//uc = cpu_to_le16(pcb_data->wstr[i]);
		uc =cpu_to_le16(tmp);
		pcb_data->hash = exfat_checksum16(pcb_data->hash, &uc, sizeof(uc));
	}
	pcb_data->inode = NULL;
	exfat_debug("pcb_data.wlen : %d",pcb_data->wlen);
	exfat_Print_Buf("pcb_data.wstr",(unsigned char *)pcb_data->wstr,pcb_data->wlen*2);
	return 0;
}

static int find_parse(struct inode *dir, loff_t pos,
			struct exfat_parse_data *pd,
			struct lookup_parse_data *pcb_data)
{
	struct super_block *sb = dir->i_sb;
	struct exfat_sb_info *sbi = EXFAT_SB(sb);
	struct exfat_parse_iter_data pd_iter;
	struct exfat_chunk_dirent *dirent;
	struct exfat_chunk_data *data;
	struct exfat_chunk_name *name;
	wchar_t *wstr;
	u16 csum;
	u8 len, de_namelen;

	exfat_debug("Entering");

	dirent = pd_iter_first_de(pd, &pd_iter);
	if (dirent->type != EXFAT_TYPE_DIRENT)
		return PARSE_NEXT;
	/* ->sub_chunk was already checked by caller. */
	DEBUG_ON((1 + dirent->sub_chunks) << EXFAT_CHUNK_BITS != pd->size,//目录总大小
		 "sub_chunks %u, pd->size %lu\n", dirent->sub_chunks, pd->size);
	//exfat_debug("exFAT: 0x%02x found", dirent->type);
	/* Skip dirent->checksum field */
	csum = exfat_checksum16(0, dirent, 2);//跳过checksum(0x02-0x03) 本身
	csum = exfat_checksum16(csum, (u8 *)dirent + 4, EXFAT_CHUNK_SIZE - 4);

	data = pd_iter_next_de(pd, &pd_iter);//下一个chunck:0xc0
	if (!data)
		return PARSE_NEXT;
	if (data->type != EXFAT_TYPE_DATA)
		return PARSE_NEXT;
	//exfat_debug("wlen %u, name_len %u", 
	 //      pcb_data->wlen, data->name_len);
	if (pcb_data->wlen != data->name_len)//文件或目录名长度不等
		return PARSE_NEXT;
	//exfat_debug("hash 0x%04x, checksum 0x%04x",
	//       pcb_data->hash, le16_to_cpu(data->hash));
	if (pcb_data->hash != le16_to_cpu(data->hash))//哈希值不等
		return PARSE_NEXT;
	de_namelen = data->name_len;
	csum = exfat_checksum16(csum, data, EXFAT_CHUNK_SIZE);

	wstr = pcb_data->wstr;
	while (de_namelen) {
		name = pd_iter_next_de(pd, &pd_iter);//文件名:0xc1
		if (!name)
			return PARSE_NEXT;
		if (name->type != EXFAT_TYPE_NAME)
			return PARSE_NEXT;

		len = min_t(u8, de_namelen, EXFAT_CHUNK_NAME_SIZE);
		if (!exfat_match(sbi, wstr, name->name, len))//文件名不匹配
			return PARSE_NEXT;
		wstr += len;
		de_namelen -= len;

		csum = exfat_checksum16(csum, name, EXFAT_CHUNK_SIZE);
	}

	/* Checksum of the remaining entries (may not be exfat_chunk_name) */
	while ((name = pd_iter_next_de(pd, &pd_iter)) != NULL) {
		if (!(name->type & EXFAT_TYPE_SUBCHUNK))
			return PARSE_NEXT;
		csum = exfat_checksum16(csum, name, EXFAT_CHUNK_SIZE);
	}

	if (le16_to_cpu(dirent->checksum) != csum) {
		exfat_fs_panic(sb, "checksum failed for directory entry"
			       " in lookup (0x%04x != 0x%04x)",
			       le16_to_cpu(dirent->checksum), csum);
		return -EIO;
	}

	exfat_debug("found\n");
	return PARSE_STOP;
}

/*return 1: not found; 0: found; others :error */
static int exfat_find(struct inode *dir, struct qstr *qname)
{
	struct exfat_parse_data pd;
	struct super_block *sb = dir->i_sb;
	struct exfat_sb_info *sbi = EXFAT_SB(sb);
	struct lookup_parse_data pcb_data;
	struct buffer_head map_bh, *bh;
	sector_t uninitialized_var(blocknr), last_iblock;
	unsigned long blocksize, blocks, offset, de_left;
	loff_t pos;
	int ret, err;

	exfat_debug("Entering");

	/* FIXME: can't use converted name/hash? */
	err = ChangeName(sbi,qname,&pcb_data);
	if(err)
		goto error;

	blocksize = 1 << dir->i_blkbits;
	last_iblock = dir->i_size >> dir->i_blkbits;

	/* FIXME: there is this limitation? Or limitation is cluster size? */
	if (dir->i_size & (blocksize - 1)) {//目录大小与扇区大小对齐?应该是与簇对齐吧
		exfat_fs_panic(sb, "invalid directory size (size %lld)",
			       dir->i_size);
		return -EIO;
	}

	pos = 0;
	pd_init(&pd);
	/* FIXME: can cache the free space for next create()/mkdir()/etc? */	
	err = ret = 0;
	blocks = de_left = 0;
	while (!ret && pos < dir->i_size) {		
		if (!blocks) {
			/* FIXME: readahead, probably last_iblock is too big */
			sector_t iblock;

			iblock = pos >> dir->i_blkbits;

			map_bh.b_state = 0;
			map_bh.b_blocknr = 0;
			map_bh.b_size = (last_iblock-iblock) << dir->i_blkbits;

			err = exfat_get_block(dir, iblock, &map_bh, 0);
			if (err){
				exfat_debug("get block err!");
				break;
			}
			DEBUG0_ON(!buffer_mapped(&map_bh));

			blocknr = map_bh.b_blocknr;				//起始block号
			blocks = map_bh.b_size >> dir->i_blkbits;	//map的block数
		}

		offset = pos & (blocksize - 1);
		bh = sb_bread(sb, blocknr);//读取blocknr的数据
		if (!bh) {
			err = -EIO;
			break;
		}
#ifdef DEBUG
		pd.bh_cnt++;//buffer head个数
#endif
		if (de_left) {
			unsigned long len;
#ifdef DEBUG
			pd.bh_cnt++;
#endif
			get_bh(bh);
			pd.bhs[pd.nr_bhs] = bh;
			pd.nr_bhs++;

			len = min(de_left, blocksize);
			de_left -= len;
			offset += len;
			pos += len;

			//exfat_debug("%s: blocknr %llu, de_left %lu, offset %lu,"
			//       " pos %llu, len %lu\n", __func__,
			//       (llu)bh->b_blocknr, de_left, offset, pos, len);
			if (!de_left)
				goto dirent_complete;

			goto next_block;
		}

		while (offset < bh->b_size) {
			struct exfat_chunk_dirent *dirent;
			unsigned long len, chunks;

			dirent = (void *)bh->b_data + offset;//得到目录的entry

			if (dirent->type == EXFAT_TYPE_EOD) {
				pos = dir->i_size;
				break;
			} else if (!(dirent->type & EXFAT_TYPE_VALID)) {
				offset += EXFAT_CHUNK_SIZE;
				pos += EXFAT_CHUNK_SIZE;
				continue;
			}

			if (dirent->type == EXFAT_TYPE_DIRENT)
				chunks = 1 + dirent->sub_chunks;
			else
				chunks = 1;//total chunks number

			de_left = chunks << EXFAT_CHUNK_BITS;//entry总大小

			pd.size = de_left;
#ifdef DEBUG
			pd.bh_cnt++;
#endif
			get_bh(bh);
			pd.bhs[0] = bh;
			pd.nr_bhs = 1;
			pd.bh_offset = offset;

			len = min(de_left, blocksize - offset);
			de_left -= len;
			offset += len;
			pos += len;

			//exfat_debug("blocknr %llu, de_left %lu, offset %lu,"
			 //      " pos %llu, len %lu", 
			 //      (llu)bh->b_blocknr, de_left, offset, pos, len);
			if (!de_left) {
dirent_complete:
				ret = find_parse(dir, pos - pd.size, &pd, &pcb_data);		
				pd_brelse(&pd);
				if (ret != PARSE_NEXT) {
					if (ret < 0)
						err = ret;
					pos -= pd.size;
					break;
				}
				
			}
		}
next_block:
		brelse(bh);
#ifdef DEBUG
		pd.bh_cnt--;
#endif
		blocknr++;
		blocks--;
	}
	
	pd_brelse(&pd);
#ifdef DEBUG
	DEBUG_ON(pd.bh_cnt, "pd.bh_cnt %d\n", pd.bh_cnt);
#endif

error:
	kfree(pcb_data.wstr);
	exfat_debug("Done,err :%d",err);

	return err;

}
/*@difference: the number of culsteres needed to free*/
static  int exfat_shrink_file(struct inode *inode, sector_t cur_clus, sector_t difference)
{
	struct exfat_inode_info *exi = EXFAT_I(inode);
	struct exfat_clus_map cmap;
	u32  previous,inext;	
	int ret = 0;
	u32 FreeNUM =0;
	struct exfat_ent exfat_ent;			

	exfat_debug("Entering,current :0x%llx,difference: 0x%llx ",cur_clus,difference);
	if (difference == 0){
		exfat_debug("zero difference passed");
		return -EIO;
	}
	if (!exi->clusnr){
		exfat_debug("unable to shrink empty file");
		return -EIO;
	}
	if (cur_clus < difference)		
		exfat_error("file underflow (%llu < %llu), all the clusters are going to free", cur_clus, difference);		

	/* crop the file */
	if (cur_clus > difference)
	{		
		/*get the (current-difference) culster*/
		exfat_get_cluster(inode,cur_clus - difference,1,&cmap);

		/*set the (current-difference) culster to be 0XFFFFFFFFr*/
		if(exi->data_flag != EXFAT_DATA_CONTIGUOUS){	
			exfat_debug("the data is not contiguous ");
			exfat_ent_init(&exfat_ent);
			ret = exfat_ent_read(inode, &exfat_ent, cmap.clusnr, &previous);
			if (ret >= 0) {				
				ret = exfat_ent_write(inode, &exfat_ent, EXFAT_ENT_EOF);
				exfat_ent_release(&exfat_ent);
			}
		}	
		inext = cur_clus - difference+1;
		FreeNUM = difference;
	}
	else/*no enough clusters to free, just free all the culsters */
	{
		previous = exi->clusnr;
		//exi->clusnr = 0;
		inext = 0;		
		FreeNUM = cur_clus;
	}	

	/* free remaining clusters */
	while (FreeNUM--)
	{	
		exfat_debug("inext : 0x%x, FreeNUM : 0x%x ",inext,FreeNUM);
		exfat_get_cluster(inode,inext,1,&cmap);	 

		if(exi->data_flag == EXFAT_DATA_CONTIGUOUS){
			exfat_free_clusters(inode,FreeNUM+1 ,cmap.clusnr);
			break;
		}
		else
			exfat_free_clusters(inode,1,cmap.clusnr);
		
		inext++;
		/*set the (current-difference culster to be the end cluster*/
		if(exi->data_flag != EXFAT_DATA_CONTIGUOUS){		
			exfat_ent_init(&exfat_ent);
			ret = exfat_ent_read(inode, &exfat_ent, cmap.clusnr, &previous);
			if (ret >= 0) {				
				ret = exfat_ent_write(inode, &exfat_ent, EXFAT_ENT_FREE);
				exfat_ent_release(&exfat_ent);
			}
		}				
	}
	exfat_debug("Done");
	return 0;
}

/*@difference: the number of culsteres needed to allocate*/
static int exfat_grow_file(struct inode *inode, sector_t cur_clus, sector_t  difference)
{
	struct super_block *sb = inode->i_sb;
	struct exfat_sb_info *sbi = EXFAT_SB(sb);
	struct exfat_inode_info *exi = EXFAT_I(inode);
	u32 last_iclus,last_dclus;
	sector_t allocated = 0;
	int err = 0;

	exfat_debug("Entering");
	
	if (difference == 0){
		exfat_debug("zero clusters count passed");
		return -EIO;
	}
	if(cur_clus > difference){
		exfat_debug("file overflow (%llu > %llu)", cur_clus, difference);
			return -EIO;
	}
	
	if (exi->clusnr)
	{
		/* get the last cluster of the file */
		err = exfat_get_last_cluster(inode,&last_iclus,&last_dclus);
		if(err)
			return err;	
		if(!exfat_ent_valid(sbi,last_dclus) || exfat_ent_bad(sbi,last_dclus)){
			exfat_error("invalid cluster or bad cluster");
			return -EIO;
		}
	}
	else
	{	
		exfat_debug("file does not have clusters (i.e. is empty), allocate the first one for it ");		
		err  = exfat_add_cluster(inode);			
		allocated = 1;		
	}

	while (allocated < difference)
	{
		 err  = exfat_add_cluster(inode);		
		 if(err)
		 	return err;		
		allocated++;
	}	
	exfat_debug("Done,");
	return 0;
}


/*以簇为单位*/
int exfat_truncate(struct inode *inode, u64 new_size)
{
	struct super_block *sb = inode->i_sb;
	struct exfat_sb_info *sbi = EXFAT_SB(sb);
	struct exfat_inode_info *exi = EXFAT_I(inode);
	u64 old_size;
	int rc = 0;
	sector_t c1,c2;

	old_size =  i_size_read(inode);
	exfat_debug("Entering, new_size :0x%llx, old_size :0x%llx",new_size,old_size);	

	c1 = exfat_div_roundup(old_size,sbi->clus_size);
	c2 = exfat_div_roundup(new_size,sbi->clus_size);
	exfat_debug("c1 :0x%llx, c2: 0x%llx ",c1,c2);
	if(c1 == c2){		
		exfat_debug("no size change");
		return 0;
	}
	else if(c1 >c2)
		rc= exfat_shrink_file(inode,c1,c1-c2);
	else
		rc = exfat_grow_file(inode,c1,c2-c1);
	
/*	rc = exfat_erase_range();
	if (rc != 0)
		return rc;
*/	
	if(rc)
		return rc;
	i_size_write(inode,new_size);
	exi->phys_size = new_size;
	mark_inode_dirty(inode);	
	
	exfat_debug("Done");
	return 0;
}

void exfat_truncate_vfs(struct inode *inode) {
	//exfat_truncate(inode);
	exfat_debug("sorry, not finished yet");
}
static int grow_directory(struct inode * dir,	uint64_t asize, uint32_t difference)
{
	struct exfat_sb_info *sbi = EXFAT_SB(dir->i_sb);

	return exfat_truncate(dir,
			exfat_div_roundup(asize + difference, sbi->clus_size)
				*  sbi->clus_size);
}

/*shrink father directory when delete child file or dir */
static int shrink_directory(struct inode *dir,struct erase_entry last_ere)
{	
	struct super_block *sb = dir->i_sb;
	struct exfat_sb_info *sbi = EXFAT_SB(sb);
	struct buffer_head *bh =NULL,map_bh;
	off_t deleted_offset ;	
	uint64_t entries = 1; /* a directory always has at leat 1 entry (EOD) */
	uint64_t new_size;
	struct exfat_chunk eod;
	u32 eod_offset;
	off_t off = 0;
	int result = 0,err=0;	
	sector_t blocknr =0,i = 0,j=0r;
	unsigned long block_size = dir->i_sb->s_blocksize;
	
	exfat_debug("Entering,dir->size: 0x%llx",dir->i_size);

	deleted_offset =last_ere.offset;
	if(deleted_offset > dir->i_size){
		exfat_error("deleted file size overflows!");
		return -1;
	}
	
	for(j=0;j<exfat_div_roundup(dir->i_size,block_size);j++)
	{		
		map_bh.b_state = 0;
		map_bh.b_blocknr = 0;
		map_bh.b_size = 1 << dir->i_blkbits;
		err= exfat_get_block(dir, j, &map_bh,0);
		if(err)
			goto out; 
		blocknr = map_bh.b_blocknr;				//起始block号		
		exfat_debug("j: 0x%llx,blocknr: 0x%llx",j,blocknr);
		if(blocknr == last_ere.blocknr)	/*the deleted file is in the j block of the dir */
			break;
	}
	if(j>=exfat_div_roundup(dir->i_size,block_size)){
		exfat_error("the entry is not in the dir");
		return -1;
	}
	i = j;		
	
	/*begin from the delteted_offset*/
	off =deleted_offset;
	while (j <exfat_div_roundup(dir->i_size,block_size)) {/*to check if there are other entries behind the deleted one*/
		unsigned char flag;
		
		exfat_debug("j: 0x%llx,blocknr: 0x%llx,off: 0x%lx",j,blocknr,off);
		bh = sb_bread(dir->i_sb,blocknr);
		if (!bh) {
			exfat_error("unable to read inode block "
			       "for updating (blocknr 0x%llx)\n", blocknr);
			return -EIO;
		}
		for( ;off< block_size;off+= EXFAT_CHUNK_SIZE)//offset += EXFAT_CHUNK_SIZE)
		{
			//exfat_debug("j: 0x%llx,blocknr: 0x%llx,off: 0x%lx",j,blocknr,off);
			//_Print_Buf("buf",bh->b_data,bh->b_size);
			flag = bh->b_data[off];
			//exfat_debug("flag : 0x%x ",flag);
			if((flag & EXFAT_TYPE_VALID) || (flag == __EXFAT_TYPE_LABLE))		
			{
				result = 1;
				exfat_debug("there are other entries after the removed one no way to shrink this directory ");
				brelse(bh);
				return result;;
			}			
		}		
		j++;
		map_bh.b_state = 0;
		map_bh.b_blocknr = 0;
		map_bh.b_size = 1 << dir->i_blkbits;
		exfat_get_block(dir, j,&map_bh,0);
		blocknr =map_bh.b_blocknr;
		off = 0;
		
	}	
	BUG_ON(!bh);	
	if (i)
	{	/* offset of the last entry */	
		entries = deleted_offset /EXFAT_CHUNK_SIZE;		
		entries +=div_u64( i * block_size,EXFAT_CHUNK_SIZE);
	}else{/*the deleted entry is the first entry*/
		entries  =1;
	}
	
	exfat_debug("entries :0x%llx",entries);
	new_size = exfat_div_roundup(entries * EXFAT_CHUNK_SIZE,
				sbi->clus_size) * sbi->clus_size;
	result = exfat_truncate(dir, new_size);
	if (result != 0){
		exfat_error("exfat_truncate err!");
		goto out;
	}

	/* put EOD entry at the end of the last cluster */
	memset(&eod, 0,EXFAT_CHUNK_SIZE);
	div_u64_rem(new_size,dir->i_sb->s_blocksize,&eod_offset);
	exfat_debug("eod_offset :0x%x",eod_offset);
	if (i)
		memcpy(bh->b_data+eod_offset,&eod, EXFAT_CHUNK_SIZE);
	else
		memcpy(bh->b_data,&eod, EXFAT_CHUNK_SIZE);
	exfat_debug("Done");
	mark_buffer_dirty(bh);			
	sync_dirty_buffer(bh);	
out:
	brelse(bh);
	return result;
}


static int fetch_next_entry(struct inode* dir,struct iterator *it)
{
	struct exfat_sb_info *sbi = EXFAT_SB(dir->i_sb);
	int err= 0 ;	
	struct exfat_clus_map cmap;	
	struct buffer_head * bh;
	
	//exfat_debug("Entering");
	/* move iterator to the next entry in the directory */
	it->offset+= EXFAT_CHUNK_SIZE;
	/* fetch the next cluster if needed */
	if((it->offset & (dir->i_sb->s_blocksize -1))==0){
		if ((it->offset & ( sbi->clus_size- 1)) == 0)
		{
			it->iclustnr ++;
			err =  exfat_get_cluster(dir,it->iclustnr,1,&cmap);
			if(err)
				return err;
			it->cluster =cmap.clusnr;
			it->blocknr = exfat_clus_to_blknr(sbi, it->cluster) ;
			it->offset = 0;
		}
		else
			it->blocknr = it->blocknr +1;
		bh = sb_bread(dir->i_sb, it->blocknr);
		if (!bh) {
			exfat_error("unable to read inode block "
			       "for updating (blocknr 0x%llx)\n",  it->blocknr);
			return -EIO;
		}
		memcpy(it->chunk,bh->b_data,bh->b_size);
		brelse(bh);
	}

	//exfat_debug("Done");
	return 0;
}
/*@iclustnr: loginc cluster number
 * @ dluster: phsyic cluster numer
 * @offset: the offset in the founded @dcluser
 */
static int find_slot( struct inode* dir,u32* iclustnr,sector_t  *dcluster, off_t* offset, int subentries)
{
	struct exfat_sb_info *sbi = EXFAT_SB(dir->i_sb);
	struct exfat_chunk* entry;
	int contiguous = 0;		/*the number of contiguous free entries*/
	int rc;
	struct iterator it;

	exfat_debug("Entering");
	
	*iclustnr = 0,
	*dcluster = 0;
	*offset = 0;
	
	init_it(dir,&it);
	for(;;)
	{		
		if (contiguous == 0)
		{
			*iclustnr = it.iclustnr;
			*dcluster = it.cluster;
			*offset = it.offset;
			//exfat_debug( "it.offset :0x%lx ,it.cluster :0x%llx , it.iclustnr :0x%llx", it.offset,it.cluster, it.iclustnr);
		}
		entry = (struct exfat_chunk*)(it.chunk + it.offset % dir->i_sb->s_blocksize);
	
		if (entry->type == EXFAT_TYPE_EOD)
		{
			rc = grow_directory(dir,
					it.iclustnr * sbi->clus_size +it.offset+ sizeof(struct exfat_chunk), /* actual size */
					(subentries - contiguous) * sizeof(struct exfat_chunk));
			if (rc){
				free_it(&it);
				return rc;
			}
			break;
		}
		if (entry->type & EXFAT_TYPE_VALID)
			contiguous = 0;
		else
			contiguous++;
		if (contiguous == subentries)
			break;	/* suitable slot it found */		
		if(fetch_next_entry(dir,&it) != 0)
		{
			free_it(&it);
			return -EIO;
		}	
	}
	
	free_it(&it);
	//exfat_debug("Done");	
	return 0;

}

int next_entry(struct inode* dir,u32 *iclusnr, sector_t * cluster, off_t* offset)
{
	struct exfat_sb_info *sbi = EXFAT_SB(dir->i_sb);
	struct exfat_clus_map cmap;	
	int err;

	//exfat_debug("Entering");

	*offset += EXFAT_CHUNK_SIZE;
	if (*offset % sbi->clus_size == 0){
		/* next cluster cannot be invalid */
		err = exfat_get_cluster(dir, *iclusnr++, 1, &cmap);
		if(err)
			return err;
		*cluster = cmap.clusnr;
		*offset = 0;
	}
	exfat_debug("next_entry offset : 0x%lx ",*offset);
	return 0;
	
}
static int write_entries(struct inode* dir, struct lookup_parse_data * pcb_data,
					uint16_t attrib,sector_t  dcluster,struct exfat_entry_data *ed,char is_dir)
{
	struct exfat_sb_info *sbi = EXFAT_SB(dir->i_sb);
	struct exfat_chunk_dirent *dirent;
	struct exfat_chunk_data  *data;
	int name_entries ;
	int i,err=0;
	u16 csum;
	struct timespec ctime;
	sector_t blocknr, pre_blocknr, cluster;
	off_t offset;
	struct buffer_head *bh, *bh_next;
	u32 iclusnr = 0;
	u8 name_len;
	unsigned long block_size =dir->i_sb->s_blocksize;
	
	exfat_debug("Entering");

	/*the number of 0xc1 chunkes*/
	name_entries = DIV_ROUND_UP(pcb_data->wlen, EXFAT_CHUNK_NAME_SIZE);

	err = find_slot(dir,&iclusnr, &cluster, &offset,name_entries+2);
	exfat_debug("iclusnr : 0x%x, cluster: 0x%llx ,offset :0x%lx ",iclusnr,cluster,offset);
	if(err)
		goto err_out;
	dirent = kmalloc(EXFAT_CHUNK_SIZE,GFP_KERNEL);
	if (!dirent){
		exfat_debug("kmalloc err ");
		goto err_out;
	}
	data = kmalloc(EXFAT_CHUNK_SIZE,GFP_KERNEL);
	if (!data){
		exfat_debug("kmalloc err ");
		goto err_out;
	}
	
	blocknr = exfat_clus_to_blknr(sbi, cluster) + div64_u64(offset ,block_size);
	/*************set the 0x85 chunk********************/
	bh = sb_bread(dir->i_sb, blocknr);
	if (!bh) {
		exfat_error("unable to read inode block "
		       "for updating (blocknr 0x%llx)\n", blocknr);
		goto err_out;
	}
	ed_reflesh(ed, blocknr);
	ed->bh_offset =  offset % block_size;

	exfat_debug("ed->bh_offset : 0x%lx, blocknr :0x%llx",ed->bh_offset,blocknr);
	
	memset(dirent, 0, EXFAT_CHUNK_SIZE);
	dirent->type = EXFAT_TYPE_DIRENT;
	dirent->sub_chunks = 1 + name_entries;
	dirent->attrib = cpu_to_le16(attrib);
	//exfat_debug("next");

	/* crtime_cs and mtime_cs contain addition to the time in centiseconds;
	  just ignore those fields because we operate with 2 sec resolution */
	 ctime = current_kernel_time() ;
	exfat_time_unix2exfat(&ctime, &dirent->crtime, &dirent->crdate,&dirent->crtime_cs);
	exfat_time_unix2exfat(&ctime, &dirent->mtime, &dirent->mdate,&dirent->mtime_cs);
	exfat_time_unix2exfat(&ctime, &dirent->atime, &dirent->adate, NULL);
	/* Skip dirent->checksum field */
	csum = exfat_checksum16(0, dirent, 2);
	csum = exfat_checksum16(csum, (u8 *)dirent + 4, EXFAT_CHUNK_SIZE - 4);

	//exfat_debug("next");

	/*************set the 0xc0 chunk********************/
	memset(data, 0, EXFAT_CHUNK_SIZE);
	data->type = EXFAT_TYPE_DATA;	
	data->name_len = pcb_data->wlen;/*unicode name length*/
	data->hash = pcb_data->hash;
	if(is_dir){
		data->clusnr =cpu_to_le32(dcluster);
		data->size =data->size2 = cpu_to_le64(sbi->clus_size);
		data->flag = EXFAT_DATA_CONTIGUOUS;
	}else{
		data->clusnr = cpu_to_le32(EXFAT_ENT_FREE);
		data->size = data->size2 =cpu_to_le64(0);
		data->flag = EXFAT_DATA_NORMAL;
	}
	csum = exfat_checksum16(csum, data, EXFAT_CHUNK_SIZE);

	//exfat_debug("next");

	pre_blocknr = blocknr;
	next_entry(dir,&iclusnr, &cluster ,&offset);
	blocknr = exfat_clus_to_blknr(sbi, cluster) + div64_u64(offset ,block_size);
	exfat_debug("blocknr :0x%llx,offset:0x%lx",blocknr,offset);

	if(pre_blocknr != blocknr){
		bh_next = sb_bread(dir->i_sb, blocknr);		
		if (!bh_next) {
			exfat_error("unable to read inode block "
			       "for updating (blocknr 0x%llx)\n", blocknr);
			goto err_out;
		}
		ed_reflesh(ed, blocknr);
		memcpy(bh_next->b_data+ (offset % block_size) ,data,EXFAT_CHUNK_SIZE);	
	}
	else{
		memcpy(bh->b_data+ (offset % block_size) ,data,EXFAT_CHUNK_SIZE);	
	}
	/*************set the 0xc1 chunkes********************/
	name_len = pcb_data->wlen;
	for (i = 0; i < name_entries; i++)
	{		
		struct exfat_chunk_name name_entry = {EXFAT_TYPE_NAME, 0};
		u8 tmp;
		
		pre_blocknr = blocknr;
		next_entry(dir,&iclusnr, &cluster ,&offset);
		blocknr = exfat_clus_to_blknr(sbi, cluster) + div64_u64(offset ,block_size);
		exfat_debug("blocknr :0x%llx,offset:0x%lx",blocknr,offset);

		if(pre_blocknr != blocknr){
			if(bh_next){
				mark_buffer_dirty(bh_next);			
				err = sync_dirty_buffer(bh_next);		
				brelse(bh_next);
				if(err)
					goto err_out;
			}
			bh_next = sb_bread(dir->i_sb, blocknr);
			if (!bh_next) {
				exfat_error("unable to read inode block "
				   	"for updating (blocknr 0x%llx)\n", blocknr);
			goto err_out;
			}
			ed_reflesh(ed, blocknr);
		}
		
		if(name_len >EXFAT_CHUNK_NAME_SIZE){
			name_len  -= EXFAT_CHUNK_NAME_SIZE;
			tmp = EXFAT_CHUNK_NAME_SIZE;
		}
		else
			tmp = name_len;
		memcpy(name_entry.name, pcb_data->wstr+ i * EXFAT_CHUNK_NAME_SIZE,tmp*2);		
		if(bh_next)
			memcpy(bh_next->b_data+ (offset % block_size),&name_entry,EXFAT_CHUNK_SIZE);
		else
			memcpy(bh->b_data + (offset % block_size) ,&name_entry,EXFAT_CHUNK_SIZE);
		csum = exfat_checksum16(csum, &name_entry, EXFAT_CHUNK_SIZE);	
	}
	
	dirent->checksum = csum;
	memcpy(bh->b_data+ed->bh_offset ,dirent,EXFAT_CHUNK_SIZE);
	
	mark_buffer_dirty(bh);			
	err = sync_dirty_buffer(bh);		
	brelse(bh);
	if(bh_next){
		mark_buffer_dirty(bh_next);			
		err = sync_dirty_buffer(bh_next);		
		brelse(bh_next);
	}	
	if(err)
		goto err_out;
	//exfat_update_mtime(dir);		

	ed->size += (name_entries +2)*EXFAT_CHUNK_SIZE;
	kfree(dirent);	
	kfree(data);
	exfat_debug("Done");

	return 0;
err_out:
	exfat_debug("Failed");
	if(bh)
		brelse(bh);
	if(bh_next)
		brelse(bh_next);
	if(dirent)
		kfree(dirent);
	if(data)
		kfree(data);
	return -1;
	
}
static int exfat_add_entry(struct inode *dir, u16 attrib,struct qstr *qname, 
			  sector_t  dcluster, struct timespec *ts,
			  struct exfat_entry_data *ed,char is_dir)
{
	struct exfat_sb_info *sbi = EXFAT_SB(dir->i_sb);
	struct lookup_parse_data pcb_data;
	int err;	
	
	exfat_debug("Entering");
	
	if(is_dir && !dcluster)
		exfat_error("have not allocate new cluster for the dir inode");
	
	/* FIXME: can't use converted name/hash? */
	err = ChangeName(sbi,qname,&pcb_data);
	if(err)
		goto cleanup;	

	err =write_entries(dir,&pcb_data,attrib,dcluster,ed,is_dir);
	if (err)
		goto cleanup;
			
	/* update timestamp */
	dir->i_ctime = dir->i_mtime = dir->i_atime = *ts;
	if (IS_DIRSYNC(dir))
		(void)exfat_sync_inode(dir);
	else
		mark_inode_dirty(dir);
	kfree(pcb_data.wstr);
	exfat_debug("Done");
	return 0;
	
cleanup:
	kfree(pcb_data.wstr);
	exfat_debug("Failed");
	return err;
}

static int exfat_create(struct inode *dir, struct dentry *dentry, int mode,
		       struct nameidata *nd)
{
	struct super_block *sb = dir->i_sb;
	struct inode *inode;
	struct timespec ts;
	int err = 0;
	u16 attrib = EXFAT_ATTR_ARCH;
	struct exfat_entry_data ed;
	
	exfat_debug("Entering");
	//lock_super(sb);

	ts = CURRENT_TIME_SEC;
	
	ed_init(&ed);
	err = exfat_add_entry(dir, attrib,&dentry->d_name, 0, &ts, &ed,0);
	if (err)
		goto out;
	dir->i_version++;

	inode = exfat_build_inode(sb,&ed,attrib,0,0);	
	ed_brelse(&ed);
	
	if (IS_ERR(inode)) {
		err = PTR_ERR(inode);
		goto out;
	}
	/*have been set in exfat_bulid_inode */ 
		inode->i_version++;
		inode->i_mtime = inode->i_atime = inode->i_ctime = ts;
	/**/
	/* timestamp is already written, so mark_inode_dirty() is unneeded. */

	dentry->d_time = dentry->d_parent->d_inode->i_version;
	d_instantiate(dentry, inode);
	exfat_write_inode(dir,1);
out:
	//unlock_super(sb);
	exfat_debug("Done\n");
	return err;
}

static int exfat_mkdir(struct inode *dir, struct dentry *dentry, int mode)
{
	struct super_block *sb = dir->i_sb;
	struct exfat_sb_info *sbi = EXFAT_SB(sb);
	struct inode *inode;
	struct timespec ts;
	int err;
	sector_t cluster;
	u16 attrib =  EXFAT_ATTR_DIR;
	struct exfat_entry_data ed;
	
	exfat_debug("Entering");
	//lock_super(sb);
	
	ts = CURRENT_TIME_SEC;
	err = exfat_alloc_clusters(dir, &cluster,1);
	if (err) 
		goto out;
	
	ed_init(&ed);
	err = exfat_add_entry(dir, attrib,&dentry->d_name, cluster, &ts, &ed,1);
	if (err)
		goto out_free;
	dir->i_version++;
	inc_nlink(dir);

	inode = exfat_build_inode(sb,&ed,attrib,sbi->clus_size,cluster);	
	ed_brelse(&ed);
	if (IS_ERR(inode)) {
		err = PTR_ERR(inode);
		/* the directory was completed, just return a error */
		goto out;
	}
	/*  have been set in exfat_bulid_inode*/
		inode->i_version++;
		inode->i_nlink = 2;/*sub directory number: "." and ".."*/
		inode->i_mtime = inode->i_atime = inode->i_ctime = ts;
	/**/
	/* timestamp is already written, so mark_inode_dirty() is unneeded. */

	dentry->d_time = dentry->d_parent->d_inode->i_version;
	d_instantiate(dentry, inode);

	//unlock_super(sb);
	exfat_debug("Done");
	return 0;

out_free:
	exfat_free_clusters(dir, 1,cluster);
out:
	//unlock_super(sb);
	return err;
}

static int exfat_erase_entry(struct inode *inode,struct erase_entry *last_ere)
{
	struct exfat_inode_info *exi = EXFAT_I(inode);
	struct buffer_head *bh;	
	off_t offset = exi->de_offset;	
	int name_entries = exfat_div_roundup(exi->de_size-2*EXFAT_CHUNK_SIZE,EXFAT_CHUNK_SIZE);
	uint8_t entry_type;
	int i= 0,err =0;
						
	exfat_debug("Entering,name_entries :%d",name_entries);
	
	/*modify 0x85->0x05*/
	bh = sb_bread(inode->i_sb, exi->de_blocknr[i]);
	if (!bh) {
		exfat_error("unable to read inode block "
		       "for updating (blocknr 0x%llx)\n", exi->de_blocknr[i]);
		return -EIO;
	}
	
	//bh = pd->bhs[i++];
	entry_type = EXFAT_TYPE_DIRENT & ~EXFAT_TYPE_VALID;
	memcpy(bh->b_data+offset ,&entry_type,sizeof(uint8_t));
	exfat_debug("have erased 0x85 in blocknr :0x%llx, offset :0x%lx",bh->b_blocknr,offset);
	
	/*modify 0xc0->0x40*/
	if(offset>=0x1E0){		
		mark_buffer_dirty(bh);			
		err = sync_dirty_buffer(bh);		
		brelse(bh);

		bh = sb_bread(inode->i_sb, exi->de_blocknr[++i]);
		if (!bh) {
		exfat_error("unable to read inode block "
		       "for updating (blocknr 0x%llx)\n", exi->de_blocknr[i]);
		return -EIO;
		}
		offset = 0;	
		//bh = pd->bhs[i++];
	}else
		offset += EXFAT_CHUNK_SIZE;
	entry_type = EXFAT_TYPE_DATA & ~EXFAT_TYPE_VALID;
	memcpy(bh->b_data+offset ,&entry_type,sizeof(uint8_t));	
	exfat_debug("have erased 0xc0 in blocknr :0x%llx, offset :0x%lx",bh->b_blocknr,offset);

	/*modify 0xc1->0x41*/
	while (name_entries--)
	{
		if(offset>=0x1E0){
			mark_buffer_dirty(bh);			
			err = sync_dirty_buffer(bh);		
			brelse(bh);
			
			bh = sb_bread(inode->i_sb, exi->de_blocknr[++i]);
			if (!bh) {
			exfat_error("unable to read inode block "
			       "for updating (blocknr 0x%llx)\n", exi->de_blocknr[i]);
			return -EIO;
			}
			
			offset = 0;	
			//bh = pd->bhs[i++];
		}else
			offset += EXFAT_CHUNK_SIZE;
		entry_type = EXFAT_TYPE_NAME & ~EXFAT_TYPE_VALID;
		memcpy(bh->b_data+offset ,&entry_type,sizeof(uint8_t));	
		exfat_debug("have erased 0xc1 in blocknr :0x%llx, offset :0x%lx",bh->b_blocknr,offset);

	}
	mark_buffer_dirty(bh);			
	err = sync_dirty_buffer(bh);		
	brelse(bh);
	exfat_debug("Done");
	last_ere->blocknr = bh->b_blocknr;
	last_ere->offset = offset+EXFAT_CHUNK_SIZE;
	return 0;
		
}

static int exfat_unlink(struct inode *dir, struct dentry *dentry)
{
	struct inode *inode = dentry->d_inode;
	int err = 0;
	struct erase_entry last_ere;
	//struct exfat_parse_data pd;
	
	//lock_super(sb);
	exfat_debug("Entering");

	//pd_init(&pd);
	err = exfat_find(dir,&dentry->d_name);
	if(err){
		exfat_error("cannot unlink a un-existed file ");		
		return -EIO;
	}
	dir->i_version++;
	
	ere_init(&last_ere);
	err = exfat_erase_entry(inode,&last_ere);	
	if (err)
		goto out;
	shrink_directory(dir, last_ere);
	
	dir->i_mtime = dir->i_atime = CURRENT_TIME_SEC;
	if (IS_DIRSYNC(dir))
		(void)exfat_sync_inode(dir);
	else
		mark_inode_dirty(dir);

	exfat_truncate(inode,0);
	clear_nlink(inode);
	inode->i_mtime = inode->i_atime = CURRENT_TIME_SEC;
	exfat_detach(inode);
	exfat_debug("Done");
	return 0;
out:
	//unlock_super(sb);
	//pd_brelse(&pd);
	exfat_debug("Failed");
	return err;
}

/* See if directory is empty, 0: empty, 1: non_empty */
int exfat_dir_empty(struct inode *dir)
{
	struct buffer_head *bh,map_bh;
	struct super_block *sb = dir->i_sb;
	unsigned char flag;
	off_t offset = 0,off = 0;
	int result = 0;	
	int i = 0;
	sector_t blocknr;
	int err = 0;
	
	exfat_debug("Entering, dir->i_no :0x%lx",dir->i_ino);
	exfat_debug("dir->i_size :0x%llx",dir->i_size);

	offset = off = 0;		
	while (offset < dir->i_size) {		
		map_bh.b_state = 0;
		map_bh.b_blocknr = 0;
		map_bh.b_size = 1 << dir->i_blkbits;
		err = exfat_get_block(dir,i++,&map_bh,0);
		if(err)
			return err;
		blocknr = map_bh.b_blocknr;
		
		exfat_debug("blocknr :0x%llx ,offset:0x%lx",blocknr,offset);
		bh = sb_bread(dir->i_sb,blocknr);
		if (!bh) {
			exfat_error("unable to read inode block "
			       "for updating (blocknr 0x%llx)\n", blocknr);
			return -EIO;
		}
		for( ;off< sb->s_blocksize;off+= EXFAT_CHUNK_SIZE,offset += EXFAT_CHUNK_SIZE)
		{
			flag = bh->b_data[off] ;
			//exfat_debug("flag :0x%x ",flag);
			if((flag & EXFAT_TYPE_VALID) || (flag == __EXFAT_TYPE_LABLE))		
			{
				result = 1;
				exfat_debug("the directory is not empty");
				brelse(bh);
				return result;

			}			
		}
		off = 0;

	}
	exfat_debug("Done,result :%d",result);
	return result;
}
static int exfat_rmdir(struct inode *dir, struct dentry *dentry)
{
	struct inode *inode ;
	int err;
	struct erase_entry last_ere;
	//struct exfat_parse_data pd;

	//lock_super(sb);
	exfat_debug("Entering");
	//pd_init(&pd);
	err = exfat_find(dir,&dentry->d_name);
	if(err){
		exfat_error("cannot unlink a un-existed file ");		
		goto out;
	}
	dir->i_version++;

	inode = dentry->d_inode;
	err = exfat_dir_empty(inode);
	if (err)
		goto out;

	dir->i_version++;
	ere_init(&last_ere);
	err = exfat_erase_entry(inode,&last_ere);	
	if (err)
		goto out;
	shrink_directory(dir, last_ere);
	drop_nlink(dir);
	
	exfat_truncate(inode,0);
	clear_nlink(inode);
	inode->i_mtime = inode->i_atime = CURRENT_TIME_SEC;
	exfat_detach(inode);
	exfat_debug("Done");
	return 0;

out:
	//unlock_super(sb);
	//pd_brelse(&pd);
	exfat_debug("Failed");
	return err;
}

static int  rename_entry(struct inode* old_dir,	struct inode* new_dir, struct inode* inode, 
			struct lookup_parse_data *pcb_data,	struct exfat_entry_data *ed)
{
	struct super_block *sb = old_dir->i_sb;
	struct exfat_sb_info *sbi = EXFAT_SB(sb);
	struct exfat_inode_info *exi = EXFAT_I(inode);
	struct exfat_chunk_dirent *dirent, *new_dirent;
	struct exfat_chunk_data *data;	
	off_t old_offset = exi->de_offset;	
	struct buffer_head *old_bh =NULL ,*new_bh = NULL,*old_bh1=NULL,*new_bh1 = NULL;
	const int name_entries = DIV_ROUND_UP(pcb_data->wlen, EXFAT_CHUNK_NAME_SIZE);	
	int i,err = 0,j;
	sector_t  new_cluster,new_blocknr,pre_blocknr;
	u32 new_iclusnr;
	off_t new_offset;
	u16 csum = 0;
 	unsigned long block_size = old_dir->i_sb->s_blocksize;
	
	/*find the slotes to fill the new exfat records*/
	err = find_slot(new_dir,&new_iclusnr, &new_cluster, &new_offset,name_entries+2);
	exfat_debug("new_iclusnr : 0x%x, new_cluster: 0x%llx ,new_offset :0x%lx ",new_iclusnr,new_cluster,new_offset);
	if(err)
		goto err_out;

	i = 0;
	/*read the old exfat_chunk_dirent:0x85,and make some change*/
	old_bh = sb_bread(sb, exi->de_blocknr[i++]);
	if (!old_bh) {
		exfat_error("unable to read inode block "
		       "for updating (blocknr 0x%llx)\n", exi->de_blocknr[i-1]);
		goto err_out;
	}
	dirent =(struct exfat_chunk_dirent *) old_bh->b_data + old_offset;
	dirent->sub_chunks = 1 + name_entries;
	csum = exfat_checksum16(0, dirent, 2);
	csum = exfat_checksum16(csum, (u8 *)dirent + 4, EXFAT_CHUNK_SIZE - 4);

	/*read the old exfat_chunk_data:0xc0,and make some change*/
	if(old_offset >=0X1E0)
	{
		old_bh1 = sb_bread(sb, exi->de_blocknr[i++]);
		if (!old_bh1) {
			exfat_error("unable to read inode block "
			       "for updating (blocknr 0x%llx)\n",  exi->de_blocknr[i-1]);
			goto err_out;
		}
		data = (struct exfat_chunk_data *)(old_bh1->b_data);

	}else{
		old_offset += EXFAT_CHUNK_SIZE;
		data = (struct exfat_chunk_data *)(old_bh1->b_data+old_offset);
	}
	data->hash  =pcb_data->hash;
	data->name_len = pcb_data->wlen;
	csum = exfat_checksum16(csum, data, EXFAT_CHUNK_SIZE);

	/*read the new exfat_chunk_dirent:0x85, donot write it at present*/
	new_blocknr = exfat_clus_to_blknr(sbi, new_cluster) + div64_u64(new_offset ,block_size);
	new_bh = sb_bread(new_dir->i_sb, new_blocknr);
	if (!new_bh) {
		exfat_error("unable to read inode block "
		       "for updating (blocknr 0x%llx)\n", new_blocknr);
		goto err_out;
	}
	ed->bh_offset  =new_offset %block_size;
	ed_reflesh(ed,new_blocknr);
	new_dirent  =  (struct exfat_chunk_dirent *) new_bh->b_data +ed->bh_offset;
	
	/*read the new exfat_chunk_data:0xc0,write with the old 0xc0*/
	pre_blocknr = new_blocknr;
	next_entry(new_dir,&new_iclusnr, &new_cluster ,&new_offset);
	new_blocknr = exfat_clus_to_blknr(sbi, new_cluster) + div64_u64(new_offset ,block_size);
	exfat_debug("blocknr :0x%llx",new_blocknr);

	if(pre_blocknr != new_blocknr){
		new_bh1 = sb_bread(new_dir->i_sb, new_blocknr);		
		if (!new_bh1) {
			exfat_error("unable to read inode block "
			       "for updating (blocknr 0x%llx)\n", new_blocknr);
			goto err_out;;
		}
		memcpy(new_bh1->b_data+ (new_offset % block_size) ,data,EXFAT_CHUNK_SIZE);
		ed_reflesh(ed,new_blocknr);
	}
	else{
		memcpy(new_bh->b_data+ (new_offset % block_size) ,data,EXFAT_CHUNK_SIZE);	
	}
	
	/*************according to the new name,set the 0xc1 chunkes********************/	
	for (j= 0; j < name_entries; j++)
	{		
		struct exfat_chunk_name name_entry = {EXFAT_TYPE_NAME, 0};
		pre_blocknr = new_blocknr;
		next_entry(new_dir,&new_iclusnr, &new_cluster ,&new_offset);
		new_blocknr = exfat_clus_to_blknr(sbi, new_cluster) + div64_u64(new_offset ,block_size);
		exfat_debug("blocknr :0x%llx",new_blocknr);

		if(pre_blocknr != new_blocknr){
			if(new_bh1){
				mark_buffer_dirty(new_bh1);			
				err = sync_dirty_buffer(new_bh1);		
				brelse(new_bh1);
				if(err)
					goto err_out;
			}
			new_bh1 = sb_bread(new_dir->i_sb, new_blocknr);
			if (!new_bh1) {
				exfat_error("unable to read inode block "
				   	"for updating (blocknr 0x%llx)\n", new_blocknr);
				goto err_out;
			}
			ed_reflesh(ed, new_blocknr);
		}
		memcpy(name_entry.name, pcb_data->wstr+j * EXFAT_CHUNK_NAME_SIZE,
				EXFAT_CHUNK_NAME_SIZE );
		if(new_bh1)
			memcpy(new_bh1->b_data+ (new_offset % block_size),&name_entry,EXFAT_CHUNK_SIZE);
		else
			memcpy(new_bh->b_data + (new_offset % block_size) ,&name_entry,EXFAT_CHUNK_SIZE);
		csum = exfat_checksum16(csum, &name_entry, EXFAT_CHUNK_SIZE);	
	}
	
	dirent->checksum = csum;
	memcpy(new_dirent ,dirent,EXFAT_CHUNK_SIZE);
	if(new_bh1){
		mark_buffer_dirty(new_bh1);			
		err = sync_dirty_buffer(new_bh1);		
		brelse(new_bh1);
	}	
	mark_buffer_dirty(new_bh);			
	err = sync_dirty_buffer(new_bh);		
	brelse(new_bh);
	brelse(old_bh);
	if(old_bh1)
		brelse(old_bh1);
	
	ed->size += (name_entries +2)*EXFAT_CHUNK_SIZE;
	exfat_debug("Done");
	return 0;
	//node->entry_cluster = new_cluster;
	//node->entry_offset = new_offset;

	//tree_detach(node);
	//tree_attach(dir, node);
err_out:
	if(new_bh1)
		brelse(new_bh1);
	if(new_bh)
		brelse(new_bh);
	if(old_bh)
		brelse(old_bh);
	if(old_bh1)
		brelse(old_bh1);
	exfat_debug("Failed");
	return -1;
}


static int exfat_rename(struct inode *old_dir, struct dentry *old_dentry,
		       struct inode *new_dir, struct dentry *new_dentry)
{
	struct inode *old_inode, *new_inode;
	struct timespec ts;
	int err, is_dir;
	struct super_block *sb = old_dir->i_sb;
	struct exfat_sb_info *sbi = EXFAT_SB(sb);
	struct lookup_parse_data pcb_data;
	struct erase_entry  last_ere;
	struct exfat_entry_data ed;
	
	ts = CURRENT_TIME_SEC;

	exfat_debug("Entering");
	
	old_inode = old_dentry->d_inode;
	new_inode = new_dentry->d_inode;
	if(!new_inode)
	{
		exfat_debug("no new_inode");
		return -1;
	}
	
	err = exfat_find(old_dir, &old_dentry->d_name);
	if (err){
		exfat_error("the old name is not existing");
		goto out;
	}
	err = exfat_find(new_dir, &new_dentry->d_name);
	if (!err){
		exfat_error("the new name is existing");
		goto out;
	}
	err = ChangeName(sbi,&new_dentry->d_name,&pcb_data);
	if(err)
		goto out;
	
	ed_init(&ed);
	err  = rename_entry(old_dir, new_dir,old_inode, &pcb_data,&ed);
	if(err)
		goto out;
	err = exfat_erase_entry(old_inode,&last_ere);
	if(err)
		goto out;
	
	is_dir = S_ISDIR(old_inode->i_mode);

	exfat_detach(old_inode);
	//if(!new_inode)
	//	new_inode = exfat_build_inode(sb,&ed, attrib, size, dclusnr);
	
	exfat_attach2(new_inode,&ed);

	new_dir->i_version++;
	old_dir->i_version++;
	old_dir->i_ctime = old_dir->i_mtime = ts;
	
	if(is_dir){
		drop_nlink(old_dir);
		inc_nlink(new_dir);
	}
	
	if (IS_DIRSYNC(old_dir))
		(void)exfat_sync_inode(old_dir);
	else
		mark_inode_dirty(old_dir);
	
 	if (IS_DIRSYNC(new_dir)) {
		err = exfat_sync_inode(old_inode);
		if (err)
			goto out;
	} else
		mark_inode_dirty(old_inode);
	
	exfat_debug("Done");
	return 0;
out:
	err = -1;
	exfat_debug("Failed");
	return err;

}

#endif

const struct inode_operations exfat_dir_inode_ops = {
	.lookup		= exfat_lookup,
#ifdef EXFAT_MODIFY
	.create		= exfat_create,
	.truncate		= exfat_truncate_vfs,
//	.link		= ext3_link,
	.unlink		= exfat_unlink,
//	.symlink	= ext3_symlink,
	.mkdir		= exfat_mkdir,
	.rmdir		= exfat_rmdir,
//	.mknod		= ext3_mknod,
	.rename		= exfat_rename,
//	.setattr	= ext3_setattr,
#endif
	.getattr	= exfat_getattr
#ifdef CONFIG_EXT3_FS_XATTR
//	.setxattr	= generic_setxattr,
//	.getxattr	= generic_getxattr,
//	.listxattr	= ext3_listxattr,
//	.removexattr	= generic_removexattr,
#endif
//	.permission	= ext3_permission,
	/* FIXME: why doesn't ext4 support this for directory? */
//	.fallocate	= ext4_fallocate,
//	.fiemap		= ext4_fiemap,
};

const struct file_operations exfat_dir_ops = {
	.llseek		= generic_file_llseek,
	.read		= generic_read_dir,
	.readdir		= exfat_readdir,
//	.unlocked_ioctl	= exfat_ioctl,
#ifdef CONFIG_COMPAT
//	.compat_ioctl	= exfat_compat_ioctl,
#endif
	.fsync		= file_fsync,
//	.release	= exfat_release_dir,
};
