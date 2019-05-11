#include <asm/mipsregs.h>
#include <asm/param.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/semaphore.h>
#include <linux/fs.h>
#include <linux/fcntl.h>
#include <asm/errno.h>
#include <asm/string.h>
#include <actions_io.h>
#include <sys_cfg.h>
#include <am7x_board.h>

#define GET_MULTI_RES	1
#define PUT_MULTI_RES	2

extern void serial_init (void);
extern int am_printf(const char *fmt, ...);

struct multi_res_sem{
	int is_valid;
	struct semaphore sem;
};

static struct multi_res_sem multi_sem[MAX_MULTI_RES];
struct sys_cfg system_info;

struct sys_cfg* get_sys_info(void)
{
	return &system_info;
}
EXPORT_SYMBOL(get_sys_info);

#if 0
void enable_jtag(void)
{
	static int jtag_enabled = 0;
	if(jtag_enabled)
		return;
	act_orl(2, 0xb0010080);
	act_orl(0xb0, 0xb0010098);
	
	act_orl((1<<4)|(1<<6)|(1<<7), 0xb00f0100);
	act_orl(1, 0xb00f00f4);
	act_orl(3<<23, 0xb00f00fc);

	act_andl(~(3<<26), 0xb01c0030);
	act_andl(~(15<<20), 0xb01c0034);
	
// MFP0(0x40): [2:0]=0 [6:4]=0
// MFP1(0x44): [10:8]=0 [14:12]=0 [18:16]=0
// MFP4(0x50): [14:12]=0
 	act_andl(~(7|(7<<4)), 0xb01c0040);
	act_andl(~(0x7ff<<8), 0xb01c0044);
	act_andl(~(7<<12), 0xb01c0050);
//	__asm__ __volatile__("1: b 1b\n\t");
	jtag_enabled = 1;
}
#endif

void enable_jtag(void)
{
	act_andl(~0x0007ff00, GPIO_MFCTL2);
    act_andl(~0x00003007, GPIO_MFCTL3);
#if CONFIG_AM_CHIP_ID != 1213
	act_andl(~0x70000000, GPIO_MFCTL5);
#endif
//	__asm__ __volatile__("1: b 1b\n\t");
}
EXPORT_SYMBOL(enable_jtag);

int am_init_multi_res(AM_MULTI_TYPE id,unsigned int val)
{
	if(id>MAX_MULTI_ID)
		return -EINVAL;
	if(multi_sem[id].is_valid)
		return -EBUSY;
	
	sema_init(&(multi_sem[id].sem),val);
	multi_sem[id].is_valid = 1;

	return 0;
}
EXPORT_SYMBOL(am_init_multi_res);

static int am_op_multi_res(AM_MULTI_TYPE id, int op)
{
	if(id>MAX_MULTI_ID)
		return -EINVAL;
	if(!(multi_sem[id].is_valid))
		return -ENXIO;

	if(op==GET_MULTI_RES)
		down(&(multi_sem[id].sem));
	else
		up(&(multi_sem[id].sem));

	return 0;
}

int am_get_multi_res(AM_MULTI_TYPE id)
{
	return am_op_multi_res(id,GET_MULTI_RES);
}
EXPORT_SYMBOL(am_get_multi_res);

int am_release_multi_res(AM_MULTI_TYPE id)
{
	return am_op_multi_res(id,PUT_MULTI_RES);
}
EXPORT_SYMBOL(am_release_multi_res);

int am_get_config(char *path, char *buf, loff_t offset, size_t count)
{
	int ret=0;
	struct file *fp;
	struct kstat fstat;
	mm_segment_t fs; 

	fs = get_fs(); 
    	set_fs(KERNEL_DS);
	ret = vfs_stat(path, &fstat);
	if(ret){
		printk("stat config %s fail %d\n",path,ret);
		goto END;
	}
	offset += EZ_OFFSET;
#if 0
	if(fstat.size<(offset+count)){
		ret = -EINVAL;
		printk("read overflow\n");
		goto END;
	}
#endif
	fp = filp_open(path, O_RDONLY, 0644);
	if (IS_ERR(fp)) { 
       		printk("open %s error\n",path); 
		ret = -ENOENT;
        		goto END;
	}

	vfs_llseek(fp,offset,SEEK_SET);
	ret = vfs_read(fp,buf,count,&fp->f_pos);
	filp_close(fp,NULL);
	set_fs(fs);
	
END:
	return ret;
}
EXPORT_SYMBOL(am_get_config);

asmlinkage void __init board_init(void)
{
//	enable_jtag();
	serial_init();
	am_printf("board init\n");

	write_c0_status(0x00400000);
	write_c0_cause(0x00800000);
	
	memset(multi_sem,0,sizeof(struct multi_res_sem)*MAX_MULTI_RES);	//set multi_sem null
	am_init_multi_res(DEVICE_RES,1);
#ifdef CONFIG_AM_8251
	am_init_multi_res(MAC_I2C,1);
#endif
}

