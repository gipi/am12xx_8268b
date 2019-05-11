#include "am_types.h"
/**
* @brief memory allocates a buffer.
* @param num_bytes buffer size(bytes).
* @return
* - ==NULL: malloc fail.
* - !=NULL: the buffer address.
* @note   1) You MUST invoke malloc(num_bytes) and free(p) in pair.  In other words, for every \n
*                 call to malloc(num_bytes) you MUST have a call to free(p).
* @ingroup heap_mgr
*/
void *malloc(INT32U num_bytes);

/**
* @brief free a buffer.
* @param p a pointer to the buffer address.
* @return None
* @note   1) You MUST invoke malloc(num_bytes) and free(p) in pair.  In other words, for every \n
*                 call to malloc(num_bytes) you MUST have a call to free(p).
* @ingroup heap_mgr
*/
void free(void *p);

INT32S init_mheap(INT32U startaddr, INT32U length);
INT32S init_heap();



