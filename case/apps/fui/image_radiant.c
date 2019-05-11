#include "image_decode.h"

typedef struct img_radiant_pixel_s
{
	unsigned char y;
	unsigned char cb;
	unsigned char cr;
	int x_coor;
	int y_coor;
	int y_add;
	int c_add;
	int stride;
	int mode;
}img_radiant_pixel_t;

typedef struct img_radiant_roi_s
{
	int mode;//0:YUV422;1:YUV420
	int add;//image address
	int stride;//image stride; stride%2=0
	int width;//image width;width%2==0
	int height;//image height
	int xRoi;//roi x; xRoi%4==0
	int yRoi;//roi y;yRoi%2==0
	int wRoi;//roi width;wRoi%4==0
	int hRoi;//roi height;hRoi%2==0
}img_radiant_roi_t;

//extern INT8U radiant_color_en;
unsigned char  radiant_color_en=1;

void DemuxYuv422(unsigned short int* val,unsigned char type,unsigned char* a,unsigned char* y,unsigned char* u,unsigned char* v)
{
	*a=0;
	*y=(*val)&0xff;
	if(type==0)
	{
		*u=(*val>>8)&0xff;
		*v=(*(val+1)>>8)&0xff;
	}
	else
	{
		*u=(*(val-1)>>8)&0xff;
		*v=(*val>>8)&0xff;
	}
};

void pixelRead(img_radiant_pixel_t* pixel)
{
	if(pixel->mode==0)
	{
		unsigned short int* val;
		unsigned char a;

		val=(unsigned short int*)(pixel->y_add+(pixel->y_coor*pixel->stride+pixel->x_coor)*2);
		DemuxYuv422(val,pixel->x_coor%2,&a,(unsigned char*)(&(pixel->y)),(unsigned char*)(&(pixel->cb)),(unsigned char*)(&(pixel->cr)));
	}else if(pixel->mode==1)
	{
		int x,y;
		pixel->y=*(unsigned char*)(pixel->y_add+pixel->y_coor*pixel->stride+pixel->x_coor);
		x=pixel->x_coor/2*2;
		y=pixel->y_coor/2*2;
		pixel->cb=*(unsigned char*)(pixel->c_add+y*pixel->stride/2+x);
		pixel->cr=*(unsigned char*)(pixel->c_add+1+y*pixel->stride/2+x);
	}

}

void pixelWrite(img_radiant_pixel_t* pixel)
{
	if(pixel->mode==0)
	{
		*(unsigned char*)(pixel->y_add+pixel->y_coor*pixel->stride*2+pixel->x_coor*2)=pixel->y;
		if(pixel->x_coor%2==0)
		{
			*(unsigned char*)(pixel->y_add+1+pixel->y_coor*pixel->stride*2+pixel->x_coor*2)=pixel->cb;
		}else
		{
			*(unsigned char*)(pixel->y_add+1+pixel->y_coor*pixel->stride*2+pixel->x_coor*2)=pixel->cr;
		}
	}else if(pixel->mode==1)
	{
		*(unsigned char*)(pixel->y_add+pixel->y_coor*pixel->stride+pixel->x_coor)=pixel->y;
		if((pixel->x_coor%2==0)&&(pixel->y_coor%2==0))
		{
			*(unsigned char*)(pixel->c_add+pixel->y_coor*pixel->stride/2+pixel->x_coor)=pixel->cb;
			*(unsigned char*)(pixel->c_add+1+pixel->y_coor*pixel->stride/2+pixel->x_coor)=pixel->cr;
		}
		
	}
}

void linearFill(unsigned char* addIni,unsigned char valIni,unsigned char valEnd,int addStep,int length)
{
	int i;
	int val;

	for(i=0;i<length;i++)
	{
		val=(valEnd-valIni)*i/length+valIni;
		*(addIni+i*addStep)=val;
	}
}

void pixelLineFill(img_radiant_pixel_t* pixelIni,img_radiant_pixel_t* pixelEnd,int type)
{
	int pos,pos_start,pos_end;
	img_radiant_pixel_t pixel;
	int y_total,cb_total,cr_total;
	int length=0;

	pixel=*pixelIni;
	y_total=pixelEnd->y-pixelIni->y;
	cb_total=pixelEnd->cb-pixelIni->cb;
	cr_total=pixelEnd->cr-pixelIni->cr;
	switch(type)
	{
		case 0://hor
			length=pixelEnd->x_coor-pixelIni->x_coor;
			pos_start=pixelIni->x_coor;
			pos_end=pixelEnd->x_coor;
			break;
		case 1://ver
			length=pixelEnd->y_coor-pixelIni->y_coor;
			pos_start=pixelIni->y_coor;
			pos_end=pixelEnd->y_coor;
			break;
	}
	if(length==0)
		pixelWrite(&pixel);
	else
	{
		for(pos=pos_start;pos<=pos_end;pos++)
		{
			if(type==0)
				pixel.x_coor=pos;
			else
				pixel.y_coor=pos;
			pixel.y=y_total*(pos-pos_start)/length+pixelIni->y;
			pixel.cb=cb_total*(pos-pos_start)/length+pixelIni->cb;
			pixel.cr=cr_total*(pos-pos_start)/length+pixelIni->cr;
			pixelWrite(&pixel);
		}
	}

}

void cordLimit(int* in,int min,int max)
{
	if(*in<min)
		*in=min;
	else if(*in>max)
		*in=max;
}

void cornorFill_L_U(img_radiant_roi_t* roi)
{
	int x,y;
	int xstart,ystart,xend,yend;
	int xborder,yborder;
	img_radiant_pixel_t pixelIni;
	img_radiant_pixel_t pixelEnd;
	int stride;
	int type;
	int error=2;
	img_radiant_pixel_t pixel;
  
	stride=roi->stride;
	xstart=0;
	ystart=0;
	xend=roi->xRoi-1;
	yend=roi->yRoi-1;
	pixel.y_add=roi->add;
	pixel.mode=roi->mode;
	pixel.stride=roi->stride;
	if(pixel.mode==1)
		pixel.c_add=pixel.y_add+pixel.stride*roi->height;

	for(y=ystart;y<=yend;y++)
	{
		xborder=(xend-xstart)*(y-ystart)/(yend-ystart)+xstart-error;
		cordLimit(&xborder,xstart,xend);
		pixel.x_coor=xend+1;
		pixel.y_coor=y;
		pixelRead(&pixel);
		pixelIni=pixel;
		pixelIni.y=pixelIni.y/2;
		pixelIni.x_coor=xborder;
		pixelIni.y_coor=y;
		pixelEnd=pixel;
		pixelEnd.x_coor=xend;
		pixelEnd.y_coor=y;
		type=0;
		pixelLineFill(&pixelIni,&pixelEnd,type);
	}
	for(x=xstart;x<=xend;x++)
	{
		yborder=(yend-ystart)*(x-xstart)/(xend-xstart)+ystart-error;
		cordLimit(&yborder,ystart,yend);
		pixel.x_coor=x;
		pixel.y_coor=yend+1;
		pixelRead(&pixel);
		pixelIni=pixel;
		pixelIni.y=pixelIni.y/2;
		pixelIni.x_coor=x;
		pixelIni.y_coor=yborder;
		pixelEnd=pixel;
		pixelEnd.x_coor=x;
		pixelEnd.y_coor=yend;
		type=1;
		pixelLineFill(&pixelIni,&pixelEnd,type);
	}
}

void cornorFill_R_U(img_radiant_roi_t* roi)
{

	int x,y;
	int xstart,ystart,xend,yend;
	int xborder,yborder;
	img_radiant_pixel_t pixelIni;
	img_radiant_pixel_t pixelEnd;
	int stride;
	int type;
	int error=2;
	img_radiant_pixel_t pixel;
  
	stride=roi->stride;
	xstart=roi->xRoi+roi->wRoi;
	ystart=0;
	xend=roi->width-1;
	yend=roi->yRoi-1;
	pixel.y_add=roi->add;
	pixel.mode=roi->mode;
	pixel.stride=roi->stride;
	if(pixel.mode==1)
		pixel.c_add=pixel.y_add+pixel.stride*roi->height;

	for(y=ystart;y<=yend;y++)
	{
		xborder=(xend-xstart)*(y-yend)/(ystart-yend)+xstart+error;
		cordLimit(&xborder,xstart,xend);
		pixel.x_coor=xstart-1;
		pixel.y_coor=y;
		pixelRead(&pixel);
		pixelIni=pixel;
		pixelIni.x_coor=xstart;
		pixelIni.y_coor=y;
		pixelEnd=pixel;
		pixelEnd.y=pixel.y/2;
		pixelEnd.x_coor=xborder;
		pixelEnd.y_coor=y;
		type=0;
		pixelLineFill(&pixelIni,&pixelEnd,type);
	}
	for(x=xstart;x<=xend;x++)
	{
		yborder=(ystart-yend)*(x-xstart)/(xend-xstart)+yend-error;
		cordLimit(&yborder,ystart,yend);
		pixel.x_coor=x;
		pixel.y_coor=yend+1;
		pixelRead(&pixel);
		pixelIni=pixel;
		pixelIni.y=pixelIni.y/2;
		pixelIni.x_coor=x;
		pixelIni.y_coor=yborder;
		pixelEnd=pixel;
		pixelEnd.x_coor=x;
		pixelEnd.y_coor=yend;
		type=1;
		pixelLineFill(&pixelIni,&pixelEnd,type);
	}
}


void cornorFill_R_D(img_radiant_roi_t* roi)
{

	int x,y;
	int xstart,ystart,xend,yend;
	int xborder,yborder;
	img_radiant_pixel_t pixelIni;
	img_radiant_pixel_t pixelEnd;
	int stride;
	int type;
	int error=2;
	img_radiant_pixel_t pixel;
  
	stride=roi->stride;
	xstart=roi->xRoi+roi->wRoi;
	ystart=roi->yRoi+roi->hRoi;
	xend=roi->width-1;
	yend=roi->height-1;
	pixel.y_add=roi->add;
	pixel.mode=roi->mode;
	pixel.stride=roi->stride;
	if(pixel.mode==1)
		pixel.c_add=pixel.y_add+pixel.stride*roi->height;

	for(y=ystart;y<=yend;y++)
	{
		xborder=(xend-xstart)*(y-ystart)/(yend-ystart)+xstart+error;
		cordLimit(&xborder,xstart,xend);
		pixel.x_coor=xstart-1;
		pixel.y_coor=y;
		pixelRead(&pixel);
		pixelIni=pixel;
		pixelIni.x_coor=xstart;
		pixelIni.y_coor=y;
		pixelEnd=pixel;
		pixelEnd.y=pixel.y/2;
		pixelEnd.x_coor=xborder;
		pixelEnd.y_coor=y;
		type=0;
		pixelLineFill(&pixelIni,&pixelEnd,type);
	}
	for(x=xstart;x<=xend;x++)
	{
		yborder=(yend-ystart)*(x-xstart)/(xend-xstart)+ystart+error;
		cordLimit(&yborder,ystart,yend);
		pixel.x_coor=x;
		pixel.y_coor=ystart-1;
		pixelRead(&pixel);
		pixelIni=pixel;
		pixelIni.x_coor=x;
		pixelIni.y_coor=ystart;
		pixelEnd=pixel;
		pixelEnd.y=pixel.y/2;
		pixelEnd.x_coor=x;
		pixelEnd.y_coor=yborder;
		type=1;
		pixelLineFill(&pixelIni,&pixelEnd,type);
	}
}


void cornorFill_L_D(img_radiant_roi_t* roi)
{

	int x,y;
	int xstart,ystart,xend,yend;
	int xborder,yborder;
	img_radiant_pixel_t pixelIni;
	img_radiant_pixel_t pixelEnd;
	int stride;
	int type;
	int error=2;
	img_radiant_pixel_t pixel;
  
	stride=roi->stride;
	xstart=0;
	ystart=roi->yRoi+roi->hRoi;
	xend=roi->xRoi-1;
	yend=roi->height-1;
	pixel.y_add=roi->add;
	pixel.mode=roi->mode;
	pixel.stride=roi->stride;
	if(pixel.mode==1)
		pixel.c_add=pixel.y_add+pixel.stride*roi->height;

	for(y=ystart;y<=yend;y++)
	{
		xborder=(xend-xstart)*(y-yend)/(ystart-yend)+xstart-error;
		cordLimit(&xborder,xstart,xend);
		pixel.x_coor=xend+1;
		pixel.y_coor=y;
		pixelRead(&pixel);
		pixelIni=pixel;
		pixelIni.y=pixel.y/2;
		pixelIni.x_coor=xborder;
		pixelIni.y_coor=y;
		pixelEnd=pixel;
		pixelEnd.x_coor=xend;
		pixelEnd.y_coor=y;
		type=0;
		pixelLineFill(&pixelIni,&pixelEnd,type);
	}
	for(x=xstart;x<=xend;x++)
	{
		yborder=(ystart-yend)*(x-xstart)/(xend-xstart)+yend+error;
		cordLimit(&yborder,ystart,yend);
		pixel.x_coor=x;
		pixel.y_coor=ystart-1;
		pixelRead(&pixel);
		pixelIni=pixel;
		pixelIni.x_coor=x;
		pixelIni.y_coor=ystart;
		pixelEnd=pixel;
		pixelEnd.y=pixel.y/2;
		pixelEnd.x_coor=x;
		pixelEnd.y_coor=yborder;
		type=1;
		pixelLineFill(&pixelIni,&pixelEnd,type);
	}
}


void pixelSum(int* ysum,int* cbsum,int* crsum,int fator,img_radiant_pixel_t* pixel)
{
	*ysum=*ysum+(pixel->y)*fator;
	*cbsum=*cbsum+(pixel->cb)*fator;
	*crsum=*crsum+(pixel->cr)*fator;

}

void pixelAverage(img_radiant_pixel_t* pixel,int ysum,int cbsum,int crsum,int fatorsum)
{
	if(fatorsum!=0){
		pixel->y=ysum/fatorsum;
		pixel->cb=cbsum/fatorsum;
		pixel->cr=crsum/fatorsum;
	}
	else
		printf("[ERROR]%s,%d:Crazy fatorsum==0",__FILE__,__LINE__);
}

#define FILTER 12
struct _filter
{
	img_radiant_pixel_t fifo[FILTER];
	int num;
};

void pixelFilter(struct _filter* flt, img_radiant_pixel_t* in)
{
	int i;
	int y_total;
	int cb_total;
	int cr_total;

	flt->num++;
	if(flt->num>FILTER)
		flt->num=FILTER;
	for(i=flt->num-1;i>0;i--)
		(flt->fifo)[i]=(flt->fifo)[i-1];
	(flt->fifo)[0]=*in;
	y_total=0;
	cb_total=0;
	cr_total=0;
	for(i=0;i<flt->num;i++)
	{
		y_total=y_total+((flt->fifo)[i]).y;
		cb_total=cb_total+((flt->fifo)[i]).cb;
		cr_total=cr_total+((flt->fifo)[i]).cr;
	}
	in->y=y_total/(flt->num);
	in->cb=cb_total/(flt->num);
	in->cr=cr_total/(flt->num);
}


#define LINEBUFFER 800

  void frame(img_radiant_roi_t* roi)
  {
	//int y,cb,cr;
  	int ySumX=0,cbSumX=0,crSumX=0;
  	static int ySumY[LINEBUFFER];
  	static int cbSumY[LINEBUFFER];
  	static int crSumY[LINEBUFFER];
  	int xfator,yfator;
  	int fatorSumX=0,fatorSumY=0;
  	int x,y;
  	static  img_radiant_pixel_t pixelIni;
  	static  img_radiant_pixel_t pixelEnd;
  	int stride;
  	int type;
	static struct _filter XFilter0;
	static struct _filter XFilter1;
	static struct _filter YFilter;
	img_radiant_pixel_t pixel;

  	memset(ySumY,0,sizeof(int)*LINEBUFFER);
  	memset(cbSumY,0,sizeof(int)*LINEBUFFER);
	memset(crSumY,0,sizeof(int)*LINEBUFFER);
	
	//roi->add=(roi->add)&0x8fffffff;
	stride=roi->stride;
	XFilter0.num=0;
	XFilter1.num=0;
	YFilter.num=0;
	pixel.y_add=roi->add;
	pixel.mode=roi->mode;
	pixel.stride=roi->stride;
	if(pixel.mode==1)
		pixel.c_add=pixel.y_add+pixel.stride*roi->height;
	pixelIni=pixel;
	pixelEnd=pixel;

  	for(y=roi->yRoi;y<(roi->yRoi+roi->hRoi);y++)
  	{
  		if((y==roi->yRoi)||(y==roi->yRoi+roi->hRoi/2))
  		{
  			yfator=0;
  			fatorSumY=0;
  		}else
  		{
  			if(y<roi->yRoi+roi->hRoi/2)
  				yfator=roi->yRoi+roi->hRoi/2-y;
  			else		
  				yfator=y-roi->yRoi-roi->hRoi/2;
  			fatorSumY=fatorSumY+yfator;
  		}
  		for(x=roi->xRoi;x<(roi->xRoi+roi->wRoi);x++)
  		{
			pixel.x_coor=x;
			pixel.y_coor=y;
			if(x>=LINEBUFFER){
				printf("%s,%d: Error: Crazy, The Line Buffer should be extended!",__FILE__,__LINE__);
				while(1);
			}
			pixelRead(&pixel);
  			if((x==roi->xRoi)||((x-roi->xRoi)==(roi->wRoi/2)))
  			{
  				ySumX=0;
  				cbSumX=0;
  				crSumX=0;
  				xfator=0;
  				fatorSumX=0;
  			}else
  			{
  				if((x-roi->xRoi)<(roi->wRoi/2))
  					xfator=roi->xRoi+roi->wRoi/2-x;
  				else
  					xfator=x-roi->xRoi-roi->wRoi/2;
  				fatorSumX=fatorSumX+xfator;
  			}
  			pixelSum(&ySumX,&cbSumX,&crSumX,xfator,&pixel);
  			if(x==roi->xRoi+roi->wRoi/2-1)
  			{ 				
  				pixelAverage(&pixelIni,ySumX,cbSumX,crSumX,fatorSumX);
				pixelFilter(&XFilter0,&pixelIni);
  				pixelEnd=pixelIni;
  				pixelEnd.y=pixelEnd.y/2;
				pixelIni.x_coor=0;
				pixelIni.y_coor=y;
				pixelEnd.x_coor=roi->xRoi-1;
				pixelEnd.y_coor=y;
  				type=0;
				pixelLineFill(&pixelIni,&pixelEnd,type);
  
  			}else if(x==(roi->xRoi+roi->wRoi-1))
  			{
  				pixelAverage(&pixelIni,ySumX,cbSumX,crSumX,fatorSumX);
				pixelFilter(&XFilter1,&pixelIni);
 				pixelEnd=pixelIni;
  				pixelIni.y=pixelIni.y/2;
				pixelIni.x_coor=roi->xRoi+roi->wRoi;
				pixelIni.y_coor=y;
				pixelEnd.x_coor=roi->width-1;
				pixelEnd.y_coor=y;
  				type=0;
				pixelLineFill(&pixelIni,&pixelEnd,type);

  			}
  			if((y==roi->yRoi)||(y==roi->yRoi+roi->hRoi/2+1))
  			{
  				ySumY[x]=0;
  				cbSumY[x]=0;
  				crSumY[x]=0;
  			}else
  			 	pixelSum(&(ySumY[x]),&(cbSumY[x]),&(crSumY[x]),yfator,&pixel);
			  if(y==(roi->yRoi+roi->hRoi/2-1))
  			{
  

				pixelAverage(&pixelIni,ySumY[x],cbSumY[x],crSumY[x],fatorSumY);
				pixelFilter(&YFilter,&pixelIni);

  				pixelEnd=pixelIni;
  				pixelEnd.y=pixelEnd.y/2;
				pixelIni.x_coor=x;
				pixelIni.y_coor=0;
				pixelEnd.x_coor=x;
				pixelEnd.y_coor=roi->yRoi-1;
  				type=1;
				pixelLineFill(&pixelIni,&pixelEnd,type);

  			}else if(y==(roi->yRoi+roi->hRoi-1))
  			{
				pixelAverage(&pixelIni,ySumY[x],cbSumY[x],crSumY[x],fatorSumY);
				if(x==roi->xRoi)
					YFilter.num=0;
				pixelFilter(&YFilter,&pixelIni);
 				pixelEnd=pixelIni;
  				pixelIni.y=pixelIni.y/2;
				pixelIni.x_coor=x;
				pixelIni.y_coor=roi->yRoi+roi->hRoi;
				pixelEnd.x_coor=x;
				pixelEnd.y_coor=roi->height-1;
  				type=1;
  				pixelLineFill(&pixelIni,&pixelEnd,type);
					
  			}
  		}
  	}
  	
  	cornorFill_L_U(roi); 
  	cornorFill_R_U(roi);	
  	cornorFill_R_D(roi); 			
  	cornorFill_L_D(roi);
  }


//----------mirror----------------------------------
void pixelInit(img_radiant_pixel_t* pixel,img_radiant_roi_t* roi)
{
         pixel->y_add=roi->add;
         pixel->mode=roi->mode;
         pixel->stride=roi->stride;
         if(pixel->mode==1)
                   pixel->c_add=pixel->y_add+pixel->stride*roi->height;       

}

void pixelCopy(img_radiant_pixel_t* dst,img_radiant_pixel_t* src)
{
	pixelRead(src);
	dst->cb=src->cb;
	dst->cr=src->cr;
	dst->y=src->y;
	pixelWrite(dst);
}
void mirror_u_d(img_radiant_roi_t* roi,int div_roi,int up)
{
	int x,y;
	int xstart,ystart,xend,yend;
	img_radiant_pixel_t src,dst;
	int ys,ye;
	
	pixelInit(&src,roi);
	pixelInit(&dst,roi);	
	xstart=roi->xRoi;
	xend=roi->xRoi+roi->wRoi-1;
	ystart=roi->yRoi;
	yend=roi->yRoi+roi->hRoi-1;
	if(up==1)
	{
		ys=ystart;
		ye=div_roi-1;
	}else
	{
		ys=div_roi;
		ye=yend;
	}
	if(ys==ye)
		return;
	for(y=ys;y<=ye;y++)
		for(x=xstart;x<=xend;x++)
		{
			src.x_coor=x;
			if(up==1)
				src.y_coor=(yend-div_roi)*(div_roi-y)/(ye-ys)+div_roi;
			else
				src.y_coor=(div_roi-ystart)*(div_roi-y)/(ye-ys)+div_roi;
			dst.x_coor=x;
			dst.y_coor=y;
			pixelCopy(&dst,&src);			
		}
}
void mirror_l_r(img_radiant_roi_t* roi,int div_roi,int left)
{
	int x,y;
	int xstart,ystart,xend,yend;
	img_radiant_pixel_t src,dst;
	int xs,xe;
	
	pixelInit(&src,roi);
	pixelInit(&dst,roi);	
	xstart=roi->xRoi;
	xend=roi->xRoi+roi->wRoi-1;
	ystart=roi->yRoi;
	yend=roi->yRoi+roi->hRoi-1;
	if(left==1)
	{
		xs=xstart;
		xe=div_roi-1;
	}else
	{
		xs=div_roi;
		xe=xend;
	}
	if(xs==xe)
		return;
	for(y=ystart;y<=yend;y++)
		for(x=xs;x<=xe;x++)
		{
			if(left==1)
				src.x_coor=(xend-div_roi)*(div_roi-x)/(xe-xs)+div_roi;
			else
				src.x_coor=(div_roi-xstart)*(div_roi-x)/(xe-xs)+div_roi;
			src.y_coor=y;
			dst.x_coor=x;
			dst.y_coor=y;
			pixelCopy(&dst,&src);			
		}
}
void frame_mirror(img_radiant_roi_t* roi)
{
	img_radiant_roi_t roi_i,*roi_p=&roi_i;
	
	roi_p->add=roi->add;
	roi_p->height=roi->height;
	roi_p->stride=roi->stride;
	roi_p->mode=roi->mode;
	roi_p->width=roi->width;

	roi_p->xRoi=0;
	roi_p->wRoi=roi->xRoi+roi->wRoi/2;
	roi_p->yRoi=roi->yRoi;
	roi_p->hRoi=roi->hRoi;
	mirror_l_r(roi_p,roi->xRoi,1);

	roi_p->xRoi=roi->xRoi+roi->wRoi/2;
	roi_p->wRoi=roi->width-roi_p->xRoi;
	roi_p->yRoi=roi->yRoi;
	roi_p->hRoi=roi->hRoi;
	mirror_l_r(roi_p,roi->xRoi+roi->wRoi-1,0);	

	roi_p->xRoi=0;
	roi_p->wRoi=roi->width;
	roi_p->yRoi=0;
	roi_p->hRoi=roi->yRoi+roi->hRoi/2;
	mirror_u_d(roi_p,roi->yRoi,1);

	roi_p->xRoi=0;
	roi_p->wRoi=roi->width;
	roi_p->yRoi=roi->yRoi+roi->hRoi/2;
	roi_p->hRoi=roi->height-roi_p->yRoi;
	mirror_u_d(roi_p,roi->yRoi+roi->hRoi-1,0);

}

void  image_radiant(bg_img_dec_t *img_dec,INT8S type)
  {
  	img_radiant_roi_t roi;	
	//if(radiant_color_en==0||(imgInfo.rect_width==imgInfo.img_actual_width&&imgInfo.rect_height==imgInfo.img_actual_height))
	//	return;
	#if 0
	printf("out_width=%d,height=%d,real_width=%d,real_height=%d,src_width=%d,src_height=%d\n",img_dec->img_dec_para_info.output_width,\
		img_dec->img_dec_para_info.output_height,img_dec->img_dec_ret_info.img_actual_wid,img_dec->img_dec_ret_info.img_actual_hei,\
		img_dec->img_dec_ret_info.src_width,img_dec->img_dec_ret_info.src_height);
	#endif
	if(type!=PHOTO_BKG_RADIANT && type!=PHOTO_BKG_RADIANTMIRROR)
		return;

	if(img_dec->img_dec_para_info.output_width==img_dec->img_dec_ret_info.img_actual_wid && \
		img_dec->img_dec_para_info.output_height== img_dec->img_dec_ret_info.img_actual_hei){
		return ;
	}
	
	roi.add=(int)img_dec->img_dec_para_info.out_buf_y.buf;
	roi.width=img_dec->img_dec_para_info.output_width;
	roi.mode=0;
	roi.stride=roi.width;
	roi.height=img_dec->img_dec_para_info.output_height;
	roi.xRoi=img_dec->img_dec_ret_info.img_start_x;
	roi.yRoi=img_dec->img_dec_ret_info.img_start_y;

	if(type==0&&roi.xRoi!=0&&roi.yRoi!=0)
	{
		roi.xRoi+=2;
		roi.yRoi+=2;
	}
	roi.wRoi=roi.width-roi.xRoi*2;
	roi.hRoi=roi.height-roi.yRoi*2;
	
	if(roi.xRoi==1)
		roi.xRoi++;
	if(roi.yRoi==1)
		roi.yRoi++;
	if(roi.height==1)
		roi.height++;
	if(roi.width==1)
		roi.width++;	
	
	if(type==PHOTO_BKG_RADIANT)
		frame(&roi);
	else if(type==PHOTO_BKG_RADIANTMIRROR)
		frame_mirror(&roi);
	else
		printf("%s,%d:Photo Radiant Type Error type=%d!",__FILE__,__LINE__,type);

  }
