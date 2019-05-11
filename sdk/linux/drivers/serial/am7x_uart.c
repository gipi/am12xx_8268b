/*
 * am7x_uart.c: Serial port driver for AM7XXX chipset
 *
 * Email: reachpan@actions-micro.com 
 *
 *
 */

#undef DEBUG_DZ

#if defined(CONFIG_SERIAL_AM7X_CONSOLE) && defined(CONFIG_MAGIC_SYSRQ)
#define SUPPORT_SYSRQ
#endif

#include <linux/bitops.h>
#include <linux/compiler.h>
#include <linux/console.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/module.h>
#include <linux/serial.h>
#include <linux/serial_core.h>
#include <linux/sysrq.h>
#include <linux/tty.h>

#include <asm/atomic.h>
#include <asm/bootinfo.h>
#include <asm/io.h>
#include <asm/system.h>

#include "actions_io.h"
#include "dz.h"
#define  AM_NB_PORT     2
#define  PORT_REG_SIZE  32
#define  am7x_uart_slot_size  32


//#define  UART_CONFIG_VALUE    0x000c8403
#define  UART_CONFIG_VALUE    0x00048403


enum {
	AM_CS5 = 0x0,
	AM_CS6 = 0x1,
	AM_CS7 = 0x2,
	AM_CS8 = 0x3,
};

#define U_CTL   0x00
#define U_RXDAT 0x04
#define U_TXDAT 0x08
#define U_STAT  0x0c

#define AM_TFER  (1<<3) /* tx fifo error */
#define AM_RFEM  (1<<5) /* rx fifo empty */
#define AM_TFFU  (1<<6) /* tx fifo full */
#define AM_RXST  (1<<4) /* rx error */


MODULE_DESCRIPTION("ActionsMicro AM7XXX serial driver");
MODULE_LICENSE("GPL");


static char am_name[] __initdata = "ActionsMicro AM7XXX serial driver version ";
static char am_version[] __initdata = "0.14";

struct am_port {
	struct am_mux		*mux;
	struct uart_port	port;
	unsigned int		cflag;
	int                 id;
	unsigned long       save_config;
	atomic_t            map_guard;
	atomic_t            irq_guard;

};

struct am_mux {
	struct am_port	dport[AM_NB_PORT];
	int				initialised;
};

static struct am_mux am_mux;

static inline struct am_port *to_dport(struct uart_port *uport)
{
	return container_of(uport, struct am_port, port);
}

static inline int uport_rx_empty(struct uart_port *uport)
{
	return am7x_readl((AM7X_HWREG)uport->membase+U_STAT) & AM_RFEM;
}

void uport_tx(struct uart_port *uport, int ch)
{
	while( am7x_readl((AM7X_HWREG)uport->membase+U_STAT)&AM_TFFU ) {} 
	am7x_writel(ch, (AM7X_HWREG)uport->membase+U_TXDAT);
}

/*
 * ------------------------------------------------------------
 * rs_stop () and rs_start ()
 *
 * These routines are called before setting or resetting
 * tty->stopped. They enable or disable transmitter interrupts,
 * as necessary.
 * ------------------------------------------------------------
 */

static void am_stop_tx(struct uart_port *uport)
{
	/* nothing to do */
}

static void am_transmit_chars(struct uart_port *mux);
static void am_start_tx(struct uart_port *uport)
{
//	struct am_port *dport = to_dport(uport);
	am_transmit_chars(uport);
}

static void am_stop_rx(struct uart_port *uport)
{
	/* nothing to do */
}

static void am_enable_ms(struct uart_port *uport)
{
	/* nothing to do */
}


/*
 * ------------------------------------------------------------
 * receive_char ()
 *
 * This routine deals with inputs from any lines.
 * ------------------------------------------------------------
 */
static inline void am_receive_chars(struct uart_port *uport)
{
	struct tty_struct *tty;
	struct uart_icount *icount = &uport->icount;
	unsigned char ch, flag;
	u16 rx_count = 0;

	tty = uport->info->port.tty;	/* point to the proper dev */
	while( ! uport_rx_empty(uport) ) {
		ch = am7x_readl((AM7X_HWREG)uport->membase+U_RXDAT);	/* grab the char */
		flag = TTY_NORMAL;

		icount->rx++;
		rx_count++;

		if (uart_handle_sysrq_char(uport, ch))
			continue;
		uart_insert_char(uport, 0, 0, ch, flag);
	}
	//must be mark BUG_ON(rx_count == 0);
	//Because if rx_count == 0,BUG_ON generate a mips trap exception.
	//When in uart receive interrupt . Press a key ,then uart receive interrupt pending is set.
	//But at this uart receive interrupt,all of the rx data is read. so fifo is empty. so next interrupt ,
	//rx_count == 0.
	//BUG_ON(rx_count == 0);
	if(rx_count > 0)
		tty_flip_buffer_push(tty);
}

static void am_transmit_chars(struct uart_port *uport)
{
	struct circ_buf *xmit;
	unsigned char tmp;

	xmit = &uport->info->xmit;

	if (uport->x_char) {		/* XON/XOFF chars */
		uport->icount.tx++;
		uport->x_char = 0;
		return;
	}
	/* If nothing to do or stopped or hardware stopped. */
	if (uart_circ_empty(xmit)) {
		return;
	}

	/*
	 * If something to do... (remember the dz has no output fifo,
	 * so we go one char at a time) :-<
	 */
	while ( !uart_circ_empty(xmit) ) {
		tmp = xmit->buf[xmit->tail];
		xmit->tail = (xmit->tail + 1) & (UART_XMIT_SIZE - 1);
		uport_tx(uport, tmp);
		uport->icount.tx++;
	/* Are we are done. */
	}
}


/*
 * ------------------------------------------------------------
 * am_interrupt ()
 *
 * this is the main interrupt routine for the AM chip.
 * It deals with the multiple ports.
 * ------------------------------------------------------------
 */
static irqreturn_t am_interrupt(int irq, void *dev_id)
{
	struct uart_port *uport = dev_id;
	void *reg = uport->membase + U_STAT;
	unsigned int status = am7x_readl((AM7X_HWREG)reg);

	/* if rx fifo error, just exit irq */
	if(status & AM_RXST) 
		goto rx_error;
	//while( ! uport_rx_empty(uport) ) { int ch = am7x_readl(uport->membase+U_RXDAT); am_printf("%c",ch); } 
	am_receive_chars(uport);
rx_error:
	/* clear uart status */
	IRQ_CLEAR_PENDING(status, (AM7X_HWREG)reg);
	return IRQ_HANDLED;
}

/*
 * -------------------------------------------------------------------
 * Here ends the DZ interrupt routines.
 * -------------------------------------------------------------------
 */

static unsigned int am_get_mctrl(struct uart_port *uport)
{
	unsigned int mctrl = TIOCM_CTS | TIOCM_RTS;
	return mctrl;
}

static void am_set_mctrl(struct uart_port *uport, unsigned int mctrl)
{
	/* nothing to do */
}

static void do_mfp_config(void)
{
#if CONFIG_AM_CHIP_ID == 1211
	unsigned int tmp = am7x_readl(GPIO_MFCTL3);
	tmp &= ~((7 << 4) | (7 << 8));
	tmp |= (2 << 4) | (3 << 8);
	am7x_writel(tmp, GPIO_MFCTL3);
#elif CONFIG_AM_CHIP_ID == 1220
	unsigned int tmp = am7x_readl(GPIO_MFCTL7);
	tmp &= ~((7 << 9) | (7 << 13));
	tmp |= (1 << 9) | (1 << 13);
	am7x_writel(tmp, GPIO_MFCTL7);
#elif CONFIG_AM_CHIP_ID == 1213
#if  (CONFIG_AM_CHIP_MINOR==8258 ||CONFIG_AM_CHIP_MINOR==8268)
	unsigned int tmp = am7x_readl(GPIO_MFCTL0);
	tmp &= ~(0x7<<16) ;
	tmp |= (0x3<<16);
	am7x_writel(tmp, GPIO_MFCTL0);
#else
#ifdef CONFIG_AM_8251
	unsigned int tmp = am7x_readl(GPIO_MFCTL3);
	tmp &= ~((0x7<<18) | (0x7<<21));
	tmp |= ((0x4<<18) | (0x4<<21));
	am7x_writel(tmp, GPIO_MFCTL3);
#else
	unsigned int tmp = am7x_readl(GPIO_MFCTL4);
	tmp &= ~((0x3<<3) | (0x3<<5));
	tmp |= ((0x2<<3) | (0x2<<5));
	am7x_writel(tmp, GPIO_MFCTL4);
#endif
#endif
#endif
}

extern void enable_uart_clock(int);
extern void disable_uart_clock(int);

/*
 * -------------------------------------------------------------------
 * startup ()
 *
 * various initialization tasks
 * -------------------------------------------------------------------
 */
static int am_startup(struct uart_port *uport)
{
	struct am_port *dport = to_dport(uport);
	unsigned long flags;
	int irq_guard;
	int ret;
	unsigned long reg_uart_ctl = (unsigned long)uport->membase + U_CTL;

	irq_guard = atomic_add_return(1, &dport->irq_guard);
	if (irq_guard != 1)
		return 0;

	
//	act_andl(~(1<<19), (unsigned long)uport->membase+U_CTL); /* Disable TX IRQ */
//	enable_uart_clock(dport->id);
	do_mfp_config();
	dport->save_config = act_readl(reg_uart_ctl);
	am_printf("write %x to %x\n", UART_CONFIG_VALUE, reg_uart_ctl);
	act_writel(UART_CONFIG_VALUE, reg_uart_ctl);
	flags = am7x_readl((AM7X_HWREG)uport->membase+U_STAT); /* clear rx&tx fifo */
	am7x_writel(flags, (AM7X_HWREG)uport->membase+U_STAT);
	
	ret = request_irq(dport->port.irq, am_interrupt,
			  IRQF_DISABLED, "am7x-uart", uport);
	if (ret) {
		atomic_add(-1, &dport->irq_guard);
		printk(KERN_ERR "am7x-uart: Cannot get IRQ %d, err=%d!\n", dport->port.irq, ret);
		return ret;
	}
	printk("COM%d start up!\n", dport->id);
	return 0;
}

/*
 * -------------------------------------------------------------------
 * shutdown ()
 *
 * This routine will shutdown a serial port; interrupts are disabled, and
 * DTR is dropped if the hangup on close termio flag is on.
 * -------------------------------------------------------------------
 */
static void am_shutdown(struct uart_port *uport)
{
	struct am_port *dport = to_dport(uport);
	unsigned long flags;
	int irq_guard;

	spin_lock_irqsave(&dport->port.lock, flags);
	am_stop_tx(&dport->port);
	spin_unlock_irqrestore(&dport->port.lock, flags);

	am_printf("1. irq_guard = %d\n", dport->irq_guard);
	irq_guard = atomic_add_return(-1, &dport->irq_guard);
	am_printf("2. irq_guard = %d\n", irq_guard);
	if (!irq_guard) {
		unsigned long reg_uart_ctl = (unsigned long)uport->membase + U_CTL;
		disable_uart_clock(dport->id);
		/* Disable interrupts.  */
		act_writel(dport->save_config, reg_uart_ctl);
		free_irq(dport->port.irq, uport);
		printk("COM%d shut down!\n", dport->id);
	}
}

static unsigned int am_tx_empty(struct uart_port *uport)
{
	unsigned int tmp;

	tmp = am7x_readl((AM7X_HWREG)uport->membase + U_STAT);
	return (tmp & AM_TFER) ? 0 : TIOCSER_TEMT;
}

static void am_break_ctl(struct uart_port *uport, int break_state)
{
	/* nothing to do */
}


static void am_reset(struct am_port *dport)
{
	struct am_mux *mux = dport->mux;

	if (mux->initialised)
		return;
	mux->initialised = 1;
}

static void am_set_termios(struct uart_port *uport, struct ktermios *termios,
			   struct ktermios *old_termios)
{
	struct am_port *dport = to_dport(uport);
	unsigned long flags;
	unsigned int cflag, baud;

//am_printf("%s:%d\n", __func__,__LINE__);

	termios->c_cflag &= ~(HUPCL | CMSPAR);
	termios->c_cflag |= CLOCAL;

	cflag = 0;

	switch (termios->c_cflag & CSIZE) {
	case CS5:
		cflag |= AM_CS5;
		break;
	case CS6:
		cflag |= AM_CS6;
		break;
	case CS7:
		cflag |= AM_CS7;
		break;
	case CS8:
	default:
		cflag |= AM_CS8;
	}

	if (termios->c_cflag & CSTOPB)
		/* nothing to do! */;
	if (termios->c_cflag & PARENB)
		/* nothing to do! */;
	if (termios->c_cflag & PARODD)
		/* nothing to do! */;

	baud = uart_get_baud_rate(uport, termios, old_termios, 50, 115200);
	tty_termios_encode_baud_rate(termios, baud, baud);

	if (termios->c_cflag & CREAD)
		/* nothing to do! */;

	spin_lock_irqsave(&dport->port.lock, flags);

	uart_update_timeout(uport, termios->c_cflag, baud);

	dport->cflag = cflag;

	/* setup accept flag */
	if (termios->c_iflag & INPCK)
		/* nothing to do! */;
	if (termios->c_iflag & (BRKINT | PARMRK))
		/* nothing to do! */;

	/* characters to ignore */
	uport->ignore_status_mask = 0;
	if ((termios->c_iflag & (IGNPAR | IGNBRK)) == (IGNPAR | IGNBRK))
		/* nothing to do! */;
	if (termios->c_iflag & IGNPAR)
		/* nothing to do! */;
	if (termios->c_iflag & IGNBRK)
		/* nothing to do! */;

	spin_unlock_irqrestore(&dport->port.lock, flags);
}

/*
 * Hack alert!
 * Required solely so that the initial PROM-based console
 * works undisturbed in parallel with this one.
 */
static void am_pm(struct uart_port *uport, unsigned int state,
		  unsigned int oldstate)
{
	struct am_port *dport = to_dport(uport);
	unsigned long flags;

	spin_lock_irqsave(&dport->port.lock, flags);
	spin_unlock_irqrestore(&dport->port.lock, flags);
}


static const char *am_type(struct uart_port *uport)
{
	return "AM";
}

static void am_release_port(struct uart_port *uport)
{
	struct am_port *dport = to_dport(uport);
	int map_guard;

	iounmap(uport->membase);
	uport->membase = NULL;

	map_guard = atomic_add_return(-1, &dport->map_guard);
	if (!map_guard)
		release_mem_region(uport->mapbase, am7x_uart_slot_size);
}

static int am_map_port(struct uart_port *uport)
{
	if (!uport->membase)
		uport->membase = ioremap_nocache(uport->mapbase,
						 am7x_uart_slot_size);
	if (!uport->membase) {
		printk(KERN_ERR "am: Cannot map MMIO\n");
		return -ENOMEM;
	}
	am_printf(KERN_ERR "am: membase@0x%08x, irq%d\n", uport->membase, uport->irq);
	return 0;
}

static int am_request_port(struct uart_port *uport)
{
	struct am_port *dport = to_dport(uport);
	int map_guard;
	int ret;

	map_guard = atomic_add_return(1, &dport->map_guard);
	if (map_guard == 1) {
		if (!request_mem_region(uport->mapbase, am7x_uart_slot_size, "am7x-uart")) {
			atomic_add(-1, &dport->map_guard);
			printk(KERN_ERR
			       "am7x-uart: Unable to reserve MMIO resource\n");
			return -EBUSY;
		}
	}
	ret = am_map_port(uport);
	if (ret) {
		map_guard = atomic_add_return(-1, &dport->map_guard);
		if (!map_guard)
			release_mem_region(uport->mapbase, am7x_uart_slot_size);
		return ret;
	}
	return 0;
}

static void am_config_port(struct uart_port *uport, int flags)
{
	struct am_port *dport = to_dport(uport);

	if (flags & UART_CONFIG_TYPE) {
		if (am_request_port(uport))
			return;

		uport->type = PORT_DZ;

		am_reset(dport);
	}
}

/*
 * Verify the new serial_struct (for TIOCSSERIAL).
 */
static int am_verify_port(struct uart_port *uport, struct serial_struct *ser)
{
	int ret = 0;

	if (ser->type != PORT_UNKNOWN && ser->type != PORT_DZ)
		ret = -EINVAL;
	if (ser->irq != uport->irq)
		ret = -EINVAL;
	return ret;
}

static struct uart_ops am_ops = {
	.tx_empty	= am_tx_empty,
	.get_mctrl	= am_get_mctrl,
	.set_mctrl	= am_set_mctrl,
	.stop_tx	= am_stop_tx,
	.start_tx	= am_start_tx,
	.stop_rx	= am_stop_rx,
	.enable_ms	= am_enable_ms,
	.break_ctl	= am_break_ctl,
	.startup	= am_startup,
	.shutdown	= am_shutdown,
	.set_termios	= am_set_termios,
	.pm		= am_pm,
	.type		= am_type,
	.release_port	= am_release_port,
	.request_port	= am_request_port,
	.config_port	= am_config_port,
	.verify_port	= am_verify_port,
};

static void __init am_init_ports(void)
{
	static char *const uart_base[2] = {(char *const)UART1_CTL, (char *const)UART2_CTL};
	static int first = 1;
	int line;

	if (!first)
		return;
	first = 0;

	for (line = 0; line < AM_NB_PORT; line++) {
		struct am_port *dport = &am_mux.dport[line];
		struct uart_port *uport = &dport->port;

		dport->mux	= &am_mux;
		uport->fifosize	= 1;
		uport->iotype	= UPIO_MEM;
		uport->flags	= UPF_BOOT_AUTOCONF;
		uport->ops	= &am_ops;
		uport->line	= line;
		uport->irq	= IRQ_UART1 - line;
		uport->mapbase	= KVA_TO_PA(uart_base[line]);
		dport->id       = line;
	}
}

#ifdef CONFIG_SERIAL_AM7X_CONSOLE
/*
 * -------------------------------------------------------------------
 * am_console_putchar() -- transmit a character
 *
 * Polled transmission.  This is tricky.  We need to mask transmit
 * interrupts so that they do not interfere, enable the transmitter
 * for the line requested and then wait till the transmit scanner
 * requests data for this line.  But it may request data for another
 * line first, in which case we have to disable its transmitter and
 * repeat waiting till our line pops up.  Only then the character may
 * be transmitted.  Finally, the state of the transmitter mask is
 * restored.  Welcome to the world of PDP-11!
 * -------------------------------------------------------------------
 */
static void am_console_putchar(struct uart_port *uport, int ch)
{
	struct am_port *dport = to_dport(uport);
	unsigned long flags;
	
	spin_lock_irqsave(&dport->port.lock, flags);
	uport_tx(uport, ch);
	spin_unlock_irqrestore(&dport->port.lock, flags);
}

/*
 * -------------------------------------------------------------------
 * am_console_print ()
 *
 * am_console_print is registered for printk.
 * The console must be locked when we get here.
 * -------------------------------------------------------------------
 */
static void am_console_print(struct console *co,
			     const char *str,
			     unsigned int count)
{
	struct am_port *dport = &am_mux.dport[co->index];
//	am_printf("tty_write: %s", (char *) str);
	uart_console_write(&dport->port, str, count, am_console_putchar);
}

static int __init am_console_setup(struct console *co, char *options)
{
	struct am_port *dport = &am_mux.dport[co->index];
	struct uart_port *uport = &dport->port;
	int baud = 9600;
	int bits = 8;
	int parity = 'n';
	int flow = 'n';
	int ret;

	ret = am_map_port(uport);
	if (ret)
		return ret;

	spin_lock_init(&dport->port.lock);	/* For am_pm().  */

	am_reset(dport);
	am_pm(uport, 0, -1);

	if (options)
		uart_parse_options(options, &baud, &parity, &bits, &flow);

	//return uart_set_options(&dport->port, co, baud, parity, bits, flow);
	ret = uart_set_options(&dport->port, co, baud, parity, bits, flow);
	am_printf("co%d, baud=%d, parity=%c, bits=%d, flow=%c\n", co->index, baud, parity, bits, flow);
	return ret;
}

static struct uart_driver am_reg;
static struct console am_console = {
	.name	= "ttyS",
	.write	= am_console_print,
	.device	= uart_console_device,
	.setup	= am_console_setup,
	.flags	= CON_PRINTBUFFER,
	.index	= -1,
	.data	= &am_reg,
};

static int __init am_serial_console_init(void)
{
	am_init_ports();
	register_console(&am_console);
printk("printk okay!\n");
	return 0;
}

console_initcall(am_serial_console_init);

#define SERIAL_DZ_CONSOLE	&am_console
#else
#define SERIAL_DZ_CONSOLE	NULL
#error  No console!
#endif /* CONFIG_SERIAL_AM7X_CONSOLE */

static struct uart_driver am_reg = {
	.owner			= THIS_MODULE,
	.driver_name		= "serial",
	.dev_name		= "ttyS",
	.major			= TTY_MAJOR,
	.minor			= 64,
	.nr			= AM_NB_PORT,
	.cons			= SERIAL_DZ_CONSOLE,
};

static int __init am_init(void)
{
	int ret, i;

	printk("%s%s\n", am_name, am_version);

	am_init_ports();

	ret = uart_register_driver(&am_reg);
	if (ret)
		return ret;

	for (i = 0; i < AM_NB_PORT; i++)
		uart_add_one_port(&am_reg, &am_mux.dport[i].port);
	return 0;
}

module_init(am_init);


