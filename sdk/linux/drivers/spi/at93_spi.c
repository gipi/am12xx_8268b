/* ASIX AX88796C SPI Fast Ethernet Linux driver */

/* 
 * Copyright (c) 2010 ASIX Electronics Corporation
 *
 *  This program is free software; you can distribute it and/or modify it
 *  under the terms of the GNU General Public License (Version 2) as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 *  for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place - Suite 330, Boston MA 02111-1307, USA.
 *
 */

/* INCLUDE FILE DECLARATIONS */
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/spi/spi.h>
#include "actions_regs.h"
#include "actions_io.h"
#include "sys_at24c02.h"


#define AT93_RD   0X06
#define AT93_WR   0X05
#define AT93_EWEN 0X13
#define AT93_EWDS 0X10
#define AT93_WRAL 0X11
#define AT93_ERSE 0X07
#define AT93_RDAL 0X12

struct at93_data {
	struct spi_device	*spi;
	struct mutex		lock;
//	unsigned		addrlen;
};
#define ON              1
#define OFF            (!ON)

#define SPI_EE_TYPE     0       //spi eeprom type ,0: AT93C46; 1:AT93C56;2:AT93C66;3:AT93C06;4:AT93C76;5:AT93C86
#if     SPI_EE_TYPE==0
#define	NO_SUPPORT_SQE_READ    ON  /*ON :no support sequence  read;OFF :support sequence  read*/
#define AT93_MAXADDRLEN	7		/* 7 bit addresses */
#define AT93_CAP       128      /*capability is 128Bytes*/
#elif   SPI_EE_TYPE==1
#define	NO_SUPPORT_SQE_READ    OFF  /*ON :no support sequence  read;OFF :support sequence  read*/
#define AT93_MAXADDRLEN	9		/* 9 bit addresses */
#define AT93_CAP       256      /*capability is 256Bytes*/
#elif   SPI_EE_TYPE==2
#define	NO_SUPPORT_SQE_READ    OFF  /*ON :no support sequence  read;OFF :support sequence  read*/
#define AT93_MAXADDRLEN	9		/* 9 bit addresses */
#define AT93_CAP       512      /*capability is 512Bytes*/
#elif   SPI_EE_TYPE==3
#define	NO_SUPPORT_SQE_READ    OFF  /*ON :no support sequence  read;OFF :support sequence  read*/
#define AT93_MAXADDRLEN	7		/* 9 bit addresses */
#define AT93_CAP       32      /*capability is 32Bytes*/
#elif   SPI_EE_TYPE==4
#define	NO_SUPPORT_SQE_READ    OFF  /*ON :no support sequence  read;OFF :support sequence  read*/
#define AT93_MAXADDRLEN	11		/* 9 bit addresses */
#define AT93_CAP       1024      /*capability is 1024Bytes*/
#elif   SPI_EE_TYPE==5
#define	NO_SUPPORT_SQE_READ    OFF  /*ON :no support sequence  read;OFF :support sequence  read*/
#define AT93_MAXADDRLEN	11		/* 9 bit addresses */
#define AT93_CAP       2048      /*capability is 2048Bytes*/
#endif
#define AT93_ORG        1       //AT93CXX  internal organization ,0: x8  organization;1: x16 organization
static  struct   at93_data  spi_ee={
	.spi = NULL,
};
static void at93_spi_pin_config(void)
{
#if CONFIG_AM_CHIP_ID == 1213
	act_writel(~(0x01<<17)&act_readl(GPIO_MFCTL0),GPIO_MFCTL0);
	printk(KERN_INFO "mfp1=%x\n",act_readl(GPIO_MFCTL1));	/*miso:bit4~6		000		clk:bit8~10	001	*/
	printk(KERN_INFO "mfp2=%x\n",act_readl(GPIO_MFCTL2));	/*mosi:bit16~18	101	*/
	printk(KERN_INFO "mfp3=%x\n",act_readl(GPIO_MFCTL3));	/*nss2:bit8~11	0110	int2:bit12~15	0110 */
	printk(KERN_INFO "mfp0=%x\n",act_readl(GPIO_MFCTL0));	
	printk(KERN_INFO "spi ctl=%x\n",act_readl(SPI_CTL));	
#endif
}

static int at93_read_byte(struct spi_device *spi,u16 addr)
{	
	u16   temp;
	u8 status,txbuf[2],rxbuf[4];
	temp = (AT93_RD << (AT93_MAXADDRLEN)) + (addr &((1<<AT93_MAXADDRLEN)-1));
	txbuf[0] = temp >>8;
	txbuf[1] = temp;
#if AT93_ORG == 1 
    status = spi_write_then_read(spi,txbuf, 2,rxbuf,4);	
//	rxbuf[0] = (rxbuf[0]<<1)+(rxbuf[1]>>7);
//	rxbuf[1] = (rxbuf[1]<<1)+(rxbuf[2]>>7);
#else
    status = spi_write_then_read(spi,txbuf, 2,rxbuf,2);	
//	rxbuf[0] = (rxbuf[0]<<1)+(rxbuf[1]>>7);
#endif
	return status;
}
/*
 * ----------------------------------------------------------------------------
 * Function Name: at93_ee_read
 * Purpose:
 * ----------------------------------------------------------------------------
 */

static ssize_t
at93_ee_read(
	struct at93_data	*at93,
	unsigned char			*buf,
	unsigned		offset,
	size_t			count
)
{
	ssize_t			status;
	u16   temp,len;
	u8    txbuf[2];	
	if(offset>= AT93_CAP)
		return -EFAULT;
#if AT93_ORG == 1
	if(count & 0x01)
		len = count+1;
	else
		len = count+2;
	temp = (AT93_RD << (AT93_MAXADDRLEN-1)) + (offset & ((1<<(AT93_MAXADDRLEN-1))-1));
#else
	len = count;
	temp = (AT93_RD << (AT93_MAXADDRLEN)) + (offset & ((1<<AT93_MAXADDRLEN)-1));
#endif
	txbuf[0] = temp >>8;
	txbuf[1] = temp;

 	mutex_lock(&at93->lock);   
	status = spi_write_then_read(at93->spi,txbuf,2,buf,len);	
	/* Read it all at once.
	 *
	 * REVISIT that's potentially a problem with large chips, if
	 * other devices on the bus need to be accessed regularly or
	 * this chip is clocked very slowly
	 */
	for(temp=0;temp<count;temp++){
		*(buf+temp) = ((*(buf+temp)) << 1) + ((*(buf+temp+1)) >> 7);
	}

	dev_dbg(&at93->spi->dev,
		"read %Zd bytes at %d --> %d\n",
		count, offset, (int) status);
	mutex_unlock(&at93->lock);
	return status ? count : status;
}

static int at93_ee_ewen(struct at93_data *at93){
	u16             temp;
	ssize_t			status;
	u8			bounce[2];
#if AT93_ORG == 1
	temp = (AT93_EWEN << (AT93_MAXADDRLEN-3));
#else
	temp = (AT93_EWEN << (AT93_MAXADDRLEN-2));
#endif
	bounce[0] = (u8)(temp >>8);
	bounce[1] = (u8)(temp);	
	mutex_lock(&at93->lock);
	status = spi_write(at93->spi, bounce, 2);	
	mutex_unlock(&at93->lock);
	return status;
}

static int at93_ee_ewds(struct at93_data *at93){
	u16             temp;
	ssize_t			status;
	u8			bounce[2];
#if AT93_ORG == 1
	temp = (AT93_EWDS << (AT93_MAXADDRLEN-3));
#else
	temp = (AT93_EWDS << (AT93_MAXADDRLEN-2));
#endif
	bounce[0] = (u8)(temp >>8);
	bounce[1] = (u8)(temp);	
	mutex_lock(&at93->lock);
	status = spi_write(at93->spi, bounce, 2);	
	mutex_unlock(&at93->lock);
	return status;
}


static ssize_t
at93_ee_write(struct at93_data *at93, char *buf, loff_t off,ssize_t count)
{
	ssize_t			status = 0;
	unsigned		written = 0;
	char        *cp=NULL;
	u8			bounce[4];
	u16         temp=0,offset=off;
	/* Temp buffer starts with command and address */
	if(off>=AT93_CAP)
		return -EFAULT;
	if(at93_ee_ewen(at93)){
		dev_dbg(&at93->spi->dev, " at93 EWEN failed\n");		
		return -EFAULT;
	}
	cp = buf;
#if AT93_ORG == 1
	temp = (AT93_WR << (AT93_MAXADDRLEN-1)) + (((1<<(AT93_MAXADDRLEN -1))-1)& offset);
#else
	temp = (AT93_WR << (AT93_MAXADDRLEN)) + (((1<<(AT93_MAXADDRLEN))-1)& offset);
#endif
	mutex_lock(&at93->lock);
	while ((offset< AT93_CAP)&& (count> 0))
	{
		bounce[0] = (u8)(temp >>8);
		bounce[1] = (u8)(temp);
		bounce[2] = *cp++;	
#if	AT93_ORG == 1
		bounce[3] = *cp++;
		status = spi_write(at93->spi,bounce,4);
		count -= 2;
#else
		status = spi_write(at93->spi,bounce,3);			       
		count--;
#endif		
		if (status < 0) {
			printk("write data to spi eeprom failed\n");
			dev_dbg(&at93->spi->dev, "WREN --> %d\n",
					(int) status);
			break;
		}
		msleep(10);
#if	AT93_ORG == 1
		written += 2;
#else		
		written++;
#endif
		offset++;
		temp++;
	} 
	mutex_unlock(&at93->lock);
	at93_ee_ewds(at93);
	return written ? written : status;	
}

int spi_write_mac_addr(mac_address_p mac_value){
	unsigned int temp;
	int ret=0;
	char tx_buf[16];
	u16 offset=0,count;
	if(mac_value==NULL){
		ret = -1;
	}
	else{
		count = 13;
		tx_buf[0] = 'A';
		tx_buf[1] = 'C';
		tx_buf[2] = 'T';
		tx_buf[3] = 'M';
		tx_buf[4] = 'A';
		tx_buf[5] = 'C';
		tx_buf[6] = mac_value->mac_addr_0;
		tx_buf[7] = mac_value->mac_addr_1;
		tx_buf[8] = mac_value->mac_addr_2;
		tx_buf[9] = mac_value->mac_addr_3;
		tx_buf[10] = mac_value->mac_addr_4;
		tx_buf[11] = mac_value->mac_addr_5;
		temp = 0x1a9+tx_buf[6];
		temp +=tx_buf[7];
		temp +=tx_buf[8];
		temp +=tx_buf[9];
		temp +=tx_buf[10];
		temp +=tx_buf[11];
		tx_buf[12] = (unsigned char)temp;
		if(at93_ee_write(&spi_ee,tx_buf,offset,count)<0){
			ret = -1;
		}
	}
	return ret;
}

EXPORT_SYMBOL(spi_write_mac_addr);

int spi_read_mac_addr(mac_address_p mac_value){
	unsigned int temp;
	int ret=0,i;
	unsigned char rx_buf[20];
	u16 offset=0,count;
	if(mac_value==NULL){
		ret = -1;
	}
	else{
		count= 13;
		offset = 0;
#if NO_SUPPORT_SQE_READ ==ON
	#if	AT93_ORG == 1
		for(i=0;i<1+count/2;i++){
           	if(at93_ee_read(&spi_ee,rx_buf+(i<<1),offset+i,2)<0){
                ret = -1;
                goto rd_err;
            }			
		}
	#else
		for(i=0;i<count;i++){
			if(at93_ee_read(&spi_ee,rx_buf+i,offset+i,1)<0){
				ret = -1;
				goto rd_err;
			}			
		}
	#endif
#else
		if(at93_ee_read(&spi_ee,rx_buf,offset,count)<0){
			ret = -1;
			goto rd_err;
		}
#endif		
		if('A' != rx_buf[0]){
			ret = -1;			
			goto rd_err;
		}
		if('C' != rx_buf[1]){
			ret = -1;
			goto rd_err;
		}
		if('T' != rx_buf[2]){
			ret = -1;
			goto rd_err;
		}
		if('M' != rx_buf[3]){
			ret = -1;
			goto rd_err;
		}
		if('A' != rx_buf[4]){
			ret = -1;

			goto rd_err;
		}
		if('C' != rx_buf[5]){
			ret = -1;
			goto rd_err;
		}	
		temp = 0x1a9+rx_buf[6];
		temp +=rx_buf[7];
		temp +=rx_buf[8];
		temp +=rx_buf[9];
		temp +=rx_buf[10];
		temp +=rx_buf[11];
		if(rx_buf[12] != (unsigned char)temp){
			ret = -2;			
			printk("6:%d\n",rx_buf[12]);
			goto rd_err;			
		}
		mac_value->mac_addr_0 = rx_buf[6];
		mac_value->mac_addr_1 = rx_buf[7];
		mac_value->mac_addr_2 = rx_buf[8];
		mac_value->mac_addr_3 = rx_buf[9];
		mac_value->mac_addr_4 = rx_buf[10];
		mac_value->mac_addr_5 = rx_buf[11];	
		ret = 0;

	}
rd_err:
	return ret;
}

EXPORT_SYMBOL(spi_read_mac_addr);

/*-------------------------------------------------------------------------*/

static int  __devinit at93_probe(struct spi_device *spi)
{
	struct at93_data	*at93 = &spi_ee;
	int			err;
    int         status;
	at93_spi_pin_config();
	status = at93_read_byte(spi,5);
	if (status < 0 ) {
		dev_dbg(&spi->dev, "rdsr --> %d\n", status);
		err = -ENXIO;		
	}

	mutex_init(&at93->lock);
	at93->spi = spi_dev_get(spi);
	dev_set_drvdata(&spi->dev, at93);
	return 0;
}

/*
 * ----------------------------------------------------------------------------
 * Function Name: at93_remove
 * Purpose:
 * ----------------------------------------------------------------------------
 */
static int __devexit at93_remove(struct spi_device *spi)
{
	struct at93_data	*at93;	
	at93 = dev_get_drvdata(&spi->dev);
	kfree(at93);
	return 0;
}

static struct spi_driver at93_spi_driver = {
	.driver = {
		.name = "spidev",
		.owner = THIS_MODULE,
	},
	.probe = at93_probe,
	.remove = __devexit_p(at93_remove),
};

/*
 * ----------------------------------------------------------------------------
 * Function Name: at93_spi_init
 * Purpose:
 * ----------------------------------------------------------------------------
 */
static __init int at93_spi_init(void)
{
	printk(KERN_INFO "Register AT93CXX SPI EEPROM Driver.\n");
	return spi_register_driver(&at93_spi_driver);
}

/*
 * ----------------------------------------------------------------------------
 * Function Name: at93_spi_exit
 * Purpose:
 * ----------------------------------------------------------------------------
 */
static __exit void at93_spi_exit(void)
{
	spi_unregister_driver(&at93_spi_driver);
}

module_init(at93_spi_init);
module_exit(at93_spi_exit);
MODULE_AUTHOR("leiwengang<leiwg@actions-micro.com>");
MODULE_DESCRIPTION(" at93 series SPI interface EEPROM driver");
MODULE_LICENSE("GPL");

