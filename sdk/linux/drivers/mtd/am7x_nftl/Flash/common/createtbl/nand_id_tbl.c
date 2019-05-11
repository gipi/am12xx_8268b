/*
*********************************************************************************************************
*                                       NAND FLASH DRIVER MODULE
*                               (c) Copyright 2007, Actions Co,Ld.
*                                          All Right Reserved
* File    : nand_id_tbl.c
* By      : mengzh
* Version : V0.1
* Date    : 2007-7-27 16:25
*********************************************************************************************************
*/

#include "nand_flash_driver_type.h"


/* for samsung nand flash */
struct SpecialCmdType SpecialType_Samsung_SLC_512 =
    { {0x11, 0x80}, {0x00, 0x00, 0x35}, {0x85, 0x11, 0x81}, 1, 0x01, 0x70, 0xf1, 0xf2 ,{0,0}};
struct SpecialCmdType SpecialType_Samsung_SLC_2K =
    { {0x11, 0x81}, {0x00, 0x00, 0x35}, {0x85, 0x11, 0x81}, 1, 0x01, 0x70, 0xf1, 0xf2,{0,0} };
struct SpecialCmdType SpecialType_Samsung_SLC_4K =
    { {0x11, 0x81}, {0x60, 0x60, 0x35}, {0x85, 0x11, 0x81}, 1, 0x01, 0x70, 0xf1, 0xf2,{0,0} };
struct SpecialCmdType SpecialType_Samsung_MLC_2K =
    { {0x11, 0x81}, {0x00, 0x00, 0x35}, {0x85, 0x11, 0x81}, 1, 0x02, 0x70, 0xf1, 0xf2,{0,0} };
struct SpecialCmdType SpecialType_Samsung_MLC_4K =
    { {0x11, 0x81}, {0x60, 0x60, 0x35}, {0x85, 0x11, 0x81}, 1, 0x02, 0x70, 0xf1, 0xf2,{0,0} };

/* for micron nand flash */
struct SpecialCmdType SpecialType_Micron =
    { {0x11, 0x80}, {0x00, 0x00, 0x35}, {0x85, 0x11, 0x80}, 1, 0x01, 0x70, 0x78, 0x78,{0,0} };

struct SpecialCmdType SpecialType_Micron_8K =
    { {0x11, 0x80}, {0x00, 0x00, 0x35}, {0x85, 0x11, 0x80}, 0, 0x01, 0x70, 0x78, 0x78,{0,0} };

/* for toshiba nand flash */
struct SpecialCmdType SpecialType_Toshiba_SLC_2K =
    { {0x11, 0x80}, {0x00, 0x00, 0x30}, {0x8c, 0x11, 0x8c}, 0, 0x00, 0x71, 0x70, 0x70 ,{0,0}};

struct SpecialCmdType SpecialType_Toshiba_MLC_2K_1024 =
    { {0x11, 0x80}, {0x00, 0x00, 0x30}, {0x8c, 0x11, 0x8c}, 1024, 0x02, 0x71, 0x70, 0x70,{0,0} };

struct SpecialCmdType SpecialType_Toshiba_MLC_2K_2048 =
    { {0x11, 0x80}, {0x00, 0x00, 0x30}, {0x8c, 0x11, 0x8c}, 2048, 0x02, 0x71, 0x70, 0x70,{0,0} };

struct SpecialCmdType SpecialType_Toshiba_MLC_8K =
    { {0x11, 0x80}, {0x00, 0x00, 0x30}, {0x8c, 0x11, 0x8c}, 1, 0x02, 0x71, 0x70, 0x70,{0,0} };
struct SpecialCmdType SpecialType_Spansion_SLC_2K =
    { {0x11, 0x81}, {0x00, 0x00, 0x35}, {0x85, 0x11, 0x81}, 1024, 0x01, 0x78, 0xf1, 0xf2,{0,0} };

/*==================================================================*/
/*==================SAMSUNG NAND FLASH ID TABLE=====================*/
/*==================================================================*/
struct NandDevPar   SamsungNandTbl[] =
{
    /*       NAND_ID         DieCnt  SecCnt  PagCnt  Freq    BlkCnt  DatBlk   OpOpt     SpeCmd */

	/* SLC */
	{ {0xec, 0xf1, 0xff, 0x15}, 1,     4,      64,    15,     1024,   984,   0x0010, 15,0x06, 64,&SpecialType_Samsung_SLC_2K },   /* K9F1G08 */
	{ {0xec, 0xda, 0xff, 0x15}, 2,     4,      64,    15,     1024,   984,   0x0000, 15,0x06, 64,&SpecialType_Samsung_SLC_2K },   /* K9K2G08 */  /* test pass */
	/*@fish add 2008-10-27 
	add K9F1G08U0A Flash ID:0xEC,0xF1,0x80,0x15
	add K9F1G08U0B Flash ID:0xEC,0xF1,0x00,0x95,0x40  0x10 CopyBack
	add K9F2G08U0A Flash ID:0xEC,0xDA,0x10,0x95,0x44  0x14 Copyback+twoPlaneWrite
	Cache Program Not Support 
	*/
	{ {0xec, 0xf1, 0x00, 0x95}, 1,     4,      64,    15,     1024,   984,   0x0010, 15, 0x06,64,&SpecialType_Samsung_SLC_2K },   /* K9F1G08U0B */

	/*
	* K9F2G08U0M: don't support two-plane program, support cache program
	* K9F2G08U0A: support two-plane program, don't support cache program
	*/
	{ {0xec, 0xda, 0x10, 0x95}, 1,     4,      64,    15,     2048,   984,   0x0010,  15, 0x06, 64,&SpecialType_Samsung_SLC_2K },   /* K9F2G08U0C */	
	{ {0xec, 0xda, 0x10, 0x15}, 1,     4,      64,    15,     2048,   984,   0x0010,  15, 0x06, 64,&SpecialType_Samsung_SLC_2K },   /* K9F2G08 */
	{ {0xec, 0xdc, 0xc1, 0x15}, 2,     4,      64,    15,     2048,   984,   0x0010,  15, 0x06, 64,&SpecialType_Samsung_SLC_2K },   /* K9K4G08 */ /* test pass */
	{ {0xec, 0xdc, 0x10, 0x95}, 1,     4,      64,    15,     4096,   984,   0x0014,  15, 0x06, 64,&SpecialType_Samsung_SLC_2K },   /* K9F4G08 */ /* test pass */
	{ {0xec, 0xd3, 0x51, 0x95}, 2,     4,      64,    30,     4096,   984,   0x0014,  25, 0x06, 64,&SpecialType_Samsung_SLC_2K },   /* K9K8G08 */ /* test pass */

	/* 4KB page SLC */
	/*@fish add 2010-03-01  0xec, 0xd3, 0x10, 0xa6  K9F8G08M */ 
	{ {0xec, 0xd3, 0x10, 0xa6}, 1,     8,      64,    30,     4096,   984,   0x0014,  25, 0x06,128,  &SpecialType_Samsung_SLC_4K },  
	{ {0xec, 0xd3, 0x50, 0xa6}, 1,     8,      64,    30,     4096,   984,   0x0014,  25, 0x06,128,  &SpecialType_Samsung_SLC_4K },   /* K9F8G08 */
	{ {0xec, 0xd5, 0x51, 0xa6}, 2,     8,      64,    30,     4096,   984,   0x0034,  25, 0x06,128,  &SpecialType_Samsung_SLC_4K },   /* K9KAG08 */

	/* MLC */

	{ {0xec, 0xda, 0x14, 0x25}, 1,     4,     128,    20,     1024,   984,   0x0000,  20, 0x06, 64, &SpecialType_Samsung_MLC_2K },   /* K9G2G08U0M */ /* 2008-08-08 added */
	{ {0xec, 0xdc, 0x14, 0x25}, 1,     4,     128,    20,     2048,   984,   0x0004,  20, 0x06,64,  &SpecialType_Samsung_MLC_2K },   /* K9G4G08 */ /* test pass */
	{ {0xec, 0xdc, 0x14, 0xa5}, 1,     4,     128,    30,     2048,   984,   0x0004,  25, 0x06,64, &SpecialType_Samsung_MLC_2K },   /* K9G4G08 */ /* test pass */
	{ {0xec, 0xd3, 0x55, 0x25}, 2,     4,     128,    20,     2048,   984,   0x0004,  20, 0x06,64,  &SpecialType_Samsung_MLC_2K },   /* K9L8G08U0M */ /* test pass */
	{ {0xec, 0xd3, 0x55, 0xa5}, 2,     4,     128,    30,     2048,   984,   0x0004,  25, 0x06,64, &SpecialType_Samsung_MLC_2K },   /* K9L8G08U0A */ /* test pass */
	{ {0xec, 0xd3, 0x14, 0x25}, 1,     4,     128,    20,     4096,   984,   0x0004,  20, 0x06,64,  &SpecialType_Samsung_MLC_2K },   /* K9G8G08 */ /* test pass */
	{ {0xec, 0xd3, 0x14, 0xa5}, 1,     4,     128,    30,     4096,   984,   0x0004,  25, 0x06,64, &SpecialType_Samsung_MLC_2K },   /* K9G8G08 */ /* test pass */
	{ {0xec, 0xd5, 0x55, 0x25}, 2,     4,     128,    20,     4096,   984,   0x0024,  20, 0x06,64, &SpecialType_Samsung_MLC_2K },   /* K9LAG08U0M K9HBG08U1M*/ /* test pass */
	{ {0xec, 0xd5, 0x55, 0xa5}, 2,     4,     128,    20,     4096,   984,   0x0024,  20, 0x06,64, &SpecialType_Samsung_MLC_2K },   /* K9LAG08U0A K9HBG08U1A*/ /* test pass */
	{ {0xec, 0xd7, 0xd5, 0x29}, 2,	   8,	  128,	  25, 	  4096,	  984,   0x0004,  25, 0x06,218,  &SpecialType_Samsung_MLC_4K },	 /* K9LBG08U0D */

	/* 4KB page MLC */

	{ {0xec, 0xd5, 0x14, 0xb6}, 1,     8,     128,    25,     4096,   984,   0x0004,  25,  0x06,128, &SpecialType_Samsung_MLC_4K },   /* K9GAG08 */ /* test pass */
	{ {0xec, 0xd5, 0x94, 0x29}, 1,     8,     128,    25,     4096,   984,   0x0004,  25,  0x06,218, &SpecialType_Samsung_MLC_4K },   /* K9GAG08U0D (42nm) ,2008-07-07*/
	{ {0xec, 0xd7, 0x55, 0xb6}, 2,     8,     128,    25,     4096,   984,   0x0024,  25,  0x06,128, &SpecialType_Samsung_MLC_4K },   /* K9LBG08 */ /* test pass */
	//K9GAG08UOE   24Bit ECC 1KB/24Bit 8K Page 2010 -04-28 
	//K9LBG08UOE   24Bit ECC 1KB/24Bit 8K Page 2010 -06-28 0xec, 0xd5, 0xc4, 0x72
	//K9LBG08UOE(K9HCG08U1E)   K9GBG08U0M  24Bit ECC 1KB/24Bit 8K Page 2010 -04-28 
	//@fish modify DataBlock 984-->954    2010-05-13 
	//K9GBG08UOA  24Bit ECC 1KB/24Bit  {0xec, 0xd7, 0x94, 0x72},  
	//fish add 2011-01-19 0x7694d7ec 支持ranomizer 功能
	{ {0xec, 0xd5, 0x84, 0x72}, 1,     16,  128,    25,     2048,   954,   0x4000, 35,0x0b ,436, &SpecialType_Samsung_MLC_4K },  //K9GAG08UOE   24Bit ECC 1KB
	{ {0xec, 0xd5, 0xc4, 0x72}, 1,     16,  128,    25,     4096,   954,   0x4000, 35,0x0b, 436, &SpecialType_Samsung_MLC_4K },  //K9LBG08UOE   24Bit ECC 
	{ {0xec, 0xd7, 0xC5, 0x72}, 1,     16,  128,    25,     4096,   954,   0x4000, 35,0x0b, 436, &SpecialType_Samsung_MLC_4K },  //K9LBG08UOE(K9HCG08U1E)
	{ {0xec, 0xd7, 0x94, 0x72}, 1,     16,  128,    25,     4096,   954,   0x4004, 35,0x0b, 436, &SpecialType_Samsung_MLC_4K }, //K9GBG08UOM   24Bit ECC 1KB/24Bit 
	{ {0xec, 0xd7, 0x94, 0x76}, 1,     16,  128,    25,     4096,   954,   0x4800, 20,0x0b, 436, &SpecialType_Samsung_MLC_4K }, // K9GBG08UOA   24Bit ECC 1KB/24Bit 
	{ {0xec, 0xd5, 0x94, 0x76}, 1,	   16,	128,	25, 	2048,	954,   0x4004, 25,0x0b, 512, &SpecialType_Samsung_MLC_4K },  /* K9GAG08U0F	*/	
//	{ {0xec, 0xd5, 0x94, 0x76}, 1,	   16,	128,	25, 	2048,	954,   0x4004, 25,0x0b, 512, &SpecialType_Samsung_MLC_4K },  /* K9GAG08U0F	*/	
	{ {0xec, 0xd7, 0x94, 0x7a}, 1,	   16,	128,	25, 	4096,	954,   0x6824, 20,0x0f, 640, &SpecialType_Samsung_MLC_4K }, // K9GBG08UOA V1.2	24Bit ECC 1KB/24Bit  ranomizer
	{ {0xec, 0xd7, 0x94, 0x7e}, 1,	   16,	128,	25, 	4096,	954,   0x6804, 20,0x16, 1024, &SpecialType_Samsung_MLC_4K }, // K9GBG08UOb V1.2	40Bit ECC 1KB/24Bit  ranomizer
	
	{ {0xff, 0xff, 0xff, 0xff}, 0,     0,       0,     0,        0,     0,   0x0000,  15, 0x06, 64, NULL },   /* NULL */
};


/*==================================================================*/
/*====================HYNIX NAND FLASH ID TABLE=====================*/
/*==================================================================*/
struct NandDevPar   HynixNandTbl[] =
{
    /*       NAND_ID         DieCnt  SecCnt  PagCnt  Freq    BlkCnt  DatBlk   OpOpt     SpeCmd */

 
	/* SLC */
	/*  @fish add  2010-03-01  0xad 0xf1 0x0 0x1d H27U1G8F2BTR*/
	{ {0xad, 0xf1, 0x00, 0x1d}, 1,     4,      64,    15,     1024,   984,   0x0000,  15,  0x06,64, &SpecialType_Samsung_SLC_2K },   /* HY27UF081G2M - tRC = 50ns */
	{ {0xad, 0xf1, 0x80, 0x15}, 1,     4,      64,    15,     1024,   984,   0x0000,  15,  0x06,64, &SpecialType_Samsung_SLC_2K },   /* HY27UF081G2M - tRC = 50ns */
	{ {0xad, 0xf1, 0x80, 0x1d}, 1,     4,      64,    20,     1024,   984,   0x0000,  15,  0x06,64, &SpecialType_Samsung_SLC_2K },   /* HY27UF081G2A - tRC = 30ns */
	{ {0xad, 0xda, 0x80, 0x15}, 1,     4,      64,    15,     2048,   984,   0x0000,  15,  0x06,64,  &SpecialType_Samsung_SLC_2K },   /* HY27UF082G2M - tRC = 30ns */
	{ {0xad, 0xda, 0x80, 0x1d}, 1,     4,      64,    20,     2048,   984,   0x0000,  20,  0x06,64, &SpecialType_Samsung_SLC_2K },   /* HY27UF082G2A - tRC = 30ns */ /* NOTE:copy back only take in the same 1024 */
	{ {0xad, 0xda, 0x90, 0x95}, 1,	   4,	   64,	  20,	  2048,   984,	 0x0004,  20,  0x06,64, &SpecialType_Samsung_SLC_2K },	 /* H27U2G8F2C & H27S2G8F2C */
	/*  @fish add  2008-10-27 
	HY27UF082G2B  OPOpt 0x14, 0x14 CopyBack+twoPlaneWrite
	add HY27UF082G2B Flash ID:0xAD,0xDA,0x10,0x95 
	*/
	{ {0xad, 0xda, 0x10, 0x95}, 1,     4,      64,    20,     2048,   984,  0x0014,  20,  0x06,64, &SpecialType_Samsung_SLC_2K },   /* HY27UF082G2B - tRC = 30ns */ /* NOTE:copy back only take in the same 1024 */		
	{ {0xad, 0xdc, 0x80, 0x15}, 4,     4,      64,    15,     1024,   984,   0x0000,  15,  0x06,64, &SpecialType_Samsung_SLC_2K },   /* HY27UH084G2M */
	{ {0xad, 0xdc, 0x80, 0x95}, 1,     4,      64,    20,     4096,   984,   0x0000,  20,  0x06,64, &SpecialType_Samsung_SLC_2K },   /* HY27UF084G2M, HY27UG088G5M */
	{ {0xad, 0xdc, 0x90, 0x95}, 1,	   4,	   64,	  20,	  4096,   984,	 0x0014,  20,  0x06,64, &SpecialType_Samsung_SLC_2K },	 /* H27U4G8F2D, H27S4G8F2D */
	{ {0xad, 0xdc, 0x10, 0x95}, 1,     4,      64,    20,     4096,   984,   0x0004,  20,  0x06,64, &SpecialType_Samsung_SLC_2K },   /* HY27UF084G2B, HY27UG088G5B */
	{ {0xad, 0xd3, 0x80, 0x15}, 4,     4,      64,    20,     2048,   984,   0x0000, 20,   0x06,64, &SpecialType_Samsung_SLC_2K },   /* HY27UG084G2M, HY27H088G2M */
	{ {0xad, 0xd3, 0xc1, 0x95}, 2,     4,      64,    20,     4096,   984,   0x0000,  20,  0x06,64, &SpecialType_Samsung_SLC_2K },   /* HY27UG088G2M, HY27UH08AG5M */
	/*  @fish add  2010-03-01  0xad, 0xd3, 0x51, 0x95 H2Y7UH08AG5B*/
	{ {0xad, 0xd3, 0x51, 0x95}, 2,     4,      64,    20,     4096,   984,   0x0000,  20,  0x06,64, &SpecialType_Samsung_SLC_2K }, 

	/** add 2011-05-12 **/
	/** 2KB page  SLC**/
	{ {0xad, 0xdc, 0x90, 0x95}, 1,     4,      64,    30,     4096,   984,   0x0004,  25,  0x06,128, &SpecialType_Samsung_SLC_4K },   /* H27U4G8F2M*/

	/* 4KB page SLC */

	{ {0xad, 0xd3, 0x14, 0xa6}, 1,     8,      64,    30,     4096,   984,   0x0004,  25,  0x06,128, &SpecialType_Samsung_SLC_4K },   /* H27U8G8F2M, H27UAG8G5M */
	{ {0xad, 0xd5, 0x51, 0xa6}, 2,     8,      64,    30,     4096,   984,   0x0004,  25,  0x06,128, &SpecialType_Samsung_SLC_4K },   /* H27UBG8H5M, H27UCG8KFM */

	/* MLC */
	/* @fish add 
	HY27UV08BG5M  Flash ID:0xad, 0xd5, 0x55, 0xa5,0x68
	HY27UV08BGFM  Flash ID:0xad, 0xd3, 0x14, 0xa5,0x64 
	H27UBG8U5M      Flash ID:0xad, 0xd5, 0x14, 0xb6,0x68
	*/
	{ {0xad, 0xda, 0x14, 0xa5}, 1,     4,      128,    25,    1024,   984,   0x0000,  25, 0x06, 64, &SpecialType_Samsung_MLC_2K },   /* H27U2G8T2M, 57nm MLC, 2008-06-04 */
	{ {0xad, 0xdc, 0x14, 0xa5}, 1,     4,      128,    25,    2048,   924,   0x0004,  20, 0x06, 64, &SpecialType_Samsung_MLC_2K },   /* HY27UT084G2A ,2008-3-27*/
	{ {0xad, 0xdc, 0x84, 0x25}, 1,     4,      128,    15,    2048,   984,   0x0000,  25,  0x06,64, &SpecialType_Samsung_MLC_2K },   /* HY27UT084G2M, HY27UU088G5M */  /* test pass */
	{ {0xad, 0xd3, 0x85, 0x25}, 2,     4,      128,    15,    2048,   984,   0x0000,  15,  0x06,64, &SpecialType_Samsung_MLC_2K },   /* HY27UV08AG5M, HY27UW08BGFM */
	{ {0xad, 0xd3, 0x14, 0x25}, 1,     4,      128,    15,    4096,   984,   0x0004,  15,  0x06,64, &SpecialType_Samsung_MLC_2K },   /* HY27UT088G2M, HY27UU08AG5M */
	{ {0xad, 0xd3, 0x14, 0x2d}, 1,     4,      128,    25,    4096,   984,   0x0004,  20,  0x06,64, &SpecialType_Samsung_MLC_2K },   /* HY27UT088G2M, HY27UU08AG5M */
	//H27U8G8T2BR  2009-12-05
	{ {0xad, 0xd3, 0x14, 0xb6}, 1,     8,      128,    25,    2048,   984,   0x0004, 25,  0x06,128,  &SpecialType_Samsung_MLC_2K },   /* HY27UT088G2M, HY27UU08AG5M */
	{ {0xad, 0xd3, 0x14, 0xa5}, 1,     4,      128,    30,    4096,   984,   0x0004,  25, 0x06, 64, &SpecialType_Samsung_MLC_2K },   /* HY27UT088G2M, HY27UU08AG5M */
	{ {0xad, 0xd5, 0x55, 0x25}, 2,     4,      128,    15,    4096,   984,   0x0004,  15, 0x06,64, &SpecialType_Samsung_MLC_2K },   /* HY27UV08BG5M, HY27UW08CGFM */
	{ {0xad, 0xd5, 0x55, 0x2d}, 2,     4,      128,    25,    4096,   984,   0x0004,  25, 0x06,64,  &SpecialType_Samsung_MLC_2K },   /* HY27UV08BG5M, HY27UW08CGFM */
	{ {0xad, 0xd5, 0x55, 0xa5}, 2,     4,      128,    30,    4096,   984,   0x0004,  25,  0x06,64, &SpecialType_Samsung_MLC_2K },   /* HY27UV08BG5M, HY27UW08CGFM */

	/* 4KB page MLC */
	/* H27UAG8T2M, H27UBG8U5M */   /* H27UCG8V5M, H27UDG8(W_Y)FM */
	{ {0xad, 0xd5, 0x14, 0xb6}, 1,     8,      128,    20,    4096,   984,   0x0014,  20,  0x06,128, &SpecialType_Samsung_MLC_4K },   	
	{ {0xad, 0xd7, 0x55, 0xb6}, 2,     8,      128,    20,    4096,   984,   0x0014,  20, 0x06, 128, &SpecialType_Samsung_MLC_4K },   


	//2009-12-09 H27UAG8T2ATR 12bit   0xad, 0xd5, 0x94, 0x25
	//2010-12-21 H27UAG8T2BTR 24bit   0xad, 0xd5, 0x94, 0x9a
	//H27UBG8T2MYR 12bit    0xad, 0xd5, 0x94, 0x25
	//@fish add 2010-10-26  H27UBG8T2A  8KB Page 256   0xad, 0xd7, 0x94, 0x9a
	{ {0xad, 0xd5, 0x94, 0x25}, 1,    8,   128,    30,    4096,   954,    0x2004,  25, 0x03, 224, &SpecialType_Samsung_MLC_2K },  	
	{ {0xad, 0xd5, 0x94, 0x9a}, 1,   16,   256,    30,    1024,   924,    0x4000,  25, 0x06, 448, &SpecialType_Samsung_MLC_2K },
	{ {0xad, 0xd5, 0x94, 0xda}, 1,	 16,   256,    15,	  1024,   924,	  0x6824,  15, 0x12, 640, &SpecialType_Samsung_MLC_4K },  /*H27UBG8T2CTR */ 
	{ {0xad, 0xd5, 0x94, 0x25}, 2,    8,   128,    30,    4096,   954,    0x2004,  25, 0x03, 224, &SpecialType_Samsung_MLC_2K },  	 
	{ {0xad, 0xd7, 0x94, 0x9a}, 1,   16,   256,    20,    2048,   954,    0x4000,  20, 0x06, 448,&SpecialType_Samsung_MLC_4K },   
	{ {0xad, 0xd7, 0x94, 0xda}, 1,	 16,   256,	   20,	  2048,	  924,	  0x6800,  20, 0x10, 640,&SpecialType_Samsung_MLC_4K },	/*H27UBG8T2BTR  26nm  24bit ecc*/
	{ {0xad, 0xde, 0x94, 0xda}, 1,	 16,   256,	   20,	  4096,	  924,	  0x6004,  15, 0x10, 640,&SpecialType_Samsung_MLC_4K },	
	


	{ {0xff, 0xff, 0xff, 0xff}, 0,     0,       0,     0,        0,     0,   0x0000,  20,  0x06,0,NULL },   /* NULL */
};

/*==================================================================*/
/*==================TOSHIBA NAND FLASH ID TABLE=====================*/
/*==================================================================*/
struct NandDevPar   ToshibaNandTbl[] =
{
    /*       NAND_ID         DieCnt  SecCnt  PagCnt  Freq    BlkCnt  DatBlk   OpOpt     SpeCmd */


    /* SLC */
        { {0x98, 0xf1, 0x00, 0x1D}, 1,     4,      64,    20,     1024,   984,   0x0000,  20,  0x03, 64,&SpecialType_Toshiba_SLC_2K },/*MP1G08KABS*/
	{ {0x98, 0xf1, 0x80, 0x95}, 1,     4,      64,    20,     1024,   984,   0x0000,  20,  0x06, 64,&SpecialType_Toshiba_SLC_2K },   /* TC58NVG0S3B */	
	{ {0x98, 0xf1, 0x80, 0x25}, 1,     4,      64,    20,     1024,   984,   0x0000,  20,  0x06, 64,&SpecialType_Toshiba_SLC_2K },   /* TC58NVG0S3B */
	{ {0x98, 0xf1, 0x80, 0x15}, 1,     4,      64,    20,     1024,   984,   0x0000,  20,  0x06,128,&SpecialType_Toshiba_SLC_2K },   /* TC58NVG0S3HTA00*/
	{ {0x98, 0xD1, 0x90, 0x15}, 1,     4,      64,    20,     1024,   984,   0x0000,  20,  0x06, 64,&SpecialType_Toshiba_SLC_2K },    //@fish add TC58NVG0S3ETA00
	{ {0x98, 0xDc, 0x90, 0x15}, 1,     4,      64,    20,     4096,   924,   0x0000,  20,  0x06, 64,&SpecialType_Toshiba_SLC_2K },   //@fish add TC58NVG2S3ETA00 2010-11-10 add  1590dc98
	{ {0x98, 0xda, 0x90, 0x15}, 1,     4,      64,    20,     2048,   984,   0x0000,  20,  0x06, 64,&SpecialType_Toshiba_SLC_2K },   /* TC58BVG1S3HTA00 */	
	{ {0x98, 0xda, 0xff, 0x95}, 1,     4,      64,    20,     2048,   984,   0x0000,  20, 0x06, 64,&SpecialType_Toshiba_SLC_2K },   /* TC58NVG1S3B */  /* test pass */
	{ {0x98, 0xdc, 0x81, 0x95}, 1,     4,      64,    20,     4096,   984,   0x0000,  20,  0x06,64,&SpecialType_Toshiba_SLC_2K },   /* TC58NVG2S3B */  /* test pass */

	/* MLC */

	{ {0x98, 0xda, 0x84, 0xa5}, 1,     4,     128,    20,     1024,   984,   0x0000,  20,  0x06,64,&SpecialType_Toshiba_MLC_2K_1024 },   /* TC58NVG1D4B */
	{ {0x98, 0xdc, 0x84, 0xa5}, 1,     4,     128,    20,     2048,   984,   0x0004,  20,  0x06,64,&SpecialType_Toshiba_MLC_2K_1024 },   /* TC58NVG2D4B */  /* test pass */
	{ {0x98, 0xd3, 0x84, 0xa5}, 1,     4,     128,    20,     4096,   984,   0x0004,  20,  0x06,64,&SpecialType_Toshiba_MLC_2K_2048 },   /* TC58NVG3D4C */  /* test pass */
	{ {0x98, 0xd5, 0x85, 0xa5}, 2,     4,     128,    20,     4096,   984,   0x0004,  20,  0x06,64,&SpecialType_Toshiba_MLC_2K_2048 },   /* TC58NVG4D4C, TC58NVG5D4C */  /* test pass */

	/* 4K page MLC */
	{ {0x98, 0xd3, 0x94, 0xba}, 1,     8,     128,    20,     2048,   984,   0x0004,  25,  0x06,128,&SpecialType_Toshiba_MLC_2K_1024 },   /* TC58NVG3D1DTG00 */
	{ {0x98, 0xd5, 0x94, 0xba}, 1,     8,     128,    20,     4096,   984,   0x0004,  25,  0x06,128,&SpecialType_Toshiba_MLC_2K_2048 },   /* TC58NVG4D1DTG00 */
	{ {0x98, 0xd7, 0x94, 0xba}, 2,     8,     128,    20,     4096,   984,   0x0004,  25,  0x06,128,&SpecialType_Toshiba_MLC_2K_2048 },   /* TH58NVG5D1DTG20, TH58NVG6D1DTG20, 8 bits ECC */

    { {0x98, 0xd7, 0x84, 0x93}, 1,     16,    256,    20,     1024,   960,   0x6800,  20,  0x10,1280,&SpecialType_Toshiba_MLC_2K_2048 },  /*TC58TEG5DCKTA00*/

	/*8K Page MLC 0x3294d598   6D2E //98 d7 95 32 @fish add 2009-11-05 */     
	//@fish modify DataBlock 984-->954    2010-05-13 
	//@fish 2010-05-25 0x3294d798 5D2FTA00  4GB 
	//TC58NVG5D2FTA00    4GB   TH58NVG6D2FTA20  4GB*2  0x98, 0xd7, 0x94, 0x32
	//TC58NVG4D2HTA00   2GB   @added by leiwg 2012-3-22  
	
	{ {0x98, 0xd5, 0x84, 0x32}, 1,     16,     128,    20,     2048,   924,   0x4000,  30,  0x0b,640,&SpecialType_Toshiba_MLC_8K }, /*TC58NVG4D2HTA00 16Gbit*/
	{ {0x98, 0xd5, 0x94, 0x32}, 1,     16,     128,    20,     2048,   924,   0x4000,  30,  0x0b,376,&SpecialType_Toshiba_MLC_8K }, /*TC58NVG4D2ETA00 16Gbit*/
	{ {0x98, 0xd7, 0x95, 0x32}, 1,     16,     128,    20,     2048,   924,   0x4000,  25,  0x0b,376,&SpecialType_Toshiba_MLC_8K }, /*TC58NVG6D2ETA20 64Gbit 24Bit ECC*/  
	{ {0x98, 0xd7, 0x94, 0x32}, 1,     16,     128,    20,     4096,   954,   0x4000,  25,  0x0b,448,&SpecialType_Toshiba_MLC_8K }, /*TC58NVG5D2HTA20 32Gbit 40Bit ECC*/
	{ {0x98, 0xde, 0x94, 0x82}, 1,	   16,	   256,    20,	   4096,   874,   0x6000,  25,	0x0b,640,&SpecialType_Toshiba_MLC_8K }, /* TC58NVG6D2GTA00  40BIT ECC */


	{ {0xff, 0xff, 0xff, 0xff}, 0,     0,       0,     0,        0,     0,   0x0000, 20,0x06,64,NULL },   /* NULL */
};




/*==================================================================*/
/*===================MICRON/INTEL NAND FLASH ID TABLE=====================*/
/*==================================================================*/
struct NandDevPar   MicronNandTbl[] =
{
	/*       NAND_ID         DieCnt  SecCnt  PagCnt  Freq    BlkCnt  DatBlk   OpOpt     SpeCmd */

	/* SLC */

	{ {0x2c, 0xda, 0xff, 0x15}, 1,     4,      64,    25,     2048,   984,   0x0010, 25,  0x06, 64,&SpecialType_Micron },   /* MT29F2G08AAC, JS29F02G08AAN */ /* test pass */
	{ {0x2c, 0xf1, 0x80, 0x95}, 1,     4,      64,    25,     1024,   984,   0x0000, 25,  0x06, 64,&SpecialType_Micron },   /* MT29F1G08ABAEA,  */
	{ {0x2c, 0xdc, 0xff, 0x15}, 2,     4,      64,    25,     2048,   984,   0x0010, 25,  0x06,64,&SpecialType_Micron },   /* MT29F4G08BAB, MT29F8G08FAB, JS29F04G08BAN, JS29F08G08FAN */ /* test pass */
	{ {0x2c, 0xdc, 0x90, 0x95}, 1,     4,      64,    25,     4096,   984,   0x0014, 25,   0x06,64,&SpecialType_Micron },   /* MT29F4G08AAA, MT29F8G08DAA, JS29F04G08AAN */ /* test pass */
	{ {0x2c, 0xd3, 0xd1, 0x95}, 2,     4,      64,    25,     4096,   984,   0x0014, 25,  0x06,64,&SpecialType_Micron },   /* MT29F8G08BAB, MT29F16G08FAB, JS29F08G08BAN, JS29F16G08FAN */ /* test pass */
	
	{ {0x2c, 0x48, 0x00, 0x26}, 1,     8,     128,    25,     4096,   984,   0x0014, 25,   0x06,224,&SpecialType_Micron },   /* MT29F16G08ABACA */ /* 2014/01/03 add */

	/* MLC */

	{ {0x2c, 0xdc, 0x84, 0x25}, 1,     4,     128,    20,     2048,   984,   0x0000, 20, 0x06,64, &SpecialType_Micron },   /* MT29F4G08MAA, MT29F8G08QAA */ /* nic timeout */
	{ {0x2c, 0xd3, 0x85, 0x25}, 2,     4,     128,    20,     2048,   984,   0x0000,20,  0x06,64,&SpecialType_Micron },   /* MT29F16GTAA */
	{ {0x2c, 0xd3, 0x94, 0x2d}, 1,     4,     128,    15,     4096,   984,   0x0014, 20, 0x06, 64,&SpecialType_Micron },   /* MT29F8G08MAD 	2008-3-11*/
	{ {0x2c, 0xd3, 0x94, 0xa5}, 1,     4,     128,    30,     4096,   984,   0x0014, 20, 0x06, 64,&SpecialType_Micron },   /* MT29F8G08MAA, MT29F16G08QAA, JS29F08G08AAM, JS29F16G08CAM */
	{ {0x2c, 0xd5, 0x95, 0xa5}, 2,     4,     128,    20,     4096,   984,   0x0004,20,  0x06,64,&SpecialType_Micron },   /* MT29F32G08TAA, JS29F32G08FAM */  /* test pass */
	{ {0x2c, 0xd5, 0xd5, 0xa5}, 2,     4,     128,    20,     4096,   984,   0x0034, 20,  0x06,64,&SpecialType_Micron },   /* MT29F32G08TAA, JS29F32G08FAM */  /* test pass */

	/* 4KB page SLC */

	{ {0x2c, 0xd3, 0x90, 0x2e}, 1,     8,      64,    40,     4096,   984,   0x0004, 30,  0x06,128,&SpecialType_Micron },   /* copy from intel (50nm) - 2008-07-25 */
	{ {0x2c, 0xd5, 0xd1, 0x2e}, 2,     8,      64,    40,     4096,   984,   0x0004, 30,  0x06,128,&SpecialType_Micron },   /* copy from intel (50nm) - 2008-07-25 */

	/* 4KB page MLC */

	{ {0x2c, 0xd5, 0x94, 0x3e}, 1,     8,     128,    30,     4096,   984,   0x2004, 30,  0x04,218,&SpecialType_Micron },   /* MT29F16G08MAA, MT29F32G08QAA, JS29F32G08AAM, JS29F32G08CAM, 8 bits ECC */
	{ {0x2c, 0xd7, 0xd5, 0x3e}, 2,     8,     128,    30,     4096,   984,   0x2004, 30,  0x04,218,&SpecialType_Micron },   /* MT29F64G08TAA, 8 bits ECC  */

	/* MT29F32G08BAAAA,   MT29F64G08CFAAA,   MT29F128G08TAA12bits ECC     2009-08-07  add   */	
	//@fish add for 2010-04-29 for 34nm      //MT29F64G08CBAAA  	
	{ {0x2c, 0xd7, 0x94, 0x3e}, 2,     8,     128,    30,     4096,   954,   0x2004, 30,  0x06,218,&SpecialType_Micron },    
	{ {0x2c, 0xd9, 0xd5, 0x3e}, 4,     8,     128,    30,     4096,   944,   0x2004, 30,  0x04,218,&SpecialType_Micron },  //MT29F64G08CBAAA 
	{ {0x2c, 0x88, 0x04, 0x4b}, 1,    16,     256,    30,     4096,   944,   0x4004, 30, 0x08,448,&SpecialType_Micron },//MT29F32G08CBABA for Star Ram  4604682c
	{ {0x2c, 0x68, 0x04, 0x46}, 1,     8,     256,    30,     4096,   954,   0x2004, 30, 0x04,224,&SpecialType_Micron },   
	{ {0x2c, 0x48, 0x04, 0x4a}, 1,	   8,	  256,	  20,	  2048,   944,	 0x4800, 20, 0x06,224,&SpecialType_Micron },// MT29F16G08CBACA
	{ {0x2c, 0x68, 0x04, 0x4a}, 1,	   8,	  256,	  20,	  4096,   944,	 0x4000, 20, 0x06,224,&SpecialType_Micron },// MT29F32G08CBACA
	{ {0x2c, 0x44, 0x44, 0x4b}, 1,	   16,	  256,	  20,	  2048,   914,	 0x6820, 20, 0x18,744,&SpecialType_Micron_8K },// MT29F32G08CBADA
	{ {0x2c, 0x64, 0x44, 0x4b}, 1,	   16,	  256,	  20,	  4096,   914,	 0x6800, 20, 0x18,744,&SpecialType_Micron_8K },// MT29F64G08CBABA
	

	{ {0xff, 0xff, 0xff, 0xff}, 0,     0,       0,     0,        0,     0,   0x0000,20,   0x06, 0x00,NULL },   /* NULL */
};


struct NandDevPar   IntelNandTbl[] =
{
	/* MLC */

	{ {0x89, 0xd3, 0x94, 0xa5}, 1,     4,     128,    30,     4096,   984,   0x0014,25,   0x06,64,&SpecialType_Micron },   /* 29F08G08AAMB2, 29F16G08CAMB2 */
	{ {0x89, 0xd5, 0xd5, 0xa5}, 2,     4,     128,    20,     4096,   984,   0x0014,20,   0x06,64,&SpecialType_Micron },   /* 29F32G08FAMB2 */
	//@fish add 2008-10-22 把 29F32G08FAMB2 OperationOpt 0x34改成0x14 去除IntelINTERNAL_INTERLEAVE		

	/* 4KB page SLC */

	{ {0x89, 0xd3, 0x90, 0x2e}, 1,     8,      64,    40,     4096,   984,   0x0004,30,   0x06,128,&SpecialType_Micron },   /* JS29F08G08AANC1, JS29F16G08CANC1 (50nm) - 2008-07-25 */
	{ {0x89, 0xd5, 0xd1, 0x2e}, 2,     8,      64,    40,     4096,   984,   0x0004, 30,  0x06,128,&SpecialType_Micron },   /* JS29F32G08FANC1 (50nm) - 2008-07-25 */

	/* 4KB page MLC */
	{ {0x89, 0xd5, 0x94, 0x3e}, 1,     8,     128,    30,     4096,   984,   0x0004, 25,  0x06,128,&SpecialType_Micron },   /* JS29F16G08AAM, JS29F32G08CAM, 8 bits ECC */
	//device ID:4GB  12Bit ECC 
	{ {0x89, 0xd7, 0x94, 0x3e}, 1,     8,     128,    30,     4096,   984,   0x4004, 25, 0x08,218, &SpecialType_Micron },   /* JS29F16G08AAM, JS29F32G08CAM, 8 bits ECC */
	{ {0x89, 0xd7, 0xd5, 0x3e}, 2,     8,     128,    30,     4096,   984,   0x0004, 25, 0x06,128, &SpecialType_Micron },   /* JS29F64G08FAM, 8 bits ECC  */
	// //4K Page 32nm JS29F32G08AAMDB 2010-08-11 
	{ {0x89, 0x68, 0x04, 0x46}, 1,     8,     256,    30,     4096,   954,   0x4000, 25,  0x08,224,&SpecialType_Micron },   
	{ {0x89, 0x88, 0x24, 0x4b}, 1,	   16,	  256,	  30,	  4096,   954,	 0x2004, 25,  0x06,448,&SpecialType_Micron },	/*29f64g08aamfi  */

	{ {0xff, 0xff, 0xff, 0xff}, 0,     0,       0,     0,        0,     0,   0x0000,20,   0x06, 64,NULL },   /* NULL */
};


/*==================================================================*/
/*=======================ST NAND FLASH ID TABLE=====================*/
/*==================================================================*/
struct NandDevPar   StNandTbl[] =
{
    /*       NAND_ID         DieCnt  SecCnt  PagCnt  Freq    BlkCnt  DatBlk   OpOpt     SpeCmd */

   /* SLC */

    { {0x20, 0xf1, 0x80, 0x15}, 1,     4,      64,    15,     1024,   984,   0x0010, 15, 0x06, 64,&SpecialType_Samsung_SLC_2K },   /* NAND01GW3B */  /* test pass */
    { {0x20, 0xda, 0x80, 0x15}, 1,     4,      64,    15,     2048,   984,   0x0000,15,  0x06, 64,&SpecialType_Samsung_SLC_2K },   /* NAND02GW3B */  /* test pass */ //NOTE:copy back must in the
    { {0x20, 0xdc, 0x80, 0x95}, 1,     4,      64,    15,     4096,   984,   0x0000, 15, 0x06, 64,&SpecialType_Samsung_SLC_2K },   /* NAND04GW3B */  /* test pass */
    { {0x20, 0xdc, 0x10, 0x95}, 1,     4,      64,    30,     4096,   984,   0x0004, 30, 0x06, 64,&SpecialType_Samsung_SLC_2K },   /* NAND04GW3B2D,NAND08GW3B4C, 2008-07-07*/
    { {0x20, 0xd3, 0xc1, 0x95}, 2,     4,      64,    15,     4096,   984,   0x0000, 15, 0x06, 64,&SpecialType_Samsung_SLC_2K },   /* NAND08GW3B */
    { {0x20, 0xd3, 0x51, 0x95}, 2,     4,      64,    30,     4096,   984,   0x0004, 25, 0x06,64, &SpecialType_Samsung_SLC_2K },   /* NAND08GW3B2CN6, 2008-07-07 */

    /* MLC */

    { {0x20, 0xdc, 0x84, 0x25}, 1,     4,      128,    15,     2048,   984,   0x0000,15,  0x06, 64,&SpecialType_Samsung_MLC_2K },   /* NAND04GW3C */  /* test pass */
    { {0x20, 0xd3, 0x85, 0x25}, 2,     4,      128,    15,     2048,   984,   0x0000, 15, 0x06, 64,&SpecialType_Samsung_MLC_2K },   /* NAND08GW3C */
    { {0x20, 0xd3, 0x14, 0xa5}, 1,     4,      128,    30,     4096,   984,   0x0004, 25, 0x06,64,&SpecialType_Samsung_MLC_2K },   /* NAND08GW3C2B,NAND16GW3C4B, 2008-07-07 */

    { {0xff, 0xff, 0xff, 0xff}, 0,     0,       0,     0,        0,     0,   0x0000, 20, 0x06,64,NULL },   /* NULL */
};


/*==================================================================*/
/*====================RENESSAS NAND FLASH ID TABLE==================*/
/*==================================================================*/
struct NandDevPar   RenessasNandTbl[] =
{
    /*       NAND_ID         DieCnt  SecCnt  PagCnt  Freq    BlkCnt  DatBlk   OpOpt     SpeCmd */

    { {0xff, 0xff, 0xff, 0xff}, 0,     0,       0,     0,        0,     0,   0x0000, 20,  0x06,64,NULL },   /* NULL */
};

/*==================================================================*/
/*====================INFINEON NAND FLASH ID TABLE==================*/
/*==================================================================*/

struct NandDevPar   InfineonNandTbl[] =
{
    /*       NAND_ID         DieCnt  SecCnt  PagCnt  Freq    BlkCnt  DatBlk   OpOpt     SpeCmd */

    /* SLC Small block */

    { {0xC1, 0x76, 0xff, 0xff}, 2,     4,      8,    10,     2048,   984,   0x0080, 10, 0x06, 64, &SpecialType_Samsung_SLC_512 },   /* HYF33DS512800ATC/ATI 512Mb */ 
    { {0xC1, 0x66, 0xff, 0xff}, 1,     4,      8,    10,     1024,   984,   0x0080, 10, 0x06, 64,&SpecialType_Samsung_SLC_512 },   /* HYF33DS51280[4/5]BT[C/I] 512Mb */ 


    { {0xff, 0xff, 0xff, 0xff}, 0,     0,       0,     0,        0,     0,   0x0000, 20, 0x06, 64,&SpecialType_Samsung_SLC_2K },
};
/*==================================================================*/
/*====================INFINEON NAND FLASH ID TABLE==================*/
/*==================================================================*/

struct NandDevPar   PowerFlashNandTbl[] =
{
    /*       NAND_ID         DieCnt  SecCnt  PagCnt  Freq    BlkCnt  DatBlk   OpOpt     SpeCmd */

    /* SLC  */
   //2504dc92 MLC  1500da92  1500dc92
    { {0x92, 0xda, 0x00, 0x15}, 1,     4,      64,    15,     2048,   984,   0x0000,15, 0x06, 64, &SpecialType_Samsung_SLC_512 },   /* ASU2GA30GT-G30CA */ 
    { {0x92, 0xdc, 0x00, 0x15}, 1,     4,      64,    15,     4096,   984,   0x0000,15,  0x06,64, &SpecialType_Samsung_SLC_512 },   /* ASU4GA30GT-G30CA */ 
	  { {0x92, 0xf1, 0x80, 0x95}, 1,	   4,	     64,	  15,	    1024,	  984,	 0x0000, 15, 0x06,64, &SpecialType_Samsung_SLC_512 },	/* F59LG81A*/

     /* MLC  */
    { {0x92, 0xda, 0x04, 0x25}, 1,     4,       128,     20,    2048,     984,   0x0000, 20,  0x06,64,&SpecialType_Samsung_SLC_2K },    /* A1U4GA30GT-G30CA*/
    { {0x92, 0xdc, 0x04, 0x25}, 1,     4,       128,     20,    2048,     984,   0x0000, 20, 0x06,64, &SpecialType_Samsung_SLC_2K },    /* A1U4GA30GT-G30CA*/
	

    { {0xff, 0xff, 0xff, 0xff}, 0,     0,       0,     0,        0,     0,   0x0000, 20,  0x06,64,&SpecialType_Samsung_SLC_2K },
};

/*==================================================================*/
/*====================USER CREATE FLASH ID TABLE==================*/
/*==================================================================*/
struct NandDevPar   UserCreateNandTbl[] =
{
    /*       NAND_ID         DieCnt  SecCnt  PagCnt  Freq    BlkCnt  DatBlk   OpOpt     SpeCmd */

    { {0xff, 0xff, 0xff, 0xff}, 0,     0,       0,     0,        0,     0,   0x0000, 20,  0x06,64,&SpecialType_Samsung_SLC_2K },
};

struct NandDevPar MiraFlashNandTbl[]=
{
	{ {0xc8, 0xd5, 0x14, 0x29}, 1,     8,      128,    20,       4096,     984,   0x2000, 20, 0x06,218, &SpecialType_Samsung_SLC_2K }, /* P1UAGA30AT-GCA  12bit ECC**/
	{ {0xc8, 0xda, 0x90, 0x95}, 1,	   4,	    64,    20,		 2048,	   984,   0x0000, 20, 0x03, 64, &SpecialType_Samsung_SLC_2K }, /* F59L2G81A  4bit ECC**/
	{ {0xc8, 0xdc, 0x90, 0x95}, 1,	   4,		64,    20,		 4096,	   984,   0x0000, 20, 0x03, 64, &SpecialType_Samsung_SLC_2K }, /* F59L4G81A  4bit ECC**/
	{ {0xc8, 0xd1, 0x80, 0x95}, 1,	   4,		64,    20,		 1024,	   984,   0x0000, 20, 0x03, 64, &SpecialType_Samsung_SLC_2K }, /* PSU1GA30BT  1bit ECC**/
	{ {0xc8, 0xf1, 0x80, 0x1D}, 1,	   4,		64,    20,		 1024,	   984,   0x0000, 20, 0x03, 128, &SpecialType_Samsung_SLC_2K }, /* GD9FU1G8F2A  4bit ECC**/	
	{ {0xff, 0xff, 0xff, 0xff}, 0,	   0,		0,	   0,		 0, 	0,	 0x0000, 20, 6,64, &SpecialType_Samsung_SLC_2K },
};

struct NandDevPar SpansionFlashNandTbl[]=
{
	{ {0x01, 0xda, 0x90, 0x95}, 1,     4,       64,    30,       2048,     984,   0x0000, 30, 0x03,128, &SpecialType_Spansion_SLC_2K }, /* S34ML02G2  4bit ECC**/
	{ {0x01, 0xf1, 0x80, 0x1d}, 1,	   4,	    64,    30,		 1024,	   984,   0x0000, 30, 0x03, 64, &SpecialType_Spansion_SLC_2K }, /* S34ML01G2 4bit ECC**/
	{ {0x01, 0xdc, 0x90, 0x95}, 1,	   4,		64,    30,		 4096,	   984,   0x0000, 30, 0x03, 128, &SpecialType_Spansion_SLC_2K }, /* S34ML04G2  4bit ECC**/
		
	{ {0xff, 0xff, 0xff, 0xff}, 0,	   0,		0,	   0,		 0, 	0,	 0x0000, 20, 6,64, &SpecialType_Spansion_SLC_2K },
};

struct NandDevPar MxicFlashNandTbl[]=
{
	{ {0xc2, 0xf1, 0x80, 0x1d}, 1,     4,      64,    25,       1024,     984,   0x0000, 25, 0x03, 64,&SpecialType_Samsung_SLC_2K }, /*MX30LF1G08AA  1bit ECC**/
    { {0xc2, 0xf1, 0x80, 0x95}, 1,     4,      64,    25,       1024,     984,   0x0000, 25, 0x03, 64,&SpecialType_Samsung_SLC_2K }, /*MX30LF1G18AC  1bit ECC**/
	{ {0xc2, 0xda, 0x90, 0x95}, 1,	   4,	   64,	  25,		2048,	  984,	 0x0000, 25, 0x03,112,&SpecialType_Samsung_SLC_2K }, /*MX30LF2G28AB  8bit ECC OK**/

	{ {0xff, 0xff, 0xff, 0xff}, 0,	   0,		0,	   0,		 0, 	0,	 0x0000, 20, 0x03, 64,&SpecialType_Samsung_SLC_2K },
};

struct NandDevPar WinBondFlashNandTbl[]=
{
	{ {0xef, 0xf1, 0x80, 0x95}, 1,     4,      64,    25,       1024,     984,   0x0000, 25, 0x03, 64,&SpecialType_Samsung_SLC_2K }, /*  W29N01GV 8bit ECC**/
	{ {0xef, 0xf1, 0x00, 0x95}, 1,     4,      64,    25,       1024,     984,   0x0000, 25, 0x03, 64,&SpecialType_Samsung_SLC_2K }, /*  W29N01HV 1bit/4bit ECC**/
	{ {0xef, 0xda, 0x90, 0x95}, 1,     4,      64,    25,       2048,     984,   0x0000, 25, 0x03, 64,&SpecialType_Samsung_SLC_2K }, /*  W29N02GV 1bit/4bit ECC**/
	{ {0xff, 0xff, 0xff, 0xff}, 0,	   0,		0,	   0,		 0, 	0,	 0x0000, 20, 0x03, 64,&SpecialType_Samsung_SLC_2K },
};

struct NandDevPar MakerFounderFlashNandTbl[]=
{
    { {0x9B, 0xF1, 0x00, 0x1D}, 1,     4,      64,    25,       1024,     984,   0x0000, 25, 0x03, 64,&SpecialType_Samsung_SLC_2K }, 
	{ {0xff, 0xff, 0xff, 0xff}, 0,	   0,		0,	   0,		 0, 	0,	 0x0000, 20, 0x03, 64,&SpecialType_Samsung_SLC_2K },
};

