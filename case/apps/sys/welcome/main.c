/**
@file: main.c

@abstract: main entry source file.

@notice: Copyright (c), 2010-2015 Actions-Mirco Communications, Inc.
 *
 *  This program is develop for Actions-Mirco Boot Display;
 *  include code recover, upgrade, welcome logo	 
 *
 *  The initial developer of the original code is scopengl
 *
 *  scopengl@gmail.com
 *
 */
 
#include "boot_display_api.h"
#include "syscfg.h"
#include "welcome.h"
#include "../../../include/ezcast_public.h"
#ifndef __ACT_MODULE_CONFIG_H__ //For LinuxTool bug!! (i.e.; #if !define xxx)
#define __ACT_MODULE_CONFIG_H__
#include "../../../../scripts/mconfig.h"
#endif

/** FIXME syscfg.h config the am7x_chipinfo extern defination  **/
struct	am7x_chipinfo am7x_chipinfo;

/** global data for store boot display about param **/
static struct welcome_global_data global_data ={
	.default_brightness= 21,
	.lcm_init_flag = 0,
};

int buffer_width = 0;
int buffer_height = 0; 

char *background_file[] = {
	"WELCOME BIN",
	"WELCOME BIN",
	"WELCOME BIN",
	"WELCOME BIN",
	"COVER   BIN",
	"FAILED  BIN",
	"SUCCESS BIN",
};

int init_hardware(void)
{
	int ret = 0;

 	boot_mfp_config((void*)(global_data.platform_data_addr+CONFIG_DATA_OFFSET));
	#if (EZMUSIC_ENABLE&&GPIO_POWER_ON_LED)
	boot_set_gpio(GPIO_POWER_ON_LED,0);//PWR-ON-LED   
	#endif

	ret = boot_lcm_init((void *)(global_data.lcm_config_addr+CONFIG_DATA_OFFSET),(void*)(global_data.lcm_color_addr+CONFIG_DATA_OFFSET));
	if(ret!=0){
		printf("lcm init error\n");
		goto end;
	}

	ret = boot_backlight_init((void*)(global_data.backlight_config_addr+CONFIG_DATA_OFFSET));
	if(ret!=0){
		printf("backlight init error\n");
		goto end;
	}
end:
	return ret;
}

/** NOTICE : only process yuv422**/
void process_buffer(unsigned int base,unsigned int line_len,unsigned int height_total,unsigned int color,unsigned int progress)
{
	unsigned int* p = (unsigned int*)base ;// 
	int i = 0,j=0;
	int fill_unit_per_line = (line_len*progress)/(2*100); //yuv422

	for(i=0;i<height_total;i++)
	{
		for(j=0;j<fill_unit_per_line;j++) //fill two pixel one time when yuv422
		{
			*(p+i*line_len/2+j) = color;  	
		}
	}
	
}

void dynamic_pic_process_buffer(unsigned int fwstatus)
{
	struct boot_device_info device_info;
	Fwu_status_t  *status = (Fwu_status_t*)fwstatus;
	 // YUV422
	int progress_bar_height = 40;
	int progress_color = 0x00; //YUV422 
	int error_color = 0xffff; //YUV422 
	unsigned int base = global_data.pic_data_addr+16;

	int color = 0,prg =0;
	
//	boot_get_device_info((void*)&device_info);  //NOTICE must call this func after hardware init
	device_info.buffer_height = buffer_height;
	device_info.buffer_width = buffer_width;
	

	base = base+(device_info.buffer_width*2*(device_info.buffer_height/2 - progress_bar_height/2)); 

	switch(status->state)
	{
		case S_RUNNING:
			color  = progress_color;
			prg = status->prg;
			break;
		case S_ERROR:
			color = error_color;
			prg = 100;
			break;
		default:
			break;
	}

	process_buffer(base,device_info.buffer_width,progress_bar_height,color,prg);


}

int __attribute__((section(".entry")))display_boot_pic(unsigned int flash_read_fun_addr,unsigned int chip_info_addr,unsigned int show_type,unsigned int fwstatus)
{
	int ret = 0 ;

	unsigned int background_pic_name_addr =0;
	int a = 0;

	struct boot_device_info device_info; 
	

	serial_init();
	if(global_data.lcm_init_flag == 1&&show_type == SHOW_TYPE_UPGRAGD )
	{
		dynamic_pic_process_buffer((unsigned int)fwstatus);
		return 0;
	}

//	dynamic mode /static mode 
	switch(show_type)
	{
		case SHOW_TYPE_RECOVER:
			background_pic_name_addr = (unsigned int)background_file[0];
			break;
		case SHOW_TYPE_CARD:
			background_pic_name_addr = (unsigned int)background_file[1];
			break;
		case SHOW_TYPE_UPGRAGD:
			background_pic_name_addr = (unsigned int)background_file[2];
			break;	
		case SHOW_TYPE_WELCOME:
			background_pic_name_addr = (unsigned int)background_file[3];
			break;
		case SHOW_TYPE_COVER:
			background_pic_name_addr = (unsigned int)background_file[4];
			break;
		case SHOW_TYPE_FAILED:
			background_pic_name_addr = (unsigned int)background_file[5];
			break;
		case SHOW_TYPE_SUCCESSFUL:
			background_pic_name_addr = (unsigned int)background_file[6];
			break;	
		default:
			break;
	}
	
	init_global_buffer();
	
	load_resource(flash_read_fun_addr,background_pic_name_addr);

	{
			
		buffer_width = device_info.buffer_width = *(unsigned short*)(global_data.pic_data_addr+8);

		buffer_height = device_info.buffer_height =*(unsigned short*)(global_data.pic_data_addr+10) ;

		printf("%d %d\n",device_info.buffer_width,device_info.buffer_height);
		
		device_info.color_space = 5;//PIX_FMT_YCBCR_4_2_2_SEMIPLANAR
	}
	boot_set_device_info(&device_info);

	//after open backlight ,the flag will be set to true
	{
		ret = init_hardware(); // serial init  ,after this printf effect
		if(ret !=0)
		{
			printf("init hard failed");
			goto end;
		}
		
	}

	switch(show_type)
	{
		case SHOW_TYPE_RECOVER:	
		case SHOW_TYPE_CARD:
		case SHOW_TYPE_WELCOME:  
		case SHOW_TYPE_COVER:
		case SHOW_TYPE_FAILED:
		case SHOW_TYPE_SUCCESSFUL: //need no extra process ,just static picture 
			break;
		case SHOW_TYPE_UPGRAGD:  //dynamic pic ,need process the buffer
			dynamic_pic_process_buffer((unsigned int)fwstatus);
			break;	
		default:
			break;
	}
	
	printf("global_data.pic_data_addr:%x\n",global_data.pic_data_addr);

	ret = boot_logo_display((void*)(global_data.pic_data_addr+16)); //skip first 16 byte head
	if(ret!=0)
	{
		printf("logo display error\n");
		goto end;
	}

	{
		ret = boot_backlight_open(global_data.default_brightness);
		if(ret!=0){
			printf("backlight open error\n");
			goto end;
		}
	}
	global_data.lcm_init_flag = 1;
end:
	printf("exit display boot pic\n");
	
	return ret;
}

void init_global_buffer(void)
{
	int ddr_cap;

	ddr_cap = boot_get_ddr_cap();

	

	if(ddr_cap == DDR_CAP_64M){
		global_data.lcm_config_addr = 0xa0000000+32*1024*1024;
		printf("---- bootdisplay test ddr cap 64M\n");
	}
	else if(ddr_cap == DDR_CAP_128M){
		global_data.lcm_config_addr = 0xa0000000+80*1024*1024;
		printf("---- bootdisplay test ddr cap 128M\n");
	}
	else{
		global_data.lcm_config_addr = 0xa0000000+32*1024*1024;
		printf("---- bootdisplay test ddr cap 64M\n");
	}
	global_data.lcm_color_addr = global_data.lcm_config_addr + MEMGAP_SIZE;
	global_data.backlight_config_addr =global_data.lcm_color_addr + MEMGAP_SIZE;
	global_data.platform_data_addr = global_data.backlight_config_addr + MEMGAP_SIZE;
	global_data.pic_data_addr = global_data.platform_data_addr + MEMGAP_SIZE;
}

#ifdef PIC_SIMULATE
void simulate_boot_pic(unsigned char* data)
{
	int i=0,size = 800*600/2;
	unsigned int* p = (unsigned int*)data;
	while(i++<size)
	{
		*(p+i) = 0x80008000;
	}
	
}
#endif



void load_resource(unsigned int fentry,unsigned int file_name)
{
	unsigned char* temp_buf = (unsigned char*)(global_data.pic_data_addr +5*1024*1024); 
	unsigned char *filename = (unsigned char*)file_name;	
	unsigned char* data = 0;
//	printf("load lcm\n");
	//load lcm config 
	data = (unsigned char*)global_data.lcm_config_addr ;
	boot_read_data((unsigned char*)LCM_DATA_FILE,(void*)fentry,data,temp_buf);	

//   printf("load gamma file\n");
	// load color config file
	data = (unsigned char*)global_data.lcm_color_addr ;
	boot_read_data((unsigned char*)COLOR_DATA_FILE,(void*)fentry,data,temp_buf);	

//	printf("load backlight\n");
	//load backlight config
	data = (unsigned char*)global_data.backlight_config_addr;
	boot_read_data((unsigned char*)BACKLIHT_DATA_FILE,(void*)fentry,data,temp_buf);

//	printf("load gpio\n");
	//load mfp config
	data = (unsigned char*)global_data.platform_data_addr ;
	boot_read_data((unsigned char*)PLATFORM_DATA_FILE,(void*)fentry,data,temp_buf);


	
	//load pic data 
#ifdef PIC_SIMULATE
	data = (unsigned char*)global_data.pic_data_addr;
	simulate_boot_pic(data);
#else
//	printf("load pic data\n");
	data = (unsigned char*)global_data.pic_data_addr;
	boot_read_data(filename,(void*)fentry,data,temp_buf);
#endif


}




