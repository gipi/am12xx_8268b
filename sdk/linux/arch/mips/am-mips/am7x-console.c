#include <linux/kernel.h>
#include <linux/module.h>
#include "asm-mips/types.h"
#include "actions_io.h"
#include "sys_cfg.h"

#define  UART_CTL       0x00
#define  UART_RXDAT     0x04
#define  UART_TXDAT     0x08
#define  UART_STAT      0x0C

static const unsigned long uart_reg_base[2] = { UART1_CTL, UART2_CTL };
static const unsigned long cmu_uartclk[2]   = { CMU_UART1CLK, CMU_UART2CLK };
static unsigned long UART_BASE;

#ifdef CONFIG_AM_8251
#define USE_UART   1
#endif
void serial_init (void)
{
	UART_BASE = uart_reg_base[USE_UART];
}

void serial_setbrg (unsigned long baudrate)
{
	int use_ddrpll;
	unsigned long busclk;
	unsigned long tmp;
	unsigned long cclk, clk_div;
#if CONFIG_AM_CHIP_MINOR == 8268
	static const unsigned char mul[] = { 24, 24 };
#else
	static const unsigned char mul[] = { 6, 8 };
#endif

	busclk = act_readl(CMU_BUSCLK);

	switch( (busclk >> 6) & 3 ) {
	case 0:
		cclk = 32 * 1000;
		break;
	case 1:
		cclk = 24 * 1000000;
		break;
	default:
#if CONFIG_AM_CHIP_ID == 1211 || CONFIG_AM_CHIP_ID == 1220 || CONFIG_AM_CHIP_ID == 1213
		use_ddrpll = (busclk >> 11) & 1;
		if( use_ddrpll )
			tmp  = act_readl(CMU_DDRPLL);
		else
			tmp  = act_readl(CMU_COREPLL);
#if CONFIG_AM_CHIP_MINOR == 8268
		cclk  = (tmp >> 1) & 0x3f;
		cclk *= mul[use_ddrpll];
#else
		cclk  = (tmp >> 2) & 0x3f;
		cclk *= mul[use_ddrpll];
#endif
#else
		tmp     = act_readl(CMU_COREPLL);
		cclk = (tmp >> 2) & 0x3f;
		cclk *= 6;
#endif
		cclk *= 1000000 ;
		clk_div = (act_readl(CMU_BUSCLK) >> 4)& 0x1;
		clk_div++;
		cclk /= clk_div;
		break;
	}
	tmp = cclk/(16*baudrate);
	
	tmp |= 0x10000;
	act_writel(tmp, cmu_uartclk[USE_UART]);
}
EXPORT_SYMBOL(serial_setbrg);

static inline void __serial_putchar (const char c)
{
/* Wait for fifo to shift out some bytes */
	while(act_readl(UART_BASE + UART_STAT)&0x40) {}
	act_writel(c, UART_BASE + UART_TXDAT );
}

static inline void serial_putchar(char c)
{
	if(c == '\n')
		__serial_putchar('\r');
	__serial_putchar(c);
}

static void serial_puts (const char *s)
{
	while(*s)
		serial_putchar (*s++);
}

#define CFG_PBSIZE  128
static char printbuffer[CFG_PBSIZE];

int am_printf(const char *fmt, ...)
{
	char *const pb = printbuffer;
	va_list args;
	unsigned int i;

	va_start (args, fmt);
/* For this to work, printbuffer must be larger than
 * anything we ever want to print.
 */
	i = vsnprintf (pb, sizeof(printbuffer), fmt, args);
	va_end (args);
/* Print the string */
	if(i > CFG_PBSIZE)
		return 0;
	serial_puts (pb);
	return i;
}


int prom_putchar(char c)
{
	serial_putchar(c);
	return 1;
}

int prom_printf(const char *fmt, ...) __attribute__((weak, alias("am_printf")));

#define CMU_UARTCLK (0xb0010028)

void enable_uart_clock(int uart)
{
	if( uart != USE_UART ) {
		unsigned long clk = act_readl(CMU_UARTCLK + USE_UART * 4);
		act_writel(clk, CMU_UARTCLK + uart * 4);
	}
}
EXPORT_SYMBOL(enable_uart_clock);

void disable_uart_clock(int uart)
{
	if( uart != USE_UART ) {
		act_writel(0, CMU_UARTCLK + uart * 4);
	}
}

EXPORT_SYMBOL(disable_uart_clock);
