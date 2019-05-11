/*
 * Pan Ruochen, reachpan@actions-micro.com
 * Copyright (C) 2010 Actions MicroEletronics, Ltd.  All rights reserved.
 *
 * This program is free software; you can distribute it and/or modify it
 * under the terms of the GNU General Public License (Version 2) as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston MA 02111-1307, USA.
 *
 * Kernel command line creation from BREC.
 */
#include <linux/init.h>
#include <linux/string.h>
#include <linux/module.h>

#include <asm/bootinfo.h>
#include <asm/page.h>
#include "sys_cfg.h"

struct am7x_chipinfo am7x_chipinfo;

char am7x_partInfo[128];
        
char * __init prom_getcmdline(void)
{
	return &(arcs_cmdline[0]);
}

void  __init prom_init_cmdline(void)
{
	unsigned char initramfs_source[] = { CONFIG_INITRAMFS_SOURCE };
	char *cmdline, *cp;
	int len;

	cp = &(arcs_cmdline[0]);

	cmdline = (char *) fw_arg0;
	*(int *) &am7x_chipinfo = fw_arg1;
	len = strlen(cmdline);
	am_printf("@cmdline length: %u", strlen(cmdline));
	am_printf("@chipinfo: 0x%08x\n", fw_arg1);
	if( len > 1 ) { /* copy kernel command line */
		strcpy(cp, cmdline);
	}

	if( initramfs_source[0] != 0 ) {
		am_printf("AM: internal initramfs\n");
	} else {
		am_printf("AM: external initramfs\n");
	}
}

EXPORT_SYMBOL_GPL(am7x_chipinfo);


void  __init prom_AM7X_PARTS(void)
{
	char cmdline[CL_SIZE], *ptr;
	int iLoop;
	strcpy(cmdline, arcs_cmdline);
	ptr = strstr(cmdline, "AM7X_PARTS");    
	strcpy(am7x_partInfo,ptr);
	for(iLoop=0x00;iLoop<128;iLoop++){
                // printk("%x",am7x_partInfo[iLoop]);
                if(0x20==am7x_partInfo[iLoop] ){
                        am7x_partInfo[iLoop] = '\0';
                        break;
                }          
              //  printk("%c ",am7x_partInfo[iLoop]);
  	} 
	printk(KERN_DEBUG"x_partInfo prom cmdline: size:%d,%s\n", iLoop,am7x_partInfo); 
}

char *  prom_getAM7X_PARTS(void)
{ 	
	return (char*)&(am7x_partInfo[0]);
}

EXPORT_SYMBOL(prom_getAM7X_PARTS);

