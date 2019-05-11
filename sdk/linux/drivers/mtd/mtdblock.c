/*
 * Direct MTD block device access
 *
 * (C) 2000-2003 Nicolas Pitre <nico@cam.org>
 * (C) 1999-2003 David Woodhouse <dwmw2@infradead.org>
 */

#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/vmalloc.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/blktrans.h>
#include <linux/mutex.h>
#include "sys_buf_def.h"
#include <linux/kthread.h>


#if (defined CONFIG_MTD_M25P80 == 1)
void  nand_adfu_open();
INT32S  nand_adfu_read(INT32U Lba, INT8U *Buf, INT32U Length);
void nand_adfu_close();
INT32S  nand_adfu_write(INT32U Lba, INT8U *Buf, INT32U Length);
#endif

static struct mtdblk_dev {
	struct mtd_info *mtd;
	int count;
	struct mutex cache_mutex;
	unsigned char *cache_data;
	unsigned long cache_offset;
	unsigned int cache_size;
	enum { STATE_EMPTY, STATE_CLEAN, STATE_DIRTY } cache_state;
        void * priv; 
} *mtdblks[MAX_MTD_DEVICES];



static struct mtd_blktrans_dev * g_dev[MAX_MTD_DEVICES];
struct mutex g_lock; // when ota ,lock mtdblocka


/*
 * Cache stuff...
 *
 * Since typical flash erasable sectors are much larger than what Linux's
 * buffer cache can handle, we must implement read-modify-write on flash
 * sectors for each block write requests.  To avoid over-erasing flash sectors
 * and to speed things up, we locally cache a whole flash sector while it is
 * being written to until a different sector is required.
 */

#define ACTIONS_SYCN_MODE
#if (defined ACTIONS_SYCN_MODE == 1) 

#define SYNC_WAIT_TIME   (HZ) 
typedef struct actions_sync{
    struct timer_list timer;
    struct task_struct *thread;
}actions_sync;

static int write_cached_data (struct mtdblk_dev *mtdblk);
static void snor_poll_event(unsigned long arg) 
{
    struct mtdblk_dev *mtdblk = (struct mtdblk_dev *)arg;
    actions_sync * sync = mtdblk->priv;

    wake_up_process(sync->thread);
    mod_timer(&sync->timer,jiffies+SYNC_WAIT_TIME);  
}

void snor_sync_timer(struct mtdblk_dev * mtdblk){
    actions_sync * sync = mtdblk->priv;
    if(mtdblk->mtd->index == 0){
        mod_timer(&sync->timer,jiffies+SYNC_WAIT_TIME);  
    }
}
int snor_sync_thread(void *data)
{
    struct mtdblk_dev *mtdblk = (struct mtdblk_dev *)data;
    while(!kthread_should_stop())
    {          
        mutex_lock(&mtdblk->cache_mutex);
	write_cached_data(mtdblk);
	mutex_unlock(&mtdblk->cache_mutex);
        if (mtdblk->mtd->sync)
		mtdblk->mtd->sync(mtdblk->mtd);
        set_current_state(TASK_INTERRUPTIBLE);
        schedule();
    }
    return 0x00;
}

static void snor_sync_init(struct mtdblk_dev * mtdblk){
    actions_sync * sync;
    char threadName[128];
    struct mtd_info *mtd = mtdblk->mtd;
    
    if(mtdblk->mtd->index == 0){
        sprintf(threadName,"%s%d\n",mtd->name,mtd->index);
        sync= kzalloc(sizeof(actions_sync), GFP_KERNEL);
        init_timer(&sync->timer);
        sync->timer.expires = jiffies + SYNC_WAIT_TIME;   
        sync->timer.function = snor_poll_event;
        sync->timer.data = mtdblk;
        sync->thread = kthread_create(snor_sync_thread,mtdblk,"%s",threadName);
        add_timer(&sync->timer);
        mtdblk->priv = sync;
    }
    
}

static void snor_sync_destroy(struct mtdblk_dev * mtdblk){
    if(mtdblk->mtd->index == 0){
        if(mtdblk->priv){
            actions_sync * sync = (actions_sync *)mtdblk->priv;
            del_timer(&sync->timer);
            kthread_stop(sync->thread);
            kfree(mtdblk->priv);
        }
        mtdblk->priv = NULL;
    }
}
#endif

static void erase_callback(struct erase_info *done)
{
	wait_queue_head_t *wait_q = (wait_queue_head_t *)done->priv;
	wake_up(wait_q);
}

static int erase_write (struct mtd_info *mtd, unsigned long pos,
			int len, const char *buf)
{
	struct erase_info erase;
	DECLARE_WAITQUEUE(wait, current);
	wait_queue_head_t wait_q;
	size_t retlen;
	int ret;

	/*
	 * First, let's erase the flash block.
	 */

	init_waitqueue_head(&wait_q);
	erase.mtd = mtd;
	erase.callback = erase_callback;
	erase.addr = pos;
	erase.len = len;
	erase.priv = (u_long)&wait_q;

	set_current_state(TASK_INTERRUPTIBLE);
	add_wait_queue(&wait_q, &wait);

	ret = mtd->erase(mtd, &erase);
	if (ret) {
		set_current_state(TASK_RUNNING);
		remove_wait_queue(&wait_q, &wait);
		printk (KERN_WARNING "mtdblock: erase of region [0x%lx, 0x%x] "
				     "on \"%s\" failed\n",
			pos, len, mtd->name);
		return ret;
	}

	schedule();  /* Wait for erase to finish. */
	remove_wait_queue(&wait_q, &wait);

	/*
	 * Next, writhe data to flash.
	 */

	ret = mtd->write(mtd, pos, len, &retlen, buf);
	if (ret)
		return ret;
	if (retlen != len)
		return -EIO;
	return 0;
}


static int write_cached_data (struct mtdblk_dev *mtdblk)
{
	struct mtd_info *mtd = mtdblk->mtd;
	int ret;

	if (mtdblk->cache_state != STATE_DIRTY)
		return 0;

	DEBUG(MTD_DEBUG_LEVEL2, "mtdblock: writing cached data for \"%s\" "
			"at 0x%lx, size 0x%x\n", mtd->name,
			mtdblk->cache_offset, mtdblk->cache_size);

	ret = erase_write (mtd, mtdblk->cache_offset,
			   mtdblk->cache_size, mtdblk->cache_data);
	if (ret)
		return ret;

	/*
	 * Here we could argubly set the cache state to STATE_CLEAN.
	 * However this could lead to inconsistency since we will not
	 * be notified if this content is altered on the flash by other
	 * means.  Let's declare it empty and leave buffering tasks to
	 * the buffer cache instead.
	 */
	mtdblk->cache_state = STATE_EMPTY;
	return 0;
}


static int do_cached_write (struct mtdblk_dev *mtdblk, unsigned long pos,
			    int len, const char *buf)
{
	struct mtd_info *mtd = mtdblk->mtd;
	unsigned int sect_size = mtdblk->cache_size;
	size_t retlen;
	int ret;

	DEBUG(MTD_DEBUG_LEVEL2, "mtdblock: write on \"%s\" at 0x%lx, size 0x%x\n",
		mtd->name, pos, len);

	if (!sect_size)
		return mtd->write(mtd, pos, len, &retlen, buf);

	while (len > 0) {
		unsigned long sect_start = (pos/sect_size)*sect_size;
		unsigned int offset = pos - sect_start;
		unsigned int size = sect_size - offset;
		if( size > len )
			size = len;

		if (size == sect_size) {
			/*
			 * We are covering a whole sector.  Thus there is no
			 * need to bother with the cache while it may still be
			 * useful for other partial writes.
			 */
			ret = erase_write (mtd, pos, size, buf);
			if (ret)
				return ret;
		} else {
			/* Partial sector: need to use the cache */

			if (mtdblk->cache_state == STATE_DIRTY &&
			    mtdblk->cache_offset != sect_start) {
				ret = write_cached_data(mtdblk);
				if (ret)
					return ret;
			}

			if (mtdblk->cache_state == STATE_EMPTY ||
			    mtdblk->cache_offset != sect_start) {
				/* fill the cache with the current sector */
				mtdblk->cache_state = STATE_EMPTY;
				ret = mtd->read(mtd, sect_start, sect_size,
						&retlen, mtdblk->cache_data);
				if (ret)
					return ret;
				if (retlen != sect_size)
					return -EIO;

				mtdblk->cache_offset = sect_start;
				mtdblk->cache_size = sect_size;
				mtdblk->cache_state = STATE_CLEAN;
			}

			/* write data to our local cache */
			memcpy (mtdblk->cache_data + offset, buf, size);
			mtdblk->cache_state = STATE_DIRTY;
		}

		buf += size;
		pos += size;
		len -= size;
	}

	return 0;
}


static int do_cached_read (struct mtdblk_dev *mtdblk, unsigned long pos,
			   int len, char *buf)
{
	struct mtd_info *mtd = mtdblk->mtd;
	unsigned int sect_size = mtdblk->cache_size;
	size_t retlen;
	int ret;

	DEBUG(MTD_DEBUG_LEVEL2, "mtdblock: read on \"%s\" at 0x%lx, size 0x%x\n",
			mtd->name, pos, len);

	if (!sect_size)
		return mtd->read(mtd, pos, len, &retlen, buf);

	while (len > 0) {
		unsigned long sect_start = (pos/sect_size)*sect_size;
		unsigned int offset = pos - sect_start;
		unsigned int size = sect_size - offset;
		if (size > len)
			size = len;

		/*
		 * Check if the requested data is already cached
		 * Read the requested amount of data from our internal cache if it
		 * contains what we want, otherwise we read the data directly
		 * from flash.
		 */
		mutex_lock(&mtdblk->cache_mutex);
		if (mtdblk->cache_state != STATE_EMPTY &&
		    mtdblk->cache_offset == sect_start) {
			memcpy (buf, mtdblk->cache_data + offset, size);
                       mutex_unlock(&mtdblk->cache_mutex);
		} else {
		        mutex_unlock(&mtdblk->cache_mutex);
			ret = mtd->read(mtd, pos, size, &retlen, buf);
			if (ret)
				return ret;
			if (retlen != size)
				return -EIO;
		}

		buf += size;
		pos += size;
		len -= size;
	}

	return 0;
}

static int mtdblock_readsect(struct mtd_blktrans_dev *dev,
			      unsigned long block, char *buf)
{
	struct mtdblk_dev *mtdblk = mtdblks[dev->devnum];
	if(dev->devnum == 0){
		mutex_lock(&g_lock);
		mutex_unlock(&g_lock);
	}
#if (defined ACTIONS_SYCN_MODE == 1) 
        snor_sync_timer(mtdblk);
#endif
	return do_cached_read(mtdblk, (block+dev->mbr_pos)<<9, 512, buf);// added by jjf for skip boot part
}

static int mtdblock_writesect(struct mtd_blktrans_dev *dev,
			      unsigned long block, char *buf)
{
	struct mtdblk_dev *mtdblk = mtdblks[dev->devnum];
	if(dev->devnum == 0){
		mutex_lock(&g_lock);
		mutex_unlock(&g_lock);
		printk("sys w b:0x%x\n",block+dev->mbr_pos);
	}

	
	if (unlikely(!mtdblk->cache_data && mtdblk->cache_size)) {
		mtdblk->cache_data = vmalloc(mtdblk->mtd->erasesize);
		if (!mtdblk->cache_data)
			return -EINTR;
		/* -EINTR is not really correct, but it is the best match
		 * documented in man 2 write for all cases.  We could also
		 * return -EAGAIN sometimes, but why bother?
		 */
	}
#if (defined ACTIONS_SYCN_MODE == 1)
        snor_sync_timer(mtdblk);
#endif
    
	return do_cached_write(mtdblk, (block+dev->mbr_pos)<<9, 512, buf);// added by jjf for skip boot part
}

static int mtdblock_open(struct mtd_blktrans_dev *mbd)
{
	struct mtdblk_dev *mtdblk;
	struct mtd_info *mtd = mbd->mtd;
	int dev = mbd->devnum;

	DEBUG(MTD_DEBUG_LEVEL1,"mtdblock_open\n");

	if (mtdblks[dev]) {
		mtdblks[dev]->count++;
		return 0;
	}

	/* OK, it's not open. Create cache info for it */
	mtdblk = kzalloc(sizeof(struct mtdblk_dev), GFP_KERNEL);
	if (!mtdblk)
		return -ENOMEM;

        mtdblk->mtd = mtd;

	mutex_init(&mtdblk->cache_mutex);
	mtdblk->cache_state = STATE_EMPTY;
	if ( !(mtdblk->mtd->flags & MTD_NO_ERASE) && mtdblk->mtd->erasesize) {
		mtdblk->cache_size = mtdblk->mtd->erasesize;
		mtdblk->cache_data = NULL;
	}

	mtdblks[dev] = mtdblk;
        mtdblk->count = 1;
#if (defined ACTIONS_SYCN_MODE == 1) 
        printk("%s %d %d\n",__func__,__LINE__,mtdblk->mtd->index);
        snor_sync_init(mtdblk);
#endif

	DEBUG(MTD_DEBUG_LEVEL1, "ok\n");

	return 0;
}

static int mtdblock_release(struct mtd_blktrans_dev *mbd)
{
	int dev = mbd->devnum;
	struct mtdblk_dev *mtdblk = mtdblks[dev];

   	DEBUG(MTD_DEBUG_LEVEL1, "mtdblock_release\n");

	mutex_lock(&mtdblk->cache_mutex);
	write_cached_data(mtdblk);
	mutex_unlock(&mtdblk->cache_mutex);

	if (!--mtdblk->count) {
		/* It was the last usage. Free the device */
		mtdblks[dev] = NULL;
		if (mtdblk->mtd->sync)
			mtdblk->mtd->sync(mtdblk->mtd);
#if (defined ACTIONS_SYCN_MODE == 1) 
                snor_sync_destroy(mtdblk);
#endif
		vfree(mtdblk->cache_data);
		kfree(mtdblk);
	}
	DEBUG(MTD_DEBUG_LEVEL1, "ok\n");

	return 0;
}

static int mtdblock_flush(struct mtd_blktrans_dev *dev)
{
	struct mtdblk_dev *mtdblk = mtdblks[dev->devnum];

	mutex_lock(&mtdblk->cache_mutex);
	write_cached_data(mtdblk);
	mutex_unlock(&mtdblk->cache_mutex);

	if (mtdblk->mtd->sync)
		mtdblk->mtd->sync(mtdblk->mtd);
	return 0;
}
#if 1
int find_mbr_position(struct mtd_blktrans_dev *dev)
{
#define LFIHEADSTART (SNOR_BREC_OFFSET+SNOR_BREC_SIZE)
	int mbrPos;


	int block = LFIHEADSTART/512; //LFI的位置现在68K的位置 转换为sector位置

	struct mtd_blktrans_ops *tr = dev->tr;
	
	char * buffer = kmalloc(1024, GFP_KERNEL);
	if(buffer == NULL)
		printk("find_mbr_position kmalloc error!\n");

	
	//1. open dev
	tr->open(dev);

	//2.read dev
	tr->readsect(dev,block,buffer);

	/*寻找mbr的位置*/
	mbrPos = *(u32 *)(buffer+0x90);

	if(dev->devnum == 0){
		dev->mbr_pos = (mbrPos)+LFIHEADSTART/512;
	}
	else
	{
		dev->mbr_pos = 0;
	}

	printk("dev->devnum is :%d mbr_pos is:0x%x\n",dev->devnum,dev->mbr_pos);


	kfree(buffer);
	buffer = NULL;


	//1. close dev 
	tr->release(dev);
	return 0;
}
#endif

static void mtdblock_add_mtd(struct mtd_blktrans_ops *tr, struct mtd_info *mtd)
{
	struct mtd_blktrans_dev *dev = kzalloc(sizeof(*dev), GFP_KERNEL);

	if (!dev)
		return;

	dev->mtd = mtd;
	dev->devnum = mtd->index;

	dev->size = mtd->size >> 9;
	dev->tr = tr;

	if (!(mtd->flags & MTD_WRITEABLE))
		dev->readonly = 1;

	if(mtd->index == 0)
		mutex_init(&g_lock);


	find_mbr_position(dev);
	add_mtd_blktrans_dev(dev);


	
	g_dev[mtd->index] = dev;// record mtdblockb
	

#if 0
	//for test read write 
	if(mtd->index)
	{
		
		int i;
		char * wbuffer = kzalloc(4096, GFP_KERNEL);
		char * rbuffer = kzalloc(4096, GFP_KERNEL);
		nand_adfu_open();
		if(wbuffer == NULL)
			printk("nand_adfu_write kmalloc error!\n");
		if(rbuffer == NULL)
			printk("nand_adfu_read kmalloc error!\n");

		for(i = 0;i<4096;i++){
			wbuffer[i]=i;
		}
		
		nand_adfu_write(0, wbuffer, 8);
		nand_adfu_read(0, rbuffer, 8);


		for(i = 0;i<4096;i++){
			if(i%16==0)
				printk("\n");
			printk("%d",rbuffer[i]);
		}
		
		kfree(wbuffer);
		kfree(rbuffer);
		nand_adfu_close();
	}
#endif
}

static void mtdblock_remove_dev(struct mtd_blktrans_dev *dev)
{
	del_mtd_blktrans_dev(dev);
	kfree(dev);
}

static struct mtd_blktrans_ops mtdblock_tr = {
	.name		= "mtdblock",
	.major		= 31,
	.part_bits	= 4,
	.blksize 	= 512,
	.open		= mtdblock_open,
	.flush		= mtdblock_flush,
	.release	= mtdblock_release,
	.readsect	= mtdblock_readsect,
	.writesect	= mtdblock_writesect,
	.add_mtd	= mtdblock_add_mtd,
	.remove_dev	= mtdblock_remove_dev,
	.owner		= THIS_MODULE,
};

static int __init init_mtdblock(void)
{
	return register_mtd_blktrans(&mtdblock_tr);
}

static void __exit cleanup_mtdblock(void)
{
	deregister_mtd_blktrans(&mtdblock_tr);
}


#if (defined CONFIG_MTD_M25P80)
void  nand_adfu_open()
{
	struct mtd_blktrans_ops *tr = g_dev[1]->tr;

	if(mtdblks[g_dev[0]->devnum])
		mtdblock_flush(g_dev[0]);// flush mtdblocka
	mutex_lock(&g_lock);            //lock mtdblocka


	tr->open(g_dev[1]);
	
}
EXPORT_SYMBOL(nand_adfu_open);

INT32S  nand_adfu_read(INT32U Lba, INT8U *Buf, INT32U Length)
{
	int nsect = Length;
	int block = Lba;
	INT8U *readBuffer = Buf;

	
	struct mtd_blktrans_ops *tr = g_dev[1]->tr;
	while(nsect){
		tr->readsect(g_dev[1], block, readBuffer);
		nsect--;
		block++;
		readBuffer+=512;
	}
	return 0;
}
EXPORT_SYMBOL(nand_adfu_read);

INT32S  nand_adfu_write(INT32U Lba, INT8U *Buf, INT32U Length)
{
	int nsect = Length;
	int block = Lba;
	INT8U *writeBuffer = Buf;

	struct mtd_blktrans_ops *tr = g_dev[1]->tr;
	while(nsect){
		tr->writesect(g_dev[1], block, writeBuffer);
		nsect--;
		block++;
		writeBuffer+=512;
	}

	return 0;
}
EXPORT_SYMBOL(nand_adfu_write);

void nand_adfu_close()
{
	struct mtd_blktrans_ops *tr = g_dev[1]->tr;
	tr->release(g_dev[1]);

	mutex_unlock(&g_lock);
}
EXPORT_SYMBOL(nand_adfu_close);

void nand_adfu_end()
{
	struct mtd_blktrans_ops *tr = g_dev[1]->tr;
	tr->release(g_dev[1]);
}
EXPORT_SYMBOL(nand_adfu_end);



INT32U get_nand_flash_type(void)
{
       INT32U nand_type;
	nand_type = '0' * 0x1000000 + '8' * 0x10000 + '0' * 0x100 + 'N';
	return nand_type;
}
EXPORT_SYMBOL(get_nand_flash_type);
#endif


module_init(init_mtdblock);
module_exit(cleanup_mtdblock);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nicolas Pitre <nico@cam.org> et al.");
MODULE_DESCRIPTION("Caching read/erase/writeback block device emulation access to MTD devices");
