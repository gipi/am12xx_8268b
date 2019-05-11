#ifndef __AM7X_NAND_REGS
#define __AM7X_NAND_REGS

/***  Nand Flash Block Base Address ***/ /*What meaning?*/
#define NAND_IOMAP_BASE             (0xA0000000 + 0x100A0000)
#define NAND_IOMAP_PHY_BASE         (0x100A0000)    /* get this at runtime? */
#define NAND_IOMAP_SIZE             (64*1024)

//--------FLASH I/F--------------------------------------------------
#define  NAND_CTL             0x000
#define  NAND_STATUS          0x004
#define  NAND_CLKCTL          0x008
#define  NAND_SPARE_COLADDR	  0x00c	
#define  NAND_MAIN_COLADDR	  0x010
#define  NAND_ROWADDRLO       0x014
#define  NAND_ROWADDRHI       0x018
#define  NAND_BC              0x01C
#if CONFIG_AM_CHIP_ID==1203
#define  NAND_UDATA           0x020
#elif CONFIG_AM_CHIP_ID==1211 || CONFIG_AM_CHIP_ID == 1220 || (CONFIG_AM_CHIP_ID == 1213)
#define NAND_PAGE_CTL      0x20
#define NAND_UDATA             0x90
#define NAND_UDATA1           0x94
#define NAND_UDATA2           0x98
#define NAND_UDATA3           0x9c
#define NAND_UDATA4           0xa0
#define NAND_UDATA5           0xa4
#define NAND_UDATA6           0xa8
#define NAND_UDATA7           0xac
#define NAND_BIT_MAP         0xb0
#define NAND_RB_CTL           0xB4
#define NAND_MBIST_CTL     0xB8
#endif
#define  NAND_CMD_FSM         0x024
#define  NAND_DATA            0x028
#define  NAND_ECCCTL          0x02C
#if CONFIG_AM_CHIP_ID == 1220 || (CONFIG_AM_CHIP_ID == 1213)
#define  NAND_ECCEX0          0x130
#define  NAND_ECCEX1          0x134
#define  NAND_ECCEX2          0x138
#define  NAND_ECCEX3          0x13C
#define  NAND_ECCEX4          0x140
#define  NAND_ECCEX5          0x144
#define  NAND_ECCEX6          0x148
#define  NAND_ECCEX7          0x14C
#else
#define  NAND_ECCEX0          0x030
#define  NAND_ECCEX1          0x034
#define  NAND_ECCEX2          0x038
#define  NAND_ECCEX3          0x03C
#define  NAND_ECCEX4          0x040
#define  NAND_ECCEX5          0x044
#define  NAND_ECCEX6          0x048
#define  NAND_ECCEX7          0x04C
#endif

/* Alias */
#define NAND_BYTECNT          (0x001C)
#define NAND_CMD              (0x0024)
#define NAND_FIFODATA         (0x0028)


/*----------------------------------------------------
 AL1203 for 7531 
 MF1  NF_D0~D7
 MF2 NF_CLE,NF_WE,NF_ALE,NF_RE,NF_RB,NF_CS2,NF_CS1
------------------------------------------------------*/
#define NF_MF1_MSK_7531   ~((3<<28))
#define NF_MF1_VAL_7531    ((0<<28))
#define NF_MF2_MSK_7531   ~((7<<0)|(3<<4)|(3<<8)|(3<<12)|(3<<16)|(3<<20)|(3<<24))
#define NF_MF2_VAL_7531     ((1<<0)|(1<<4)|(1<<8)|(1<<12)|(1<<16)|(1<<20)|(1<<24))
#define NF_MF2_MSK_7531_1CE   ~((7<<0)|(3<<4)|(3<<8)|(3<<12)|(3<<16)|(3<<24))
#define NF_MF2_VAL_7531_1CE     ((1<<0)|(1<<4)|(1<<8)|(1<<12)|(1<<16)|(1<<24))

/*----------------------------------------------------
 AL1203 for 7331 
 MF2  NF_CS1,NF_CS1
 MF3 NF_CLE,NF_WE,NF_ALE,NF_RE,NF_RB,NF_D0~D7
------------------------------------------------------*/
/*----------------------------------------------------
 AL1203 for 7331 
 MF2  NF_CS1,NF_CS1
 MF3 NF_CLE,NF_WE,NF_ALE,NF_RE,NF_RB,NF_D0~D7
------------------------------------------------------*/
#define NF_MF2_MSK_7331   ~((3<<20)|(7<<24)|(3<<16)|(7<<28))
#define NF_MF2_VAL_7331     ((1<<20)|(1<<24)|(1<<16)|(3<<28))
//#define NF_MF2_MSK_7331_1CE   ~((7<<24)|(3<<16)|(7<<28))
//#define NF_MF2_VAL_7331_1CE     ((1<<24)|(1<<16)|(3<<28))
#define NF_MF2_MSK_7331_1CE   ~((7<<24)|(7<<28))
#define NF_MF2_VAL_7331_1CE     ((1<<24)|(3<<28))
//#define NF_MF3_MSK_7331   ~((7<<0)|(7<<4)|(7<<8)|(7<<12)|(15<<16))
//#define NF_MF3_VAL_7331     ((3<<0)|(3<<4)|(2<<8)|(6<<12)|(5<<16))
/**/
#define NF_MF3_MSK_7331   ~((7<<0)|(7<<4)|(7<<8)|(7<<12)|(15<<16)|(7<<29))
#define NF_MF3_VAL_7331     ((3<<0)|(3<<4)|(2<<8)|(6<<12)|(5<<16)|(6<<29))
/*==================================================================================*/
/*
MF3:  NF_CS1[21:20],NF_RB[25:24],NF_RE[30:28],NF_CS2[10:8]
MF4:  NF_WE[1:0], NF_CLE[6:4],NFALE[10:8],NF_D7,D6[23:20],
NF_D5[27:24],NF_D4[31:28],
MF5: NF_D3,D2[2:0],NF_D1[6:4],NF_D0[10:8]
*/
#define NF_MF3_MSK_7555  ~((7<<8)|(3<<20)|(3<<24)|(7<<28))  
#define NF_MF3_VAL_7555     ((5<<8)|(1<<20)|(1<<24)|(1<<28))  
#define NF_MF3_MSK_7555_1CE  ~((3<<20)|(3<<24)|(7<<28))  
#define NF_MF3_VAL_7555_1CE     ((1<<20)|(1<<24)|(1<<28))  
#define NF_MF4_MSK_7555    ~((3<<0)|(7<<4)|(7<<8)|(0xF<<20)|(0xF<<24)|(0xF<<28))
#define NF_MF4_VAL_7555       ((1<<0)|(1<<4)|(1<<8)|(0x1<<20)|(0x1<<24)|(0x1<<28))
#define NF_MF5_MSK_7555  ~((7<<0)|(7<<4)|(7<<8))
#define NF_MF5_VAL_7555      ((1<<0)|(1<<4)|(1<<8)) 

#define NF_MF2_MSK_8250  ~((7<<22)|(1<<25)|(7<<26)|(3<<29))  
#define NF_MF2_VAL_8250     ((3<<22)|(1<<25)|(1<<26)|(1<<29))  

#define NF_MF2_MSK_8250_1CE  ~((7<<22)|(1<<25)) 
#define NF_MF2_VAL_8250_1CE     ((3<<22)|(1<<25))  
#define NF_MF2_MSK_8250_2CE  ~((7<<22)|(1<<25)|(7<<26))  
#define NF_MF2_VAL_8250_2CE     ((3<<22)|(1<<25)|(1<<26)) 

#define NF_MF3_MSK_8250    ~((3<<0)|(7<<2)|(3<<5)|(7<<7))
#define NF_MF3_VAL_8250       ((1<<0)|(1<<2)|(1<<5)|(1<<7))
#define NF_MF3_MSK_8250_2CE    ~((7<<2)|(3<<5)|(7<<7))
#define NF_MF3_VAL_8250_2CE      ((1<<2)|(1<<5)|(1<<7)) 

#define FlashDMA_R_SDR_32   (2<<1)|(20<<3)|(1<<8)|(0<<13)|(2<<17)|(16<<19)|(0<<24)|(3<<29)
#define FlashDMA_R_SDR_8    (2<<1)|(20<<3)|(1<<8)|(0<<13)|(0<<17)|(16<<19)|(0<<24)|(3<<29)

#define FlashDMA_W_SDR_32   (2<<1)|(16<<3)|(0<<8)|(3<<13)|(2<<17)|(20<<19)|(1<<24)|(0<<29)
#define FlashDMA_W_SDR_8     (0<<1)|(16<<3)|(0<<8)|(3<<13)|(2<<17)|(20<<19)|(1<<24)|(0<<29)



#endif

