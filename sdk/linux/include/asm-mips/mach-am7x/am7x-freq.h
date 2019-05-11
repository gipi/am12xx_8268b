#ifndef AM7X_FREQ_H
#define AM7X_FREQ_H
/**
*@file am7x_freq.h
*
*@author yekai
*@date 2010-08-24
*@version 0.1
*/

#if !defined(CONFIG_AM_CHIP_ID)
#error unknown chip id!
#endif

/**
*@addtogroup frequency_lib_s
*@{
*/

#define REG_READ(__reg)      (*(volatile unsigned long *)(__reg))

#define _1K  1000
#define _1M  1000000

/**
*@brief get core pll
*
*@param[in] NULL
*@return system core pll, hz
*/
static inline unsigned long get_core_pll(void)
{
	unsigned long val = REG_READ(CMU_COREPLL);	

#if CONFIG_AM_CHIP_ID == 1203
	val >>= 2;
	val &= 0x3f;
	val *= 6;
#elif CONFIG_AM_CHIP_ID == 1211
	unsigned long bus_val = REG_READ(CMU_BUSCLK);
	unsigned int ssc_ensdm = val&(1<<24);
	unsigned int ssc_enss = val&(1<<1);

	if(bus_val&(1<<11)){	//DDRPLL
		val = REG_READ(CMU_DDRPLL);

		val >>=2;
		val &= 0x3f;
		val *= 8;
	}else{
		val >>= 2;
		val &= 0x7f;
		//spread
    	if(ssc_ensdm && ssc_enss)
	    	val += 6;
		val *= 6;
	}
#elif CONFIG_AM_CHIP_ID == 1220 || CONFIG_AM_CHIP_ID == 1213 
#if CONFIG_AM_CHIP_MINOR == 8268
	val >>= 1;
	val &= 0x3f;
	val *= 24;
#else
	val >>= 2;
	val &= 0x7f;
	val *= 6;
#endif
#endif
	return val*_1M;
}

/**
*@brief get core clock
*
*@param[in] NULL
*@return system core clock, hz
*/
static inline unsigned long get_core_clk(void)
{
	unsigned long val, div;
	
	val = REG_READ(CMU_BUSCLK);
	div = (val>>6) & 3;
	switch(div) {
	case 0: return 32 * _1K;
	case 1: return 24 * _1M;
	default: return get_core_pll();
	}
}

/**
*@brief get cpu clock
*
*@param[in] NULL
*@return cpu clock, hz
*/
static inline unsigned long am7x_get_cclk(void)
{
#if CONFIG_AM_CHIP_ID==1203
	return get_core_clk();
#elif CONFIG_AM_CHIP_ID==1211 || CONFIG_AM_CHIP_ID == 1220 || CONFIG_AM_CHIP_ID == 1213
	unsigned long val=0;

	val = get_core_clk();
	if(REG_READ(CMU_BUSCLK)&(1<<2))
		return val/2;
	else
		return val;
#endif
}

/**
*@brief get system bus clock
*
*@param[in] NULL
*@return system bus clock, hz
*/
static inline unsigned long get_hclk(void)
{
	unsigned long div = REG_READ(CMU_BUSCLK);
#if CONFIG_AM_CHIP_ID == 1203 || CONFIG_AM_CHIP_ID == 1211
	div >>= 4;
	div &=  1;
	return am7x_get_cclk()/(1+div);
#elif CONFIG_AM_CHIP_ID == 1220 || CONFIG_AM_CHIP_ID == 1213
	unsigned int coreclk_div[8]={4,4,5,6,3,7,8,4};
	div >>= 28;
	div &=  7;	
	return get_core_clk()*2/coreclk_div[div];
#endif
}

/**
*@brief get system peripheral clock
*
*@param[in] NULL
*@return system peripheral clock, hz
*/
static inline unsigned long get_pclk(void)
{
	unsigned long div = REG_READ(CMU_BUSCLK);

	div >>= 8;
	div &=  3;
	BUG_ON(div == 2);
#if CONFIG_AM_CHIP_ID==1203
	if(div <= 1)
		return get_hclk()/2;
	else if(div == 3)
		return get_hclk()/4;
#elif CONFIG_AM_CHIP_ID==1211 || CONFIG_AM_CHIP_ID == 1220 || CONFIG_AM_CHIP_ID == 1213
	if(div < 1)
		return get_hclk()/2;
	else
		return get_hclk()/(div+1);
#endif
	return 0; /* Eliminate warnings */
}


/**
*@brief get nand clock
*
*@param[in] NULL
*@return nand clock, hz
*/
static inline unsigned long get_nand_clk(void)
{
	unsigned long div;
	unsigned long core_clk;
	div = REG_READ(CMU_NANDCLK);
	div &= 0xf;
	core_clk = get_core_clk();
	if( core_clk > 200 * _1M )
		core_clk = 200 * _1M;
	return core_clk / (1+div);
}

/**
*@brief get spi clock
*
*@param[in] NULL
*@return spi clock, hz
*/
static inline unsigned long get_spi_clk(void)
{
	unsigned long div;
	unsigned long c_clk;
	div = REG_READ(SPI_CLKDIV);
	div &= 0x2ff;
	div = div + 1;
	c_clk = get_hclk();
	return c_clk / (div*2);
}

/**
*@brief get sdram clock
*
*@param[in] NULL
*@return sdram clock, hz
*/
static inline unsigned long get_sdram_clk(void)
{
	return get_hclk();
}

/**
*@brief start_cp0_count
*
*@param[in] NULL
*@return 
*/
static inline void start_cp0_count(void)
{
	unsigned long tmp;

	tmp  = read_c0_cause();
	tmp &= ~(1<<27);
	write_c0_cause(tmp);
}

/**
*@brief convert C0_COUNT value to milliseconds
*
*@param[in] val : c0 count value read from c0_count register
*@return milliseconds
*/
static inline unsigned long c0_count_to_msec(unsigned long val)
{
	const unsigned long cclk = am7x_get_cclk();
	return (2*val)/(cclk/1000);
}

/**
*@brief convert C0_COUNT value to microseconds 
*
*@param[in] val : c0 count value read from c0_count register
*@return microseconds 
*/
static inline
unsigned long c0_count_to_usec(unsigned long val)
{
	const unsigned long cclk = am7x_get_cclk();
	return (2*val)/(cclk/1000000);
}

/**
 *@}
 */
 
#endif


