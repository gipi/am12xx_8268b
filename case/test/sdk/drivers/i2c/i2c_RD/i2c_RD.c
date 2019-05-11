/**
* i2c驱动 的特别在于:
* read()/write()只完成一条消息的传送；
* 一次需完成多条消息的传送需要ioctl(fd,I2C_RDWR..)实现；
* 本测试文档实现的是:通过read()/write()实现单次一条消息的传送
*
* 注意:先要打开make menuconfig->device drivers->[*]i2c support->[*]i2c device interfae
**/
#include <stdio.h>
#include <linux/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h>
 
#define I2C_RETRIES      0x0701
#define I2C_TIMEOUT     0x0702
#define I2C_SLAVE_FORCE      	   0x0706
 
/**
* argv[1]: i2c子设备号，从0开始递增:/dev/i2c-0；
* argv[2]:device_address,为i2c子设备地址
* argv[3]:register_address,为设备的寄存器地址
* argv[4]:需要传送的消息条数；
* eg:   I2C_RD.app /dev/i2c-0 71 0 1 表示读取设备i2c-0的偏移0处的值,当然要知道该设备的地址为0x71
**/
int main(int argc, char **argv)
{
	unsigned int fd;
   	unsigned short device_address,register_address;
   	unsigned short size;
  	unsigned short idx;
   	#define BUFF_SIZE    32
   	char buf[BUFF_SIZE];
	char cswap;
	int ret;
	
	union
	{
		unsigned short addr;
		char bytes[2];
	} tmp;
	if (argc < 4)
	{
		printf("Use:\n%s /dev/i2c-x mem_addr size\n", argv[0]);
		return 0;
	}
   	sscanf(argv[2], "%x", &device_address);
   	sscanf(argv[3], "%x", &register_address);
	sscanf(argv[4], "%d", &size);

	if (size > BUFF_SIZE)
	 	size = BUFF_SIZE;
	fd = open(argv[1], O_RDWR);

   	if (!fd)
 	 {
   	 	 printf("Error on opening the device file\n");
     		return 0;
  	 }
 
	  ioctl(fd, I2C_SLAVE_FORCE, device_address); /* 设置从设备地址 */
	  ioctl(fd, I2C_TIMEOUT, 1); /* 设置超时 */
	  ioctl(fd, I2C_RETRIES, 1); /* 设置重试次数 */

	  for (idx = 0; idx < size; ++idx, ++register_address)
	   {
	    	tmp.addr = register_address;
	    	//cswap = tmp.bytes[0];
	    	//tmp.bytes[0] = tmp.bytes[1];
	    	//tmp.bytes[1] = cswap;
	     	write(fd, &tmp.addr, 1); //send the register addr
		ret = read(fd, &buf[idx], 1);//read datas from the register
		if (ret < 0)
		 {
		     printf("Error during I2C_RDWR ioctl with error code: %d\n", ret);
		     return 0;
		 } 
		printf("buf[%d]=0x%x\n",idx, buf[idx]);

	   }
   	close(fd);
   	return 0;
 }


