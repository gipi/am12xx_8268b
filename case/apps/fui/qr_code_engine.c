
#ifdef MODULE_CONFIG_QRCODE_GENERATOR
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "swf_ext.h"
#include "swf_types.h"
#include "am7x_qrcode_api.h"

struct qrc_code *qrcoder=NULL;

void __qrcode_gen_close()
{
	if(qrcoder)
	{
		qrc_close(qrcoder);
		qrcoder = NULL;
	}
}

int __qrcode_gen_open()
{
	__qrcode_gen_close();
	
	qrcoder = qrc_open();
	if(qrcoder == NULL)
	{
		printf("open QR encoder error\n");
		return -1;
	}

	return 0;
}

static int qrcode_engine_open(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);

	int ret = __qrcode_gen_open();
	Swfext_PutNumber((ret==0)?1:0);

	SWFEXT_FUNC_END();	
}

static int qrcode_engine_close(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);

	__qrcode_gen_close();

	SWFEXT_FUNC_END();	
}

int __qrcode_gen_encode(const unsigned char *str)
{
	int err = -1;
	
	if(qrcoder)
	{
		err = qrc_encode(qrcoder, str);
		if(err == -1){
			printf("QR encode error\n");
		}
	}

	return err;
}

static int qrcode_engine_encode(void * handle)
{
	int err;
	char *str;
	
	SWFEXT_FUNC_BEGIN(handle);

	str = Swfext_GetString();
	err = __qrcode_gen_encode((unsigned char *)str);
	if(err == -1){
		printf("QR encode error\n");
		Swfext_PutNumber(0);
	}
	else{
		Swfext_PutNumber(1);
	}
	
	SWFEXT_FUNC_END();	
}

int __qrcode_gen_get_width()
{
	if(qrcoder)
	{
		return qrc_get_width(qrcoder);
	}

	return 0;
}

static int qrcode_engine_get_width(void * handle)
{
	int width;
	
	SWFEXT_FUNC_BEGIN(handle);

	width = __qrcode_gen_get_width();
	Swfext_PutNumber(width);
	
	SWFEXT_FUNC_END();	
}

int __qrcode_gen_get_value_by_pos(int x, int y)
{
	if(qrcoder)
	{
		return qrc_get_code_at_pos(qrcoder, x, y);
	}

	return 0;
}

static int qrcode_engine_get_value_by_pos(void * handle)
{
	int x,y;
	int value;
	
	SWFEXT_FUNC_BEGIN(handle);

	x = Swfext_GetNumber();
	y = Swfext_GetNumber();

	value = __qrcode_gen_get_value_by_pos(x,y);
	Swfext_PutNumber(value);
	
	SWFEXT_FUNC_END();	
}

int __qrcode_gen_bitmap(char *file)
{
	int w,i,j,k,m;
	unsigned char *buf,*ptr;
	int black,margin=4,pixsize=4,err;
	int filesize=0;
	unsigned int *pInt;
	
	if(file == NULL){
		return 0;
	}

	w = qrc_get_width(qrcoder);
	if(w <= 0){
		printf("QR CODE w=%d error\n",w);
		return 0;
	}

	buf = (unsigned char *)malloc((w+2*margin)*(w+2*margin)*2*pixsize*pixsize);
	if(buf == NULL){
		printf("QR CODE generate error:not enough memory\n");
		return 0;
	}

	pInt = (unsigned int *)buf;
	for(i=0;i<(w+2*margin)*(w+2*margin)*2*pixsize*pixsize/4;i++){
		*(pInt + i) = 0x80EB80EB;
	}

	for(i=0;i<w;i++){
		for(j=0;j<w;j++){
			black = qrc_get_code_at_pos(qrcoder,j,i);
			if(black==1){
				ptr = buf + (i+margin)*pixsize*(w+2*margin)*pixsize*2 + (j+margin)*pixsize*2; 
				for(k=0;k<pixsize;k++){
					pInt = (unsigned int *)ptr;
					for(m=0;m<pixsize/2;m++){
						*(pInt + m) = 0x80008000;
					}
					ptr += (w + 2*margin)*pixsize*2;
				}
				
			}
			
		}
	}

	err = jpeg_encode((void *)file,(void*)buf,NULL,NULL,(w+2*margin)*pixsize*2,4,0,0,(w+2*margin)*pixsize,(w+2*margin)*pixsize,3,&filesize,1);
	if(err != 0){
		printf("QR CODE generate error:encode error\n");
		err = 0;
	}
	else{
		err = 1;
	}

	free(buf);

	return err;
}

static int qrcode_engine_generate_bitmap(void * handle)
{
	int value;
	char *file;
	
	SWFEXT_FUNC_BEGIN(handle);

	file = Swfext_GetString();
	value = __qrcode_gen_bitmap(file);
	Swfext_PutNumber(value);
	
	SWFEXT_FUNC_END();	
}

static int __qrcode_test()
{
	qrcoder = qrc_open();
	if(qrcoder == NULL){
		printf("open QR encoder error\n");
		return -1;
	}

	qrc_encode(qrcoder,(unsigned char *)"actions-micro:qrcode test");
	__qrcode_gen_bitmap("/mnt/udisk/qr1.jpg");

	return 0;
}

int swfext_qrcode_register(void)
{
	SWFEXT_REGISTER("qrcode_open", qrcode_engine_open);
	SWFEXT_REGISTER("qrcode_close", qrcode_engine_close);
	SWFEXT_REGISTER("qrcode_encode", qrcode_engine_encode);
	SWFEXT_REGISTER("qrcode_get_width", qrcode_engine_get_width);
	SWFEXT_REGISTER("qrcode_get_value_by_pos", qrcode_engine_get_value_by_pos);
	SWFEXT_REGISTER("qrcode_generate_bitmap", qrcode_engine_generate_bitmap);

	//__qrcode_test();
	
	return 0;
}

#endif /** MODULE_CONFIG_QRCODE_GENERATOR */

