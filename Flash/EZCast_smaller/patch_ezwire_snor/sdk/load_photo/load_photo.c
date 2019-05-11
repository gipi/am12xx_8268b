
/* load photo lib */

#include "load_photo.h"

static 
void *_malloc_(int size)
{
	void *p = DWLmalloc(size);
//	OSprintf("jpg_malloc(), size:%d, addr:0x%08x\n", size, (unsigned long)p);
	return p;
}

static 
void _free_(void *p)
{
//	OSprintf("jpg_free(), addr:0x%08x\n", (unsigned long)p);
	DWLfree(p);
}

#ifdef WIN32
static int init_sys_buf(LP_SYS_BUF_INST *sys_buf_inst, int flags)
{
	return 0;
}

static int release_sys_buf(LP_SYS_BUF_INST *sys_buf_inst)
{
	return 0;
}

static int malloc_sys_buf(LP_SYS_BUF_INST *sys_buf_inst, LP_LINEAR_BUFFER *linear_buf)
{
	int size = linear_buf->size;
	linear_buf->buf = (unsigned char *)OSmalloc(size);
	if (NULL == linear_buf->buf)
	{
		OSprintf("malloc_sys_buf error!\n");
		return -1;
	}
	linear_buf->bus_addr = ((unsigned long)linear_buf->buf & 0x0fffffff);
	return 0;
}

static void *png_malloc_frame(void *p_heap, int size, unsigned long *p_bus_addr)
{
	void *p = OSmalloc(size);
	if (p && p_bus_addr)
		*p_bus_addr = (unsigned long)p;
	return p;
}

static int free_sys_buf(LP_SYS_BUF_INST *sys_buf_inst, LP_LINEAR_BUFFER *linear_buf)
{
	if (linear_buf->buf != NULL)
	{
		OSfree(linear_buf->buf);
		linear_buf->buf = NULL;
	}
	return 0;
}

static void png_free_frame(void *p_heap, void *buf, unsigned long bus_addr)
{
	if (buf != NULL)
	{
		OSfree(buf);
		buf = NULL;
	}
}

#else
static int init_sys_buf(LP_SYS_BUF_INST *sys_buf_inst, int flags)
{
	sys_buf_inst->arg = (struct mem_dev *)OSmalloc(sizeof(struct mem_dev));
	if (NULL == sys_buf_inst->arg)
	{
		OSprintf("init_sys_buf() error: can not malloc arg!\n");
		goto error;
	}
	OSmemset(sys_buf_inst->arg, 0, sizeof(struct mem_dev));

	sys_buf_inst->arg->buf_attr = UNCACHE;

	sys_buf_inst->fd = open("/dev/sysbuf", flags);
	if (sys_buf_inst->fd < 0)
	{
		OSprintf("init_sys_buf() error: open sysbuf error!\n");
		goto error;
	}
	return 0;

error:
	if (sys_buf_inst->arg != NULL)
	{
		OSfree(sys_buf_inst->arg);
		sys_buf_inst->arg = NULL;
	}
	close(sys_buf_inst->fd);
	return -1;
}

static int release_sys_buf(LP_SYS_BUF_INST *sys_buf_inst)
{
	if (sys_buf_inst->arg != NULL)
	{
		OSfree(sys_buf_inst->arg);
		sys_buf_inst->arg = NULL;
	}
	close(sys_buf_inst->fd);
	return 0;
}

/* 最好4K对齐 */
static int malloc_sys_buf(LP_SYS_BUF_INST *sys_buf_inst, LP_LINEAR_BUFFER *linear_buf)
{
	int ret = 0;
	sys_buf_inst->arg->request_size = linear_buf->size;
	/* get memory */
	ret = ioctl(sys_buf_inst->fd, MEM_GET, sys_buf_inst->arg);
	if (ret < 0)
	{
		OSprintf("malloc_sys_buf() error!\n");
		return -1;
	}
	linear_buf->bus_addr = sys_buf_inst->arg->physic_address;
	linear_buf->buf = (unsigned char*)sys_buf_inst->arg->logic_address;

	return 0;
}

static void *png_malloc_frame(void *p_heap, int size, unsigned long *p_bus_addr)
{
	int ret = 0;
	LP_SYS_BUF_INST *sys_buf = (LP_SYS_BUF_INST *)p_heap;
	sys_buf->arg->request_size = size;
	ioctl(sys_buf->fd, MEM_GET, sys_buf->arg);
	if (ret < 0)
	{
		OSprintf("malloc_frame() error!\n");
		return NULL;
	}
	if (NULL == p_bus_addr)
	{
		OSprintf("malloc_frame() error: p_bus_addr is NULL!\n");
		return NULL;
	}
	*p_bus_addr = sys_buf->arg->physic_address;
	return (void*)sys_buf->arg->logic_address;
}

static int free_sys_buf(LP_SYS_BUF_INST *sys_buf_inst, LP_LINEAR_BUFFER *linear_buf)
{
	if (linear_buf->buf != NULL)
	{
		sys_buf_inst->arg->physic_address = linear_buf->bus_addr;	
		sys_buf_inst->arg->logic_address = (unsigned long)linear_buf->buf;
		sys_buf_inst->arg->request_size = linear_buf->size;
		ioctl(sys_buf_inst->fd, MEM_PUT, sys_buf_inst->arg);
		linear_buf->buf = NULL;
	}
	
	return 0;
}

static void png_free_frame(void *p_heap, void *buf, unsigned long bus_addr)
{
	LP_SYS_BUF_INST *sys_buf = (LP_SYS_BUF_INST *)p_heap;
	if (buf != NULL)
	{
		sys_buf->arg->physic_address = bus_addr;
		sys_buf->arg->logic_address = (unsigned long)buf;
		ioctl(sys_buf->fd, MEM_PUT, sys_buf->arg);
		buf = NULL;
	}
}
#endif

static void JPEG_decRet(JpegDecRet ret)
{
	OSprintf("Decode result: ");
	switch (ret)
	{
	case JPEGDEC_SLICE_READY:
		OSprintf("JPEGDEC_SLICE_READY\n");
		break;
	case JPEGDEC_FRAME_READY:
		OSprintf("JPEGDEC_FRAME_READY\n");
		break;
	case JPEGDEC_STRM_PROCESSED:
		OSprintf("JPEGDEC_STRM_PROCESSED\n");
		break;
	case JPEGDEC_OK:
		OSprintf("JPEGDEC_OK\n");
		break;
	case JPEGDEC_ERROR:
		OSprintf("JPEGDEC_ERROR\n");
		break;
	case JPEGDEC_UNSUPPORTED:
		OSprintf("JPEGDEC_UNSUPPORTED\n");
		break;
	case JPEGDEC_PARAM_ERROR:
		OSprintf("JPEGDEC_PARAM_ERROR\n");
		break;
	case JPEGDEC_MEMFAIL:
		OSprintf("JPEGDEC_MEMFAIL\n");
		break;
	case JPEGDEC_INITFAIL:
		OSprintf("JPEGDEC_INITFAIL\n");
		break;
	case JPEGDEC_INVALID_STREAM_LENGTH:
		OSprintf("JPEGDEC_INVALID_STREAM_LENGTH\n");
		break;
	case JPEGDEC_STRM_ERROR:
		OSprintf("JPEGDEC_STRM_ERROR\n");
		break;
	case JPEGDEC_INVALID_INPUT_BUFFER_SIZE:
		OSprintf("JPEGDEC_INVALID_INPUT_BUFFER_SIZE\n");
		break;
	case JPEGDEC_HW_RESERVED:
		OSprintf("JPEGDEC_HW_RESERVED\n");
		break;
	case JPEGDEC_INCREASE_INPUT_BUFFER:
		OSprintf("JPEGDEC_INCREASE_INPUT_BUFFER\n");
		break;
	case JPEGDEC_DWL_HW_TIMEOUT:
		OSprintf("JPEGDEC_DWL_HW_TIMEOUT\n");
		break;
	case JPEGDEC_DWL_ERROR:
		OSprintf("JPEGDEC_DWL_ERROR\n");
		break;
	case JPEGDEC_HW_BUS_ERROR:
		OSprintf("JPEGDEC_HW_BUS_ERROR\n");
		break;
	case JPEGDEC_SYSTEM_ERROR:
		OSprintf("JPEGDEC_SYSTEM_ERROR\n");
		break;
	default:
		OSprintf("Other %d\n", ret);
		break;
	}
}

static 
void ppRet(PPResult ret)
{
	OSprintf("PP result: ");
	switch (ret)
	{
	case PP_PARAM_ERROR:
		OSprintf("PP_PARAM_ERROR\n");
		break;
	case PP_MEMFAIL:
		OSprintf("PP_MEMFAIL\n");
		break;
	case PP_SET_IN_SIZE_INVALID:
		OSprintf("PP_SET_IN_SIZE_INVALID\n");
		break;
	case PP_SET_IN_ADDRESS_INVALID:
		OSprintf("PP_SET_IN_ADDRESS_INVALID\n");
		break;
	case PP_SET_IN_FORMAT_INVALID:
		OSprintf("PP_SET_IN_FORMAT_INVALID\n");
		break;
	case PP_SET_CROP_INVALID:
		OSprintf("PP_SET_CROP_INVALID\n");
		break;
	case PP_SET_ROTATION_INVALID:
		OSprintf("PP_SET_ROTATION_INVALID\n");
		break;
	case PP_SET_OUT_SIZE_INVALID:
		OSprintf("PP_SET_OUT_SIZE_INVALID\n");
		break;
	case PP_SET_OUT_ADDRESS_INVALID:
		OSprintf("PP_SET_OUT_ADDRESS_INVALID\n");
		break;
	case PP_SET_OUT_FORMAT_INVALID:
		OSprintf("PP_SET_OUT_FORMAT_INVALID\n");
		break;
	case PP_SET_VIDEO_ADJUST_INVALID:
		OSprintf("PP_SET_VIDEO_ADJUST_INVALID\n");
		break;
	case PP_SET_RGB_BITMASK_INVALID:
		OSprintf("PP_SET_RGB_BITMASK_INVALID\n");
		break;
	case PP_SET_FRAMEBUFFER_INVALID:
		OSprintf("PP_SET_FRAMEBUFFER_INVALID\n");
		break;
	case PP_SET_MASK1_INVALID:
		OSprintf("PP_SET_MASK1_INVALID\n");
		break;
	case PP_SET_MASK2_INVALID:
		OSprintf("PP_SET_MASK2_INVALID\n");
		break;
	case PP_SET_SLICE_HEIGHT_INVALID:
		OSprintf("PP_SET_SLICE_HEIGHT_INVALID\n");
		break;
	case PP_SET_SLICE_AND_PIPELINE:
		OSprintf("PP_SET_SLICE_AND_PIPELINE\n");
		break;
	case PP_BUSY:
		OSprintf("PP_BUSY\n");
		break;
	case PP_HW_BUS_ERROR:
		OSprintf("PP_HW_BUS_ERROR\n");
		break;
	case PP_HW_TIMEOUT:
		OSprintf("PP_HW_TIMEOUT\n");
		break;
	case PP_DWL_ERROR:
		OSprintf("PP_DWL_ERROR\n");
		break;
	case PP_SYSTEM_ERROR:
		OSprintf("PP_SYSTEM_ERROR\n");
		break;
	case PP_DEC_PIPELINE_ERROR:
		OSprintf("PP_DEC_PIPELINE_ERROR\n");
		break;
	default:
		OSprintf("Other %d\n", ret);
		break;
	}
}

static void print_pp_cfg(PPConfig *pp_cfg)
{
	OSprintf("\tppInImg.width:%d\n",						pp_cfg->ppInImg.width);
	OSprintf("\tppInImg.height:%d\n",						pp_cfg->ppInImg.height);
	OSprintf("\tppInImg.pixFormat:0x%08x, ",				pp_cfg->ppInImg.pixFormat);
	switch(pp_cfg->ppInImg.pixFormat)
	{
	case PP_PIX_FMT_YCBCR_4_0_0:
		OSprintf("YCBCR_4_0_0\n");
		break;
	case PP_PIX_FMT_YCBCR_4_4_4_SEMIPLANAR:
		OSprintf("YCBCR_4_4_4_SEMIPLANAR\n");
		break;
	case PP_PIX_FMT_YCBCR_4_2_2_INTERLEAVED:
		OSprintf("YCBCR_4_2_2_INTERLEAVED\n");
		break;
	case PP_PIX_FMT_YCBCR_4_2_2_SEMIPLANAR:
		OSprintf("YCBCR_4_2_2_SEMIPLANAR\n");
		break;
	case PP_PIX_FMT_YCBCR_4_4_0_SEMIPLANAR:
		OSprintf("YCBCR_4_4_0_SEMIPLANAR\n");
		break;
	case PP_PIX_FMT_YCBCR_4_2_0_PLANAR:
		OSprintf("YCBCR_4_2_0_PLANAR\n");
		break;
	case PP_PIX_FMT_YCBCR_4_2_0_SEMIPLANAR:
		OSprintf("YCBCR_4_2_0_SEMIPLANAR\n");
		break;
	case PP_PIX_FMT_RGB16_5_5_5:
		OSprintf("RGB16_5_5_5\n");
		break;
	case PP_PIX_FMT_RGB16_5_6_5:
		OSprintf("RGB16_5_6_5\n");
		break;
	case PP_PIX_FMT_RGB32:
		OSprintf("RGB32\n");
		break;
	default:
		OSprintf("unsupport!\n");
	}
	OSprintf("\tppInImg.videoRange:%d\n",					pp_cfg->ppInImg.videoRange);
	OSprintf("\tppInImg.bufferBusAddr:0x%08X\n",			pp_cfg->ppInImg.bufferBusAddr);
	OSprintf("\tppInImg.bufferCbBusAddr:0x%08X\n",			pp_cfg->ppInImg.bufferCbBusAddr);
	OSprintf("\tppInImg.bufferCrBusAddr:0x%08X\n",			pp_cfg->ppInImg.bufferCrBusAddr);
	OSprintf("\tppInImg.vc1MultiResEnable:%d\n",			pp_cfg->ppInImg.vc1MultiResEnable);
	OSprintf("\tppInImg.vc1RangeRedFrm:%d\n",				pp_cfg->ppInImg.vc1RangeRedFrm);
	OSprintf("\tppInImg.deinterlaceEnable:%d\n",			pp_cfg->ppInImg.deinterlaceEnable);
	OSprintf("\tppInImg.deinterlceThreshold:%d\n",			pp_cfg->ppInImg.deinterlceThreshold);
	OSprintf("\tppInImg.sliceMode:%d\n",					pp_cfg->ppInImg.sliceMode);
	OSprintf("\tppInImg.sliceHeight:%d\n",					pp_cfg->ppInImg.sliceHeight);
	OSprintf("\tppInCrop.enable:%d\n",						pp_cfg->ppInCrop.enable);
	OSprintf("\tppInCrop.originX:%d\n",						pp_cfg->ppInCrop.originX);
	OSprintf("\tppInCrop.originY:%d\n",						pp_cfg->ppInCrop.originY);
	OSprintf("\tppInCrop.width:%d\n",						pp_cfg->ppInCrop.width);
	OSprintf("\tppInCrop.height:%d\n",						pp_cfg->ppInCrop.height);
	OSprintf("\tppInRotation.rotation:%d, ",				pp_cfg->ppInRotation.rotation);
	switch(pp_cfg->ppInRotation.rotation)
	{
	case PP_ROTATION_NONE:
		OSprintf("(PP_ROTATION_NONE)\n");
		break;
	case PP_ROTATION_RIGHT_90:
		OSprintf("(PP_ROTATION_RIGHT_90)\n");
		break;
	case PP_ROTATION_LEFT_90:
		OSprintf("(PP_ROTATION_LEFT_90)\n");
		break;
	case PP_ROTATION_HOR_FLIP:
		OSprintf("(PP_ROTATION_HOR_FLIP)\n");
		break;
	case PP_ROTATION_VER_FLIP:
		OSprintf("(PP_ROTATION_VER_FLIP)\n");
		break;
	case PP_ROTATION_180:
		OSprintf("(PP_ROTATION_180)\n");
		break;
	}
	OSprintf("\tppOutImg.width:%d\n",						pp_cfg->ppOutImg.width);
	OSprintf("\tppOutImg.height:%d\n",						pp_cfg->ppOutImg.height);
	OSprintf("\tppOutImg.pixFormat:0x%08x, ",				pp_cfg->ppOutImg.pixFormat);
	switch(pp_cfg->ppOutImg.pixFormat)
	{
	case PP_PIX_FMT_YCBCR_4_2_2_INTERLEAVED:
		OSprintf("YCBCR_4_2_2_INTERLEAVED\n");
		break;
	case PP_PIX_FMT_YCBCR_4_2_0_SEMIPLANAR:
		OSprintf("YCBCR_4_2_0_SEMIPLANAR\n");
		break;
	case PP_PIX_FMT_RGB16_CUSTOM:
		OSprintf("RGB16_CUSTOM\n");
		break;
	case PP_PIX_FMT_RGB16_5_5_5:
		OSprintf("RGB16_5_5_5\n");
		break;
	case PP_PIX_FMT_RGB16_5_6_5:
		OSprintf("RGB16_5_6_5\n");
		break;
	case PP_PIX_FMT_BGR16_5_5_5:
		OSprintf("BGR16_5_5_5\n");
		break;
	case PP_PIX_FMT_BGR16_5_6_5:
		OSprintf("BGR16_5_6_5\n");
		break;
	case PP_PIX_FMT_RGB32_CUSTOM:
		OSprintf("RGB32_CUSTOM\n");
		break;
	case PP_PIX_FMT_RGB32:
		OSprintf("RGB32\n");
		break;
	case PP_PIX_FMT_BGR32:
		OSprintf("BGR32\n");
		break;
	default:
		OSprintf("unsupport!\n");
	}
	OSprintf("\tppOutImg.bufferBusAddr:0x%08X\n",			pp_cfg->ppOutImg.bufferBusAddr);
	OSprintf("\tppOutImg.bufferChromaBusAddr:0x%08X\n",		pp_cfg->ppOutImg.bufferChromaBusAddr);
	OSprintf("\tppOutImg.maskAlphaColorDepth:%d\n",			pp_cfg->ppOutImg.maskAlphaColorDepth);
	OSprintf("\tppOutRgb.rgbTransform:%d\n",				pp_cfg->ppOutRgb.rgbTransform);
	OSprintf("\tppOutRgb.brightness:%d\n",					pp_cfg->ppOutRgb.brightness);
	OSprintf("\tppOutRgb.contrast:%d\n",					pp_cfg->ppOutRgb.contrast);
	OSprintf("\tppOutRgb.saturation:%d\n",					pp_cfg->ppOutRgb.saturation);
	OSprintf("\tppOutRgb.alpha:%d\n",						pp_cfg->ppOutRgb.alpha);
	OSprintf("\tppOutRgb.transparency:%d\n",				pp_cfg->ppOutRgb.transparency);
	OSprintf("\tppOutMask1.enable:%d\n",					pp_cfg->ppOutMask1.enable);
	OSprintf("\tppOutMask1.originX:%d\n",					pp_cfg->ppOutMask1.originX);
	OSprintf("\tppOutMask1.originY:%d\n",					pp_cfg->ppOutMask1.originY);
	OSprintf("\tppOutMask1.width:%d\n",						pp_cfg->ppOutMask1.width);
	OSprintf("\tppOutMask1.height:%d\n",					pp_cfg->ppOutMask1.height);
	OSprintf("\tppOutMask1.alphaBlendEna:%d\n",				pp_cfg->ppOutMask1.alphaBlendEna);
	OSprintf("\tppOutMask1.blendComponentBase:0x%08X\n",	pp_cfg->ppOutMask1.blendComponentBase);
	OSprintf("\tppOutMask1.fixedAlpha:%d\n",				pp_cfg->ppOutMask1.fixedAlpha);
	OSprintf("\tppOutMask1.alpha:%d\n",						pp_cfg->ppOutMask1.alpha);
	OSprintf("\tppOutMask2.enable:%d\n",					pp_cfg->ppOutMask2.enable);
	OSprintf("\tppOutMask2.originX:%d\n",					pp_cfg->ppOutMask2.originX);
	OSprintf("\tppOutMask2.originY:%d\n", 					pp_cfg->ppOutMask2.originY);
	OSprintf("\tppOutMask2.width:%d\n",						pp_cfg->ppOutMask2.width);
	OSprintf("\tppOutMask2.height:%d\n",					pp_cfg->ppOutMask2.height);
	OSprintf("\tppOutMask2.alphaBlendEna:%d\n",				pp_cfg->ppOutMask2.alphaBlendEna);
	OSprintf("\tppOutMask2.blendComponentBase:0x%08X\n",	pp_cfg->ppOutMask2.blendComponentBase);
	OSprintf("\tppOutMask2.fixedAlpha:%d\n",				pp_cfg->ppOutMask2.fixedAlpha);
	OSprintf("\tppOutMask2.alpha:%d\n",						pp_cfg->ppOutMask2.alpha);
	OSprintf("\tppOutFrmBuffer.enable:%d\n",				pp_cfg->ppOutFrmBuffer.enable);
	OSprintf("\tppOutFrmBuffer.writeOriginX:%d\n",			pp_cfg->ppOutFrmBuffer.writeOriginX);
	OSprintf("\tppOutFrmBuffer.writeOriginY:%d\n",			pp_cfg->ppOutFrmBuffer.writeOriginY);
	OSprintf("\tppOutFrmBuffer.frameBufferWidth:%d\n",		pp_cfg->ppOutFrmBuffer.frameBufferWidth);
	OSprintf("\tppOutFrmBuffer.frameBufferHeight:%d\n",		pp_cfg->ppOutFrmBuffer.frameBufferHeight);
}

/* software scale */

static 
void *_memcpy_(void *d, const void *s, long n)
{
	unsigned char *dst = (unsigned char*)d;
	unsigned char *src = (unsigned char*)s;

	if(((unsigned long)dst&3) == ((unsigned long)src&3))
	{
		while(((unsigned long)dst&3) != 0 && n > 0)
		{
			*dst++ = *src++;
			n--;
		}
		while(n >= 16)
		{
			unsigned long t0, t1, t2, t3;

			t0 = ((unsigned long*)src)[0];
			t1 = ((unsigned long*)src)[1];
			t2 = ((unsigned long*)src)[2];
			t3 = ((unsigned long*)src)[3];
			((unsigned long*)dst)[0] = t0;
			((unsigned long*)dst)[1] = t1;
			((unsigned long*)dst)[2] = t2;
			((unsigned long*)dst)[3] = t3;
			n -= 16;
			src += 16;
			dst += 16;
		}
		while(n >= 4)
		{
			*(unsigned long*)dst = *(unsigned long*)src;
			n -= 4;
			src += 4;
			dst += 4;
		}
	}
	while(n > 0)
	{
		n--;
		*dst++ = *src++;
	}

	return d;
}

static 
int IMG_ZOOM_RGB565_TO_RGB565(unsigned char *src,	//source image buffer address
							  long src_pix_stride,	//source image stride in pixel
							  long src_width,		//source image width in pixel
							  long src_height,		//source image height in pixel
							  unsigned char *dst,	//dst image buffer address
							  long dst_pix_stride,	//dst image stride in pixel
							  long zoom_width,		//zoom width
							  long zoom_height)		//zoom height
{
	long dx, dy;	//dst x and y

	if(src_width == zoom_width && src_height == zoom_height)
	{
		if(src != dst)
		{
			unsigned char *pls = src;
			unsigned char *pld = dst;
			for(dy=0;dy<=zoom_height-1;dy++)
			{
				_memcpy_(pld, pls, zoom_width*2);
				pls += src_pix_stride*2;
				pld += dst_pix_stride*2;
			}
		}
		return 0;
	}
	else
	{
		unsigned long scale_h, scale_v;
		long tx, ty;	//temp x and y
		unsigned char *line_buffer0_mem = NULL;	//source line buffer 0
		unsigned char *line_buffer1_mem = NULL;	//source line buffer 1
		unsigned char *line_buffer2_mem = NULL;	//dst line buffer
		unsigned char *line_buf0;
		unsigned char *line_buf1;
		unsigned char *line_buf2;
		unsigned char *line_buft;
		unsigned char *pld;
		long line_y0;
		long line_y1;

		line_buffer0_mem = (unsigned char*)DWLmalloc(src_width*2+4);
		if(!line_buffer0_mem)
			goto end;
		if((unsigned long)line_buffer0_mem&0x03)
			line_buf0 = line_buffer0_mem-((unsigned long)line_buffer0_mem&0x03)+4;
		else
			line_buf0 = line_buffer0_mem;

		line_buffer1_mem = (unsigned char*)DWLmalloc(src_width*2+4);
		if(!line_buffer1_mem)
			goto end;
		if((unsigned long)line_buffer1_mem&0x03)
			line_buf1 = line_buffer1_mem-((unsigned long)line_buffer1_mem&0x03)+4;
		else
			line_buf1 = line_buffer1_mem;

		line_buffer2_mem = (unsigned char*)DWLmalloc(zoom_width*2+4);
		if(!line_buffer2_mem)
			goto end;
		if((unsigned long)line_buffer2_mem&0x03)
			line_buf2 = line_buffer2_mem-((unsigned long)line_buffer2_mem&0x03)+4;
		else
			line_buf2 = line_buffer2_mem;

		if(src_width == zoom_width)
		{
			scale_v = ((src_height-1)<<16)/(zoom_height-1);
			if(!(((src_height-1)<<16)%(zoom_height-1)))
				scale_v--;
			if(src_height < zoom_height)
			{
				line_y0 = src_height;
				line_y1 = src_height+1;
				pld = dst+(zoom_height-1)*dst_pix_stride*2;
				ty = scale_v*(zoom_height-1);
				for(dy=zoom_height-1;dy>=0;dy--)
				{
					long sy = ty>>16;		//src y
					long hy = ty-(sy<<16);	//half pixel x
					unsigned char *pls0 = src + sy*src_pix_stride*2;
					unsigned char *pls1 = pls0 + src_pix_stride*2;
					unsigned char *d = line_buf2;
					if(line_y0 > sy+1)
					{
						_memcpy_(line_buf0, pls0, src_width*2);
						_memcpy_(line_buf1, pls1, src_width*2);
					}
					else if(line_y0 == sy+1)
					{
						line_buft = line_buf0;
						line_buf0 = line_buf1;
						line_buf1 = line_buft;
							_memcpy_(line_buf0, pls0, src_width*2);
					}
					line_y0 = sy;
					line_y1 = sy+1;
					pls0 = line_buf0;
					pls1 = line_buf1;

					for(dx=0;dx<=zoom_width-1;dx++)
					{
						unsigned short t0, t1;
						long ta, tc, r, g, b;
						t0 = *(unsigned short*)pls0;
						t1 = *(unsigned short*)pls1;
						ta = t0>>11;
						tc = t1>>11;
						r = ta+(((tc-ta)*hy)>>16);
						ta = t0>>5;
						tc = t1>>5;
						ta &= 0x3f;
						tc &= 0x3f;
						g = ta+(((tc-ta)*hy)>>16);
						ta = t0&0x1f;
						tc = t1&0x1f;
						b = ta+(((tc-ta)*hy)>>16);
						*(unsigned short*)d = (unsigned short)((r<<11)|(g<<5)|b);
						pls0 += 2;
						pls1 += 2;
						d += 2;
					}
					ty -= scale_v;
					_memcpy_(pld, line_buf2, zoom_width*2);
					pld -= dst_pix_stride*2;
				}
			}
			else //if(src_height > zoom_height)
			{
				line_y0 = -2;
				line_y1 = -1;
				pld = dst;
				ty = 0;
				for(dy=0;dy<=zoom_height-1;dy++)
				{
					long sy = ty>>16;		//src y
					long hy = ty-(sy<<16);	//half pixel x
					unsigned char *pls0 = src + sy*src_pix_stride*2;
					unsigned char *pls1 = pls0 + src_pix_stride*2;
					unsigned char *d = line_buf2;
					if(line_y1 < sy)
					{
						_memcpy_(line_buf0, pls0, src_width*2);
						_memcpy_(line_buf1, pls1, src_width*2);
					}
					else if(line_y1 == sy)
					{
						line_buft = line_buf0;
						line_buf0 = line_buf1;
						line_buf1 = line_buft;
							_memcpy_(line_buf1, pls1, src_width*2);
					}
					line_y0 = sy;
					line_y1 = sy+1;
					pls0 = line_buf0;
					pls1 = line_buf1;

					for(dx=0;dx<=zoom_width-1;dx++)
					{
						unsigned short t0, t1;
						long ta, tc, r, g, b;
						t0 = *(unsigned short*)pls0;
						t1 = *(unsigned short*)pls1;
						ta = t0>>11;
						tc = t1>>11;
						r = ta+(((tc-ta)*hy)>>16);
						ta = t0>>5;
						tc = t1>>5;
						ta &= 0x3f;
						tc &= 0x3f;
						g = ta+(((tc-ta)*hy)>>16);
						ta = t0&0x1f;
						tc = t1&0x1f;
						b = ta+(((tc-ta)*hy)>>16);
						*(unsigned short*)d = (unsigned short)((r<<11)|(g<<5)|b);
						pls0 += 2;
						pls1 += 2;
						d += 2;
					}
					ty += scale_v;
					_memcpy_(pld, line_buf2, zoom_width*2);
					pld += dst_pix_stride*2;
				}
			}
		}
		else if(src_height == zoom_height)
		{
			unsigned char *pls = src;
			pld = dst;
			scale_h = ((src_width-1)<<16)/(zoom_width-1);
			if(!(((src_width-1)<<16)%(zoom_width-1)))
				scale_h--;
			for(dy=0;dy<=zoom_height-1;dy++)
			{
				unsigned char *d = line_buf2;
				_memcpy_(line_buf0, pls, src_width*2);
				tx = 0;
				for(dx=0;dx<=zoom_width-1;dx++)
				{
					long sx = tx>>16;		//src x
					long hx = tx-(sx<<16);	//half pixel x
					unsigned char *s = line_buf0 + sx*2;
					unsigned short t0, t1;
					long ta, tb, r, g, b;
					t0 = *(unsigned short*)s;
					t1 = *(unsigned short*)(s+2);
					ta = t0>>11;
					tb = t1>>11;
					r = ta+(((tb-ta)*hx)>>16);
					ta = t0>>5;
					tb = t1>>5;
					ta &= 0x3f;
					tb &= 0x3f;
					g = ta+(((tb-ta)*hx)>>16);
					ta = t0&0x1f;
					tb = t1&0x1f;
					b = ta+(((tb-ta)*hx)>>16);
					*(unsigned short*)d = (unsigned short)((r<<11)|(g<<5)|b);
					d += 2;
					tx += scale_h;
				}
				_memcpy_(pld, line_buf2, zoom_width*2);
				pls += src_pix_stride*2;
				pld += dst_pix_stride*2;
			}
		}
		else if(src_height < zoom_height)
		{
			scale_h = ((src_width-1)<<16)/(zoom_width-1);
			if(!(((src_width-1)<<16)%(zoom_width-1)))
				scale_h--;
			scale_v = ((src_height-1)<<16)/(zoom_height-1);
			if(!(((src_height-1)<<16)%(zoom_height-1)))
				scale_v--;

			line_y0 = src_height;
			line_y1 = src_height+1;
			pld = dst+(zoom_height-1)*dst_pix_stride*2;
			ty = scale_v*(zoom_height-1);
			for(dy=zoom_height-1;dy>=0;dy--)
			{
				long sy = ty>>16;		//src y
				long hy = ty-(sy<<16);	//half pixel x
				unsigned char *pls0 = src + sy*src_pix_stride*2;
				unsigned char *pls1 = pls0 + src_pix_stride*2;
				unsigned char *d = line_buf2;
				if(line_y0 > sy+1)
				{
					_memcpy_(line_buf0, pls0, src_width*2);
					_memcpy_(line_buf1, pls1, src_width*2);
				}
				else if(line_y0 == sy+1)
				{
					line_buft = line_buf0;
					line_buf0 = line_buf1;
					line_buf1 = line_buft;
						_memcpy_(line_buf0, pls0, src_width*2);
				}
				line_y0 = sy;
				line_y1 = sy+1;
				pls0 = line_buf0;
				pls1 = line_buf1;
				tx = 0;
				for(dx=0;dx<=zoom_width-1;dx++)
				{
					long sx = tx>>16;		//src x
					long hx = tx-(sx<<16);	//half pixel x
					unsigned char *s0 = pls0 + sx*2;
					unsigned char *s1 = pls1 + sx*2;
					unsigned short t0, t1, t2, t3;
					long ta, tb, tc, td, r, g, b;
					t0 = *(unsigned short*)s0;
					t1 = *(unsigned short*)(s0+2);
					t2 = *(unsigned short*)s1;
					t3 = *(unsigned short*)(s1+2);
					ta = t0>>11;
					tb = t1>>11;
					tc = t2>>11;
					td = t3>>11;
					ta = ta+(((tb-ta)*hx)>>16);
					tc = tc+(((td-tc)*hx)>>16);
					r = ta+(((tc-ta)*hy)>>16);
					ta = t0>>5;
					tb = t1>>5;
					tc = t2>>5;
					td = t3>>5;
					ta &= 0x3f;
					tb &= 0x3f;
					tc &= 0x3f;
					td &= 0x3f;
					ta = ta+(((tb-ta)*hx)>>16);
					tc = tc+(((td-tc)*hx)>>16);
					g = ta+(((tc-ta)*hy)>>16);
					ta = t0&0x1f;
					tb = t1&0x1f;
					tc = t2&0x1f;
					td = t3&0x1f;
					ta = ta+(((tb-ta)*hx)>>16);
					tc = tc+(((td-tc)*hx)>>16);
					b = ta+(((tc-ta)*hy)>>16);
					*(unsigned short*)d = (unsigned short)((r<<11)|(g<<5)|b);
					d += 2;
					tx += scale_h;
				}
				ty -= scale_v;
				_memcpy_(pld, line_buf2, zoom_width*2);
				pld -= dst_pix_stride*2;
			}
		}
		else //if(src_height > zoom_height)
		{
			//Y
			scale_h = ((src_width-1)<<16)/(zoom_width-1);
			if(!(((src_width-1)<<16)%(zoom_width-1)))
				scale_h--;
			scale_v = ((src_height-1)<<16)/(zoom_height-1);
			if(!(((src_height-1)<<16)%(zoom_height-1)))
				scale_v--;

			line_y0 = -2;
			line_y1 = -1;
			pld = dst;
			ty = 0;
			for(dy=0;dy<=zoom_height-1;dy++)
			{
				long sy = ty>>16;		//src y
				long hy = ty-(sy<<16);	//half pixel x
				unsigned char *pls0 = src + sy*src_pix_stride*2;
				unsigned char *pls1 = pls0 + src_pix_stride*2;
				unsigned char *d = line_buf2;
				if(line_y1 < sy)
				{
					_memcpy_(line_buf0, pls0, src_width*2);
					_memcpy_(line_buf1, pls1, src_width*2);
				}
				else if(line_y1 == sy)
				{
					line_buft = line_buf0;
					line_buf0 = line_buf1;
					line_buf1 = line_buft;
						_memcpy_(line_buf1, pls1, src_width*2);
				}
				line_y0 = sy;
				line_y1 = sy+1;
				pls0 = line_buf0;
				pls1 = line_buf1;
				tx = 0;
				for(dx=0;dx<=zoom_width-1;dx++)
				{
					long sx = tx>>16;		//src x
					long hx = tx-(sx<<16);	//half pixel x
					unsigned char *s0 = pls0 + sx*2;
					unsigned char *s1 = pls1 + sx*2;
					unsigned short t0, t1, t2, t3;
					long ta, tb, tc, td, r, g, b;
					t0 = *(unsigned short*)s0;
					t1 = *(unsigned short*)(s0+2);
					t2 = *(unsigned short*)s1;
					t3 = *(unsigned short*)(s1+2);
					ta = t0>>11;
					tb = t1>>11;
					tc = t2>>11;
					td = t3>>11;
					ta = ta+(((tb-ta)*hx)>>16);
					tc = tc+(((td-tc)*hx)>>16);
					r = ta+(((tc-ta)*hy)>>16);
					ta = t0>>5;
					tb = t1>>5;
					tc = t2>>5;
					td = t3>>5;
					ta &= 0x3f;
					tb &= 0x3f;
					tc &= 0x3f;
					td &= 0x3f;
					ta = ta+(((tb-ta)*hx)>>16);
					tc = tc+(((td-tc)*hx)>>16);
					g = ta+(((tc-ta)*hy)>>16);
					ta = t0&0x1f;
					tb = t1&0x1f;
					tc = t2&0x1f;
					td = t3&0x1f;
					ta = ta+(((tb-ta)*hx)>>16);
					tc = tc+(((td-tc)*hx)>>16);
					b = ta+(((tc-ta)*hy)>>16);
					*(unsigned short*)d = (unsigned short)((r<<11)|(g<<5)|b);
					d += 2;
					tx += scale_h;
				}
				ty += scale_v;
				_memcpy_(pld, line_buf2, zoom_width*2);
				pld += dst_pix_stride*2;
			}
		}
end:
		if(line_buffer0_mem)
			DWLfree(line_buffer0_mem);
		if(line_buffer1_mem)
			DWLfree(line_buffer1_mem);
		if(line_buffer2_mem)
			DWLfree(line_buffer2_mem);
	}

	return 0;
}

static 
int IMG_ZOOM_RGB32_TO_RGB32(unsigned char *src,	//source image buffer address
							long src_pix_stride,//source image stride in pixel
							long src_width,		//source image width in pixel
							long src_height,	//source image height in pixel
							unsigned char *dst,	//dst image buffer address
							long dst_pix_stride,//dst image stride in pixel
							long zoom_width,	//zoom width
							long zoom_height)	//zoom height
{
	long dx, dy;	//dst x and y

	if(src_width == zoom_width && src_height == zoom_height)
	{
		if(src != dst)
		{
			unsigned char *pls = src;
			unsigned char *pld = dst;
			for(dy=0;dy<=zoom_height-1;dy++)
			{
				_memcpy_(pld, pls, zoom_width*4);
				pls += src_pix_stride*4;
				pld += dst_pix_stride*4;
			}
		}
		return 0;
	}
	else
	{
		unsigned long scale_h, scale_v;
		long tx, ty;	//temp x and y
		unsigned char *line_buffer0_mem = NULL;	//source line buffer 0
		unsigned char *line_buffer1_mem = NULL;	//source line buffer 1
		unsigned char *line_buffer2_mem = NULL;	//dst line buffer
		unsigned char *line_buf0;
		unsigned char *line_buf1;
		unsigned char *line_buf2;
		unsigned char *line_buft;
		unsigned char *pld;
		long line_y0;
		long line_y1;

		line_buffer0_mem = (unsigned char*)DWLmalloc(src_width*4+4);
		if(!line_buffer0_mem)
			goto end;
		if((unsigned long)line_buffer0_mem&0x03)
			line_buf0 = line_buffer0_mem-((unsigned long)line_buffer0_mem&0x03)+4;
		else
			line_buf0 = line_buffer0_mem;

		line_buffer1_mem = (unsigned char*)DWLmalloc(src_width*4+4);
		if(!line_buffer1_mem)
			goto end;
		if((unsigned long)line_buffer1_mem&0x03)
			line_buf1 = line_buffer1_mem-((unsigned long)line_buffer1_mem&0x03)+4;
		else
			line_buf1 = line_buffer1_mem;

		line_buffer2_mem = (unsigned char*)DWLmalloc(zoom_width*4+4);
		if(!line_buffer2_mem)
			goto end;
		if((unsigned long)line_buffer2_mem&0x03)
			line_buf2 = line_buffer2_mem-((unsigned long)line_buffer2_mem&0x03)+4;
		else
			line_buf2 = line_buffer2_mem;

		if(src_width == zoom_width)
		{
			scale_v = ((src_height-1)<<16)/(zoom_height-1);
			if(!(((src_height-1)<<16)%(zoom_height-1)))
				scale_v--;
			if(src_height < zoom_height)
			{
				line_y0 = src_height;
				line_y1 = src_height+1;
				pld = dst+(zoom_height-1)*dst_pix_stride*4;
				ty = scale_v*(zoom_height-1);
				for(dy=zoom_height-1;dy>=0;dy--)
				{
					long sy = ty>>16;		//src y
					long hy = ty-(sy<<16);	//half pixel x
					unsigned char *pls0 = src + sy*src_pix_stride*4;
					unsigned char *pls1 = pls0 + src_pix_stride*4;
					unsigned char *d = line_buf2;
					if(line_y0 > sy+1)
					{
						_memcpy_(line_buf0, pls0, src_width*4);
						_memcpy_(line_buf1, pls1, src_width*4);
					}
					else if(line_y0 == sy+1)
					{
						line_buft = line_buf0;
						line_buf0 = line_buf1;
						line_buf1 = line_buft;
							_memcpy_(line_buf0, pls0, src_width*4);
					}
					line_y0 = sy;
					line_y1 = sy+1;
					pls0 = line_buf0;
					pls1 = line_buf1;
					for(dx=0;dx<=zoom_width-1;dx++)
					{
						long ta, tc;
						ta = pls0[0];
						tc = pls1[0];
						d[0] = (unsigned char)(ta+(((tc-ta)*hy)>>16));
						ta = pls0[1];
						tc = pls1[1];
						d[1] = (unsigned char)(ta+(((tc-ta)*hy)>>16));
						ta = pls0[2];
						tc = pls1[2];
						d[2] = (unsigned char)(ta+(((tc-ta)*hy)>>16));
						ta = pls0[3];
						tc = pls1[3];
						d[3] = (unsigned char)(ta+(((tc-ta)*hy)>>16));
						pls0 += 4;
						pls1 += 4;
						d += 4;
					}
					ty -= scale_v;
					_memcpy_(pld, line_buf2, zoom_width*4);
					pld -= dst_pix_stride*4;
				}
			}
			else //if(src_height > zoom_height)
			{
				line_y0 = -2;
				line_y1 = -1;
				pld = dst;
				ty = 0;
				for(dy=0;dy<=zoom_height-1;dy++)
				{
					long sy = ty>>16;		//src y
					long hy = ty-(sy<<16);	//half pixel x
					unsigned char *pls0 = src + sy*src_pix_stride*4;
					unsigned char *pls1 = pls0 + src_pix_stride*4;
					unsigned char *d = line_buf2;
					if(line_y1 < sy)
					{
						_memcpy_(line_buf0, pls0, src_width*4);
						_memcpy_(line_buf1, pls1, src_width*4);
					}
					else if(line_y1 == sy)
					{
						line_buft = line_buf0;
						line_buf0 = line_buf1;
						line_buf1 = line_buft;
							_memcpy_(line_buf1, pls1, src_width*4);
					}
					line_y0 = sy;
					line_y1 = sy+1;
					pls0 = line_buf0;
					pls1 = line_buf1;
					for(dx=0;dx<=zoom_width-1;dx++)
					{
						long ta, tc;
						ta = pls0[0];
						tc = pls1[0];
						d[0] = (unsigned char)(ta+(((tc-ta)*hy)>>16));
						ta = pls0[1];
						tc = pls1[1];
						d[1] = (unsigned char)(ta+(((tc-ta)*hy)>>16));
						ta = pls0[2];
						tc = pls1[2];
						d[2] = (unsigned char)(ta+(((tc-ta)*hy)>>16));
						ta = pls0[3];
						tc = pls1[3];
						d[3] = (unsigned char)(ta+(((tc-ta)*hy)>>16));
						pls0 += 4;
						pls1 += 4;
						d += 4;
					}
					ty += scale_v;
					_memcpy_(pld, line_buf2, zoom_width*4);
					pld += dst_pix_stride*4;
				}
			}
		}
		else if(src_height == zoom_height)
		{
			unsigned char *pls = src;
			pld = dst;
			scale_h = ((src_width-1)<<16)/(zoom_width-1);
			if(!(((src_width-1)<<16)%(zoom_width-1)))
				scale_h--;
			for(dy=0;dy<=zoom_height-1;dy++)
			{
				unsigned char *d = line_buf2;
				_memcpy_(line_buf0, pls, src_width*4);
				tx = 0;
				for(dx=0;dx<=zoom_width-1;dx++)
				{
					long sx = tx>>16;		//src x
					long hx = tx-(sx<<16);	//half pixel x
					unsigned char *s = line_buf0 + sx*4;
					long ta, tb;
					ta = s[0];
					tb = s[4];
					d[0] = (unsigned char)(ta+(((tb-ta)*hx)>>16));
					ta = s[1];
					tb = s[5];
					d[1] = (unsigned char)(ta+(((tb-ta)*hx)>>16));
					ta = s[2];
					tb = s[6];
					d[2] = (unsigned char)(ta+(((tb-ta)*hx)>>16));
					ta = s[3];
					tb = s[7];
					d[3] = (unsigned char)(ta+(((tb-ta)*hx)>>16));
					d += 4;
					tx += scale_h;
				}
				_memcpy_(pld, line_buf2, zoom_width*4);
				pls += src_pix_stride*4;
				pld += dst_pix_stride*4;
			}
		}
		else if(src_height < zoom_height)
		{
			scale_h = ((src_width-1)<<16)/(zoom_width-1);
			if(!(((src_width-1)<<16)%(zoom_width-1)))
				scale_h--;
			scale_v = ((src_height-1)<<16)/(zoom_height-1);
			if(!(((src_height-1)<<16)%(zoom_height-1)))
				scale_v--;

			line_y0 = src_height;
			line_y1 = src_height+1;
			pld = dst+(zoom_height-1)*dst_pix_stride*4;
			ty = scale_v*(zoom_height-1);
			for(dy=zoom_height-1;dy>=0;dy--)
			{
				long sy = ty>>16;		//src y
				long hy = ty-(sy<<16);	//half pixel x
				unsigned char *pls0 = src + sy*src_pix_stride*4;
				unsigned char *pls1 = pls0 + src_pix_stride*4;
				unsigned char *d = line_buf2;
				if(line_y0 > sy+1)
				{
					_memcpy_(line_buf0, pls0, src_width*4);
					_memcpy_(line_buf1, pls1, src_width*4);
				}
				else if(line_y0 == sy+1)
				{
					line_buft = line_buf0;
					line_buf0 = line_buf1;
					line_buf1 = line_buft;
						_memcpy_(line_buf0, pls0, src_width*4);
				}
				line_y0 = sy;
				line_y1 = sy+1;
				pls0 = line_buf0;
				pls1 = line_buf1;
				tx = 0;
				for(dx=0;dx<=zoom_width-1;dx++)
				{
					long sx = tx>>16;		//src x
					long hx = tx-(sx<<16);	//half pixel x
					unsigned char *s0 = pls0 + sx*4;
					unsigned char *s1 = pls1 + sx*4;
					long ta, tb, tc, td;
					ta = s0[0];
					tb = s0[4];
					tc = s1[0];
					td = s1[4];
					ta = ta+(((tb-ta)*hx)>>16);
					tc = tc+(((td-tc)*hx)>>16);
					d[0] = (unsigned char)(ta+(((tc-ta)*hy)>>16));
					ta = s0[1];
					tb = s0[5];
					tc = s1[1];
					td = s1[5];
					ta = ta+(((tb-ta)*hx)>>16);
					tc = tc+(((td-tc)*hx)>>16);
					d[1] = (unsigned char)(ta+(((tc-ta)*hy)>>16));
					ta = s0[2];
					tb = s0[6];
					tc = s1[2];
					td = s1[6];
					ta = ta+(((tb-ta)*hx)>>16);
					tc = tc+(((td-tc)*hx)>>16);
					d[2] = (unsigned char)(ta+(((tc-ta)*hy)>>16));
					ta = s0[3];
					tb = s0[7];
					tc = s1[3];
					td = s1[7];
					ta = ta+(((tb-ta)*hx)>>16);
					tc = tc+(((td-tc)*hx)>>16);
					d[3] = (unsigned char)(ta+(((tc-ta)*hy)>>16));
					d += 4;
					tx += scale_h;
				}
				ty -= scale_v;
				_memcpy_(pld, line_buf2, zoom_width*4);
				pld -= dst_pix_stride*4;
			}
		}
		else //if(src_height > zoom_height)
		{
			//Y
			scale_h = ((src_width-1)<<16)/(zoom_width-1);
			if(!(((src_width-1)<<16)%(zoom_width-1)))
				scale_h--;
			scale_v = ((src_height-1)<<16)/(zoom_height-1);
			if(!(((src_height-1)<<16)%(zoom_height-1)))
				scale_v--;

			line_y0 = -2;
			line_y1 = -1;
			pld = dst;
			ty = 0;
			for(dy=0;dy<=zoom_height-1;dy++)
			{
				long sy = ty>>16;		//src y
				long hy = ty-(sy<<16);	//half pixel x
				unsigned char *pls0 = src + sy*src_pix_stride*4;
				unsigned char *pls1 = pls0 + src_pix_stride*4;
				unsigned char *d = line_buf2;
				if(line_y1 < sy)
				{
					_memcpy_(line_buf0, pls0, src_width*4);
					_memcpy_(line_buf1, pls1, src_width*4);
				}
				else if(line_y1 == sy)
				{
					line_buft = line_buf0;
					line_buf0 = line_buf1;
					line_buf1 = line_buft;
						_memcpy_(line_buf1, pls1, src_width*4);
				}
				line_y0 = sy;
				line_y1 = sy+1;
				pls0 = line_buf0;
				pls1 = line_buf1;
				tx = 0;
				for(dx=0;dx<=zoom_width-1;dx++)
				{
					long sx = tx>>16;		//src x
					long hx = tx-(sx<<16);	//half pixel x
					unsigned char *s0 = pls0 + sx*4;
					unsigned char *s1 = pls1 + sx*4;
					long ta, tb, tc, td;
					ta = s0[0];
					tb = s0[4];
					tc = s1[0];
					td = s1[4];
					ta = ta+(((tb-ta)*hx)>>16);
					tc = tc+(((td-tc)*hx)>>16);
					d[0] = (unsigned char)(ta+(((tc-ta)*hy)>>16));
					ta = s0[1];
					tb = s0[5];
					tc = s1[1];
					td = s1[5];
					ta = ta+(((tb-ta)*hx)>>16);
					tc = tc+(((td-tc)*hx)>>16);
					d[1] = (unsigned char)(ta+(((tc-ta)*hy)>>16));
					ta = s0[2];
					tb = s0[6];
					tc = s1[2];
					td = s1[6];
					ta = ta+(((tb-ta)*hx)>>16);
					tc = tc+(((td-tc)*hx)>>16);
					d[2] = (unsigned char)(ta+(((tc-ta)*hy)>>16));
					ta = s0[3];
					tb = s0[7];
					tc = s1[3];
					td = s1[7];
					ta = ta+(((tb-ta)*hx)>>16);
					tc = tc+(((td-tc)*hx)>>16);
					d[3] = (unsigned char)(ta+(((tc-ta)*hy)>>16));
					d += 4;
					tx += scale_h;
				}
				ty += scale_v;
				_memcpy_(pld, line_buf2, zoom_width*4);
				pld += dst_pix_stride*4;
			}
		}
end:
		if(line_buffer0_mem)
			DWLfree(line_buffer0_mem);
		if(line_buffer1_mem)
			DWLfree(line_buffer1_mem);
		if(line_buffer2_mem)
			DWLfree(line_buffer2_mem);
	}

	return 0;
}

//static 
int IMG_ZOOM_YUV420SEMIPLANAR_TO_YUV420SEMIPLANAR(unsigned char *src_y,	//source image buffer address, Y
												  unsigned char *src_uv,//source image buffer address, CbCr
												  long src_pix_stride,	//source image stride in pixel
												  long src_width,		//source image width in pixel
												  long src_height,		//source image height in pixel
												  unsigned char *dst_y,	//dst image buffer address, Y
												  unsigned char *dst_uv,//dst image buffer address, CbCr
												  long dst_pix_stride,	//dst image stride in pixel
												  long zoom_width,		//zoom width
												  long zoom_height)		//zoom height
{
	long dx, dy;	//dst x and y

	zoom_width -= zoom_width&1;
	zoom_height -= zoom_height&1;

	if(src_width == zoom_width && src_height == zoom_height)
	{
		if(src_y != dst_y)
		{
			unsigned char *pls = src_y;
			unsigned char *pld = dst_y;
			for(dy=0;dy<=zoom_height-1;dy++)
			{
				_memcpy_(pld, pls, zoom_width);
				pls += src_pix_stride;
				pld += dst_pix_stride;
			}
		}
		if(src_uv != dst_uv)
		{
			unsigned char *pls = src_uv;
			unsigned char *pld = dst_uv;
			for(dy=0;dy<=(zoom_height/2)-1;dy++)
			{
				_memcpy_(pld, pls, zoom_width);
				pls += src_pix_stride;
				pld += dst_pix_stride;
			}
		}
		return 0;
	}
	else
	{
		unsigned long scale_h, scale_v;
		long tx, ty;	//temp x and y
		unsigned char *line_buffer0_mem = NULL;	//source line buffer 0
		unsigned char *line_buffer1_mem = NULL;	//source line buffer 1
		unsigned char *line_buffer2_mem = NULL;	//dst line buffer
		unsigned char *line_buf0;
		unsigned char *line_buf1;
		unsigned char *line_buf2;
		unsigned char *line_buft;
		unsigned char *pld;
		long line_y0;
		long line_y1;

		line_buffer0_mem = (unsigned char*)DWLmalloc(src_width+4);
		if(!line_buffer0_mem)
			goto end;
		if((unsigned long)line_buffer0_mem&0x03)
			line_buf0 = line_buffer0_mem-((unsigned long)line_buffer0_mem&0x03)+4;
		else
			line_buf0 = line_buffer0_mem;

		line_buffer1_mem = (unsigned char*)DWLmalloc(src_width+4);
		if(!line_buffer1_mem)
			goto end;
		if((unsigned long)line_buffer1_mem&0x03)
			line_buf1 = line_buffer1_mem-((unsigned long)line_buffer1_mem&0x03)+4;
		else
			line_buf1 = line_buffer1_mem;

		line_buffer2_mem = (unsigned char*)DWLmalloc(zoom_width+4);
		if(!line_buffer2_mem)
			goto end;
		if((unsigned long)line_buffer2_mem&0x03)
			line_buf2 = line_buffer2_mem-((unsigned long)line_buffer2_mem&0x03)+4;
		else
			line_buf2 = line_buffer2_mem;

		if(src_width == zoom_width)
		{
			if(src_height < zoom_height)
			{
				//Y
				scale_v = ((src_height-1)<<16)/(zoom_height-1);
				if(!(((src_height-1)<<16)%(zoom_height-1)))
					scale_v--;

				line_y0 = src_height;
				line_y1 = src_height+1;
				pld = dst_y+(zoom_height-1)*dst_pix_stride;
				ty = scale_v*(zoom_height-1);
				for(dy=zoom_height-1;dy>=0;dy--)
				{
					long sy = ty>>16;		//src y
					long hy = ty-(sy<<16);	//half pixel y
					unsigned char *pls0 = src_y + sy*src_pix_stride;
					unsigned char *pls1 = pls0 + src_pix_stride;
					unsigned char *d = line_buf2;
					if(line_y0 > sy+1)
					{
						_memcpy_(line_buf0, pls0, src_width);
						_memcpy_(line_buf1, pls1, src_width);
					}
					else if(line_y0 == sy+1)
					{
						line_buft = line_buf0;
						line_buf0 = line_buf1;
						line_buf1 = line_buft;
							_memcpy_(line_buf0, pls0, src_width);
					}
					line_y0 = sy;
					line_y1 = sy+1;
					pls0 = line_buf0;
					pls1 = line_buf1;

					for(dx=0;dx<=zoom_width-1;dx++)
					{
						long ta, tc;
						ta = *pls0++;
						tc = *pls1++;
						*d++ = (unsigned char)(ta+(((tc-ta)*hy)>>16));
					}
					ty -= scale_v;
					_memcpy_(pld, line_buf2, zoom_width);
					pld -= dst_pix_stride;
				}
				
				//UV
				src_width /= 2;
				src_height /= 2;
				zoom_width /= 2;
				zoom_height /=2;

				scale_v = ((src_height-1)<<16)/(zoom_height-1);
				if(!(((src_height-1)<<16)%(zoom_height-1)))
					scale_v--;

				line_y0 = src_height;
				line_y1 = src_height+1;
				pld = dst_uv+(zoom_height-1)*dst_pix_stride;
				ty = scale_v*(zoom_height-1);
				for(dy=zoom_height-1;dy>=0;dy--)
				{
					long sy = ty>>16;		//src y
					long hy = ty-(sy<<16);	//half pixel y
					unsigned char *pls0 = src_uv + sy*src_pix_stride;
					unsigned char *pls1 = pls0 + src_pix_stride;
					unsigned char *d = line_buf2;
					if(line_y0 > sy+1)
					{
						_memcpy_(line_buf0, pls0, src_width*2);
						_memcpy_(line_buf1, pls1, src_width*2);
					}
					else if(line_y0 == sy+1)
					{
						line_buft = line_buf0;
						line_buf0 = line_buf1;
						line_buf1 = line_buft;
							_memcpy_(line_buf0, pls0, src_width*2);
					}
					line_y0 = sy;
					line_y1 = sy+1;
					pls0 = line_buf0;
					pls1 = line_buf1;

					for(dx=0;dx<=zoom_width-1;dx++)
					{
						long ta, tc;
						//U
						ta = pls0[0];
						tc = pls1[0];
						d[0] = (unsigned char)(ta+(((tc-ta)*hy)>>16));
						//V
						ta = pls0[1];
						tc = pls1[1];
						d[1] = (unsigned char)(ta+(((tc-ta)*hy)>>16));
						pls0 += 2;
						pls1 += 2;
						d += 2;
					}
					ty -= scale_v;
					_memcpy_(pld, line_buf2, zoom_width*2);
					pld -= dst_pix_stride;
				}
			}
			else //if(src_height > zoom_height)
			{
				//Y
				scale_v = ((src_height-1)<<16)/(zoom_height-1);
				if(!(((src_height-1)<<16)%(zoom_height-1)))
					scale_v--;

				line_y0 = -2;
				line_y1 = -1;
				pld = dst_y;
				ty = 0;
				for(dy=0;dy<=zoom_height-1;dy++)
				{
					long sy = ty>>16;		//src y
					long hy = ty-(sy<<16);	//half pixel y
					unsigned char *pls0 = src_y + sy*src_pix_stride;
					unsigned char *pls1 = pls0 + src_pix_stride;
					unsigned char *d = line_buf2;
					if(line_y1 < sy)
					{
						_memcpy_(line_buf0, pls0, src_width);
						_memcpy_(line_buf1, pls1, src_width);
					}
					else if(line_y1 == sy)
					{
						line_buft = line_buf0;
						line_buf0 = line_buf1;
						line_buf1 = line_buft;
						_memcpy_(line_buf1, pls1, src_width);
					}
					line_y0 = sy;
					line_y1 = sy+1;
					pls0 = line_buf0;
					pls1 = line_buf1;

					for(dx=0;dx<=zoom_width-1;dx++)
					{
						long ta, tc;
						ta = *pls0++;
						tc = *pls1++;
						*d++ = (unsigned char)(ta+(((tc-ta)*hy)>>16));
					}
					ty += scale_v;
					_memcpy_(pld, line_buf2, zoom_width);
					pld += dst_pix_stride;
				}
				
				//UV
				src_width /= 2;
				src_height /= 2;
				zoom_width /= 2;
				zoom_height /=2;

				scale_v = ((src_height-1)<<16)/(zoom_height-1);
				if(!(((src_height-1)<<16)%(zoom_height-1)))
					scale_v--;

				line_y0 = -2;
				line_y1 = -1;
				pld = dst_uv;
				ty = 0;
				for(dy=0;dy<=zoom_height-1;dy++)
				{
					long sy = ty>>16;		//src y
					long hy = ty-(sy<<16);	//half pixel y
					unsigned char *pls0 = src_uv + sy*src_pix_stride;
					unsigned char *pls1 = pls0 + src_pix_stride;
					unsigned char *d = line_buf2;
					if(line_y1 < sy)
					{
						_memcpy_(line_buf0, pls0, src_width*2);
						_memcpy_(line_buf1, pls1, src_width*2);
					}
					else if(line_y1 == sy)
					{
						line_buft = line_buf0;
						line_buf0 = line_buf1;
						line_buf1 = line_buft;
						_memcpy_(line_buf1, pls1, src_width*2);
					}
					line_y0 = sy;
					line_y1 = sy+1;
					pls0 = line_buf0;
					pls1 = line_buf1;

					for(dx=0;dx<=zoom_width-1;dx++)
					{
						long ta, tc;
						//U
						ta = pls0[0];
						tc = pls1[0];
						d[0] = (unsigned char)(ta+(((tc-ta)*hy)>>16));
						//V
						ta = pls0[1];
						tc = pls1[1];
						d[1] = (unsigned char)(ta+(((tc-ta)*hy)>>16));
						pls0 += 2;
						pls1 += 2;
						d += 2;
					}
					ty += scale_v;
					_memcpy_(pld, line_buf2, zoom_width*2);
					pld += dst_pix_stride;
				}
			}
		}
		else if(src_height == zoom_height)
		{
			unsigned char *pls = src_y;
			pld = dst_y;
			//Y
			scale_h = ((src_width-1)<<16)/(zoom_width-1);
			if(!(((src_width-1)<<16)%(zoom_width-1)))
				scale_h--;
			for(dy=0;dy<=zoom_height-1;dy++)
			{
				unsigned char *d = line_buf2;
				_memcpy_(line_buf0, pls, src_width);
				tx = 0;
				for(dx=0;dx<=zoom_width-1;dx++)
				{
					long sx = tx>>16;		//src x
					long hx = tx-(sx<<16);	//half pixel x
					unsigned char *s = line_buf0 + sx;

					long ta, tb;

					ta = s[0];
					tb = s[1];
					*d++ = (unsigned char)(ta+(((tb-ta)*hx)>>16));
					tx += scale_h;
				}
				_memcpy_(pld, line_buf2, zoom_width);
				pls += src_pix_stride;
				pld += dst_pix_stride;
			}

			//UV
			src_width /= 2;
			src_height /= 2;
			zoom_width /= 2;
			zoom_height /=2;

			scale_h = ((src_width-1)<<16)/(zoom_width-1);
			if(!(((src_width-1)<<16)%(zoom_width-1)))
				scale_h--;

			pls = src_uv;
			pld = dst_uv;
			for(dy=0;dy<=zoom_height-1;dy++)
			{
				unsigned char *d = line_buf2;
				_memcpy_(line_buf0, pls, src_width*2);
				tx = 0;
				for(dx=0;dx<=zoom_width-1;dx++)
				{
					long sx = tx>>16;		//src x
					long hx = tx-(sx<<16);	//half pixel x
					unsigned char *s = line_buf0 + sx*2;
					long ta, tb;
					//U
					ta = s[0];
					tb = s[2];
					d[0] = (unsigned char)(ta+(((tb-ta)*hx)>>16));
					//V
					ta = s[1];
					tb = s[3];
					d[1] = (unsigned char)(ta+(((tb-ta)*hx)>>16));
					d += 2;
					tx += scale_h;
				}
				_memcpy_(pld, line_buf2, zoom_width*2);
				pls += src_pix_stride;
				pld += dst_pix_stride;
			}
		}
		else if(src_height < zoom_height)
		{
			//Y
			scale_h = ((src_width-1)<<16)/(zoom_width-1);
			if(!(((src_width-1)<<16)%(zoom_width-1)))
				scale_h--;
			scale_v = ((src_height-1)<<16)/(zoom_height-1);
			if(!(((src_height-1)<<16)%(zoom_height-1)))
				scale_v--;

			line_y0 = src_height;
			line_y1 = src_height+1;
			pld = dst_y+(zoom_height-1)*dst_pix_stride;
			ty = scale_v*(zoom_height-1);
			for(dy=zoom_height-1;dy>=0;dy--)
			{
				long sy = ty>>16;		//src y
				long hy = ty-(sy<<16);	//half pixel y
				unsigned char *pls0 = src_y + sy*src_pix_stride;
				unsigned char *pls1 = pls0 + src_pix_stride;
				unsigned char *d = line_buf2;
				if(line_y0 > sy+1)
				{
					_memcpy_(line_buf0, pls0, src_width);
					_memcpy_(line_buf1, pls1, src_width);
				}
				else if(line_y0 == sy+1)
				{
					line_buft = line_buf0;
					line_buf0 = line_buf1;
					line_buf1 = line_buft;
						_memcpy_(line_buf0, pls0, src_width);
				}
				line_y0 = sy;
				line_y1 = sy+1;
				pls0 = line_buf0;
				pls1 = line_buf1;
				tx = 0;
				for(dx=0;dx<=zoom_width-1;dx++)
				{
					long sx = tx>>16;		//src x
					long hx = tx-(sx<<16);	//half pixel x
					unsigned char *s0 = pls0 + sx;
					unsigned char *s1 = pls1 + sx;

					long ta, tb, tc, td;

					ta = s0[0];
					tb = s0[1];
					tc = s1[0];
					td = s1[1];
					ta = ta+(((tb-ta)*hx)>>16);
					tc = tc+(((td-tc)*hx)>>16);
					*d++ = (unsigned char)(ta+(((tc-ta)*hy)>>16));
					tx += scale_h;
				}
				ty -= scale_v;
				_memcpy_(pld, line_buf2, zoom_width);
				pld -= dst_pix_stride;
			}
			
			//UV
			src_width /= 2;
			src_height /= 2;
			zoom_width /= 2;
			zoom_height /=2;

			scale_h = ((src_width-1)<<16)/(zoom_width-1);
			if(!(((src_width-1)<<16)%(zoom_width-1)))
				scale_h--;
			scale_v = ((src_height-1)<<16)/(zoom_height-1);
			if(!(((src_height-1)<<16)%(zoom_height-1)))
				scale_v--;

			line_y0 = src_height;
			line_y1 = src_height+1;
			pld = dst_uv+(zoom_height-1)*dst_pix_stride;
			ty = scale_v*(zoom_height-1);
			for(dy=zoom_height-1;dy>=0;dy--)
			{
				long sy = ty>>16;		//src y
				long hy = ty-(sy<<16);	//half pixel y
				unsigned char *pls0 = src_uv + sy*src_pix_stride;
				unsigned char *pls1 = pls0 + src_pix_stride;
				unsigned char *d = line_buf2;
				if(line_y0 > sy+1)
				{
					_memcpy_(line_buf0, pls0, src_width*2);
					_memcpy_(line_buf1, pls1, src_width*2);
				}
				else if(line_y0 == sy+1)
				{
					line_buft = line_buf0;
					line_buf0 = line_buf1;
					line_buf1 = line_buft;
						_memcpy_(line_buf0, pls0, src_width*2);
				}
				line_y0 = sy;
				line_y1 = sy+1;
				pls0 = line_buf0;
				pls1 = line_buf1;

				tx = 0;
				for(dx=0;dx<=zoom_width-1;dx++)
				{
					long sx = tx>>16;		//src x
					long hx = tx-(sx<<16);	//half pixel x
					unsigned char *s0 = pls0 + sx*2;
					unsigned char *s1 = pls1 + sx*2;

					long ta, tb, tc, td;

					//U
					ta = s0[0];
					tb = s0[2];
					tc = s1[0];
					td = s1[2];
					ta = ta+(((tb-ta)*hx)>>16);
					tc = tc+(((td-tc)*hx)>>16);
					*d++ = (unsigned char)(ta+(((tc-ta)*hy)>>16));
					//V
					ta = s0[1];
					tb = s0[3];
					tc = s1[1];
					td = s1[3];
					ta = ta+(((tb-ta)*hx)>>16);
					tc = tc+(((td-tc)*hx)>>16);
					*d++ = (unsigned char)(ta+(((tc-ta)*hy)>>16));

					tx += scale_h;
				}
				ty -= scale_v;
				_memcpy_(pld, line_buf2, zoom_width*2);
				pld -= dst_pix_stride;
			}
		}
		else //if(src_height > zoom_height)
		{
			//Y
			scale_h = ((src_width-1)<<16)/(zoom_width-1);
			if(!(((src_width-1)<<16)%(zoom_width-1)))
				scale_h--;
			scale_v = ((src_height-1)<<16)/(zoom_height-1);
			if(!(((src_height-1)<<16)%(zoom_height-1)))
				scale_v--;

			line_y0 = -2;
			line_y1 = -1;
			pld = dst_y;
			ty = 0;
			for(dy=0;dy<=zoom_height-1;dy++)
			{
				long sy = ty>>16;		//src y
				long hy = ty-(sy<<16);	//half pixel y
				unsigned char *pls0 = src_y + sy*src_pix_stride;
				unsigned char *pls1 = pls0 + src_pix_stride;
				unsigned char *d = line_buf2;
				if(line_y1 < sy)
				{
					_memcpy_(line_buf0, pls0, src_width);
					_memcpy_(line_buf1, pls1, src_width);
				}
				else if(line_y1 == sy)
				{
					line_buft = line_buf0;
					line_buf0 = line_buf1;
					line_buf1 = line_buft;
					_memcpy_(line_buf1, pls1, src_width);
				}
				line_y0 = sy;
				line_y1 = sy+1;
				pls0 = line_buf0;
				pls1 = line_buf1;

				tx = 0;
				for(dx=0;dx<=zoom_width-1;dx++)
				{
					long sx = tx>>16;		//src x
					long hx = tx-(sx<<16);	//half pixel x
					unsigned char *s0 = pls0 + sx;
					unsigned char *s1 = pls1 + sx;

					long ta, tb, tc, td;

					ta = s0[0];
					tb = s0[1];
					tc = s1[0];
					td = s1[1];
					ta = ta+(((tb-ta)*hx)>>16);
					tc = tc+(((td-tc)*hx)>>16);
					*d++ = (unsigned char)(ta+(((tc-ta)*hy)>>16));
					tx += scale_h;
				}
				ty += scale_v;
				_memcpy_(pld, line_buf2, zoom_width);
				pld += dst_pix_stride;
			}
			
			//UV
			src_width /= 2;
			src_height /= 2;
			zoom_width /= 2;
			zoom_height /=2;

			scale_h = ((src_width-1)<<16)/(zoom_width-1);
			if(!(((src_width-1)<<16)%(zoom_width-1)))
				scale_h--;
			scale_v = ((src_height-1)<<16)/(zoom_height-1);
			if(!(((src_height-1)<<16)%(zoom_height-1)))
				scale_v--;

			line_y0 = -2;
			line_y1 = -1;
			pld = dst_uv;
			ty = 0;
			for(dy=0;dy<=zoom_height-1;dy++)
			{
				long sy = ty>>16;		//src y
				long hy = ty-(sy<<16);	//half pixel y
				unsigned char *pls0 = src_uv + sy*src_pix_stride;
				unsigned char *pls1 = pls0 + src_pix_stride;
				unsigned char *d = line_buf2;
				if(line_y1 < sy)
				{
					_memcpy_(line_buf0, pls0, src_width*2);
					_memcpy_(line_buf1, pls1, src_width*2);
				}
				else if(line_y1 == sy)
				{
					line_buft = line_buf0;
					line_buf0 = line_buf1;
					line_buf1 = line_buft;
					_memcpy_(line_buf1, pls1, src_width*2);
				}
				line_y0 = sy;
				line_y1 = sy+1;
				pls0 = line_buf0;
				pls1 = line_buf1;

				tx = 0;
				for(dx=0;dx<=zoom_width-1;dx++)
				{
					long sx = tx>>16;		//src x
					long hx = tx-(sx<<16);	//half pixel x
					unsigned char *s0 = pls0 + sx*2;
					unsigned char *s1 = pls1 + sx*2;

					long ta, tb, tc, td;

					//U
					ta = s0[0];
					tb = s0[2];
					tc = s1[0];
					td = s1[2];
					ta = ta+(((tb-ta)*hx)>>16);
					tc = tc+(((td-tc)*hx)>>16);
					*d++ = (unsigned char)(ta+(((tc-ta)*hy)>>16));
					//V
					ta = s0[1];
					tb = s0[3];
					tc = s1[1];
					td = s1[3];
					ta = ta+(((tb-ta)*hx)>>16);
					tc = tc+(((td-tc)*hx)>>16);
					*d++ = (unsigned char)(ta+(((tc-ta)*hy)>>16));

					tx += scale_h;
				}
				ty += scale_v;
				_memcpy_(pld, line_buf2, zoom_width*2);
				pld += dst_pix_stride;
			}
		}
end:
		if(line_buffer0_mem)
			DWLfree(line_buffer0_mem);
		if(line_buffer1_mem)
			DWLfree(line_buffer1_mem);
		if(line_buffer2_mem)
			DWLfree(line_buffer2_mem);
	}

	return 0;
}

static 
int IMG_ZOOM_YUV422INTERLEAVED_TO_YUV422INTERLEAVED(unsigned char *src,		//source image buffer address
													long src_pix_stride,	//source image stride in pixel
													long src_width,			//source image width in pixel
													long src_height,		//source image height in pixel
													unsigned char *dst,		//dst image buffer address
													long dst_pix_stride,	//dst image stride in pixel
													long zoom_width,		//zoom width
													long zoom_height)		//zoom height
{
	long dx, dy;	//dst x and y

	zoom_width -= zoom_width&1;

	if(src_width == zoom_width && src_height == zoom_height)
	{
		if(src != dst)
		{
			unsigned char *pls = src;
			unsigned char *pld = dst;
			for(dy=0;dy<=zoom_height-1;dy++)
			{
				_memcpy_(pld, pls, zoom_width*2);
				pls += src_pix_stride*2;
				pld += dst_pix_stride*2;
			}
		}
		return 0;
	}
	else
	{
		unsigned long scale_h, scale_v, scale_h_uv;
		long tx, ty, tx_uv;	//temp x and y
		unsigned char *line_buffer0_mem = NULL;	//source line buffer 0
		unsigned char *line_buffer1_mem = NULL;	//source line buffer 1
		unsigned char *line_buffer2_mem = NULL;	//dst line buffer
		unsigned char *line_buf0;
		unsigned char *line_buf1;
		unsigned char *line_buf2;
		unsigned char *line_buft;
		unsigned char *pld;
		long line_y0;
		long line_y1;

		line_buffer0_mem = (unsigned char*)DWLmalloc(src_width*2+4);
		if(!line_buffer0_mem)
			goto end;
		if((unsigned long)line_buffer0_mem&0x03)
			line_buf0 = line_buffer0_mem-((unsigned long)line_buffer0_mem&0x03)+4;
		else
			line_buf0 = line_buffer0_mem;

		line_buffer1_mem = (unsigned char*)DWLmalloc(src_width*2+4);
		if(!line_buffer1_mem)
			goto end;
		if((unsigned long)line_buffer1_mem&0x03)
			line_buf1 = line_buffer1_mem-((unsigned long)line_buffer1_mem&0x03)+4;
		else
			line_buf1 = line_buffer1_mem;

		line_buffer2_mem = (unsigned char*)DWLmalloc(zoom_width*2+4);
		if(!line_buffer2_mem)
			goto end;
		if((unsigned long)line_buffer2_mem&0x03)
			line_buf2 = line_buffer2_mem-((unsigned long)line_buffer2_mem&0x03)+4;
		else
			line_buf2 = line_buffer2_mem;

		if(src_width == zoom_width)
		{
			scale_v = ((src_height-1)<<16)/(zoom_height-1);
			if(!(((src_height-1)<<16)%(zoom_height-1)))
				scale_v--;
			if(src_height < zoom_height)
			{
				line_y0 = src_height;
				line_y1 = src_height+1;
				pld = dst+(zoom_height-1)*dst_pix_stride*2;
				ty = scale_v*(zoom_height-1);
				for(dy=zoom_height-1;dy>=0;dy--)
				{
					long sy = ty>>16;		//src y
					long hy = ty-(sy<<16);	//half pixel y
					unsigned char *pls0 = src + sy*src_pix_stride*2;
					unsigned char *pls1 = pls0 + src_pix_stride*2;
					unsigned char *d = line_buf2;
					if(line_y0 > sy+1)
					{
						_memcpy_(line_buf0, pls0, src_width*2);
						_memcpy_(line_buf1, pls1, src_width*2);
					}
					else if(line_y0 == sy+1)
					{
						line_buft = line_buf0;
						line_buf0 = line_buf1;
						line_buf1 = line_buft;
							_memcpy_(line_buf0, pls0, src_width*2);
					}
					line_y0 = sy;
					line_y1 = sy+1;
					pls0 = line_buf0;
					pls1 = line_buf1;

					for(dx=0;dx<=zoom_width-1;dx++)
					{
						long ta, tc;
						//Y
						ta = pls0[0];
						tc = pls1[0];
						d[0] = (unsigned char)(ta+(((tc-ta)*hy)>>16));
						//U or V
						ta = pls0[1];
						tc = pls1[1];
						d[1] = (unsigned char)(ta+(((tc-ta)*hy)>>16));
						pls0 += 2;
						pls1 += 2;
						d += 2;
					}
					ty -= scale_v;
					_memcpy_(pld, line_buf2, zoom_width*2);
					pld -= dst_pix_stride*2;
				}
			}
			else //if(src_height > zoom_height)
			{
				line_y0 = -2;
				line_y1 = -1;
				pld = dst;
				ty = 0;
				for(dy=0;dy<=zoom_height-1;dy++)
				{
					long sy = ty>>16;		//src y
					long hy = ty-(sy<<16);	//half pixel y
					unsigned char *pls0 = src + sy*src_pix_stride*2;
					unsigned char *pls1 = pls0 + src_pix_stride*2;
					unsigned char *d = line_buf2;
					if(line_y1 < sy)
					{
						_memcpy_(line_buf0, pls0, src_width*2);
						_memcpy_(line_buf1, pls1, src_width*2);
					}
					else if(line_y1 == sy)
					{
						line_buft = line_buf0;
						line_buf0 = line_buf1;
						line_buf1 = line_buft;
						_memcpy_(line_buf1, pls1, src_width*2);
					}
					line_y0 = sy;
					line_y1 = sy+1;
					pls0 = line_buf0;
					pls1 = line_buf1;

					for(dx=0;dx<=zoom_width-1;dx++)
					{
						long ta, tc;
						//Y
						ta = pls0[0];
						tc = pls1[0];
						d[0] = (unsigned char)(ta+(((tc-ta)*hy)>>16));
						//U or V
						ta = pls0[1];
						tc = pls1[1];
						d[1] = (unsigned char)(ta+(((tc-ta)*hy)>>16));
						pls0 += 2;
						pls1 += 2;
						d += 2;
					}
					ty += scale_v;
					_memcpy_(pld, line_buf2, zoom_width*2);
					pld += dst_pix_stride*2;
				}
			}
		}
		else if(src_height == zoom_height)
		{
			unsigned char *pls = src;
			pld = dst;
			scale_h = ((src_width-1)<<16)/(zoom_width-1);
			if(!(((src_width-1)<<16)%(zoom_width-1)))
				scale_h--;
			scale_h_uv = ((src_width/2-1)<<16)/(zoom_width/2-1);
			if(!(((src_width/2-1)<<16)%(zoom_width/2-1)))
				scale_h_uv--;
			for(dy=0;dy<=zoom_height-1;dy++)
			{
				unsigned char *d = line_buf2;
				_memcpy_(line_buf0, pls, src_width*2);
				tx = 0;
				tx_uv = 0;
				for(dx=0;dx<=zoom_width-1;dx+=2)
				{
					long sx = tx>>16;		//src x
					long hx = tx-(sx<<16);	//half pixel x
					unsigned char *s = line_buf0 + sx*2;
					long ta, tb;
					//Y0
					ta = s[0];
					tb = s[2];
					d[0] = (unsigned char)(ta+(((tb-ta)*hx)>>16));
					tx += scale_h;
					//Y1
					sx = tx>>16;		//src x
					hx = tx-(sx<<16);	//half pixel x
					s = line_buf0 + sx*2;
					ta = s[0];
					tb = s[2];
					d[2] = (unsigned char)(ta+(((tb-ta)*hx)>>16));
					tx += scale_h;
					//UV
					sx = tx_uv>>16;			//src x
					hx = tx_uv-(sx<<16);	//half pixel x
					s = line_buf0 + sx*4;
					//U
					ta = s[1];
					tb = s[5];
					d[1] = (unsigned char)(ta+(((tb-ta)*hx)>>16));
					//V
					ta = s[3];
					tb = s[7];
					d[3] = (unsigned char)(ta+(((tb-ta)*hx)>>16));
					tx_uv += scale_h_uv;
					d += 4;
				}
				_memcpy_(pld, line_buf2, zoom_width*2);
				pls += src_pix_stride*2;
				pld += dst_pix_stride*2;
			}
		}
		else if(src_height < zoom_height)
		{
			scale_h = ((src_width-1)<<16)/(zoom_width-1);
			if(!(((src_width-1)<<16)%(zoom_width-1)))
				scale_h--;
			scale_h_uv = ((src_width/2-1)<<16)/(zoom_width/2-1);
			if(!(((src_width/2-1)<<16)%(zoom_width/2-1)))
				scale_h_uv--;
			scale_v = ((src_height-1)<<16)/(zoom_height-1);
			if(!(((src_height-1)<<16)%(zoom_height-1)))
				scale_v--;

			line_y0 = src_height;
			line_y1 = src_height+1;
			pld = dst+(zoom_height-1)*dst_pix_stride*2;
			ty = scale_v*(zoom_height-1);
			for(dy=zoom_height-1;dy>=0;dy--)
			{
				long sy = ty>>16;		//src y
				long hy = ty-(sy<<16);	//half pixel y
				unsigned char *pls0 = src + sy*src_pix_stride*2;
				unsigned char *pls1 = pls0 + src_pix_stride*2;
				unsigned char *d = line_buf2;
				if(line_y0 > sy+1)
				{
					_memcpy_(line_buf0, pls0, src_width*2);
					_memcpy_(line_buf1, pls1, src_width*2);
				}
				else if(line_y0 == sy+1)
				{
					line_buft = line_buf0;
					line_buf0 = line_buf1;
					line_buf1 = line_buft;
						_memcpy_(line_buf0, pls0, src_width*2);
				}
				line_y0 = sy;
				line_y1 = sy+1;
				pls0 = line_buf0;
				pls1 = line_buf1;

				tx = 0;
				tx_uv = 0;
				for(dx=0;dx<=zoom_width-1;dx+=2)
				{
					long sx = tx>>16;		//src x
					long hx = tx-(sx<<16);	//half pixel x
					unsigned char *s0 = pls0 + sx*2;
					unsigned char *s1 = pls1 + sx*2;
					long ta, tb, tc, td;
					//Y0
					ta = s0[0];
					tb = s0[2];
					tc = s1[0];
					td = s1[2];
					ta = ta+(((tb-ta)*hx)>>16);
					tc = tc+(((td-tc)*hx)>>16);
					d[0] = (unsigned char)(ta+(((tc-ta)*hy)>>16));
					tx += scale_h;
					//Y1
					sx = tx>>16;		//src x
					hx = tx-(sx<<16);	//half pixel x
					s0 = pls0 + sx*2;
					s1 = pls1 + sx*2;
					ta = s0[0];
					tb = s0[2];
					tc = s1[0];
					td = s1[2];
					ta = ta+(((tb-ta)*hx)>>16);
					tc = tc+(((td-tc)*hx)>>16);
					d[2] = (unsigned char)(ta+(((tc-ta)*hy)>>16));
					tx += scale_h;
					//UV
					sx = tx_uv>>16;			//src x
					hx = tx_uv-(sx<<16);	//half pixel x
					s0 = pls0 + sx*4;
					s1 = pls1 + sx*4;
					//U
					ta = s0[1];
					tb = s0[5];
					tc = s1[1];
					td = s1[5];
					ta = ta+(((tb-ta)*hx)>>16);
					tc = tc+(((td-tc)*hx)>>16);
					d[1] = (unsigned char)(ta+(((tc-ta)*hy)>>16));
					//V
					ta = s0[3];
					tb = s0[7];
					tc = s1[3];
					td = s1[7];
					ta = ta+(((tb-ta)*hx)>>16);
					tc = tc+(((td-tc)*hx)>>16);
					d[3] = (unsigned char)(ta+(((tc-ta)*hy)>>16));
					tx_uv += scale_h_uv;
					d += 4;
				}
				ty -= scale_v;
				_memcpy_(pld, line_buf2, zoom_width*2);
				pld -= dst_pix_stride*2;
			}
		}
		else //if(src_height > zoom_height)
		{
			//Y
			scale_h = ((src_width-1)<<16)/(zoom_width-1);
			if(!(((src_width-1)<<16)%(zoom_width-1)))
				scale_h--;
			scale_h_uv = ((src_width/2-1)<<16)/(zoom_width/2-1);
			if(!(((src_width/2-1)<<16)%(zoom_width/2-1)))
				scale_h_uv--;
			scale_v = ((src_height-1)<<16)/(zoom_height-1);
			if(!(((src_height-1)<<16)%(zoom_height-1)))
				scale_v--;

			line_y0 = -2;
			line_y1 = -1;
			pld = dst;
			ty = 0;
			for(dy=0;dy<=zoom_height-1;dy++)
			{
				long sy = ty>>16;		//src y
				long hy = ty-(sy<<16);	//half pixel y
				unsigned char *pls0 = src + sy*src_pix_stride*2;
				unsigned char *pls1 = pls0 + src_pix_stride*2;
				unsigned char *d = line_buf2;
				if(line_y1 < sy)
				{
					_memcpy_(line_buf0, pls0, src_width*2);
					_memcpy_(line_buf1, pls1, src_width*2);
				}
				else if(line_y1 == sy)
				{
					line_buft = line_buf0;
					line_buf0 = line_buf1;
					line_buf1 = line_buft;
					_memcpy_(line_buf1, pls1, src_width*2);
				}
				line_y0 = sy;
				line_y1 = sy+1;
				pls0 = line_buf0;
				pls1 = line_buf1;

				tx = 0;
				tx_uv = 0;
				for(dx=0;dx<=zoom_width-1;dx+=2)
				{
					long sx = tx>>16;		//src x
					long hx = tx-(sx<<16);	//half pixel x
					unsigned char *s0 = pls0 + sx*2;
					unsigned char *s1 = pls1 + sx*2;
					long ta, tb, tc, td;
					//Y0
					ta = s0[0];
					tb = s0[2];
					tc = s1[0];
					td = s1[2];
					ta = ta+(((tb-ta)*hx)>>16);
					tc = tc+(((td-tc)*hx)>>16);
					d[0] = (unsigned char)(ta+(((tc-ta)*hy)>>16));
					tx += scale_h;
					//Y1
					sx = tx>>16;		//src x
					hx = tx-(sx<<16);	//half pixel x
					s0 = pls0 + sx*2;
					s1 = pls1 + sx*2;
					ta = s0[0];
					tb = s0[2];
					tc = s1[0];
					td = s1[2];
					ta = ta+(((tb-ta)*hx)>>16);
					tc = tc+(((td-tc)*hx)>>16);
					d[2] = (unsigned char)(ta+(((tc-ta)*hy)>>16));
					tx += scale_h;
					//UV
					sx = tx_uv>>16;			//src x
					hx = tx_uv-(sx<<16);	//half pixel x
					s0 = pls0 + sx*4;
					s1 = pls1 + sx*4;
					//U
					ta = s0[1];
					tb = s0[5];
					tc = s1[1];
					td = s1[5];
					ta = ta+(((tb-ta)*hx)>>16);
					tc = tc+(((td-tc)*hx)>>16);
					d[1] = (unsigned char)(ta+(((tc-ta)*hy)>>16));
					//V
					ta = s0[3];
					tb = s0[7];
					tc = s1[3];
					td = s1[7];
					ta = ta+(((tb-ta)*hx)>>16);
					tc = tc+(((td-tc)*hx)>>16);
					d[3] = (unsigned char)(ta+(((tc-ta)*hy)>>16));
					tx_uv += scale_h_uv;
					d += 4;
				}
				ty += scale_v;
				_memcpy_(pld, line_buf2, zoom_width*2);
				pld += dst_pix_stride*2;
			}
		}
end:
		if(line_buffer0_mem)
			DWLfree(line_buffer0_mem);
		if(line_buffer1_mem)
			DWLfree(line_buffer1_mem);
		if(line_buffer2_mem)
			DWLfree(line_buffer2_mem);
	}

	return 0;
}

static
int desquare(unsigned int size)
{
	int i;
	for(i=1;;i++)
	{
		if( size>=(i*i)  
			&& size<=(i+1)*(i+1))
			break;
	}

	if((i+1)*(i+1)==size)
		return (i+1);
	else
		return i;
}

static LP_RET load_baseline_jpg(LP_INPUT_PARAM *lp_param)
{
	void *dwl = NULL;
	JpegDecInst dec_inst = NULL;
	JpegDecInput jpeg_dec_input, *dec_input = &jpeg_dec_input;
	JpegDecOutput jpeg_dec_output, *dec_output = &jpeg_dec_output;
	JpegDecImageInfo jpeg_dec_info, *dec_info = &jpeg_dec_info;
	JpegDecInitParam_t dec_init_param, *dec_param = &dec_init_param;
	PPInitParam_t pp_init_param, *pp_param = &pp_init_param;
	//LP_SYS_BUF_INST load_photo_sys_buf, *sys_buf = &load_photo_sys_buf;
	LP_LINEAR_BUFFER sys_buf_hantro;
	JpegDecRet dec_ret;

	PPInst pp_inst = NULL;
	PPResult pp_ret;
	PPConfig pp_config_pipeline, *pp_cfg = &pp_config_pipeline;
	int pp_pipeline_flg = 0;

	LP_LINEAR_BUFFER hw_stream_buffer;
	LP_LINEAR_BUFFER hw_frame_buffer;
	int i = 0;
	int luma_size, buf_size, luma_size_tmp;
	int first_data_block = 1;
	int stream_total_length;
	int bytes_per_pixel = 2;
	int ret = 0;

	unsigned long jpeg_output_format;
	unsigned long jpeg_output_width;
	unsigned long jpeg_output_height;
	unsigned long jpeg_display_width;
	unsigned long jpeg_display_height;

    int yuv_420_x;
	int yuv_420_y;
	unsigned int yuv_420_width;
	unsigned int yuv_420_height;

	int yuv_422_x;
	int yuv_422_y;
	unsigned int yuv_422_width;
	unsigned int yuv_422_height;

	OSmemset(dec_input, 0, sizeof(JpegDecInput));
	OSmemset(dec_output, 0, sizeof(JpegDecOutput));
	OSmemset(dec_info, 0, sizeof(JpegDecImageInfo));
	OSmemset(dec_param, 0, sizeof(JpegDecInitParam_t));
	OSmemset(pp_param, 0, sizeof(PPInitParam_t));
	OSmemset(pp_cfg, 0, sizeof(PPConfig));
	OSmemset(&hw_stream_buffer, 0, sizeof(LP_LINEAR_BUFFER));
	OSmemset(&hw_frame_buffer, 0, sizeof(LP_LINEAR_BUFFER));
	//OSmemset(sys_buf, 0, sizeof(LP_SYS_BUF_INST));
	//sys_buf->fd = -1;
	OSmemset(&sys_buf_hantro, 0, sizeof(LP_LINEAR_BUFFER));

#if 0
	ret = init_sys_buf(sys_buf, O_RDWR);
	if (ret != 0)
	{
		OSprintf("load_baseline_jpg() error: init sys_buf error!\n");
		goto error;
	}
#endif

    // Heap size for hantro
	sys_buf_hantro.size = 64 * 1024;

#if 0
	ret = malloc_sys_buf(sys_buf, &sys_buf_hantro);
	if (ret != 0)
	{
		OSprintf("load_baseline_jpg() error: can not malloc sys_buf for hantro!\n");
		goto error;
	}
#endif

    sys_buf_hantro.buf = (unsigned char *)lp_param->lp_malloc(sys_buf_hantro.size);
    if (NULL == sys_buf_hantro.buf)
    {
		OSprintf("load_baseline_jpg() error: can not malloc buf for hantro!\n");
		goto error;
	}
	sys_buf_hantro.bus_addr = lp_param->lp_get_bus_addr((unsigned long)sys_buf_hantro.buf);/////////////////////////////////////////////////////;
    sys_buf_hantro.bus_addr &= 0x1fffffff;
	dec_param->pHeap = OSHCreate((void**)(&sys_buf_hantro.buf), (unsigned long*)(&sys_buf_hantro.bus_addr), (int*)(&sys_buf_hantro.size), 1, 16);
    if (NULL == dec_param->pHeap)
    {
		OSprintf("load_baseline_jpg() error: can not OSHCreate for hantro!\n");
		goto error;
	}

	dec_param->hw_reset_enable = 1;
	dec_param->mem_usage_trace = 0;
	dec_param->reg_rw_trace = 0;

	pp_param->pHeap = dec_param->pHeap;
	pp_param->hw_reset_enable = 1;
	pp_param->mem_usage_trace = 0;
	pp_param->reg_rw_trace = 0;

	if (lp_param->lp_fseek_end(lp_param->handle, 0))
	{
		OSprintf("load_baseline_jpg() error: seek_end error!\n");
		goto error;
	}
	stream_total_length = lp_param->lp_ftell(lp_param->handle);
	if (stream_total_length <= 0)
	{
		OSprintf("load_baseline_jpg() error: stream_total_length<=0!\n");
		goto error;
	}
	if (lp_param->lp_fseek_set(lp_param->handle, 0))
	{
		OSprintf("load_baseline_jpg() error: seek_set error!\n");
		goto error;
	}

	dec_ret = JpegDecInit(&dec_inst, dec_param);
	if (dec_ret != JPEGDEC_OK)
	{
		//JPEG_decRet(dec_ret);
		goto error;
	}

	dwl = JpegDecGetDWL(dec_inst);

	pp_ret = PPInit((PPInst *)(&pp_inst), pp_param);
	if (pp_ret != PP_OK)
	{
		//ppRet(pp_ret);
		goto error;
	}

	pp_ret = PPDecPipelineEnable(pp_inst, dec_inst, PP_PIPELINED_DEC_TYPE_JPEG);
	if (pp_ret != PP_OK)
	{
		//ppRet(pp_ret);
		goto error;
	}

	pp_pipeline_flg = 1;

	pp_ret = PPGetConfig(pp_inst, pp_cfg);
	if (pp_ret != PP_OK)
	{
		//ppRet(pp_ret);
		goto error;
	}

	hw_stream_buffer.size = LP_STREAM_BUF_SIZE;
#if 0
	ret = malloc_sys_buf(sys_buf, &hw_stream_buffer);
	if (ret != 0)
	{
		OSprintf("load_baseline_jpg() error: can not malloc sys_buf for stream_buf!\n");
		goto error;
	}
#endif

    hw_stream_buffer.buf = (unsigned char *)lp_param->lp_malloc(hw_stream_buffer.size);
    if (NULL == hw_stream_buffer.buf)
    {
		OSprintf("load_baseline_jpg() error: can not malloc buf for stream_buf!\n");
		goto error;
	}

    hw_stream_buffer.bus_addr = lp_param->lp_get_bus_addr((unsigned long)hw_stream_buffer.buf);////////////////////////////////////////;
    hw_stream_buffer.bus_addr &= 0x1fffffff;
	/* set pp config */
	pp_cfg->ppOutImg.width = ((lp_param->out_pic_width + 7) / 8) * 8;
	pp_cfg->ppOutImg.height = ((lp_param->out_pic_height + 1) / 2) * 2;
   
	dec_input->streamBuffer.pVirtualAddress = (u32*)hw_stream_buffer.buf;
	dec_input->streamBuffer.busAddress = hw_stream_buffer.bus_addr;
	dec_input->bufferSize = hw_stream_buffer.size;
	dec_input->streamLength = stream_total_length;


	for (; ;)
	{
		int read_size = lp_param->lp_fread(lp_param->handle, (unsigned char *)dec_input->streamBuffer.pVirtualAddress, dec_input->bufferSize);
		if (read_size < (int)dec_input->bufferSize)
		{
			dec_input->stream_end_flag = 1;
		}
		else if (lp_param->lp_ftell(lp_param->handle) == stream_total_length)
		{
			dec_input->stream_end_flag = 1;
		}
		if (first_data_block)
		{
			first_data_block = 0;
			dec_ret = JpegDecGetImageInfo(dec_inst, dec_input, dec_info);
			if (dec_ret != JPEGDEC_OK)
			{
				//JPEG_decRet(dec_ret);
				goto error;
			}
			jpeg_output_format = dec_info->outputFormat;
			jpeg_output_width = dec_info->outputWidth;
			jpeg_output_height = dec_info->outputHeight;
			jpeg_display_width = dec_info->displayWidth;
			jpeg_display_height = dec_info->displayHeight;
			dec_input->decImageType = JPEGDEC_IMAGE;
			dec_input->sliceMbSet = 1;

			switch (jpeg_output_format)
			{
			case JPEGDEC_YCbCr400:
				pp_cfg->ppInImg.pixFormat = PP_PIX_FMT_YCBCR_4_0_0;
		//		OSprintf("jpeg output format: YUV 4:0:0\n");
				break;
			case JPEGDEC_YCbCr420_SEMIPLANAR:
				pp_cfg->ppInImg.pixFormat = PP_PIX_FMT_YCBCR_4_2_0_SEMIPLANAR;
		//      OSprintf("jpeg output format: YUV 4:2:0\n");
				break;
		    case JPEGDEC_YCbCr422_SEMIPLANAR:
				pp_cfg->ppInImg.pixFormat = PP_PIX_FMT_YCBCR_4_2_2_SEMIPLANAR;
		//      OSprintf("jpeg output format: YUV 4:2:2\n");
				break;
		    case JPEGDEC_YCbCr440_SEMIPLANAR:
				pp_cfg->ppInImg.pixFormat = PP_PIX_FMT_YCBCR_4_4_0_SEMIPLANAR;
		//      OSprintf(""jpeg output format: YUV 4:4:0\n"");
				break;
		    case JPEGDEC_YCbCr444_SEMIPLANAR:
				pp_cfg->ppInImg.pixFormat = PP_PIX_FMT_YCBCR_4_4_4_SEMIPLANAR;
		//      OSprintf("jpeg output format: YUV 4:4:4\n");
		        break;
		    default:
				OSprintf("unknown jpeg output format:%d\n", jpeg_output_format);
		        goto error;
			}

			pp_cfg->ppInImg.width = jpeg_output_width;
			pp_cfg->ppInImg.height = jpeg_output_height;
			pp_cfg->ppInImg.videoRange = 1;

			if ((jpeg_output_width != jpeg_display_width) || (jpeg_output_height != jpeg_display_height))
			{
				pp_cfg->ppInCrop.enable = 1;
				pp_cfg->ppInCrop.originX = 0;
				pp_cfg->ppInCrop.originY = 0;
				pp_cfg->ppInCrop.width = jpeg_display_width;
				pp_cfg->ppInCrop.height = jpeg_display_height;
			}

			/* 放大3倍处理 */
			if (pp_cfg->ppOutImg.width > 3 * jpeg_display_width)
			{
				pp_cfg->ppOutImg.width = (3 * jpeg_display_width / 8) * 8;
			}
			if (pp_cfg->ppOutImg.height > 3 * jpeg_display_height - 2)
			{
				pp_cfg->ppOutImg.height = ((3 * jpeg_display_height - 2) / 2) * 2;
			}

			/* 一边放大, 一边缩小处理 */
			if ((pp_cfg->ppOutImg.width > jpeg_display_width) && (pp_cfg->ppOutImg.height < jpeg_display_height) )
			{
				pp_cfg->ppOutImg.height = ((jpeg_display_height + 1) / 2) * 2;
			}
			else if ((pp_cfg->ppOutImg.width < jpeg_display_width) && (pp_cfg->ppOutImg.height > jpeg_display_height))
			{
				pp_cfg->ppOutImg.width = ((jpeg_display_width + 7) / 8) * 8;
			}

			luma_size = pp_cfg->ppOutImg.width * pp_cfg->ppOutImg.height;
			luma_size_tmp = pp_cfg->ppOutImg.width * (pp_cfg->ppOutImg.height + 1); //IC bug

			switch(lp_param->output_pix_fmt)
			{
			case LP_FMT_YCBCR_4_2_0_SEMIPLANAR:
				pp_cfg->ppOutImg.pixFormat = PP_PIX_FMT_YCBCR_4_2_0_SEMIPLANAR;
				buf_size = luma_size_tmp * 3 / 2;
				break;
			case LP_FMT_YCBCR_4_2_2_INTERLEAVED:
		        pp_cfg->ppOutImg.pixFormat = PP_PIX_FMT_YCBCR_4_2_2_INTERLEAVED;
				buf_size = luma_size_tmp * 2;
				break;
			default:
				OSprintf("load_baseline_jpg() error: not supported out fmt!\n");
				goto error;
			}

		    hw_frame_buffer.size = buf_size;
#if 0
			ret = malloc_sys_buf(sys_buf, &hw_frame_buffer);
			if (ret != 0)
			{
				OSprintf("load_baseline_jpg() error: can not malloc sys_buf for frame buf!\n");
				goto error;
			}
#endif
            hw_frame_buffer.buf = (unsigned char *)lp_param->lp_malloc(hw_frame_buffer.size);
            if (NULL == hw_frame_buffer.buf)
            {
				OSprintf("load_baseline_jpg() error: can not malloc buf for frame buf!\n");
				goto error;
			}
			hw_frame_buffer.bus_addr = lp_param->lp_get_bus_addr((unsigned long)hw_frame_buffer.buf);//////////////////////////////////;
			
			pp_cfg->ppOutImg.bufferBusAddr = hw_frame_buffer.bus_addr;
			pp_cfg->ppOutImg.bufferChromaBusAddr = hw_frame_buffer.bus_addr + luma_size;

			if (PP_PIX_FMT_YCBCR_4_2_0_SEMIPLANAR == pp_cfg->ppOutImg.pixFormat)
			{
				unsigned char *p = (unsigned char *)hw_frame_buffer.buf;
				OSmemset(p, 0, luma_size);
				OSmemset(p + luma_size, 128, luma_size / 2);
			}
			else if (PP_PIX_FMT_YCBCR_4_2_2_INTERLEAVED == pp_cfg->ppOutImg.pixFormat)
			{
				unsigned char *p = (unsigned char *)hw_frame_buffer.buf;
				for (i = 0; i < luma_size; i++)
				{
					p[0] = 0;
					p[1] = 128;
					p += 2;
				}
			}
			else //RGB to be added
			{
				;
			}

		//  print_pp_cfg(pp_cfg);

		    pp_ret = PPSetConfig(pp_inst, pp_cfg);
			if (pp_ret != PP_OK)
			{
				//ppRet(pp_ret);
				//print_pp_cfg(pp_cfg);
				goto error;
			}
		}

		dec_ret = JpegDecDecode(dec_inst, dec_input, dec_output);
		if (JPEGDEC_FRAME_READY == dec_ret)
		{
			break;
		}
		else if (dec_ret != JPEGDEC_STRM_PROCESSED)
		{
			//JPEG_decRet(dec_ret);
			goto error;
		}
		else if (dec_input->stream_end_flag)
		{
			do 
			{
				dec_ret = JpegDecDecode(dec_inst, dec_input, dec_output);
			}while(JPEGDEC_STRM_PROCESSED == dec_ret);
			//JPEG_decRet(dec_ret);
			goto error;
		}
	}

	/* software scale and move */
	switch (pp_cfg->ppOutImg.pixFormat)
	{
	case PP_PIX_FMT_YCBCR_4_2_0_SEMIPLANAR:
		yuv_420_x = (lp_param->out_pic_pos_x / 2) * 2;
		yuv_420_y = (lp_param->out_pic_pos_y / 2) * 2;
		yuv_420_width = (lp_param->out_pic_width / 2) * 2;
		yuv_420_height = (lp_param->out_pic_height / 2) * 2;
		IMG_ZOOM_YUV420SEMIPLANAR_TO_YUV420SEMIPLANAR(hw_frame_buffer.buf, hw_frame_buffer.buf + luma_size,
			pp_cfg->ppOutImg.width, pp_cfg->ppOutImg.width, pp_cfg->ppOutImg.height, lp_param->out_buf_y.buf + 
			yuv_420_y * lp_param->out_buf_width + yuv_420_x, 
			lp_param->out_buf_uv.buf + (yuv_420_y / 2) * lp_param->out_buf_width + yuv_420_x, 
			lp_param->out_buf_width, yuv_420_width, yuv_420_height);
		break;
	case PP_PIX_FMT_YCBCR_4_2_2_INTERLEAVED:
		yuv_422_x = (lp_param->out_pic_pos_x / 2) * 2;
		yuv_422_y = lp_param->out_pic_pos_y;
		yuv_422_width = (lp_param->out_pic_width / 2) * 2;
		yuv_422_height = lp_param->out_pic_height;
		IMG_ZOOM_YUV422INTERLEAVED_TO_YUV422INTERLEAVED(hw_frame_buffer.buf, pp_cfg->ppOutImg.width,
			pp_cfg->ppOutImg.width, pp_cfg->ppOutImg.height, lp_param->out_buf_y.buf + yuv_422_y * lp_param->out_buf_width * 2 
			+ yuv_422_x * 2, lp_param->out_buf_width, yuv_422_width, yuv_422_height);
		break;
	default:
		OSprintf("load_baseline_jpg() error: not supported out fmt!\n");
		goto error;
	}

    //free_sys_buf(sys_buf, &hw_frame_buffer);
    if (hw_frame_buffer.buf)
    {
		lp_param->lp_free(hw_frame_buffer.buf);
		hw_frame_buffer.buf = NULL;
	}
    //free_sys_buf(sys_buf, &hw_stream_buffer);
    if (hw_stream_buffer.buf)
    {
		lp_param->lp_free(hw_stream_buffer.buf);
		hw_stream_buffer.buf = NULL;
	}
	if (dec_inst && pp_inst && pp_pipeline_flg)
	{
		PPDecPipelineDisable(pp_inst, dec_inst);
	}
	if (dec_inst)
	{
		JpegDecRelease(dec_inst);
	}
	if (pp_inst)
	{
		PPRelease(pp_inst);
	}
    OSHDel(dec_param->pHeap);
    //free_sys_buf(sys_buf, &sys_buf_hantro);
    if (sys_buf_hantro.buf)
    {
		lp_param->lp_free(sys_buf_hantro.buf);
		sys_buf_hantro.buf = NULL;
	}
	//release_sys_buf(sys_buf);
	return LP_OK;
	
error:
	//free_sys_buf(sys_buf, &hw_frame_buffer);
	//free_sys_buf(sys_buf, &hw_stream_buffer);
	if (hw_frame_buffer.buf)
    {
		lp_param->lp_free(hw_frame_buffer.buf);
		hw_frame_buffer.buf = NULL;
	}
    if (hw_stream_buffer.buf)
    {
		lp_param->lp_free(hw_stream_buffer.buf);
		hw_stream_buffer.buf = NULL;
	}
	if (dec_inst && pp_inst && pp_pipeline_flg)
	{
		PPDecPipelineDisable(pp_inst, dec_inst);
	}
	if (dec_inst)
	{
		JpegDecRelease(dec_inst);
	}
	if (pp_inst)
	{
		PPRelease(pp_inst);
	}
	OSHDel(dec_param->pHeap);
	//free_sys_buf(sys_buf, &sys_buf_hantro);
	 if (sys_buf_hantro.buf)
    {
		lp_param->lp_free(sys_buf_hantro.buf);
		sys_buf_hantro.buf = NULL;
	}
	//release_sys_buf(sys_buf);
	
	return LP_DECODE_ERROR;
}

static LP_RET load_progressive_jpg(LP_INPUT_PARAM *lp_param)
{
	void *dec_inst = NULL;
	JPEG_PROG_IMG_INFO img_ifo;
	//LP_SYS_BUF_INST load_photo_sys_buf, *sys_buf = &load_photo_sys_buf;
	LP_LINEAR_BUFFER tmp_buf;
	int ow, oh;
	int dw, dh;
	int scale = 0;
	unsigned long output_fmt;

	unsigned char *buf_y, *buf_uv;
	int stride;
	int ret = 0;
	int i;
	int check = 0;

	int yuv_420_x;
	int yuv_420_y;
	unsigned int yuv_420_width;
	unsigned int yuv_420_height;

	int yuv_422_x;
	int yuv_422_y;
	unsigned int yuv_422_width;
	unsigned int yuv_422_height;

	//OSmemset(sys_buf, 0, sizeof(LP_SYS_BUF_INST));
	//sys_buf->fd = -1;
	OSmemset(&tmp_buf, 0, sizeof(LP_LINEAR_BUFFER));
	OSmemset(&img_ifo, 0, sizeof(JPEG_PROG_IMG_INFO));

	if (lp_param->lp_fseek_set(lp_param->handle, 0))
	{
		OSprintf("load_progressive_jpg() error: seek_set error!\n");
		goto error;
	}
#if 0
	ret = init_sys_buf(sys_buf, O_RDWR);
	if (ret != 0)
	{
		OSprintf("load_progressive_jpg() error: init sys_buf error!\n");
		goto error;
	}
#endif

#ifdef LP_DEBUG
    //OSprintf("before jpeg_progressive_decoder_open!\n");
#endif
	ret = jpeg_progressive_decoder_open(&dec_inst, 
		                                lp_param->handle, 
		                                lp_param->lp_fseek_set, 
		                                lp_param->lp_fseek_cur, 
		                                lp_param->lp_fseek_end, 
		                                lp_param->lp_fread, 
		                                lp_param->lp_ftell,
		                                _malloc_,
		                                _free_,
		                                NULL,
		                                NULL,
		                                &img_ifo);
#ifdef LP_DEBUG
    //OSprintf("after jpeg_progressive_decoder_open!\n");
#endif
	if (ret != 0)
	{
		OSprintf("load_progressive_jpg() error: open prog decoder error: %d!\n", ret);
		goto error;
	}

    dw = img_ifo.image_width;
	dh = img_ifo.image_height;
	ow = img_ifo.output_width;
	oh = img_ifo.output_height;

#if 1 ////////////////////test
	for (scale = 0; scale < 4; scale++)
	{
		ow = img_ifo.output_width >> scale;
		oh = img_ifo.output_height >> scale;
		dw = img_ifo.image_width >> scale;
		dh = img_ifo.image_height >> scale;
		switch (lp_param->output_pix_fmt)
	    {
		case LP_FMT_YCBCR_4_2_0_SEMIPLANAR:
			output_fmt = JPG_PROG_DEC_OUT_FMT_YUV420;
			tmp_buf.size = ow * oh * 3 / 2;
			break;
		case LP_FMT_YCBCR_4_2_2_INTERLEAVED:
			output_fmt = JPG_PROG_DEC_OUT_FMT_YUV422;
			tmp_buf.size = ow * oh * 2;
			break;
		default:
			OSprintf("load_progressive_jpg() error: unsupported output format!\n");
			goto error;
		}
#if 0
		if (0 == malloc_sys_buf(sys_buf, &tmp_buf))
		{
			check = 1;
			break;
		}
#endif
        tmp_buf.buf = (unsigned char *)lp_param->lp_malloc(tmp_buf.size);
        if (NULL != tmp_buf.buf)
        {
			tmp_buf.bus_addr = lp_param->lp_get_bus_addr((unsigned long)tmp_buf.buf);//////////////////////////////////////////////;
			check = 1;
			break;
		}
	}
#endif

	///////////////////////////////test
	//output_fmt = JPG_PROG_DEC_OUT_FMT_YUV420;
	//tmp_buf.size = ow * oh * 3 / 2;
	///////////////////////////////test

	if (0 == check)
	{
		OSprintf("load_progressive_jpg() error: can not malloc tmp buf for prog decode!\n");
		goto error;
	}

	buf_y = tmp_buf.buf;
	buf_uv = buf_y + ow * oh;

	switch(output_fmt)
	{
	case JPG_PROG_DEC_OUT_FMT_YUV420:
		OSmemset(buf_y, 0, ow * oh);
		OSmemset(buf_uv, 128, ow * oh / 2);
		break;
	case JPG_PROG_DEC_OUT_FMT_YUV422:
		{
			unsigned char *p = buf_y;
			for (i = 0; i< ow * oh; i++)
			{
				p[0] = 255;
				p[1] = 0;
				p += 2;
			}
		}
		break;
	default:
		OSprintf("load_progressive_jpg() error: unsupported output format!\n");
		goto error;
	}
	
	stride = ow;
	if (output_fmt == JPG_PROG_DEC_OUT_FMT_YUV422)
	{
		stride *= 2;
	}

#ifdef LP_DEBUG
    //OSprintf("before jpeg_progressive_decode!\n");
#endif
	ret = jpeg_progressive_decode(dec_inst,
		                          buf_y, 
		                          buf_uv, 
		                          stride, 
		                          scale, 
		                          output_fmt, 
		                          0, 
		                          0,
		                          img_ifo.output_width,
		                          img_ifo.output_height);

#ifdef LP_DEBUG
    //OSprintf("after jpeg_progressive_decode!\n");
#endif
	if (ret != 0)
	{
		OSprintf("load_progressive_jpg() error: decode progressive jpg error!\n");
		goto error;
	}

	//////////////////////////////////////////////test
	if (0)
	{
		void *fp = NULL;
		fp = OSfopen("prog_dec_res.yuv", "wb");
		if (fp != NULL)
		{
			OSfwrite(buf_y, 1, ow * oh * 2, fp);
			OSfclose(fp);
		}
	}
	//////////////////////////////////////////////test

#ifdef LP_DEBUG
    //OSprintf("before software scale!\n");
#endif
	/* software scale and move */
	switch (lp_param->output_pix_fmt)
    {
	case LP_FMT_YCBCR_4_2_0_SEMIPLANAR:
		yuv_420_x = (lp_param->out_pic_pos_x / 2) * 2;
		yuv_420_y = (lp_param->out_pic_pos_y / 2) * 2;
		yuv_420_width = (lp_param->out_pic_width / 2) * 2;
		yuv_420_height = (lp_param->out_pic_height / 2) * 2;
		IMG_ZOOM_YUV420SEMIPLANAR_TO_YUV420SEMIPLANAR(buf_y, buf_uv,
			ow, dw, dh, lp_param->out_buf_y.buf + 
			yuv_420_y * lp_param->out_buf_width + yuv_420_x, 
			lp_param->out_buf_uv.buf + (yuv_420_y / 2) * lp_param->out_buf_width + yuv_420_x, 
			lp_param->out_buf_width, yuv_420_width, yuv_420_height);
		break;
	case LP_FMT_YCBCR_4_2_2_INTERLEAVED:
		yuv_422_x = (lp_param->out_pic_pos_x / 2) * 2;
		yuv_422_y = lp_param->out_pic_pos_y;
		yuv_422_width = (lp_param->out_pic_width / 2) * 2;
		yuv_422_height = lp_param->out_pic_height;
		IMG_ZOOM_YUV422INTERLEAVED_TO_YUV422INTERLEAVED(buf_y, ow,
			dw, dh, lp_param->out_buf_y.buf + yuv_422_y * lp_param->out_buf_width * 2 
			+ yuv_422_x * 2, lp_param->out_buf_width, yuv_422_width, yuv_422_height);
		break;
	default:
		OSprintf("load_progressive_jpg() error: unsupported output format!\n");
		goto error;
	}

#ifdef LP_DEBUG
    //OSprintf("after software scale!\n");
#endif
	
    jpeg_progressive_decoder_close(dec_inst);
    //free_sys_buf(sys_buf, &tmp_buf);
    if (tmp_buf.buf)
    {
		lp_param->lp_free(tmp_buf.buf);
		tmp_buf.buf = NULL;
	}
	//release_sys_buf(sys_buf);
	return LP_OK;

error:
	jpeg_progressive_decoder_close(dec_inst);
	//free_sys_buf(sys_buf, &tmp_buf);
	 if (tmp_buf.buf)
    {
		lp_param->lp_free(tmp_buf.buf);
		tmp_buf.buf = NULL;
	}
	//release_sys_buf(sys_buf);
	return LP_DECODE_ERROR;
}


/* jpg soft dec*/
static LP_RET load_jpg_soft(LP_INPUT_PARAM *lp_param)
{
	void *jpg_inst = NULL;
	//LP_SYS_BUF_INST load_photo_sys_buf, *sys_buf = &load_photo_sys_buf;
	JPEG_IMG_INFO jpeg_img_info;
	JPEG_IMG_INFO *img_ifo = &jpeg_img_info;
	LP_LINEAR_BUFFER tmp_buf;
	int ow, oh;
	int dw, dh;
	int scale = 0;
	unsigned long output_fmt;

	unsigned char *buf_y, *buf_uv, *buf_v;
	int stride;
	int ret = 0;
	int i;
	int check = 0;

	int yuv_420_x;
	int yuv_420_y;
	unsigned int yuv_420_width;
	unsigned int yuv_420_height;

	int yuv_422_x;
	int yuv_422_y;
	unsigned int yuv_422_width;
	unsigned int yuv_422_height;

	OSmemset(img_ifo, 0, sizeof(JPEG_IMG_INFO));
	//OSmemset(sys_buf, 0, sizeof(LP_SYS_BUF_INST));
	//sys_buf->fd = -1;
	OSmemset(&tmp_buf, 0, sizeof(LP_LINEAR_BUFFER));

	if (lp_param->lp_fseek_set(lp_param->handle, 0))
	{
		OSprintf("load_jpg_soft() error: seek_set error!\n");
		goto error;
	}

#if 0
	ret = init_sys_buf(sys_buf, O_RDWR);
	if (ret != 0)
	{
		OSprintf("load_jpg_soft() error: init sys_buf error!\n");
		goto error;
	}
#endif

	ret = jpeg_decoder_open(&jpg_inst, 
		                    lp_param->handle, 
		                    lp_param->lp_fseek_set,
		                    lp_param->lp_fseek_cur,
		                    lp_param->lp_fseek_end, 
		                    lp_param->lp_fread, 
		                    lp_param->lp_ftell,
		                    _malloc_,
		                    _free_,
		                    img_ifo);
	if (ret != 0)
	{
		OSprintf("load_jpg_soft() error: jpeg_decoder_open error!\n");
		goto error;
	}

	dw = img_ifo->image_width;
	dh = img_ifo->image_height;
	ow = img_ifo->output_width;
	oh = img_ifo->output_height;

#if 1 ////////////////////test
	if (dh != 1)
    {
		for (scale = 0; scale < 4; scale++)
		{
			ow = img_ifo->output_width >> scale;
			oh = img_ifo->output_height >> scale;
			dw = img_ifo->image_width >> scale;
			dh = img_ifo->image_height >> scale;
			switch (lp_param->output_pix_fmt)
		    {
			case LP_FMT_YCBCR_4_2_0_SEMIPLANAR:
				output_fmt = JPG_OUT_FMT_YUV420S;
				tmp_buf.size = ow * oh * 3 / 2;
				break;
			case LP_FMT_YCBCR_4_2_2_INTERLEAVED:
				output_fmt = JPG_OUT_FMT_YUV422I;
				tmp_buf.size = ow * oh * 2;
				break;
			default:
				OSprintf("load_jpg_soft() error: unsupported output format!\n");
				goto error;
			}
#if 0
			if (0 == malloc_sys_buf(sys_buf, &tmp_buf))
			{
				check = 1;
				break;
			}
#endif
            tmp_buf.buf = (unsigned char *)lp_param->lp_malloc(tmp_buf.size);
            if (NULL != tmp_buf.buf)
            {
				tmp_buf.bus_addr = lp_param->lp_get_bus_addr((unsigned long)tmp_buf.buf);////////////////////////;
				check = 1;
				break;
			}
		}
	}
#endif

	if (0 == check)
	{
		OSprintf("load_jpg_soft() error: can not malloc tmp buf for soft decode!\n");
		goto error;
	}

	buf_y = tmp_buf.buf;
	buf_uv = buf_y + ow * oh;
	buf_v = buf_uv + ow * oh / 4;

	switch(output_fmt)
	{
	case JPG_OUT_FMT_YUV420S:
		OSmemset(buf_y, 0, ow * oh);
		OSmemset(buf_uv, 128, ow * oh / 2);
		break;
	case JPG_OUT_FMT_YUV422I:
		{
			unsigned char *p = buf_y;
			for (i = 0; i< ow * oh; i++)
			{
				p[0] = 255;
				p[1] = 0;
				p += 2;
			}
		}
		break;
	default:
		OSprintf("load_jpg_soft() error: unsupported output format!\n");
		goto error;
	}
	
	stride = ow;
	if (output_fmt == JPG_OUT_FMT_YUV422I)
	{
		stride *= 2;
	}

	ret = jpeg_decode(jpg_inst,
						buf_y,
						buf_uv,
						buf_v,
						stride,
						scale,
						output_fmt,
						0,
						0,
						img_ifo->output_width,
						img_ifo->output_height);
	if (ret != 0)
	{
		OSprintf("load_jpg_soft() error: soft decode error!\n");
		goto error;
	}

	if (dh == 1)
	{
		if (output_fmt == JPG_OUT_FMT_YUV420P)
		{
			OSmemcpy(buf_y + stride * 1, buf_y, stride);
			OSmemcpy(buf_y + stride * 2, buf_y, stride);
			OSmemcpy(buf_y + stride * 3, buf_y, stride);

			OSmemcpy(buf_uv + stride/2, buf_uv, stride/2);
			OSmemcpy(buf_v + stride/2, buf_v, stride/2);
			dh = 4;
		}
		else if (output_fmt == JPG_OUT_FMT_YUV420S)
		{
			OSmemcpy(buf_y + stride * 1, buf_y, stride);
			OSmemcpy(buf_y + stride * 2, buf_y, stride);
			OSmemcpy(buf_y + stride * 3, buf_y, stride);
			
			OSmemcpy(buf_uv + stride, buf_uv, stride);
			dh = 4;
		}
		else if (output_fmt == JPG_OUT_FMT_YUV422I)
		{
			OSmemcpy(buf_y + stride, buf_y, stride);
			dh = 2;
		}
	}

	/* software scale and move */
	switch (lp_param->output_pix_fmt)
    {
	case LP_FMT_YCBCR_4_2_0_SEMIPLANAR:
		yuv_420_x = (lp_param->out_pic_pos_x / 2) * 2;
		yuv_420_y = (lp_param->out_pic_pos_y / 2) * 2;
		yuv_420_width = (lp_param->out_pic_width / 2) * 2;
		yuv_420_height = (lp_param->out_pic_height / 2) * 2;
		IMG_ZOOM_YUV420SEMIPLANAR_TO_YUV420SEMIPLANAR(buf_y, buf_uv,
			ow, dw, dh, lp_param->out_buf_y.buf + 
			yuv_420_y * lp_param->out_buf_width + yuv_420_x, 
			lp_param->out_buf_uv.buf + (yuv_420_y / 2) * lp_param->out_buf_width + yuv_420_x, 
			lp_param->out_buf_width, yuv_420_width, yuv_420_height);
		break;
	case LP_FMT_YCBCR_4_2_2_INTERLEAVED:
		yuv_422_x = (lp_param->out_pic_pos_x / 2) * 2;
		yuv_422_y = lp_param->out_pic_pos_y;
		yuv_422_width = (lp_param->out_pic_width / 2) * 2;
		yuv_422_height = lp_param->out_pic_height;
		IMG_ZOOM_YUV422INTERLEAVED_TO_YUV422INTERLEAVED(buf_y, ow,
			dw, dh, lp_param->out_buf_y.buf + yuv_422_y * lp_param->out_buf_width * 2 
			+ yuv_422_x * 2, lp_param->out_buf_width, yuv_422_width, yuv_422_height);
		break;
	default:
		OSprintf("load_jpg_soft() error: unsupported output format!\n");
		goto error;
	}

	jpeg_decoder_close(jpg_inst);
	//free_sys_buf(sys_buf, &tmp_buf);
	if (tmp_buf.buf)
	{
		lp_param->lp_free(tmp_buf.buf);
		tmp_buf.buf = NULL;
	}
	//release_sys_buf(sys_buf);
	
	return LP_OK;
error:
	jpeg_decoder_close(jpg_inst);
	//free_sys_buf(sys_buf, &tmp_buf);
	if (tmp_buf.buf)
	{
		lp_param->lp_free(tmp_buf.buf);
		tmp_buf.buf = NULL;
	}
	//release_sys_buf(sys_buf);
	return LP_DECODE_ERROR;
}


DLL_EXPORT LP_RET load_jpeg(LP_INPUT_PARAM *lp_param)
{
	/* for simple, first try baseline, then try progressive */
	LP_RET ret = LP_OK;
	
#ifdef LP_DEBUG
    //OSprintf("enter load_jpeg()!\n");
#endif
  
	ret = load_baseline_jpg(lp_param);
	if (ret != LP_OK)
	{
		//ret = load_progressive_jpg(lp_param);
		ret = load_jpg_soft(lp_param);
		if (ret != LP_OK)
		{
			ret = load_progressive_jpg(lp_param);
			if (ret != LP_OK)
			{
				OSprintf("load_jpg error: unsupported photo or decode error!\n");
				goto error;
			}
		}
	}
	return LP_OK;

error:
	return LP_DECODE_ERROR;
}
#if 0
static void yuv420s_to_yuv422i(unsigned char *p420_buf_y,
	                               unsigned char *p420_buf_uv,
	                               unsigned char *p422_buf,
	                               int width,
	                               int height)
{
	int i;
	int j;
    int yuv422_stride = width * 2;
	unsigned char *p420y = p420_buf_y;
	unsigned char *p420uv = p420_buf_uv;
	unsigned char *p422 = p422_buf;

	for (i = 0; i < height; i++)
	{
		for (j = 0; j < width; j++)
		{
			*(p422 + i * yuv422_stride + j * 2) = *(p420y + i * width + j);
		}
	}
	#if 1
	for (i = 0; i < height / 2; i++)
	{
		for (j = 0; j < width / 2; j++)
		{
			*(p422 + (i * 2) * yuv422_stride + 4 * j + 1) =
			*(p422 + (i * 2 + 1) * yuv422_stride + 4 * j + 1) = *(p420uv + i * width + j * 2);

			*(p422 + (i * 2) * yuv422_stride + 4 * j + 3) =
			*(p422 + (i * 2 + 1) * yuv422_stride + 4 * j + 3) = *(p420uv + i * width + j * 2 + 1);
		}
	}
	#endif
	
}



DLL_EXPORT LP_RET load_bmp(LP_INPUT_PARAM *lp_param)
{
#if 1
	IMG_BMP_CONTEXT bmp;
	unsigned long stream_total_length;
	unsigned char *rgba_buf;
	unsigned char *rgb;
	int ret = 0;
	int img_w;
	int img_h;
	int pic_inverse;
	int stop = 0;
	int *stats = &stop;

	OSmemset(&bmp, 0, sizeof(IMG_BMP_CONTEXT));

	if (LP_FMT_RGB_32 != lp_param->output_pix_fmt)
	{
		OSprintf("load_bmp() error: unsupported output pix format!\n");
		goto error;
	}

	if (!lp_param->handle || !lp_param->lp_fseek_set || !lp_param->lp_fseek_cur || !lp_param->lp_fseek_end
		|| !lp_param->lp_fread || !lp_param->lp_ftell)
	{
		OSprintf("load_bmp() error: IO layer error!\n");
		goto error;
	}

	if (lp_param->lp_fseek_end(lp_param->handle, 0))
	{
		OSprintf("load_bmp() error: seek_end error!\n");
		goto error;
	}
	stream_total_length = lp_param->lp_ftell(lp_param->handle);
	if (stream_total_length <= 0)
	{
		OSprintf("load_bmp() error: stream_total_length<=0!\n");
		goto error;
	}
	if (lp_param->lp_fseek_set(lp_param->handle, 0))
	{
		OSprintf("load_bmp() error: seek_set error!\n");
		goto error;
	}

	set_param(&bmp, 16000, 12000, lp_param->handle, lp_param->lp_fread, 
		lp_param->lp_fseek_set, lp_param->lp_fseek_cur, lp_param->lp_fseek_end, 
		lp_param->lp_ftell, stream_total_length);

	ret = fill_input_buffer(&bmp);
	if (ret != 0)
	{
		OSprintf("load_bmp() error: fill_input_buffer error!\n");
		goto error;
	}

	ret = read_bmp_header(&bmp, &img_w, &img_h, &pic_inverse);
	if (-1 == ret)
	{
		OSprintf("load_bmp() error: read bmp header error!\n");
		goto error;
	}

	rgba_buf = (unsigned char *)lp_param->lp_malloc(img_w * img_h * 4);
	if (NULL == rgba_buf)
	{
		OSprintf("can not malloc out buf for bmp!\n");
		goto error;
	}

	OSmemset(rgba_buf, 0, img_w * img_h * 4);

	rgb = rgba_buf;

    ret = process_bmp(&bmp, stats, rgb);
	if (ret != 0)
	{
		OSprintf("load_bmp() error: bmp decode error!\n");
		goto error;
	}

	if(pic_inverse == 0)
	{
		ret = bottomup_bgra(rgba_buf, img_h, img_w * 4);
		if (ret != 0)
		{
			OSprintf("load_bmp() error: bottomup_bgra error!\n");
			goto error;
		}
	}

	IMG_ZOOM_RGB32_TO_RGB32((unsigned char *)rgba_buf, img_w, img_w, img_h, 
		                    lp_param->out_buf_y.buf + lp_param->out_pic_pos_y * lp_param->out_buf_width * 4 + lp_param->out_pic_pos_x * 4,
		                    lp_param->out_buf_width, lp_param->out_pic_width, lp_param->out_pic_height);

	if (rgba_buf != NULL)
	{
		lp_param->lp_free(rgba_buf);
		rgba_buf = NULL;
	}

	return LP_OK;

error:
	if (rgba_buf != NULL)
	{
		lp_param->lp_free(rgba_buf);
		rgba_buf = NULL;
	}
	return LP_DECODE_ERROR;

#endif
}



DLL_EXPORT LP_RET load_png(LP_INPUT_PARAM *lp_param)
{
	PNG_OUT png_out;
	PNG_OUT *out = &png_out;
	//LP_SYS_BUF_INST load_photo_sys_buf, *sys_buf = &load_photo_sys_buf;
	int ret = 0;

	OSmemset(out, 0, sizeof(PNG_OUT));
	//OSmemset(sys_buf, 0, sizeof(LP_SYS_BUF_INST));
	//sys_buf->fd = -1;

	if (lp_param->output_pix_fmt != LP_FMT_RGB_32)
	{
		OSprintf("load_png() error: the output fmt must be RGB32!\n");
		goto error;
	}

#if 0
	ret = init_sys_buf(sys_buf, O_RDWR);
	if (ret != 0)
	{
		OSprintf("load_png() error: init sys_buf error!\n");
		goto error;
	}
#endif

	ret = png_decode_frame(lp_param->handle, lp_param->lp_fread, lp_param->lp_fseek_cur, 
		                   NULL, lp_param->lp_malloc, out);

	if (ret != 0)
	{
		OSprintf("load_png() error: png_decode_frame() error!\n");
		goto error;
	}

	IMG_ZOOM_RGB32_TO_RGB32(out->img, out->line_size / 4, out->width, out->height, 
		                    lp_param->out_buf_y.buf + lp_param->out_pic_pos_y * lp_param->out_buf_width * 4 + lp_param->out_pic_pos_x * 4,
		                    lp_param->out_buf_width, lp_param->out_pic_width, lp_param->out_pic_height);

	//png_free_frame((void *)sys_buf, out->img, out->bus_addr);
	if (out->img)
	{
		lp_param->lp_free(out->img);
		out->img = NULL;
	}
	//release_sys_buf(sys_buf);
	
	return LP_OK;

error:
	//png_free_frame((void *)sys_buf, out->img, out->bus_addr);
	if (out->img)
	{
		lp_param->lp_free(out->img);
		out->img = NULL;
	}
	//release_sys_buf(sys_buf);
	return LP_DECODE_ERROR;
}

static ColorMapObject *ColorMap;

static void output_to_rgb32(GifFileType *GifFile, unsigned char* Buffer, GifRowType *ScreenBuffer,
			                         int ScreenWidth, int ScreenHeight)

{
    int i, j;
    GifRowType GifRow;
    static GifColorType	*ColorMapEntry;
	GifColorType *global_ColorMapEntry;
	ColorMapObject *global_ColorMap;
	int width, height, stride;
    unsigned char r,g,b;
	
	width = ScreenWidth;
	height = ScreenHeight;
	stride = width;

	OSmemset(Buffer, 0, width * height * 4); 

	if ((0 != GifFile->frame_num) && (1 == GifFile->Image.transparent_flg))
	{
		if (RESTORE_TO_BACKGROUND == GifFile->Image.disposal_method)
		{
			global_ColorMap = GifFile->SColorMap;
			global_ColorMapEntry = &global_ColorMap->Colors[GifFile->SBackGroundColor];
			for (i = 0; i < ScreenHeight; i++) 
			{
			    GifRow = ScreenBuffer[i];
				for (j = 0; j < ScreenWidth; j++)
				{
					if (GifFile->Image.transparent_color_index == GifRow[j])
					{
						r = global_ColorMapEntry->Red;
						g = global_ColorMapEntry->Green;
						b = global_ColorMapEntry->Blue;
					}
					else
					{
						ColorMapEntry = &ColorMap->Colors[GifRow[j]];
						r = ColorMapEntry->Red;
						g = ColorMapEntry->Green;
						b = ColorMapEntry->Blue;
					}

					*(GifFile->prev_img + (i * ScreenWidth + j) * 3) = r;
					*(GifFile->prev_img + (i * ScreenWidth + j) * 3 + 1) = g;
					*(GifFile->prev_img + (i * ScreenWidth + j) * 3 + 2) = b;

					*(Buffer + (i * ScreenWidth + j) * 4) = b;
					*(Buffer + (i * ScreenWidth + j) * 4 + 1) = g;
					*(Buffer + (i * ScreenWidth + j) * 4 + 2) = r;
					*(Buffer + (i * ScreenWidth + j) * 4 + 3) = 255;
			    }
			}
		}
		else //if (DO_NOT_DISPOSE == GifFile->Image.disposal_method)
		{
			for (i = 0; i < ScreenHeight; i++) 
			{
			    GifRow = ScreenBuffer[i];
				for (j = 0; j < ScreenWidth; j++)
				{
					ColorMapEntry = &ColorMap->Colors[GifRow[j]];

					if (GifFile->Image.transparent_color_index == GifRow[j])
					{
						r = *(GifFile->prev_img + (i * ScreenWidth + j) * 3);
						g = *(GifFile->prev_img + (i * ScreenWidth + j) * 3 + 1);
						b = *(GifFile->prev_img + (i * ScreenWidth + j) * 3 + 2);
					}
					else
					{
						r = ColorMapEntry->Red;
						g = ColorMapEntry->Green;
						b = ColorMapEntry->Blue;
					}

					*(GifFile->prev_img + (i * ScreenWidth + j) * 3) = r;
					*(GifFile->prev_img + (i * ScreenWidth + j) * 3 + 1) = g;
					*(GifFile->prev_img + (i * ScreenWidth + j) * 3 + 2) = b;

					*(Buffer + (i * ScreenWidth + j) * 4) = b;
					*(Buffer + (i * ScreenWidth + j) * 4 + 1) = g;
					*(Buffer + (i * ScreenWidth + j) * 4 + 2) = r;
					*(Buffer + (i * ScreenWidth + j) * 4 + 3) = 255;
			    }
			}
		}
		
	}
    else
    {
		for (i = 0; i < ScreenHeight; i++) 
		{
		    GifRow = ScreenBuffer[i];
			for (j = 0; j < ScreenWidth; j++)
			{
				ColorMapEntry = &ColorMap->Colors[GifRow[j]];

				r = ColorMapEntry->Red;
				g = ColorMapEntry->Green;
				b = ColorMapEntry->Blue;

				*(GifFile->prev_img + (i * ScreenWidth + j) * 3) = r;
				*(GifFile->prev_img + (i * ScreenWidth + j) * 3 + 1) = g;
				*(GifFile->prev_img + (i * ScreenWidth + j) * 3 + 2) = b;

				*(Buffer + (i * ScreenWidth + j) * 4) = b;
				*(Buffer + (i * ScreenWidth + j) * 4 + 1) = g;
				*(Buffer + (i * ScreenWidth + j) * 4 + 2) = r;
				*(Buffer + (i * ScreenWidth + j) * 4 + 3) = 255;
		    }
		}
    }

 }


void pass_function(void *file, void *read, void *malloc, void *realloc, void *free);


DLL_EXPORT LP_RET load_static_gif(LP_INPUT_PARAM *lp_param)
{
	int	i, j, Size, Row, Col, Width, Height, ExtCode, Count;
	GifRecordType RecordType;
    GifByteType *Extension = NULL;
    GifRowType *ScreenBuffer = NULL;
    GifFileType *GifFile = NULL;
	int output_width, output_height;
	unsigned char *outbuf = NULL;

	int ColorMapSize = 0,    
		InterlacedOffset[] = { 0, 4, 2, 1 }, /* The way Interlaced image should. */    
		InterlacedJumps[] = { 8, 8, 4, 2 };    /* be read - offsets and jumps... */
	
//	OSprintf("start gif_decode\n");

    if (lp_param->output_pix_fmt != LP_FMT_RGB_32)
    {
		OSprintf("not supported output pix fmt!\n");
		return LP_DECODE_ERROR;
	}
	
	pass_function(lp_param->handle, (void*)lp_param->lp_fread, (void*)lp_param->lp_malloc,
	              (void*)lp_param->lp_realloc, (void*)lp_param->lp_free);

	if ((GifFile = DGifOpenFileHandle((int)lp_param->handle)) == NULL) 
	{	
		OSprintf("DGifOpenFileHandle error, EXIT_FAILURE");
		goto error;
	}

	if ((ScreenBuffer = (GifRowType *)lp_param->lp_malloc(GifFile->SHeight * sizeof(GifRowType *))) == NULL)
	{
	    OSprintf("Failed to allocate memory required, aborted.");
		goto error;
	}

    Size = GifFile->SWidth * sizeof(GifPixelType);/* Size in bytes one row.*/
    if ((ScreenBuffer[0] = (GifRowType) lp_param->lp_malloc(Size)) == NULL) /* First row. */
    {
		OSprintf("Failed to allocate memory required, aborted.");
		goto error;
    }

    for (i = 0; i < GifFile->SWidth; i++)  /* Set its color to BackGround. */
		ScreenBuffer[0][i] = GifFile->SBackGroundColor;
	
	for (i = 1; i < GifFile->SHeight; i++) /* Allocate the other rows, and set their color to background too: */
	{  
		if ((ScreenBuffer[i] = (GifRowType) lp_param->lp_malloc(Size)) == NULL)
		{
			OSprintf("Failed to allocate memory required, aborted.");
			goto error;
		}

		OSmemcpy(ScreenBuffer[i], ScreenBuffer[0], Size);
    }

	output_width  = GifFile->SWidth;
	output_height = GifFile->SHeight;

	outbuf = (unsigned char *)lp_param->lp_malloc(output_width * output_height * 4);

	/* Scan the content of the GIF file and load the image(s) in: */
    do 
	{
		if (DGifGetRecordType(GifFile, &RecordType) == GIF_ERROR) 
		{
			OSprintf("DGifGetRecordType error, GIF_ERROR.");
			goto error;
		}
		switch (RecordType) 
		{
	    case IMAGE_DESC_RECORD_TYPE:
			if (DGifGetImageDesc(GifFile) == GIF_ERROR)
			{
			    GIF_EXIT("DGifGetImageDesc error, GIF_ERROR");
				goto error;
			}
			Row = GifFile->Image.Top;      /* Image Position relative to Screen. */
			Col = GifFile->Image.Left;
			Width = GifFile->Image.Width;
			Height = GifFile->Image.Height;

			if (GifFile->Image.Left + GifFile->Image.Width > GifFile->SWidth ||
			    GifFile->Image.Top + GifFile->Image.Height > GifFile->SHeight) 
			{
			    OSprintf("GIF_ERROR\n");
				goto error;
			}
			if (GifFile->Image.Interlace) 
			{
			    /* Need to perform 4 passes on the images: */
			    for (Count = i = 0; i < 4; i++)
			    {
					for (j = Row + InterlacedOffset[i]; j < Row + Height;
							 j += InterlacedJumps[i]) 
					{
					    if (DGifGetLine(GifFile, &ScreenBuffer[j][Col], Width) == GIF_ERROR) 
						{
			    			OSprintf("GIF_ERROR");
							goto error;
					    }
					}
			    }
			}
			else 
			{
			    for (i = 0; i < Height; i++)
				{
					if (DGifGetLine(GifFile, &ScreenBuffer[Row++][Col], Width) == GIF_ERROR)
					{
				    	OSprintf("GIF_ERROR");
						goto error;
					}
			    }
			}

#if 1
			/* Lets dump it - set the global variables required and do it: */
	    	//BackGround = GifFile->SBackGroundColor;
    		ColorMap = (GifFile->Image.ColorMap ? GifFile->Image.ColorMap : GifFile->SColorMap);
    		ColorMapSize = ColorMap->ColorCount;

			output_to_rgb32(GifFile, outbuf, ScreenBuffer, GifFile->SWidth, GifFile->SHeight);

			GifFile->frame_num++;
#endif
			//break;
			goto end;
	    case EXTENSION_RECORD_TYPE:
			/* Skip any extension blocks in file: */
			if (DGifGetExtension(GifFile, &ExtCode, &Extension) == GIF_ERROR) 
			{
			    OSprintf("GIF_ERROR");
				goto error;
			}

			/* add chengjiali */
			if ((GRAPHICS_EXT_FUNC_CODE == ExtCode) && (Extension != NULL))
			{
				GifFile->Image.transparent_flg = (Extension[1] & 1);
				GifFile->Image.transparent_color_index = Extension[4];
				GifFile->Image.disposal_method = (Extension[1] & 0x1C) >> 2;
				GifFile->Image.delay_time = ((unsigned short)Extension[3] << 8) + Extension[2];
			}

#if 1 // 0
			while (Extension != NULL) 
			{
			    if (DGifGetExtensionNext(GifFile, &Extension) == GIF_ERROR) 
				{
					OSprintf("GIF_ERROR");
					goto error;
			    }
			}
#endif
			break;
	    case TERMINATE_RECORD_TYPE:
			break;
	    default:		    /* Should be traps by DGifGetRecordType. */
			break;
		}
    }
    while (RecordType != TERMINATE_RECORD_TYPE);

end: 

	IMG_ZOOM_RGB32_TO_RGB32(outbuf, output_width, output_width, output_height, 
		                    lp_param->out_buf_y.buf + lp_param->out_pic_pos_y * lp_param->out_buf_width * 4 + lp_param->out_pic_pos_x * 4,
		                    lp_param->out_buf_width, lp_param->out_pic_width, lp_param->out_pic_height);

	if(outbuf != NULL)
	{
		lp_param->lp_free(outbuf);
		outbuf = NULL;
	}

	for (i = 0; i < GifFile->SHeight; i++) 
	{  
		if (ScreenBuffer[i] != NULL)
		{
			lp_param->lp_free(ScreenBuffer[i]);
			ScreenBuffer[i] = NULL;
		}
    }
	
	if(ScreenBuffer != NULL)
	{
		lp_param->lp_free(ScreenBuffer);
		ScreenBuffer = NULL;
	}

	DGifCloseFile(GifFile);
	return LP_OK;

error:

	if(outbuf != NULL)
	{
		lp_param->lp_free(outbuf);
		outbuf = NULL;
	}

	for (i = 1; i < GifFile->SHeight; i++)
	{  
		if (ScreenBuffer[i] != NULL)
		{
			lp_param->lp_free(ScreenBuffer[i]);
			ScreenBuffer[i] = NULL;
		}
    }
	
	if(ScreenBuffer != NULL)
	{
		lp_param->lp_free(ScreenBuffer);
		ScreenBuffer = NULL;
	}

	DGifCloseFile(GifFile);
	return LP_DECODE_ERROR;	
}

DLL_EXPORT LP_RET load_gif_start(void **p_gif_handle, LP_INPUT_PARAM *lp_param, unsigned short *delay_time)
{
	int	i, j, Size, Row, Col, Width, Height, ExtCode, Count;
	GifRecordType RecordType;
    GifByteType *Extension;
    GifFileType *GifFile;
	int output_width, output_height;

	int ColorMapSize = 0,    
		InterlacedOffset[] = { 0, 4, 2, 1 }, /* The way Interlaced image should. */    
		InterlacedJumps[] = { 8, 8, 4, 2 };    /* be read - offsets and jumps... */
	
//	OSprintf("start gif_decode\n");

    if (lp_param->output_pix_fmt != LP_FMT_RGB_32)
    {
		OSprintf("not supported output pix fmt!\n");
		return LP_DECODE_ERROR;
	}
	
	pass_function(lp_param->handle, (void*)lp_param->lp_fread, (void*)lp_param->lp_malloc,
	              (void*)lp_param->lp_realloc, (void*)lp_param->lp_free);

	if ((GifFile = DGifOpenFileHandle((int)lp_param->handle)) == NULL) 
	{	
		OSprintf("DGifOpenFileHandle error, EXIT_FAILURE");
		goto error;
	}

	*p_gif_handle = GifFile;

	if ((GifFile->ScreenBuffer = (GifRowType *)lp_param->lp_malloc(GifFile->SHeight * sizeof(GifRowType *))) == NULL)
	{
	    OSprintf("Failed to allocate memory required, aborted.");
		goto error;
	}

    Size = GifFile->SWidth * sizeof(GifPixelType);/* Size in bytes one row.*/
    if (((GifFile->ScreenBuffer)[0] = (GifRowType) lp_param->lp_malloc(Size)) == NULL) /* First row. */
    {
		OSprintf("Failed to allocate memory required, aborted.");
		goto error;
    }

    for (i = 0; i < GifFile->SWidth; i++)  /* Set its color to BackGround. */
		(GifFile->ScreenBuffer)[0][i] = GifFile->SBackGroundColor;
	
	for (i = 1; i < GifFile->SHeight; i++) /* Allocate the other rows, and set their color to background too: */
	{  
		if (((GifFile->ScreenBuffer)[i] = (GifRowType) lp_param->lp_malloc(Size)) == NULL)
		{
			OSprintf("Failed to allocate memory required, aborted.");
			goto error;
		}

		OSmemcpy((GifFile->ScreenBuffer)[i], (GifFile->ScreenBuffer)[0], Size);
    }

	output_width  = GifFile->SWidth;
	output_height = GifFile->SHeight;

	GifFile->outbuf = (unsigned char *)lp_param->lp_malloc(output_width * output_height * 4);

	/* Scan the content of the GIF file and load the image(s) in: */
    do 
	{
		if (DGifGetRecordType(GifFile, &RecordType) == GIF_ERROR) 
		{
			OSprintf("DGifGetRecordType error, GIF_ERROR.");
			goto error;
		}
		switch (RecordType) 
		{
	    case IMAGE_DESC_RECORD_TYPE:
			if (DGifGetImageDesc(GifFile) == GIF_ERROR)
			{
			    GIF_EXIT("DGifGetImageDesc error, GIF_ERROR");
				goto error;
			}
			Row = GifFile->Image.Top;      /* Image Position relative to Screen. */
			Col = GifFile->Image.Left;
			Width = GifFile->Image.Width;
			Height = GifFile->Image.Height;

			if (GifFile->Image.Left + GifFile->Image.Width > GifFile->SWidth ||
			    GifFile->Image.Top + GifFile->Image.Height > GifFile->SHeight) 
			{
			    OSprintf("GIF_ERROR\n");
				goto error;
			}
			if (GifFile->Image.Interlace) 
			{
			    /* Need to perform 4 passes on the images: */
			    for (Count = i = 0; i < 4; i++)
			    {
					for (j = Row + InterlacedOffset[i]; j < Row + Height;
							 j += InterlacedJumps[i]) 
					{
					    if (DGifGetLine(GifFile, &(GifFile->ScreenBuffer)[j][Col], Width) == GIF_ERROR) 
						{
			    			OSprintf("GIF_ERROR");
							goto error;
					    }
					}
			    }
			}
			else 
			{
			    for (i = 0; i < Height; i++)
				{
					if (DGifGetLine(GifFile, &(GifFile->ScreenBuffer)[Row++][Col], Width) == GIF_ERROR)
					{
				    	OSprintf("GIF_ERROR");
						goto error;
					}
			    }
			}

#if 1
			/* Lets dump it - set the global variables required and do it: */
	    	//BackGround = GifFile->SBackGroundColor;
    		ColorMap = (GifFile->Image.ColorMap ? GifFile->Image.ColorMap : GifFile->SColorMap);
    		ColorMapSize = ColorMap->ColorCount;

			output_to_rgb32(GifFile, GifFile->outbuf, GifFile->ScreenBuffer, GifFile->SWidth, GifFile->SHeight);

			GifFile->frame_num++;
#endif
			//break;
			goto end;
	    case EXTENSION_RECORD_TYPE:
			/* Skip any extension blocks in file: */
			if (DGifGetExtension(GifFile, &ExtCode, &Extension) == GIF_ERROR) 
			{
			    OSprintf("GIF_ERROR");
				goto error;
			}

			/* add chengjiali */
			if ((GRAPHICS_EXT_FUNC_CODE == ExtCode) && (Extension != NULL))
			{
				GifFile->Image.transparent_flg = (Extension[1] & 1);
				GifFile->Image.transparent_color_index = Extension[4];
				GifFile->Image.disposal_method = (Extension[1] & 0x1C) >> 2;
				GifFile->Image.delay_time = ((unsigned short)Extension[3] << 8) + Extension[2];
				*delay_time = GifFile->Image.delay_time;
			}

#if 1 // 0
			while (Extension != NULL) 
			{
			    if (DGifGetExtensionNext(GifFile, &Extension) == GIF_ERROR) 
				{
					OSprintf("GIF_ERROR");
					goto error;
			    }
			}
#endif
			break;
	    case TERMINATE_RECORD_TYPE:
			//break;
			return LP_GIF_END;
	    default:		    /* Should be traps by DGifGetRecordType. */
			break;
		}
    }
    while (RecordType != TERMINATE_RECORD_TYPE);

end: 

	IMG_ZOOM_RGB32_TO_RGB32(GifFile->outbuf, output_width, output_width, output_height, 
		                    lp_param->out_buf_y.buf + lp_param->out_pic_pos_y * lp_param->out_buf_width * 4 + lp_param->out_pic_pos_x * 4,
		                    lp_param->out_buf_width, lp_param->out_pic_width, lp_param->out_pic_height);

	return LP_OK;

error:

	if(GifFile->outbuf != NULL)
	{
		lp_param->lp_free(GifFile->outbuf);
		GifFile->outbuf = NULL;
	}

	for (i = 0; i < GifFile->SHeight; i++)
	{  
		if ((GifFile->ScreenBuffer)[i] != NULL)
		{
			lp_param->lp_free((GifFile->ScreenBuffer)[i]);
			(GifFile->ScreenBuffer)[i] = NULL;
		}
    }
	
	if(GifFile->ScreenBuffer != NULL)
	{
		lp_param->lp_free(GifFile->ScreenBuffer);
		GifFile->ScreenBuffer = NULL;
	}

	DGifCloseFile(GifFile);
	return LP_DECODE_ERROR;	
}

DLL_EXPORT LP_RET load_gif_play_next_frame(void *gif_handle, LP_INPUT_PARAM *lp_param, unsigned short *delay_time)
{
	int	i, j, Size, Row, Col, Width, Height, ExtCode, Count;
	GifRecordType RecordType;
    GifByteType *Extension;
    GifFileType *GifFile = (GifFileType *)gif_handle;
	int output_width, output_height;

	int ColorMapSize = 0,    
		InterlacedOffset[] = { 0, 4, 2, 1 }, /* The way Interlaced image should. */    
		InterlacedJumps[] = { 8, 8, 4, 2 };    /* be read - offsets and jumps... */
	
//	OSprintf("start gif_decode\n");

	output_width  = GifFile->SWidth;
	output_height = GifFile->SHeight;

	/* Scan the content of the GIF file and load the image(s) in: */
    do 
	{
		if (DGifGetRecordType(GifFile, &RecordType) == GIF_ERROR) 
		{
			OSprintf("DGifGetRecordType error, GIF_ERROR.");
			goto error;
		}
		switch (RecordType) 
		{
	    case IMAGE_DESC_RECORD_TYPE:
			if (DGifGetImageDesc(GifFile) == GIF_ERROR)
			{
			    GIF_EXIT("DGifGetImageDesc error, GIF_ERROR");
				goto error;
			}
			Row = GifFile->Image.Top;      /* Image Position relative to Screen. */
			Col = GifFile->Image.Left;
			Width = GifFile->Image.Width;
			Height = GifFile->Image.Height;

			if (GifFile->Image.Left + GifFile->Image.Width > GifFile->SWidth ||
			    GifFile->Image.Top + GifFile->Image.Height > GifFile->SHeight) 
			{
			    OSprintf("GIF_ERROR\n");
				goto error;
			}
			if (GifFile->Image.Interlace) 
			{
			    /* Need to perform 4 passes on the images: */
			    for (Count = i = 0; i < 4; i++)
			    {
					for (j = Row + InterlacedOffset[i]; j < Row + Height;
							 j += InterlacedJumps[i]) 
					{
					    if (DGifGetLine(GifFile, &(GifFile->ScreenBuffer)[j][Col], Width) == GIF_ERROR) 
						{
			    			OSprintf("GIF_ERROR");
							goto error;
					    }
					}
			    }
			}
			else 
			{
			    for (i = 0; i < Height; i++)
				{
					if (DGifGetLine(GifFile, &(GifFile->ScreenBuffer)[Row++][Col], Width) == GIF_ERROR)
					{
				    	OSprintf("GIF_ERROR");
						goto error;
					}
			    }
			}

#if 1
			/* Lets dump it - set the global variables required and do it: */
	    	//BackGround = GifFile->SBackGroundColor;
    		ColorMap = (GifFile->Image.ColorMap ? GifFile->Image.ColorMap : GifFile->SColorMap);
    		ColorMapSize = ColorMap->ColorCount;

			output_to_rgb32(GifFile, GifFile->outbuf, GifFile->ScreenBuffer, GifFile->SWidth, GifFile->SHeight);

			GifFile->frame_num++;
#endif
			//break;
			goto end;
	    case EXTENSION_RECORD_TYPE:
			/* Skip any extension blocks in file: */
			if (DGifGetExtension(GifFile, &ExtCode, &Extension) == GIF_ERROR) 
			{
			    OSprintf("GIF_ERROR");
				goto error;
			}

			/* add chengjiali */
			if ((GRAPHICS_EXT_FUNC_CODE == ExtCode) && (Extension != NULL))
			{
				GifFile->Image.transparent_flg = (Extension[1] & 1);
				GifFile->Image.transparent_color_index = Extension[4];
				GifFile->Image.disposal_method = (Extension[1] & 0x1C) >> 2;
				GifFile->Image.delay_time = ((unsigned short)Extension[3] << 8) + Extension[2];
				*delay_time = GifFile->Image.delay_time;
			}

#if 1 // 0
			while (Extension != NULL) 
			{
			    if (DGifGetExtensionNext(GifFile, &Extension) == GIF_ERROR) 
				{
					OSprintf("GIF_ERROR");
					goto error;
			    }
			}
#endif
			break;
	    case TERMINATE_RECORD_TYPE:
			//break;
			return LP_GIF_END;
	    default:		    /* Should be traps by DGifGetRecordType. */
			break;
		}
    }
    while (RecordType != TERMINATE_RECORD_TYPE);

end: 

	IMG_ZOOM_RGB32_TO_RGB32(GifFile->outbuf, output_width, output_width, output_height, 
		                    lp_param->out_buf_y.buf + lp_param->out_pic_pos_y * lp_param->out_buf_width * 4 + lp_param->out_pic_pos_x * 4,
		                    lp_param->out_buf_width, lp_param->out_pic_width, lp_param->out_pic_height);

	return LP_OK;

error:

	if(GifFile->outbuf != NULL)
	{
		lp_param->lp_free(GifFile->outbuf);
		GifFile->outbuf = NULL;
	}

	for (i = 0; i < GifFile->SHeight; i++)
	{  
		if ((GifFile->ScreenBuffer)[i] != NULL)
		{
			lp_param->lp_free((GifFile->ScreenBuffer)[i]);
			(GifFile->ScreenBuffer)[i] = NULL;
		}
    }
	
	if(GifFile->ScreenBuffer != NULL)
	{
		lp_param->lp_free(GifFile->ScreenBuffer);
		GifFile->ScreenBuffer = NULL;
	}
	DGifCloseFile(GifFile);

	return LP_DECODE_ERROR;	
}

DLL_EXPORT LP_RET load_gif_end(void *gif_handle, LP_INPUT_PARAM *lp_param)
{
    GifFileType *GifFile = (GifFileType *)gif_handle;
	int i;
	
	if(GifFile->outbuf != NULL)
	{
		lp_param->lp_free(GifFile->outbuf);
		GifFile->outbuf = NULL;
	}

	for (i = 0; i < GifFile->SHeight; i++)
	{  
		if ((GifFile->ScreenBuffer)[i] != NULL)
		{
			lp_param->lp_free((GifFile->ScreenBuffer)[i]);
			(GifFile->ScreenBuffer)[i] = NULL;
		}
    }
	
	if(GifFile->ScreenBuffer != NULL)
	{
		lp_param->lp_free(GifFile->ScreenBuffer);
		GifFile->ScreenBuffer = NULL;
	}
	DGifCloseFile(GifFile);

	return LP_OK;	
}
#endif
DLL_EXPORT LP_FUNCTIONS lp_functions = 
{
	load_jpeg
	//load_bmp,
	//load_png
	//load_static_gif,
	//load_gif_start,
	//load_gif_play_next_frame,
	//load_gif_end
};

#ifdef WIN32
LP_FUNCTIONS *get_lp_functions()
{
	return &lp_functions;
}
#endif

