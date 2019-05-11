
#include "am_types.h"

#ifdef DEBUG_ALLOC
#define DMSG_ALLOC(stuff...)		printf(stuff)
#else
#define DMSG_ALLOC(stuff...) 	do{}while(0)
#endif

struct alloc_struct {
    INT32U address;
    INT32U size;
    struct alloc_struct *next;
#ifdef DEBUG_ALLOC
   INT32U reg;
#endif
    INT32U reserve;
};

#define MY_BYTE_ALIGN(x) (((x + 15)/16)*16)             /* alloc based on 16 byte */

#if 0
#define heap_buf        0xa08c0000                      /* heap start address                          */
#define heap_buf_size   0x00040000                      /* heap size                                   */
#endif

extern unsigned char heap_start[], heap_end[];

static struct alloc_struct head,tail;

#ifdef  DEBUG_ALLOC
 #include "regdef.h"
static INT32U ra_reg=0; 
 #endif
/*
*********************************************************************************************************
*                       INIT HEAP
*
* Description: init heap.
*
* Aguments   :
*
* Returns    :
*********************************************************************************************************
*/
INT32S init_heap()
{
    //memset((void *)startaddr,0,heap_buf_size);
    head.size = tail.size = 0;
    head.address = (INT32U) heap_start;
    tail.address = (INT32U) heap_end;
    head.next = &tail;
    tail.next = NULL;

    return 1;
}


/*
*********************************************************************************************************
*                       INIT HEAP FOR NAND FLASH DRIVER
*
* Description: init heap for nand flash driver.
*
* Aguments   :
*
* Returns    :
*********************************************************************************************************
*/
INT32S init_heap_for_nand(void)
{
    INT32S result;
    result = init_heap();
    return result;
}


/*
*********************************************************************************************************
*                       MALLOC BUFFER FROM HEAP
*
* Description: malloc a buffer from heap.
*
* Aguments   : num_bytes    the size of the buffer need malloc;
*
* Returns    : the pointer to buffer has malloc.
*********************************************************************************************************
*/
void *malloc(INT32U num_bytes)
{

 #ifdef DEBUG_ALLOC
    ra_reg = GET_CPU_REG(ra);
   DMSG_ALLOC("malloc %8x,ra=%8x\n",num_bytes,ra_reg);
 #endif
 
    struct alloc_struct *ptr, *newptr;

    if (!num_bytes) return 0;

    num_bytes = MY_BYTE_ALIGN(num_bytes);       /* translate the byte count to size of long type       */

    ptr = &head;                                /* scan from the head of the heap                      */

    while (ptr && ptr->next)                    /* look for enough memory for alloc                    */
    {
        if (ptr->next->address >= (ptr->address + ptr->size + \
                2*sizeof(struct alloc_struct) + num_bytes)) break;
                                                /* find enough memory to alloc                         */
        ptr = ptr->next;
    }

    if (!ptr->next)
    {
    #ifdef DEBUG_ALLOC 
        DMSG_ALLOC("<><><><><><>malloc 0x%x failed: no spare space!<><><><><><>\n",  num_bytes);
    #endif    
        return 0;                   /* it has reached the tail of the heap now             */
    }

    newptr = (struct alloc_struct *)(ptr->address + ptr->size);
                                                /* create a new node for the memory block              */
    if (!newptr)
    {
        return 0;                   /* create the node failed, can't manage the block      */
    }

    /* set the memory block chain, insert the node to the chain */
    newptr->address = ptr->address + ptr->size + sizeof(struct alloc_struct);
    newptr->size = num_bytes;
    newptr->next = ptr->next;
    
#ifdef DEBUG_ALLOC
    newptr->reg = ra_reg;
#endif
    ptr->next = newptr;
#ifdef DEBUG_ALLOC    
   DMSG_ALLOC("malloc:node:0x%x rnt 0x%x(size:0x%x),next: 0x%x\n",newptr,newptr->address, newptr->size, newptr->next->address);
#endif
    return (void *)newptr->address;
}


/*
*********************************************************************************************************
*                       FREE BUFFER TO HEAP
*
* Description: free buffer to heap
*
* Aguments   : p    the pointer to the buffer which need be free.
*
* Returns    : none
*********************************************************************************************************
*/
void free(void *p)
{
    struct alloc_struct *ptr, *prev;

    ptr = &head;                /* look for the node which point this memory block                     */
    while (ptr && ptr->next)
    {
        if (ptr->next->address == (INT32U)p)
            break;              /* find the node which need to be release                              */
        ptr = ptr->next;
    }

   prev = ptr;
    ptr = ptr->next;

    if (!ptr) return;           /* the node is heap tail                                              */
	

    prev->next = ptr->next;     /* delete the node which need be released from the memory block chain  */

    return ;
}


