/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
@file: am7x_spi.h

@abstract: actions-mircro spi controller driver head file.

@notice: Copyright (c), 2010-2015 Actions-Mirco Communications, Inc.
 *
 *  This program is develop for Actions-Mirco spi Control Device driver;
 *  	
 *
 *
 *
 *  The initial developer of the original code is scopengl
 *
 *  scopengl@gmail.com
 *
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#ifndef AM7X_SPI_H
#define AM7X_SPI_H

#define	SPI_RESET_BIT	28
#define	SPI_CLK_BIT		10

#define	AM7X_SPI_MSB_FIRST	0
#define	AM7X_SPI_LSB_FIRST	1	

#define	SPI_DUREAD_MODE	0	
#define	SPI_WRITE_MODE	1
#define	SPI_READ_MODE	2
#define	SPI_DUPLEX_MODE	3

#define	SPI_TIMEOUT		-1


struct am7x_spi{
	spinlock_t		lock;
	void __iomem		*regs;
	struct platform_device	*pdev;
	struct spi_device	*stay;
	void			*buffer;
	unsigned char bits_per_word;
};

#endif
