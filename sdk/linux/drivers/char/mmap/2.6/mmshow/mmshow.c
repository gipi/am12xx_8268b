#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>

unsigned long pagemem;
unsigned char *kmallocmem;
unsigned char *vmallocmem;

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Chen Li-Jun, WANG Cong");

static int __init init_mmshow(void)
{
	pagemem = __get_free_page(GFP_KERNEL);
	if(!pagemem)
		goto fail3;
	printk(KERN_INFO "pagemem=0x%lx\n", pagemem);
	kmallocmem = kmalloc(100, GFP_KERNEL);
	if(!kmallocmem)
		goto fail2;
	printk(KERN_INFO "kmallocmem=0x%p\n", kmallocmem);
	vmallocmem = vmalloc(1000000);
	if(!vmallocmem)
		goto fail1;
	printk(KERN_INFO "vmallocmem=0x%p\n", vmallocmem);
	return 0;
fail1:
	kfree(kmallocmem);
fail2:
	free_page(pagemem);
fail3:
	return -1;
}

static void __exit cleanup_mmshow(void)
{
	vfree(vmallocmem);
	kfree(kmallocmem);
	free_page(pagemem);
}

module_init(init_mmshow);
module_exit(cleanup_mmshow);

