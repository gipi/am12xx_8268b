#ifndef LINEAR_H
#define LINEAR_H

#include "buddy.h"

struct mem_heap;

/**
 * The heap is made up as a list of structs of this type.
 * This does not have to be aligned since for getting its size,
 * we only use the macro SIZEOF_STRUCT_MEM, which automatically alignes.
 */
typedef struct mem {
  /** 'b' for buddy page, 'l' for linear page */
  char type;
  /** 1: this area is used; 0: this area is unused */
  unsigned char used;
  /** reserved */
  char reserved[2];
  /** index (-> ram[next]) of the next struct */
  unsigned int next;
  /** index (-> ram[next]) of the next struct */
  unsigned int prev;
  /** heap */
  struct mem_heap * heap;
} linear_page_t;

typedef struct mem_heap {
	unsigned char *ram;
	unsigned int ram_size;
	/** the last entry, always unused! */
	linear_page_t *ram_end;
	/** pointer to the lowest free block, this is used for faster search */
	linear_page_t *lfree;
} linear_heap_t;

#define SIZEOF_STRUCT_MEM    BUDDY_PAGE_SIZE /*MEM_ALIGN_SIZE(sizeof(struct mem))*/
#define SIZEOF_STRUCT_HEAP	 MEM_ALIGN_SIZE(sizeof(struct mem_heap))

void linear_init(void * buf, unsigned int size);
unsigned int linear_free(void *rmem);
void * linear_alloc(struct mem_heap * heap, unsigned int size);
int linear_dump(struct mem_heap * heap, int dump);

#endif

