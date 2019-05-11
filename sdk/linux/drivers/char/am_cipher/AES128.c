#include <linux/string.h>
#include <linux/types.h>
#include "AES128.h"
#include <linux/am7x_mconfig.h>

struct AES_Decryptor {
    uint8_t key[16 + 16 * 10];
    uint8_t iv[ 16 ];
    uint8_t last_aes_str[ 16 ];
    int n_excess_bytes_last_time;

	uint8_t *out_cur;
	uint8_t *in_cur;
} AES_Decryptor;


#if CONFIG_AM_CHIP_MINOR == 8268
#include <linux/device.h>

static __inline__ unsigned long ___arch__libswab32(unsigned long x)
{
	__asm__(
	"	wsbh	%0, %1			\n"
	"	rotr	%0, %0, 16		\n"
	: "=r" (x)
	: "r" (x));

	return x;
}
#define be32_to_le32(x)	___arch__libswab32(x)

#if 1//MODULE_CONFIG_EZWIRE_ENABLE
#define USE_DRQ 
#endif

#ifdef USE_DRQ
#include <dma.h>
static int init_flag=0;
static struct semaphore sem_dec;

static irqreturn_t dma_isr(int irq, void *dev_id)
{
	int dma_no=6;
	unsigned int DMA_IRQPD_value;

	DMA_IRQPD_value=act_readl(DMA_IRQPD);
	DMA_IRQPD_value &= 0x00003c00;

	if(0==DMA_IRQPD_value)
		return IRQ_NONE;

	act_writel(DMA_IRQPD_value,DMA_IRQPD);

	up(&sem_dec);
	
	return IRQ_HANDLED;
}
#endif

int AES_DecryptInit(uint32_t *key, uint32_t *iv)
{
	act_writel(act_readl(CMU_DEVRST2)&(~0x100), CMU_DEVRST2);		//AES reset 
	act_writel(act_readl(CMU_DEVRST2)|0x100, CMU_DEVRST2);		//AES reset 
	act_writel(act_readl(0xb001007c)|0x10, 0xb001007c); 			//AES enable
	act_writel(0x1,AES_CTL);  
	
	act_writel(be32_to_le32(key[3]),AES_KEY_P0);
	act_writel(be32_to_le32(key[2]),AES_KEY_P1);
	act_writel(be32_to_le32(key[1]),AES_KEY_P2);
	act_writel(be32_to_le32(key[0]),AES_KEY_P3);
	act_writel(be32_to_le32(iv[3]),AES_CTR_LOW);
	act_writel(be32_to_le32(iv[2]),AES_CTR_HIG);
	act_writel(be32_to_le32(iv[1]),AES_RIV_LOW);
	act_writel(be32_to_le32(iv[0]),AES_RIV_HIG);
	
	act_writel(0x13070d,AES_CTL);  //enable aes DMA mode
	act_writel(1,CMU_DMACLK);	// en DMA CLK

	act_writel(0x008001d8,DMA_MODE6);
	act_writel(0x1020800c,DMA_SRC6);

	act_writel(0x01d80080,DMA_MODE5);
	act_writel(0x10208008,DMA_DST5);

//	act_writel((act_readl(SDR_PRIORITY)& 0xfffffff) |0x80000000,SDR_PRIORITY);

	AES_Decryptor.n_excess_bytes_last_time = 0;

#ifdef USE_DRQ
	if(0==init_flag)
	{
		init_flag=1;
		request_irq(IRQ_DMA, dma_isr,IRQF_SHARED|IRQF_DISABLED, "aes","dma_aes");
		sema_init(&sem_dec, 0);
	}

	am_set_dma_irq(6, 1, 0);
	act_writel(0,DMA_TIMEOUT);
#endif

	return 0;
}

int AES_DecryptBlock_Finish(int timeout)
{
	int i,j;
	unsigned int value;
	uint32_t *intdata;
	uint32_t *intout;
#ifdef USE_DRQ
	long jiffies = msecs_to_jiffies(timeout);
	if(down_timeout(&sem_dec, jiffies))
		return 0;
#endif
	
	int res=AES_Decryptor.n_excess_bytes_last_time;

	if(res)
	{
		intdata = (uint32_t *)AES_Decryptor.last_aes_str;
		intout = (uint32_t *)AES_Decryptor.last_aes_str ; 

		act_writel((act_readl(AES_CTL) & ~0xf)|0x1,AES_CTL);	//cpu mode

		memcpy(AES_Decryptor.last_aes_str,AES_Decryptor.in_cur,res);
		memset(AES_Decryptor.last_aes_str+res,0,16-res);
		
		act_writel(*intdata++,AES_TEXT_IN);
		act_writel(*intdata++,AES_TEXT_IN);
		act_writel(*intdata++,AES_TEXT_IN);
		act_writel(*intdata++,AES_TEXT_IN);

		for (j = 0; j < 32; j++)
		{
			value=act_readl(0xb0208004);
			value+=1;
			act_writel(value,0xb0208004);
		}

		*intout++=act_readl(AES_TEXT_OUT);
		*intout++=act_readl(AES_TEXT_OUT);
		*intout++=act_readl(AES_TEXT_OUT);
		*intout++=act_readl(AES_TEXT_OUT);

		memcpy(AES_Decryptor.out_cur,AES_Decryptor.last_aes_str,res);
		dma_cache_wback_inv((uint32_t)(AES_Decryptor.out_cur) &~MASK_COST_Read,res);
	}

	return 1;
}


int AES_DecryptBlock(uint8_t *data, int len, uint8_t *out)
{
	unsigned int ret1,ret2;
	int cnt,res;	
	int i,j;
	unsigned int value;
	uint32_t *intdata;
	uint32_t *intout;	

	if (AES_Decryptor.n_excess_bytes_last_time > 0) 
	{
		j = len >= (16 - AES_Decryptor.n_excess_bytes_last_time) ? (16 - AES_Decryptor.n_excess_bytes_last_time) : (len);
		
		for (i = 0; i < j; ++i) 
		{
			out[i] = data[i] ^ AES_Decryptor.last_aes_str[AES_Decryptor.n_excess_bytes_last_time + i];
		}
		
		dma_cache_wback_inv((uint32_t)(out) &~MASK_COST_Read,j);

		data += j;
		len -= j;
		out += j;
		/* no need to add iv, we did it last time */
		AES_Decryptor.n_excess_bytes_last_time += j;
		AES_Decryptor.n_excess_bytes_last_time &= 0xf;
	}

	res = len & 0xf;
	AES_Decryptor.n_excess_bytes_last_time = res;
	if(res)
	{
		cnt =len/16*16;
		AES_Decryptor.in_cur= data+cnt;
		AES_Decryptor.out_cur=out+cnt;
	}
	else
		cnt =len;

	if(cnt)
	{

		act_writel((act_readl(AES_CTL) & ~0xf)|0xd,AES_CTL);	//dma mode

		dma_cache_wback_inv((uint32_t)((uint32_t)data &~MASK_COST_Read),cnt);
		dma_cache_wback_inv((uint32_t)((uint32_t)out &~MASK_COST_Read),cnt);

		act_writel(((uint32_t)out)&0x1fffffff,DMA_DST6);
		act_writel(cnt,DMA_CNT6);
		act_writel(0x00000001,DMA_CMD6); 

		act_writel(((uint32_t)data)&0x1fffffff,DMA_SRC5);
		act_writel(cnt,DMA_CNT5);
		act_writel(0x00000001,DMA_CMD5); 

#ifndef USE_DRQ
		do{
			ret1=act_readl(DMA_CMD5)&1;
			ret2=act_readl(DMA_CMD6)&1;
		}while(ret1||ret2);
#endif
	}

	return 0;
}
#else
static uint8_t AES_ConvertTable[ 256 ] = {
    0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76,
    0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0, 0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0,
    0xB7, 0xFD, 0x93, 0x26, 0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15,
    0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2, 0xEB, 0x27, 0xB2, 0x75,
    0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0, 0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84,
    0x53, 0xD1, 0x00, 0xED, 0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF,
    0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F, 0x50, 0x3C, 0x9F, 0xA8,
    0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5, 0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2,
    0xCD, 0x0C, 0x13, 0xEC, 0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73,
    0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14, 0xDE, 0x5E, 0x0B, 0xDB,
    0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C, 0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79,
    0xE7, 0xC8, 0x37, 0x6D, 0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08,
    0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F, 0x4B, 0xBD, 0x8B, 0x8A,
    0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E, 0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E,
    0xE1, 0xF8, 0x98, 0x11, 0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF,
    0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16
};

# define AES_ConvertByte(x) (AES_ConvertTable[x])


// all inputs have 16 bytes memory
int32_t AES_Decrypt(uint8_t *iv, uint8_t *out, uint8_t *key)
{
    uint32_t vv1, vv2, vv3, vv4, vv5, vv6, vv7;
    uint32_t vvv1, vvv2;
    uint32_t *p1;
    uint32_t *p2;
    uint32_t *p3 = (uint32_t*)(key + 0x10);
    p1 = (uint32_t*)iv;
    p2 = (uint32_t*)key;

    vv1 = *p1 ^ *p2;
    p1++; p2++;
    vv2 = *p1 ^ *p2;
    p1++; p2++;
    vv3 = *p1 ^ *p2;
    p1++; p2++;
    vv4 = *p1 ^ *p2;
  
    ///////////////////////////
    vvv1 = AES_ConvertByte(vv1 & 0xff);
    vvv1 ^= AES_ConvertByte((vv2 >> 8) & 0xff) << 0x08;
    vvv1 ^= AES_ConvertByte((vv3 >> 0x10) & 0xff) << 0x10;
    vvv1 ^= AES_ConvertByte(vv4 >> 0x18) << 0x18;


    vvv2 = AES_ConvertByte(vv2 & 0xff);
    vvv2 ^= AES_ConvertByte((vv3 >> 8) & 0xff) << 8;
    vvv2 ^= AES_ConvertByte((vv4 >> 0x10) & 0xff) << 0x10;
    vvv2 ^= AES_ConvertByte(vv1 >> 0x18) << 0x18;


    vv5 = AES_ConvertByte(vv3 & 0xff);
    vv5 ^= AES_ConvertByte((vv4 >> 8) & 0xff) << 8;
    vv5 ^= AES_ConvertByte((vv1 >> 0x10) & 0xff) << 0x10;
    vv2 >>= 0x10;
    vv5 ^= AES_ConvertByte((vv2 >> 8) & 0xff) << 0x18;
    vv4 = AES_ConvertByte(vv4 & 0xff);
    vv4 ^= AES_ConvertByte((vv1 >> 8) & 0xff) << 8;

    vv1 = vvv1;
    vv4 ^= AES_ConvertByte(vv2 & 0xff) << 0x10;
    vv2 = vvv2;
    vv4 ^= AES_ConvertByte(vv3 >> 0x18) << 0x18;

    vv3 = vv5;
    vv5 &= 0x80808080;
    vv5 -= (vv5 >> 7);
    vv6 = (vv3 << 1) & 0xfefefefe;
    vv5 &= 0x1b1b1b1b;
    vv7 = vv3;
    vv5 ^= vv6;
    vv3 ^= vv5;
    vv3 = (vv3 << 0x18) | (vv3 >> 0x08);
    vv3 ^= vv5;
    vv7 = (vv7 << 0x10) | (vv7 >> 0x10);
    vv3 ^= vv7;
    vv7 = (vv7 << 0x18) | (vv7 >> 0x08);
    vv3 ^= vv7;

    vv5 = vv4 & 0x80808080;
    vv5 -= (vv5 >> 7);
    vv6 = (vv4 << 1) & 0xfefefefe;
    vv5 &= 0x1b1b1b1b;
    vv7 = vv4;
    vv5 ^= vv6;
    vv4 ^= vv5;
    vv4 = (vv4 << 0x18) | (vv4 >> 0x08);
    vv4 ^= vv5;
    vv7 = (vv7 << 0x10) | (vv7 >> 0x10);
    vv4 ^= vv7;
    vv7 = (vv7 << 0x18) | (vv7 >> 0x08);
    vv4 ^= vv7;

    vv5 = vv1 & 0x80808080;
    vv5 -= (vv5 >> 7);
    vv6 = (vv1 << 1) & 0xfefefefe;
    vv5 &= 0x1b1b1b1b;
    vv7 = vv1;
    vv5 ^= vv6;
    vv1 ^= vv5;
    vv1 = (vv1 << 0x18) | (vv1 >> 0x08);
    vv1 ^= vv5;
    vv7 = (vv7 << 0x10) | (vv7 >> 0x10);
    vv1 ^= vv7;
    vv7 = (vv7 << 0x18) | (vv7 >> 0x08);
    vv1 ^= vv7;
    vv5 = vv2;
    vv5 &= 0x80808080;

    vv5 -= (vv5 >> 7);
    vv6 = (vv2 << 1) & 0xfefefefe;
    vv5 &= 0x1b1b1b1b;
    vv7 = vv2;
    vv5 ^= vv6;
    vv2 ^= vv5;
    vv2 = (vv2 << 0x18) | (vv2 >> 0x08);
    vv2 ^= vv5;
    vv7 = (vv7 << 0x10) | (vv7 >> 0x10);
    vv2 ^= vv7;
    vv7 = (vv7 << 0x18) | (vv7 >> 0x08);
    vv2 ^= vv7;

    // vv1 ^= *(uint32_t *)(arg10 + 0x00);
    // vv2 ^= *(uint32_t *)(arg10 + 0x04);
    // vv3 ^= *(uint32_t *)(arg10 + 0x08);
    // vv4 ^= *(uint32_t *)(arg10 + 0x0c);
    vv1 ^= *p3++;
    vv2 ^= *p3++;
    vv3 ^= *p3++;
    vv4 ^= *p3++;

    ///////////////////////////
    vvv1 = AES_ConvertByte(vv1 & 0xff);
    vvv1 ^= AES_ConvertByte((vv2 >> 8) & 0xff) << 0x08;
    vvv1 ^= AES_ConvertByte((vv3 >> 0x10) & 0xff) << 0x10;
    vvv1 ^= AES_ConvertByte(vv4 >> 0x18) << 0x18;


    vvv2 = AES_ConvertByte(vv2 & 0xff);
    vvv2 ^= AES_ConvertByte((vv3 >> 8) & 0xff) << 8;
    vvv2 ^= AES_ConvertByte((vv4 >> 0x10) & 0xff) << 0x10;
    vvv2 ^= AES_ConvertByte(vv1 >> 0x18) << 0x18;


    vv5 = AES_ConvertByte(vv3 & 0xff);
    vv5 ^= AES_ConvertByte((vv4 >> 8) & 0xff) << 8;
    vv5 ^= AES_ConvertByte((vv1 >> 0x10) & 0xff) << 0x10;
    vv2 >>= 0x10;
    vv5 ^= AES_ConvertByte((vv2 >> 8) & 0xff) << 0x18;
    vv4 = AES_ConvertByte(vv4 & 0xff);
    vv4 ^= AES_ConvertByte((vv1 >> 8) & 0xff) << 8;

    vv1 = vvv1;
    vv4 ^= AES_ConvertByte(vv2 & 0xff) << 0x10;
    vv2 = vvv2;
    vv4 ^= AES_ConvertByte(vv3 >> 0x18) << 0x18;

    vv3 = vv5;
    vv5 &= 0x80808080;
    vv5 -= (vv5 >> 7);
    vv6 = (vv3 << 1) & 0xfefefefe;
    vv5 &= 0x1b1b1b1b;
    vv7 = vv3;
    vv5 ^= vv6;
    vv3 ^= vv5;
    vv3 = (vv3 << 0x18) | (vv3 >> 0x08);
    vv3 ^= vv5;
    vv7 = (vv7 << 0x10) | (vv7 >> 0x10);
    vv3 ^= vv7;
    vv7 = (vv7 << 0x18) | (vv7 >> 0x08);
    vv3 ^= vv7;

    vv5 = vv4 & 0x80808080;
    vv5 -= (vv5 >> 7);
    vv6 = (vv4 << 1) & 0xfefefefe;
    vv5 &= 0x1b1b1b1b;
    vv7 = vv4;
    vv5 ^= vv6;
    vv4 ^= vv5;
    vv4 = (vv4 << 0x18) | (vv4 >> 0x08);
    vv4 ^= vv5;
    vv7 = (vv7 << 0x10) | (vv7 >> 0x10);
    vv4 ^= vv7;
    vv7 = (vv7 << 0x18) | (vv7 >> 0x08);
    vv4 ^= vv7;

    vv5 = vv1 & 0x80808080;
    vv5 -= (vv5 >> 7);
    vv6 = (vv1 << 1) & 0xfefefefe;
    vv5 &= 0x1b1b1b1b;
    vv7 = vv1;
    vv5 ^= vv6;
    vv1 ^= vv5;
    vv1 = (vv1 << 0x18) | (vv1 >> 0x08);
    vv1 ^= vv5;
    vv7 = (vv7 << 0x10) | (vv7 >> 0x10);
    vv1 ^= vv7;
    vv7 = (vv7 << 0x18) | (vv7 >> 0x08);
    vv1 ^= vv7;
    vv5 = vv2;
    vv5 &= 0x80808080;

    vv5 -= (vv5 >> 7);
    vv6 = (vv2 << 1) & 0xfefefefe;
    vv5 &= 0x1b1b1b1b;
    vv7 = vv2;
    vv5 ^= vv6;
    vv2 ^= vv5;
    vv2 = (vv2 << 0x18) | (vv2 >> 0x08);
    vv2 ^= vv5;
    vv7 = (vv7 << 0x10) | (vv7 >> 0x10);
    vv2 ^= vv7;
    vv7 = (vv7 << 0x18) | (vv7 >> 0x08);
    vv2 ^= vv7;

    vv1 ^= *p3++;
    vv2 ^= *p3++;
    vv3 ^= *p3++;
    vv4 ^= *p3++;
    ///////////////////////////
    vvv1 = AES_ConvertByte(vv1 & 0xff);
    vvv1 ^= AES_ConvertByte((vv2 >> 8) & 0xff) << 0x08;
    vvv1 ^= AES_ConvertByte((vv3 >> 0x10) & 0xff) << 0x10;
    vvv1 ^= AES_ConvertByte(vv4 >> 0x18) << 0x18;


    vvv2 = AES_ConvertByte(vv2 & 0xff);
    vvv2 ^= AES_ConvertByte((vv3 >> 8) & 0xff) << 8;
    vvv2 ^= AES_ConvertByte((vv4 >> 0x10) & 0xff) << 0x10;
    vvv2 ^= AES_ConvertByte(vv1 >> 0x18) << 0x18;


    vv5 = AES_ConvertByte(vv3 & 0xff);
    vv5 ^= AES_ConvertByte((vv4 >> 8) & 0xff) << 8;
    vv5 ^= AES_ConvertByte((vv1 >> 0x10) & 0xff) << 0x10;
    vv2 >>= 0x10;
    vv5 ^= AES_ConvertByte((vv2 >> 8) & 0xff) << 0x18;
    vv4 = AES_ConvertByte(vv4 & 0xff);
    vv4 ^= AES_ConvertByte((vv1 >> 8) & 0xff) << 8;

    vv1 = vvv1;
    vv4 ^= AES_ConvertByte(vv2 & 0xff) << 0x10;
    vv2 = vvv2;
    vv4 ^= AES_ConvertByte(vv3 >> 0x18) << 0x18;

    vv3 = vv5;
    vv5 &= 0x80808080;
    vv5 -= (vv5 >> 7);
    vv6 = (vv3 << 1) & 0xfefefefe;
    vv5 &= 0x1b1b1b1b;
    vv7 = vv3;
    vv5 ^= vv6;
    vv3 ^= vv5;
    vv3 = (vv3 << 0x18) | (vv3 >> 0x08);
    vv3 ^= vv5;
    vv7 = (vv7 << 0x10) | (vv7 >> 0x10);
    vv3 ^= vv7;
    vv7 = (vv7 << 0x18) | (vv7 >> 0x08);
    vv3 ^= vv7;

    vv5 = vv4 & 0x80808080;
    vv5 -= (vv5 >> 7);
    vv6 = (vv4 << 1) & 0xfefefefe;
    vv5 &= 0x1b1b1b1b;
    vv7 = vv4;
    vv5 ^= vv6;
    vv4 ^= vv5;
    vv4 = (vv4 << 0x18) | (vv4 >> 0x08);
    vv4 ^= vv5;
    vv7 = (vv7 << 0x10) | (vv7 >> 0x10);
    vv4 ^= vv7;
    vv7 = (vv7 << 0x18) | (vv7 >> 0x08);
    vv4 ^= vv7;

    vv5 = vv1 & 0x80808080;
    vv5 -= (vv5 >> 7);
    vv6 = (vv1 << 1) & 0xfefefefe;
    vv5 &= 0x1b1b1b1b;
    vv7 = vv1;
    vv5 ^= vv6;
    vv1 ^= vv5;
    vv1 = (vv1 << 0x18) | (vv1 >> 0x08);
    vv1 ^= vv5;
    vv7 = (vv7 << 0x10) | (vv7 >> 0x10);
    vv1 ^= vv7;
    vv7 = (vv7 << 0x18) | (vv7 >> 0x08);
    vv1 ^= vv7;
    vv5 = vv2;
    vv5 &= 0x80808080;

    vv5 -= (vv5 >> 7);
    vv6 = (vv2 << 1) & 0xfefefefe;
    vv5 &= 0x1b1b1b1b;
    vv7 = vv2;
    vv5 ^= vv6;
    vv2 ^= vv5;
    vv2 = (vv2 << 0x18) | (vv2 >> 0x08);
    vv2 ^= vv5;
    vv7 = (vv7 << 0x10) | (vv7 >> 0x10);
    vv2 ^= vv7;
    vv7 = (vv7 << 0x18) | (vv7 >> 0x08);
    vv2 ^= vv7;

    vv1 ^= *p3++;
    vv2 ^= *p3++;
    vv3 ^= *p3++;
    vv4 ^= *p3++;
    ///////////////////////////
    vvv1 = AES_ConvertByte(vv1 & 0xff);
    vvv1 ^= AES_ConvertByte((vv2 >> 8) & 0xff) << 0x08;
    vvv1 ^= AES_ConvertByte((vv3 >> 0x10) & 0xff) << 0x10;
    vvv1 ^= AES_ConvertByte(vv4 >> 0x18) << 0x18;


    vvv2 = AES_ConvertByte(vv2 & 0xff);
    vvv2 ^= AES_ConvertByte((vv3 >> 8) & 0xff) << 8;
    vvv2 ^= AES_ConvertByte((vv4 >> 0x10) & 0xff) << 0x10;
    vvv2 ^= AES_ConvertByte(vv1 >> 0x18) << 0x18;


    vv5 = AES_ConvertByte(vv3 & 0xff);
    vv5 ^= AES_ConvertByte((vv4 >> 8) & 0xff) << 8;
    vv5 ^= AES_ConvertByte((vv1 >> 0x10) & 0xff) << 0x10;
    vv2 >>= 0x10;
    vv5 ^= AES_ConvertByte((vv2 >> 8) & 0xff) << 0x18;
    vv4 = AES_ConvertByte(vv4 & 0xff);
    vv4 ^= AES_ConvertByte((vv1 >> 8) & 0xff) << 8;

    vv1 = vvv1;
    vv4 ^= AES_ConvertByte(vv2 & 0xff) << 0x10;
    vv2 = vvv2;
    vv4 ^= AES_ConvertByte(vv3 >> 0x18) << 0x18;

    vv3 = vv5;
    vv5 &= 0x80808080;
    vv5 -= (vv5 >> 7);
    vv6 = (vv3 << 1) & 0xfefefefe;
    vv5 &= 0x1b1b1b1b;
    vv7 = vv3;
    vv5 ^= vv6;
    vv3 ^= vv5;
    vv3 = (vv3 << 0x18) | (vv3 >> 0x08);
    vv3 ^= vv5;
    vv7 = (vv7 << 0x10) | (vv7 >> 0x10);
    vv3 ^= vv7;
    vv7 = (vv7 << 0x18) | (vv7 >> 0x08);
    vv3 ^= vv7;

    vv5 = vv4 & 0x80808080;
    vv5 -= (vv5 >> 7);
    vv6 = (vv4 << 1) & 0xfefefefe;
    vv5 &= 0x1b1b1b1b;
    vv7 = vv4;
    vv5 ^= vv6;
    vv4 ^= vv5;
    vv4 = (vv4 << 0x18) | (vv4 >> 0x08);
    vv4 ^= vv5;
    vv7 = (vv7 << 0x10) | (vv7 >> 0x10);
    vv4 ^= vv7;
    vv7 = (vv7 << 0x18) | (vv7 >> 0x08);
    vv4 ^= vv7;

    vv5 = vv1 & 0x80808080;
    vv5 -= (vv5 >> 7);
    vv6 = (vv1 << 1) & 0xfefefefe;
    vv5 &= 0x1b1b1b1b;
    vv7 = vv1;
    vv5 ^= vv6;
    vv1 ^= vv5;
    vv1 = (vv1 << 0x18) | (vv1 >> 0x08);
    vv1 ^= vv5;
    vv7 = (vv7 << 0x10) | (vv7 >> 0x10);
    vv1 ^= vv7;
    vv7 = (vv7 << 0x18) | (vv7 >> 0x08);
    vv1 ^= vv7;
    vv5 = vv2;
    vv5 &= 0x80808080;

    vv5 -= (vv5 >> 7);
    vv6 = (vv2 << 1) & 0xfefefefe;
    vv5 &= 0x1b1b1b1b;
    vv7 = vv2;
    vv5 ^= vv6;
    vv2 ^= vv5;
    vv2 = (vv2 << 0x18) | (vv2 >> 0x08);
    vv2 ^= vv5;
    vv7 = (vv7 << 0x10) | (vv7 >> 0x10);
    vv2 ^= vv7;
    vv7 = (vv7 << 0x18) | (vv7 >> 0x08);
    vv2 ^= vv7;

    vv1 ^= *p3++;
    vv2 ^= *p3++;
    vv3 ^= *p3++;
    vv4 ^= *p3++;
    ///////////////////////////
    vvv1 = AES_ConvertByte(vv1 & 0xff);
    vvv1 ^= AES_ConvertByte((vv2 >> 8) & 0xff) << 0x08;
    vvv1 ^= AES_ConvertByte((vv3 >> 0x10) & 0xff) << 0x10;
    vvv1 ^= AES_ConvertByte(vv4 >> 0x18) << 0x18;


    vvv2 = AES_ConvertByte(vv2 & 0xff);
    vvv2 ^= AES_ConvertByte((vv3 >> 8) & 0xff) << 8;
    vvv2 ^= AES_ConvertByte((vv4 >> 0x10) & 0xff) << 0x10;
    vvv2 ^= AES_ConvertByte(vv1 >> 0x18) << 0x18;


    vv5 = AES_ConvertByte(vv3 & 0xff);
    vv5 ^= AES_ConvertByte((vv4 >> 8) & 0xff) << 8;
    vv5 ^= AES_ConvertByte((vv1 >> 0x10) & 0xff) << 0x10;
    vv2 >>= 0x10;
    vv5 ^= AES_ConvertByte((vv2 >> 8) & 0xff) << 0x18;
    vv4 = AES_ConvertByte(vv4 & 0xff);
    vv4 ^= AES_ConvertByte((vv1 >> 8) & 0xff) << 8;

    vv1 = vvv1;
    vv4 ^= AES_ConvertByte(vv2 & 0xff) << 0x10;
    vv2 = vvv2;
    vv4 ^= AES_ConvertByte(vv3 >> 0x18) << 0x18;

    vv3 = vv5;
    vv5 &= 0x80808080;
    vv5 -= (vv5 >> 7);
    vv6 = (vv3 << 1) & 0xfefefefe;
    vv5 &= 0x1b1b1b1b;
    vv7 = vv3;
    vv5 ^= vv6;
    vv3 ^= vv5;
    vv3 = (vv3 << 0x18) | (vv3 >> 0x08);
    vv3 ^= vv5;
    vv7 = (vv7 << 0x10) | (vv7 >> 0x10);
    vv3 ^= vv7;
    vv7 = (vv7 << 0x18) | (vv7 >> 0x08);
    vv3 ^= vv7;

    vv5 = vv4 & 0x80808080;
    vv5 -= (vv5 >> 7);
    vv6 = (vv4 << 1) & 0xfefefefe;
    vv5 &= 0x1b1b1b1b;
    vv7 = vv4;
    vv5 ^= vv6;
    vv4 ^= vv5;
    vv4 = (vv4 << 0x18) | (vv4 >> 0x08);
    vv4 ^= vv5;
    vv7 = (vv7 << 0x10) | (vv7 >> 0x10);
    vv4 ^= vv7;
    vv7 = (vv7 << 0x18) | (vv7 >> 0x08);
    vv4 ^= vv7;

    vv5 = vv1 & 0x80808080;
    vv5 -= (vv5 >> 7);
    vv6 = (vv1 << 1) & 0xfefefefe;
    vv5 &= 0x1b1b1b1b;
    vv7 = vv1;
    vv5 ^= vv6;
    vv1 ^= vv5;
    vv1 = (vv1 << 0x18) | (vv1 >> 0x08);
    vv1 ^= vv5;
    vv7 = (vv7 << 0x10) | (vv7 >> 0x10);
    vv1 ^= vv7;
    vv7 = (vv7 << 0x18) | (vv7 >> 0x08);
    vv1 ^= vv7;
    vv5 = vv2;
    vv5 &= 0x80808080;

    vv5 -= (vv5 >> 7);
    vv6 = (vv2 << 1) & 0xfefefefe;
    vv5 &= 0x1b1b1b1b;
    vv7 = vv2;
    vv5 ^= vv6;
    vv2 ^= vv5;
    vv2 = (vv2 << 0x18) | (vv2 >> 0x08);
    vv2 ^= vv5;
    vv7 = (vv7 << 0x10) | (vv7 >> 0x10);
    vv2 ^= vv7;
    vv7 = (vv7 << 0x18) | (vv7 >> 0x08);
    vv2 ^= vv7;

    vv1 ^= *p3++;
    vv2 ^= *p3++;
    vv3 ^= *p3++;
    vv4 ^= *p3++;
    ///////////////////////////
    vvv1 = AES_ConvertByte(vv1 & 0xff);
    vvv1 ^= AES_ConvertByte((vv2 >> 8) & 0xff) << 0x08;
    vvv1 ^= AES_ConvertByte((vv3 >> 0x10) & 0xff) << 0x10;
    vvv1 ^= AES_ConvertByte(vv4 >> 0x18) << 0x18;


    vvv2 = AES_ConvertByte(vv2 & 0xff);
    vvv2 ^= AES_ConvertByte((vv3 >> 8) & 0xff) << 8;
    vvv2 ^= AES_ConvertByte((vv4 >> 0x10) & 0xff) << 0x10;
    vvv2 ^= AES_ConvertByte(vv1 >> 0x18) << 0x18;


    vv5 = AES_ConvertByte(vv3 & 0xff);
    vv5 ^= AES_ConvertByte((vv4 >> 8) & 0xff) << 8;
    vv5 ^= AES_ConvertByte((vv1 >> 0x10) & 0xff) << 0x10;
    vv2 >>= 0x10;
    vv5 ^= AES_ConvertByte((vv2 >> 8) & 0xff) << 0x18;
    vv4 = AES_ConvertByte(vv4 & 0xff);
    vv4 ^= AES_ConvertByte((vv1 >> 8) & 0xff) << 8;

    vv1 = vvv1;
    vv4 ^= AES_ConvertByte(vv2 & 0xff) << 0x10;
    vv2 = vvv2;
    vv4 ^= AES_ConvertByte(vv3 >> 0x18) << 0x18;

    vv3 = vv5;
    vv5 &= 0x80808080;
    vv5 -= (vv5 >> 7);
    vv6 = (vv3 << 1) & 0xfefefefe;
    vv5 &= 0x1b1b1b1b;
    vv7 = vv3;
    vv5 ^= vv6;
    vv3 ^= vv5;
    vv3 = (vv3 << 0x18) | (vv3 >> 0x08);
    vv3 ^= vv5;
    vv7 = (vv7 << 0x10) | (vv7 >> 0x10);
    vv3 ^= vv7;
    vv7 = (vv7 << 0x18) | (vv7 >> 0x08);
    vv3 ^= vv7;

    vv5 = vv4 & 0x80808080;
    vv5 -= (vv5 >> 7);
    vv6 = (vv4 << 1) & 0xfefefefe;
    vv5 &= 0x1b1b1b1b;
    vv7 = vv4;
    vv5 ^= vv6;
    vv4 ^= vv5;
    vv4 = (vv4 << 0x18) | (vv4 >> 0x08);
    vv4 ^= vv5;
    vv7 = (vv7 << 0x10) | (vv7 >> 0x10);
    vv4 ^= vv7;
    vv7 = (vv7 << 0x18) | (vv7 >> 0x08);
    vv4 ^= vv7;

    vv5 = vv1 & 0x80808080;
    vv5 -= (vv5 >> 7);
    vv6 = (vv1 << 1) & 0xfefefefe;
    vv5 &= 0x1b1b1b1b;
    vv7 = vv1;
    vv5 ^= vv6;
    vv1 ^= vv5;
    vv1 = (vv1 << 0x18) | (vv1 >> 0x08);
    vv1 ^= vv5;
    vv7 = (vv7 << 0x10) | (vv7 >> 0x10);
    vv1 ^= vv7;
    vv7 = (vv7 << 0x18) | (vv7 >> 0x08);
    vv1 ^= vv7;
    vv5 = vv2;
    vv5 &= 0x80808080;

    vv5 -= (vv5 >> 7);
    vv6 = (vv2 << 1) & 0xfefefefe;
    vv5 &= 0x1b1b1b1b;
    vv7 = vv2;
    vv5 ^= vv6;
    vv2 ^= vv5;
    vv2 = (vv2 << 0x18) | (vv2 >> 0x08);
    vv2 ^= vv5;
    vv7 = (vv7 << 0x10) | (vv7 >> 0x10);
    vv2 ^= vv7;
    vv7 = (vv7 << 0x18) | (vv7 >> 0x08);
    vv2 ^= vv7;

    vv1 ^= *p3++;
    vv2 ^= *p3++;
    vv3 ^= *p3++;
    vv4 ^= *p3++;
    ///////////////////////////
    vvv1 = AES_ConvertByte(vv1 & 0xff);
    vvv1 ^= AES_ConvertByte((vv2 >> 8) & 0xff) << 0x08;
    vvv1 ^= AES_ConvertByte((vv3 >> 0x10) & 0xff) << 0x10;
    vvv1 ^= AES_ConvertByte(vv4 >> 0x18) << 0x18;


    vvv2 = AES_ConvertByte(vv2 & 0xff);
    vvv2 ^= AES_ConvertByte((vv3 >> 8) & 0xff) << 8;
    vvv2 ^= AES_ConvertByte((vv4 >> 0x10) & 0xff) << 0x10;
    vvv2 ^= AES_ConvertByte(vv1 >> 0x18) << 0x18;


    vv5 = AES_ConvertByte(vv3 & 0xff);
    vv5 ^= AES_ConvertByte((vv4 >> 8) & 0xff) << 8;
    vv5 ^= AES_ConvertByte((vv1 >> 0x10) & 0xff) << 0x10;
    vv2 >>= 0x10;
    vv5 ^= AES_ConvertByte((vv2 >> 8) & 0xff) << 0x18;
    vv4 = AES_ConvertByte(vv4 & 0xff);
    vv4 ^= AES_ConvertByte((vv1 >> 8) & 0xff) << 8;

    vv1 = vvv1;
    vv4 ^= AES_ConvertByte(vv2 & 0xff) << 0x10;
    vv2 = vvv2;
    vv4 ^= AES_ConvertByte(vv3 >> 0x18) << 0x18;

    vv3 = vv5;
    vv5 &= 0x80808080;
    vv5 -= (vv5 >> 7);
    vv6 = (vv3 << 1) & 0xfefefefe;
    vv5 &= 0x1b1b1b1b;
    vv7 = vv3;
    vv5 ^= vv6;
    vv3 ^= vv5;
    vv3 = (vv3 << 0x18) | (vv3 >> 0x08);
    vv3 ^= vv5;
    vv7 = (vv7 << 0x10) | (vv7 >> 0x10);
    vv3 ^= vv7;
    vv7 = (vv7 << 0x18) | (vv7 >> 0x08);
    vv3 ^= vv7;

    vv5 = vv4 & 0x80808080;
    vv5 -= (vv5 >> 7);
    vv6 = (vv4 << 1) & 0xfefefefe;
    vv5 &= 0x1b1b1b1b;
    vv7 = vv4;
    vv5 ^= vv6;
    vv4 ^= vv5;
    vv4 = (vv4 << 0x18) | (vv4 >> 0x08);
    vv4 ^= vv5;
    vv7 = (vv7 << 0x10) | (vv7 >> 0x10);
    vv4 ^= vv7;
    vv7 = (vv7 << 0x18) | (vv7 >> 0x08);
    vv4 ^= vv7;

    vv5 = vv1 & 0x80808080;
    vv5 -= (vv5 >> 7);
    vv6 = (vv1 << 1) & 0xfefefefe;
    vv5 &= 0x1b1b1b1b;
    vv7 = vv1;
    vv5 ^= vv6;
    vv1 ^= vv5;
    vv1 = (vv1 << 0x18) | (vv1 >> 0x08);
    vv1 ^= vv5;
    vv7 = (vv7 << 0x10) | (vv7 >> 0x10);
    vv1 ^= vv7;
    vv7 = (vv7 << 0x18) | (vv7 >> 0x08);
    vv1 ^= vv7;
    vv5 = vv2;
    vv5 &= 0x80808080;

    vv5 -= (vv5 >> 7);
    vv6 = (vv2 << 1) & 0xfefefefe;
    vv5 &= 0x1b1b1b1b;
    vv7 = vv2;
    vv5 ^= vv6;
    vv2 ^= vv5;
    vv2 = (vv2 << 0x18) | (vv2 >> 0x08);
    vv2 ^= vv5;
    vv7 = (vv7 << 0x10) | (vv7 >> 0x10);
    vv2 ^= vv7;
    vv7 = (vv7 << 0x18) | (vv7 >> 0x08);
    vv2 ^= vv7;

    vv1 ^= *p3++;
    vv2 ^= *p3++;
    vv3 ^= *p3++;
    vv4 ^= *p3++;

    ///////////////////////////
    vvv1 = AES_ConvertByte(vv1 & 0xff);
    vvv1 ^= AES_ConvertByte((vv2 >> 8) & 0xff) << 0x08;
    vvv1 ^= AES_ConvertByte((vv3 >> 0x10) & 0xff) << 0x10;
    vvv1 ^= AES_ConvertByte(vv4 >> 0x18) << 0x18;


    vvv2 = AES_ConvertByte(vv2 & 0xff);
    vvv2 ^= AES_ConvertByte((vv3 >> 8) & 0xff) << 8;
    vvv2 ^= AES_ConvertByte((vv4 >> 0x10) & 0xff) << 0x10;
    vvv2 ^= AES_ConvertByte(vv1 >> 0x18) << 0x18;


    vv5 = AES_ConvertByte(vv3 & 0xff);
    vv5 ^= AES_ConvertByte((vv4 >> 8) & 0xff) << 8;
    vv5 ^= AES_ConvertByte((vv1 >> 0x10) & 0xff) << 0x10;
    vv2 >>= 0x10;
    vv5 ^= AES_ConvertByte((vv2 >> 8) & 0xff) << 0x18;
    vv4 = AES_ConvertByte(vv4 & 0xff);
    vv4 ^= AES_ConvertByte((vv1 >> 8) & 0xff) << 8;

    vv1 = vvv1;
    vv4 ^= AES_ConvertByte(vv2 & 0xff) << 0x10;
    vv2 = vvv2;
    vv4 ^= AES_ConvertByte(vv3 >> 0x18) << 0x18;

    vv3 = vv5;
    vv5 &= 0x80808080;
    vv5 -= (vv5 >> 7);
    vv6 = (vv3 << 1) & 0xfefefefe;
    vv5 &= 0x1b1b1b1b;
    vv7 = vv3;
    vv5 ^= vv6;
    vv3 ^= vv5;
    vv3 = (vv3 << 0x18) | (vv3 >> 0x08);
    vv3 ^= vv5;
    vv7 = (vv7 << 0x10) | (vv7 >> 0x10);
    vv3 ^= vv7;
    vv7 = (vv7 << 0x18) | (vv7 >> 0x08);
    vv3 ^= vv7;

    vv5 = vv4 & 0x80808080;
    vv5 -= (vv5 >> 7);
    vv6 = (vv4 << 1) & 0xfefefefe;
    vv5 &= 0x1b1b1b1b;
    vv7 = vv4;
    vv5 ^= vv6;
    vv4 ^= vv5;
    vv4 = (vv4 << 0x18) | (vv4 >> 0x08);
    vv4 ^= vv5;
    vv7 = (vv7 << 0x10) | (vv7 >> 0x10);
    vv4 ^= vv7;
    vv7 = (vv7 << 0x18) | (vv7 >> 0x08);
    vv4 ^= vv7;

    vv5 = vv1 & 0x80808080;
    vv5 -= (vv5 >> 7);
    vv6 = (vv1 << 1) & 0xfefefefe;
    vv5 &= 0x1b1b1b1b;
    vv7 = vv1;
    vv5 ^= vv6;
    vv1 ^= vv5;
    vv1 = (vv1 << 0x18) | (vv1 >> 0x08);
    vv1 ^= vv5;
    vv7 = (vv7 << 0x10) | (vv7 >> 0x10);
    vv1 ^= vv7;
    vv7 = (vv7 << 0x18) | (vv7 >> 0x08);
    vv1 ^= vv7;
    vv5 = vv2;
    vv5 &= 0x80808080;

    vv5 -= (vv5 >> 7);
    vv6 = (vv2 << 1) & 0xfefefefe;
    vv5 &= 0x1b1b1b1b;
    vv7 = vv2;
    vv5 ^= vv6;
    vv2 ^= vv5;
    vv2 = (vv2 << 0x18) | (vv2 >> 0x08);
    vv2 ^= vv5;
    vv7 = (vv7 << 0x10) | (vv7 >> 0x10);
    vv2 ^= vv7;
    vv7 = (vv7 << 0x18) | (vv7 >> 0x08);
    vv2 ^= vv7;

    vv1 ^= *p3++;
    vv2 ^= *p3++;
    vv3 ^= *p3++;
    vv4 ^= *p3++;
    ///////////////////////////
    vvv1 = AES_ConvertByte(vv1 & 0xff);
    vvv1 ^= AES_ConvertByte((vv2 >> 8) & 0xff) << 0x08;
    vvv1 ^= AES_ConvertByte((vv3 >> 0x10) & 0xff) << 0x10;
    vvv1 ^= AES_ConvertByte(vv4 >> 0x18) << 0x18;


    vvv2 = AES_ConvertByte(vv2 & 0xff);
    vvv2 ^= AES_ConvertByte((vv3 >> 8) & 0xff) << 8;
    vvv2 ^= AES_ConvertByte((vv4 >> 0x10) & 0xff) << 0x10;
    vvv2 ^= AES_ConvertByte(vv1 >> 0x18) << 0x18;


    vv5 = AES_ConvertByte(vv3 & 0xff);
    vv5 ^= AES_ConvertByte((vv4 >> 8) & 0xff) << 8;
    vv5 ^= AES_ConvertByte((vv1 >> 0x10) & 0xff) << 0x10;
    vv2 >>= 0x10;
    vv5 ^= AES_ConvertByte((vv2 >> 8) & 0xff) << 0x18;
    vv4 = AES_ConvertByte(vv4 & 0xff);
    vv4 ^= AES_ConvertByte((vv1 >> 8) & 0xff) << 8;

    vv1 = vvv1;
    vv4 ^= AES_ConvertByte(vv2 & 0xff) << 0x10;
    vv2 = vvv2;
    vv4 ^= AES_ConvertByte(vv3 >> 0x18) << 0x18;

    vv3 = vv5;
    vv5 &= 0x80808080;
    vv5 -= (vv5 >> 7);
    vv6 = (vv3 << 1) & 0xfefefefe;
    vv5 &= 0x1b1b1b1b;
    vv7 = vv3;
    vv5 ^= vv6;
    vv3 ^= vv5;
    vv3 = (vv3 << 0x18) | (vv3 >> 0x08);
    vv3 ^= vv5;
    vv7 = (vv7 << 0x10) | (vv7 >> 0x10);
    vv3 ^= vv7;
    vv7 = (vv7 << 0x18) | (vv7 >> 0x08);
    vv3 ^= vv7;

    vv5 = vv4 & 0x80808080;
    vv5 -= (vv5 >> 7);
    vv6 = (vv4 << 1) & 0xfefefefe;
    vv5 &= 0x1b1b1b1b;
    vv7 = vv4;
    vv5 ^= vv6;
    vv4 ^= vv5;
    vv4 = (vv4 << 0x18) | (vv4 >> 0x08);
    vv4 ^= vv5;
    vv7 = (vv7 << 0x10) | (vv7 >> 0x10);
    vv4 ^= vv7;
    vv7 = (vv7 << 0x18) | (vv7 >> 0x08);
    vv4 ^= vv7;

    vv5 = vv1 & 0x80808080;
    vv5 -= (vv5 >> 7);
    vv6 = (vv1 << 1) & 0xfefefefe;
    vv5 &= 0x1b1b1b1b;
    vv7 = vv1;
    vv5 ^= vv6;
    vv1 ^= vv5;
    vv1 = (vv1 << 0x18) | (vv1 >> 0x08);
    vv1 ^= vv5;
    vv7 = (vv7 << 0x10) | (vv7 >> 0x10);
    vv1 ^= vv7;
    vv7 = (vv7 << 0x18) | (vv7 >> 0x08);
    vv1 ^= vv7;
    vv5 = vv2;
    vv5 &= 0x80808080;

    vv5 -= (vv5 >> 7);
    vv6 = (vv2 << 1) & 0xfefefefe;
    vv5 &= 0x1b1b1b1b;
    vv7 = vv2;
    vv5 ^= vv6;
    vv2 ^= vv5;
    vv2 = (vv2 << 0x18) | (vv2 >> 0x08);
    vv2 ^= vv5;
    vv7 = (vv7 << 0x10) | (vv7 >> 0x10);
    vv2 ^= vv7;
    vv7 = (vv7 << 0x18) | (vv7 >> 0x08);
    vv2 ^= vv7;

    vv1 ^= *p3++;
    vv2 ^= *p3++;
    vv3 ^= *p3++;
    vv4 ^= *p3++;




    vvv1 = AES_ConvertByte(vv1 & 0xff);
    vvv1 ^= AES_ConvertByte((vv2 >> 8) & 0xff) << 0x08;
    vvv1 ^= AES_ConvertByte((vv3 >> 0x10) & 0xff) << 0x10;
    vvv1 ^= AES_ConvertByte(vv4 >> 0x18) << 0x18;

    vvv2 = AES_ConvertByte(vv2 & 0xff);
    vv2 >>= 0x10;
    vvv2 ^= AES_ConvertByte((vv3 >> 8) & 0xff) << 8;
    vvv2 ^= AES_ConvertByte((vv4 >> 0x10) & 0xff) << 0x10;
    vvv2 ^= AES_ConvertByte(vv1 >> 0x18) << 0x18;

    vv5 = AES_ConvertByte(vv3 & 0xff);
    vv6 = AES_ConvertByte((vv4 >> 8) & 0xff) << 8;
    vv5 ^= vv6;
    vv6 = AES_ConvertByte((vv1 >> 0x10) & 0xff) << 0x10;
    vv5 ^= vv6;
    vv6 = AES_ConvertByte((vv2 >> 8) & 0xff) << 0x18;
    vv5 ^= vv6;

    vv4 = AES_ConvertByte(vv4 & 0xff);
    vv4 ^= AES_ConvertByte((vv1 >> 8) & 0xff) << 8;
    vv4 ^= AES_ConvertByte(vv2 & 0xff) << 0x10;
    vv4 ^= AES_ConvertByte(vv3 >> 0x18) << 0x18;

    p1 = (uint32_t *)out;
    *p1 = vvv1 ^ *p3;
    p1++; p3++;
    *p1 = vvv2 ^ *p3;
    p1++; p3++;
    *p1 = vv5 ^ *p3;
    p1++; p3++;
    *p1 = vv4 ^ *p3;

    return 0;
}


static uint8_t AES_ExtendKeyTable1[16 *4] = { /* we need 40 */
    0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00,
    0x10, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00,
    0x1B, 0x00, 0x00, 0x00, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};


// key should point to memory of size 16 + 16 * 10
void AES_ExtendKey(uint8_t *key)
{
    int i = 0;
    uint8_t *p = key;
    uint8_t s[4];
    uint8_t *extendKeyTable = AES_ConvertTable;

    for (i = 0; i < 0x0a; ++i, p += 0x10)
    {
        memcpy(s, p, 4);
        s[3] ^= extendKeyTable[p[12]];
        s[0] ^= extendKeyTable[p[13]];
        s[1] ^= extendKeyTable[p[14]];
        s[2] ^= extendKeyTable[p[15]];

        uint32_t *s1 = (uint32_t *)s;
        *s1 ^= *(uint32_t *)(AES_ExtendKeyTable1 + 4 * i);
        memcpy(p + 0x10, s, 4);
        *s1 ^= *(uint32_t *)(p + 0x04);
        memcpy(p + 0x14, s, 4);
        *s1 ^= *(uint32_t *)(p + 0x08);
        memcpy(p + 0x18, s, 4);
        *s1 ^= *(uint32_t *)(p + 0x0c);
        memcpy(p + 0x1c, s, 4);
    }
}


int AES_DecryptInit(uint8_t *key, uint8_t *iv)
{
	memset(&AES_Decryptor, 0, sizeof(AES_Decryptor));

	memcpy(AES_Decryptor.key, key, 16);

	memcpy(AES_Decryptor.iv, iv, 16);

	AES_Decryptor.n_excess_bytes_last_time = 0;

	AES_ExtendKey(AES_Decryptor.key);

	return 0;
}


int AES_DecryptBlock(uint8_t *data, int len, uint8_t *out)
{
    int i;
    int j;
    uint8_t *tmp;

    /* process excess bytes last time */
    if (AES_Decryptor.n_excess_bytes_last_time > 0) 
    {
        j = len >= (16 - AES_Decryptor.n_excess_bytes_last_time) ? (16 - AES_Decryptor.n_excess_bytes_last_time) : (len);
        for (i = 0; i < j; ++i) 
        {
            out[i] = data[i] ^ AES_Decryptor.last_aes_str[AES_Decryptor.n_excess_bytes_last_time + i];
        }
        data += j;
        len -= j;
        out += j;
        /* no need to add iv, we did it last time */
        AES_Decryptor.n_excess_bytes_last_time += j;
#if 0
        AES_Decryptor.n_excess_bytes_last_time %= 16;
#else
        AES_Decryptor.n_excess_bytes_last_time &= 0xf;
#endif
    }

    int cnt = len / 16;
    int res = len & 0xf;

    for (i = 0; i < cnt; ++i)
    {
    	uint32_t *p_out, *p_tmp, *p_txt;

        AES_Decrypt(AES_Decryptor.iv, AES_Decryptor.last_aes_str, AES_Decryptor.key);

        j = i << 4;
        tmp = data + j;

		//*(uint32_t*)(out + j + 0) = *(uint32_t*)(tmp + 0) ^ *(uint32_t*)(AES_Decryptor.last_aes_str + 0);
        p_out = (uint32_t *)(out + j + 0);
        p_tmp = (uint32_t *)(tmp + 0);
        p_txt = (uint32_t *)(AES_Decryptor.last_aes_str + 0);
        *p_out = *p_tmp ^ *p_txt;
		//*(uint32_t*)(out + j + 4) = *(uint32_t*)(tmp + 4) ^ *(uint32_t*)(AES_Decryptor.last_aes_str + 4);
        p_out = (uint32_t *)(out + j + 4);
        p_tmp = (uint32_t *)(tmp + 4);
        p_txt = (uint32_t *)(AES_Decryptor.last_aes_str + 4);
        *p_out = *p_tmp ^ *p_txt;
		//*(uint32_t*)(out + j + 8) = *(uint32_t*)(tmp + 8) ^ *(uint32_t*)(AES_Decryptor.last_aes_str + 8);
        p_out = (uint32_t *)(out + j + 8);
        p_tmp = (uint32_t *)(tmp + 8);
        p_txt = (uint32_t *)(AES_Decryptor.last_aes_str + 8);
        *p_out = *p_tmp ^ *p_txt;
		//*(uint32_t*)(out + j + 12) = *(uint32_t*)(tmp + 12) ^ *(uint32_t*)(AES_Decryptor.last_aes_str + 12);
        p_out = (uint32_t *)(out + j + 12);
        p_tmp = (uint32_t *)(tmp + 12);
        p_txt = (uint32_t *)(AES_Decryptor.last_aes_str + 12);
        *p_out = *p_tmp ^ *p_txt;

        /* renew iv */
        tmp = AES_Decryptor.iv + 15;
        while (0xff == (*tmp))
        {
            *tmp = 0x00;
            tmp--;
        }
        *tmp += 1;
    }

    if (res > 0)
    {
        AES_Decrypt(AES_Decryptor.iv, AES_Decryptor.last_aes_str, AES_Decryptor.key);
        AES_Decryptor.n_excess_bytes_last_time = res;
        tmp = data + 16 * cnt;
        out += (cnt << 4);
        for (j = 0; j < res; ++j)
        {
            out[j] = tmp[j] ^ AES_Decryptor.last_aes_str[j];
        }

        /* renew iv */
        tmp = AES_Decryptor.iv + 15;
        while (0xff == (*tmp))
        {
            *tmp = 0x00;
            tmp--;
        }
        *tmp += 1;
    }

    return 0;
}
int AES_DecryptBlock_Finish(int timeout)
{
	return 1;
}
#endif


