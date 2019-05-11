

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <linux/string.h>

#include "sys_buf.h"
#include "osapi.h"
#include "display.h"

typedef unsigned char u8;

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

typedef struct _mem_
{
	int phy_addr;
	int vir_addr;
	int size;
	int mem_pgoff;
	int ret;
}OS_BUF;


static void OSBufMalloc(OS_BUF* mem)
{
#if 1
	struct mem_dev mem1,*Mem=&mem1;
	int fd;
	int ret;
	 
	fd = open("/dev/sysbuf",O_RDWR);	
	if(fd<0)
		printf("OSmemMalloc failed\n");
	else
	{
		Mem->buf_attr=UNCACHE;
		//Mem->buf_attr=CACHE;
		Mem->request_size=mem->size;
		ret=ioctl(fd,MEM_GET,Mem);	 
		if(ret<0)
		{
			printf("OSmemMalloc failed\n");
			mem->phy_addr=0;
			mem->vir_addr=0;
		}else
		{
			mem->phy_addr=Mem->physic_address;
			mem->vir_addr=Mem->logic_address;
			mem->mem_pgoff=Mem->mem_pgoff;
			mem->ret=Mem->ret;
		}
	}
	close(fd);
#else
	void* ret;
	
	ret=(void*)SWF_Malloc(mem->size);
	if(ret!=NULL)
	{
		mem->phy_addr=(int)SWF_MemVir2Phy(ret);
		mem->vir_addr=(int)ret;
		mem->mem_pgoff=0;
		mem->ret=(int)ret;	
	}else
	{
		printf("OSmemMalloc failed\n");
		mem->phy_addr=0;
		mem->vir_addr=0;		
	}
#endif

}

static void OSBufFree(OS_BUF* mem)
{
#if 1
	struct mem_dev mem1,*Mem=&mem1;
	int fd;
	int ret;
	
	if(mem->vir_addr!=0)
	{
		fd = open("/dev/sysbuf",O_RDWR);	
		if(fd<0)
			printf("OSmemFree failed\n");
		else
		{	
			//Mem->buf_attr=UNCACHE;
			Mem->buf_attr=CACHE;
			Mem->logic_address=mem->vir_addr;
			Mem->mem_pgoff=mem->mem_pgoff;
			Mem->physic_address=mem->phy_addr;
			Mem->request_size=mem->size;
			Mem->ret=mem->ret;
			ret=ioctl(fd,MEM_PUT,Mem);
			if(ret<0)
				printf("OSmemFree failed\n");
			else
			{
				mem->phy_addr=0;
				mem->vir_addr=0;
			}
		}
		close(fd);
	}
#else
	if((void*)(mem->vir_addr)!=NULL)
		SWF_Free((void *)mem->vir_addr);
#endif
}

#define DE_REG_NUM 64
void range_limit(int* in,int min,int max)
{
	if(*in>max)
		*in=max;
	else if(*in<min)
		*in=min;
}
void RGBToYCbCr(u8* y,u8* cb, u8* cr,u8* r,u8* g,u8* b,int width,int height)
{

	int temp;
	int i;

	//rgb->ycbcr bt709
	
		for(i=0;i<(width*height);i++)
		{
			temp=(183*(*(r+i))+614*(*(g+i))+62*(*(b+i)))/1000+16;
			range_limit(&temp,0,255);
			*(y+i)=temp;
			temp=(-101*(*(r+i))-338*(*(g+i))+439*(*(b+i)))/1000+128;
			range_limit(&temp,0,255);
			*(cb+i)=temp;
			temp=(439*(*(r+i))-399*(*(g+i))-40*(*(b+i)))/1000+128;
			range_limit(&temp,0,255);
			*(cr+i)=temp;
		}
		
}


void bmp_to_image(DE_config *conf)
{
	FILE* fp;
	int bmp_width,bmp_height,bmp_offset;
	u8 *rin,*gin,*bin,*yin,*cbin,*crin;
	int i,j;
	int alpha;
	u8* buffer;
	struct mem_dev mem ;
	int fmt=0;
	int width=0,height=0;
	int fd;
	int ret;


	width=conf->input.width;
	height=conf->input.height;
	fmt=conf->input.pix_fmt;
	buffer=(u8*)(conf->input.img);
//		#if 0
//		buffer=malloc(width*height*4);
//		#else
//  		mem.request_size =width*height*2/4096*4096+4096;
//	 	fd = open("/dev/sysbuf",O_RDWR);
//	 	ret=ioctl(fd,MEM_GET,&mem);
//	 	if(ret<0)
//	 	{
//	 		printf("memory failed");
//	 		return ;
//	 	}
//		buffer=(u8*)mem.logic_address;
//		conf->input.bus_addr=(int)mem.physic_address;
//		conf->input.bus_addr_uv=(int)mem.physic_address+width*height;
//		#endif	
	fp=fopen("./input/input.bmp","rb");
	if(fp==NULL)
		printf("fopen error\n");
	fseek(fp,0xa,0);
	fread(&bmp_offset,4,1,fp);
	fseek(fp,0x12,0);
	fread(&bmp_width,4,1,fp);
	fread(&bmp_height,4,1,fp);
	if(bmp_height<0)
		bmp_height=-bmp_height;
		
	printf("bmp_width=%d\n",bmp_width);
	printf("bmp_height=%d\n",bmp_height);
	rin=(u8*)malloc(bmp_width*bmp_height);
	if(rin==NULL)
	{
		printf("malloc failed\n");
		return;
	}		
	gin=(u8*)malloc(bmp_width*bmp_height);
	if(gin==NULL)
	{
		printf("malloc failed\n");
		return;
	}	
	bin=(u8*)malloc(bmp_width*bmp_height);
	if(bin==NULL)
	{
		printf("malloc failed\n");
		return;
	}	
	yin=(u8*)malloc(bmp_width*bmp_height);
	if(yin==NULL)
	{
		printf("malloc failed\n");
		return;
	}	
	cbin=(u8*)malloc(bmp_width*bmp_height);
	if(cbin==NULL)
	{
		printf("malloc failed\n");
		return;
	}	
	crin=(u8*)malloc(bmp_width*bmp_height);
	if(crin==NULL)
	{
		printf("malloc failed\n");
		return;
	}	
	printf("malloc success\n");	
    fseek(fp,bmp_offset,0);
	for(i=0;i<bmp_width*bmp_height;i++)
	{

		*(bin+i)=fgetc(fp);
		*(gin+i)=fgetc(fp);
		*(rin+i)=fgetc(fp);
		alpha=fgetc(fp);

	}
	printf("read file success \n");	
	fclose(fp);

	if((fmt==DE_PIX_FMT_YCBCR_4_2_0_SEMIPLANAR)||(fmt==DE_PIX_FMT_YCBCR_4_2_2_INTERLEAVED))
		RGBToYCbCr(yin,cbin,crin,rin,gin,bin,bmp_width,bmp_height);
	printf("RGBToYCbCr success\n");

	if(fmt==DE_PIX_FMT_YCBCR_4_2_0_SEMIPLANAR)
	{	
		int  add_offset;
		u8  *c_buffer;
		
		for(i=0;i<height;i++)
		{
			add_offset=i*bmp_width;
			for(j=0;j<width;j++)
			{
				add_offset=add_offset+1;
				*(buffer+i*width+j)=*(yin+add_offset);
			}
		}		
		c_buffer=buffer+width*height;
		for(i=0;i<height/2;i++)
		{
			add_offset=2*i*bmp_width;
			for(j=0;j<width/2;j++)
			{
				add_offset=add_offset+2;
				*(c_buffer+i*width+2*j)=*(cbin+add_offset);
				*(c_buffer+1+i*width+2*j)=*(crin+add_offset);
			}
		}
	}else if(fmt==DE_PIX_FMT_YCBCR_4_2_2_INTERLEAVED)
	{
		
		for(i=0;i<height;i++)
		{
			for(j=0;j<width;j++)
			{
				if((i>=bmp_height)||(j>=bmp_width))
					*(buffer+(i*width+j)*2)=0x10;
				else 
					*(buffer+(i*width+j)*2)=*(yin+i*bmp_width+j);
			}
		}
		for(i=0;i<height;i++)
		{
			for(j=0;j<width/2;j++)
			{
				if((i>=bmp_height)||(j>=bmp_width/2))
				{
					*(buffer+1+(i*width/2+j)*4)=0x80;
					*(buffer+3+(i*width/2+j)*4)=0x80;
				}
				else 
				{
					*(buffer+1+(i*width/2+j)*4)=*(cbin+i*bmp_width+j*2);
					*(buffer+3+(i*width/2+j)*4)=*(crin+i*bmp_width+j*2);
				}
			}
		}
		printf("DE_PIX_FMT_YCBCR_4_2_2_INTERLEAVED\n");
	}
	if(0)
	{
		FILE* fp;
		int length;

		if(fmt==DE_PIX_FMT_YCBCR_4_2_2_INTERLEAVED)
			length=width*height*2;
		else
			length=width*height*3/2;
		fp=fopen("D:/soft_system/de_porject/test_code/output/buffer.yuv","wb+");
		fwrite(buffer,length,1,fp);
		fclose(fp);

	}

	free(rin);
	free(gin);
	free(bin);
	free(yin);
	free(cbin);
	free(crin);
	
//		#if 0
//		free(buffer);
//		#else
//	 	ioctl(fd,MEM_PUT,&buffer);
//		close (fd);
//		#endif	
}
void bmp_to_osd_16bit(DE_config *conf)
{
	FILE* fp;
	int bmp_width,bmp_height,bmp_offset;
	u8 *rin,*gin,*bin;
	int i,j,k;
	int alpha;
	u8* buffer;
	DE_OSD_input* osd_input[2]={&(conf->osd_input1),&(conf->osd_input2)};
	DE_OSD_output* osd_output[2]={&(conf->osd_output1),&(conf->osd_output2)};
	struct mem_dev mem ;
	int fmt=0;
	int width=0,height=0;
	int fd;
	int ret;
	
	for(k=0;k<2;k++)
	{
		if(osd_input[k]->enable==1)
		{
			if(osd_input[k]->pix_fmt==DE_PIX_FMT_OSDBIT16MODE)
			{
				width=osd_input[k]->stride;
				height=osd_output[k]->height;
				fmt=osd_input[k]->pix_fmt;
				buffer=(u8*)(osd_input[k]->img);
//				#if 0
//				buffer=malloc(width*height*4);
//				#else
//  				mem.request_size =width*height*2/4096*4096+4096;
//				fd = open("/dev/sysbuf",O_RDWR);
//				ret=ioctl(fd,MEM_GET,&mem);
//				if(ret<0)
//				{
//	 				printf("memory failed");
//	 				return ;
//				}
//				buffer=(u8*)mem.logic_address;
//				osd_input[k]->bus_addr=(int)mem.physic_address;
//				#endif	
				fp=fopen("./input/osd_16bit.bmp","rb");
				if(fp==NULL)
					printf("fopen error\n");
				fseek(fp,0xa,0);
				fread(&bmp_offset,4,1,fp);
				fseek(fp,0x12,0);
				fread(&bmp_width,4,1,fp);
				fread(&bmp_height,4,1,fp);
				if(bmp_height<0)
					bmp_height=-bmp_height;
				printf("bmp_width=%d\n",bmp_width);
				printf("bmp_height=%d\n",bmp_height);
				rin=(u8*)malloc(bmp_width*bmp_height);
				if(rin==NULL)
				{
					printf("malloc failed\n");
					return;
				}		
				gin=(u8*)malloc(bmp_width*bmp_height);
				if(gin==NULL)
				{
					printf("malloc failed\n");
					return;
				}	
				bin=(u8*)malloc(bmp_width*bmp_height);
				if(bin==NULL)
				{
					printf("malloc failed\n");
					return;
				}	
				printf("malloc success\n");	
				fseek(fp,bmp_offset,0);
				for(i=0;i<bmp_width*bmp_height;i++)
				{

					*(bin+i)=fgetc(fp);
					*(gin+i)=fgetc(fp);
					*(rin+i)=fgetc(fp);
					alpha=fgetc(fp);

				}
				printf("read file success \n");	
				fclose(fp);

	
				for(i=0;i<height;i++)
				{
					for(j=0;j<width;j++)
					{
						unsigned int b,g,r,data;
						b=*(bin+i*bmp_width+j)/8;
						g=*(gin+i*bmp_width+j)/4;
						r=*(rin+i*bmp_width+j)/8;
						data=b|(g<<5)|(r<<11);
						*(unsigned short*)(buffer+(i*width+j)*2)=data;
					}
				}
				printf("OSDBIT16MODE\n");

				free(rin);
				free(gin);
				free(bin);
				
				
//				#if 0
//				free(buffer);
//				#else
//				ioctl(fd,MEM_PUT,&buffer);
//				close (fd);
//				#endif	
			}
		}
	}

}
static 
void rgb565_to_idx8_2(unsigned char *src,
					int src_stride,
					int width,
					int height,
					unsigned char *dst,
					int dst_stride,
					unsigned short *idx)
{
	int i;
	int tmp_tbl[5][10][5][4];
	int scale_r = 1;
	int scale_g = 1;
	int scale_b = 1;
	int max_r = 0;
	int max_g = 0;
	int max_b = 0;
	int min_r = (1<<5)-1;
	int min_g = (1<<6)-1;
	int min_b = (1<<5)-1;
	
	memset(tmp_tbl, 0, sizeof(tmp_tbl));
	for(i=0;i<height;i++)
	{
		unsigned short *s = (unsigned short*)(src+i*src_stride);
		int j;
		for(j=0;j<width;j++)
		{
			unsigned short t = *s++;
			int r = (t&0xf800)>>11;
			int g = (t&0x07e0)>>5;
			int b =  t&0x001f;
			if(r > max_r)
				max_r = r;
			if(g > max_g)
				max_g = g;
			if(b > max_b)
				max_b = b;
			if(r < min_r)
				min_r = r;
			if(g < min_g)
				min_g = g;
			if(b < min_b)
				min_b = b;
		}
	}

	if(max_r-min_r)
		scale_r = (5L<<16)/(max_r-min_r);
	if(max_g-min_g)
		scale_g = (10L<<16)/(max_g-min_g);
	if(max_b-min_b)
		scale_b = (5L<<16)/(max_b-min_b);

	for(i=0;i<height;i++)
	{
		unsigned short *s = (unsigned short*)(src+i*src_stride);
		unsigned char *d = dst+i*dst_stride;
		int j;
		for(j=0;j<width;j++)
		{
			unsigned short t = *s++;
			int r = (t&0xf800)>>11;
			int g = (t&0x07e0)>>5;
			int b =  t&0x001f;
			int idx_r, idx_g, idx_b;
			int *p;

			idx_r = ((r-min_r)*scale_r)>>16;
			idx_g = ((g-min_g)*scale_g)>>16;
			idx_b = ((b-min_b)*scale_b)>>16;

			p = tmp_tbl[idx_r][idx_g][idx_b];
			p[0] += r-min_r;
			p[1] += g-min_g;
			p[2] += b-min_b;
			p[3]++;
			*d++ = (unsigned char)(idx_r*10*5+idx_g*5+idx_b);
		}
	}

	for(i=0;i<5;i++)
	{
		int j;
		for(j=0;j<10;j++)
		{
			int k;
			for(k=0;k<5;k++)
			{
				int r, g, b;
				int *p = tmp_tbl[i][j][k];
				if(p[3])
				{
					r = min_r+((p[0]+p[3]/2)/p[3]);
					g = min_g+((p[1]+p[3]/2)/p[3]);
					b = min_b+((p[2]+p[3]/2)/p[3]);
					*idx = (unsigned short)((r<<11)|(g<<5)|b);
				}
				idx++;
			}
		}
	}

}
void bmp_to_osd_8bit(DE_config *conf)
{
	FILE* fp;
	int bmp_width,bmp_height,bmp_offset;
	u8 *rin,*gin,*bin;
	int i,j,k;
	int alpha;
	u8* buffer;
	DE_OSD_input* osd_input[2]={&(conf->osd_input1),&(conf->osd_input2)};
	DE_OSD_output* osd_output[2]={&(conf->osd_output1),&(conf->osd_output2)};
	struct mem_dev mem ;
	int fmt=0;
	int width=0,height=0;
	int fd;
	int ret;
	
	for(k=0;k<2;k++)
	{
		if(osd_input[k]->enable==1)
		{
			if(osd_input[k]->pix_fmt==DE_PIX_FMT_OSDBIT8MODE)
			{
				width=osd_input[k]->stride;
				height=osd_output[k]->height;
				fmt=osd_input[k]->pix_fmt;
				//buffer=malloc(width*height*4);
				buffer=(u8*)(osd_input[k]->img);
				fp=fopen("./input/osd_8bit.bmp","rb");
				if(fp==NULL)
					printf("fopen error\n");
				fseek(fp,0xa,0);
				fread(&bmp_offset,4,1,fp);
				fseek(fp,0x12,0);
				fread(&bmp_width,4,1,fp);
				fread(&bmp_height,4,1,fp);
				if(bmp_height<0)
					bmp_height=-bmp_height;
				printf("bmp_width=%d\n",bmp_width);
				printf("bmp_height=%d\n",bmp_height);
				rin=(u8*)malloc(bmp_width*bmp_height);
				if(rin==NULL)
				{
					printf("malloc failed\n");
					return;
				}		
				gin=(u8*)malloc(bmp_width*bmp_height);
				if(gin==NULL)
				{
					printf("malloc failed\n");
					return;
				}	
				bin=(u8*)malloc(bmp_width*bmp_height);
				if(bin==NULL)
				{
					printf("malloc failed\n");
					return;
				}	
				printf("malloc success\n");	
				fseek(fp,bmp_offset,0);
				for(i=0;i<bmp_width*bmp_height;i++)
				{

					*(bin+i)=fgetc(fp);
					*(gin+i)=fgetc(fp);
					*(rin+i)=fgetc(fp);
					alpha=fgetc(fp);

				}
				printf("read file success \n");	
				fclose(fp);

	
				for(i=0;i<height;i++)
				{
					for(j=0;j<width;j++)
					{
						unsigned int b,g,r,data;
						b=*(bin+i*bmp_width+j)/8;
						g=*(gin+i*bmp_width+j)/4;
						r=*(rin+i*bmp_width+j)/8;
						data=b|(g<<5)|(r<<11);
						*(unsigned short*)(buffer+(i*width+j)*2)=data;
					}
				}
				printf("torgb565 ok\n");
//				void rgb565_to_idx8_2(unsigned char *src,
//					int src_stride,
//					int width,
//					int height,
//					unsigned char *dst,
//					int dst_stride,
//					unsigned short *idx)
				rgb565_to_idx8_2((unsigned char *)buffer, width*2, width, height, (unsigned char*)(osd_input[k]->img), width,(unsigned short*)(osd_input[k]->index));
				
				printf("OSDBIT8MODE\n");

				free(rin);
				free(gin);
				free(bin);
					
			}
		}
	}

}

void bmp_to_osd(DE_config *conf)
{
	bmp_to_osd_16bit(conf);
	bmp_to_osd_8bit(conf);
}

void bmp_to_buffer(DE_config *conf)
{
	if(conf->input.enable==1)
		bmp_to_image(conf);
	bmp_to_osd(conf);
}
void set_para(DE_config* conf,char* fname)
{
	FILE *fp;
	char para[128] ;

	fp=fopen(fname,"r");
	if(fp==NULL)
	{
		printf("%s can not open",fname);
		return;
	}
	while(fscanf(fp, "%s\n", para) > 0)
	{		
		if(strcmp(para,"dev_info[DE_OUT_DEV_ID_LCD].enable:")==0)
			fscanf(fp, "%d\n", &(conf->dev_info[DE_OUT_DEV_ID_LCD].enable));
		else if(strcmp(para,"dev_info[DE_OUT_DEV_ID_HDMI].enable:")==0)
			fscanf(fp, "%d\n", &(conf->dev_info[DE_OUT_DEV_ID_HDMI].enable));
		else if(strcmp(para,"dev_info[DE_OUT_DEV_ID_HDMI].mode:")==0)
			fscanf(fp, "%d\n", &(conf->dev_info[DE_OUT_DEV_ID_HDMI].mode));
		else if(strcmp(para,"input.enable:")==0)
			fscanf(fp, "%d\n", &(conf->input.enable));
		else if(strcmp(para,"input.default_color:")==0)
			fscanf(fp, "%d\n", &(conf->input.default_color));
		else if(strcmp(para,"input.width:")==0)
			fscanf(fp, "%d\n", &(conf->input.width));
		else if(strcmp(para,"input.height:")==0)
			fscanf(fp, "%d\n", &(conf->input.height));
		else if(strcmp(para,"input.pix_fmt:")==0)
			fscanf(fp, "%d\n", &(conf->input.pix_fmt));
		else if(strcmp(para,"input.video_range:")==0)
			fscanf(fp, "%d\n", &(conf->input.video_range));
		else if(strcmp(para,"input.img:")==0)
			fscanf(fp, "%d\n", &(conf->input.img));
		else if(strcmp(para,"input.img_uv:")==0)
			fscanf(fp, "%d\n", &(conf->input.img_uv));
		else if(strcmp(para,"input.bus_addr:")==0)
			fscanf(fp, "%d\n", &(conf->input.bus_addr));
		else if(strcmp(para,"input.bus_addr_uv:")==0)
			fscanf(fp, "%d\n", &(conf->input.bus_addr_uv));
		else if(strcmp(para,"input.crop_x:")==0)
			fscanf(fp, "%d\n", &(conf->input.crop_x));
		else if(strcmp(para,"input.crop_y:")==0)
			fscanf(fp, "%d\n", &(conf->input.crop_y));
		else if(strcmp(para,"input.crop_width:")==0)
			fscanf(fp, "%d\n", &(conf->input.crop_width));
		else if(strcmp(para,"input.crop_height:")==0)
			fscanf(fp, "%d\n", &(conf->input.crop_height));
		else if(strcmp(para,"output.pip_x:")==0)
			fscanf(fp, "%d\n", &(conf->output.pip_x));
		else if(strcmp(para,"output.pip_y:")==0)
			fscanf(fp, "%d\n", &(conf->output.pip_y));
		else if(strcmp(para,"output.pip_width:")==0)
			fscanf(fp, "%d\n", &(conf->output.pip_width));
		else if(strcmp(para,"output.pip_height:")==0)
			fscanf(fp, "%d\n", &(conf->output.pip_height));
		else if(strcmp(para,"output.dar_width:")==0)
			fscanf(fp, "%d\n", &(conf->output.dar_width));
		else if(strcmp(para,"output.display_mode:")==0)
			fscanf(fp, "%d\n", &(conf->output.display_mode));
		else if(strcmp(para,"output.brightness:")==0)
			fscanf(fp, "%d\n", &(conf->output.brightness));
		else if(strcmp(para,"output.contrast:")==0)
			fscanf(fp, "%d\n", &(conf->output.contrast));
		else if(strcmp(para,"output.saturation:")==0)
			fscanf(fp, "%d\n", &(conf->output.saturation));
		else if(strcmp(para,"output.hue:")==0)
			fscanf(fp, "%d\n", &(conf->output.hue));
		else if(strcmp(para,"output.sharpness:")==0)
			fscanf(fp, "%d\n", &(conf->output.sharpness));
		else if(strcmp(para,"osd_input1.enable:")==0)
			fscanf(fp, "%d\n", &(conf->osd_input1.enable));
		else if(strcmp(para,"osd_input1.pix_fmt:")==0)
			fscanf(fp, "%d\n", &(conf->osd_input1.pix_fmt));
		else if(strcmp(para,"osd_input1.tparent_color:")==0)
			fscanf(fp, "0x%x\n", &(conf->osd_input1.tparent_color));
		else if(strcmp(para,"osd_input1.idx_fmt:")==0)
			fscanf(fp, "%d\n", &(conf->osd_input1.idx_fmt));
		else if(strcmp(para,"osd_input1.stride:")==0)
			fscanf(fp, "%d\n", &(conf->osd_input1.stride));
		else if(strcmp(para,"osd_output1.alpha:")==0)
			fscanf(fp, "%d\n", &(conf->osd_output1.alpha));
		else if(strcmp(para,"osd_output1.pip_x:")==0)
			fscanf(fp, "%d\n", &(conf->osd_output1.pip_x));
		else if(strcmp(para,"osd_output1.pip_y:")==0)
			fscanf(fp, "%d\n", &(conf->osd_output1.pip_y));
		else if(strcmp(para,"osd_output1.width:")==0)
			fscanf(fp, "%d\n", &(conf->osd_output1.width));
		else if(strcmp(para,"osd_output1.height:")==0)
			fscanf(fp, "%d\n", &(conf->osd_output1.height));
		else if(strcmp(para,"osd_input2.enable:")==0)
			fscanf(fp, "%d\n", &(conf->osd_input2.enable));
		else if(strcmp(para,"osd_input2.pix_fmt:")==0)
			fscanf(fp, "%d\n", &(conf->osd_input2.pix_fmt));
		else if(strcmp(para,"osd_input2.tparent_color:")==0)
			fscanf(fp, "0x%x\n", &(conf->osd_input2.tparent_color));
		else if(strcmp(para,"osd_input2.idx_fmt:")==0)
			fscanf(fp, "%d\n", &(conf->osd_input2.idx_fmt));
		else if(strcmp(para,"osd_input2.stride:")==0)
			fscanf(fp, "%d\n", &(conf->osd_input2.stride));
		else if(strcmp(para,"osd_output2.alpha:")==0)
			fscanf(fp, "%d\n", &(conf->osd_output2.alpha));
		else if(strcmp(para,"osd_output2.pip_x:")==0)
			fscanf(fp, "%d\n", &(conf->osd_output2.pip_x));
		else if(strcmp(para,"osd_output2.pip_y:")==0)
			fscanf(fp, "%d\n", &(conf->osd_output2.pip_y));
		else if(strcmp(para,"osd_output2.width:")==0)
			fscanf(fp, "%d\n", &(conf->osd_output2.width));
		else if(strcmp(para,"osd_output2.height:")==0)
			fscanf(fp, "%d\n", &(conf->osd_output2.height));
		else if(strcmp(para,"output.gamma_enable:")==0)
			fscanf(fp, "%d\n", &(conf->output.gamma_enable));
		else if(strcmp(para,"output.gamma:")==0)
		{
			int i;
			for(i=0;i<MAX_GAMMA_NUM;i++)
				fscanf(fp, "0x%x\n", &(conf->output.gamma[i]));
		}
	}
	fclose(fp);
}
void set_reg_mask(int* mask,char* fname)
{
	FILE* fp;
	char para[128] ;


	memset(mask,0,DE_REG_NUM*4);
	fp=fopen(fname,"r");

	while(fscanf(fp, "%s\n", para) > 0)
	{
		if(strcmp(para,"DE_REG_MASK")==0)
		{
			int ret;
			int i;
			for(i=0;i<DE_REG_NUM;i++)
			{
				ret=fscanf(fp, "0x%x\n", &(mask[i]));
				if(ret<=0)
					break;
			}
		}
	}
	fclose(fp);
}
	int  DeBaseadd;
	int  LcdBaseadd;
	int CmuBaseadd;
	int gpio_base_add;
	int DMA_base;
	int DRAM_base;
	
	void bdi_debug(void)
{
	printf("bdi_debug\n");
	RegBitSet(0x0,gpio_base_add+0x48,11,8);
	RegBitSet(0x0,gpio_base_add+0x48,15,12);
	RegBitSet(0x0,gpio_base_add+0x48,18,16);
	RegBitSet(0x0,gpio_base_add+0x4c,2,0);
	RegBitSet(0x0,gpio_base_add+0x4c,13,12);
	RegBitSet(0x0,gpio_base_add+0x54,30,28);		
	while(1);
}
void dma_copy(int add_src,int add_dst,int count,int channel,int burst)
{
	int data,i;
	
	act_writel(0,DMA_base+0x114+channel*0x20);
	act_writel(0,DMA_base);
	act_writel(0x10800090,DMA_base+0x100+channel*0x20);
	RegBitSet(burst,DMA_base+0x100+channel*0x20,31,29);
	RegBitSet(burst,DMA_base+0x100+channel*0x20,15,13);
	act_writel(add_src&0x1fffffff,DMA_base+0x104+channel*0x20);
	act_writel(add_dst&0x1fffffff,DMA_base+0x108+channel*0x20);
	act_writel(count,DMA_base+0x10c+channel*0x20);
	act_writel(1,DMA_base+0x114+channel*0x20);
	//while(act_readl(DMA_base+0x114+channel*0x20)==1);
	for(i=0;i<10;i++)
	{
		data=act_readl(DMA_base+0x100+channel*0x20+4*i);
		printf("/*0x%x*/0x%08x\n",0xb0060000+0x100+channel*0x20+4*i,data);
	}	
}
int dclk_test(int clk)
{

	int ret;
	
	RegBitSet(0xf,DeBaseadd+0xcc,7,4);
	RegBitSet(clk,CmuBaseadd+0x90,8,2);
	OSSleep(8000);//3s;
	if(RegBitRead(DeBaseadd+0xcc,7,4)!=0)
		ret= -1;
	else
		ret= 0;
	printf("clk:%d is %d\n",clk,ret);
	
	return ret;
}
void dclk_find(void)
{
	FILE* fp;
	int fd;
	int mclk,dclk;
	int hclk,cclk,hclk_div;
	int i,data;
	int start,end,mid;
	int add;
		
	fd = open("/dev/mem", O_RDWR | O_SYNC);	
	if(fd<0)
		printf("open/dev/mem error\n");
	DeBaseadd=(int)mmap(0, 0x200, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0x10040000);
	LcdBaseadd=(int)mmap(0, 0x200, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0x100f0000);
	CmuBaseadd=(int)mmap(0, 0x200, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0x10010000);
	gpio_base_add=(int)mmap(0, 0x200, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0x101c0000);
	DMA_base=(int)mmap(0, 0x200, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0x10060000);
	DRAM_base=(int)mmap(0, 0x200, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0x10070000);
	close(fd);	
	
	add=RegBitRead(DeBaseadd+0x10,27,0)&0xffffff00;
	dma_copy(add,add,0x1000,3,0);//0:single,3:burst 4,5:burst 8
	
	RegBitSet(0x1,CmuBaseadd+0x98,3,0);	
	start=20;//15M
	end=127;//95M	
	while(start<end-1)//end-start<=0.75M
	{
		mid=(start+end)/2;
		if(dclk_test(mid)==0)
			start=mid;
		else
			end=mid;			
	}
	dclk_test(end);
	
	mclk=RegBitRead(CmuBaseadd+0xb4,7,2)*8/2;
	cclk=RegBitRead(CmuBaseadd+0x0,7,2)*6;
	hclk_div=RegBitRead(CmuBaseadd+0xc,4,4)+1;
	hclk=cclk/hclk_div;
	dclk=RegBitRead(CmuBaseadd+0x90,8,2)*3/2/2;
	printf("MCLK:%dM\n",mclk);
	printf("HCLK:%dM\n",hclk);
	printf("MAX DCLK:%dM\n",dclk);	
	for(i=0;i<DE_REG_NUM;i++)
	{
		data=act_readl((int)DeBaseadd+4*i);
		printf("/*0x%x*/0x%08x\n",0xb0040000+4*i,data);
	};
	for(i=0;i<20;i++)
	{
		data=act_readl((int)DRAM_base+4*i);
		printf("/*0x%x*/0x%08x\n",0xb0070000+4*i,data);
	};
	
	dclk_test(start);
}
void reg_out(int* mask,char* fname)
{
	FILE* fp;
	int i;
	int data;
	int fd;


#if 0
	fp=fopen(fname,"wb");
	if(fp==NULL)
		printf("write file error\n");
	fprintf(fp,"de_reg_result\n");
	for(i=0;i<DE_REG_NUM;i++)
	{
		data=act_readl((int)DeBaseadd+4*i)&mask[i];
		fprintf(fp,"/*0x%x*/0x%08x\n",0xb0040000+4*i,data);
	}
	fclose(fp);

#else
	fd = open("/dev/mem", O_RDWR | O_SYNC);	
	if(fd<0)
		printf("open/dev/mem error\n");
	DeBaseadd=(int)mmap(0, 0x200, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0x10040000);
	LcdBaseadd=(int)mmap(0, 0x200, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0x100f0000);
	CmuBaseadd=(int)mmap(0, 0x200, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0x10010000);
	gpio_base_add=(int)mmap(0, 0x200, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0x101c0000);
	close(fd);
	

	for(i=0;i<DE_REG_NUM;i++)
	{
		data=act_readl((int)DeBaseadd+4*i);
		printf("/*0x%x*/0x%08x\n",0xb0040000+4*i,data);
	}
	for(i=0;i<50;i++)
	{
		data=act_readl((int)LcdBaseadd+4*i);
		printf("/*0x%x*/0x%08x\n",0xb00f0000+4*i,data);
	}	


#endif
	
}


void case_test(int start_no,int end_no)
{
	void* ds_inst;
	DE_config conf;
	char fname[128] ;
	int i,j;

	int de_reg_mask[DE_REG_NUM];	
	int fd;
	int ret;
	OS_BUF img,osd1,osd2;

	sprintf(fname,"./input/reg_mask.txt");
	//set_reg_mask(de_reg_mask,fname);
	//example
	
	//img.size=2048*1024*4;
	//OSBufMalloc(&img);
	conf.input.bus_addr=(int)img.phy_addr;
	conf.input.img=(unsigned long *)img.vir_addr;
	//conf.input.bus_addr_uv=(int)img.physic_address+width*height;
	//osd1.size=2048*1024*4;
	osd1.size=800*600*2;
	OSBufMalloc(&osd1);
	conf.osd_input1.bus_addr=(int)osd1.phy_addr;
	conf.osd_input1.img=(unsigned long *)osd1.vir_addr;
	//osd2.size=2048*1024*4;
	//OSBufMalloc(&osd2);
	conf.osd_input2.bus_addr=(int)osd2.phy_addr;
	conf.osd_input2.img=(unsigned long *)osd2.vir_addr;
	
	for(i=start_no;i<=end_no;i++)
	{		
		de_init(&ds_inst);
		//get config
		//de_get_config(ds_inst,&conf,DE_CFG_IN);
		//de_get_config(ds_inst,&conf,DE_CFG_OSD1);
		//de_get_config(ds_inst,&conf,DE_CFG_OSD2);
		//your config
		sprintf(fname,"./input/input_%d.txt",i);
		set_para(&conf,fname);
		bmp_to_buffer(&conf);

		if(conf.input.enable!=2)
			de_set_Config(ds_inst,&conf,DE_CFG_IN);
		de_set_Config(ds_inst,&conf,DE_CFG_OSD1);
		de_set_Config(ds_inst,&conf,DE_CFG_OSD2);
		conf.cursor.enable=1;
		conf.cursor.x=10;
		conf.cursor.y=10;
		de_set_Config(ds_inst,&conf,DE_CFG_CURSOR);
		for(j=0;j<0x10000;j++);
		//sprintf(fname,"./output/output_%d.txt",i);
		reg_out(de_reg_mask,fname);
		//dclk_find();
		de_release(ds_inst);
		printf("case %d OK \n",i);
		
	}
	
	OSBufFree(&img);
	OSBufFree(&osd1);
	OSBufFree(&osd2);

}

int main(int argc,char **argv)
{
	#if 1
	int start,end;

	start = atoi(argv[1]);
	end=atoi(argv[2]);
	
	printf("start=%d\n",start);
	printf("end=%d\n",end);
	//am7x_lcm_get_config();

	case_test(start,end);
	#else
	dclk_find();
	#endif
	
	return 0;
}

