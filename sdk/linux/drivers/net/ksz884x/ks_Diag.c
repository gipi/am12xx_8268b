/* ---------------------------------------------------------------------------
          Copyright (c) 2003-2004 Micrel, Inc.  All rights reserved.
   ---------------------------------------------------------------------------

   ks884xDiag.c - This file includes SCORs for diagnosis

   Modification History:
   PCD   06/13/05  0.1.9    Added CLI for QC test.
   PCD   08/25/04           Created file.
   ---------------------------------------------------------------------------
*/

#define ACT_DEBUG_	printk("%s : %d \n", __FILE__, __LINE__);

#if !defined( DEF_LINUX )  &&  !defined( KS_ARM )
#include <stdlib.h>
#include <string.h>
#endif
#include "hardware.h"
#ifdef DEF_VXWORKS
#include "ioLib.h"
#ifdef KS_PCI_BUS
#include "ks884xEnd.h"
#endif
#ifdef KS_ISA_BUS
#include "ks884xEnd_shBus.h"
#endif
#endif

#undef CLI_DEBUG

/* if define it, then separate CLI command's argv by SPACE, otherwise, by ',' */
#ifdef KS_QC_TEST
#undef  SEPARATE_CLI_ARGV_BYSPACE       /* for QC test: SEPARATE CLI ARGV BY ',' */
#else
#define SEPARATE_CLI_ARGV_BYSPACE       /* normal: SEPARATE CLI ARGV BY SPACE  */
#endif

#define DELAY_IN_nMS_TO_CHECK      5    /* delay in ms unit to check if we received packet */


#define MIN_BANK_NUM            0
#define MAX_BANK_NUM           63
#define MIN_REGADDR_OFFSET      0
#ifdef KS_ISA_BUS
#define MAX_REGADDR_OFFSET     15
#elif KS_PCI_BUS
#define MAX_REGADDR_OFFSET     0x0554
#define MAX_DWORD_OFFSET       0x0200
#endif


#define MSG_ERROR_PREFIX        "ERROR: "
#define SPACE                   " "

#define BUFFER_COUNT            6
#define BUFFER_LENGTH           (2 * 1024)

static unsigned char FAR * apbBuffer[ BUFFER_COUNT ] =
{
   (unsigned char *) 0,
   (unsigned char *) 0
} ;
unsigned char packetBuf[ BUFFER_COUNT ][BUFFER_LENGTH];

extern UCHAR TestPacket[];


#define LINKMD_INFO_COUNT       10
unsigned long tempBuf[ LINKMD_INFO_COUNT ];
unsigned char FAR tempStrBuf[ (LINKMD_INFO_COUNT + 1) * sizeof (unsigned long) ];


const char FAR *hwHelpMsgStr[] =
{
#ifndef DEF_VXWORKS
#ifdef KS_ISA_BUS
/*  0 */  "BankNum    = { 0 ..     3f }   -- Bank number."NEWLINE,
/*  1 */  "RegNum     = { 0 ..      f }   -- Register number."NEWLINE,
#else
/*  0 */  "BankNum    = { 0           }   -- Dummy Bank number."NEWLINE,
/*  1 */  "RegNum     = { 0 ..   0554 }   -- Register number."NEWLINE,
#endif
/*  2 */  "RegData    = { 0 ..   ffff }   -- Register data to write."NEWLINE,
/*  3 */  "Width      = { 1 ..   2, 4 }   -- Register data width to read\\write (1:in BYTE, 2:in WORD, 4:in LONG)."NEWLINE,
/*  4 */  "BitMask    = { 0 ..   0001 }   -- Mask defining bit pattern field."NEWLINE,
/*  5 */  "BitPat     = { 0 ..   0001 }   -- Bit pattern to check for."NEWLINE,
/*  6 */  "BufNum     = { 0 ..      5 }   -- Debug buffer number."NEWLINE,
/*  7 */  "BufOffset  = { 0 ..    7ff }   -- Debug buffer offset."NEWLINE,
/*  8 */  "Len        = { 1 ..    800 }   -- Debug buffer length."NEWLINE,
#ifdef M16C_62P
/*  9 */  "WData      = { 0 ..     ff }   -- Buffer data to write (e.g. FF 00 01 0A)."NEWLINE,
#else
/*  9 */  "WData      = { \"0 ..   ff\" }  -- Buffer data to write (e.g. \"FF 00 01 0A\")."NEWLINE,
#endif
/* 10 */  "FData      = { 0 ..     ff }   -- Buffer data to fill."NEWLINE,
/* 11 */  "BufInc     = { 0 ..     ff }   -- Buffer data to fill by increasing count."NEWLINE,
/* 12 */  "Repeat     = { 0 ..      n }   -- Repeat times."NEWLINE,
/* 13 */  "Port       = { 0 ..      3 }   -- Port number(0-by lookup, 1-direct to port1, 2-direct to port2, 3-direct to port1 and 2)"NEWLINE,
/* 14 */  "SameBuf    = { 0 ..      1 }   -- 1 - use same buffer to Tx, 0 - continuous next buffer to Tx."NEWLINE,
/* 15 */  "TimeOut    = { 0 ..      n }   -- Time in ms to wait before giving up "NEWLINE,
/* 16 */  "Index      = { 0 ..      n }   -- Table index. "NEWLINE,
/* 17 */  "Count      = { 0 ..      n }   -- Number of table entry items (max Count: mac1-8, mac2-1024, vlan-6, mib-102). "NEWLINE,
/* 18 */  "RegDataH   = { 0 ..ffffffff}   -- Table Update Write Data Register High (bit_63 ~ bit_32). "NEWLINE,
/* 19 */  "RegDataL   = { 0 ..ffffffff}   -- Table Update Write Data Register Low  (bit_31 ~ bit_0 ). "NEWLINE,
/* 20 */  "DumpFlag   = { 0 ..      1 }   -- 1 - Start dumpping packet, 0 - Stop dummping packet."NEWLINE,
/* 21 */  "Delay      = { 0 ..     ff }   -- Delay ms between continous transmit packets."NEWLINE,
/* 22 */  "Table = {mac1,mac2,vlan,mib}   -- Table identifier (mac1=static MAC, mac2=dynamic MAC)"NEWLINE,
#ifdef KS_PCI_BUS
/* 23 */  "BusNo      = { 0 ..      n }   -- PCI bus number."NEWLINE,
/* 24 */  "DevNo      = { 0 ..      n }   -- PCI device number."NEWLINE,
/* 25 */  "FuncNo     = { 0 ..      n }   -- PCI function number."NEWLINE,
/* 26 */  "RegNo      = { 0 ..      n }   -- PCI register number."NEWLINE,
#endif /* #ifdef KS_PCI_BUS */

#else  /* vxWorks Shell command */
/*  0 */  "BankNum    = { 0              }  -- Dummy Bank number."NEWLINE,
/*  1 */  "RegNum     = { 0 ..    0x0554 }  -- Register number."NEWLINE
/*  2 */  "RegData    = { 0 ..0xffffffff }  -- Register data to write."NEWLINE,
/*  3 */  "Width      = { 1 ..      2, 4 }  -- Register data width to read\\write (1:in BYTE, 2:in WORD, 4:in LONG)."NEWLINE,
/*  4 */  "BitMask    = { 0 ..0x00000001 }  -- Mask defining bit pattern field. "NEWLINE,
/*  5 */  "BitPat     = { 0 ..0x00000001 }  -- Bit pattern to check for. "NEWLINE,
/*  6 */  "BufNum     = { 0 ..         5 }  -- Debug buffer number."NEWLINE,
/*  7 */  "BufOffset  = { 0 ..     0x7ff }  -- Debug buffer offset."NEWLINE,
/*  8 */  "Len        = { 1 ..     0x800 }  -- Debug buffer length "NEWLINE,
/*  9 */  "WData      = { \"0 ..      ff\" }  -- Buffer data to write (e.g. \"FF 00 01 0A\")."NEWLINE,
/* 10 */  "FData      = { 0 ..      0xff }  -- Buffer data to fill."NEWLINE,
/* 11 */  "BufInc     = { 0 ..         1 }  -- Buffer data to fill by increasing count."NEWLINE,
/* 12 */  "Repeat     = { 0 ..         n }  -- Repeat times."NEWLINE,
/* 13 */  "Port       = { 0 ..         3 }  -- Port number(0-by lookup, 1-direct to port1, 2-direct to port2, 3-direct to port1 and 2)"NEWLINE,
/* 14 */  "SameBuf    = { 0 ..         1 }  -- 1 - use same buffer to Tx, 0 - continuous next buffer to Tx."NEWLINE,
/* 15 */  "TimeOut    = { 0 ..         n }  -- Time in ms to wait before giving up "NEWLINE,
/* 16 */  "Index      = { 0 ..         n }  -- Table index. "NEWLINE,
/* 17 */  "Count      = { 0 ..         n }  -- Number of table entry items (max Count: mac1-8, mac2-1024, vlan-6, mib-102). "NEWLINE,
/* 18 */  "RegDataH   = { 0 ..0xffffffff }  -- Table Update Write Data Register High (bit_63 ~ bit_32). "NEWLINE,
/* 19 */  "RegDataL   = { 0 ..0xffffffff }  -- Table Update Write Data Register Low  (bit_31 ~ bit_0 ). "NEWLINE,
/* 20 */  "DumpFlag   = { 0 ..         1 }  -- 1 - Start dumpping packet, 0 - Stop dummping packet."NEWLINE,
/* 21 */  "Delay      = { 0 ..      0xff }  -- Delay ms between continous transmit packets."NEWLINE,
/* 22 */  "Table = {\"mac1\",\"mac2\",\"vlan\",\"mib\"}  -- Table identifier (mac1=static MAC, mac2=dynamic MAC)"NEWLINE,
#ifdef KS_PCI_BUS
/* 23 */  "BusNum     = { 0 ..         n }  -- PCI bus number."NEWLINE,
/* 24 */  "DevNum     = { 0 ..         n }  -- PCI device number."NEWLINE,
/* 25 */  "FuncNum    = { 0 ..         n }  -- PCI function number."NEWLINE,
/* 26 */  "RegNum     = { 0 ..         n }  -- PCI register number."NEWLINE,
#endif /* #ifdef KS_PCI_BUS */

#endif  /* #ifndef DEF_VXWORKS */

/*    */  " "NEWLINE,
#if defined (SEPARATE_CLI_ARGV_BYSPACE) && !defined (DEF_VXWORKS)
/*    */  "hwhelp                                              -- Display help message."NEWLINE,
/*    */  "hwread       BankNum  RegNum    Width               -- Read ks884X register."NEWLINE,
/*    */  "hwwrite      BankNum  RegNum    RegData  Width      -- Modify ks884X register."NEWLINE,
/*    */  "hwpoll       BankNum  RegNum BitMask BitPat TimeOut -- Poll register bit if it match bit pattern."NEWLINE,
/*    */  "hwbufread    BufNum   BufOffset Len                 -- Display contents of debug buffer."NEWLINE,
/*    */  "hwbufwrite   BufNum   BufOffset WData [...]         -- Write data to debug buffer."NEWLINE,
/*    */  "hwbuffill    BufNum   BufOffset FData  BufInc  Len  -- Fill data to debug buffer."NEWLINE,
/*    */  "hwbuftx      Port BufNum Len Repeat SameBuf Delay   -- Tx packet from debug buffer to ks884X."NEWLINE,
/*    */  "hwbufrx      BufNum   TimeOut                       -- Rx packet from ks884X and store in debug buffer(start at BufNum)."NEWLINE,
/*    */  "hwtableread  Table Index   Count                    -- Read ks884X table data register."NEWLINE,
/*    */  "hwtablewrite Table Index   RegDataH  RegDataL       -- Write ks884X table data register. "NEWLINE,
/*    */  "hwtableshow  Table Index   Count                    -- Show ks884X table."NEWLINE,
/*    */  "hwclearmib   Port                                   -- Clear software MIB counter database (clear all if Port=4)."NEWLINE,
/*    */  "hwdumptx     DumpFlag                               -- Start/Stop dumpping transmit packet data."NEWLINE,
/*    */  "hwdumprx     DumpFlag                               -- Start/Stop dumpping receive packet data."NEWLINE,
/*    */  "hwreset      TimeOut                                -- Reset the device."NEWLINE,
#ifdef DEF_KS8842
/*    */  "hwrepeat     Flag                                   -- Enable/Disable Repeater mode (1 - Enable, 0 - Disable)."NEWLINE,
#endif /* #ifdef DEF_KS8842 */
#ifdef DEF_KS8841
#ifdef KS_ISA_BUS
/*    */  "hwrxthres    FData                                  -- Set Early Receive Threshold (in unit of 64-byte)."NEWLINE,
/*    */  "hwtxthres    FData                                  -- Set Early Transmit Threshold (in unit of 64-byte)."NEWLINE,
/*    */  "hwearly      Flag                                   -- Enable/Disable Early Transmit/Receive (1 - Enable, 0 - Disable)."NEWLINE,
#endif /* #ifdef KS_ISA_BUS */
#endif /* #ifdef DEF_KS8841 */
/*    */  "hwmagic                                             -- Enable Wake-on-LAN by receiving magic packet."NEWLINE,
/*    */  "hwwolframe   FrameNum  CRC  ByteMask                -- Enable Wake-on-LAN by receiving wake-up frame (FrameNum:0~3, ByteMask:0~63)."NEWLINE,
/*    */  "hwclearpme                                          -- Clear PME_Staus to deassert PMEN pin."NEWLINE,
#ifdef KS_PCI_BUS
/*    */  "hwpciread    BusNum DevNum FuncNum RegNum           -- Read PCI Configuration Space register."NEWLINE,
/*    */  "hwpciwrite   BusNum DevNum FuncNum RegNum RegData   -- Modify PCI Configuration Space register."NEWLINE,
#endif /* #ifdef KS_PCI_BUS */
/*    */  "hwtestcable  Port                                   -- Ethernet cable diagnostics."NEWLINE,
/*    */  "hwgetlink    Port                                   -- Get Link status."NEWLINE,
/*    */  "hwsetlink    Port  Data                             -- Set Link speed."NEWLINE,
/*    */  "hwgetmac                                            -- Get device MAC address."NEWLINE,
/*    */  "hwgetip                                             -- Get device IP address."NEWLINE,
/*    */  "hwgettxcnt1                                         -- Get driver port 1 total transmit packets counters."NEWLINE,
/*    */  "hwgettxcnt2                                         -- Get driver port 2 total transmit packets counters."NEWLINE,
/*    */  "hwgetrxcnt1                                         -- Get driver port 1 total received packets counters."NEWLINE,
/*    */  "hwgetrxcnt2                                         -- Get driver port 2 total received packets counters."NEWLINE,
/*    */  "hwtxarp      Port  Repeat  MAC  IP                  -- Tx ARP request packet to ks884X."NEWLINE,
#ifdef KS_QC_TEST
/*    */  "qctxtest     Port  Repeat                           -- QC Tx fix packets from debug buffers to ks884X."NEWLINE,
/*    */  "qcgetmib     Port  MibIndex                         -- QC Get device port MIB counter."NEWLINE,
/*    */  "qcgetcable   Port                                   -- QC Get device port LinkMD cable status."NEWLINE,
/*    */  "qcecho       Flag                                   -- Enable/Disable echo received character to screen (1-Enable, 0-Disable)."NEWLINE,
#endif
#else  /* vxWorks Shell command */
/*    */  "hwhelp                                              -- Display help message."NEWLINE,
/*    */  "hwread       BankNum, RegNum,   Width               -- Read ks884X register."NEWLINE,
/*    */  "hwwrite      BankNum, RegNum,   RegData,  Width     -- Modify ks884X register."NEWLINE,
/*    */  "hwpoll       BankNum, RegNum,BitMask,BitPat,TimeOut -- Poll register bit if it match bit pattern."NEWLINE,
/*    */  "hwbufread    BufNum,  BufOffset,Len                 -- Display contents of debug buffer."NEWLINE,
/*    */  "hwbufwrite   BufNum,  BufOffset,WData [...]         -- Write data to debug buffer (maximum length is 64)."NEWLINE,
/*    */  "hwbuffill    BufNum,  BufOffset,FData, BufInc, Len  -- Fill data to debug buffer."NEWLINE,
/*    */  "hwbuftx      Port,BufNum,Len,Repeat,SameBuf,Delay   -- Tx packet from debug buffer to ks884X."NEWLINE,
/*    */  "hwbufrx      BufNum,  TimeOut                       -- Rx packet from ks884X and store in debug buffer(start at BufNum)."NEWLINE,
/*    */  "hwtableread  Table,Index,  Count                    -- Read ks884X table data register."NEWLINE,
/*    */  "hwtablewrite Table,Index,  RegDataH, RegDataL       -- Write ks884X table data register. "NEWLINE,
/*    */  "hwtableshow  Table,Index,  Count, 1                 -- Show ks884X table."NEWLINE,
/*    */  "hwclearmib   Port                                   -- Clear software MIB counter database (clear all if Port=4)."NEWLINE,
/*    */  "hwdumptx     DumpFlag                               -- Start/Stop dumpping transmit packet data."NEWLINE,
/*    */  "hwdumprx     DumpFlag                               -- Start/Stop dumpping receive packet data."NEWLINE,
/*    */  "hwloopback   Flag                                   -- Start/Stop loopback received packet data to transmit."NEWLINE,
/*    */  "hwreset      TimeOut                                -- Reset the device."NEWLINE,
#ifdef DEF_KS8842
/*    */  "hwrepeat     Flag                                   -- Enable/Disable Repeater mode (1 - Enable, 0 - Disable)."NEWLINE,
#endif /* #ifdef DEF_KS8842 */
#ifdef DEF_KS8841
#ifdef KS_ISA_BUS
/*    */  "hwrxthres    FData                                  -- Set Early Receive Threshold (in unit of 64-byte)."NEWLINE,
/*    */  "hwtxthres    FData                                  -- Set Early Transmit Threshold (in unit of 64-byte)."NEWLINE,
/*    */  "hwearly      Flag                                   -- Enable/Disable Early Transmit/Receive (1 - Enable, 0 - Disable)."NEWLINE,
#endif /* #ifdef KS_ISA_BUS */
#endif /* #ifdef DEF_KS8841 */
/*    */  "hwmagic                                             -- Enable Wake-on-LAN by receiving magic packet."NEWLINE,
/*    */  "hwwolframe   FrameNum, CRC, ByteMask                -- Enable Wake-on-LAN by receiving wake-up frame (FrameNum:0~3, ByteMask:0~63)."NEWLINE,
/*    */  "hwclearpme                                          -- Clear PME_Staus to deassert PMEN pin."NEWLINE,
#ifdef KS_PCI_BUS
/*    */  "hwpciread    BusNum,DevNum,FuncNum,RegNum           -- Read PCI Configuration Space register."NEWLINE,
/*    */  "hwpciwrite   BusNum,DevNum,FuncNum,RegNum, RegData  -- Modify PCI Configuration Space register."NEWLINE,
#endif /* #ifdef KS_PCI_BUS */
/*    */  "hwtestcable  Port                                   -- Ethernet cable diagnostics."NEWLINE,
/*    */  "hwgetlink    Port                                   -- Get Link status."NEWLINE,
/*    */  "hwsetlink    Port, Data                             -- Set Link speed."NEWLINE,
/*    */  "hwgetmac                                            -- Get device MAC address."NEWLINE,
/*    */  "hwgetip                                             -- Get device IP address."NEWLINE,
/*    */  "hwgettxcnt1                                         -- Get driver port 1 total transmit packets counters."NEWLINE,
/*    */  "hwgettxcnt2                                         -- Get driver port 2 total transmit packets counters."NEWLINE,
/*    */  "hwgetrxcnt1                                         -- Get driver port 1 total received packets counters."NEWLINE,
/*    */  "hwgetrxcnt2                                         -- Get driver port 2 total received packets counters."NEWLINE,
/*    */  "hwtxarp      Port, Repeat, MAC, IP                  -- Tx ARP request packet to ks884X."NEWLINE,
#ifdef KS_QC_TEST
/*    */  "qctxtest     Port, Repeat                           -- QC Tx fix packets from debug buffers to ks884X."NEWLINE,
/*    */  "qcgetmib     Port, MibIndex                         -- QC Get device port MIB counter."NEWLINE,
/*    */  "qcgetcable   Port                                   -- QC Get device port LinkMD cable status."NEWLINE,
/*    */  "qcecho       Flag                                   -- Enable/Disable echo received character to screen (1-Enable, 0-Disable)."NEWLINE,
#endif

#endif /* #ifdef SEPARATE_CLI_ARGV_BYSPACE */
	      ""
};


const char * fdbStatusStr[] =
{
/*  0 */  "valid    ",
/*  1 */  "invalid  ",
/*  2 */  "learned  ",
/*  3 */  "static   ",
	      ""
};

#ifdef M16C_62P

extern PHARDWARE  phw;

extern UINT8  eth_pkt_hdr_rx_buf[];
extern UINT8  eth_pkt_rx_buf[];
extern struct ethernet_frame received_frame;
extern UINT8  fEcho;	        /* TRUE to echo received character to screen, FALSE, otherwise. */

extern char FAR * strtok_p(char FAR *, const char FAR *, char FAR **);
extern UINT8  ksSendFrame ( UINT8 *, UINT16, ULONG );
extern void wol_lcd_off ();

#endif /* #ifdef M16C_62P */

#ifdef _EZ80L92

extern PHARDWARE  phw;

extern SYSCALL send_packet ( EMACFRAME *, int );
extern UINT8  ksSendFrame ( UINT8 *, UINT16 );
extern SYSCALL ksPollReceive ( EMACFRAME * );
extern SYSCALL ksPollStart ( void );
extern SYSCALL ksPollStop ( void );

#endif /* #ifdef _EZ80L92 */

#ifdef DEF_VXWORKS

extern int	 consoleFd;		/* fd of initial console device */
extern char    ksIpAddress[];

#endif /* #ifdef DEF_VXWORKS */


#if defined( _WIN32 )  ||  defined( DEF_LINUX )  ||  defined( KS_ARM )
extern PHARDWARE phw;

#define UINT8  unsigned char
#endif

#if defined( KS_ARM )
#include "util.h"
#endif


#ifdef KS_ISA_BUS
extern void  SwitchReadTableAllword_ISA(PHARDWARE, int, USHORT, PUSHORT, PULONG, PULONG );
extern void  SwitchWriteTableQword_ISA (PHARDWARE, int, USHORT, ULONG,  ULONG );
#else
extern void  SwitchReadTableAllword_PCI(PHARDWARE, int, USHORT, PUSHORT, PULONG, PULONG );
extern void  SwitchWriteTableQword_PCI (PHARDWARE, int, USHORT, ULONG,  ULONG );
#endif



/* externally visible functions. */
int   hwhelp ( void );
int   hwread ( unsigned char, unsigned long, unsigned char );
int   hwwrite ( unsigned char, unsigned long, unsigned long, unsigned char );
int   hwpoll ( unsigned char, unsigned long, unsigned long, unsigned long, unsigned long );
int   hwbufread ( unsigned long, unsigned long, unsigned long );
int   hwbufwrite ( unsigned long, unsigned long, unsigned char FAR * );
int   hwbuffill ( unsigned char, unsigned long, unsigned long, unsigned long, unsigned long );
int   hwbuftx ( unsigned long, unsigned long, unsigned long, unsigned long, BOOLEAN, unsigned long );
int   hwbufrx ( unsigned long, unsigned long );
int   hwtableread ( char FAR *, unsigned long, unsigned long );
int   hwtablewrite ( char FAR *, unsigned long, unsigned long, unsigned long );
int   hwtableshow ( char FAR *, unsigned long, unsigned long, BOOLEAN );
int   hwclearmib ( unsigned long );
int   hwdumptx ( unsigned long );
int   hwdumprx ( unsigned long );
int   hwloopback ( unsigned long );
int   hwrepeat ( unsigned long );
int   hwreset ( unsigned long );
int   hwtestcable ( unsigned long );
int   hwgetlink ( unsigned long );
int   hwsetlink ( unsigned long, unsigned long );
int   hwgetmac ( void );
int   hwgetip ( void );
int   hwgettxcnt1 ( void );
int   hwgettxcnt2 ( void );
int   hwgetrxcnt1 ( void );
int   hwgetrxcnt2 ( void );
int   hwtxarp ( unsigned long, unsigned long, unsigned char FAR *, unsigned char FAR * );
int   hwrxthres ( unsigned long );
int   hwtxthres ( unsigned long );
int   hwearly ( unsigned long );
int   hwmagic ( void );
int   hwwolframe ( unsigned long, unsigned long, unsigned char );
int   hwclearpme ( void );

int   hwpciread ( unsigned long, unsigned long, unsigned long, unsigned long );
int   hwpciwrite( unsigned long, unsigned long, unsigned long, unsigned long, unsigned long );


/* this section are for FAB QC testing only */

#ifdef KS_QC_TEST

int   qctxtest ( unsigned long, unsigned long );
int   qcgetmib ( unsigned long, unsigned long );
int   qcgetcable ( unsigned long );
int   qcecho ( unsigned long );

#endif


void  usagDisplay ( unsigned short );
void  unknownDisplay ( void );

/* local functions. */
static void  DisplayErrRetMsg( unsigned long );
static unsigned char isValidBank ( unsigned long );
static unsigned char isValidRegAddr ( unsigned long );
static unsigned char BufferInit ( void );
static void  formatPortToStr ( unsigned char, unsigned char, char * );
static int   TableNameXlat ( char FAR *, unsigned long * );
static void  strToMac ( unsigned char FAR *, unsigned char FAR * );
static void  strToIp ( unsigned char FAR *, unsigned char FAR * );
static void  macDisplayTitle ( unsigned long );
static void  showMibTable ( PHARDWARE );
static void  showTableEntry (unsigned long,
                             unsigned long,
                             unsigned char,
                             unsigned short,
                             unsigned char *,
                             unsigned char,
                             BOOLEAN,
                             BOOLEAN,
                             unsigned char,
                             unsigned long );
#ifdef DEF_VXWORKS
static void  hwbuftx_vxWorks ( unsigned long, unsigned long, unsigned long, unsigned long, BOOLEAN, unsigned long );
static unsigned long hwbufrx_vxWorks ( unsigned long, long );
#endif /* DEF_VXWORKS */

#ifdef M16C_62P
static void  hwbuftx_M16C ( unsigned long, unsigned long, unsigned long, unsigned long, BOOLEAN, unsigned long );
static unsigned long hwbuftr_M16C ( unsigned long, long );
#endif /* M16C_62P */

#ifdef _EZ80L92
static void  hwbuftx_EZ80L92 ( unsigned long, unsigned long, unsigned long, unsigned long, BOOLEAN, unsigned long );
static unsigned long hwbufrx_EZ80L92 ( unsigned long, long );
#endif /* _EZ80L92 */




/*****************************************************************/

/*****************************************************************
*
* Command: hwhelp
*
* Format: "hwhelp "
*
******************************************************************/
int hwhelp
(
)
{
   int i=0;
   char FAR ** pbMsg = (char FAR **)hwHelpMsgStr;


   while (*pbMsg[i])
      DBG_PRINT( "%s", (char FAR *)pbMsg[i++] );

   DBG_PRINT( NEWLINE );

   return (TRUE);
}

int hwread2
(
   unsigned char   BankNum,
   unsigned long   RegAddr,
   unsigned char   Width
)
{
#ifdef DEF_VXWORKS
   PHARDWARE       phw = gpDrvCtrl[0]->pHW;
#endif



   /*
    * Validate parameters
    */

   if ( !(isValidBank( BankNum )) )
       return FALSE;

   if ( !(isValidRegAddr( RegAddr )) )
       return FALSE;

   if ( phw == NULL )
   {
        DisplayErrRetMsg( 3 ) ;
        return FALSE;
   }

   /*
    * Read from the register and display it's contents
    */

#ifdef KS_ISA_BUS
   switch (Width)
   {
        case 1: /* read by BYTE unit */
            {
            unsigned char  RegData=0;

            HardwareSelectBank( phw, (UCHAR)BankNum );
            HW_READ_BYTE( phw, (UCHAR)RegAddr, &RegData );

            DBG_PRINT( "bank%02d-reg.%02d : %02x"NEWLINE, BankNum, (unsigned char)RegAddr, RegData );
            }
            break;
        case 4: /* read by DWORD unit */
            {
            unsigned long  RegData=0;

            HardwareSelectBank( phw, (UCHAR)BankNum );
            HW_READ_DWORD( phw, (UCHAR)RegAddr, &RegData );

            DBG_PRINT( "bank%02d-reg.%02d : %08lx"NEWLINE, BankNum, (unsigned char)RegAddr, RegData );
            }
            break;
        case 2: /* read by WORD unit */
        default:
            {
            unsigned short  RegData=0;

            HardwareSelectBank( phw, (UCHAR)BankNum );
            HW_READ_WORD( phw, (UCHAR)RegAddr, &RegData );

            DBG_PRINT( "bank%02d-reg.%02d : %04x"NEWLINE, BankNum, (unsigned char)RegAddr, RegData );
            }
            break;
   }


#endif /* #ifdef KS_ISA_BUS */

   DBG_PRINT( NEWLINE);

   return TRUE ;
}



/*****************************************************************
*
* Command: hwread
*
* Format: "hwread  BankNum RegAddr Width"
*
******************************************************************/
int hwread
(
   unsigned char   BankNum,
   unsigned long   RegAddr,
   unsigned char   Width
)
{
#ifdef DEF_VXWORKS
   PHARDWARE       phw = gpDrvCtrl[0]->pHW;
#endif



   /*
    * Validate parameters
    */

   if ( !(isValidBank( BankNum )) )
       return FALSE;

   if ( !(isValidRegAddr( RegAddr )) )
       return FALSE;

   if ( phw == NULL )
   {
        DisplayErrRetMsg( 3 ) ;
        return FALSE;
   }

   /*
    * Read from the register and display it's contents
    */

#ifdef KS_ISA_BUS
   switch (Width)
   {
        case 1: /* read by BYTE unit */
            {
            unsigned char  RegData=0;

            HardwareReadRegByte( phw,
                                 (UCHAR)BankNum,
                                 (UCHAR)RegAddr,
                                 (PUCHAR)&RegData
                               );

            #ifdef KS_QC_TEST
            DBG_PRINT( "^[%02x]$"NEWLINE, RegData );
            #else
            DBG_PRINT( "bank%02d-reg.%02d : %02x"NEWLINE, BankNum, (unsigned char)RegAddr, RegData );
            #endif
            }
            break;
        case 4: /* read by DWORD unit */
            {
            unsigned long  RegData=0;

            HardwareReadRegDWord( phw,
                                 (UCHAR)BankNum,
                                 (UCHAR)RegAddr,
                                 (PULONG)&RegData
                               );

            #ifdef KS_QC_TEST
            DBG_PRINT( "^[%08lx]$"NEWLINE, RegData );
            #else
            DBG_PRINT( "bank%02d-reg.%02d : %08lx"NEWLINE, BankNum, (unsigned char)RegAddr, RegData );
            #endif
            }
            break;
        case 2: /* read by WORD unit */
        default:
            {
            unsigned short  RegData=0;

            HardwareReadRegWord( phw,
                                 (UCHAR)BankNum,
                                 (UCHAR)RegAddr,
                                 (PUSHORT)&RegData
                               );

            #ifdef KS_QC_TEST
            DBG_PRINT( "^[%04x]$"NEWLINE, RegData );
            #else
            DBG_PRINT( "bank%02d-reg.%02d : %04x"NEWLINE, BankNum, (unsigned char)RegAddr, RegData );
            #endif
            }
            break;
   }

#else  /* PCI BUS */


   switch (Width)
   {
        case 1: /* read by BYTE unit */
            {
            unsigned char  RegData=0;

            HW_READ_BYTE( phw,
                           (ULONG)RegAddr,
                           (PUCHAR)&RegData
                         );

            DBG_PRINT( "reg.%08X : %02x "NEWLINE, (int) RegAddr, RegData );
            }
            break;
        case 4: /* read by DWORD unit */
            {
            unsigned long  RegData=0;

            HW_READ_DWORD( phw,
                           (ULONG)RegAddr,
                           (PULONG)&RegData
                         );

            DBG_PRINT( "reg.%08X : %08lx "NEWLINE, (int) RegAddr, RegData );
            }
            break;
        case 2: /* read by WORD unit */
        default:
            {
            unsigned short  RegData=0;

            HW_READ_WORD( phw,
                           (ULONG)RegAddr,
                           (PUSHORT)&RegData
                         );

            DBG_PRINT( "reg.%08X : %04x "NEWLINE, (int) RegAddr, RegData );
            }
            break;
   }

#endif /* #ifdef KS_ISA_BUS */

   DBG_PRINT( NEWLINE);

   return TRUE ;
}


/*****************************************************************
*
* Command: hwwrite
*
* Format: "hwwrite BankNum RegAddr RegData Width"
*
******************************************************************/
int hwwrite
(
   unsigned char  BankNum,
   unsigned long  RegAddr,
   unsigned long  RegData,
   unsigned char  Width
)
{
#ifdef DEF_VXWORKS
   PHARDWARE       phw = gpDrvCtrl[0]->pHW;
#endif


   /*
    * Validate parameters
    */

   if ( !(isValidBank( BankNum )) )
       return FALSE;

   if ( !(isValidRegAddr( RegAddr )) )
       return FALSE;

   if ( phw == NULL )
   {
        DisplayErrRetMsg( 3 ) ;
        return FALSE;
   }

   /*
    * Write to the register
    */

#ifdef KS_ISA_BUS

   switch (Width)
   {
        case 1: /* write by BYTE unit */
            {
            unsigned char  WriteRegData=0;
#if 0
            unsigned char  ReadRegData=0 ;
#endif

            WriteRegData = (unsigned char)RegData;
            HardwareWriteRegByte( phw,
                                  (UCHAR)BankNum,
                                  (UCHAR)RegAddr,
                                  (UCHAR)WriteRegData
                                );

            /*
             * Read it back from the register and display it's contents
             */

            /*
            HardwareReadRegByte( phw,
                                 (UCHAR)BankNum,
                                 (UCHAR)RegAddr,
                                 (PUCHAR)&ReadRegData
                               );

            DBG_PRINT( "bank%02d-reg.%02d : %02x "NEWLINE, BankNum, (unsigned char)RegAddr, ReadRegData );
            */
            }
            break;
        case 4: /* write by DWORD unit */
            {
            unsigned long  WriteRegData=0;
#if 0
            unsigned long  ReadRegData=0 ;
#endif

            WriteRegData = (unsigned long)RegData;

            HardwareWriteRegDWord( phw,
                                  (UCHAR)BankNum,
                                  (UCHAR)RegAddr,
                                  (ULONG)WriteRegData
                                );

            /*
             * Read it back from the register and display it's contents
             */

            /*
            HardwareReadRegDWord( phw,
                                 (UCHAR)BankNum,
                                 (UCHAR)RegAddr,
                                 (PULONG)&ReadRegData
                               );

            DBG_PRINT( "bank%02d-reg.%02d : %08lx "NEWLINE, BankNum, (unsigned char)RegAddr, ReadRegData );
            */

            }
            break;
        case 2: /* write by WORD unit */
        default:
            {
            unsigned short  WriteRegData=0;
#if 0
            unsigned short  ReadRegData=0 ;
#endif

            WriteRegData = (unsigned short)RegData;
            HardwareWriteRegWord( phw,
                                  (UCHAR)BankNum,
                                  (UCHAR)RegAddr,
                                  (USHORT)WriteRegData
                                );

            /*
             * Read it back from the register and display it's contents
             */

            /*
            HardwareReadRegWord( phw,
                                 (UCHAR)BankNum,
                                 (UCHAR)RegAddr,
                                 (PUSHORT)&ReadRegData
                               );

            DBG_PRINT( "bank%02d-reg.%02d : %04x "NEWLINE, BankNum, (unsigned char)RegAddr, ReadRegData );
            */
            }
            break;
   }

#else  /* PCI BUS */

   switch (Width)
   {
        case 1: /* write by BYTE unit */
            {
            unsigned char  WriteRegData=0;
#if 0
            unsigned char  ReadRegData=0 ;
#endif

            WriteRegData = (unsigned char)RegData;
            HW_WRITE_BYTE( phw,
                           (ULONG)RegAddr,
                           (UCHAR)WriteRegData
                         );

            /*
            HW_READ_BYTE( phw,
                          (ULONG)RegAddr,
                          (PUCHAR)&ReadRegData
                        );

            DBG_PRINT( "reg.%08X : %02x "NEWLINE, RegAddr,  ReadRegData  );
            */
            }
            break;
        case 4: /* write by DWORD unit */
            {
            unsigned long  WriteRegData=0;
#if 0
            unsigned long  ReadRegData=0 ;
#endif

            WriteRegData = (unsigned long)RegData;
            HW_WRITE_DWORD( phw,
                            (ULONG)RegAddr,
                            (ULONG)WriteRegData
                          );

            /*
            HW_READ_DWORD( phw,
                           (ULONG)RegAddr,
                           (PULONG)&ReadRegData
                         );

            DBG_PRINT( "reg.%08X : %08lx "NEWLINE, RegAddr, ReadRegData );
            */
            }
            break;
        case 2: /* write by WORD unit */
        default:
            {
            unsigned short  WriteRegData=0;
#if 0
            unsigned short  ReadRegData=0 ;
#endif

            WriteRegData = (unsigned short)RegData;
            HW_WRITE_WORD( phw,
                           (ULONG)RegAddr,
                           (USHORT)WriteRegData
                         );

            /*
            HW_READ_WORD( phw,
                          (ULONG)RegAddr,
                          (PUSHORT)&ReadRegData
                        );

            DBG_PRINT( "reg.%08X : %04x "NEWLINE, RegAddr,  ReadRegData  );
            */
            }
            break;
   }

#endif /* #ifdef KS_ISA_BUS */

   #ifdef KS_QC_TEST
   DBG_PRINT( "^[ok]$" );
   #endif

   DBG_PRINT( NEWLINE);

   return TRUE ;
}

/*****************************************************************
*
* Command: hwpoll
*
* Format: "hwpoll BankNum RegAddr BitMask BitPattern TimeOut"
*
******************************************************************/
int hwpoll
(
   unsigned char   BankNum,
   unsigned long   RegAddr,
   unsigned long   BitMask,
   unsigned long   BitPattern,
   unsigned long   TimeOut
)
{
   unsigned long   RegData=0;
#ifdef DEF_VXWORKS
   PHARDWARE       phw = gpDrvCtrl[0]->pHW;
#endif



   /*
    * Validate parameters
    */

   if ( !(isValidBank( BankNum )) )
       return FALSE;

   if ( !(isValidRegAddr( RegAddr )) )
       return FALSE;

   if ( phw == NULL )
   {
        DisplayErrRetMsg( 3 ) ;
        return FALSE;
   }


   /*
    * Poll the register
    */

   while ( TimeOut-- )
   {
       /* Check if bit pattern exists */

#ifdef KS_ISA_BUS
       HardwareReadRegWord( phw,
                            (UCHAR)BankNum,
                            (UCHAR)RegAddr,
                            (PUSHORT)&RegData
                          );
#else  /* PCI BUS */

       if ( RegAddr < MAX_DWORD_OFFSET )
       {
           HW_READ_DWORD( phw,
                         (ULONG)RegAddr,
                         (PULONG)&RegData
                        );
       }
       else
       {
           HW_READ_WORD( phw,
                        (ULONG)RegAddr,
                        (PUSHORT)&RegData
                       );
       }
#endif

       RegData &= BitMask;

       if (RegData == BitPattern)
          break ;

       if (!TimeOut)
           /* Fail if we're out of time */
           DisplayErrRetMsg( 4 ) ;
       else
          /* Delay 1 microsecond of time to check */
          DelayMillisec( 1 ) ;
   }


   DBG_PRINT( NEWLINE);
   return TRUE ;
}

/*****************************************************************
*
* Command: hwbufread
*
* Format: "hwbufread BufNum BufOffset Length"
*
******************************************************************/
int hwbufread
(
   unsigned long  BufNum,
   unsigned long  BufOffset,
   unsigned long  CountTotal
)
{
   #define ITEMS_PER_ROW        16

   unsigned long  CountRowItems ;



   if (BufferInit() != TRUE)
   {
      return FALSE ;
   }

   /*
    * Validate parameters
    */

   if (BufNum >= BUFFER_COUNT)
   {
      DisplayErrRetMsg( 1 ) ;
      return FALSE ;
   }

   if (BufOffset >= BUFFER_LENGTH)
   {
      DisplayErrRetMsg( 1 ) ;
      return FALSE ;
   }

   if ( (BufOffset+CountTotal) > BUFFER_LENGTH)
   {
      DisplayErrRetMsg( 1 ) ;
      return FALSE ;
   }


   CountRowItems = ITEMS_PER_ROW ;

   /*
    * Read and display Count number of data bytes
    */

   while (CountTotal--)
   {
         /* Exit if reference is outside of buffer */
         if (BufOffset >= BUFFER_LENGTH)
         {
            break ;
         }

         /* Check if time to display new row of data bytes */
         if (CountRowItems == ITEMS_PER_ROW)
         {
            DBG_PRINT( NEWLINE"%08lx ", BufOffset ) ;
            CountRowItems = 0 ;
         }

         /* Display delimiter between data bytes */
         if (CountRowItems == (ITEMS_PER_ROW / 2))
         {
            DBG_PRINT( " - " ) ;
         }
         else
         {
            DBG_PRINT( " " ) ;
         }

         /* Display current data byte */
         DBG_PRINT( "%02x", apbBuffer[ BufNum ][ BufOffset ] ) ;

         /* Reference next data byte */
         BufOffset++ ;
         CountRowItems++ ;
   }

   DBG_PRINT( NEWLINE);
   DBG_PRINT( NEWLINE);
   return TRUE ;
}


/*****************************************************************
*
* Command: hwbufwrite
*
* Format: "hwbufwrite BufNum BufOffset BufData [...]"
*
******************************************************************/
int hwbufwrite
(
   unsigned long  BufNum,
   unsigned long  BufOffset,
   unsigned char FAR *BufData
)
{
#if !defined( _WIN32 )  &&  !defined( DEF_LINUX )  &&  !defined( KS_ARM )
   char FAR * pHolder = NULL;
   char FAR * tok;


   if (BufferInit() != TRUE)
   {
      return FALSE ;
   }

   /*
    * Validate parameters
    */

   if (BufNum >= BUFFER_COUNT)
   {
      DisplayErrRetMsg( 1 ) ;
      return FALSE ;
   }

   if (BufOffset >= BUFFER_LENGTH)
   {
      DisplayErrRetMsg( 1 ) ;
      return FALSE ;
   }

   /*
    * Write data bytes to buffer
    */

#if defined (M16C_62P) || defined (_EZ80L92)
   tok = (char FAR * )strtok_p ( (char FAR *)BufData, (const char FAR *)SPACE, (char FAR **)&pHolder );
#else
   tok = strtok_r ( BufData, SPACE, &pHolder );
#endif
   apbBuffer[ BufNum ][ BufOffset++ ] = (unsigned char FAR) strtol ( tok, NULL, 16 ) ;
   while (pHolder != NULL)
   {
         /* Exit if reference is outside of buffer */
         if (BufOffset >= BUFFER_LENGTH)
            break ;

         /* Write current data byte */
#if defined (M16C_62P) || defined (_EZ80L92)
         tok = strtok_p( pHolder, (const char FAR *)SPACE, &pHolder );
#else
         tok = strtok_r ( NULL, SPACE, &pHolder );
#endif
         apbBuffer[ BufNum ][ BufOffset++ ] = (unsigned char FAR) strtol ( tok, NULL, 16 ) ;
   }

#ifndef KS_QC_TEST
   DBG_PRINT( NEWLINE);
#endif

#endif
   return TRUE ;
}


/*****************************************************************
*
* Command: hwbuffill
*
* Format: "hwbuffill BufNum BufOffset BufData BufInc Length"
*
******************************************************************/
int hwbuffill
(
   unsigned char  BufNum,
   unsigned long  BufOffset,
   unsigned long  BufData,
   unsigned long  BufInc,
   unsigned long  Count
)
{


   if (BufferInit() != TRUE)
   {
      return FALSE ;
   }

   /*
    * Validate parameters
    */

   if (BufNum >= BUFFER_COUNT)
   {
      DisplayErrRetMsg( 1 ) ;
      return FALSE ;
   }

   if (BufOffset >= BUFFER_LENGTH)
   {
      DisplayErrRetMsg( 1 ) ;
      return FALSE ;
   }

   if ( (BufOffset+Count) >= BUFFER_LENGTH)
   {
      DisplayErrRetMsg( 1 ) ;
      return FALSE ;
   }


   /*
    * Write data byte to buffer
    */

   while (Count--)
   {
         /*
          * Exit if reference is outside of buffer
          */

         if (BufOffset >= BUFFER_LENGTH)
         {
            break ;
         }

         apbBuffer[ BufNum ][ BufOffset++ ] = (unsigned char FAR) BufData ;

         BufData += BufInc ;
   }

#ifndef KS_QC_TEST
   DBG_PRINT( NEWLINE);
#endif

   return TRUE ;
}


#ifdef DEF_VXWORKS
/*****************************************************************************
 *
 * hwbuftx_vxWorks - Tx packet to ks884X on Renesas SH7751R vxWorks platform.
 * hwbufrx_vxWorks - Rx packet from ks884X on Renesas SH7751R vxWorks platform.
 *
******************************************************************************/
static void hwbuftx_vxWorks
(
   unsigned long   Port,
   unsigned long   BufNum,
   unsigned long   BufLen,
   unsigned long   RepeatCount,
   BOOLEAN         SameBuf,
   unsigned long   ms_delay
)
{
   PHARDWARE       phw = gpDrvCtrl[0]->pHW;
   M_BLK_ID 	   pMblk= NULL;
   UCHAR	     * pCluster = NULL;
   DRV_CTRL      * pDrvCtrl;	/* device ptr */



   pDrvCtrl = gpDrvCtrl[0];

   if ( (phw == NULL) || ( pDrvCtrl == NULL) )
   {
        DisplayErrRetMsg( 3 ) ;
        return;
   }

   /* Set "fPollSendFromCLI" to indicate that ks884xEndPollSend from CLI command */
   pDrvCtrl->fPollSendFromCLI = TRUE;

   /* Set trasnmit destination port */
   phw->m_bPortTX = Port;

   /*
    * Transmit the buffer data
    */

   /* Grab a Mblock\ClBlk\Cluster block */
#ifdef KS_PCI_BUS
   if ( (pMblk = (M_BLK_ID)ks884xAllocMblk(pDrvCtrl, &pCluster, NULL) ) == NULL )
#else
   if ( (pMblk = (M_BLK_ID)ks884xSHAllocMblk(pDrvCtrl, &pCluster, NULL) ) == NULL )
#endif
   {
       DisplayErrRetMsg( 3 ) ;
       return ;
   }

   /* Point data buffer to the MBlock\ClBlk\Cluster structure */
   pMblk->mBlkHdr.mData = (char *)apbBuffer[BufNum];
   pMblk->mBlkHdr.mFlags |= M_PKTHDR;
   pMblk->mBlkHdr.mLen = BufLen;
   pMblk->mBlkHdr.mNext = NULL;

   while ( RepeatCount > 0 )
   {


#ifdef KS_PCI_BUS
       if ( ks884xEndSend ( pDrvCtrl, pMblk ) == 0 )
#else
       if ( ks884xSHEndSend ( pDrvCtrl, pMblk ) == 0 )
#endif
       {
           RepeatCount--;
       }

       /* Find next buffer to transmit */
       if ( !SameBuf )
       {
           if (++BufNum >= BUFFER_COUNT)
              BufNum = 0 ;
       }
       /* else, use same buffer to repeat transmit */

       /* Delay in ms if requested */
       if ( ms_delay > 0 )
       {
          taskDelay( ms_delay ) ;
       }

   } /*    while ( RepeatCount > 0 ) */

   /* Done, free Mblk\ClBlk */
   netMblkClChainFree (pMblk);

   /* Clear "fPollSendFromCLI" */
   pDrvCtrl->fPollSendFromCLI = FALSE;

}


static unsigned long hwbufrx_vxWorks
(
   unsigned long   BufNum,
   long            timeOutCount
)
{
   PHARDWARE       phw = gpDrvCtrl[0]->pHW;
   M_BLK_ID 	   pMblk;
   UCHAR	     * pCluster = NULL;
   DRV_CTRL      * pDrvCtrl;	/* device ptr */
   int             len;
   unsigned long   totalRxPacket=0;


   pDrvCtrl = gpDrvCtrl[0];

   if ( (phw == NULL) || ( pDrvCtrl == NULL) )
   {
        DisplayErrRetMsg( 3 ) ;
        return (totalRxPacket);
   }


   /* Grab a Mblock\ClBlk\Cluster block */
#ifdef KS_PCI_BUS
   if ( (pMblk = (M_BLK_ID)ks884xAllocMblk(pDrvCtrl, &pCluster, NULL) ) == NULL )
#else
   if ( (pMblk = (M_BLK_ID)ks884xSHAllocMblk(pDrvCtrl, &pCluster, NULL) ) == NULL )
#endif
   {
        DisplayErrRetMsg( 5 ) ;
        return (totalRxPacket);
   }

   /* pMblk->mBlkHdr.mData = (char *)apbBuffer[BufNum]; */
   DBG_PRINT ("pMblk->mBlkHdr.mData=0x%08x\n",pMblk->mBlkHdr.mData);

   /* start vxWorks END driver polled mode operations */
#ifdef KS_PCI_BUS
   ks884xEndPollStart ( pDrvCtrl );
#else
   ks884xSHEndPollStart ( pDrvCtrl );
#endif

   while ( ( timeOutCount-- ) && ( BufNum < BUFFER_COUNT ) )
   {
        /* Copy received data from hardware packet memory to 'apbBuffer' */
#ifdef KS_PCI_BUS
        if ( ks884xEndPollReceive( pDrvCtrl, pMblk ) == 0 )
#else
        if ( ks884xSHEndPollReceive( pDrvCtrl, pMblk ) == TRUE )
#endif
        {
            /* Copy receive data to buffer */
            len  = netMblkToBufCopy (pMblk, (char *)apbBuffer[BufNum], NULL);
            /* len  = pMblk->mBlkHdr.mLen; */
            PrintPacketData ( (char *)&apbBuffer[BufNum][0], len, phw->m_bPortRX, 2);

           /* update counter */
           BufNum++;
           totalRxPacket++;
        }
        else
        {
           if (!timeOutCount)
               /* Fail if we're out of time */
               DisplayErrRetMsg( 4 ) ;
           else
               /* Delay in ms of time to check */
               DelayMillisec( DELAY_IN_nMS_TO_CHECK ) ;
        }
   } /* while ( ( timeOutCount-- ) && ( BufNum < BUFFER_COUNT ) ) */

   /* Done poll receive packet, free Mblk\ClBlk */
   netMblkClChainFree (pMblk);

   /* stop vxWorks END driver polled mode operations */
#ifdef KS_PCI_BUS
   ks884xEndPollStop ( pDrvCtrl );
#else
   ks884xSHEndPollStop ( pDrvCtrl );
#endif

   return (totalRxPacket);
}

#endif /* DEF_VXWORKS */

#ifdef M16C_62P
/*****************************************************************************
 *
 * hwbuftx_M16C - Tx packet to ks884X on Renesas M16C_62P platform.
 * hwbufrx_M16C - Rx packet from ks884X on Renesas M16C_62P platform .
 *
******************************************************************************/
static void hwbuftx_M16C
(
   unsigned long   Port,
   unsigned long   BufNum,
   unsigned long   BufLen,
   unsigned long   RepeatCount,
   BOOLEAN         SameBuf,
   unsigned long   ms_delay
)
{
   int              i ;


   if ( phw == NULL )
   {
        DisplayErrRetMsg( 3 ) ;
        return;
   }

   while (RepeatCount > 0)
   {
       /* Send frame to device */
#ifdef EARLY_TRANSMIT
       ksSendFrame ( &apbBuffer[BufNum][0], BufLen, Port );
#else
       if ( ksSendFrame ( (UINT8 * )&apbBuffer[BufNum][0], (UINT16)BufLen, (ULONG)Port ) )
#endif
            RepeatCount--;

       /* Find next buffer to transmit */
       if ( !SameBuf )
       {
           if (++BufNum >= BUFFER_COUNT)
              BufNum = 0 ;
       }
       /* else, use same buffer to repeat transmit */

       /* Delay in ms if requested */
       if ( ms_delay > 0 )
          DelayMillisec( ms_delay ) ;

   } /* while (RepeatCount) */

}

static unsigned long hwbufrx_M16C
(
   unsigned long   BufNum,
   long            timeOutCount
)
{
   unsigned short  InterruptMask;
   unsigned long   totalRxPacket=0;


   if ( phw == NULL )
   {
        DisplayErrRetMsg( 3 ) ;
        return (totalRxPacket);
   }

   /* Save the current interrupt mask and block all interrupts. */
ACT_DEBUG_
   InterruptMask = HardwareBlockInterrupt( phw );

   while ( ( timeOutCount-- ) && ( BufNum < BUFFER_COUNT ) )
   {
       /* Copy received data from hardware packet memory to 'apbBuffer' */
       if ( NETWORK_CHECK_IF_RECEIVED() == TRUE )
       {
           /* Copy receive data to buffer */
           memcpy ( (char FAR *)&apbBuffer[BufNum][0], (char FAR *)&eth_pkt_hdr_rx_buf[0], ETH_HEADER_LEN );
           memcpy ( (char FAR *)&apbBuffer[BufNum][ETH_HEADER_LEN], (char FAR *)&eth_pkt_rx_buf[0],
                    (received_frame.frame_size - ETH_HEADER_LEN));

           /* dump received packet */
           PrintPacketData ( (UCHAR *)&apbBuffer[BufNum][0], (int)received_frame.frame_size,
                             (ULONG)phw->m_bPortRX, (ULONG)2);

           /* update counter */
           BufNum++;
           totalRxPacket++;

			/* discard received frame */
    		NETWORK_RECEIVE_END();
       }
       else
       {
           if (!timeOutCount)
               /* Fail if we're out of time */
               DisplayErrRetMsg( 4 ) ;
           else
               /* Delay in ms of time to check */
               DelayMillisec( DELAY_IN_nMS_TO_CHECK ) ;
       }
   } /* while ( ( timeOutCount-- ) && ( BufNum < BUFFER_COUNT ) ) */

   /* Restore the interrupt mask. */
   HardwareSetInterrupt( phw, InterruptMask );

   return (totalRxPacket);
}

#endif /* M16C_62P */

#ifdef _EZ80L92
/*****************************************************************************
 *
 * hwbuftx_EZ80L92 - Tx packet to ks884X on ZiLog eZ80L92 platform.
 * hwbuftx_EZ80L92 - Rx packet from ks884X on ZiLog eZ80L92 platform .
 *
******************************************************************************/
static void hwbuftx_EZ80L92
(
   unsigned long   Port,
   unsigned long   BufNum,
   unsigned long   BufLen,
   unsigned long   RepeatCount,
   BOOLEAN         SameBuf,
   unsigned long   ms_delay
)
{
   EMACFRAME     * xmit_packet;


   if ( phw == NULL )
   {
        DisplayErrRetMsg( 3 ) ;
        return;
   }

#if (0)
   /* Allocate a packet buffer  */
   xmit_packet = (EMACFRAME *)getmem ( BufLen+4 );
   if (xmit_packet == (EMACFRAME *) SYSERR)
   {
        DisplayErrRetMsg( 5 ) ;
        return ;
   }
#endif

   /*
    * Transmit the buffer data
    */

   while (RepeatCount > 0)
   {

#if (0)
       /* Copy data buffer to the EMACFRAME structure */
       memcpy ((char *)xmit_packet->DstAddr, (char *)apbBuffer[BufNum], BufLen);
       xmit_packet->Length = BufLen;
	   if ( send_packet( (EMACFRAME *) xmit_packet, Port ) == OK )
#endif
       if ( ksSendFrame ( &apbBuffer[BufNum][0], BufLen ) )
       {
            RepeatCount--;
       }

       /* Find next buffer to transmit */
       if ( !SameBuf )
       {
           if (++BufNum >= BUFFER_COUNT)
              BufNum = 0 ;
       }
       /* else, use same buffer to repeat transmit */

       /* Delay in ms if requested */
       if ( ms_delay > 0 )
          DelayMillisec( ms_delay ) ;

   } /* while (RepeatCount) */

   /* Done send packet, free packet buffer */
   freemem ( xmit_packet, (BufLen+4) );

}

static unsigned long hwbufrx_EZ80L92
(
   unsigned long   BufNum,
   long            timeOutCount
)
{
   unsigned long   totalRxPacket=0;
   EMACFRAME     * databuff=NULL;


   if ( phw == NULL )
   {
        DisplayErrRetMsg( 3 ) ;
        return (totalRxPacket);
   }

   /* Allocate a packet buffer  */
   databuff = (EMACFRAME *)getmem ( MAXIMUM_ETHERNET_PACKET_SIZE+4 );
   if (databuff == (EMACFRAME *) SYSERR)
   {
        DisplayErrRetMsg( 5 ) ;
        return (totalRxPacket);
   }


   /* start driver polled mode operations */
   ksPollStart ();

   while ( ( timeOutCount-- ) && ( BufNum < BUFFER_COUNT ) )
   {

        /* Copy received data from hardware packet memory to 'apbBuffer' */
        if ( ksPollReceive( databuff ) == TRUE )
        {
           /* Copy receive data to buffer */
           memcpy ( (char FAR *)&apbBuffer[BufNum][0], (char *)databuff->DstAddr, databuff->Length );
           PrintPacketData ( (unsigned char *)&apbBuffer[BufNum][0], (int)databuff->Length, phw->m_bPortRX, 2);

           /* update counter */
           BufNum++;
           totalRxPacket++;
        }
        else
        {
           if (!timeOutCount)
               /* Fail if we're out of time */
               DisplayErrRetMsg( 4 ) ;
           else
               /* Delay in ms of time to check */
               DelayMillisec( DELAY_IN_nMS_TO_CHECK ) ;
        }
   } /* while ( ( timeOutCount-- ) && ( BufNum < BUFFER_COUNT ) ) */

   /* Done send packet, free packet buffer */
   freemem ( databuff, (MAXIMUM_ETHERNET_PACKET_SIZE+4) );

   /* stop driver polled mode operations */
   ksPollStop ();

   return (totalRxPacket);
}

#endif /* _EZ80L92 */


/*****************************************************************
*
* Command: hwbuftx
*
* Format: "hwbuftx Port BufNum BufLen RepeatCount Delay"
*
******************************************************************/
int hwbuftx
(
   unsigned long   Port,
   unsigned long   BufNum,
   unsigned long   BufLen,
   unsigned long   RepeatCount,
   BOOLEAN         SameBuf,
   unsigned long   ms_delay
)
{


   if (BufferInit() != TRUE)
   {
      return FALSE ;
   }


   /*
    * Validate parameters
    */

#ifdef DEF_KS8842
   if ( Port > TOTAL_PORT_NUM )
#else
   if ( Port > 0 )
#endif
   {
      DisplayErrRetMsg( 1 ) ;
      return FALSE ;
   }

   if (BufNum >= BUFFER_COUNT)
   {
      DisplayErrRetMsg( 1 ) ;
      return FALSE ;
   }

   if (BufLen >= BUFFER_LENGTH)
   {
      DisplayErrRetMsg( 1 ) ;
      return FALSE ;
   }

   /*
    * Transmit the buffer data
    */

#ifdef DEF_VXWORKS
   hwbuftx_vxWorks ( Port, BufNum, BufLen, RepeatCount, SameBuf, ms_delay );
#endif /* DEF_VXWORKS */

#ifdef M16C_62P
   hwbuftx_M16C ( Port, BufNum, BufLen, RepeatCount, SameBuf, ms_delay );
#endif /* M16C_62P */

#ifdef _EZ80L92
   hwbuftx_EZ80L92 ( Port, BufNum, BufLen, RepeatCount, SameBuf, ms_delay );
#endif /* _EZ80L92 */

   DBG_PRINT( NEWLINE);

   return TRUE ;
}

/*****************************************************************
*
* Command: hwbufrx
*
* Format: "hwbufrx  Port BufNum"
*
******************************************************************/
int hwbufrx
(
   unsigned long   BufNum,
   unsigned long   TimeOut
)
{
   unsigned long   totalRxPacket=0;
   long            timeOutCount;




   if (BufferInit() != TRUE)
   {
      return FALSE ;
   }

   /*
    * Validate parameters
    */

   if (BufNum >= BUFFER_COUNT)
   {
      DisplayErrRetMsg( 1 ) ;
      return FALSE ;
   }

   timeOutCount = TimeOut /  DELAY_IN_nMS_TO_CHECK ; /* delay counter in the ms unit */
   if  (timeOutCount <= 0)
   {
      DisplayErrRetMsg( 1 ) ;
      return FALSE ;
   }

   /*
    * Receive the buffer data
    * Checking for newly-arrived Ehernet packets periodically by DELAY_IN_nMS_TO_CHECK ms
    * until Time out or received buffers are full ( BufNum < BUFFER_COUNT ).
    */


#ifdef DEF_VXWORKS
   totalRxPacket = hwbufrx_vxWorks ( BufNum, timeOutCount );
#endif /* DEF_VXWORKS */

#ifdef M16C_62P
   totalRxPacket = hwbufrx_M16C ( BufNum, timeOutCount );
#endif /* M16C_62P */

#ifdef _EZ80L92
   totalRxPacket = hwbufrx_EZ80L92 ( BufNum, timeOutCount );
#endif /* _EZ80L92 */

   DBG_PRINT( "Received %d packets."NEWLINE, ( int ) totalRxPacket);
   DBG_PRINT( NEWLINE);

   return TRUE ;
}


/*****************************************************************
*
* Command: hwtableread
*
* Format: "hwtableread  Table Index Count"
*
******************************************************************/
int hwtableread
(
   char FAR      *pbTableName,
   unsigned long Index,
   unsigned long Count
)
{
#ifdef DEF_VXWORKS
   PHARDWARE       phw = gpDrvCtrl[0]->pHW;
#endif
   unsigned short wRegDataH ;
   unsigned long  dwRegDataH ;
   unsigned long  dwRegDataL ;
   unsigned long  dwTableID ;



   /*
    * Validate parameters
    */

   if ( phw == NULL )
   {
        DisplayErrRetMsg( 3 ) ;
        return FALSE;
   }

   if ( !(TableNameXlat( pbTableName, &dwTableID )) )
   {
      DisplayErrRetMsg( 1 ) ;
      return FALSE ;
   }

   switch ( dwTableID )
   {
        case TABLE_STATIC_MAC:
            if ( (Index + Count) > STATIC_MAC_TABLE_ENTRIES )
            {
                DisplayErrRetMsg( 1 ) ;
                return FALSE ;
            }
            break;

        case TABLE_VLAN:
            if ( (Index + Count) > VLAN_TABLE_ENTRIES )
            {
                DisplayErrRetMsg( 1 ) ;
                return FALSE ;
            }
            break;
        case TABLE_DYNAMIC_MAC:
            if ( (Index + Count) > LEARNED_MAC_TABLE_ENTRIES )  /* 1K entry */
            {
                DisplayErrRetMsg( 1 ) ;
                return FALSE ;
            }
            break;
        case TABLE_MIB:
            if ( (Index + Count) > MIB_TABLE_ENTRIES )
            {
                DisplayErrRetMsg( 1 ) ;
                return FALSE ;
            }
            break;
   }

   /*
    * Read and display Count number of table entries
    */

   DBG_PRINT("      DataH DataM    DataL"NEWLINE);
   while (Count--)
   {
#ifdef KS_ISA_BUS
         SwitchReadTableAllword_ISA( phw, dwTableID, ( USHORT ) Index, &wRegDataH, &dwRegDataH, &dwRegDataL );
#else
         SwitchReadTableAllword_PCI( phw, dwTableID, ( USHORT ) Index, &wRegDataH, &dwRegDataH, &dwRegDataL );
#endif

         DBG_PRINT( "%04lx  ", Index ) ;
         DBG_PRINT( " %04x %08lx %08lx" NEWLINE, wRegDataH, dwRegDataH, dwRegDataL );
         Index++ ;
   }

   DBG_PRINT( NEWLINE);

   return TRUE ;
}


/*****************************************************************
*
* Command: hwtablewrite
*
* Format: "hwtablewrite  Table Index RegDataH RegDataL"
*
******************************************************************/
int hwtablewrite
(
   char FAR     *pbTableName,
   unsigned long Index,
   unsigned long RegDataH,
   unsigned long RegDataL
)
{
#ifdef DEF_VXWORKS
   PHARDWARE       phw = gpDrvCtrl[0]->pHW;
#endif
   unsigned long dwTableID ;



   /*
    * Validate parameters
    */

   if ( phw == NULL )
   {
        DisplayErrRetMsg( 3 ) ;
        return FALSE;
   }

   if ( !(TableNameXlat( pbTableName, &dwTableID )) )
   {
      DisplayErrRetMsg( 1 ) ;
      return FALSE ;
   }

   switch ( dwTableID )
   {
        case TABLE_STATIC_MAC:
            if ( Index >= STATIC_MAC_TABLE_ENTRIES )
            {
                DisplayErrRetMsg( 1 ) ;
                return FALSE ;
            }
            break;

        case TABLE_VLAN:
            if ( Index >= VLAN_TABLE_ENTRIES )
            {
                DisplayErrRetMsg( 1 ) ;
                return FALSE ;
            }
            break;

        case TABLE_DYNAMIC_MAC:
        case TABLE_MIB:
            DisplayErrRetMsg( 10 ) ;
            return FALSE ;
   }

   /*
    * Write to table entry
    */

#ifdef KS_ISA_BUS
   SwitchWriteTableQword_ISA( phw, dwTableID, ( USHORT ) Index, RegDataH, RegDataL );
#else
   SwitchWriteTableQword_PCI( phw, dwTableID, ( USHORT ) Index, RegDataH, RegDataL );
#endif

   DBG_PRINT( NEWLINE);

   return TRUE ;
}

/*****************************************************************
*
* Command: hwtableshow
*
* Format: "hwtableshow  Table Index Count "
*
******************************************************************/
int hwtableshow
(
   char FAR      *pbTableName,
   unsigned long  Index,
   unsigned long  Count,
   BOOLEAN        fDidplay
)
{
#ifdef DEF_VXWORKS
   PHARDWARE       phw = gpDrvCtrl[0]->pHW;
#endif
   unsigned long  dwTableID ;
   unsigned char  mac[6] ;
   unsigned char  bPorts=0 ;
   unsigned char  bFid=0 ;
   unsigned short wVid=0 ;
   unsigned char  bTimestamp=0;
   unsigned short wEntries;
   BOOLEAN        fOverride=FALSE;
   BOOLEAN        fUseFID=FALSE;
#if 0
   unsigned char  bInvalid;
#endif
   unsigned long  status = 1;
   BOOLEAN        fEntryReady=TRUE;


ACT_DEBUG_
   /*
    * Validate parameters
    */

   if ( phw == NULL )
   {
        DisplayErrRetMsg( 3 ) ;
        return FALSE;
   }

   if ( !(TableNameXlat( pbTableName, &dwTableID )) )
   {
      DisplayErrRetMsg( 1 ) ;
      return FALSE ;
   }

   switch ( dwTableID )
   {
        case TABLE_STATIC_MAC:
            if ( (Index + Count) > STATIC_MAC_TABLE_ENTRIES )
            {
                DisplayErrRetMsg( 1 ) ;
                return FALSE ;
            }
            break;

        case TABLE_DYNAMIC_MAC:
            if ( (Index + Count) > LEARNED_MAC_TABLE_ENTRIES )  /* 1K entry */
            {
                DisplayErrRetMsg( 1 ) ;
                return FALSE ;
            }
            break;

        case TABLE_VLAN:
            if ( (Index + Count) > VLAN_TABLE_ENTRIES )
            {
                DisplayErrRetMsg( 1 ) ;
                return FALSE ;
            }
            break;

        case TABLE_MIB:
            Count = 1;
            break;
   }

   /*
    * Read and display Count number of table entries
    */

   if ( fDidplay )
   {
       DBG_PRINT( NEWLINE );
       macDisplayTitle( dwTableID );
   }

   while (Count--)
   {
        memset ( mac, 0, 6);

        switch ( dwTableID )
        {
            case TABLE_STATIC_MAC:
            /* read static mac table from ks884x */
            status = 3;
            if ( !(SwitchReadStaticMacTable( phw, ( USHORT ) Index, mac, &bPorts, &fOverride, &fUseFID, &bFid) ) )
                 status = 1;
                 /*status = 3;*/
            break;

            case TABLE_DYNAMIC_MAC:
            /* read dynamic mac table from ks884x */
            status = 2;
            fEntryReady = SwitchReadDynMacTable ( phw, ( USHORT ) Index, mac, &bFid, &bPorts, &bTimestamp, &wEntries );

            if (bPorts == 0)
                bPorts += 1;
            else
                bPorts <<= 1;

            if (Index < wEntries )
                status = 2;       /* within the valid entry, this entry is valid */
            else
                status = 1;       /* beyond the valid entry, this entry is invalid */
            break;

            case TABLE_VLAN:
            status = 0;
            if ( !(SwitchReadVlanTable( phw, ( USHORT ) Index, &wVid, &bFid, &bPorts) ) )
                 status = 1;
            break;

            case TABLE_MIB:
            status = 0;
            PortReadCounters( phw, 0 );
#ifdef DEF_KS8842
            PortReadCounters( phw, 1 );
#endif
            PortReadCounters( phw, 2 );
            break;
        }

        /* display table  */

        if ( fEntryReady == FALSE)
        {
            Index++ ;
            continue;
        }

        if ( fDidplay )
        {
            if ( dwTableID == TABLE_MIB)
                showMibTable( phw );
            else
                showTableEntry( dwTableID, Index, bFid, wVid, mac, bPorts, fOverride, fUseFID, bTimestamp, status );
        }
        Index++ ;
   }

   DBG_PRINT( NEWLINE);

   return TRUE ;
}


/*****************************************************************
*
* Command: hwclearmib
*
* Format: "hwclearmib Port"
*
******************************************************************/
int hwclearmib
(
   unsigned long   Port
)
{
#if 0
   PPORT_CONFIG    pPort;
#endif
   unsigned char   bPort;
#ifdef DEF_VXWORKS
   DRV_CTRL      * pDrvCtrl= gpDrvCtrl[0];	/* device ptr */
   PHARDWARE       phw = gpDrvCtrl[0]->pHW;
#endif

ACT_DEBUG_


   /*
    * Validate parameters
    */

   if ( (Port > 4 ) || (Port < 1) )
   {
       DisplayErrRetMsg( 1 ) ;
       return FALSE ;
   }

   if ( phw == NULL )
   {
       DisplayErrRetMsg( 3 ) ;
       return FALSE ;
   }


   /*
    * Clear MIB counter database.
    */

   if ( Port < 4)
   {
       /* clear individual Port's MIB counter database */
       phw->m_bPortSelect = ( UCHAR )( Port - 1 );
       HardwareClearCounters( phw );
   }
   else
   {
       /* clear all Port's MIB counter database */
       for (bPort=0; bPort<TOTAL_PORT_NUM; bPort++)
       {
           phw->m_bPortSelect = bPort;
           HardwareClearCounters( phw );
       }

   }

   /*
   DBG_PRINT( NEWLINE );
   hwtableshow ( "mib", 0, 1, TRUE );
   */

   #ifdef KS_QC_TEST
   DBG_PRINT( "^[ok]$" );
   #endif

   DBG_PRINT( NEWLINE);
   return TRUE ;
}

/*****************************************************************
*
* Command: hwdumprx
*
* Format: "hwdumprx  DumpFlag"
*
******************************************************************/
int hwdumprx
(
   unsigned long   DumpFlag
)
{
#ifdef DEF_VXWORKS
   PHARDWARE       phw = gpDrvCtrl[0]->pHW;
#endif


   /*
    * Validate parameters
    */

   if ( DumpFlag > 1 )
       return FALSE;

   if ( phw == NULL )
   {
        DisplayErrRetMsg( 3 ) ;
        return FALSE;
   }

   /*
    * Set dump transmit packet data flag
    */


   phw->fDebugDumpRx = ( UCHAR ) DumpFlag;
   DBG_PRINT( "%s dumpping receive packets."NEWLINE, (DumpFlag) ? "Start" : "Stop" );
   DBG_PRINT( NEWLINE);

   return TRUE ;
}

/*****************************************************************
*
* Command: hwdumptx
*
* Format: "hwdumptx  DumpFlag"
*
******************************************************************/
int hwdumptx
(
   unsigned long DumpFlag
)
{
#ifdef DEF_VXWORKS
   PHARDWARE       phw = gpDrvCtrl[0]->pHW;
#endif


   /*
    * Validate parameters
    */

   if ( DumpFlag > 1 )
       return FALSE;

   if ( phw == NULL )
   {
        DisplayErrRetMsg( 3 ) ;
        return FALSE;
   }

   /*
    * Set dump transmit packet data flag
    */


   phw->fDebugDumpTx = ( UCHAR ) DumpFlag;
   DBG_PRINT( "%s dumpping trasnmit packets."NEWLINE, (DumpFlag) ? "Start" : "Stop" );
   DBG_PRINT( NEWLINE);

   return TRUE ;
}


/*****************************************************************
*
* Command: hwloopback
*
* Format: "hwloopback  Flag"
*
******************************************************************/
int hwloopback
(
   unsigned long   Flag
)
{
#ifdef DEF_VXWORKS
   PHARDWARE       phw = gpDrvCtrl[0]->pHW;
#endif


   /*
    * Validate parameters
    */

   if ( Flag > 1 )
       return FALSE;

   if ( phw == NULL )
   {
        DisplayErrRetMsg( 3 ) ;
        return FALSE;
   }

   /*
    * Set loopback received packet data to transmit flag
    */


   phw->fLoopbackStart = ( UCHAR ) Flag;
   DBG_PRINT( "%s loopback the received packets to trasnmit."NEWLINE, (Flag) ? "Start" : "Stop" );
   DBG_PRINT( NEWLINE);

   return TRUE ;
}


#ifdef DEF_KS8842
/*****************************************************************
*
* Command: hwrepeat
*
* Format: "hwrepeat  Flag"
*
******************************************************************/
int hwrepeat
(
   unsigned long   Flag
)
{
#ifdef DEF_VXWORKS
   PHARDWARE       phw = gpDrvCtrl[0]->pHW;
#endif
   UINT8           bData;
   UINT8           bPort;


   /*
    * Validate parameters
    */

   if ( Flag > 1 )
       return FALSE;

   if ( phw == NULL )
   {
        DisplayErrRetMsg( 3 ) ;
        return FALSE;
   }


   /*
    * Enable\Disable Ports Flow control
    */

   if ( Flag == 1 )
   {

        /* Disable flow control on per-port */
        for ( bPort=0; bPort < 1; bPort++ )
            PortConfigForceFlowCtrl ( phw, bPort, FALSE );

        /* Disable flow control on QMU */

   }
   else
   {
        /* Enable flow control on per-port */
        for ( bPort=0; bPort < 1; bPort++ )
            PortConfigForceFlowCtrl ( phw, bPort, TRUE );

        /* Disable flow control on QMU */
   }

   /*
    *  Enable\Disable QMU transmit flow control
    */
   if ( Flag == 1 )
   {
        /* Disable flow control on QMU */
        #ifdef KS_PCI_BUS
        phw->m_dwTransmitConfig &= ~DMA_TX_CTRL_FLOW_ENABLE;
        phw->m_dwReceiveConfig  &= ~DMA_RX_CTRL_FLOW_ENABLE;
        #else
        phw->m_wTransmitConfig  &= ~TX_CTRL_FLOW_ENABLE;
        phw->m_wReceiveConfig   &= ~RX_CTRL_FLOW_ENABLE;
        #endif
   }
   else
   {
        /* Enable flow control on QMU */
        #ifdef KS_PCI_BUS
        phw->m_dwTransmitConfig |= DMA_TX_CTRL_FLOW_ENABLE;
        phw->m_dwReceiveConfig  |= DMA_RX_CTRL_FLOW_ENABLE;
        #else
        phw->m_wTransmitConfig  |= TX_CTRL_FLOW_ENABLE;
        phw->m_wReceiveConfig   |= RX_CTRL_FLOW_ENABLE;
        #endif
   }

   #ifdef KS_PCI_BUS
   HW_WRITE_DWORD( phw, REG_DMA_TX_CTRL, phw->m_dwTransmitConfig );
   HW_WRITE_DWORD( phw, REG_DMA_RX_CTRL, phw->m_dwReceiveConfig );
   #else
   HardwareWriteRegWord( phw, REG_TX_CTRL_BANK, REG_TX_CTRL_OFFSET, phw->m_wTransmitConfig );
   HardwareWriteRegWord( phw, REG_RX_CTRL_BANK, REG_RX_CTRL_OFFSET, phw->m_wReceiveConfig );
   #endif


   /*
    * Enable\Disable Repeat Mode
    */

   #ifdef KS_ISA_BUS
   HardwareSelectBank( phw, REG_SWITCH_CTRL_BANK );
   #endif

   /* Read SGCR3 register */
   HW_READ_BYTE( phw, REG_SWITCH_CTRL_3_OFFSET, &bData );

   bData &= 0x1f;

   if ( Flag == 1 )
   {
        /* Enable Repeat Mode, set half duplex, and disable flow control */
        bData |= (SWITCH_REPEATER | SWITCH_HALF_DUPLEX );
        HW_WRITE_BYTE( phw, REG_SGCR3_OFFSET, bData );
   }
   else
   {
        /* Disable Repeat Mode, set full duplex, and enable flow control */
        bData |= (SWITCH_FLOW_CTRL );
        HW_WRITE_BYTE( phw, REG_SGCR3_OFFSET, bData );
   }

   DBG_PRINT( NEWLINE);

   return TRUE ;
}
#endif /* #ifdef DEF_KS8842 */


/*****************************************************************
*
* Command: hwreset
*
* Format: "hwreset  TimeOut"
*
******************************************************************/
int hwreset
(
   unsigned long   TimeOut
)
{
#ifdef DEF_VXWORKS
   PHARDWARE       phw = gpDrvCtrl[0]->pHW;
#endif


   /*
    * Validate parameters
    */


   if ( phw == NULL )
   {
        DisplayErrRetMsg( 3 ) ;
        return FALSE;
   }

    /* Write 1 to reset device */
#ifdef KS_ISA_BUS
    HardwareSelectBank( phw, REG_GLOBAL_CTRL_BANK );
#endif

    HW_WRITE_WORD( phw, REG_GLOBAL_CTRL_OFFSET, GLOBAL_SOFTWARE_RESET );

    /* Wait for device to reset */
    DelayMillisec( TimeOut );


    /* Write 0 to clear device reset */
#ifdef KS_ISA_BUS
    HardwareSelectBank( phw, REG_GLOBAL_CTRL_BANK );
#endif

    HW_WRITE_WORD( phw, REG_GLOBAL_CTRL_OFFSET, 0 );

    #ifdef KS_QC_TEST
    DBG_PRINT( "^[ok]$" );
    #else
    DBG_PRINT( "Reset device done."NEWLINE );
    #endif

    DBG_PRINT( NEWLINE);

    return TRUE ;
}


/*****************************************************************
*
* Command: hwtestcable
*
* Format: "hwtestcable  Port"
*
******************************************************************/
int hwtestcable
(
   unsigned long   Port
)
{
#ifdef DEF_VXWORKS
   PHARDWARE       phw = gpDrvCtrl[0]->pHW;
#endif
   ULONG     i;
#if !defined( _WIN32 )  &&  !defined( DEF_LINUX )  &&  !defined( KS_ARM )
   UCHAR    *pbBuf;
#endif

   /*
    * Validate parameters
    */


   if ( phw == NULL )
   {
        DisplayErrRetMsg( 3 ) ;
        return FALSE;
   }

#ifdef DEF_KS8842
   if ( (Port < 1) || (Port > 2) )
#else
   if ( Port != 1 )
#endif
   {
      DisplayErrRetMsg( 1 ) ;
      return FALSE ;
   }


   /*
    * Get Device LinkMD cable status
    */

   HardwareGetCableStatus ( phw, ( UCHAR )(Port-1), tempBuf );

   memset ( tempStrBuf, 0, (LINKMD_INFO_COUNT + 1) * sizeof (unsigned long) );

#if defined( _WIN32 )  ||  defined( DEF_LINUX )  ||  defined( KS_ARM )
   DBG_PRINT( "^[%08lx,", tempBuf[ 0 ]);
#else
   pbBuf = (UCHAR *)&tempBuf[0];
   DBG_PRINT ( "^[%02x%02x%02x%02x,", *pbBuf++, *pbBuf++, *pbBuf++, *pbBuf++ );
#endif

   for ( i=1; i<9; i++ )
   {
#if defined( _WIN32 )  ||  defined( DEF_LINUX )  ||  defined( KS_ARM )
       DBG_PRINT( "%08lx,", tempBuf[ i ]);
#else
       pbBuf = (UCHAR *)&tempBuf[i];
       DBG_PRINT ( "%02x%02x%02x%02x,", *pbBuf++, *pbBuf++,*pbBuf++, *pbBuf++ );
#endif
   }

#if defined( _WIN32 )  ||  defined( DEF_LINUX )  ||  defined( KS_ARM )
   DBG_PRINT( "%08lx]$", tempBuf[ 9 ]);
#else
   pbBuf = (UCHAR *)&tempBuf[9];
   DBG_PRINT ( "%02x%02x%02x%02x]$", *pbBuf++, *pbBuf++,*pbBuf++, *pbBuf++ );
#endif

   DBG_PRINT( NEWLINE );

   return TRUE ;
}


/*****************************************************************
*
* Command: hwgetlink
*
* Format: "hwgetlink  Port"
*
******************************************************************/
int hwgetlink
(
   unsigned long   Port
)
{
#ifdef DEF_VXWORKS
   PHARDWARE       phw = gpDrvCtrl[0]->pHW;
#endif
   ULONG     i;
#if !defined( _WIN32 )  &&  !defined( DEF_LINUX )  &&  !defined( KS_ARM )
   UCHAR    *pbBuf;
#endif

   /*
    * Validate parameters
    */


   if ( phw == NULL )
   {
        DisplayErrRetMsg( 3 ) ;
        return FALSE;
   }

   if ( (Port < 1) || (Port > 2) )
   {
      DisplayErrRetMsg( 1 ) ;
      return FALSE ;
   }

   /*
    * Get Link status
    */

   HardwareGetLinkStatus ( phw, ( UCHAR )(Port-1), tempBuf );

   memset ( tempStrBuf, 0, (LINKMD_INFO_COUNT + 1) * sizeof (unsigned long) );

#if defined( _WIN32 )  ||  defined( DEF_LINUX )  ||  defined( KS_ARM )
   DBG_PRINT( "^[%08lx,", tempBuf[ 0 ]);
#else
   pbBuf = (UCHAR *)&tempBuf[0];
   DBG_PRINT ( "^[%02x%02x%02x%02x,", *pbBuf++, *pbBuf++, *pbBuf++, *pbBuf++ );
#endif

   for ( i=1; i<5; i++ )
   {
#if defined( _WIN32 )  ||  defined( DEF_LINUX )  ||  defined( KS_ARM )
       DBG_PRINT( "%08lx,", tempBuf[ i ]);
#else
       pbBuf = (UCHAR *)&tempBuf[i];
       DBG_PRINT ( "%02x%02x%02x%02x,", *pbBuf++, *pbBuf++,*pbBuf++, *pbBuf++ );
#endif
   }

#if defined( _WIN32 )  ||  defined( DEF_LINUX )  ||  defined( KS_ARM )
   DBG_PRINT( "%08lx]$", tempBuf[ 5 ]);
#else
   pbBuf = (UCHAR *)&tempBuf[5];
   DBG_PRINT ( "%02x%02x%02x%02x]$", *pbBuf++, *pbBuf++,*pbBuf++, *pbBuf++ );
#endif

   DBG_PRINT( NEWLINE );

   return TRUE ;
}


/*****************************************************************
*
* Command: hwsetlink
*
* Format: "hwsetlink  Port Data"
*
******************************************************************/
int hwsetlink
(
   unsigned long   Port,
   unsigned long   Data
)
{
#ifdef DEF_VXWORKS
   PHARDWARE       phw = gpDrvCtrl[0]->pHW;
#endif


   /*
    * Validate parameters
    */


   if ( phw == NULL )
   {
        DisplayErrRetMsg( 3 ) ;
        return FALSE;
   }

   if ( (Port < 1) || (Port > 2) )
   {
      DisplayErrRetMsg( 1 ) ;
      return FALSE ;
   }


   /*
    * Set Link status
    */

   HardwareSetCapabilities ( phw, ( UCHAR )(Port-1), Data );

   #ifdef KS_QC_TEST
   DBG_PRINT( "^[ok]$"NEWLINE );
   #else
   DBG_PRINT( "Set Port %d with Data=%08x"NEWLINE, (int) Port, (int) Data );
   #endif

   return TRUE ;
}


/*****************************************************************
*
* Command: hwgetmac
*
* Format: "hwgetmac "
*
******************************************************************/
int hwgetmac
(
)
{
#ifdef DEF_VXWORKS
   PHARDWARE       phw = gpDrvCtrl[0]->pHW;
#endif


   /*
    * Validate parameters
    */


   if ( phw == NULL )
   {
        DisplayErrRetMsg( 3 ) ;
        return FALSE;
   }

   /*
    * Display MAC address
    */

   DBG_PRINT ( "^[" );
   PrintMacAddress( phw->m_bPermanentAddress );
   DBG_PRINT( "]$"NEWLINE );

   return TRUE ;
}

/*****************************************************************
*
* Command: hwgetip
*
* Format: "hwgetip "
*
******************************************************************/
int hwgetip
(
)
{
#ifdef DEF_VXWORKS
   PHARDWARE       phw = gpDrvCtrl[0]->pHW;
#endif


   /*
    * Validate parameters
    */


   if ( phw == NULL )
   {
        DisplayErrRetMsg( 3 ) ;
        return FALSE;
   }

   /*
    * Display IP address
    */

   DBG_PRINT ( "^[" );
#ifdef M16C_62P
   PrintIpAddress( localmachine.localip );
#endif
#ifdef DEF_VXWORKS
   DBG_PRINT("%s", ksIpAddress);
#endif
#if defined( _WIN32 )  ||  defined( DEF_LINUX )  ||  defined( KS_ARM )
{
   unsigned long IpAddress;
   unsigned char* bData = ( unsigned char* ) &IpAddress;
   bData[ 0 ] = 192;
   bData[ 1 ] = 168;
   bData[ 2 ] = 1;
   bData[ 3 ] = 100;
   PrintIpAddress( IpAddress );
}
#endif
   DBG_PRINT( "]$"NEWLINE );

   return TRUE ;
}

/*****************************************************************
*
* Command: hwgettxcnt1
*
* Format: "hwgettxcnt1 "
*
******************************************************************/
int hwgettxcnt1
(
)
{
#ifdef DEF_VXWORKS
   PHARDWARE       phw = gpDrvCtrl[0]->pHW;
#endif


   /*
    * Validate parameters
    */


   if ( phw == NULL )
   {
        DisplayErrRetMsg( 3 ) ;
        return FALSE;
   }

   /*
    * Display Port 1 transmit packets counters
    */

   #ifdef KS_QC_TEST
   DBG_PRINT ( "^[%08lx]$"NEWLINE, (long) phw->m_cnCounter[ 0 ][ OID_COUNTER_DIRECTED_FRAMES_XMIT ]);
   #else
   DBG_PRINT ( "^[%ld]$"NEWLINE, (long) phw->m_cnCounter[ 0 ][ OID_COUNTER_DIRECTED_FRAMES_XMIT ]);
   #endif
   return TRUE ;
}

/*****************************************************************
*
* Command: hwgettxcnt2
*
* Format: "hwgettxcnt2 "
*
******************************************************************/
int hwgettxcnt2
(
)
{
#ifdef DEF_VXWORKS
   PHARDWARE       phw = gpDrvCtrl[0]->pHW;
#endif


   /*
    * Validate parameters
    */


   if ( phw == NULL )
   {
        DisplayErrRetMsg( 3 ) ;
        return FALSE;
   }

   /*
    * Display Port 2 transmit packets counters
    */

   #ifdef KS_QC_TEST
   DBG_PRINT ( "^[%08lx]$"NEWLINE, (long) phw->m_cnCounter[ 1 ][ OID_COUNTER_DIRECTED_FRAMES_XMIT ]);
   #else
   DBG_PRINT ( "^[%ld]$"NEWLINE, (long) phw->m_cnCounter[ 1 ][ OID_COUNTER_DIRECTED_FRAMES_XMIT ]);
   #endif

   return TRUE ;
}

/*****************************************************************
*
* Command: hwgetrxcnt1
*
* Format: "hwgetrxcnt1 "
*
******************************************************************/
int hwgetrxcnt1
(
)
{
#ifdef DEF_VXWORKS
   PHARDWARE       phw = gpDrvCtrl[0]->pHW;
#endif


   /*
    * Validate parameters
    */


   if ( phw == NULL )
   {
        DisplayErrRetMsg( 3 ) ;
        return FALSE;
   }

   /*
    * Display Port 1 received packets counters
    */

   #ifdef KS_QC_TEST
   DBG_PRINT ( "^[%08lx]$"NEWLINE, (long) phw->m_cnCounter[ 0 ][ OID_COUNTER_DIRECTED_FRAMES_RCV ]);
   #else
   DBG_PRINT ( "^[%ld]$"NEWLINE, (long) phw->m_cnCounter[ 0 ][ OID_COUNTER_DIRECTED_FRAMES_RCV ]);
   #endif

   return TRUE ;
}

/*****************************************************************
*
* Command: hwgetrxcnt2
*
* Format: "hwgetrxcnt2 "
*
******************************************************************/
int hwgetrxcnt2
(
)
{
#ifdef DEF_VXWORKS
   PHARDWARE       phw = gpDrvCtrl[0]->pHW;
#endif


   /*
    * Validate parameters
    */


   if ( phw == NULL )
   {
        DisplayErrRetMsg( 3 ) ;
        return FALSE;
   }

   /*
    * Display Port 2 received packets counters
    */

   #ifdef KS_QC_TEST
   DBG_PRINT ( "^[%08lx]$"NEWLINE, (long) phw->m_cnCounter[ 1 ][ OID_COUNTER_DIRECTED_FRAMES_RCV ]);
   #else
   DBG_PRINT ( "^[%ld]$"NEWLINE, (long) phw->m_cnCounter[ 1 ][ OID_COUNTER_DIRECTED_FRAMES_RCV ]);
   #endif

   return TRUE ;
}

/*****************************************************************
*
* Command: hwtxarp
*
* Format: "hwtxarp  Port  Repeat  MAC  IP"
*
******************************************************************/
int hwtxarp
(
   unsigned long      Port,
   unsigned long      RepeatCount,
   unsigned char FAR *pMac,
   unsigned char FAR *pIp
)
{
#ifdef DEF_VXWORKS
   PHARDWARE       phw = gpDrvCtrl[0]->pHW;
#endif
    unsigned char  mac[6];
    unsigned char  ipAddr[4];


   /*
    * Validate parameters
    */


   if ( phw == NULL )
   {
        DisplayErrRetMsg( 3 ) ;
        return FALSE;
   }

   if (BufferInit() != TRUE)
   {
      return FALSE ;
   }


   /* Preparing ARP request packet */

   strToMac (pMac, mac);
   strToIp ( pIp, ipAddr );
   memcpy ( &TestPacket[0], mac, 6);
   memcpy ( &TestPacket[38], ipAddr, 4);
   memcpy ( (char FAR *)&apbBuffer[0][0], (char FAR *)&TestPacket[0], 42 );


   /*
    * Transmit ARP request packets to device
    */
#ifdef DEF_KS8841
   Port = 0;
#endif

   hwbuftx ( Port, 0, 60, RepeatCount, 1, 1 );

   return TRUE ;
}


#ifdef DEF_KS8841
#ifdef KS_ISA_BUS
/*****************************************************************
*
* Command: hwrxthres
*
* Format: "hwrxthres  FData"
*
******************************************************************/
int hwrxthres
(
   unsigned long   FData
)
{
   USHORT wData;
#ifdef DEF_VXWORKS
   PHARDWARE       phw = gpDrvCtrl[0]->pHW;
#endif


   /*
    * Validate parameters
    */

   if ( FData < 1 )
       return FALSE;

   if ( phw == NULL )
   {
        DisplayErrRetMsg( 3 ) ;
        return FALSE;
   }

   /*
    * Set Early Receive Threshold value
    */

   phw->m_wReceiveThreshold = ( USHORT )(FData * EARLY_RX_MULTIPLE);

   HardwareReadRegWord( phw, REG_EARLY_RX_BANK, REG_EARLY_RX_OFFSET, &wData );

   wData &= ~EARLY_RX_THRESHOLD;
   wData |= phw->m_wReceiveThreshold / EARLY_RX_MULTIPLE;

   HardwareWriteRegWord( phw, REG_EARLY_RX_BANK, REG_EARLY_RX_OFFSET,wData );

   DBG_PRINT( NEWLINE);

   return TRUE ;
}


/*****************************************************************
*
* Command: hwtxthres
*
* Format: "hwtxthres  FData"
*
******************************************************************/
int hwtxthres
(
   unsigned long   FData
)
{
   USHORT wData;
#ifdef DEF_VXWORKS
   PHARDWARE       phw = gpDrvCtrl[0]->pHW;
#endif


   /*
    * Validate parameters
    */

   if ( FData < 1 )
       return FALSE;

   if ( phw == NULL )
   {
        DisplayErrRetMsg( 3 ) ;
        return FALSE;
   }

   /*
    * Set Early Transmit Threshold value
    */

   phw->m_wTransmitThreshold = ( USHORT )(FData * EARLY_TX_MULTIPLE);

   HardwareReadRegWord( phw, REG_EARLY_TX_BANK, REG_EARLY_TX_OFFSET, &wData );

   wData &= ~EARLY_TX_THRESHOLD;
   wData |= phw->m_wTransmitThreshold / EARLY_TX_MULTIPLE;

   HardwareWriteRegWord( phw, REG_EARLY_TX_BANK, REG_EARLY_TX_OFFSET,wData );

   DBG_PRINT( NEWLINE);

   return TRUE ;
}


/*****************************************************************
*
* Command: hwearly
*
* Format: "hwearly  Flag"
*
******************************************************************/
int hwearly
(
   unsigned long   Flag
)
{
   USHORT wData;
#ifdef DEF_VXWORKS
   PHARDWARE       phw = gpDrvCtrl[0]->pHW;
#endif


   /*
    * Validate parameters
    */

   if ( Flag > 1 )
       return FALSE;

   if ( phw == NULL )
   {
        DisplayErrRetMsg( 3 ) ;
        return FALSE;
   }

   if ( Flag == 1 )
   {

       /*
        * Enable Early Transmit\Receive function
        */

       /* Enable Early Transmit\Receive Interrupt */
       phw->m_wInterruptMask |= ( INT_TX_UNDERRUN | INT_RX_EARLY | INT_RX_ERROR );
       HardwareAcknowledgeInterrupt( phw, 0xFFFF );
       HardwareEnableInterrupt( phw );

       /* Enable Receive Error Frame */
       phw->m_wReceiveConfig |= RX_CTRL_BAD_PACKET;
       HardwareStartReceive( phw );

       /* Enable Early Transmit */
       HardwareReadRegWord( phw, REG_EARLY_TX_BANK, REG_EARLY_TX_OFFSET, &wData );
       wData |= EARLY_TX_ENABLE;
       HardwareWriteRegWord( phw, REG_EARLY_TX_BANK, REG_EARLY_TX_OFFSET, wData );

       /* Enable Early Receive */
       HardwareReadRegWord( phw, REG_EARLY_RX_BANK, REG_EARLY_RX_OFFSET, &wData );
       wData |= EARLY_RX_ENABLE;
       HardwareWriteRegWord( phw, REG_EARLY_RX_BANK, REG_EARLY_RX_OFFSET, wData );

   }
   else
   {

       /*
        * Disable Early Transmit\Receive function
        */

       /* Disable Early Transmit */
       HardwareReadRegWord( phw, REG_EARLY_TX_BANK, REG_EARLY_TX_OFFSET, &wData );
       wData &= ~EARLY_TX_ENABLE;
       HardwareWriteRegWord( phw, REG_EARLY_TX_BANK, REG_EARLY_TX_OFFSET, wData );

       /* Disable Early Receive */
       HardwareReadRegWord( phw, REG_EARLY_RX_BANK, REG_EARLY_RX_OFFSET, &wData );
       wData &= ~EARLY_RX_ENABLE;
       HardwareWriteRegWord( phw, REG_EARLY_RX_BANK, REG_EARLY_RX_OFFSET, wData );

       /* Disable Receive Error Frame */
       phw->m_wReceiveConfig &= ~RX_CTRL_BAD_PACKET;
       HardwareStartReceive( phw );

       /* Disable Early Transmit\Receive Interrupt */
       phw->m_wInterruptMask &= ~( INT_TX_UNDERRUN | INT_RX_EARLY | INT_RX_ERROR);
       HardwareAcknowledgeInterrupt( phw, 0xFFFF );
       HardwareEnableInterrupt( phw );

   }

   DBG_PRINT( NEWLINE);

   return TRUE ;
}
#endif /* #ifdef KS_ISA_BUS */
#endif /* #ifdef DEF_KS8841 */

/*****************************************************************
*
* Command: hwmagic
*
* Format: "hwmagic "
*
******************************************************************/
int hwmagic
(
)
{
#ifdef DEF_VXWORKS
   PHARDWARE       phw = gpDrvCtrl[0]->pHW;
#endif


   /*
    * Validate parameters
    */

   if ( phw == NULL )
   {
        DisplayErrRetMsg( 3 ) ;
        return FALSE;
   }

   /*
    * Enables the magic packet pattern delection
    */
   HardwareEnableWolMagicPacket ( phw );
   DBG_PRINT( NEWLINE);

   return TRUE ;
}

/*****************************************************************
*
* Command: hwwolframe
*
* Format: "hwwolframe  FrameNum  CRC  ByteMask"
*
******************************************************************/
int hwwolframe
(
   unsigned long   FrameNum,
   unsigned long   CRC,
   unsigned char   ByteMask
)
{
#ifdef DEF_VXWORKS
   PHARDWARE       phw = gpDrvCtrl[0]->pHW;
#endif


   /*
    * Validate parameters
    */

   if ( phw == NULL )
   {
        DisplayErrRetMsg( 3 ) ;
        return FALSE;
   }
   if ( FrameNum > 3 )
   {
      DisplayErrRetMsg( 1 ) ;
      return FALSE ;
   }
   if ( ByteMask > 63 )
   {
      DisplayErrRetMsg( 1 ) ;
      return FALSE ;
   }

#if (1)
   {
   ULONG i=0;

   if (BufferInit() != TRUE)
   {
      return FALSE ;
   }

   /*
    * for testing: calculate fix received wol frame, byte mask (0,8,16,24,32,40,48,56) CRC
    * FF FF FF FF FF FF 00 00 - 00 00 00 04 08 00 45 00
    * 00 32 00 00 00 00 40 01 - F6 C5 C0 A8 01 B4 C0 A8
    * 01 01 00 00 0E EE 00 00 - 00 00 FF FF FF FF FF FF
    * 08 00 70 22 44 55 08 00 - 70 22 44 55 08 00 70 22
    */
#if (1)  /* crc32 = 0x3BF841DA */
   apbBuffer[ 0 ][ i++ ] = 0xff ;
   apbBuffer[ 0 ][ i++ ] = 0x00 ;

   apbBuffer[ 0 ][ i++ ] = 0x00 ;
   apbBuffer[ 0 ][ i++ ] = 0xf6;

   apbBuffer[ 0 ][ i++ ] = 0x01 ;
   apbBuffer[ 0 ][ i++ ] = 0x00 ;

   apbBuffer[ 0 ][ i++ ] = 0x08 ;
   apbBuffer[ 0 ][ i++ ] = 0x70 ;

#endif

#if (0)  /* crc32 = 0x404AF13B */
   apbBuffer[ 0 ][ i++ ] = 0xff ;
   apbBuffer[ 0 ][ i++ ] = 0xff ;
   apbBuffer[ 0 ][ i++ ] = 0xff ;
   apbBuffer[ 0 ][ i++ ] = 0xff ;
   apbBuffer[ 0 ][ i++ ] = 0xff ;
   apbBuffer[ 0 ][ i++ ] = 0xff ;
   apbBuffer[ 0 ][ i++ ] = 0x00 ;
   apbBuffer[ 0 ][ i++ ] = 0x00 ;

   apbBuffer[ 0 ][ i++ ] = 0xa5 ;
   apbBuffer[ 0 ][ i++ ] = 0x5a ;
   apbBuffer[ 0 ][ i++ ] = 0x3c ;
   apbBuffer[ 0 ][ i++ ] = 0x95 ;
   apbBuffer[ 0 ][ i++ ] = 0x06 ;
   apbBuffer[ 0 ][ i++ ] = 0x01 ;
   apbBuffer[ 0 ][ i++ ] = 0x00 ;
   apbBuffer[ 0 ][ i++ ] = 0x00 ;

   apbBuffer[ 0 ][ i++ ] = 0x11 ;
   apbBuffer[ 0 ][ i++ ] = 0x11 ;
   apbBuffer[ 0 ][ i++ ] = 0x11 ;
   apbBuffer[ 0 ][ i++ ] = 0x11 ;
   apbBuffer[ 0 ][ i++ ] = 0x22 ;
   apbBuffer[ 0 ][ i++ ] = 0x22 ;
   apbBuffer[ 0 ][ i++ ] = 0x22 ;
   apbBuffer[ 0 ][ i++ ] = 0x22 ;

   apbBuffer[ 0 ][ i++ ] = 0x33 ;
   apbBuffer[ 0 ][ i++ ] = 0x33 ;
   apbBuffer[ 0 ][ i++ ] = 0x33 ;
   apbBuffer[ 0 ][ i++ ] = 0x33 ;
   apbBuffer[ 0 ][ i++ ] = 0x44 ;
   apbBuffer[ 0 ][ i++ ] = 0x44 ;
   apbBuffer[ 0 ][ i++ ] = 0x44 ;
   apbBuffer[ 0 ][ i++ ] = 0x44 ;

   apbBuffer[ 0 ][ i++ ] = 0x55 ;
   apbBuffer[ 0 ][ i++ ] = 0x55 ;
   apbBuffer[ 0 ][ i++ ] = 0x55 ;
   apbBuffer[ 0 ][ i++ ] = 0x55 ;
   apbBuffer[ 0 ][ i++ ] = 0x66 ;
   apbBuffer[ 0 ][ i++ ] = 0x66 ;
   apbBuffer[ 0 ][ i++ ] = 0x66 ;
   apbBuffer[ 0 ][ i++ ] = 0x66 ;

   apbBuffer[ 0 ][ i++ ] = 0x77 ;
   apbBuffer[ 0 ][ i++ ] = 0x77 ;
   apbBuffer[ 0 ][ i++ ] = 0x77 ;
   apbBuffer[ 0 ][ i++ ] = 0x77 ;
   apbBuffer[ 0 ][ i++ ] = 0x88 ;
   apbBuffer[ 0 ][ i++ ] = 0x88 ;
   apbBuffer[ 0 ][ i++ ] = 0x88 ;
   apbBuffer[ 0 ][ i++ ] = 0x88 ;

   apbBuffer[ 0 ][ i++ ] = 0x99 ;
   apbBuffer[ 0 ][ i++ ] = 0x99 ;
   apbBuffer[ 0 ][ i++ ] = 0x99 ;
   apbBuffer[ 0 ][ i++ ] = 0x99 ;
   apbBuffer[ 0 ][ i++ ] = 0xaa ;
   apbBuffer[ 0 ][ i++ ] = 0xaa ;
   apbBuffer[ 0 ][ i++ ] = 0xaa ;
   apbBuffer[ 0 ][ i++ ] = 0xaa ;

   apbBuffer[ 0 ][ i++ ] = 0xbb ;
   apbBuffer[ 0 ][ i++ ] = 0xbb ;
   apbBuffer[ 0 ][ i++ ] = 0xbb ;
   apbBuffer[ 0 ][ i++ ] = 0xbb ;
   apbBuffer[ 0 ][ i++ ] = 0xcc ;
   apbBuffer[ 0 ][ i++ ] = 0xcc ;
   apbBuffer[ 0 ][ i++ ] = 0xcc ;
   apbBuffer[ 0 ][ i++ ] = 0xcc ;

#endif

   CRC = ether_crc ( i, (UCHAR *)&apbBuffer[0][0] );
   DBG_PRINT ("ether_crc = 0x%08lx, i=%ld"NEWLINE, CRC, i);

   }
#endif

   /*
    * Set and Enables the wake up frame pattern delection
    */
   HardwareSetWolFrameByteMask ( phw, FrameNum, ByteMask );
   HardwareSetWolFrameCRC ( phw, FrameNum, CRC );
   HardwareEnableWolFrame ( phw, FrameNum );
   DBG_PRINT( NEWLINE);

   return TRUE ;
}

/*****************************************************************
*
* Command: hwclearpme
*
* Format: "hwclearpme "
*
******************************************************************/
int hwclearpme
(
)
{
#ifdef DEF_VXWORKS
   PHARDWARE       phw = gpDrvCtrl[0]->pHW;
#endif


   /*
    * Validate parameters
    */

   if ( phw == NULL )
   {
        DisplayErrRetMsg( 3 ) ;
        return FALSE;
   }

   /*
    * Clear PME_Status to deassert PMEN pin
    */
   HardwareClearWolPMEStatus ( phw );

#ifdef M16C_62P
   /* Wake-on-LAN LED off */
   wol_lcd_off();
#endif

   DBG_PRINT( NEWLINE);

   return TRUE ;
}



#ifdef KS_PCI_BUS
/*****************************************************************
*
* Command: hwpciread - Read PCI Configuration Space Register.
* Format: "hwpciread  busNo deviceNo funcNo regNo
*
******************************************************************/
int hwpciread
(
     unsigned long	busNo,		/* bus number */
     unsigned long	devNo,	    /* device number */
     unsigned long	funcNo,		/* function number */
     unsigned long	regNo		/* register address */
)
{
#ifdef DEF_VXWORKS
   PHARDWARE       phw = gpDrvCtrl[0]->pHW;
     unsigned long  RegData ;
#endif


   /*
    * Validate parameters
    */

   if ( phw == NULL )
   {
        DisplayErrRetMsg( 3 ) ;
        return FALSE;
   }

   /*
    * Read and Display PCI Configuration Space
    */
#ifdef DEF_VXWORKS
   if (pciConfigInLong( busNo,
                        devNo,
                        funcNo,
                        regNo,
                        &RegData ) != 0)
   {
      DBG_PRINT( "read PCI Configuration Space Register Address %lx fail."NEWLINE, regNo ) ;
      return FALSE ;
   }
   DBG_PRINT( "regNo.%08X : %08x "NEWLINE, regNo, RegData ) ;

#endif
   return TRUE;
}

int hwpciread_cmd
(
   void FAR *p,
   void FAR **pp
)
{
#ifdef CLI_DEBUG
   unsigned short FAR  *pLen=p;
   unsigned short FAR   argc=pLen[0];
   int     i=0;
#endif
   char   FAR **argv=(char FAR **)pp;

   unsigned long	busNo;		/* bus number */
   unsigned long	devNo;	    /* device number */
   unsigned long	funcNo;		/* function number */
   unsigned long	regNo;      /* register address */


#ifdef CLI_DEBUG
   for(i=0;i<argc;i++)
	 DBG_PRINT("The token %d is argv[%d]=%s"NEWLINE,i+1,i,(char FAR *)argv[i]);
#endif


   /*
    * Get parameters
    */

#if !defined( _WIN32 )  &&  !defined( DEF_LINUX )  &&  !defined( KS_ARM )
   busNo = (unsigned long) strtol( argv[0], NULL, 16 );
   devNo = (unsigned long) strtol( argv[1], NULL, 16 );
   funcNo= (unsigned long) strtol( argv[2], NULL, 16 );
   regNo = (unsigned long) strtol( argv[3], NULL, 16 );
#else
   busNo = (unsigned long) ( argv[0]);
   devNo = (unsigned long) ( argv[1]);
   funcNo= (unsigned long) ( argv[2]);
   regNo = (unsigned long) ( argv[3]);
#endif

   return (  hwpciread ( busNo, devNo, funcNo, regNo ) );
}

/*****************************************************************
*
* Command: hwpciwrite - Write PCI Configuration Space Register.
* Format: "hwpciwrite  busNo deviceNo funcNo regNo RegData
*
******************************************************************/
int hwpciwrite
(
     unsigned long	busNo,		/* bus number */
     unsigned long	devNo,	    /* device number */
     unsigned long	funcNo,		/* function number */
     unsigned long	regNo,		/* register address */
     unsigned long  RegData
)
{
#ifdef DEF_VXWORKS
   PHARDWARE       phw = gpDrvCtrl[0]->pHW;
     unsigned long  ReadRegData ;
#endif


   /*
    * Validate parameters
    */

   if ( phw == NULL )
   {
        DisplayErrRetMsg( 3 ) ;
        return FALSE;
   }

   /*
    * Write to PCI Configuration Space
    */
#ifdef DEF_VXWORKS
   if (pciConfigOutLong( busNo,
                         devNo,
                         funcNo,
                         regNo,
                         RegData ) != 0)
   {
      DBG_PRINT( "write to PCI Configuration Space Register Address %lx fail."NEWLINE, regNo ) ;
      return FALSE ;
   }

#endif

   /*
    * Read back and Display PCI Configuration Space
    */

#ifdef DEF_VXWORKS
   if (pciConfigInLong( busNo,
                        devNo,
                        funcNo,
                        regNo,
                        &ReadRegData ) != 0)
   {
      DBG_PRINT( "read PCI Configuration Space Register Address %lx fail."NEWLINE, regNo ) ;
      return FALSE ;
   }
   DBG_PRINT( "regNo.%08X : %08x "NEWLINE, regNo, ReadRegData ) ;

#endif
   return TRUE;
}

int hwpciwrite_cmd
(
   void FAR *p,
   void FAR **pp
)
{
#ifdef CLI_DEBUG
   unsigned short FAR  *pLen=p;
   unsigned short FAR   argc=pLen[0];
   int     i=0;
#endif
   char   FAR **argv=(char FAR **)pp;

   unsigned long	busNo;		/* bus number */
   unsigned long	devNo;	    /* device number */
   unsigned long	funcNo;		/* function number */
   unsigned long	regNo;      /* register address */
   unsigned long    RegData;


#ifdef CLI_DEBUG
   for(i=0;i<argc;i++)
	 DBG_PRINT("The token %d is argv[%d]=%s"NEWLINE,i+1,i,(char FAR *)argv[i]);
#endif


   /*
    * Get parameters
    */

#if !defined( _WIN32 )  &&  !defined( DEF_LINUX )  &&  !defined( KS_ARM )
   busNo = (unsigned long) strtol( argv[0], NULL, 16 );
   devNo = (unsigned long) strtol( argv[1], NULL, 16 );
   funcNo= (unsigned long) strtol( argv[2], NULL, 16 );
   regNo = (unsigned long) strtol( argv[3], NULL, 16 );
   RegData = (unsigned long) strtol( argv[4], NULL, 16 );
#else
   busNo = (unsigned long) ( argv[0]);
   devNo = (unsigned long) ( argv[1]);
   funcNo= (unsigned long) ( argv[2]);
   regNo = (unsigned long) ( argv[3]);
   RegData = (unsigned long) ( argv[4]);
#endif

   return (  hwpciwrite ( busNo, devNo, funcNo, regNo, RegData ) );
}

#endif /* DEF_PCI_BUS */



/*---------------------------------------------------------------------------*/

/*****************************************************************/
void unknownDisplay ( void )
{
   DBG_PRINT( "unknown CLI command \n\r" );
}

/*****************************************************************/
void usagDisplay
(
   unsigned short msgIndex
)
{
   char FAR ** pbMsg = (char FAR **)&hwHelpMsgStr[msgIndex];


   if (**pbMsg)
      DBG_PRINT( "Usage: %s", (char FAR *)*pbMsg );

   DBG_PRINT( NEWLINE );

}


/*****************************************************************/
static void formatPortToStr
(
    unsigned char    ports,
    unsigned char    numPorts,
    char           * buf
)
{
    char * tmp = buf;
    unsigned char   x,x1;

    memset(tmp, ' ', numPorts*2);
    for( x = 0, x1 = 0; x < numPorts; x++ )
    {

        if( ports & 0x01  )
           tmp[x1] = 'M';
        else
           tmp[x1] = '-';

        ports >>= 1;
        x1 += 2;
    }
}


/*****************************************************************/
static void macDisplayTitle
(
   unsigned long dwTableID
)
{
   switch ( dwTableID )
   {
        case TABLE_STATIC_MAC:
            DBG_PRINT ("Entry  Status   MAC                FID  Port( 1 2 3)  UseFID  Override  "NEWLINE);
            break;

        case TABLE_DYNAMIC_MAC:
            DBG_PRINT ("Entry  Status   MAC                FID  Port( 1 2 3)  AgingTime"NEWLINE);
            break;

        case TABLE_VLAN:
            DBG_PRINT ("Entry  Status   VID    FID  Port( 1 2 3)  "NEWLINE);
            break;

        case TABLE_MIB:
            DBG_PRINT ("                       Port 1      Port 2      Port 3        Driver "NEWLINE);
            break;
   }
}

/******************************************************************
 *
 * showTableEntry - .
 * DESCRIPTION:
 *      Dispaly contents of MAC and Vlan Table entry.
 *
 * PARAMETERS:
 *
 *      unsigned long   dwTableID
 *         Table indication.
 *
 *      unsigned long  macIndex
 *         MAC table entry index.
 *
 *      unsigned char  Fid
 *         Filter ID.
 *
 *      unsigned char  Vid
 *         Vlan ID.
 *
 *      unsigned char * macAddr
 *		   pointer to a 6-byte of MAC DA array.
 *
 *      unsigned char   ports
 *		   PTF ports ( 1 - port is set, otherwise ).
 *
 *      BOOLEAN    fOverride
 *           TRUE  - override port enable\disable transmit\receive setting
 *           FALSE - no override
 *
 *      BOOLEAN    fUsedFID
 *         use (FID+MAC) to look up in static table
 *           TRUE  - use (FID+MAC)
 *           FALSE - use MAC only
 *
 *      unsigned char  bTimestamp
 *         Time stamp
 *
 *      unsigned long   status
 *         The status of this MAC entry.
 *           1 - "invalid"
 *           2 - "learned"
 *           3 - "static"
 *
*******************************************************************/
static void showTableEntry
(
     unsigned long     dwTableID,
     unsigned long     macIndex,
     unsigned char     Fid,
     unsigned short    Vid,
     unsigned char   * macAddr,
     unsigned char     ports,
     BOOLEAN           fOverride,
     BOOLEAN           fUsedFID,
     unsigned char     bTimestamp,
     unsigned long     status
)
{
#if 0
     char              macAddrStr[20];
#endif
     unsigned char     portListStr[7];



     DBG_PRINT( "%04d   ", ( int ) macIndex ) ;             /* Entry */

     if ( status > 3 )
         status = 1;
     DBG_PRINT ((char FAR *)fdbStatusStr[status]);          /* Status */

     /* if invalid entry, rest of infomation is invalid */
     /*
     if (status == 1)
     {
         DBG_PRINT( NEWLINE);
         return;
     }
     */

     if ( dwTableID == TABLE_VLAN)
         DBG_PRINT( "%04d ", Vid ) ;                        /* VID */
     else
         DBG_PRINT( "%02x-%02x-%02x-%02x-%02x-%02x",        /* MAC address */
                     macAddr[ 0 ], macAddr[ 1 ], macAddr[ 2 ],
                     macAddr[ 3 ], macAddr[ 4 ], macAddr[ 5 ]);


     DBG_PRINT( "  %04d  ", Fid ) ;                         /* FID */

     memset(portListStr, 0, sizeof(portListStr));
     formatPortToStr ( ports, (unsigned char)3, (char *)portListStr );
     DBG_PRINT ( "     ");
     DBG_PRINT ( (char FAR *)portListStr );                 /* Ports */

     if ( dwTableID == TABLE_STATIC_MAC)
     {
        DBG_PRINT ("  %s ", (fUsedFID) ?  "TRUE " : "FALSE");  /* UseFID */
        DBG_PRINT ("  %s ", (fOverride) ? "TRUE " : "FALSE");  /* Override */
     }

     if ( dwTableID == TABLE_DYNAMIC_MAC)
     {
        DBG_PRINT( "  %02d ", bTimestamp ) ;                  /* Aging time */
     }

     DBG_PRINT( NEWLINE);

}

/******************************************************************
 *
 * showMibTable - .
 * DESCRIPTION:
 *      Dispaly contents of MIB counters.
 *
 * PARAMETERS:
 *
 *       PHARDWARE phw
 *           Pointer to hardware instance.
 *
*******************************************************************/
static void showMibTable
(
     PHARDWARE    phw
)
{
     BOOLEAN      fPort2=FALSE;
     PPORT_CONFIG pPort1=&phw->m_Port[ 0 ];
     PPORT_CONFIG pPort2=&phw->m_Port[ 1 ];
     PPORT_CONFIG pPort3=&phw->m_Port[ 2 ];

#ifdef DEF_KS8842
     fPort2 = TRUE;
#endif

     DBG_PRINT( "RxLoPriorityByte     : %08ld    %08ld    %08ld      %08ld      %08ld    RxByte"NEWLINE,
          (long) pPort1->cnCounter[ MIB_COUNTER_RX_LO_PRIORITY ],
                 (fPort2) ? (long) pPort2->cnCounter[ MIB_COUNTER_RX_LO_PRIORITY ] : 0,
          (long) pPort3->cnCounter[ MIB_COUNTER_RX_LO_PRIORITY ],
          (long) phw->m_cnCounter[ 0 ][ OID_COUNTER_DIRECTED_BYTES_RCV ],
          (long) phw->m_cnCounter[ 1 ][ OID_COUNTER_DIRECTED_BYTES_RCV ]
                 ) ;

     DBG_PRINT( "RxHiPriorityByte     : %08ld    %08ld    %08ld      %08ld      %08ld    RxBroadcastPkts"NEWLINE,
          (long) pPort1->cnCounter[ MIB_COUNTER_RX_HI_PRIORITY ],
                 (fPort2) ? (long) pPort2->cnCounter[ MIB_COUNTER_RX_HI_PRIORITY ] : 0,
          (long) pPort3->cnCounter[ MIB_COUNTER_RX_HI_PRIORITY ],
          (long) phw->m_cnCounter[ 0 ][ OID_COUNTER_BROADCAST_FRAMES_RCV ],
          (long) phw->m_cnCounter[ 1 ][ OID_COUNTER_BROADCAST_FRAMES_RCV ]
                 ) ;

     DBG_PRINT( "RxUndersizePkt       : %08ld    %08ld    %08ld      %08ld      %08ld    RxMulticastPkts"NEWLINE,
          (long) pPort1->cnCounter[ MIB_COUNTER_RX_UNDERSIZE ],
                 (fPort2) ? (long) pPort2->cnCounter[ MIB_COUNTER_RX_UNDERSIZE ] : 0,
          (long) pPort3->cnCounter[ MIB_COUNTER_RX_UNDERSIZE ],
          (long) phw->m_cnCounter[ 0 ][ OID_COUNTER_MULTICAST_FRAMES_RCV ],
          (long) phw->m_cnCounter[ 1 ][ OID_COUNTER_MULTICAST_FRAMES_RCV ]
                 ) ;

     DBG_PRINT( "RxFragments          : %08ld    %08ld    %08ld      %08ld      %08ld    RxUnicastPkts"NEWLINE,
          (long) pPort1->cnCounter[ MIB_COUNTER_RX_FRAGMENT ],
                 (fPort2) ? (long) pPort2->cnCounter[ MIB_COUNTER_RX_FRAGMENT ] : 0,
          (long) pPort3->cnCounter[ MIB_COUNTER_RX_FRAGMENT ],
          (long) phw->m_cnCounter[ 0 ][ OID_COUNTER_UNICAST_FRAMES_RCV ],
          (long) phw->m_cnCounter[ 1 ][ OID_COUNTER_UNICAST_FRAMES_RCV ]
                 ) ;

     DBG_PRINT( "RxOversize           : %08ld    %08ld    %08ld      %08ld      %08ld    RxTotalPkts"NEWLINE,
          (long) pPort1->cnCounter[ MIB_COUNTER_RX_OVERSIZE ],
                 (fPort2) ? (long) pPort2->cnCounter[ MIB_COUNTER_RX_OVERSIZE ] : 0,
          (long) pPort3->cnCounter[ MIB_COUNTER_RX_OVERSIZE ],
          (long) phw->m_cnCounter[ 0 ][ OID_COUNTER_DIRECTED_FRAMES_RCV ],
          (long) phw->m_cnCounter[ 1 ][ OID_COUNTER_DIRECTED_FRAMES_RCV ]
                 ) ;

     DBG_PRINT( "RxJabbers            : %08ld    %08ld    %08ld"NEWLINE,
          (long) pPort1->cnCounter[ MIB_COUNTER_RX_JABBER ],
                 (fPort2) ? (long) pPort2->cnCounter[ MIB_COUNTER_RX_JABBER ] : 0,
          (long) pPort3->cnCounter[ MIB_COUNTER_RX_JABBER ]
                 ) ;

     DBG_PRINT( "RxSymbolError        : %08ld    %08ld    %08ld      %08ld      %08ld    RxRuntError"NEWLINE,
          (long) pPort1->cnCounter[ MIB_COUNTER_RX_SYMBOL_ERR ],
                 (fPort2) ? (long) pPort2->cnCounter[ MIB_COUNTER_RX_SYMBOL_ERR ] : 0,
          (long) pPort3->cnCounter[ MIB_COUNTER_RX_SYMBOL_ERR ],
          (long) phw->m_cnCounter[ 0 ][ OID_COUNTER_RCV_ERROR_RUNT ],
          (long) phw->m_cnCounter[ 1 ][ OID_COUNTER_RCV_ERROR_RUNT ]
                 ) ;

     DBG_PRINT( "RxCRCError           : %08ld    %08ld    %08ld      %08ld      %08ld    RxCRCError"NEWLINE,
          (long) pPort1->cnCounter[ MIB_COUNTER_RX_CRC_ERR ],
                 (fPort2) ? (long) pPort2->cnCounter[ MIB_COUNTER_RX_CRC_ERR ] : 0,
          (long) pPort3->cnCounter[ MIB_COUNTER_RX_CRC_ERR ],
          (long) phw->m_cnCounter[ 0 ][ OID_COUNTER_RCV_ERROR_CRC ],
          (long) phw->m_cnCounter[ 1 ][ OID_COUNTER_RCV_ERROR_CRC ]
                 ) ;

     DBG_PRINT( "RxAlignmentError     : %08ld    %08ld    %08ld      %08ld      %08ld    RxMIIError"NEWLINE,
          (long) pPort1->cnCounter[ MIB_COUNTER_RX_ALIGNMENT_ERR ],
                 (fPort2) ? (long) pPort2->cnCounter[ MIB_COUNTER_RX_ALIGNMENT_ERR ] : 0,
          (long) pPort3->cnCounter[ MIB_COUNTER_RX_ALIGNMENT_ERR ],
          (long) phw->m_cnCounter[ 0 ][ OID_COUNTER_RCV_ERROR_MII ],
          (long) phw->m_cnCounter[ 1 ][ OID_COUNTER_RCV_ERROR_MII ]
                 ) ;

     DBG_PRINT( "RxCtrl8808Pkts       : %08ld    %08ld    %08ld      %08ld      %08ld    RxTooLong"NEWLINE,
          (long) pPort1->cnCounter[ MIB_COUNTER_RX_CTRL_8808 ],
                 (fPort2) ? (long) pPort2->cnCounter[ MIB_COUNTER_RX_CTRL_8808 ] : 0,
          (long) pPort3->cnCounter[ MIB_COUNTER_RX_CTRL_8808 ],
          (long) phw->m_cnCounter[ 0 ][ OID_COUNTER_RCV_ERROR_TOOLONG ] ,
          (long) phw->m_cnCounter[ 1 ][ OID_COUNTER_RCV_ERROR_TOOLONG ]
                 ) ;

     DBG_PRINT( "RxPausePkts          : %08ld    %08ld    %08ld      %08ld      %08ld    RxInvalidFrame"NEWLINE,
          (long) pPort1->cnCounter[ MIB_COUNTER_RX_PAUSE ],
                 (fPort2) ? (long) pPort2->cnCounter[ MIB_COUNTER_RX_PAUSE ] : 0,
          (long) pPort3->cnCounter[ MIB_COUNTER_RX_PAUSE ],
          (long) phw->m_cnCounter[ 0 ][ OID_COUNTER_RCV_INVALID_FRAME ],
          (long) phw->m_cnCounter[ 1 ][ OID_COUNTER_RCV_INVALID_FRAME ]
                 ) ;

     DBG_PRINT( "RxBroadcastPkts      : %08ld    %08ld    %08ld      %08ld      %08ld    RxIPChecksumError"NEWLINE,
          (long) pPort1->cnCounter[ MIB_COUNTER_RX_BROADCAST ],
                 (fPort2) ? (long) pPort2->cnCounter[ MIB_COUNTER_RX_BROADCAST ] : 0,
          (long) pPort3->cnCounter[ MIB_COUNTER_RX_BROADCAST ],
          (long) phw->m_cnCounter[ 0 ][ OID_COUNTER_RCV_ERROR_IP ] ,
          (long) phw->m_cnCounter[ 1 ][ OID_COUNTER_RCV_ERROR_IP ]
                 ) ;

     DBG_PRINT( "RxMulticastPkts      : %08ld    %08ld    %08ld      %08ld      %08ld    RxTCPChecksumError"NEWLINE,
          (long) pPort1->cnCounter[ MIB_COUNTER_RX_MULTICAST ],
                 (fPort2) ? (long) pPort2->cnCounter[ MIB_COUNTER_RX_MULTICAST ] : 0,
          (long) pPort3->cnCounter[ MIB_COUNTER_RX_MULTICAST ],
          (long) phw->m_cnCounter[ 0 ][ OID_COUNTER_RCV_ERROR_TCP ] ,
          (long) phw->m_cnCounter[ 1 ][ OID_COUNTER_RCV_ERROR_TCP ]
                 ) ;

     DBG_PRINT( "RxUnicastPkts        : %08ld    %08ld    %08ld      %08ld      %08ld    RxUDPChecksumError"NEWLINE,
          (long) pPort1->cnCounter[ MIB_COUNTER_RX_UNICAST ],
                 (fPort2) ? (long) pPort2->cnCounter[ MIB_COUNTER_RX_UNICAST ] : 0,
          (long) pPort3->cnCounter[ MIB_COUNTER_RX_UNICAST ],
          (long) phw->m_cnCounter[ 0 ][ OID_COUNTER_RCV_ERROR_UDP ] ,
          (long) phw->m_cnCounter[ 1 ][ OID_COUNTER_RCV_ERROR_UDP ]
                 ) ;

     DBG_PRINT( "Rx64Octets           : %08ld    %08ld    %08ld      %08ld      %08ld    RxTotalError"NEWLINE,
          (long) pPort1->cnCounter[ MIB_COUNTER_RX_OCTET_64 ],
                 (fPort2) ? (long) pPort2->cnCounter[ MIB_COUNTER_RX_OCTET_64 ] : 0,
          (long) pPort3->cnCounter[ MIB_COUNTER_RX_OCTET_64 ],
          (long) phw->m_cnCounter[ 0 ][ OID_COUNTER_RCV_ERROR ] ,
          (long) phw->m_cnCounter[ 1 ][ OID_COUNTER_RCV_ERROR ]
                 ) ;

     DBG_PRINT( "Rx65to127Octets      : %08ld    %08ld    %08ld      %08ld      %08ld    RxErrorFrameInterrupt"NEWLINE,
          (long) pPort1->cnCounter[ MIB_COUNTER_RX_OCTET_65_127 ],
                 (fPort2) ? (long) pPort2->cnCounter[ MIB_COUNTER_RX_OCTET_65_127 ] : 0,
          (long) pPort3->cnCounter[ MIB_COUNTER_RX_OCTET_65_127 ],
          (long) phw->m_cnCounter[ 0 ][ OID_COUNTER_RCV_INT_ERROR ] ,
          (long) phw->m_cnCounter[ 1 ][ OID_COUNTER_RCV_INT_ERROR ]
                 ) ;


     DBG_PRINT( "Rx128to255Octets     : %08ld    %08ld    %08ld      %08ld      %08ld    RxOverrunInterrupt"NEWLINE,
          (long) pPort1->cnCounter[ MIB_COUNTER_RX_OCTET_128_255 ],
                 (fPort2) ? (long) pPort2->cnCounter[ MIB_COUNTER_RX_OCTET_128_255 ] : 0,
          (long) pPort3->cnCounter[ MIB_COUNTER_RX_OCTET_128_255 ],
          (long) phw->m_cnCounter[ 0 ][ OID_COUNTER_RCV_NO_BUFFER ] ,
          (long) phw->m_cnCounter[ 1 ][ OID_COUNTER_RCV_NO_BUFFER ]
                 ) ;

     DBG_PRINT( "Rx256to511Octets     : %08ld    %08ld    %08ld      %08ld      %08ld    RxStopInterrupt"NEWLINE,
          (long) pPort1->cnCounter[ MIB_COUNTER_RX_OCTET_256_511 ],
                 (fPort2) ? (long) pPort2->cnCounter[ MIB_COUNTER_RX_OCTET_256_511 ] : 0,
          (long) pPort3->cnCounter[ MIB_COUNTER_RX_OCTET_256_511 ],
          (long) phw->m_cnCounter[ 0 ][ OID_COUNTER_RCV_INT_STOP ],
          (long) phw->m_cnCounter[ 1 ][ OID_COUNTER_RCV_INT_STOP ]
                 ) ;

     DBG_PRINT( "Rx512to1023Octets    : %08ld    %08ld    %08ld      %08ld      %08ld    RxOSDroppedPkts"NEWLINE,
          (long) pPort1->cnCounter[ MIB_COUNTER_RX_OCTET_512_1023 ],
                 (fPort2) ? (long) pPort2->cnCounter[ MIB_COUNTER_RX_OCTET_512_1023 ] : 0,
          (long) pPort3->cnCounter[ MIB_COUNTER_RX_OCTET_512_1023 ],
          (long) phw->m_cnCounter[ 0 ][ OID_COUNTER_RCV_DROPPED ],
          (long) phw->m_cnCounter[ 1 ][ OID_COUNTER_RCV_DROPPED ]
                 ) ;

     DBG_PRINT( "Rx1024to1522Octets   : %08ld    %08ld    %08ld      %08ld      %08ld    RxInterrupt"NEWLINE,
          (long) pPort1->cnCounter[ MIB_COUNTER_RX_OCTET_1024_1522 ],
                 (fPort2) ? (long) pPort2->cnCounter[ MIB_COUNTER_RX_OCTET_1024_1522 ] : 0,
          (long) pPort3->cnCounter[ MIB_COUNTER_RX_OCTET_1024_1522 ],
          (long) phw->m_cnCounter[ 0 ][ OID_COUNTER_RCV_INT ] ,
          (long) phw->m_cnCounter[ 1 ][ OID_COUNTER_RCV_INT ]
                 ) ;
     DBG_PRINT( NEWLINE);

     DBG_PRINT( "TxLoPriorityByte     : %08ld    %08ld    %08ld      %08ld      %08ld    TxByte"NEWLINE,
          (long) pPort1->cnCounter[ MIB_COUNTER_TX_LO_PRIORITY ],
                 (fPort2) ? (long) pPort2->cnCounter[ MIB_COUNTER_TX_LO_PRIORITY ] : 0,
          (long) pPort3->cnCounter[ MIB_COUNTER_TX_LO_PRIORITY ],
          (long) phw->m_cnCounter[ 0 ][ OID_COUNTER_DIRECTED_BYTES_XMIT ],
          (long) phw->m_cnCounter[ 1 ][ OID_COUNTER_DIRECTED_BYTES_XMIT ]
                 ) ;

     DBG_PRINT( "TxHiPriorityByte     : %08ld    %08ld    %08ld      %08ld      %08ld    TxTotalPkts"NEWLINE,
          (long) pPort1->cnCounter[ MIB_COUNTER_TX_HI_PRIORITY ],
                 (fPort2) ? (long) pPort2->cnCounter[ MIB_COUNTER_TX_HI_PRIORITY ] : 0,
          (long) pPort3->cnCounter[ MIB_COUNTER_TX_HI_PRIORITY ],
          (long) phw->m_cnCounter[ 0 ][ OID_COUNTER_DIRECTED_FRAMES_XMIT ],
          (long) phw->m_cnCounter[ 1 ][ OID_COUNTER_DIRECTED_FRAMES_XMIT ]
                 ) ;

     DBG_PRINT( "TxPausePkts          : %08ld    %08ld    %08ld"NEWLINE,
          (long) pPort1->cnCounter[ MIB_COUNTER_TX_PAUSE ],
                 (fPort2) ? (long) pPort2->cnCounter[ MIB_COUNTER_TX_PAUSE ] : 0,
          (long) pPort3->cnCounter[ MIB_COUNTER_TX_PAUSE ]
                 ) ;

     DBG_PRINT( "TxBroadcastPkts      : %08ld    %08ld    %08ld      %08ld      %08ld    TxLateCollision"NEWLINE,
          (long) pPort1->cnCounter[ MIB_COUNTER_TX_BROADCAST ],
                 (fPort2) ? (long) pPort2->cnCounter[ MIB_COUNTER_TX_BROADCAST ] : 0,
          (long) pPort3->cnCounter[ MIB_COUNTER_TX_BROADCAST ],
          (long) phw->m_cnCounter[ 0 ][ OID_COUNTER_XMIT_LATE_COLLISION ],
          (long) phw->m_cnCounter[ 1 ][ OID_COUNTER_XMIT_LATE_COLLISION ]
                 ) ;

     DBG_PRINT( "TxMulticastPkts      : %08ld    %08ld    %08ld      %08ld      %08ld    TxMaximumCollision"NEWLINE,
          (long) pPort1->cnCounter[ MIB_COUNTER_TX_MULTICAST ],
                 (fPort2) ? (long) pPort2->cnCounter[ MIB_COUNTER_TX_MULTICAST ] : 0,
          (long) pPort3->cnCounter[ MIB_COUNTER_TX_MULTICAST ],
          (long) phw->m_cnCounter[ 0 ][ OID_COUNTER_XMIT_MORE_COLLISIONS ],
          (long) phw->m_cnCounter[ 1 ][ OID_COUNTER_XMIT_MORE_COLLISIONS ]
                 ) ;

     DBG_PRINT( "TxUnicastPkts        : %08ld    %08ld    %08ld      %08ld      %08ld    TxUnderrun"NEWLINE,
          (long) pPort1->cnCounter[ MIB_COUNTER_TX_UNICAST ],
                 (fPort2) ? (long) pPort2->cnCounter[ MIB_COUNTER_TX_UNICAST ] : 0,
          (long) pPort3->cnCounter[ MIB_COUNTER_TX_UNICAST ],
          (long) phw->m_cnCounter[ 0 ][ OID_COUNTER_XMIT_UNDERRUN ],
          (long) phw->m_cnCounter[ 1 ][ OID_COUNTER_XMIT_UNDERRUN ]
                 ) ;

     DBG_PRINT( "TxLateCollision      : %08ld    %08ld    %08ld      %08ld      %08ld    TxTotalError"NEWLINE,
          (long) pPort1->cnCounter[ MIB_COUNTER_TX_LATE_COLLISION ],
                 (fPort2) ? (long) pPort2->cnCounter[ MIB_COUNTER_TX_LATE_COLLISION ] : 0,
          (long) pPort3->cnCounter[ MIB_COUNTER_TX_LATE_COLLISION ],
          (long) phw->m_cnCounter[ 0 ][ OID_COUNTER_XMIT_ERROR ],
          (long) phw->m_cnCounter[ 1 ][ OID_COUNTER_XMIT_ERROR ]
                 ) ;

     DBG_PRINT( "TxDeferred           : %08ld    %08ld    %08ld      %08ld      %08ld    TxAllocMemFail"NEWLINE,
          (long) pPort1->cnCounter[ MIB_COUNTER_TX_DEFERRED ],
                 (fPort2) ? (long) pPort2->cnCounter[ MIB_COUNTER_TX_DEFERRED ] : 0,
          (long) pPort3->cnCounter[ MIB_COUNTER_TX_DEFERRED ],
          (long) phw->m_cnCounter[ 0 ][ OID_COUNTER_XMIT_ALLOC_FAIL ],
          (long) phw->m_cnCounter[ 1 ][ OID_COUNTER_XMIT_ALLOC_FAIL ]
                 ) ;

     DBG_PRINT( "TxTotalCollision     : %08ld    %08ld    %08ld      %08ld      %08ld    TxStopInterrupt"NEWLINE,
          (long) pPort1->cnCounter[ MIB_COUNTER_TX_TOTAL_COLLISION ],
                 (fPort2) ? (long) pPort2->cnCounter[ MIB_COUNTER_TX_TOTAL_COLLISION ] : 0,
          (long) pPort3->cnCounter[ MIB_COUNTER_TX_TOTAL_COLLISION ],
          (long) phw->m_cnCounter[ 0 ][ OID_COUNTER_XMIT_INT_STOP ],
          (long) phw->m_cnCounter[ 1 ][ OID_COUNTER_XMIT_INT_STOP ]
                 ) ;

     DBG_PRINT( "TxExcessiveCollision : %08ld    %08ld    %08ld      %08ld      %08ld    TxOSDroppedPkts"NEWLINE,
          (long) pPort1->cnCounter[ MIB_COUNTER_TX_EXCESS_COLLISION ],
                 (fPort2) ? (long) pPort2->cnCounter[ MIB_COUNTER_TX_EXCESS_COLLISION ] : 0,
          (long) pPort3->cnCounter[ MIB_COUNTER_TX_EXCESS_COLLISION ],
          (long) phw->m_cnCounter[ 0 ][ OID_COUNTER_XMIT_DROPPED ],
          (long) phw->m_cnCounter[ 1 ][ OID_COUNTER_XMIT_DROPPED ]
                 ) ;

     DBG_PRINT( "TxSingleCollision    : %08ld    %08ld    %08ld      %08ld      %08ld    TxUnderrunInterrupt"NEWLINE,
          (long) pPort1->cnCounter[ MIB_COUNTER_TX_SINGLE_COLLISION ],
                 (fPort2) ? (long) pPort2->cnCounter[ MIB_COUNTER_TX_SINGLE_COLLISION ] : 0,
          (long) pPort3->cnCounter[ MIB_COUNTER_TX_SINGLE_COLLISION ],
          (long) phw->m_cnCounter[ 0 ][ OID_COUNTER_XMIT_INT_UNDERRUN ],
          (long) phw->m_cnCounter[ 1 ][ OID_COUNTER_XMIT_INT_UNDERRUN ]
                 ) ;

     DBG_PRINT( "TxMultiCollision     : %08ld    %08ld    %08ld      %08ld      %08ld    TxDoneInterrupt"NEWLINE,
          (long) pPort1->cnCounter[ MIB_COUNTER_TX_MULTI_COLLISION ],
                 (fPort2) ? (long) pPort2->cnCounter[ MIB_COUNTER_TX_MULTI_COLLISION ] : 0,
          (long) pPort3->cnCounter[ MIB_COUNTER_TX_MULTI_COLLISION ],
          (long) phw->m_cnCounter[ 0 ][ OID_COUNTER_XMIT_INT ],
          (long) phw->m_cnCounter[ 1 ][ OID_COUNTER_XMIT_INT ]
                 ) ;
     DBG_PRINT( NEWLINE);


     DBG_PRINT( "RxDroppedPackets     : %08ld    %08ld    %08ld"NEWLINE,
          (long) pPort1->cnCounter[ MIB_COUNTER_RX_DROPPED_PACKET ],
                 (fPort2) ? (long) pPort2->cnCounter[ MIB_COUNTER_RX_DROPPED_PACKET ] : 0,
          (long) pPort3->cnCounter[ MIB_COUNTER_RX_DROPPED_PACKET ]
                 ) ;

     DBG_PRINT( "TxDroppedPackets     : %08ld    %08ld    %08ld"NEWLINE,
          (long) pPort1->cnCounter[ MIB_COUNTER_TX_DROPPED_PACKET ],
                 (fPort2) ? (long) pPort2->cnCounter[ MIB_COUNTER_TX_DROPPED_PACKET ] : 0,
          (long) pPort3->cnCounter[ MIB_COUNTER_TX_DROPPED_PACKET ]
                 ) ;

}



/*****************************************************************/
static void DisplayErrRetMsg(unsigned long hwReturn)
{
   char FAR * pbMsg ;

   /*
    * Following must match definitions in hw.h
    */

   #define HWRETURN_SUCCESS                0
   #define HWRETURN_ERROR_BAD_PARAMETER    1
   #define HWRETURN_ERROR_BUSY             2
   #define HWRETURN_ERROR_FAIL             3
   #define HWRETURN_ERROR_TIMEOUT          4
   #define HWRETURN_ERROR_OS               5
   #define HWRETURN_ERROR_INTERNAL         6
   #define HWRETURN_ERROR_NO_MATCH         7
   #define HWRETURN_ERROR_NOT_OWNED        8
   #define HWRETURN_ERROR_TABLE_FULL       9
   #define HWRETURN_ERROR_TABLE_READ      10

   switch (hwReturn)
   {
    case HWRETURN_SUCCESS:
        pbMsg = "Success" ;
        break ;

    case HWRETURN_ERROR_BAD_PARAMETER:
        pbMsg = "Bad parameter" ;
        break ;

    case HWRETURN_ERROR_BUSY:
        pbMsg = "Busy" ;
        break ;

    case HWRETURN_ERROR_FAIL:
        pbMsg = "Fail" ;
        break ;

    case HWRETURN_ERROR_TIMEOUT:
        pbMsg = "Time out" ;
        break ;

    case HWRETURN_ERROR_OS:
        pbMsg = "OS error" ;
        break ;

    case HWRETURN_ERROR_INTERNAL:
        pbMsg = "Internal error" ;
        break ;

    case HWRETURN_ERROR_NO_MATCH:
        pbMsg = "No match" ;
        break ;

    case HWRETURN_ERROR_NOT_OWNED:
        pbMsg = "Not owned" ;
        break ;

    case HWRETURN_ERROR_TABLE_FULL:
        pbMsg = "Table full" ;
        break ;

    case HWRETURN_ERROR_TABLE_READ:
        pbMsg = "Table ready only" ;
        break ;

    default:
        pbMsg = "Internal error" ;
        break ;
   }

   DBG_PRINT( MSG_ERROR_PREFIX "%s"NEWLINE, pbMsg );
}

/*****************************************************************/
static unsigned char  isValidBank
(
    unsigned long bankNum
)
{
    if ( bankNum > MAX_BANK_NUM )
    {
        DisplayErrRetMsg( 1 ) ;
        return FALSE;
    }
    return TRUE;
}

/*****************************************************************/
static unsigned char  isValidRegAddr
(
    unsigned long regAddr
)
{
    if ( regAddr > MAX_REGADDR_OFFSET )
    {
        DisplayErrRetMsg( 1 ) ;
        return FALSE;
    }
    return TRUE;
}

/*--------------------------------------------------------------------------
 *
 *  STATUS TableNameXlat
 *
 *  Description:
 *      A caller uses this function to translate a hardware switch table
 *      name to the equivalent hardware switch table identifier.
 *
 *  Parameters:
 *      PBYTE pbTableName
 *         Name of hardware switch table.
 *
 *      PDWORD pdwTableID
 *         Address of DWORD to contain hardware switch table identifier.
 *
 *  Return (STATUS):
 *      OK if successful, ERROR otherwise.
 *
 *--------------------------------------------------------------------------
 */

static int TableNameXlat
(
   char FAR   *pbTableName,
   unsigned long      *pdwTableID
)
{
   typedef struct tagNAMEXLAT
   {
      char FAR      *pbTableName ;
      unsigned long  dwTableID ;

   } NAMEXLAT ;

   NAMEXLAT asNameXlat[] =
   {
      { "mac1"   , TABLE_STATIC_MAC  },
      { "mac2"   , TABLE_DYNAMIC_MAC },
      { "vlan"   , TABLE_VLAN        },
      { "mib"    , TABLE_MIB         },
      { 0,         0                 }
   } ;

   int i = 0 ;

   /*
    * Validate pointer
    */

   if (!pbTableName)
   {
      return FALSE ;
   }

   /*
    * Validate pointer
    */

   if (!pdwTableID)
   {
      return FALSE ;
   }

   /*
    * Translate name to identifier
    */

   while (asNameXlat[ i ].pbTableName)
   {
         if (!strcmp( (char FAR *)asNameXlat[ i ].pbTableName,
                      (char FAR *)pbTableName ))
         {
            *pdwTableID = asNameXlat[ i ].dwTableID ;
            return TRUE ;
         }

         i++ ;
   }

   return FALSE ;

} /* TableNameXlat() */


/*****************************************************************/
static unsigned char BufferInit(void)
{
   int iBufNum ;

   /*
    * Check if buffer has been allocated
    */


   if (apbBuffer[ 0 ])
   {
      return TRUE ;
   }

   for ( iBufNum = 0 ;
         iBufNum < BUFFER_COUNT ;
         iBufNum++ )
   {
       /*
        * Attempt to allocate buffer
        */

       /* apbBuffer[ iBufNum ] = (unsigned char FAR *) malloc( BUFFER_LENGTH ) ; */
       apbBuffer[ iBufNum ] = (unsigned char FAR *) &packetBuf[ iBufNum ][0];

       if (apbBuffer[ iBufNum ])
       {
          int i ;

          /*
           * Initialize the buffer to contain all zeros
           */

          for ( i = 0 ;
                i < BUFFER_LENGTH ;
                i++ )
          {
              apbBuffer[ iBufNum ][ i ] = 0 ;
          }
       }
       else
       {
          /*
           * Failed to allocate a buffer so free all buffers allocated
           */

          while ( iBufNum >= 0 )
          {
                if (apbBuffer[ iBufNum ])
                {
                   /* free( apbBuffer[ iBufNum ] ) ; */
                }

                iBufNum-- ;
          }

          DBG_PRINT( MSG_ERROR_PREFIX "Buffer allocation "NEWLINE ) ;

          return FALSE ;
       }
   }

   return TRUE ;
}


#if !defined( _WIN32 )  &&  !defined( DEF_LINUX ) && !defined( _EZ80L92 )  &&  !defined( KS_ARM )
/*****************************************************************************/
static int strParse
(
	UINT8 FAR *str,
	UINT8 FAR *pattern,
	UINT8 FAR **argz,
	int max_argz
)
{
#if !defined( _WIN32 )  &&  !defined( DEF_LINUX ) && !defined( _EZ80L92 )  &&  !defined( KS_ARM )
    UINT8 FAR *pt1, *pt = str;
#endif
    int count = 0;

    max_argz--;                        /* save room for null, to mark end */
                                       /* of the pointer arrary */

    argz[count] = (UINT8 *)0;        /* set to null just in case we do */
                                       /* not find any string tokens */

    /* loop until we have completly tokenized the string. */
    /* strtok is a UNIX stdio library call. */

#if !defined( _WIN32 )  &&  !defined( DEF_LINUX ) && !defined( _EZ80L92 )  &&  !defined( KS_ARM )
    while( (pt1 = strtok((char FAR *) pt, (char FAR *) pattern)) != (UINT8 FAR *)0 )
    {
        argz[count++] = pt1;           /* found a token save it's address */
        pt = (UINT8 *)0;             /* set to null so that strtok function */
                                       /* will continue to parse the same line */

        argz[count] = (UINT8 *)0;    /* set next pointer to null */

        if( count >= max_argz )        /* break wheb we have reached count */
            break;
    }
#endif

    return(count);                     /* return number of tokens found */
}
#endif

/*****************************************************************************
*
* strToMac
*
* DESCRIPTION
* This routine converts a string of the form mm:mm:mm:mm:mm:mm to a MAC address
*
*  Parameters:
*
*      unsigned char * str
*           pointer to the buffer containing the string.
*
*      unsigned char * mac
*           return the converted string.
*/
static void strToMac
(
    unsigned char FAR * str,
    unsigned char FAR * mac
)
{
#if !defined( _WIN32 )  &&  !defined( DEF_LINUX ) && !defined( _EZ80L92 )  &&  !defined( KS_ARM )
    int i, n, x;
    UINT8 FAR *args[10];
    UINT8 *p;
    UINT8 tmp[32];
    UINT8 nn[3];

    strcpy(tmp, str);
    memset(mac, 0, 6);

    n = strParse((UINT8 FAR *)tmp, (UINT8 FAR *)":", (UINT8 FAR **)args, (int)10);

    /* check and see if use typed MAC address without colons */
    if (n == 1)
    {
        if (strlen(args[0]) < 12)
            return;

        for (i = 0, x = 0; i < 12;)
        {
            nn[0] = args[0][i++];
            nn[1] = args[0][i++];
            nn[2] = '\0';
            mac[x++] = (unsigned char FAR) strtol((char FAR * ) nn, (char FAR **) & p, (int)16);
        }
    }
    else
    {
        if (n != 6)            /* if user did use colons then is MAC complete? */
            return;

        for (i = 0; i < n; i++)
            mac[i] = (unsigned char FAR) strtol((char FAR * ) args[i], (char FAR **) & p, (int)16);
    }
#endif
}

/*******************************************************************************
*
* strToIp
*
* DESCRIPTION
* This routine converts a string of the form mmm.mmm.mmm.mmm to a IP address

* RETURNS
* Network order in and returns host order.
*
* NOMANUAL
*/

static void strToIp
(
    unsigned char FAR * str,
    unsigned char FAR * ip
)
{
#if !defined( _WIN32 )  &&  !defined( DEF_LINUX ) && !defined( _EZ80L92 )  &&  !defined( KS_ARM )
    int i, n, x;
    UINT8 FAR *args[10];
    UINT8 *p;
    UINT8 tmp[32];
    UINT8 nn[3];

    strcpy(tmp, str);
    memset(ip, 0, 4);

    n = strParse((UINT8 FAR *)tmp, (UINT8 FAR *)".", (UINT8 FAR **)args, (int)10);

    /* check and see if use typed MAC address without colons */
    if (n == 1)
    {
        if (strlen(args[0]) < 12)
            return;

        for (i = 0, x = 0; i < 12;)
        {
            nn[0] = args[0][i++];
            nn[1] = args[0][i++];
            nn[2] = '\0';
            ip[x++] = (unsigned char FAR) strtol((char FAR * ) nn, NULL, (int)10);
        }
    }
    else
    {
        if (n != 4)            /* if user did use . then is IP complete? */
            return;

        for (i = 0; i < n; i++)
            ip[i] = (unsigned char FAR) strtol((char FAR * ) args[i], NULL, (int)10);
    }
#endif
}

/*---------------------------------------------------------------------------*/

#ifdef KS_QC_TEST

/*-----------------------------------------------------------------------------
 *  This section are for FAB QC testing only
 *  The DUT is KS8841 or KS8842 on M16C generic 16bit bus using
 *  QC test board.
 *  All the CLI are issued from QC test program running on PC Windows 98
 *  through RS-232 serial port (COM1 or COM2) with baud rate (38400 8-n-1).
 */


/******************************************************************************
*
* Command: qctxtest
*
* Format: "qctxtest Port RepeatCount"
*
* Description:
*   This CLI transmit 3 internal buffers repeated by 'RepeatCount' to the
*   device port 'Port'.
*   Buffers are filled  with following broadcast packets:
*   buffer 0: all 0, 64 in length
*   buffer 1: all 1, 128 in length
*   buffer 2: increment by 1, 256 in length
*   buffer 3: increment by 2, 512 in length
*   buffer 4: increment by 4, 1024 in length
*   buffer 5: increment by 8, 1514 in length
*
******************************************************************************/
int qctxtest
(
   unsigned long   Port,
   unsigned long   RepeatCount
)
{
   unsigned long   BufNum;



   /*
    * Validate parameters
    */

   if ( Port > 3 )
   {
      DisplayErrRetMsg( 1 ) ;
      return FALSE ;
   }

   /*
    * Prepare the buffers with broadcast packet.
    */

   /* Write DA, SA to all the buffers */

   for ( BufNum=0; BufNum < 3; BufNum++ )
   {
       hwbufwrite ( BufNum, 0, (unsigned char FAR *)"ff ff ff ff ff ff 08 00 70 22 44 55" );
   }

   /* Fill the buffers with different test pattern data */

   hwbuffill (0, 12, 0, 0, 64 );    /* for bufer 0 */
   hwbuffill (1, 12, 1, 0, 128 );   /* for bufer 1 */
   hwbuffill (2, 12, 1, 1, 256 );   /* for bufer 2 */

#if (0)
   hwbuffill (3, 12, 0, 2, 1514 );   /* for bufer 3 */
   hwbuffill (4, 12, 0, 4, 1514 );   /* for bufer 4 */
   hwbuffill (5, 12, 0, 8, 1514 );   /* for bufer 5 */
#endif


   /*
    * Transmit the buffer data
    */



   while (RepeatCount > 0)
   {
      if (RepeatCount == 0)
          break;
#ifdef M16C_62P
      hwbuftx_M16C ( Port, 0, 60, 1, 1, 0 );
#endif /* M16C_62P */

#ifdef DEF_VXWORKS
      hwbuftx_vxWorks ( Port, 0, 60, 1, 1, 0 );
#endif /* DEF_VXWORKS */

      RepeatCount--;

      if (RepeatCount == 0)
          break;
#ifdef M16C_62P
      hwbuftx_M16C ( Port, 1, 124, 1, 1, 0 );
#endif /* M16C_62P */

#ifdef DEF_VXWORKS
      hwbuftx_vxWorks ( Port, 1, 124, 1, 1, 0 );
#endif /* DEF_VXWORKS */

      RepeatCount--;

      if (RepeatCount == 0)
          break;
#ifdef M16C_62P
      hwbuftx_M16C ( Port, 2, 252, 1, 1, 0 );
#endif /* M16C_62P */

#ifdef DEF_VXWORKS
      hwbuftx_vxWorks ( Port, 2, 252, 1, 1, 0 );
#endif /* DEF_VXWORKS */

      RepeatCount--;

#if (0)
      if (RepeatCount == 0)
          break;
      hwbuftx_M16C ( Port, 3, 508, 1, 1, 0 );
      RepeatCount--;

      if (RepeatCount == 0)
          break;
      hwbuftx_M16C ( Port, 4, 1020, 1, 1, 0 );
      RepeatCount--;

      if (RepeatCount == 0)
          break;
      hwbuftx_M16C ( Port, 5, 1510, 1, 1, 0 );
      RepeatCount--;
#endif
   }


   DBG_PRINT ( "^[ok]$"NEWLINE );
   return TRUE ;
}

/******************************************************************************
*
* Command: qcgetmib
*
* Format: "qcgetmib Port MibIndex"
*
* Description:
*   This CLI gets device MIB counter specific by MIB index 'mibIndex'
*   on specific port 'Port'.
*
******************************************************************************/
int qcgetmib
(
   unsigned long   Port,
   unsigned long   MibIndex
)
{
#ifdef DEF_VXWORKS
   PHARDWARE       phw = gpDrvCtrl[0]->pHW;
#endif
   PPORT_CONFIG pPort;

ACT_DEBUG_


   /*
    * Validate parameters
    */

   if ( ( Port < 1 ) || ( Port > TOTAL_PORT_NUM ) )
   {
      DisplayErrRetMsg( 1 ) ;
      return FALSE ;
   }

   if ( MibIndex >= TOTAL_PORT_COUNTER_NUM )
   {
      DisplayErrRetMsg( 1 ) ;
      return FALSE ;
   }


   /*
    * Get Device MIB counter
    */

   hwtableshow ( "mib", 0, 1, FALSE );

   /*
    * Display Port's MIB counter
    */

   Port--;
   pPort = &phw->m_Port[ Port ];

   DBG_PRINT ( "^[%08lx]$"NEWLINE, pPort->cnCounter[ MibIndex ]);

   return TRUE ;
}

/******************************************************************************
*
* Command: qcgetcable_cmd
*
* Format: "qcgetcable  Port"
*
* Description:
*   This CLI gets device LinkMD cable status on specific port 'Port'.
*
******************************************************************************/
int qcgetcable
(
   unsigned long   Port
)
{
#ifdef DEF_VXWORKS
   PHARDWARE       phw = gpDrvCtrl[0]->pHW;
#endif
   ULONG    *pdwBuf;

   /*
    * Validate parameters
    */

   if ( ( Port < 1 ) || ( Port > TOTAL_PORT_NUM ) )
   {
      DisplayErrRetMsg( 1 ) ;
      return FALSE ;
   }

   /*
    * Get Device LinkMD cable status
    */

   HardwareGetCableStatus ( phw, (Port-1), tempBuf );

   /*
    * Display Port's cable status
    */

   memset ( tempStrBuf, 0, (LINKMD_INFO_COUNT + 1) * sizeof (unsigned long) );

   pdwBuf = (ULONG *)&tempBuf[1];
   DBG_PRINT ( "^[%08lx]$"NEWLINE, *pdwBuf );

   return TRUE ;
}
#endif /* #ifdef KS_QC_TEST */

/******************************************************************************
*
* Command: qcecho
*
* Format: "qcecho Flag"
*
* Description:
*   If Flag is TRUE, echo received character from serial port to VT100 screen.
*   otherwise, don't echo character.
*
******************************************************************************/
int qcecho
(
   unsigned long   Flag
)
{

   /*
    * Validate parameters
    */

   if ( Flag > 1 )
       return FALSE;

   /*
    * Set echo Flag to global varible fEcho
    */

#ifdef M16C_62P
   fEcho = Flag;
#endif

   DBG_PRINT ( "^[ok]$"NEWLINE );
   return TRUE ;
}

