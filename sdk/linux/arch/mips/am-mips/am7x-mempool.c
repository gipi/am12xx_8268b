/*
*	port from ucos alloc subsystem  
*/

#include "linux/kernel.h"
#include "linux/mm.h"
#include "am_types.h"
#include "linux/module.h"
/*
*	Description: Mempool from system ,now only one region	
*		for framebuffer ,video and jpeg decoder buffer
*		buffer start,size are pass by boot command arg	
*/
static unsigned long  reserved_mem_start = 0,reserved_mem_size = 0;	 	


/*
*
*	|***node struct****|********|node struct***|********|
*/


struct alloc_struct {
    INT32U address;
    INT32U size;
    struct alloc_struct *next;
    INT32U reserve;
};


static struct alloc_struct head,tail;


/*
*
*/
static void init_linklist(INT32U startaddr,INT32U length)
{
    head.size = tail.size = 0;    
    head.address = startaddr;
    tail.address = startaddr + length;
    head.next = &tail; 
    tail.next = NULL;    
}
/**
*	Description: Get the max avail memory size in the heap
**/
unsigned long mempool_query_max(void)
{
	int size =0;
	struct alloc_struct *ptr = &head;
	
	while(ptr&&ptr->next){
		if(size < (ptr->next->address - ptr->address - ptr->size))
			size = (ptr->next->address - ptr->address - ptr->size);  
		ptr = ptr->next;
	}

	return size ;
}

EXPORT_SYMBOL(mempool_query_max);

/**
*	Description : get the size of of the address start 
**/
unsigned long mempool_get_size(unsigned int start)
{
	unsigned long size = 0;

	struct alloc_struct *ptr = &head;	
	while(ptr->next){	
		if(ptr->next->address == start){
			size = ptr->next->size;
			break;
		}
		ptr = ptr->next;
	}
	
	return size;
}
EXPORT_SYMBOL(mempool_get_size);


/*
*	Description: Get a memory erea from mempool 
*/
unsigned long 	mempool_malloc(INT32U num_bytes)
{
    
    struct alloc_struct *ptr, *newptr;
        
    if (num_bytes <=0||num_bytes > reserved_mem_size)
    {
        printk(KERN_ERR"mempool illegal agument:%d\n",num_bytes);
        return 0;   
    }
 
    ptr = &head;
/*
*	Traverse the linklist to find the big enough space
*/    
    while (ptr && ptr->next) 
    {
       if (ptr->next->address >= (ptr->address + ptr->size + num_bytes))
                break; /*get enough space node*/
        ptr = ptr->next;
    }
    if (!ptr->next)  /* end of list */
    {
        printk(KERN_ERR"mempool failed: no spare space!\n");
        return 0; 
    }

    newptr = (struct alloc_struct*)kzalloc(sizeof(struct alloc_struct),GFP_ATOMIC);	
    
    if (!newptr)
    {
        return 0;
    }


   newptr->address = ptr->address + ptr->size;

   newptr->size = num_bytes;
   newptr->next = ptr->next;
   ptr->next = newptr;
        
   printk("mempool():node:0x%x returning 0x%x(size:0x%x),next: 0x%x\n",(unsigned int)newptr,(unsigned int)newptr->address, (unsigned int)newptr->size, (unsigned int)newptr->next->address);

   return newptr->address;
}


EXPORT_SYMBOL(mempool_malloc);

int mempool_dalloc(unsigned long p)
{
   
    struct alloc_struct *ptr, *prev;

    if(p==0)
    {
    	 printk(KERN_ERR"free error!p:%x\n",(unsigned int)p);
	 return -EFAULT;
    }
    ptr = &head; 

    while (ptr->next) {
        if (ptr->next->address == (unsigned long)p) /*find the right node*/
            break;
        ptr = ptr->next;
    }
 
    /* the one being freed is ptr->next */
    prev = ptr; 
    ptr = ptr->next;

    if (!ptr) {
        printk(KERN_ERR"free(0x%x) but addr not allocated\n",(unsigned int)p);
        return EFAULT;
    }
    printk(KERN_DEBUG"free():freeing 0x%x (0x%x) next 0x%x\n",(unsigned int)ptr->address,(unsigned int)ptr->size, (unsigned int)ptr->next->address);
    prev->next = ptr->next;
    kfree(ptr);
    return 0;
}

EXPORT_SYMBOL(mempool_dalloc);

#if 1
static int __init reserved_mem_early(char *p)
{
	reserved_mem_size = memparse(p, &p);	
	printk(KERN_DEBUG"%s:%d  reserve_mem_size:0x%x\n",__func__,__LINE__,(unsigned int)reserved_mem_size);

	return 0;
}

early_param("reserved_mem", reserved_mem_early);
#endif

unsigned long  __init mem_pool_init(unsigned long pfn, unsigned int is_end)
{
#if 0
	unsigned long next_pfn;
	reserve_mem_start =__pa(PFN_PHYS(start_pfn));
	next_pfn = PFN_UP(__pa(reserve_mem_start+PFN_ALIGN(reserve_mem_size)));
	printk("reserve mem start_pfn:%x   next_pfn:%x\n",(unsigned int)start_pfn,(unsigned int)next_pfn);
	/*init linklist*/
//	init_linklist(reserve_mem_start,PFN_ALIGN(reserve_mem_size));
	printk("a. reserved: start=0x%08x, size=0x%08x\n", reserve_mem_start, reserve_mem_size);
#else
	reserved_mem_size = PFN_ALIGN(reserved_mem_size);
	reserved_mem_start = (unsigned long)(is_end?(__va(PFN_PHYS(pfn))-reserved_mem_size):__va(PFN_PHYS(pfn)));
	
	printk(KERN_DEBUG"b. reserved: start=0x%08lx, size=0x%08lx\n", reserved_mem_start, reserved_mem_size);
	init_linklist(reserved_mem_start, reserved_mem_size);
#endif
	return is_end?PFN_UP((unsigned long)__pa(reserved_mem_start)):PFN_UP((unsigned long)__pa(reserved_mem_start+reserved_mem_size));
}

int mem_get_pgoff(unsigned long start)
{
	return PFN_DOWN(start -reserved_mem_start);
}

int mem_get_phy(unsigned long mem_offset, unsigned long *addr)
{
	if(mem_offset >=reserved_mem_size)
		return -EFAULT;
	
	*addr = reserved_mem_start + mem_offset;
	
	return 0;
}
