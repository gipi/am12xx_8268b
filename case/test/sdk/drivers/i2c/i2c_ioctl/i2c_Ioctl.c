/**
* i2c驱动 的特别在于:
* read()/write()只完成一条消息的传送；
* 一次需完成多条消息的传送需要ioctl(fd,I2C_RDWR..)实现；
* 本测试文档实现的是:通过ioctl实现单次多条消息的传送
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
#define I2C_RDWR          0x0707
#define MSG_LEN 1

struct i2c_msg
{
   __u16 addr; 		/* 从地址 */
   __u16 flags;
   #define I2C_M_RD        0x01
   __u16 len;			/* msg length	*/
   __u8 *buf; 			/* 消息数据指针 */
 };

 struct i2c_rdwr_ioctl_data
 {
  struct i2c_msg *msgs; /* i2c_msg[]指针 */
  int nmsgs; 			/* i2c_msg数量 */
 };
 
/**
* argv[1]: i2c子设备号，从0开始递增；
* argv[2]:device_address,为i2c子设备地址
* argv[3]:register_address,为设备的寄存器地址
* argv[4]:需要传送的消息条数；
* eg:   I2C_IOCTL.app /dev/i2c-0 71 0 1 表示读取设备i2c-0的偏移0处的值,当然要知道该设备的地址为0x71
**/
 int main(int argc, char **argv)
{
  struct i2c_rdwr_ioctl_data work_queue;
  unsigned int idx;
  unsigned int fd,num;
  unsigned short device_address,register_address;
   int ret;
  #define BUFF_SIZE    32
  char tmp[BUFF_SIZE] = {0};
  
   if (argc < 4)
   {
    	printf("Usage:\n%s /dev/i2c-x start_addr\n", argv[0]);
     	return 0;
   }
 
   fd = open(argv[1], O_RDWR);
 
   if (!fd)
   {
    	printf("Error on opening the device file\n");
     	return 0;
   }
   sscanf(argv[2], "%x", &device_address);
   sscanf(argv[3], "%x", &register_address);
   sscanf(argv[4], "%d", &num); /* 消息数量 */
   
   work_queue.nmsgs = num+1;
   work_queue.msgs = (struct i2c_msg*)malloc(work_queue.nmsgs *sizeof(struct i2c_msg));
   if (!work_queue.msgs)
   {
    	printf("Memory alloc error\n");
    	close(fd);
     	return 0;
   }

   //send the read register address first!   
  (work_queue.msgs[0]).flags  = 0;
  (work_queue.msgs[0]).len = 1;
  (work_queue.msgs[0]).addr = device_address;
  (work_queue.msgs[0]).buf = &register_address;

//then make the read buf  point to tmp[]
  for (idx = 1; idx < work_queue.nmsgs; ++idx)
   {
   	(work_queue.msgs[idx]).flags |= I2C_M_RD;
    	(work_queue.msgs[idx]).len = 1;
    	(work_queue.msgs[idx]).addr = device_address;
     	(work_queue.msgs[idx]).buf = &tmp[idx];
  }
 
   ioctl(fd, I2C_TIMEOUT, 2); /* 设置超时 */
   ioctl(fd, I2C_RETRIES, 1); /* 设置重试次数 */

   ret = ioctl(fd, I2C_RDWR, (unsigned long) &work_queue);
  for (idx = 1; idx < work_queue.nmsgs; ++idx)
   {
   	printf("read buf[%d] = 0x%x\n",idx,tmp[idx]);
  }
   if (ret < 0)
   {
     printf("Error during I2C_RDWR ioctl with error code: %d\n", ret);
   }
 
   close(fd);
   free( work_queue.msgs );
  return 0 ;
 }



