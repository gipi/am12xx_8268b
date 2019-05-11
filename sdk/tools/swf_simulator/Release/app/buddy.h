#ifndef BUDDY_H
#define BUDDY_H

#define MEM_ALIGNMENT       32 /* May not work when burst = 8 */
#define MEM_ALIGN(addr)     ((void*)(((unsigned int)(addr) + MEM_ALIGNMENT - 1) & ~(unsigned int)(MEM_ALIGNMENT-1)))
#define MEM_ALIGN_SIZE(size) (((unsigned int)(size) + MEM_ALIGNMENT - 1) & ~(unsigned int)(MEM_ALIGNMENT-1))

#define BUDDY_PAGE_SIZE		MEM_ALIGN_SIZE(sizeof(buddy_page_t))
#define PAGE_ORDER          5  //32 Bytes
#define PAGE_SIZE           (1 << PAGE_ORDER)
#define MAX_TOTAL_ORDER		24 //16M Bytes
#define MAX_TOTAL_MEM		(1 << MAX_TOTAL_ORDER) 
#define BUDDY_UNIT_ORDER	16 //64K Bytes
#define BUDDY_UNIT_SIZE		((1 << BUDDY_UNIT_ORDER) - BUDDY_PAGE_SIZE)
#define MAX_BUDDY_HEAP      (MAX_TOTAL_MEM / BUDDY_UNIT_SIZE)
#define MAX_BUDDY_ORDER		(BUDDY_UNIT_ORDER - PAGE_ORDER + 1)

typedef struct page_list_head {
	struct page_list_head *next, *prev;
} page_list_head_t;

typedef struct {
	unsigned int heap_start;
	int heap_order;
} buddy_heap_t;

typedef struct {
	char type;					// 1 bytes
	char order;					// 1 bytes
	char reserved[2];			// 2 bytes 
	page_list_head_t lru;		// 8 bytes
	buddy_heap_t * heap;		// 4 bytes
} buddy_page_t;

/************************************************************************/
/* heap size = PAGE_SIZE * 2 ^ head_order                               */
/************************************************************************/
void buddy_init(void);
buddy_heap_t * buddy_create(unsigned int heap_start, unsigned int heap_order);
void * buddy_alloc(unsigned int size);
buddy_page_t * buddy_free(void * p);
int buddy_destroy(buddy_heap_t * heap);

#endif

