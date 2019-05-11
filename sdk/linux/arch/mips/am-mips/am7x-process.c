/**
 * arch/mips/am-mips/am7x-process.c
 *
 *this driver is for system process monitor
 *
 *author: yekai
 *date:2010-08-31
 *version:0.1
 */
 #define USE_KERNEL_MONITOR	0

 #if USE_KERNEL_MONITOR
#include <linux/module.h>
#include <linux/sched.h>

#define PRO_MAX		30

struct am_monitor{
	pid_t pid;
	cputime_t utime;
	cputime_t stime;
};

struct am_monitor am_pro[PRO_MAX];

void am_top_print(struct am_monitor *base, struct task_struct *p)
{
	int tick_unit = 1000/HZ;
	cputime_t utime_con, stime_con;

	utime_con = p->utime - base->utime;
	stime_con = p->stime - base->stime;
	if(utime_con || stime_con)
		printk("%d\t%ld\t%ld\t\n",base->pid,utime_con*tick_unit,stime_con*tick_unit);
}

void am_top_start(void)
{
	int i=0;
	struct task_struct *p;
		
	for_each_process(p){
		am_pro[i].pid = p->pid;
		am_pro[i].utime = p->utime;
		am_pro[i].stime = p->stime;
		if(++i==PRO_MAX){
			printk("over max process number\n");
			break;
		}
	}
}

void am_top_end(void)
{
	int i=0;
	struct task_struct *p;

	for_each_process(p){
		for(i=0;i<PRO_MAX;i++){
			if(am_pro[i].pid == p->pid){
				am_top_print(&am_pro[i],p);
				break;
			}
		}

		 /* new process */
		if(i==PRO_MAX){
			printk("new process %d\n",p->pid);
		}
	}
}

EXPORT_SYMBOL(am_top_start);
EXPORT_SYMBOL(am_top_end);
#endif

