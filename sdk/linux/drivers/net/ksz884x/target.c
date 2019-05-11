/* ---------------------------------------------------------------------------
             Copyright (c) 2003-2004 Micrel, Inc.  All rights reserved.
------------------------------------------------------------------------------

    target.c - Target platform functions

Author  Date      Version  Description
PCD     04/08/05  0.1.1    Changed Print MAC address from LSB to MSB.
THa     10/04/04           Updated for PCI version.
THa     12/10/03           Created file.
------------------------------------------------------------------------------
*/


#ifdef _WIN32
/* -------------------------------------------------------------------------- *
 *                               WIN32 OS                                     *
 * -------------------------------------------------------------------------- */

#ifdef NDIS_MINIPORT_DRIVER

#ifndef UNDER_CE
#include <ntddk.h>

#else
#include <ndis.h>
typedef unsigned long ULONG;
#endif


#define DBG_PRINT  DbgPrint
#define NEWLINE    "\n"

/* -------------------------------------------------------------------------- */

#define MAX_DELAY_MICROSEC  50
#define TIME_MILLISEC       10000
#define TIME_MICROSEC       10

/*
    DelayMicrosec

    Description:
        This routine delays in microseconds.

    Parameters:
        ULONG microsec
            Number of microseconds to delay.

    Return (None):
*/

void DelayMicrosec (
    ULONG microsec )
{
    while ( microsec >= MAX_DELAY_MICROSEC )
    {
#ifdef UNDER_CE
        NdisStallExecution( MAX_DELAY_MICROSEC );
#else
        KeStallExecutionProcessor( MAX_DELAY_MICROSEC );
#endif
        microsec -= MAX_DELAY_MICROSEC;
    }
    if ( microsec )
    {
#ifdef UNDER_CE
        NdisStallExecution( microsec );
#else
        KeStallExecutionProcessor( microsec );
#endif
    }
}  /* DelayMicrosec */


/*
    DelayMillisec

    Description:
        This routine delays in milliseconds.

    Parameters:
        ULONG millisec
            Number of milliseconds to delay.

    Return (None):
*/

void DelayMillisec (
    ULONG millisec )
{
#ifndef UNDER_CE
    ULONG count;
#endif

    /* convert to microsecond */
    ULONG microsec = millisec * 1000;

#ifndef UNDER_CE
    if ( KeGetCurrentIrql() > PASSIVE_LEVEL  ||
            microsec < KeQueryTimeIncrement() / TIME_MICROSEC )
    {
        DelayMicrosec( microsec );
    }
    else
    {
        LARGE_INTEGER interval;
        ULONGLONG     diffTime;
        ULONGLONG     lastTime;

        lastTime = KeQueryInterruptTime();
        interval = RtlConvertLongToLargeInteger( microsec * -TIME_MICROSEC );
        KeDelayExecutionThread( KernelMode, FALSE, &interval );
        diffTime = KeQueryInterruptTime() - lastTime;
        count = ( ULONG )( diffTime / TIME_MICROSEC );

        /* delay not long enough */
        if ( microsec > count )
        {
            microsec -= count;
            DelayMicrosec( microsec );
        }
    }

#else
    NdisMSleep( microsec );
#endif
}  /* DelayMillisec */

#else
#include "target.h"
#include <time.h>


/*
    DelayMicrosec

    Description:
        This routine delays in microseconds.

    Parameters:
        ULONG microsec
            Number of microseconds to delay.

    Return (None):
*/

void DelayMicrosec (
    ULONG microsec )
{
}  /* DelayMicrosec */


/*
    DelayMillisec

    Description:
        This routine delays in milliseconds.

    Parameters:
        ULONG millisec
            Number of milliseconds to delay.

    Return (None):
*/

void DelayMillisec (
    ULONG millisec )
{
    clock_t start;
    clock_t stop;

    if ( millisec ) {
        start = clock();
        do {
            stop = clock();
        } while ( ( ULONG )( stop - start ) < millisec );
    }
}  /* DelayMillisec */
#endif
#endif  /* #ifdef _WIN32 */

#ifdef DEF_LINUX
/* -------------------------------------------------------------------------- *
 *                             LINUX OS                                       *
 * -------------------------------------------------------------------------- */
#if defined( KS_ISA_BUS )  ||  !defined( KS_ISA )
#include <linux/spinlock.h>
#include <asm/processor.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include "target.h"


/*
    DelayMicrosec

    Description:
        This routine delays in microseconds.

    Parameters:
        ULONG microsec
            Number of microseconds to delay.

    Return (None):
*/

void DelayMicrosec (
    ULONG microsec )
{
    DWORD millisec = microsec / 1000;

    microsec %= 1000;
    if ( millisec )
        mdelay( millisec );
    if ( microsec )
        udelay( microsec );
}  /* DelayMicrosec */


/*
    DelayMillisec

    Description:
        This routine delays in milliseconds.

    Parameters:
        ULONG millisec
            Number of milliseconds to delay.

    Return (None):
*/

void DelayMillisec (
    ULONG millisec )
{
#if 1
    unsigned long ticks = millisec * HZ / 1000;

    if ( !ticks  ||  in_interrupt() ) {
        mdelay( millisec );
    }
    else {
        set_current_state( TASK_INTERRUPTIBLE );
        schedule_timeout( ticks );
    }
#endif
//    mdelay(millisec);
}  /* DelayMillisec */
#endif
#endif /* #ifdef DEF_LINUX */


#ifdef KS_ARM
#include "target.h"
#endif


/* -------------------------------------------------------------------------- *
 *               Renesas SH7751R MS7751Rse BSP  (32-bit PCI bus)              *
 *                   VxWorks OS  ( Tornado 2.2.1 \vxWorks 5.5.1)              *
 * -------------------------------------------------------------------------- */

#ifdef DEF_VXWORKS
#include "vxWorks.h"                    /* define STATUS */
#include "target.h"
#include "ks884xEnd.h"

extern void sysMsDelay (ULONG);


void DelayMicrosec (
    ULONG microsec )
{
    ULONG millisec = microsec / 1000;
    
    DelayMillisec (millisec);

}  /* DelayMicrosec */

void DelayMillisec
(
    ULONG millisec
)
{
    sysMsDelay ( millisec );
}

#ifdef KS_PCI_BUS
void hw_pci_read_byte
(
    PHARDWARE  phw,
    int    addr,
    BYTE * data
)
{
    PCI_RESOURCES * p = ks884xPciRsrcs;

    pciConfigInByte (p->pciBus, p->pciDevice, p->pciFunc, addr, data );
}

void hw_pci_write_byte
(
    PHARDWARE  phw,
    int    addr,
    BYTE   data
)
{
    PCI_RESOURCES * p = ks884xPciRsrcs;

    pciConfigOutByte (p->pciBus, p->pciDevice, p->pciFunc, addr, data );
}

void hw_pci_read_word
(
    PHARDWARE  phw,
    int      addr,
    USHORT * data
)
{
    PCI_RESOURCES * p = ks884xPciRsrcs;

    pciConfigInWord (p->pciBus, p->pciDevice, p->pciFunc, addr, data );
}

void hw_pci_write_word
(
    PHARDWARE  phw,
    int        addr,
    USHORT     data
)
{
    PCI_RESOURCES * p = ks884xPciRsrcs;

    pciConfigOutWord (p->pciBus, p->pciDevice, p->pciFunc, addr, data );
}

void hw_pci_read_dword
(
    PHARDWARE  phw,
    int      addr,
    ULONG  * data
)
{
    PCI_RESOURCES * p = ks884xPciRsrcs;

    pciConfigInLong (p->pciBus, p->pciDevice, p->pciFunc, addr, data );
}

void hw_pci_write_dword
(
    PHARDWARE  phw,
    int      addr,
    ULONG    data
)
{
    PCI_RESOURCES * p = ks884xPciRsrcs;

    pciConfigOutLong (p->pciBus, p->pciDevice, p->pciFunc, addr, data );
}

#endif /* #ifdef KS_PCI_BUS */

#endif /* #ifdef DEF_VXWORKS */


/* -------------------------------------------------------------------------- *
 *               Renesas M16C/62P      (16-bit Generic bus)                   *
 *                      MTOOL/OpenTCP  (version 1.04)                         *
 * -------------------------------------------------------------------------- */

#ifdef M16C_62P
#include <inet/system.h>
#include <stdarg.h>
#include "target.h"

#define RCV_CH_SIZE     240


void DelayMillisec
(
    ULONG millisec
)
{
	int i;

	i = get_timer();
	init_timer(i, millisec/10 );		/* Set timer in 10ms intervals */
	while( check_timer(i) );			/* Wait for timer to expire */
	free_timer( i );

}


void uart_print (char FAR *format, ...)
{
    char FAR strBuf[RCV_CH_SIZE];
    va_list ap;


    va_start(ap, format);
    memset ( strBuf, 0, RCV_CH_SIZE );
    vsprintf(strBuf, format, ap);
    va_end(ap);
    text_write(strBuf);
}


#endif /* #ifdef M16C_62P */


/* -------------------------------------------------------------------------- *
 *     ZiLog eZ80L92 (eZ80 Webserver-i E-NET Module)  (8-bit Generic bus)     *
 *                   ZTP 1.3.2 / EMAC DDK 1.3.2                               *
 * -------------------------------------------------------------------------- */

#ifdef _EZ80L92
#include "target.h"
/*
    DelayMicrosec

    Description:
        This routine delays in microseconds.

    Parameters:
        ULONG microsec (us)
            Number of microseconds to delay. (1s = 1000000 us)

    Return (None):
*/

void DelayMicrosec (
    ULONG microsec )
{
    DWORD millisec = microsec / 1000;

    if ( millisec )
        DelayMillisec( millisec );
    else
        DelayMillisec( 1 );

}  /* DelayMicrosec */


/*
    DelayMillisec

    Description:
        This routine delays in milliseconds.

    Parameters:
        ULONG millisec  (ms)
            Number of milliseconds to delay. ( 1s = 1000 ms)

    Return (None):
*/

void DelayMillisec (
    ULONG millisec )
{
    unsigned long ms_10 = millisec / 100;  /* in 10ms unit */

    if ( millisec )
        KE_TaskSleep100( ms_10 );
    else
        KE_TaskSleep100( 1 );   /* min unit is in 10ms sleep */

}  /* DelayMillisec */


/******************************************************************************
*
* strtok_p -
*
* Break down a string into tokens.
*
* Parameters:
*    s1 - string to break into tokens
*    s2 - the separators
*    ppLast - pointers to serve as string index
*
* RETURNS: Pointer to the first character of a token, or NULL if there is no token.
*/
char *strtok_p
(
    char *s1,
    const char *s2,
    char **ppLast
)
{
    static char *_tokp = NULL;

    char *ptr;
    int  n;

    if ( s1 == 0 ) s1 = _tokp;
    s1 += strspn( s1, s2 );
    if ( ( ptr = strpbrk( s1, s2 ) ) == 0 ) {
        for ( _tokp = s1; *_tokp; _tokp++ ) ;
        *ppLast = '\0';
        return ( ( *s1 == '\0' ) ? ( char * )0 : s1 );
    }
    if ( *ptr != '\0' ) *ptr++ = '\0';
    _tokp = ptr;
    *ppLast = _tokp;
    return ( s1 );
}


#endif /* #ifdef _EZ80L92 */

/* -------------------------------------------------------------------------- */

#if !defined( DEF_LINUX )  ||  defined( KS_ISA_BUS )  ||  !defined( KS_ISA )
void PrintMacAddress (
    PUCHAR bAddr )
{
    DBG_PRINT( "%02x:%02x:%02x:%02x:%02x:%02x",
        bAddr[ 0 ], bAddr[ 1 ], bAddr[ 2 ],
        bAddr[ 3 ], bAddr[ 4 ], bAddr[ 5 ]);
}  /* PrintMacAddress */

void PrintIpAddress (
    ULONG          IpAddr )
{

    DBG_PRINT( "%ld.%ld.%ld.%ld",
               ((IpAddr >> 24) & 0x000000ff),
               ((IpAddr >> 16) & 0x000000ff),
               ((IpAddr >> 8 ) & 0x000000ff),
               (IpAddr & 0x000000ff)
             );
}  /* PrintIpAddress */

/*
 * PrintPacketData
 *	This function is use to dump given packet for debugging.
 *
 * Argument(s)
 *	data		pointer to the beginning of the packet to dump
 *	len			length of the packet
 *  flag        1: dump tx packet, 2: dump rx packet
 *
 * Return(s)
 *	NONE.
 */

void PrintPacketData
(
    UCHAR         *data,
    int            len,
    ULONG         port,
    ULONG         flag
)
{
    if  ( (flag < 1)  || (flag > 2) )
        return;


    if ( flag == 1 )
         DBG_PRINT ("Tx On port %d "NEWLINE, ( int ) port);
    else
         DBG_PRINT ("Rx On port %d "NEWLINE, ( int ) port);


	/* if (len >= 18) */
    {
		DBG_PRINT("Pkt Len=%d"NEWLINE, len);
		DBG_PRINT("DA=%02x:%02x:%02x:%02x:%02x:%02x"NEWLINE,
				*data, *(data + 1), *(data + 2), *(data + 3), *(data + 4), *(data + 5));
		DBG_PRINT("SA=%02x:%02x:%02x:%02x:%02x:%02x"NEWLINE,
				*(data + 6), *(data + 7), *(data + 8), *(data + 9), *(data + 10), *(data + 11));
		DBG_PRINT("Type=%02x%02x"NEWLINE, (*(UCHAR *)(data + 12)), (*(UCHAR *)(data + 13)) );

        {
			int	j = 0, k;

			do {
				DBG_PRINT(NEWLINE" %04x   ", j);
				for (k = 0; (k < 16 && len); k++, data++, len--)
					DBG_PRINT("%02x  ", *data);
				j += 16;
			} while (len > 0);
			DBG_PRINT(NEWLINE);
		}
	}
}
#endif
