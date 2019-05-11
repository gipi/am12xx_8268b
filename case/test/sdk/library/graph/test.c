#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

#include <sys_buf.h>

#include "amTypedef.h"
//#include "swf_types.h"
#include "GuiDraw.h"
#include "osapi.h"
#include "display.h"


#include "mem_data.h"



#define act_writel(val,reg)  (*(volatile unsigned int *)(reg) = (val))
#define act_readl(port)  (*(volatile unsigned int *)(port))
static void RegBitSet(int val,int reg,int msb,int lsb){                                             
unsigned int mask = 0xFFFFFFFF;
unsigned int old  = act_readl(reg);

	mask = mask << (31 - msb) >> (31 - msb + lsb) << lsb;
	act_writel((old & ~mask) | ((val << lsb) & mask), reg);	         
}                                                                                                                                                                                                                                                                                
static unsigned int RegBitRead(int reg,int msb,int lsb){                                                     
	unsigned int valr;
	
	valr=act_readl(reg);                                                                       
	return (valr<<(31-msb)>>(31-msb+lsb));                                                     
}

typedef struct _mem
{
	int phy_add;
	int vir_add;
	int mem_size;
} MEM;

MEM mem;

typedef struct _test_info
{
	char ic_type[16];
	int num;
} TEST_INFO;

TEST_INFO test_info;

int mem_map(MEM* mem,int size)
{

	struct mem_dev example ;
	int fd;

#if 1	
	mem->mem_size=size;
	printf("size=%d\n",size);
  example.request_size = mem->mem_size;
	 fd = open("/dev/sysbuf",O_RDWR);
	 ioctl(fd,MEM_GET,&example);
	 mem->phy_add= (int)example.physic_address;
	 mem->vir_add=(int)example.logic_address;	
	 
	 memset((void*)(mem->vir_add),0x0,size);
	 #endif 
	 return 0;
           
}
int RegMap(int* flag)
{
	return 0;
}
void image_test_handler(int signal)
{
	int reg_add;
	char fname[128];
	static int reg_map_falg=0;
	FILE* fp;
	int add=0;
	int size=0;
	int width=0;
	int height=0;
	int format=0;
	
	reg_add=RegMap(&reg_map_falg);
	if(strcmp(test_info.ic_type,"am1203")==0)
	{	
		add=mem.vir_add+((RegBitRead(reg_add+0x18,27,0)|0xa0000000)-mem.phy_add);
		width=RegBitRead(reg_add+0x10,11,0);
		height=RegBitRead(reg_add+0x14,27,16);
		format=RegBitRead(reg_add+0x14,15,14);
		if(format==0)
			size=width*height*4;
		else if(format==3)
			size=width*height*2;
	}	
	sprintf(fname, "format=%d_%dx%d.out",format,width,height);
	fp = fopen(fname, "w+");
	if(fp == NULL)
	{
		printf("fopen %s error\n",fname);
		return ;
	}
  fwrite((int*)add,1,size,fp);
  fclose(fp);
}
void reg_test_handler(int signal)
{
	int reg_add;
	int reg_val;
	char fname[128];
	FILE *fp = NULL;
	static int reg_map_falg=0;
	char para[64];
	int i;
	
	
	printf("reg_test_handler \n");
	reg_add=RegMap(&reg_map_falg);
	#if 0
	sprintf(fname, "./output_now/%s/case_%d",test_info.ic_type,test_info.num);
	fp = fopen(fname, "w+");
	if(fp == NULL)
	{
		printf("fopen %s error\n",fname);
		return ;
	}
	for(i=0;i<0x100;i=i+4)
	{
		reg_val=act_readl(reg_add+i);
		fprintf(fp,"/*0x%x*/0x%08x\n",0xb0048000+i,reg_val);
	}	
	fclose(fp);
	#endif
		for(i=0;i<0x100;i=i+4)
	{
		reg_val=act_readl(reg_add+i);
		printf("/*0x%x*/0x%08x\n",0xb0048000+i,reg_val);
	}
	
	
}
#if 0
void register_async_event(void (*p)(int))
{
	unsigned int oflags;
	struct sigaction sa;
	int fd;

	
   /* asynchronus notification handler */
	  memset(&sa, 0, sizeof(sa));
	 // sa.sa_handler = p;
	 sa.sa_handler = reg_test_handler;
	  //sa.sa_flags |= SA_RESTART;  /* restart of system calls */
	  sa.sa_flags=0;
	  sigaction(SIGIO, &sa, NULL);
	 /* Register with the kernel module that this process is registered for the signal */
   /* asynchronus notification */
  fd=open("/dev/graph", O_RDWR);
  fcntl(fd, F_SETOWN, getpid()); /* this process will receive SIGIO */
  oflags = fcntl(fd, F_GETFL);
  fcntl(fd, F_SETFL, oflags | FASYNC);   /* set ASYNC notification flag */
  close(fd);
}
#endif
int initial_parameter(FILLINFO* fillInfo,TEST_INFO* test)
{
	char filename[64];
	char para[64];
	FILE* fp=NULL;
		

	sprintf(filename, "./input/case_%d", test->num);
	fp = fopen(filename, "r");
	if(fp == NULL)
	{
		//printf("fopen %s error\n",filename);
		return -1;
	}
	while(fscanf(fp, "%s\n", para) > 0)
	{		
		if(strcmp(para,"DefColor=")==0)
			fscanf(fp, "0x%x\n", &(fillInfo->DefColor));
		else if(strcmp(para,"ScaleX=")==0)
			fscanf(fp, "0x%x\n", &((fillInfo->Matrix).ScaleX));
		else if(strcmp(para,"ScaleY=")==0)
			fscanf(fp, "0x%x\n", &((fillInfo->Matrix).ScaleY));
		else if(strcmp(para,"RotateSkew0=")==0)
			fscanf(fp, "0x%x\n", &((fillInfo->Matrix).RotateSkew0));
		else if(strcmp(para,"RotateSkew1=")==0)
			fscanf(fp, "0x%x\n", &((fillInfo->Matrix).RotateSkew1));	
		else if(strcmp(para,"TranslateX=")==0)
			fscanf(fp, "0x%x\n", &((fillInfo->Matrix).TranslateX));
		else if(strcmp(para,"TranslateX=")==0)
			fscanf(fp, "0x%x\n", &((fillInfo->Matrix).TranslateX));
		else if(strcmp(para,"input_addr=")==0)
			fscanf(fp, "0x%x\n", &(fillInfo->input_addr));
		else if(strcmp(para,"input_type=")==0)
			fscanf(fp, "0x%x\n", &(fillInfo->input_type));	
		else if(strcmp(para,"input_stride=")==0)
			fscanf(fp, "0x%x\n", &(fillInfo->input_stride));
		else if(strcmp(para,"input_start_x=")==0)
			fscanf(fp, "0x%x\n", &(fillInfo->input_start_x));
		else if(strcmp(para,"input_start_y=")==0)
			fscanf(fp, "0x%x\n", &(fillInfo->input_start_y));
		else if(strcmp(para,"input_height=")==0)
			fscanf(fp, "0x%x\n", &(fillInfo->input_height));	
		else if(strcmp(para,"input_width=")==0)
			fscanf(fp, "0x%x\n", &(fillInfo->input_width));
		else if(strcmp(para,"input_repeat=")==0)
			fscanf(fp, "0x%x\n", &(fillInfo->input_repeat));
		else if(strcmp(para,"input_format=")==0)
			fscanf(fp, "0x%x\n", &(fillInfo->input_format));
		else if(strcmp(para,"input_need_blend=")==0)
			fscanf(fp, "0x%x\n", &(fillInfo->input_need_blend));	
		else if(strcmp(para,"input_need_interpolate=")==0)
			fscanf(fp, "0x%x\n", &(fillInfo->input_need_interpolate));
		else if(strcmp(para,"output_addr=")==0)
			fscanf(fp, "0x%x\n", &(fillInfo->output_addr));
		else if(strcmp(para,"output_type=")==0)
			fscanf(fp, "0x%x\n", &(fillInfo->output_type));
		else if(strcmp(para,"output_offset=")==0)
			fscanf(fp, "0x%x\n", &(fillInfo->output_offset));	
		else if(strcmp(para,"output_stride=")==0)
			fscanf(fp, "0x%x\n", &(fillInfo->output_stride));
		else if(strcmp(para,"output_width=")==0)
			fscanf(fp, "0x%x\n", &(fillInfo->output_width));
		else if(strcmp(para,"output_height=")==0)
			fscanf(fp, "0x%x\n", &(fillInfo->output_height));
		else if(strcmp(para,"output_format=")==0)
			fscanf(fp, "0x%x\n", &(fillInfo->output_format));	
		else if(strcmp(para,"pair_addr=")==0)
			fscanf(fp, "0x%x\n", &(fillInfo->pair_addr));
		else if(strcmp(para,"pair_num=")==0)
			fscanf(fp, "0x%x\n", &(fillInfo->pair_num));
		else if(strcmp(para,"z_buffer=")==0)
			fscanf(fp, "0x%x\n", &(fillInfo->z_buffer));
		else if(strcmp(para,"z_stride=")==0)
			fscanf(fp, "0x%x\n", &(fillInfo->z_stride));	
		else if(strcmp(para,"z_depth=")==0)
			fscanf(fp, "0x%x\n", &(fillInfo->z_depth));
		else if(strcmp(para,"aa_mode=")==0)
			fscanf(fp, "0x%x\n", &(fillInfo->aa_mode));																											
		}
	fclose(fp);
	return 0;			
}

int initial_mem_data(FILLINFO* fillInfo)
{
	int i;
	int dst_image_phy;
	int dst_image_vir;
	int src_image_phy;
	int src_image_vir;
	int pp_add_phy   ;
	int pp_add_vir   ;	
	
	mem_map(&mem,0x200000);//<0x400000 
	dst_image_phy=(mem.phy_add);                        
	dst_image_vir=(mem.vir_add);                        
	src_image_phy=(dst_image_phy+sizeof(dst_image));
	src_image_vir=(dst_image_vir+sizeof(dst_image));
	pp_add_phy   =(src_image_phy+sizeof(src_image));
	pp_add_vir   =(src_image_vir+sizeof(src_image));	
	for(i=0;i<sizeof(dst_image)/4;i++)
		act_writel(dst_image[i],dst_image_vir+4*i);
	fillInfo->output_addr=dst_image_phy;
	for(i=0;i<sizeof(src_image)/4;i++)
		act_writel(src_image[i],src_image_vir+4*i);
	fillInfo->input_addr=src_image_phy;
	for(i=0;i<sizeof(pp_add)/4;i++)
		act_writel(pp_add[i],pp_add_vir+4*i);
	fillInfo->pair_addr=pp_add_phy;
	
	return 0;
			
}
int image_test(TEST_INFO* test)//Not complete
{
	FILLINFO fillInfo;
	int ret;
	
	ret=initial_parameter(&fillInfo,test);
	if(ret<0)
		return ret;
	initial_mem_data(&fillInfo);
	Cont2DInt(1);
	register_async_event(image_test_handler);
	Render(&fillInfo);	
	return 0;
}
int reg_test(TEST_INFO* test)
{
	FILLINFO fillInfo;
	int ret;
	
	ret=initial_parameter(&fillInfo,test);
	if(ret<0)
		return ret;
	Cont2DInt(1);
	register_async_event(reg_test_handler);
	Render(&fillInfo);
	printf("reg_test %d\n",test->num);
	
	return 0;
	
}
void  get_current_canvas(IMGCANVAS *cvs)
{
	int fd;
	int DeBaseadd;
	int buf_vadd_ori;
	int buf_padd_ori;	
	int buf_offset;
	int buf_type;
	int buf_width;
	int buf_height;
	unsigned int buf_size;



	fd = open("/dev/mem", O_RDWR | O_SYNC);	
	if(fd<0)
		printf("open/dev/mem error\n");
	DeBaseadd=(int)mmap(0, 0x200, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0x10040000);
	printf("DeBaseadd=%x\n",DeBaseadd);
	buf_padd_ori=RegBitRead(DeBaseadd+0x10,27,0)&0x0ffff000;
	buf_offset=RegBitRead(DeBaseadd+0x10,27,0)&0x0fff;
	buf_type=RegBitRead(DeBaseadd,17,16);//InImageMode;	
	buf_width=RegBitRead(DeBaseadd+0x64,26,16);
	buf_height=RegBitRead(DeBaseadd+0x64,10,0);
	if((buf_width==0)||(buf_height==0))
	{
		buf_width=RegBitRead(DeBaseadd+0x8,26,16);
		buf_height=RegBitRead(DeBaseadd+0x8,10,0);
	}
	buf_size=((buf_width*buf_height*2)&0x0ffff000)+0x2000;
	buf_vadd_ori=(int)mmap(0, buf_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf_padd_ori);
	if(buf_vadd_ori==0)
		printf("mmap failed\n");
	//memset(dev->dev_vaddr,0,buf_size-0x2000);
	
	if(buf_type==1)
		cvs->pixel_format=3;
	else if(buf_type==2)
		cvs->pixel_format=0;
	cvs->ori_paddr=buf_padd_ori+buf_offset;
	cvs->ori_vaddr=buf_vadd_ori+buf_offset;
	cvs->width=buf_width;
	cvs->height=buf_height;
	//close(fd);
}
#if 0
int main(int argc,char **argv)
{
	int i;
	int STARTNO=0;
	int ENDNO=0;
	int cmu;
	int interrupt;
	int reg_add;
	int reg_val;
	static int reg_map_falg=0;
	int fd;
	
printf("2d:main\n");
	
	if(argc < 2)
	{
		printf("usage:  -ic  [ic type for test]\n");
		return -1;
	};
	for(i=1;i<argc;i++)
	{
		if(strcmp(argv[i], "-ic") == 0)
		{
			i++;
			strcpy(test_info.ic_type,argv[i]);
		}
	};
	
		fd = open("/dev/mem", O_RDWR | O_SYNC);	
	if(fd<0)
		printf("open/dev/mem error\n");
	interrupt=(int)mmap(0, 0x100, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0x10020000);
	cmu=(int)mmap(0, 0x100, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0x10010000);
	close(fd);
	reg_add=RegMap(&reg_map_falg);
	#if 0
	for(i=0;i<0x100;i=i+4)
	{
		reg_val=act_readl(cmu+i);
		printf("/*0x%x*/0x%08x\n",0xb0010000+i,reg_val);
	}
	for(i=0;i<0x20;i=i+4)
	{
		reg_val=act_readl(interrupt+i);
		printf("/*0x%x*/0x%08x\n",0xb0020000+i,reg_val);
	}
#endif
	
	for(test_info.num=STARTNO;test_info.num<=ENDNO;test_info.num++)
	{
		reg_test(&test_info);
		if (test_info.num==0)	
		{	
			while(0)
			{
					for(i=0;i<0x100;i=i+4)
					{
						reg_val=act_readl(cmu+i);
						printf("/*0x%x*/0x%08x\n",0xb0010000+i,reg_val);
					}
					for(i=0;i<0x20;i=i+4)
					{
						reg_val=act_readl(interrupt+i);
						printf("/*0x%x*/0x%08x\n",0xb0020000+i,reg_val);
					}			
					for(i=0;i<0x100;i=i+4)
					{
						reg_val=act_readl(reg_add+i);
						printf("/*0x%x*/0x%08x\n",0xb0048000+i,reg_val);
					}
				}	
			}

	};
	//image_test(&test_info);

while(1);
	return 0;
}
#else

int main(int argc,char **argv)
{
	IMGCANVAS src,dst;
	void* ds_inst;
	DE_config conf;
	POINT pointArray[5];
	POLYGON polygon;
	RECT rect;
	OS_BUF mem,*Mem=&mem;
		
	get_current_canvas(&src);
	//mem_map(&mem,(src.width)*(src.height)*2);
	Mem->size=(src.width)*(src.height)*2;
	OSBufMalloc(Mem);
	if(Mem->vir_addr==(int)NULL)
	{
		OSprintf("mem_malloc failed\n");
		return -1;
	}
	OSmemset((void*)(Mem->vir_addr),0x0,Mem->size);
	printf("mem_map ok\n");
	
	dst.pixel_format=src.pixel_format;
	dst.width=src.height;
	dst.height=src.width;	 
	dst.ori_vaddr=Mem->vir_addr;
	dst.ori_paddr=Mem->phy_addr;	
	dst.matrix.ScaleX=0*65536;//cos;
	dst.matrix.ScaleY=0*65536;//cos;
	dst.matrix.RotateSkew0=1*65536;//sin;
	dst.matrix.RotateSkew1=-1*65536;//-sin;
	dst.matrix.TranslateX=src.width*65536;
	dst.matrix.TranslateY=0;
	GuiTransfer(&dst,&src);

	
	printf("display \n");
	de_init(&ds_inst);
	//get config
	de_get_config(ds_inst,&conf,DE_CFG_ALL);
	//your config
	conf.input.bus_addr=dst.ori_paddr;
	conf.input.enable=1;
	conf.input.width=dst.width;
	conf.input.height=dst.height;
	de_set_Config(ds_inst,&conf,DE_CFG_ALL);
	de_release(ds_inst);
	printf("display OK \n");
	
	#if 0
	src.roi_rect.x=0;
	src.roi_rect.y=0;
	src.roi_rect.width=300;
	src.roi_rect.height=300;
	dst.pixel_format=src.pixel_format;
	dst.width=src.width;
	dst.height=src.height;	 
	dst.ori_vaddr=Mem->vir_addr;
	dst.ori_paddr=Mem->phy_addr;
	dst.roi_rect.x=200;
	dst.roi_rect.y=100;
	dst.roi_rect.width=src.roi_rect.width;
	dst.roi_rect.height=src.roi_rect.height;
	//GuiTranslation(&dst,&src);

	GuiSetBrush(0xff0000);
	 pointArray[0].x=400;
	 pointArray[0].y=100;
	 pointArray[1].x=700;
	 pointArray[1].y=100; 
	 pointArray[2].x=700;
	 pointArray[2].y=400;
	 pointArray[3].x=550;
	 pointArray[3].y=400;
	 polygon.npoints=4;
	 polygon.pointArray=pointArray;
	printf("translation\n");
	GuiFillPolygon(&dst,&polygon);




	GuiSetBrush(0xff0000);
	rect.left=10;
	rect.right=20;
	rect.top=10;
	rect.bottom=20;

	GuiFillEllipse(&dst,&rect);
	GuiDrawRect(&dst,&rect);
	
	
	printf("display \n");
	de_init(&ds_inst);
	//get config
	de_get_config(ds_inst,&conf,DE_CFG_ALL);
	//your config
	conf.input.bus_addr=dst.ori_paddr;
	conf.input.enable=0;
	de_set_Config(ds_inst,&conf,DE_CFG_ALL);
	de_release(ds_inst);
	printf("display OK \n");
	while(1)
	{
	GuiSetBrush(0xff0000);
	rect.left=10;
	rect.right=30;
	rect.top=10;
	rect.bottom=30;

	GuiFillEllipse(&dst,&rect);
	//GuiFillRect(&dst,&rect);
	}
		#endif
	OSBufFree(Mem);
	
	return 0;
	
}
#endif

