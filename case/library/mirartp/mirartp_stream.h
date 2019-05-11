#ifndef STREAM_API_H
#define STREAM_API_H

#include <stdio.h>
#include <stdlib.h>


#ifdef __cplusplus
extern "C" {
#endif

extern int mirartpStreamStart(int rtpPort,unsigned char *vHeap,unsigned int pHeap,unsigned int heapSize,int w,int h);

extern int mirartpStreamStop();

#ifdef __cplusplus
}
#endif

#endif

