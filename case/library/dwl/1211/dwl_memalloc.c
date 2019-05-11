#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "dwl.h"
#include "basetype.h"
#include "sys_buf.h"
#include <fcntl.h>


static i32 sysbuf_fd = -1;

i32 DWLSysbufMalloc(u32 size, DWLLinearMem_t * info)
{
	DWLLinearMem_t *buff = (DWLLinearMem_t *)info;
	void * buf_addr;
	i32 err;
	struct mem_dev heap;

	if((buff==NULL)||(size==0) )
		return DWL_ERROR;

	if(sysbuf_fd == -1){
		sysbuf_fd = open("/dev/sysbuf",O_RDWR);
		if(sysbuf_fd == -1)
		{
			printf("open /dev/sysbuf error\n");
			return DWL_ERROR;
		}
	}

	heap.request_size = size;
	heap.buf_attr = UNCACHE;
	err = ioctl(sysbuf_fd,MEM_GET,&heap);
	if(err == -1){
		return DWL_ERROR;
	}

	buff->virtualAddress =(u32 *) heap.logic_address;
	buff->busAddress = (u32) heap.physic_address & 0x1fffffff;
	buff->size=size;

	return DWL_OK;
}

i32 DWLSysbufFree(DWLLinearMem_t * info)
{
	i32 err;
	struct mem_dev heap;
	
	if(info == NULL || sysbuf_fd==-1){
		return DWL_ERROR;
	}

	heap.physic_address = info->busAddress | 0x80000000;
	heap.request_size = info->size;
	err = ioctl(sysbuf_fd,MEM_PUT,&heap);
	if(err == -1){
		return DWL_ERROR;
	}

	return DWL_OK;
}

