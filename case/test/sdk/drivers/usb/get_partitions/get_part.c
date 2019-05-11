#include <sys/mount.h>
#include <values.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <unistd.h>

#include <memory.h>
#include <fcntl.h>

//#define DEBUG

#ifdef DEBUG
#define dbg(format, arg...)		\
		printf( format , ## arg)	
#else
#define dbg( format, arg...) 		\
		do{}while(0)
#endif

#define DEFAULT_SECTOR_SIZE      				512
/*boot_ind*/
#define ACTIVE_FLAG								0x80

/*sys_ind*/
#define DOS_EXTENDED                0x05
#define WIN98_EXTENDED				0x0f
#define LINUX_PARTITION         	0x81

typedef unsigned int u32;
typedef unsigned int __le32;
typedef unsigned short __le16;
typedef unsigned char __u8;

struct partition {
	unsigned char boot_ind;         /* 0x80 - active */
	unsigned char head;             /* starting head */
	unsigned char sector;           /* starting sector */
	unsigned char cyl;              /* starting cylinder */
	unsigned char sys_ind;          /* what partition type */
	unsigned char end_head;         /* end head */
	unsigned char end_sector;       /* end sector */
	unsigned char end_cyl;          /* end cylinder */
	unsigned char start4[4];        /* starting sector counting from 0 */
	unsigned char size4[4];         /* nr of sectors in partition */
} PACKED;


struct fat_boot_sector {
	__u8	ignored[3];	/* Boot strap short or near jump */
	__u8	system_id[8];	/* Name - can be used to special case
				   partition manager volumes */
	__u8	sector_size[2];	/* bytes per logical sector */
	__u8	sec_per_clus;	/* sectors/cluster */
	__le16	reserved;	/* reserved sectors */
	__u8	fats;		/* number of FATs */
	__u8	dir_entries[2];	/* root directory entries */
	__u8	sectors[2];	/* number of sectors */
	__u8	media;		/* media code */
	__le16	fat_length;	/* sectors/FAT */
	__le16	secs_track;	/* sectors per track */
	__le16	heads;		/* number of heads */
	__le32	hidden;		/* hidden sectors (unused) */
	__le32	total_sect;	/* number of sectors (if sectors == 0) */

	/* The following fields are only used by FAT32 */
	__le32	fat32_length;	/* sectors/FAT */
	__le16	flags;		/* bit 8: fat mirroring, low 4: active fat */
	__u8	version[2];	/* major, minor filesystem version */
	__le32	root_cluster;	/* first cluster in root directory */
	__le16	info_sector;	/* filesystem info sector */
	__le16	backup_boot;	/* backup boot sector */
	__le16	reserved2[6];	/* Unused */
};


static unsigned
read4_little_endian(const unsigned char *cp)
{
	return cp[0] + (cp[1] << 8) + (cp[2] << 16) + (cp[3] << 24);
}

static unsigned
get_start_sect(const struct partition *p)
{
	return read4_little_endian(p->start4);
}

static unsigned
get_nr_sect(const struct partition *p)
{
	return read4_little_endian(p->size4);
}

static inline int fat_valid_media(__u8 media)
{
	return ((0xf8 <= media) || (media == 0xf0));
}

/*
#define get_unaligned(p)					\
({								\
	struct packed_dummy_struct {				\
		typeof(*(p)) __val;				\
	} __attribute__((packed)) *__ptr = (void *) (p);	\
								\
	__ptr->__val;						\
})

#define NR_SECTS(p)	({ u32 __a =	get_unaligned(&p->nr_sects);	})
#define START_SECT(p)	({ u32 __a =	get_unaligned(&p->start_sect);	})
*/

#define pt_offset(b, n) \
	((struct partition *)((b) + 0x1be + (n) * sizeof(struct partition)))


static void 
dump_hex_buf(__u8 *buf,int len)
{
	int idx;
	for(idx = 0; idx < len; idx++) {
		dbg("%02x ",buf[idx]);
		if (((idx+1) & 0xf ) == 0) dbg("\n");
	}
	dbg("\n");
}



static inline int 
is_extended_partition(struct partition *p)
{
		if(	(p->sys_ind == DOS_EXTENDED) ||
		(p->sys_ind == WIN98_EXTENDED) ||
		(p->sys_ind == LINUX_PARTITION))
			return 1;
		return 0;
}


static int
valid_part_table_flag(const unsigned char *phead)
{
	 if((phead[0] == 0x55) && (phead[1] == 0xaa))
		 return 1;

	 return 0;
}


static int read_dev_sector(int fd,unsigned char *buffer,int offset)
{	
	int sect_size = DEFAULT_SECTOR_SIZE;
	unsigned long long secno = (unsigned long long)offset * (unsigned long long)sect_size;
	dbg("seek %d to get extend boot sector\n",secno);	
	if (/*secno > MAXINT(off_t) ||*/ lseek64(fd,secno, SEEK_SET) == -1 ) {
		dbg("unable to seek,try lseek64 \n");
		//if (lseek64(fd, (off64_t)secno, SEEK_SET) == (off64_t) -1){
		//	dbg("unable to do lseek64\n");
		//	return -1;
		//}
		return -1;
	}
	
	if (read(fd, buffer, sect_size) != sect_size){
		dbg("read dev sector failed\n");
		return -1;
	}

	dbg("extended boot sector is:\n");	
	dump_hex_buf((void *)buffer, sect_size);

	return 0;
}



static int
parse_extended(int fd,u32 first_sector, u32 first_size)
{
	struct partition *p;
	//Sector sect;
	unsigned char data[512];
	u32 this_sector, this_size;
	int result = 0;

	//int sector_size = bdev_hardsect_size(bdev) / 512;
	int sector_size = DEFAULT_SECTOR_SIZE;
	int loopct = 0;		/* number of links followed
				   without finding a data partition */
	int i;
	int totals;

	this_sector = first_sector;
	this_size = first_size;

	totals = 0;

	while (1) {
		if (++loopct > 100){
			dbg("so many\n");
			return -1;
		}
		
		/*
		if (state->next == state->limit){
			return -1;
		}
		*/

		memset(data,0,512);

		dbg("this sector:%d\n",this_sector);
		result = read_dev_sector(fd, data,this_sector);
		if (result < 0){
			dbg("read logic boot sector failed\n");
			return -1;
		}

		if (valid_part_table_flag(data + 510) == 0 ){
			dbg("sector content invalid\n");
			return -1; 
		}

		/*
		 * Usually, the first entry is the real data partition,
		 * the 2nd entry is the next extended partition, or empty,
		 * and the 3rd and 4th entries are unused.
		 * However, DRDOS sometimes has the extended partition as
		 * the first entry (when the data partition is empty),
		 * and OS/2 seems to use all four entries.
		 */

		/* 
		 * First process the data partition(s)
		 */

		p = pt_offset(data, 0);
		for (i=0; i<4; i++,p++) {
			u32 offs, size, next;

			if (!get_nr_sect(p) || is_extended_partition(p))
				continue;

			/* Check the 3rd and 4th entries -
			   these sometimes contain random garbage */
			
			//offs = START_SECT(p)*sector_size;
			//size = NR_SECTS(p)*sector_size;
			offs = get_start_sect(p);
			size = get_nr_sect(p);

			next = this_sector + offs;
			if (i >= 2) {
				if (offs + size > this_size)
					continue;
				if (next < first_sector)
					continue;
				if (next + size > first_sector + first_size)
					continue;
			}
			totals++;
		}

		/*
		 * Next, process the (first) extended partition, if present.
		 * (So far, there seems to be no reason to make
		 *  parse_extended()  recursive and allow a tree
		 *  of extended partitions.)
		 * It should be a link to the next logical partition.
		 */
		p -= 4;
		for (i=0; i<4; i++, p++)
			if (get_nr_sect(p) && is_extended_partition(p))
				break;
		if (i == 4)
			break;	 /* nothing left to do */

		this_sector = first_sector + get_start_sect(p);
		this_size = get_nr_sect(p);
		//put_dev_sector(sect);
	}

	//put_dev_sector(sect);
	return totals;
}




static int 
parsed_mbr(int fd,const unsigned char * mbr_ptr,int *total_parts)
{
	const unsigned char *bufp = (const unsigned char *)mbr_ptr;
	struct partition *p;
	int slot;
	int total,extend_logic;
	struct fat_boot_sector *fb;

	p = pt_offset(bufp, 0);
	for (slot = 1; slot <= 4; slot++,p++) {
		//dbg("=====================================\n");
		//dump_hex_buf((void *)p,sizeof(struct partition));
		if ((p->boot_ind != 0) && (p->boot_ind != 0x80)) {
			/*
			 * Even without a valid boot inidicator value
			 * its still possible this is valid FAT filesystem
			 * without a partition table.
			 */
			fb = (struct fat_boot_sector *)mbr_ptr;
			if (slot == 1 && fb->reserved && fb->fats && fat_valid_media(fb->media)) {
				dbg("no mbr but a fat boot sector\n");
				*total_parts = 1;
				return 0;
			} else {
				dbg("invalid information\n");
				return -1;
			}
		}
	}

	total = 0;
	p-=4;
	for (slot = 1 ; slot <= 4 ; slot++,p++) {
		u32 start = get_start_sect(p);
		u32 size = get_nr_sect(p);
		if (!size)
			continue;

		if (is_extended_partition(p)) {
			dbg("=====================================\n");
			dump_hex_buf((void *)p,sizeof(struct partition));
			dbg("has extended partitions,start:%d\n",start);
			extend_logic = parse_extended(fd,start,size);
			if(extend_logic > 0){
				dbg("parse extend part:%d\n",extend_logic);
				total += extend_logic;
			}
			continue;
		}
		total++;
	}

	*total_parts = total;
	return 0;
}

/*api for used to get blockdev partitions--liucan*/
int 
get_bdev_partnums(const char *disk_device)
{
	int fd;
	int result;
	int parts = 0;
	int sect_size = DEFAULT_SECTOR_SIZE;
	unsigned char mbr_buffer[DEFAULT_SECTOR_SIZE];
	
	fd = open(disk_device, O_RDONLY );
	if (fd < 0) {
		dbg("unable to open block dev:%s\n",disk_device);
		return -1;
	}

	memset(mbr_buffer,0,DEFAULT_SECTOR_SIZE);
	result = read(fd,mbr_buffer,DEFAULT_SECTOR_SIZE);
	if((result < 0 ) || (result != DEFAULT_SECTOR_SIZE) ){
		dbg("read mbr from:%s failed\n",disk_device);
		return -1;
	}
	
	if(valid_part_table_flag(mbr_buffer + 510) == 0 ){
		dbg("check mbr failed valid failed\n");
		dump_hex_buf(mbr_buffer,DEFAULT_SECTOR_SIZE);
		return -1;
	}

	result = parsed_mbr(fd,mbr_buffer,&parts);
	if(result != 0){
		dbg("parse mbr failed\n");
		return -1;
	}

	dbg("parts:%d\n",parts);
	return parts;
}


int  main(int argc,char* argv[])
{
	char  disk_name[20];
	int result;

	memset(disk_name,0,sizeof(disk_name));
	strcpy(disk_name,argv[1]);
	
	dbg("disk_name:%s\n",disk_name);

	result = get_bdev_partnums(disk_name);
	if(result < 0)
		dbg("get dev partnums failed\n");

	dbg("get dev partnums:%d\n",result);
	
	return result;
}








