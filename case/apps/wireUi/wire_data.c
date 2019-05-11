#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#define DEFAULT_RECV_LEN	(1*1024*1024)


typedef struct recvInfo_t{
	int remainLen;
	int buffLen;
	unsigned char *recvBuff;
}recvInfo_t;

recvInfo_t recvInfo = {0, 0, NULL};

typedef struct amPkgHeader_t {
    int count;
    int totalSize;
    int flag;
    char a;
    char b;
    char c;
    char d;
    int format;
    int width;
    int height;
    int size;   
}amPkgHeader_t;

int isVideoStart = 0;


int initVideo()
{
    printf("ENTER initVideo");
   // QuattroTX_DisplayInitVideo();
	isVideoStart = 1;
    
    return 0;
}

void uninitVideo()
{
	printf("ENTER uninitVideo");
	isVideoStart = 0;
}



// find a binary pattern in an array
uint8_t *find_in_buffer(const uint8_t *buffer, int buflen, const uint8_t *needle, int needle_len)
{
    uint8_t *buf = (uint8_t *)buffer;
    uint8_t *found = NULL;
    
    #if 0
    while (1) {
        uint8_t *p = (uint8_t *)memmem(buf, buflen, needle, needle_len); 
        if (!p) 
            break;
        last_needle = p;
        buflen -= (p + needle_len) - buf;
        buf = p + needle_len;
    }
    #endif

    if(buf == NULL || buflen <= 0 || needle == NULL || needle_len <= 0)
    {    
        printf("args error(buf: %p, buflen: %d, needle: %p, needle_len: %d)", buf, buflen, needle, needle_len);
        return NULL;
    }    
    found = (uint8_t *)memmem(buf, buflen, needle, needle_len); 

    return found;
}

int recvOnePackage(amPkgHeader_t *header, unsigned char *data, unsigned int len)
{
	if(header == NULL || data == NULL)
	{
		printf("header[%p] or data[%p] is NULL\n", header, data);
		return -1;
	}
	//DumpMemory(data, (len < 64)?len:64);
	unsigned char *vdata = data;
	int vlen = len;

	if(header->format == 7)
	{
#if 1	
		if(len > 5 && vdata[0] == 0 && vdata[1] == 0 && vdata[2] == 0 && vdata[3] == 0x1)
		{
			if((vdata[4]&0x1f) == 7 || (vdata[4]&0x1f) == 8)
			{
				unsigned char *codec = vdata;
				int codecLen = 5;
				vdata += 5;
				vlen -= 5;
				while(vdata != NULL)
				{
					unsigned char *p = NULL;
					uint8_t h[4] = {0x00, 0x00, 0x00, 0x01};
					p = find_in_buffer(vdata, vlen, h, 4);
					if(p == NULL)
					{
						codecLen = len;
						vdata = NULL;
						vlen = 0;
						break;
					}
					if((p[4]&0x1f) == 7 || (p[4]&0x1f) == 8)
					{
						p += 5;
						vdata = p;
						vlen = len - (vdata - data);
					}
					else
					{
						vdata = p;
						codecLen = vdata - data;
						vlen = len - codecLen;
						break;
					}
				}
				wire_SetFrameSPSPPS(codec, codecLen);//spspps
			}
		}
		if(vdata != NULL)
		       wire_HantroPlay(vdata, vlen, header->width, header->height);
#else		// For test
		unsigned char *start = vdata;
		while(vdata != NULL)
		{
			int plen = vlen;
			unsigned char *p = NULL;
			uint8_t h[4] = {0x00, 0x00, 0x00, 0x01};
			p = find_in_buffer(vdata+4, vlen, h, 4);
			if(p != NULL)
			{
				plen = p - vdata;
				vlen -= plen;
			}
			printf("width: %d, height: %d", header->width, header->height);
			DumpMemory(vdata, (plen < 64)?plen:64);
			vdata = p;
			if(vlen <= 0 || vdata == NULL)
				break;
		}
#endif
	}
#ifdef DEBUG_PRINT_HEX
	else
	{
		printf("Unknown data.");
	}
#endif

	return 0;
}

int ezMirrorDataHandle(void *data, int len)
{
	if(recvInfo.recvBuff == NULL)
	{
		printf("To malloc %d size.\n", DEFAULT_RECV_LEN);
		recvInfo.recvBuff = (unsigned char *)malloc(DEFAULT_RECV_LEN);
		if(recvInfo.recvBuff == NULL)
		{
			printf("Malloc buffer fail.(%s)", strerror(errno));
			return -1;
		}
		recvInfo.remainLen = 0;
		recvInfo.buffLen = DEFAULT_RECV_LEN;
	}

	if(recvInfo.remainLen + len > recvInfo.buffLen)
	{
		int newLen = recvInfo.remainLen + len + 1;
		printf("To realloc %d size.", newLen);
		recvInfo.recvBuff = realloc(recvInfo.recvBuff, (newLen));
		if(recvInfo.recvBuff == NULL)
		{
			printf("Realloc buffer fail.(%s)", strerror(errno));
			recvInfo.remainLen = 0;
			return -1;
		}
		recvInfo.buffLen = newLen;
	}
	
	memcpy(recvInfo.recvBuff+recvInfo.remainLen, data, len);
	recvInfo.remainLen += len;

	while(1)
	{
		if(recvInfo.remainLen < sizeof(amPkgHeader_t))
			return 0;
		
		amPkgHeader_t *header = (amPkgHeader_t *)recvInfo.recvBuff;
		if(header->flag != 2 || header->size > 4194304) //Max package lenght is 4M.
		{
			printf("The package is broken.");
			recvInfo.remainLen = 0;
			return -1;
		}
		
		if((recvInfo.remainLen - sizeof(amPkgHeader_t)) < header->size)
			return 0;
		
		unsigned char *p = recvInfo.recvBuff + sizeof(amPkgHeader_t);
		recvOnePackage(header, p, header->size);
		
		recvInfo.remainLen -= (sizeof(amPkgHeader_t) + header->size);
		if(recvInfo.remainLen > 0)
		{
			p += header->size;
			memmove(recvInfo.recvBuff, p, recvInfo.remainLen);
		}
	}
	
	return 0;
}
int ezMirrorDataExit()
{
	printf("ezMirrorDataExit!\n");
	recvInfo.remainLen=0;
	return 0;
}


