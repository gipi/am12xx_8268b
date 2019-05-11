#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>  


#define NET_DEVICE_STAT_PATH    "/proc/net/dev"

static long long us_gettimeofday()
{
        struct timeval tv;
        
        gettimeofday(&tv, NULL);

        return ((long long)tv.tv_sec)*1000*1000 + tv.tv_usec;

}

static int getNetDeviceMbps(char *device)
{
    FILE * fp;
    char buf[1025];
    char tmp[64];
    char *str, *path = NET_DEVICE_STAT_PATH;
    int len, i;
    int ret = 0;
    long long bytes, nowTime;
    double bps = 0.0;
    static long long oldTime = 0;
    static long long oldBytes = 0;

    fp = fopen(path, "r");

    if(NULL ==  fp){
        printf("open fail, path=%s\n", path);
        ret = -1;
        goto EXIT;
    }

    nowTime = us_gettimeofday();
    len = fread(buf, 1, sizeof(buf)-1, fp);

    if(len <= 0){
        printf("read fail, len=%d\n", len);
        ret = -1;
        goto EXIT;
    }
    
    buf[len] = '\0';
    str = strstr(buf, device);

    if(str){
        str += strlen(device) + 1;
        i = 0;
        do{
            tmp[i++] = *str++;
        }while(*str != ' ');
        tmp[i] = '\0';

        bytes = atoll(tmp);

        double cBytes = bytes - oldBytes;
        double cTime = nowTime - oldTime;

        cBytes /= 1024*1024;//MB
        cTime /= 1000*1000;//s

        if(oldTime){
            bps = cBytes*8/cTime;
            printf("%s: %llfMbps\n", device, bps);
        }
        oldBytes = bytes;
        oldTime = nowTime;
    }

EXIT:
    if(fp){
        fclose(fp);
    }
    
    return ret;
    
}


int main(int argn, char *argc[])
{
    char *device = "wlan0";    
    int scanTime = 1;

    if(argn > 1)
        device = argc[1];

    if(argn > 2){
        scanTime = atoi(argc[2]);
    }

    printf("Net Mbps test, device=%s, scantime=%ds\n", device, scanTime);

    while(1){
        if(getNetDeviceMbps(device)<0){
            break;
        }
        sleep(scanTime);
    }
    
    
EXIT:
    return 0;
}


