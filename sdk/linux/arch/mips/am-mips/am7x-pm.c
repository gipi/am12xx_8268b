/**
 * arch/mips/am-mips/am7x-pm.c
 *
 *this driver is for system message op
 *
 *author: yekai
 *date:2010-08-27
 *version:0.1
 */
#if CONFIG_PM
#include <linux/module.h>
#include <linux/suspend.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/major.h>
#include <asm/addrspace.h>
#include <asm/delay.h>
#include <asm/r4kcache.h>
#include <asm/cacheflush.h>
#include <asm/mipsregs.h>
#include <asm/regdef.h>
#include <actions_io.h>
#include <am7x_pm.h>
#include <sys_pmu.h>
#include <sys_rtc.h>
#include <linux/delay.h>

#define DBG_PM_DEV		0
#if  DBG_PM_DEV
#define DBG_PM_MSG(format,args...)   printk(format,##args)
#else
#define DBG_PM_MSG(format,args...)   do {} while (0)
#endif

#define STANDBY_BIN_PATH	"/am7x/sdk/standby.bin"
static char* standby_buf = NULL;
static int standby_size = 0;

static struct board_pm_ops board_pm_ctl={
	.board_pm_ioctl =  NULL,
};

static inline int am_inc_ddr_delay(int inc)
{
	int old_dly,new_dly,tmp;

	old_dly = am7x_readl(SDR_CLKDLY);
	new_dly = old_dly&0xf00ffffc;
	/* low byte */
	tmp = ((old_dly&1)<<4)+((old_dly>>20)&0xf)+inc;
	if((tmp>0x1f )||(tmp<0) )
		goto DLY_ERR;
	new_dly |= ((tmp&0x10)>>4)|((tmp&0xf)<<20);
	/* high byte */
	tmp = ((old_dly&0x2)<<3)+((old_dly>>24)&0xf)+inc;
	if((tmp>0x1f )||(tmp<0) )
		goto DLY_ERR;
	new_dly |= (tmp&0x10)>>3|((tmp&0xf)<<24);

	DBG_PM_MSG("new=%x,old=%x\n",new_dly,old_dly);
	return new_dly;

DLY_ERR:
	DBG_PM_MSG("delay set error\n");
	return 0;
}

void am_change_ddr_delay(unsigned int new_vdd,unsigned int old_vdd)
{
	int tmp=0,dly_inc=0;

	DBG_PM_MSG("vdd new=%d,old=%d\n",new_vdd,old_vdd);
	if(new_vdd == old_vdd)
		return;
	
	if(new_vdd>old_vdd){
		tmp = new_vdd - old_vdd;
		dly_inc = 1;
	}else if(new_vdd<old_vdd){
		tmp = old_vdd - new_vdd;
		dly_inc = -1;
	}
		
	switch(tmp){
	case 50:
		dly_inc *= VDD_500_DLY;
		break;
	case 100:
		dly_inc *= VDD_1000_DLY;
		break;
	default:
		return;
	}

	tmp = am_inc_ddr_delay(dly_inc);
	if(tmp){
		am7x_writel(tmp,SDR_CLKDLY);
		udelay(1);
	}
	
}
EXPORT_SYMBOL(am_change_ddr_delay);

static int am_change_ddr_clk(unsigned int clk)
{
	unsigned long irq_flag=0;
	
	if(am7x_readl(CMU_DDRPLL)==clk)
		return -EAGAIN; 
		
	local_irq_save(irq_flag);

	if(clk==DDR_LOW_CLOCK)
		change_ddr_clk(clk, am_inc_ddr_delay(3));
	else
		change_ddr_clk(clk, am_inc_ddr_delay(-3));

	local_irq_restore(irq_flag);
	
	return 0;
}

int am_set_pm_ctl(void* func)
{
	if(!func)
		return -EINVAL;
	board_pm_ctl.board_pm_ioctl = (int (*)(am_pm_type  id, unsigned int cmd, void  *arg))func;
	
	return 0;
}
EXPORT_SYMBOL(am_set_pm_ctl);

int am_pm_ioctl(am_pm_type  id, unsigned int cmd, u8  *arg)
{
	int ret=0;
	
	/*if no board pmu, use dummy functions*/
	if(board_pm_ctl.board_pm_ioctl){
		ret = board_pm_ctl.board_pm_ioctl(id,cmd,arg);
	}
	return  ret;
}
EXPORT_SYMBOL(am_pm_ioctl);

static struct am_pm_info standby_info;
static struct cdev  *pmu_cdev=NULL;
static dev_t  pmu_dev;

static int am_pm_valid(suspend_state_t state)
{
	DBG_PM_MSG("[pm]valid\n");
	return standby_info.func_addr?1:0;
}

int am_pm_begin(suspend_state_t state)
{
	DBG_PM_MSG("[pm]begin\n");
	return 0;
}

int am_pm_prepare(void)
{
	DBG_PM_MSG("[pm]prepare\n");
	return 0;
}

static int am_pm_enter(suspend_state_t state)
{
	int tmp = 0,i = 0;
	unsigned int ddr_test_addr;
	
	DBG_PM_MSG("[pm]enter\n");
	/* flush cache */
	am7x_flush_cache_all();

	/* disable cache and enable sram */
	switch_to_uncache();
	tmp = read_c0_config();
	tmp = (tmp&~CONF_CM_CMASK)|CONF_CM_UNCACHED;
	write_c0_config(tmp);

	/* cclk should divided in sram mode */
	act_orl(0x10,CMU_BUSCLK);
	mdelay(10);
	act_orl(0x4,CMU_BUSCLK);
	ndelay(5);
	
	act_orl(0x3,SRAM_SW_CTL);

	printk("standby_info.func_addr:%x standby_info.func_size:%x \n",standby_info.func_addr,standby_info.func_size);
	/* copy standby.bin into sram */
	standby_info.sram_func = (void (*)(struct am_pm_arg *,unsigned int,unsigned int))SRAM_FUNC_START;
	memcpy(standby_info.sram_func,standby_info.func_addr,standby_info.func_size);
	
	DBG_PM_MSG("[pm]enter\n");
	ddr_test_addr = (unsigned int)kzalloc(1024*4,GFP_ATOMIC);
	if(!ddr_test_addr)
		printk(KERN_ERR"ddr test mem alloc fail\n");
	printk("mode:%x\n",standby_info.arg.wakeup_mode);
	printk("ddr_test_addr:%x\n",ddr_test_addr);
	/* goto sram and run */
	tmp = save_sp();
	standby_info.sram_func(&standby_info.arg,ddr_test_addr,1024);
	restore_sp(tmp);

	if(ddr_test_addr)
		kfree((void*)ddr_test_addr);

	/* enable cache and disable sram */
	act_andl(0,SRAM_SW_CTL);

	act_andl(~0x4,CMU_BUSCLK);
	ndelay(5);

	write_c0_taglo(0);
	write_c0_dtaglo(0);
	for(i=KSEG0; i < KSEG0+ (cpu_dcache_line_size()*1024); i += cpu_dcache_line_size()) {
		__asm__ __volatile__(
			"cache 8, 0(%0)\n\t" 
			"cache 9, 0(%0)\n\t" 
			::"r"(i) );
	}

	tmp = read_c0_config();
	tmp = (tmp&~CONF_CM_CMASK)|CONF_CM_CACHABLE_NONCOHERENT;
	write_c0_config(tmp);	

	return 0;
}

void am_pm_finish(void)
{
	DBG_PM_MSG("[pm]finish\n");
}

void am_pm_end(void)
{
	DBG_PM_MSG("[pm]end\n");
}

void am_pm_recover(void)
{
	DBG_PM_MSG("[pm]recover\n");
}

static int load_standby_bin(void)
{
	struct file *fp;
	mm_segment_t fs; 
		
	fs = get_fs(); 
    set_fs(KERNEL_DS); 
	fp = filp_open(STANDBY_BIN_PATH, O_RDONLY, 0644);
	if (IS_ERR(fp)) { 
       	printk("open %s error\n",STANDBY_BIN_PATH); 
        return -1; 
    }
	vfs_llseek(fp,0,SEEK_END);
	standby_size = fp->f_pos;
	
	standby_buf = (char *)kzalloc(standby_size,GFP_ATOMIC);
	if(!standby_buf){
		printk(KERN_ERR"pmu malloc fail\n");
		return -ENOMEM;
	}
	
	vfs_llseek(fp,0,SEEK_SET);
	vfs_read(fp,(char*)standby_buf,standby_size,&fp->f_pos);
	set_fs(fs);
	filp_close(fp,NULL);	

	standby_info.func_size = standby_size;
	standby_info.func_addr = standby_buf;
	

	return 0;
}

static int am_set_pmu(struct am_pm_info *arg)
{
			
	copy_from_user(&standby_info.arg,&arg->arg,sizeof(struct am_pm_arg));
#if 0
	buf = (char *)kmalloc(arg->func_size,GFP_KERNEL);
	if(!buf){
		printk(KERN_ERR"pmu malloc fail\n");
		return -ENOMEM;
	}
	copy_from_user(buf,arg->func_addr,arg->func_size);
#endif	
	if(standby_buf == NULL)
		load_standby_bin();
	standby_info.func_size = standby_size;
	standby_info.func_addr = standby_buf;

#if 0
	{
		int i;
		DBG_PM_MSG("func size=%d\n",standby_info.func_size);
		DBG_PM_MSG("mode=%x\n",standby_info.arg.wakeup_mode);
		for(i=0;i<4;i++)
			DBG_PM_MSG("%8x\t",standby_info.arg.param[i]);
		DBG_PM_MSG("\n");
	}
#endif
	return 0;
}

/**************************
param definition
-For GPIO_MODE
param[0]: alarm ymd
param[1]: alarm hms
param[2]: output voltage
-For GEN_MODE
param[0]: power off alarm ymd
param[1]: power off alarm hms
param[2]: output voltage
param[3]: power on alarm ymd
param[4]: power on alarm hms
-For SPEC_MODE
param[0]: power off alarm ymd
param[1]: power off alarm hms
param[2]: pulse width
param[3]: power on alarm ymd
param[4]: power on alarm hms
***************************/
static int am_set_alarm(unsigned int cmd,struct am_pm_arg *u_arg)
{
#if CONFIG_AM_CHIP_ID == 1211 || CONFIG_AM_CHIP_ID == 1220 || CONFIG_AM_CHIP_ID == 1213
	unsigned int reg_tmp,tmp;
	struct am_pm_arg arg;

	DBG_PM_MSG("[pm]set alarm\n");
	copy_from_user(&arg,u_arg,sizeof(struct am_pm_arg));
	
	reg_tmp = RTC_READ(RTC_ALARM);
	RTC_WRITE(reg_tmp&~(RTC_ALARM_EN|RTC_ALARM1_EN),RTC_ALARM);
	reg_tmp &= RTC_ALARM_CONF_MASK;
	switch(arg.wakeup_mode){
	case PM_GPIO_MODE:
		tmp = arg.rtc_param[2]&RTC_ALARM_GPO_MASK;
		reg_tmp |= RTC_ALARM_GPIO|RTC_ALARM_GPOE|tmp;
		break;
	case PM_SPEC_MODE:
		if((arg.rtc_param[2]>=RTC_ALARM_PW_MIN) && (arg.rtc_param[2]<=RTC_ALARM_PW_MAX))
			tmp = (arg.rtc_param[2])<<RTC_ALARM_PW_OFFSET;
		else
			return -EINVAL;
		reg_tmp |= RTC_ALARM_SPEC|RTC_ALARM_SMT_EN|tmp;
		break;
	case PM_GEN_MODE:
		if(!(reg_tmp&RTC_ALARM_GPOSEL_MASK)){
			tmp = arg.rtc_param[2]&RTC_ALARM_GPO_MASK;
		}else{
			tmp = (~(arg.rtc_param[2]))&RTC_ALARM_GPO_MASK;
		}
		reg_tmp |= RTC_ALARM_GEN|RTC_ALARM_GPOE|tmp;
		break;
	default:
		return -EINVAL;
	}
	
	if(arg.wakeup_mode != PM_GPIO_MODE){
		RTC_WRITE(arg.rtc_param[0],RTC_YMDALM);
		RTC_WRITE(arg.rtc_param[1],RTC_DHMSALM);
		RTC_WRITE(arg.rtc_param[3],RTC_YMDALM1);
		RTC_WRITE(arg.rtc_param[4],RTC_DHMSALM1);
		RTC_WRITE(reg_tmp|RTC_ALARM_EN|RTC_ALARM1_EN,RTC_ALARM);
	}else{
		RTC_WRITE(reg_tmp,RTC_ALARM);
	}
	//RTC_WRITE(RTC_READ(RTC_CTL)|RTC_ALARM_IRQ_EN,RTC_CTL);
#endif
	return 0;
}


#define DCACHE_SIZE  (cpu_data[0].dcache.waysize * cpu_data[0].dcache.ways)

void cache_exit(void)
{
	register unsigned int val;

	{
		register unsigned int addr;
		for(addr=KSEG0; addr < KSEG0 + DCACHE_SIZE; addr += cpu_dcache_line_size()) {
			__asm__ __volatile__(
				"cache 1, 0(%0)\n\t"
				::"r"(addr) );
		}
	}

	act_writel(3, 0xB0030000 + 0x08);
	val = read_c0_config();
	val &= ~7;
	val |= 2;
	write_c0_config(val);
}

void cache_init(void)
{
	register unsigned int val;

	act_writel(0, 0xB0030000 + 0x08);

	{
		register unsigned int addr;
		__asm__ __volatile__("mtc0 $0, $28, 0\n\t");
		__asm__ __volatile__("mtc0 $0, $28, 2\n\t");
		for(addr=KSEG0; addr < KSEG0 + DCACHE_SIZE; addr += cpu_dcache_line_size()) {
			__asm__ __volatile__(
				"cache 8, 0(%0)\n\t"
				"cache 9, 0(%0)\n\t"
				::"r"(addr) );
		}
	}

	val = read_c0_config();
	val &= ~7;
	val |= 3;
	write_c0_config(val);
}

void mem_dump_long(void *buf, int len)
{
	int i;
	unsigned int *p;
	
	for(i=0, p=(unsigned int *)buf; i<len; ) {
		if( i % 16 == 0 )
			printk("%08x:  ", p );
		printk("%08x ", *p);
		p++;
		i += 4;
		if( i % 16 == 0 )
			printk("\n");
	}
	if( i % 16 != 0 )
		printk("\n");
	printk("\n");
}

static
unsigned int crc32(unsigned char *ptr, unsigned int len)
{   
	unsigned int   i;   
	unsigned int   crc=0;   
	while(len--!=0)   
	{   
		for(i=0x80;   i!=0;   i/=2)   
		{   
			if((crc&0x8000)!=0)   
			{   
				crc*=2;   
				crc^=0x1021;   
			}   /*   余式CRC   乘以2   再求CRC   */   
			else   
			{   
				crc*=2;   
			}   
			if((*ptr&i)!=0)   
			{   
				crc^=0x1021;   /*   再加上本位的CRC   */   
			}   
		}   
		ptr++;
	}   
    
	return(crc);   
}

unsigned long sdr_clkdly[2];

static int am_pmu_ioctl(struct inode *pinode, struct file *pfile, unsigned int cmd, unsigned long arg)
{
	int ret = -EINVAL;
	DBG_PM_MSG("[pm]ioctl\n");

	switch(cmd){
	case AM_PMU_SET:
		ret = am_set_pmu((struct am_pm_info *)arg);
	case AM_PMU_GET:
		copy_to_user((void*)arg,&standby_info,sizeof(struct am_pm_info));
		ret = 0;
		break;
	case AM_PMU_VALID:
		ret = am_pm_valid(0);
		copy_to_user((void *)arg,&ret,sizeof(int));
		ret = 0;
		break;
	case AM_PMU_POFF:
		ret = am_set_alarm(cmd,(struct am_pm_arg *)arg);
		break;
/*----------------------
	case AM_PMU_PLOW:
		am_change_ddr_clk(DDR_LOW_CLOCK);
		break;
	case AM_PMU_PHIGH:
		am_change_ddr_clk(DDR_HIGH_CLOCK);
		break;
--------------------------*/
	
	case AM_PMU_GETCD: /* Get clk dly */
	case AM_PMU_PLOW:
	case AM_PMU_PHIGH:
		{
			struct am_chpll_arg xarg;
			unsigned long irq_flag;
			unsigned int busclk;
			int memsize;
			void *buf;
			void serial_setbrg(unsigned long);
			int status;
			
			copy_from_user(&xarg, arg, sizeof(xarg));
			
			printk(" Entry: 0x%08x\n", xarg.sram_entry);
			printk(" Code start: 0x%08x\n", xarg.code_start);
			printk(" Code size:  0x%08x\n", xarg.code_size);
			printk(" Clock:      0x%08x\n", xarg.clock);

			memsize = (xarg.code_size < 512 ? 512 : xarg.code_size);
			buf = kmalloc(memsize, GFP_KERNEL);
			copy_from_user(buf, xarg.code_start, xarg.code_size);
			mem_dump_long(buf, 64);
			printk("1. crc = 0x%08x\n", crc32(buf, xarg.code_size));
			
			local_irq_save(irq_flag);
			cache_exit();
			memcpy(xarg.sram_entry, buf, xarg.code_size);
			mem_dump_long(xarg.sram_entry, 64);
			printk("2. crc = 0x%08x\n", crc32(xarg.sram_entry, xarg.code_size));
//			void enable_jtag(void);
//			enable_jtag();
//			__asm__ __volatile__("1:	b 1b\n\t");
			busclk = act_readl(CMU_BUSCLK);

			act_writel(busclk | 4, CMU_BUSCLK);

			if( cmd == AM_PMU_GETCD ) {
				if( xarg.sram_entry(DDR_LOW_PLL, buf) != 0 )
					panic("Failed to get clkdly for %d\n",DDR_LOW_PLL);
				sdr_clkdly[0] = act_readl(SDR_CLKDLY);
				if( xarg.sram_entry(DDR_HIGH_PLL, buf) != 0 )
					panic("Failed to get clkdly for %d\n",DDR_HIGH_PLL);
				sdr_clkdly[1] = act_readl(SDR_CLKDLY);

				act_writel(busclk, CMU_BUSCLK);	

				printk("0x%08x for %d\n",DDR_LOW_PLL, sdr_clkdly[0]);
				printk("0x%08x for %d\n",DDR_HIGH_PLL, sdr_clkdly[1]);
				
			} else {
				unsigned long clkdly;
				if( xarg.clock == DDR_LOW_PLL )
					clkdly = sdr_clkdly[0];
				else if ( xarg.clock == DDR_HIGH_PLL )
					clkdly = sdr_clkdly[1];
				else {
					printk("Invaliad clock %u\n", xarg.clock);
					goto inv_clk;
				}
				status = xarg.sram_entry(xarg.clock, (void *)clkdly);

				act_writel(busclk, CMU_BUSCLK);
				serial_setbrg(115200);
				if( status < 0 ) {
					panic("Failed to changed pll\n");
				}
			}
			printk("=========HAHA===============\n");
inv_clk:
			cache_init();
			kfree(buf);
			local_irq_restore(irq_flag);
		}
		break;
	default:
		break;
	}
	
	return ret;
}

static struct platform_suspend_ops am_pm_ops = {
	.valid = am_pm_valid,
	.begin = am_pm_begin,
	.prepare = am_pm_prepare,
	.enter = am_pm_enter,
	.finish = am_pm_finish,
	.end = am_pm_end,
	.recover = am_pm_recover,
};

static struct file_operations pmudev_fops=
{
	.owner  = THIS_MODULE,
	.ioctl = am_pmu_ioctl,
};

#if 0
#include <linux/timer.h>
static struct timer_list pm_timer;
static void am_pm_timer_isr(unsigned long nr)
{
	//printk("pm timer\n");
	udelay(100);
	pm_timer.expires = jiffies + HZ/125;
	add_timer(&pm_timer);
}
#endif

static int __init am_pm_init(void)
{
	int result;
	
	DBG_PM_MSG("[PM]init\n");
	
	pmu_dev =MKDEV(AM_PMU_MAJOR,0);
	result = register_chrdev_region(pmu_dev,PMU_MAX_DEVS,"am_pmu");
	if(result){
		printk(KERN_ERR "alloc_chrdev_region() failed for pmu\n");
		return -EIO;
	}

	pmu_cdev = kzalloc(sizeof(struct cdev),GFP_KERNEL);
	if(!pmu_cdev){
		printk(KERN_ERR "malloc memory  fails for pmu device\n");
		unregister_chrdev_region(pmu_dev,PMU_MAX_DEVS);
		return -ENOMEM;
	}
  	cdev_init(pmu_cdev, &pmudev_fops);
	if(cdev_add(pmu_cdev, pmu_dev, 1))
		goto out_err;

	memset(&standby_info,0,sizeof(struct am_pm_info));

	suspend_set_ops(&am_pm_ops);
#if 0
	init_timer(&pm_timer);	
	pm_timer.data = 0;
	pm_timer.expires =  jiffies + 1;
	pm_timer.function = am_pm_timer_isr;
	add_timer(&pm_timer);
#endif

	return 0;

out_err:
	printk(KERN_ERR "register failed  for pmu device\n");
	kfree(pmu_cdev);
	unregister_chrdev_region(pmu_dev,PMU_MAX_DEVS);
	return -ENODEV;	
}

static void __exit am_pm_exit(void)
{
	if(pmu_cdev)
	{
		cdev_del(pmu_cdev);
		kfree(pmu_cdev);
	}
	unregister_chrdev_region(pmu_dev,PMU_MAX_DEVS);
}

module_init(am_pm_init);
module_exit(am_pm_exit);
#endif
