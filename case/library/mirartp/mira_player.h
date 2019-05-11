
#ifndef  _EZ_PLAYER_H_
#define  _EZ_PLAYER_H_

#include <sys/time.h>
#include <pthread.h>
#include <semaphore.h>
#include "av_api.h"


int ezMiraSetFile();

int ezMiraSetParam(unsigned char*vheap,unsigned int pheap,unsigned int heapsize,int w,int h);

void *ezMiraOpen();

void ezMiraClose(void);

void ezMiraPlay(void);

void ezMiraStop(void);

void ezMiraInit();

long _mirartpStreamSeek(void *fp, int64 contentOffset);
int _mirartpStreamRead(void *fp, unsigned char *buf, int size);
void *_mirartpStreamOpen(char *path, char *mode);
int _mirartpStreamClose(void *fp);
int64 _mirartpStreamTell(void *fp);
int64 _mirartpStreamGetLen(void *fp);
int _mirartpStreamGetSeekable(void *fp);



#endif //_EZ_PLAYER_H_

