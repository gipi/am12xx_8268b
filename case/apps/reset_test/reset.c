/*  
创建一个守护进程，监视系统所有运行的进程   
*/    
#include <unistd.h>    
#include <signal.h>    
#include <sys/types.h>    
#include <sys/stat.h>    
#include <stdio.h>    
#include <stdlib.h>    
#include <sys/resource.h> 

//创建一个守护进程    
void init()    
{    
    int pid;    
    int i;           
    printf( "---------------%s %s %d \n",__FILE__,__FUNCTION__,__LINE__);
    if(pid = fork())    
        exit(0);                          //父进程，退出    
    
    else if(pid < 0)                          //开辟进程失败，退出并关闭所有进程    
        exit(1);    
    /* 子进程继续执行 */    
    setsid();                               //创建新的会话组，子进程成为组长，并与控制终端分离    
    
    /* 防止子进程（组长）获取控制终端 */    
    if(pid = fork())    
        exit(0);                       //父进程，退出     
    else if(pid < 0)     
           exit(1);                        //开辟进程失败，退出并关闭所有进程    
  
/* 第二子进程继续执行 , 第二子进程不再是会会话组组长*/    
    
    /* 关闭打开的文件描述符*/ 
	//int des=getdtablesize();
      //  for (i = 0; i <des; i++)    
           //  close(i);         
    chdir("/tmp"); // 切换工作目录     
    umask(0);       // 重设文件创建掩码     
    return;    
}    
    
int main()    
{    
  
    signal(SIGCHLD, SIG_IGN); // 忽略子进程结束信号，防止出现僵尸进程      
    init();           //初始化守护进程，就是创建一个守护进程
 //    printf( "---------------%s %s %d \n",__FILE__,__FUNCTION__,__LINE__);
    while(1)//每隔一分钟向test.log报告运行状态 
    { 
        sleep(3);//睡眠一分钟 
        if(access("/tmp/reset.txt",F_OK) == 0)
        {
		printf("it is ready to reset\n");
		system("killall -9 wireUI.app manager.app pthsystem.app");
		sleep(1);
		//system("ps");
		//printf("fuser\n");
		//system("fuser -m /mnt/vram");
 		system("umount  /mnt/vram");
		system("mkfs.vfat	/dev/partitions/vram");
		system("reboot");

        }
    } 

    return 0;    
} 
