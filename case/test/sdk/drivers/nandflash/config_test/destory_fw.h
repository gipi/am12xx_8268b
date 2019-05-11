#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys_msg.h>
#include <sys_gpio.h>
#include <sys_rtc.h>
#include <sys_timer.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys_vram.h>

typedef struct 
{
	char  Sys_Flag[16];
	unsigned int Sys_offset;
	unsigned int Sys_len;
	unsigned int Sys_para1;
	unsigned int Sys_para2;
}__attribute__ ((packed))Sys_File;

typedef struct
{
    unsigned int Scodecap;     //系统代码空间大小，扇区为单位
    unsigned int Scodebakcap;  //代码空间备份区大小，扇区为单位
    unsigned int Appinfocap;   //Appinfo空间大小，扇区为单位
    unsigned int Liccap;       //license空间大小，扇区为单位，flash驱动初始化时根据flash大小与license空间对应关系修改
    unsigned int Micap;        //Mtp info空间大小，扇区为单位，flash驱动初始化时根据flash大小与mtpinfo空间对应关系修改
    unsigned int Dspcap;       //DSP CODE空间大小，扇区为单位
    unsigned int Autocap;     //用户代码空间大小，扇区为单位，fwmaker工具根据用户设置修改
    unsigned int Udiskcap;     //Udisk空间大小，扇区为单位
    unsigned int Logicalcap;   //整个flash的逻辑空间大小，扇区为单位
    unsigned int Reserved[3];	
}__attribute__ ((packed)) CapInfo_t;             //48字节

typedef struct
{
    char LFIFlag[4];        //={0x55,0xaa,0xf2,0x0f};
    char Version[15];       //版本号xx.xx.xx.xxxx
    char Reserve0;
    char Date[4];           //xxxx.xx.xx
    unsigned short VID;               //xxxx
    unsigned short PID;               //xxxx
    long DIRItemCheckSum;   //所有的240个目录项加起来，//DWORD累加
	//32字节
	
    char OEMName[16];
    char Designer[16];
    char Productor[32];
    char DeviceName[32];
    CapInfo_t CapInfo;      //48字节
    char SysComVar[64];     //64字节
    char Reserve1[16];      //保留
	//7*32
	
    char UsbVendor[8];
    char UsbProductID[16];
    char UsbVersion[8];
    char UsbSetupInfo[48];
    char Mtpmfrinfo[33];
    char Mtpprodinfo[33];
    char Mtpprodver[17];
    char Mtpprodsn[17];
    char MtpVID[2];
    char MtpPID[2];
    char Reserve2[15];      //保留
    char Volume[5][11];
    unsigned short Headchecksum;      //前510byte校验和, WORD累加
	//    LDIR_t diritem[240];    //逻辑目录项7680byte
} __attribute__ ((packed)) LFIHead_t;                 //8k



void static hex_printf2(  const INT8U* pData, INT16U inLen);
int static destory_kernel(int flag1,int flag2);
int static destory_root(int flag1);

 int Destory_FW(unsigned int sum_param,char * param_buf);