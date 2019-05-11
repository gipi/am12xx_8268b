#ifndef _SYSINFO_H
#define _SYSINFO_H

#include "comval.h"
//#include "amTypedef.h"
//#include "../include/comval.h"

//#define MS_DRM_MTP


/*************************DIR_t*************************************************/
//定义fw image文件目录项的数据结构
typedef struct
{
    char filename[11];  //文件名8.3
    char type;          //’A’,’B’,’H’,’F’, ’S’, ’U’, ’I’
    long DownloadAddr;  //download 地址
    long offset;        //在文件中的偏移，byte为单位
    long length;        //长度byte为单位（512取整）
    char Subtype[4];    //子类型
    long checksum;      //DWORD累加
}__attribute__ ((packed)) DIR_t;                 //32字节


typedef enum
{
	S_INIT,
    	S_WAIT,
    	S_RUNNING,
	S_FINISHED,
	S_ERROR
}Fwu_state_t;   //表示upgrade.drv的运行状态

typedef struct
{
    int prg;	//进度值，0～100，由upgrade.drv写入
    Fwu_state_t state;
}Fwu_status_t;	//upgrade.drv返回的结构体


/*struct SD_DIR
{
    INT8S    fname[11];
    INT8U    fattr;
    INT8U    reserve0[2];
    INT16U   version;
    INT32U   offset;
    INT32U   size;
    INT32U   reserve1;
    INT32U   checksum;
} ;***/

typedef struct
{
    char    fname[11];
    char    fattr;
    char    compress;
    char    reserve0;
    short   version;
    unsigned int    offset;
    unsigned int    size;
    unsigned int    reserve1;
    unsigned int    checksum;
} SD_DIR;


//typedef enum
//{
//	SHOW_RECOVER_PIC,   // 恢复显示
//    	SHOW_CARDUP_PIC,	// Card Mass product  界面
   // 	SHOW_PROGRESS_PIC// 
	
//}show_type;   //表示upgrade.drv的运行状态
#define SHOW_RECOVER_PIC  0x00
#define SHOW_CARDUP_PIC	0x01
#define SHOW_PROGRESS_PIC  0x02
/**typedef void (*recoverfun)(long param0, long param1, long param2, long param3);
//param0: nand_flash_entry  Read API Function
//param1: chipinfo: 
//param2: Show_ type: 
     param3: Fwu_status_t 结构体指针
**/	


/*
******************************************
DownloadAddr  Byte 0    type
                0       Ram
                16      on board physical flash
                17      on board logic flash

type  ‘A’ = ADFUS
      ‘B’ = BREC
      ‘H’ = HWSC
      ‘F’ = FWSC
      ‘S’ = RCSL，RCSE
      ‘U’ = ADFU AP
      ‘I’ = LFI
      ‘M’ = MBrec
******************************************
*/


/*************************LDIR_t*************************************************/
//定义logical firmware目录项的数据结构
typedef struct
{
    char filename[11];  //文件名8.3
    char attr;
    char subtype;
    char reserve1;
    unsigned short version;
    long offset;        //相对于LFIHead_t的偏移，以扇区（512字节）为单位
    long length;        //长度byte为单位（512取整）
    char reserve2[4];
    long checksum;      //DWORD累加
}__attribute__ ((packed)) LDIR_t;                //32字节


/*************************AFIHead_t*************************************************/
//定义firmware image 文件头的数据结构
typedef struct
{
    char AFIFlag[4];    //={'A','F','I',' '};
    unsigned short PID;           //设备pid
    unsigned short VID;           //设备vid
    char Version[13];   //版本号xx.xx.xx.xxxx
    char Date[4];
    char reserve1[7];
    DIR_t diritem[126]; //最多126
    char reserve2[28];
    long checksum;      //afi head校验和，DWORD累加
}__attribute__ ((packed)) AFIHead_t ;             //4kB

/*************************LFIHead_t*************************************************/
//定义logical firmware image信息头的数据结构
//typedef struct
//{
//    char magic[2];
//    char systemtime[4];
//    char RTCRate[2];
//    char DisplayContrast;
//    char LightTime;
//    char StandbyTime;
//    char SleepTime;
//    char langid;
//    char ReplayMode;
//    char OnlineMode;
//    char BatteryType;
//    char FMBuildInFlag;
//    char RecordType;
//    char BLightColor;
//    char Onlinedev;
//    char SuppCard;
//    char MTPFormatType;
//    char Reserve[42];   //保留
//}__attribute__ ((packed)) ComVar_t ;              //64字节

#define SCODE_ACCESS        0
#define SCODEBAK_ACCESS     1
#define APPINFO_ACCESS      2
#define LICENSE_ACCESS      3
//#define MTPINFO_ACCESS      4
#define USRPRIV_ACCESS      4
#define DSPCODE_ACCESS      5
#define AUTO_RUN_ACCESS        6
#define UDISK_ACCESS        7

#define Scode_cap           0x7B0//0x1E00
#define Scodebak_cap        0
#define Appinfo_cap         32//128
#define Lic_cap             0
#define Usr_cap             0
#define Dspcode_cap         0
#define Auto_cap           0
#define Udisk_cap           0x1000//0x1400

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
    comval_t SysComVar;     //64字节
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


/*************************MBREC*************************************************/
typedef struct
{
    char code[500];                     //BrecLauncher code segment
    char Actflag[6];                    //"ActBrm"
    short int MBrecSign;                //Flag:0x55AA
    int CheckSum;                       //Check sum of MBREC +offset(0x1234)
}__attribute__ ((packed)) MBRC_t ;      //512bytes


/*************************BREC**************************************************/
//定义brec文件的数据结构
typedef struct
{
    char code[0xEFFC];                //Brec code segment
    unsigned short flag;             //Flag:0x55aa
    unsigned short checksum;         //Check sum of BREC
}__attribute__ ((packed)) BREC_t;


/*************************HWSC**************************************************/
//定义hardware scan任务文件的数据结构
typedef struct
{
    char jump[4];                           //第一条语句
    char HWSCFlag[4];//={'H','W','S','C'};
    char type[4];                           //"0000"
    unsigned short version;                           //HWSC version, x.x.xx
    char date[4];                           //xxxx.xx.xx
    char reserve[2];
    unsigned short length;
//    char code[length];
}__attribute__ ((packed)) HWSCTask_t;


/*************************FWSC**************************************************/
//定义fwscan任务文件的数据结构
typedef struct
{
    char jump[4];               //第一条语句
    char FWSCFlag[4];//={'F','W','S','C'};
    char type[4];               //'F644'/'F641'/'F321'...
    unsigned short version;               //fwsc version, x.x.xx
    char date[4];               //xxxx.xx.xx
    char reserve[2];
    unsigned short length;
//    char code[length];
}__attribute__ ((packed)) FWSCTask_t;


/*************************HWSC RETURN*********************************************/
typedef struct
{
    char StgInfor[4][2];    //Storage 的连接信息
    unsigned int CExCap[8];        //Cap of storages, the unit is sector
}__attribute__ ((packed)) STGInfo_t;                 //40字节


typedef struct {
    char Frametype[2];      //='H''W'
    unsigned short ICVersion;         //3963
    char SubVersion[2];     //A,B,C,D...
    char BromVersion[4];    //such as "3.0.00.8888"
    char BromDate[4];       //such as "20,06,02,14"
    unsigned int BootDiskType;     //boot disk 的存储类型, 'F644'/'F648'/'F321'...
    STGInfo_t stginfo;
    unsigned short SectorPerBlock; //Block size
    char reserve[20];
}__attribute__ ((packed)) ADFU_HWScanInfo_t;         //80bytes


/*************************FWSC RETURN**********************************************/
/*@fish add 2011-01-11 在取代 , 为了与以前兼容，名称暂时不修改，
char DeviceName[32];    //设备名
长度定义32个Byte, 
*/
typedef struct {    
    unsigned short   PagePerBlk;        //Page number per physical block    
    char    BCH_type;          //BCH8,BCH12,BCH24,
    unsigned short     Page_size;         //boot data Page 2K for BCH8,bch12,4KB for bch24
    unsigned short    SECTSIZE;          //boot Data sector 512byte 1KB
    char    rev[25];    
}__attribute__ ((packed))BOOT_Code;

typedef struct
{
    char FrameType[2];      //'F','W'
    unsigned short VID;
    unsigned short PID;
    char fwVersion[13];     //韧件版本号xx.xx.xx.xxxx
    char reserved[3];       //rev[0]=0x55 表示新的wrtie 
    char FwStatus[2];       //0 正确；1 mbrc恢复错误；2 brec恢复错误
    char Productor[32];     //制造商
    char DeviceName[32];    //设备名
    char FirmwareDate[4];
//    char BrecCheckSum[4];     //Nanflash 读出的Brec Code 的CheckSum， add in 2006-02-15
    unsigned short Brecsign;           //BREC标志,0x55AA
    unsigned short BrecCheckSum;       //不能修改，Nanflash 读出的Brec Code 的CheckSum， add in 2006-02-15
    CapInfo_t CapInfo;          //48字节 不能修改
}__attribute__ ((packed)) ADFU_FWScanInfo_t;         //144bytes


/*************************SYSINFO**************************************************/
typedef struct
{
    char SYSInfoFlag[8];    //'S','Y','S',' ','I','N','F','O'
    ADFU_HWScanInfo_t adfu_hwscaninfo;
    ADFU_FWScanInfo_t adfu_fwscaninfo;
    char Reserve[24];
}__attribute__ ((packed)) ADFU_SysInfo_t;        //8+80+144+24=256 bytes

typedef struct
{
        int addr;
        int length;
}__attribute__ ((packed)) ADFU_TaskInfo_t;

/**********************chip type*******************************/
#define SNOR080		0x3038304e
#define SNOR090		0x3039304e
#define NAND284 		0x34383246
#define NAND644 		0x34343646
#define NAND648 		0x38343646
#define NAND649 		0x39343646

#endif
