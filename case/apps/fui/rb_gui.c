#include <string.h>
#include "rb_gui.h"
#include "swfdec.h"
#include "load_photo_api.h"
#include "fui_memfs.h"
#include "fui_common.h"

#ifdef MODULE_CONFIG_RB_GUI

#define RBVDC_MAX_W 1920
#define RBVDC_MAX_H 1080

#define RB_BPP 2

#define RB_ARGB_MUX(a,r,g,b)\
    (((a) << 24) | ((r) << 16) | ((g) << 8) | (b))

#define RB_RGB2YUV(y,u,v,r,g,b)\
{\
	y = (((66 * r + 129 * g + 25 * b) + 128) >> 8) + 16;\
	u = (((-38 * r - 74 * g + 112 * b) + 128) >> 8) + 128;\
	v = (((112 * r - 94 * g - 18 * b) + 128) >> 8) + 128;\
}

#define RB_MAX(a,b) (((a) > (b)) ? (a) : (b))
#define RB_MIN(a,b) (((a) < (b)) ? (a) : (b))

#define RB_YUV2RGB(r, g, b, y, u, v)\
{\
	int c,d,e;\
	c = y - 16;\
	d = u - 128;\
	e = v - 128;\
	r = RB_MAX(0,RB_MIN(255,(298 * c + 409 * e + 128) >> 8));\
	g = RB_MAX(0,RB_MIN(255,(298 * c - 100 * d - 208 * e + 128) >> 8));\
	b = RB_MAX(0,RB_MIN(255,(298 * c + 516 * d + 128) >> 8));\
}

static void rbInitIdentifyMatrix(SWF_MATRIX * pMatrix)
{
	pMatrix->ScaleX=0x10000;
	pMatrix->ScaleY=0x10000;
	pMatrix->RotateSkew0=0;
	pMatrix->RotateSkew1=0;
	pMatrix->TranslateX=0;
	pMatrix->TranslateY=0;
}

static void rbInitCxform(SWF_CXFORM * colortransform)
{
	colortransform->RedMultTerm = 0x100;
	colortransform->GreenMultTerm = 0x100;
	colortransform->BlueMultTerm = 0x100;
	colortransform->AlphaMultTerm = 0x100;
	colortransform->RedAddTerm = 0;
	colortransform->GreenAddTerm =0;
	colortransform->BlueAddTerm = 0;
	colortransform->AlphaAddTerm = 0;
}

#define RB_BASIC_LINE_W 2


static unsigned short rbGetUtf16(char * str, int * index)
{
	int i = *index;
	
	if((str[i] & 0x80) == 0)
	{
		*index = i + 1;
		return str[i];
	}
	else if((str[i] & 0x20) == 0)
	{
		*index = i + 2;
		return ((str[i] << 6) & 0x07c0) | (str[i+1] & 0x003f);
	}
	else if((str[i] & 0x10) == 0)
	{
		*index = i + 3;
		return ((str[i] << 12) & 0xf000) | ((str[i+1] << 6) & 0x0fc0) | (str[i+2] & 0x003f);
	}
	
	return 0;
}

static int rbUtf8ToUtf16(char * u8str, unsigned short * u16str)
{
	int i, j;
	for(i = 0, j = 0; (u16str[j] = rbGetUtf16(u8str, &i)) != 0; j++);
	return j;
}

static int rbGetUtf16LenFromUtf8(char * u8str)
{
	int i=0, j=0;

	if(u8str == NULL){
		return 0;
	}
	while(rbGetUtf16(u8str, &i)){
		j++;
	}

	return j;
}

static int rbGetEllipsePath(UIPATH* upath,RECT*rect)
{
	upath->pointArray[0].x=rect->left*16;
	upath->pointArray[0].y=(rect->top+rect->bottom)/2*16;
	upath->pointArray[0].flag=TYPE_MOVETO;
	upath->pointArray[1].x=rect->left*16;
	upath->pointArray[1].y=rect->top*16;
	upath->pointArray[1].flag=TYPE_CURVETO;		
	upath->pointArray[2].x=(rect->left+rect->right)/2*16;
	upath->pointArray[2].y=rect->top*16;
	upath->pointArray[2].flag=TYPE_CURVETO;
	upath->pointArray[3].x=rect->right*16;
	upath->pointArray[3].y=rect->top*16;
	upath->pointArray[3].flag=TYPE_CURVETO;
	upath->pointArray[4].x=rect->right*16;
	upath->pointArray[4].y=(rect->top+rect->bottom)/2*16;
	upath->pointArray[4].flag=TYPE_CURVETO;
	upath->pointArray[5].x=rect->right*16;
	upath->pointArray[5].y=rect->bottom*16;
	upath->pointArray[5].flag=TYPE_CURVETO;
	upath->pointArray[6].x=(rect->left+rect->right)/2*16;
	upath->pointArray[6].y=rect->bottom*16;
	upath->pointArray[6].flag=TYPE_CURVETO;
	upath->pointArray[7].x=rect->left*16;
	upath->pointArray[7].y=rect->bottom*16;
	upath->pointArray[7].flag=TYPE_CURVETO;
	upath->pointArray[8].x=rect->left*16;
	upath->pointArray[8].y=(rect->top+rect->bottom)/2*16;
	upath->pointArray[8].flag=TYPE_CURVETO;

	upath->npoints=9;	
	
	return 0;
}

HVDC rbCreateVDC(int width,int height)
{
	HVDC dev=NULL;

	dev = malloc(sizeof(VDC));
	if(dev == NULL){
		return NULL;
	}

	if((width<=0) || (width>RBVDC_MAX_W)){
		printf("rbCreateVDC error: width=%d\n",width);
		free(dev);
		return NULL;
	}

	if((height<=0) || (height>RBVDC_MAX_H)){
		printf("rbCreateVDC error: height=%d\n",height);
		free(dev);
		return NULL;
	}

	dev->width = width;
	dev->height = height;
	dev->pixel_format = SWF_BMP_FMT_RGB565;
	dev->text_color = 0xff000000;
	dev->text_bkcolor = 0xff000000;
	dev->text_bkcolor_en = 0;
	dev->font_size = 16;
	dev->dev_vaddr = (unsigned int)SWF_Malloc(width*height*2);
	if((void *)dev->dev_vaddr == NULL){
		free(dev);
		return NULL;
	}
	dev->dev_paddr = fui_get_bus_address(dev->dev_vaddr);

	memset((void *)dev->dev_vaddr,0,width*height*2);
	
	return dev;
}

int rbDeleteVDC(HVDC hvDC)
{
	if(hvDC != NULL){
		if(hvDC->dev_vaddr){
			SWF_Free((void *)hvDC->dev_vaddr);
		}
		free(hvDC);
		hvDC = NULL;
	}
	return 1;
}

int rbSetTextColor(HVDC hvDC,rbCOLOR color)
{

	hvDC->text_color = RB_ARGB_MUX(color.alpha,color.red,color.green,color.blue);
	return 1;
}

int rbSetBkColor(HVDC hvDC,rbCOLOR color)
{
	hvDC->text_bkcolor_en = 1;
	hvDC->text_bkcolor = RB_ARGB_MUX(color.alpha,color.red,color.green,color.blue);
	return 1;
}

int rbFillRect(HVDC hvDC,RECT *lprc,rbCOLOR color)
{
	UIPATH Polygon;

	Polygon.npoints =5;
	Polygon.color = RB_ARGB_MUX(color.alpha,color.red,color.green,color.blue);
	Polygon.pointArray = SWF_Malloc(5*sizeof(UIPOINT));

	Polygon.pointArray[0].x = lprc->left * 16;
	Polygon.pointArray[0].y = lprc->top * 16;
	Polygon.pointArray[0].flag = TYPE_MOVETO;

	Polygon.pointArray[1].x = lprc->right* 16;
	Polygon.pointArray[1].y = lprc->top * 16;
	Polygon.pointArray[1].flag = TYPE_LINETO;

	Polygon.pointArray[2].x = lprc->right * 16;
	Polygon.pointArray[2].y = lprc->bottom* 16;
	Polygon.pointArray[2].flag = TYPE_LINETO;

	Polygon.pointArray[3].x = lprc->left * 16;
	Polygon.pointArray[3].y = lprc->bottom * 16;
	Polygon.pointArray[3].flag = TYPE_LINETO;

	Polygon.pointArray[4].x = lprc->left * 16;
	Polygon.pointArray[4].y = lprc->top * 16;
	Polygon.pointArray[4].flag = TYPE_LINETO;

	SWF_Sync2D();
	SWF_GUIFillPolygon(hvDC, &Polygon);
	SWF_Free(Polygon.pointArray);
	
	return 1;
}

int rbTextOut(HVDC hvDC,int x,int y,char * lpString,int cbString)
{
	int a,r,g,b;
	UITEXT Utext;
	int utf16len=0;

	if((lpString==NULL) || (cbString<0)){
		return 0;
	}

	utf16len = rbGetUtf16LenFromUtf8(lpString);

	Utext.color = hvDC->text_color;
	Utext.size=hvDC->font_size;
	Utext.x=x;
	Utext.y=y;
	Utext.string=(char *)SWF_Malloc(sizeof(unsigned short)*(utf16len+1));
	if(Utext.string == NULL){
		return 0;
	}
	rbUtf8ToUtf16(lpString,(unsigned short *)Utext.string);
	SWF_Sync2D();
	SWF_GUIDrawText(hvDC, &Utext);
	SWF_Free((void *)Utext.string);
	
	return 1;
}

int rbInvertRect(HVDC hvDC,RECT *lprc)
{
	int x,y,w,h;
	int i,j;
	int offset;
	int yuv;
	int u,v;
	int y1,y2;

	x = lprc->left;
	if(x<0){
		x = 0;
	}
	else if(x>=hvDC->width){
		return 0;
	}
	
	w = lprc->right - x;
	if(w<=0){
		return 0;
	}
	else if(w>(hvDC->width-x)){
		w = hvDC->width-x;
	}

	y = lprc->top;
	if(y<=0){
		y = 0;
	}
	else if(y>hvDC->height){
		return 0;
	}
	
	h = lprc->bottom-y+1;
	if(h<=0){
		return 0;
	}
	else if(h>=(hvDC->height-y)){
		h=(hvDC->height-y);
	}

	for(i=0;i<h;i++){
		unsigned short *ptr;
		unsigned int r,g,b;
		unsigned int rgb;
		
		offset = (y+i)*hvDC->width*2 +x*2;
		ptr = (unsigned short *)(hvDC->dev_vaddr + offset);
		for(j=0;j<w;j++){
			rgb = *(ptr+j);
			r = 255-((rgb>>8)&0xf8);
			g = 255-((rgb>>3)&0xfc);
			b = 255-((rgb<<3)&0xf8);
			*(ptr+j) = ((r<<8)&0xf800) | ((g<<3)&0x7e0) | ((b>>3)&0x1f);
		}
	}

	
	return 1;
}

int rbAttachVdc(void *target,int width,int height,HVDC hvDC)
{
	if(target==NULL || hvDC==NULL){
		return 0;
	}

	SWF_Sync2D();
	SWF_AttachBitmap(target,(unsigned int*)hvDC->dev_vaddr,width,height,width,height,width,SWF_BMP_FMT_RGB565,NULL);

	return 1;
}

int rbDettachVdc(void *target)
{
	if(target==NULL){
		return 0;
	}

	SWF_Sync2D();
	SWF_DetachBitmap(target,0);

	return 1;
}

int rbDrawLine(HVDC hvDC,int x1,int y1,int x2,int y2, rbCOLOR color)
{
	UIPATH line;

	line.npoints =2;
	line.width = RB_BASIC_LINE_W * 16;
	line.color = RB_ARGB_MUX(color.alpha,color.red,color.green,color.blue);
	line.pointArray = (UIPOINT *)SWF_Malloc(line.npoints*sizeof(UIPOINT));

	if(line.pointArray == NULL){
		printf("draw line error:[malloc failed]\n");
		return 0;
	}
	
	line.pointArray[0].x = x1 * 16;
	line.pointArray[0].y = y1 * 16;
	line.pointArray[0].flag = TYPE_MOVETO;

	line.pointArray[1].x = x2* 16;
	line.pointArray[1].y = y2 * 16;
	line.pointArray[1].flag = TYPE_LINETO;

	SWF_Sync2D();
	SWF_GUIDrawLine(hvDC, &line);

	if(line.pointArray){
		SWF_Free(line.pointArray);
	}
	
	return 1;
}


int rbSetPixel(HVDC hvDC,int x,int y,rbCOLOR color)
{
	unsigned short *ptr;
	unsigned short rgb;
	
	SWF_Sync2D();
	
	if(x<0 || x>=hvDC->width){
		return 0;
	}

	if(y<0 || y>=hvDC->height){
		return 0;
	}

	ptr = (unsigned short*)(hvDC->dev_vaddr + y*hvDC->width*2 + x*2);

	*ptr = (unsigned short)(((color.red<<8)&0xf800) | ((color.green<<3)&0x7e0) | ((color.blue>>3)&0x1f));
	
	
	return 1;
}

int rbDrawRectangle(HVDC hvDC,  RECT *lprc,rbCOLOR bordercolor, rbCOLOR fillcolor)
{
	UIPATH line;

	rbFillRect(hvDC,lprc,fillcolor);
	
	line.npoints =5;
	line.width = RB_BASIC_LINE_W * 16;
	line.color = RB_ARGB_MUX(bordercolor.alpha,bordercolor.red,bordercolor.green,bordercolor.blue);
	line.pointArray = SWF_Malloc(line.npoints*sizeof(UIPOINT));

	if(line.pointArray == NULL){
		return 0;
	}

	line.pointArray[0].x = lprc->left * 16;
	line.pointArray[0].y = lprc->top * 16;
	line.pointArray[0].flag = TYPE_MOVETO;

	line.pointArray[1].x = lprc->right* 16;
	line.pointArray[1].y = lprc->top * 16;
	line.pointArray[1].flag = TYPE_LINETO;

	line.pointArray[2].x = lprc->right * 16;
	line.pointArray[2].y = lprc->bottom* 16;
	line.pointArray[2].flag = TYPE_LINETO;

	line.pointArray[3].x = lprc->left * 16;
	line.pointArray[3].y = lprc->bottom * 16;
	line.pointArray[3].flag = TYPE_LINETO;

	line.pointArray[4].x = lprc->left * 16;
	line.pointArray[4].y = lprc->top * 16;
	line.pointArray[4].flag = TYPE_LINETO;

	SWF_Sync2D();
	SWF_GUIDrawLine(hvDC, &line);

	SWF_Free(line.pointArray);
	
	return 1;
}

int rbDrawEllipse(HVDC hvDC,  RECT *lprc,rbCOLOR bordercolor, rbCOLOR fillcolor)
{
	UIPATH Upath;
	UIPOINT pointArray[9];

	Upath.pointArray = pointArray;
	rbGetEllipsePath(&Upath,lprc);

	SWF_Sync2D();
	
	/**
	* fill the internal.
	*/
	Upath.color = RB_ARGB_MUX(fillcolor.alpha,fillcolor.red,fillcolor.green,fillcolor.blue);
	SWF_GUIFillPolygon(hvDC, &Upath);

	/**
	* draw the border.
	*/
	Upath.color = RB_ARGB_MUX(bordercolor.alpha,bordercolor.red,bordercolor.green,bordercolor.blue);
	Upath.width = RB_BASIC_LINE_W * 16;
	SWF_GUIDrawLine(hvDC, &Upath);
	
	return 0;
}

int rbSetFont(HVDC hvDC,char fontname)
{
	return 0;
}

int rbSetFontSize(HVDC hvDC,int size)
{
	if(size <= 0){
		return 0;
	}
	hvDC->font_size = size;
	return 1;
}

int rbExtSetFont(HVDC hvDC,char fontname,int size,int isBold,int isItalic,int isUnderline)
{
	return 0;
}

static int rbLoadImage(char *imgdata, long imgsize, unsigned char **decdata, int *decw, int *dech, int *decstride,int format)
{
	int w,h;
	void *fp;
	LP_INPUT_PARAM lp_param;
	long len;
	int ret=0;
	
	/**
	* make width 2 aligned.
	*/
	//w = ((*decw+15)/16)*16;
	//h = ((*dech+15)/16)*16;
	w = ((*decw+1)/2)*2;
	h = ((*dech+1)/2)*2;
	*decstride = w;

	if((format == 0) || (format == 1) || (format == 2) || (format == 3)){
		len = w*(h+1)*4;
	}
	else{
		len = w*(h+1)*2;
	}

	/**
	* malloc decoder data.
	*/
	*decdata = (unsigned char *)SWF_Malloc(len);
	if(*decdata == NULL){
		printf("fui_dec_internal_jpeg malloc error\n");
		return LP_DECODE_ERROR;
	}

	/**
	* open memory fs.
	*/
	fp = fui_memfs_open(imgdata, imgsize);
	if(fp == NULL){
		SWF_Free(*decdata);
		*decdata = NULL;
		printf("fui_dec_internal_jpeg fp error\n");
		return LP_DECODE_ERROR;
	}

	/**
	* begin decode.
	*/
	lp_param.handle = fp;
	lp_param.lp_fread = fui_memfs_read;
	lp_param.lp_fseek_cur = fui_memfs_seek_cur;
	lp_param.lp_fseek_end = fui_memfs_seek_end;
	lp_param.lp_fseek_set = fui_memfs_seek_set;
	lp_param.lp_ftell = fui_memfs_tell;	
	lp_param.lp_malloc = (void *(*)(unsigned long))SWF_Malloc;
	lp_param.lp_free = SWF_Free;
	lp_param.lp_get_bus_addr = fui_get_bus_address;
	lp_param.lp_realloc = NULL;
	if((format == 0) || (format == 1) || (format == 2) || (format == 3)){
		lp_param.output_pix_fmt = LP_FMT_RGB_32;
	}
	else{
		lp_param.output_pix_fmt = LP_FMT_YCBCR_4_2_2_INTERLEAVED;
	}
	lp_param.out_buf_y.buf = *decdata;
	lp_param.out_buf_y.bus_addr = fui_get_bus_address((unsigned long)decdata);
	lp_param.out_buf_y.size = len;
	lp_param.out_buf_uv.buf = NULL;
	lp_param.out_buf_v.buf = NULL;
	lp_param.out_buf_width = w;
	lp_param.out_buf_height = h;
	lp_param.out_pic_pos_x = 0;
	lp_param.out_pic_pos_y = 0;
	lp_param.out_pic_width = w;
	lp_param.out_pic_height = h;

	/**
	* do decode.
	*/
	if((format == 0) || (format == 1)){
		ret = load_pic(&lp_param,".bmp");
	}
	else if(format == 2){
		ret = load_pic(&lp_param,".png");
	}
	else if(format == 3){
		ret = load_pic(&lp_param,".gif");
	}
	else if(format == 4){
		ret = load_pic(&lp_param,".jpg");
	}
	else{
		ret = LP_DECODE_ERROR;
	}
	
	
	if(ret != LP_OK){
		SWF_Free(*decdata);
		*decdata = NULL;
	}

	if(fp){
		fui_memfs_close(fp);
		fp = NULL;
	}
	
	return ret;
	
}

/**
* @brief Convert from RGB565 to YUV422
* @param prgb: The rgb565 data pointer.
* @param pyuv: The yuv422 data pointer.
* @param w: The width of the area that to be converted.
* @param h: The height of the area that to be converted.
* @param src_stride: The source data stride in pixels.
* @param dst_stride: The source data stride in pixels.
* @note Please make sure that width is 2 aligned.
*/
static int rbRgb565ToYuv422(void *prgb,void *pyuv,int *w,int *h,int src_stride,int dst_stride)
{
	int i,j;
	int width = *w,height = *h;
	unsigned int *psrc,*pdst;
	
	if((prgb == NULL) || (pyuv == NULL)){
		return -1;
	}

	if(width <= 0 || height<=0){
		return -1;
	}

	SWF_Sync2D();


	for(i=0;i<height;i++){
		int r1,g1,b1,r2,b2,g2,rgb;
		int y1,u1,v1,y2,u2,v2;
		int yuv,u,v;
		psrc = (unsigned int *)((unsigned int)prgb + src_stride*2*i);
		pdst = (unsigned int *)((unsigned int)pyuv + dst_stride*2*i);
		for(j=0;j<width/2;j++){
			rgb = *(psrc+j);
		
			r1 = (rgb>>8) & 0xf8;
			b1 = (rgb<<3) & 0xf8;
			r2 = (rgb>>24) & 0xf8;
			b2 = (rgb>>13) & 0xf8;
			g1 = (rgb>>3) & 0xfc;
			g2 = (rgb>>19) & 0xfc;
			
			RB_RGB2YUV(y1,u1,v1,r1,g1,b1);
			RB_RGB2YUV(y2,u2,v2,r2,g2,b2);
			u = (u1 + u2)/2;
			v = (v1 + v2)/2;
			yuv = (y1&0xff) | ((u<<8)&0xff00) | ((y2<<16)&0xff0000) | ((v<<24)&0xff000000);
			*(pdst+j) = yuv;
		}
	}

	return 0;
}



int rbDrawImage(HVDC hvDC,rbIMAGE *img,int x,int y)
{
	unsigned char *decImage=NULL;
	int width,height,stride;
	int err;
	FILLINFO fillinfo;
	
	if((hvDC == NULL) || ((void *)hvDC->dev_vaddr == NULL) || (img==NULL)){
		printf("rbDrawImage Error: Pointer NULL!\n");
		return 0;
	}

	if((x<0) || (x>=hvDC->width)){
		printf("rbDrawImage Error: x coordinate error!\n");
		return 0;
	}

	if((y<0) || (y>=hvDC->height)){
		printf("rbDrawImage Error: y coordinate error!\n");
		return 0;
	}

	width = img->width;
	height = img->height;
	stride = img->width;

	/**
	* For bmp/png/gif, the decoded format is ARGB.
	* For jpeg, the decoded format is YUV422.
	*/
	if((img->type < 0) || (img->type > 5)){
		printf("rbDrawImage Error: image format error!\n");
		return 0;
	}

	/**
	* decode image.
	*/
	if(img->type != 5){
		rbLoadImage(img->data,img->datasize, &decImage, &width, &height,&stride,img->type);
		if(decImage == NULL){
			printf("rbDrawImage Error: decode error!\n");
			return 0;
		}
	}
	else{
		/**
		* deal with rgb565 data.
		*/
		int i;
		char *src,*dst;
		
		if(x<0 || x>=hvDC->width){
			return 0;
		}

		if(y<0 || y>=hvDC->height){
			return 0;
		}

		if( (x+width) > hvDC->width){
			width = hvDC->width -x;
		}

		if( (y+height) > hvDC->height){
			height = hvDC->height - y;
		}
		
		SWF_Sync2D();

		src = img->data;
		dst = (char *)(hvDC->dev_vaddr + y*hvDC->width*2 + x*2);
		
		for(i=0;i<height;i++){
			memcpy(dst,src,width*2);
			src += img->width*2;
			dst += hvDC->width*2;
		}

		return 0;
		
	}

	if( (x+width) > hvDC->width){
		width = hvDC->width -x;
	}

	if( (y+height) > hvDC->height){
		height = hvDC->height - y;
	}


	/**
	* use 2D to do draw.
	*/	
	fillinfo.input_type = 1;
	fillinfo.DefColor = 0;
	fillinfo.input_addr    = (int)decImage;
	fillinfo.input_start_x = 0;
	fillinfo.input_start_y = 0;
	fillinfo.input_stride  = stride;
	fillinfo.input_repeat  = 0;
	if((img->type==0) || (img->type==1) || (img->type==2) || (img->type==3)){
		fillinfo.input_need_blend = 1;
		fillinfo.input_format  = 0;
	}
	else if(img->type == 4){
		fillinfo.input_need_blend = 1;
		fillinfo.input_format  = 3;
	}
	else{
		return 0;
		///fillinfo.input_need_blend = 0;
		///fillinfo.input_format  = 3;
	}
	fillinfo.input_need_interpolate = 0;
	fillinfo.input_width   = width;
	fillinfo.input_height  = height;
	fillinfo.output_type   = 0;
	fillinfo.output_format = 2;
	fillinfo.output_addr   = hvDC->dev_vaddr;
	fillinfo.output_offset = y*hvDC->width+x;
	fillinfo.output_stride = hvDC->width;
	fillinfo.output_width  = width;
	fillinfo.output_height = height;
	fillinfo.z_buffer      = 0;
	fillinfo.pair_addr = 0;
	fillinfo.blend_mode = 0;
	fillinfo.clip.Xmin = 0;
	fillinfo.clip.Xmax = width - 1;
	fillinfo.clip.Ymin = 0;
	fillinfo.clip.Ymax = height - 1;
	rbInitCxform(&fillinfo.Cxform);
	rbInitIdentifyMatrix(&fillinfo.Matrix);
	SWF_2DFill(&fillinfo);

	/**
	* free the decoded buffer
	*/
	if(decImage != NULL){
		SWF_Free(decImage);
	}
	
	return 1;
}

int rbBitBlt(HVDC hvDC_dst,int xDes,int yDes,int width,int height,HVDC hvDC_src,int xSrc,int ySrc)
{
	int i,j;
	unsigned int psrc,pdest;
	int w1,h1;
	
	if((hvDC_dst == NULL) || (hvDC_src==NULL) ){
		return 0;
	}

	if(((void *)hvDC_dst->dev_vaddr == NULL) || ((void *)hvDC_src->dev_vaddr == NULL)){
		return 0;
	}

	if((width > hvDC_dst->width) || (width > hvDC_src->width)){
		return 0;
	}

	if((height > hvDC_dst->height) || (height > hvDC_src->height)){
		return 0;
	}

	/**
	* souce clip.
	*/
	if((xSrc>hvDC_src->width) || (xSrc < 0)){
		return 0;
	}
	if((ySrc>hvDC_src->height) || (ySrc < 0)){
		return 0;
	}
	
	if((xSrc + width)>hvDC_src->width){
		w1 = hvDC_src->width-xSrc;
	}
	else{
		w1 = width;
	}

	if((ySrc + height)>hvDC_src->height){
		h1 = hvDC_src->height-ySrc;
	}
	else{
		h1 = height;
	}

	/**
	* dest clip
	*/
	if((xDes>hvDC_dst->width) || (xDes < 0)){
		return 0;
	}
	if((yDes>hvDC_dst->height) || (yDes < 0)){
		return 0;
	}
	
	if((xDes + width)>hvDC_dst->width){
		width = hvDC_dst->width-xDes;
	}

	if((yDes + height)>hvDC_dst->height){
		height = hvDC_dst->height-yDes;
	}

	if(width > w1){
		width = w1;
	}

	if(height > h1){
		height = h1;
	}

	SWF_Sync2D();

	/**
	* do copy
	*/
	psrc = hvDC_src->dev_vaddr + ySrc*hvDC_src->width*RB_BPP + xSrc*RB_BPP;
	pdest = hvDC_dst->dev_vaddr + yDes*hvDC_dst->width*RB_BPP + xDes*RB_BPP;
	for(i=0;i<height;i++){
		memcpy((void *)pdest,(void *)psrc,width*RB_BPP);
		psrc += RB_BPP*hvDC_src->width;
		pdest += RB_BPP*hvDC_dst->width;
	}
	
	return 1;
}

int rbGetPixel(HVDC hvDC,int x,int y,rbCOLOR *color)
{
	int yuv,yc,u,v;
	unsigned short rgb;
	int odd = 0;
	
	if((hvDC == NULL) || (color==NULL) || ((void *)hvDC->dev_vaddr == NULL)){
		return 0;
	}

	color->red = 0;
	color->green= 0;
	color->blue= 0;
	color->alpha = 0xff;

	if((x<0) || (x>=hvDC->width)){
		return 0;
	}

	if((y<0) || (y>=hvDC->height)){
		return 0;
	}

	rgb = *((unsigned short *)(hvDC->dev_vaddr + y*hvDC->width*RB_BPP + x*RB_BPP));
	color->red = (rgb>>8)&0xf8;
	color->green= (rgb>>3)&0xfc;
	color->blue= (rgb<<3)&0xf8;
	color->alpha = 0xff;

	return 1;
			
	
}

#endif /** MODULE_CONFIG_RB_GUI */


