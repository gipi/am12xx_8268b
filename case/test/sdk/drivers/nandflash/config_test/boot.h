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

#define Multi_FLAG "MultiSectorBoot" //size of 16 


struct BootBlk_Info
{
    INT8U       name[4]; // mbrc or brec
    INT8U       blknum; // blocknum 0 1,2,3 ....
    INT8U       check; // check sum fail or ok
    INT16U      Flag; //0x55 aa 
    INT16U      Flashchecksum; //flash IC checksum
    INT16U      Calchecksum;  // 计算checksum ;
    INT8U       ECC_Bit_Max;    //boot code  each sector max error ECC bit ; 
    INT16U      ECC_Bit_Total;  //boot code  total Error ECC bit
    INT16U		ECC_Err_Cnt;    //boot code toal sector count 
    INT8U       RecalaimVal;    //read recalim 根据不同flash ID此值不相同。
    INT8U       BCHType;        //BCH8 value 8, BCH12 value 12 ....
    INT8U       rev[12];        //reserved 
};

INT32U bSet_Value_ECC(INT8U *Buffer);
INT32S static brec_read_check_sum(unsigned short BlockStartNum, unsigned short SectorStartNum, INT8U *BufferAddr,unsigned int MAXSec);
int static m_mbrccheck(void);
int static m_breccheck(void);
int m_bootcheck(void);
