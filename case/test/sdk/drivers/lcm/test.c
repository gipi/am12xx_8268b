#include "lcm_op.h"
#include "stdio.h"
#include "unistd.h"
#include "fcntl.h"

int main(void)
{
   int fd = open("/dev/lcm",O_RDWR); 	  
   int backlight_val;
   if(fd<0)
	printf("open lcm error\n");	
	
   backlight_val = 0;	
   ioctl(fd,BACKLIGHT_SET,&backlight_val);
   ioctl(fd,BACKLIGHT_GET,&backlight_val);	   
   printf("backlight value:%d\n",backlight_val);	  

   sleep(10);	
   	
   backlight_val = 31;	
   ioctl(fd,BACKLIGHT_SET,&backlight_val);
   ioctl(fd,BACKLIGHT_GET,&backlight_val);
   printf("backlight value:%d\n",backlight_val);	  

	   
   return 0;
}
