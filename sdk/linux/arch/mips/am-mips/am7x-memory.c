/*
 * Pan Ruochen, reachpan@actions-micro.com
 * Copyright (C) 2010 Actions MicroEletronics, Ltd.  All rights reserved.
 *
 *  This program is free software; you can distribute it and/or modify it
 *  under the terms of the GNU General Public License (Version 2) as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 *  for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place - Suite 330, Boston MA 02111-1307, USA.
 *
 * PROM library functions for acquiring/using memory descriptors given to
 * us from the YAMON.
 */
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/bootmem.h>
#include <linux/pfn.h>
#include <linux/string.h>

#include <asm/bootinfo.h>
#include <asm/page.h>
#include <asm/sections.h>

#include <asm/mips-boards/prom.h>
//@amchip
#include <actions_io.h>
#include <actions_regs.h>

/*#define DEBUG*/

enum yamon_memtypes {
	yamon_dontuse,
	yamon_prom,
	yamon_free,
};
static struct prom_pmemblock mdesc[PROM_MAX_PMEMBLOCKS];

#ifdef DEBUG
static char *mtypes[3] = {
	"Dont use memory",
	"YAMON PROM memory",
	"Free memmory",
};
#endif

static inline unsigned long am7x_get_physical_memory_size(void)
{
	const unsigned int ctl = act_readl(SDR_CTL);
	const unsigned int cap = (ctl >> 4) & 7;
	return (1 << (1 + cap)) * 0x100000;
}

#if 0
/* determined physical memory size, not overridden by command line args  */
unsigned long phys_memsize, user_memsize;

/*#define  DIRECTIVE "memsize="*/
#define DIRECTIVE  "mem="
static struct prom_pmemblock * __init prom_getmdesc(void)
{
	unsigned long memsize;
	char cmdline[CL_SIZE], *ptr;

	phys_memsize = am7x_get_physical_memory_size();
	printk("Physical Memory size: %luM\n",phys_memsize>>20);

	memset(mdesc, 0, sizeof(mdesc));
#if 1
	/* Check the command line for a memsize directive that overrides
	   the physical/default amount */
	strcpy(cmdline, arcs_cmdline);
	ptr = strstr(cmdline, DIRECTIVE);
	if (ptr && (ptr != cmdline) && (*(ptr - 1) != ' '))
		ptr = strstr(ptr, " "DIRECTIVE);
	if (ptr)
		memsize = memparse(ptr + sizeof(DIRECTIVE) -1, &ptr);
	else
		memsize = phys_memsize;
	printk("overidden memsize: %08lx\n", memsize);
	if(memsize > phys_memsize) {
		memsize = phys_memsize;
		printk("memsize -> %08lx\n", memsize);
	}
	mdesc[0].type = yamon_free;
	mdesc[0].base = 0x00000000;
	mdesc[0].size = memsize;
#else
	mdesc[0].type = yamon_free;
	mdesc[0].base = 0x00000000;
	mdesc[0].size = phys_memsize;
	/*am_printf("mdesc[0].size: %dM\n",mdesc[0].size>>20);*/
#endif
	user_memsize = memsize;
	printk("_end: %08lx\n", (unsigned long)_end);
	return &mdesc[0];
}

static void prom_get_reserved_memregion(void **start, unsigned long *size)
{
	*start = (void *) user_memsize;
	*size  = phys_memsize - user_memsize;
}


void mem_pool_init(unsigned long, unsigned long);
#else
static struct prom_pmemblock * __init prom_getmdesc(void)
{
	unsigned long phys_memsize = am7x_get_physical_memory_size();
	printk(KERN_DEBUG"Physical Memory size: %luM\n",phys_memsize>>20);

	memset(mdesc, 0, sizeof(mdesc));
	mdesc[0].type = yamon_free;
	mdesc[0].base = 0x00000000;
	mdesc[0].size = phys_memsize;

	printk(KERN_DEBUG"_end: %08lx\n", (unsigned long)_end);
	return &mdesc[0];
}
#endif

static int __init prom_memtype_classify(unsigned int type)
{
	switch (type) {
	case yamon_free:
		return BOOT_MEM_RAM;
	case yamon_prom:
		return BOOT_MEM_ROM_DATA;
	default:
		return BOOT_MEM_RESERVED;
	}
}

void __init prom_meminit(void)
{
	struct prom_pmemblock *p;
	//unsigned long start, size;

#ifdef DEBUG
	pr_debug("YAMON MEMORY DESCRIPTOR dump:\n");
	p = prom_getmdesc();
	while (p->size) {
		int i = 0;
		pr_debug("[%d,%p]: base<%08lx> size<%08lx> type<%s>\n",
			 i, p, p->base, p->size, mtypes[p->type]);
		p++;
		i++;
	}
#endif
	p = prom_getmdesc();

	while (p->size) {
		long type;
		unsigned long base, size;

		type = prom_memtype_classify(p->type);
		base = p->base;
		size = p->size;

		add_memory_region(base, size, type);
		p++;
	}
	//prom_get_reserved_memregion((void**)&start, &size);
	//mem_pool_init(start, size);
}

void __init prom_free_prom_memory(void)
{
	unsigned long addr;
	int i;

	for (i = 0; i < boot_mem_map.nr_map; i++) {
		if (boot_mem_map.map[i].type != BOOT_MEM_ROM_DATA)
			continue;
		addr = boot_mem_map.map[i].addr;
		am_printf("%s: 0x%08x-0x%08x\n", __func__, addr, addr+boot_mem_map.map[i].size);
	}

	for (i = 0; i < boot_mem_map.nr_map; i++) {
		if (boot_mem_map.map[i].type != BOOT_MEM_ROM_DATA)
			continue;
		addr = boot_mem_map.map[i].addr;
		free_init_pages("prom memory",
				addr, addr + boot_mem_map.map[i].size);
	}
	am_printf("%s okay\n", __func__);
}


