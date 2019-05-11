#include <stdio.h>
#include <string.h>
#include "jpegdecapi.h"
#include "ppapi.h"
#include "dwl.h"
#include "swfdec.h"
#include "osapi.h"
//#include "actions_reg_213x.h"

#define MAX(a,b) (((a) > (b)) ? (a) : (b))

extern unsigned long swf_heap_logic_start;
extern unsigned long swf_heap_physic_start;


#if 0
void reg_delay(void)
{
	volatile int i;
	for(i=0;i<100;i++)
		;
}
void read_modify_clear(int reg, int bit)
{
	unsigned int tmp;
	tmp = *(volatile unsigned int *)reg;
	reg_delay();
	*(volatile unsigned int *)reg = tmp&(~(1<<bit));
	reg_delay();
}
void read_modify_set(int reg, int bit)
{
	unsigned int tmp;
	tmp = *(volatile unsigned int *)reg;
	reg_delay();
	*(volatile unsigned int *)reg = tmp|(1<<bit);
	reg_delay();


}
void hantor_clk_ctl(int open)
{
	if(open)
		read_modify_set(CMU_DEVCLKEN, 15);
	else
		read_modify_clear(CMU_DEVCLKEN, 15);
}
#endif

static void decRet(JpegDecRet ret)
{
	switch (ret)
	{
	case JPEGDEC_SLICE_READY:
		printf("JPEGDEC_SLICE_READY\n");
		break;
	case JPEGDEC_FRAME_READY:
		printf("JPEGDEC_FRAME_READY\n");
		break;
	case JPEGDEC_STRM_PROCESSED:
		printf("JPEGDEC_STRM_PROCESSED\n");
		break;
	case JPEGDEC_OK:
		printf("JPEGDEC_OK\n");
		break;
	case JPEGDEC_ERROR:
		printf("JPEGDEC_ERROR\n");
		break;
	case JPEGDEC_UNSUPPORTED:
		printf("JPEGDEC_UNSUPPORTED\n");
		break;
	case JPEGDEC_PARAM_ERROR:
		printf("JPEGDEC_PARAM_ERROR\n");
		break;
	case JPEGDEC_MEMFAIL:
		printf("JPEGDEC_MEMFAIL\n");
		break;
	case JPEGDEC_INITFAIL:
		printf("JPEGDEC_INITFAIL\n");
		break;
	case JPEGDEC_INVALID_STREAM_LENGTH:
		printf("JPEGDEC_INVALID_STREAM_LENGTH\n");
		break;
	case JPEGDEC_STRM_ERROR:
		printf("JPEGDEC_STRM_ERROR\n");
		break;
	case JPEGDEC_INVALID_INPUT_BUFFER_SIZE:
		printf("JPEGDEC_INVALID_INPUT_BUFFER_SIZE\n");
		break;
	case JPEGDEC_HW_RESERVED:
		printf("JPEGDEC_HW_RESERVED\n");
		break;
	case JPEGDEC_DWL_HW_TIMEOUT:
		printf("JPEGDEC_DWL_HW_TIMEOUT\n");
		break;
	case JPEGDEC_DWL_ERROR:
		printf("JPEGDEC_DWL_ERROR\n");
		break;
	case JPEGDEC_HW_BUS_ERROR:
		printf("JPEGDEC_HW_BUS_ERROR\n");
		break;
	case JPEGDEC_SYSTEM_ERROR:
		printf("JPEGDEC_SYSTEM_ERROR\n");
		break;
	default:
		printf("Other %d\n", ret);
		break;
	}
}

static void print_pp_cfg(PPConfig *pp_cfg)
{
	printf("\tppInImg.width:%d\n",pp_cfg->ppInImg.width);
	printf("\tppInImg.height:%d\n",pp_cfg->ppInImg.height);
	printf("\tppInImg.pixFormat:0x%08x, ",pp_cfg->ppInImg.pixFormat);
	switch(pp_cfg->ppInImg.pixFormat)
	{
		case PP_PIX_FMT_YCBCR_4_0_0:
			printf("YCBCR_4_0_0\n");
			break;
		case PP_PIX_FMT_YCBCR_4_4_4_SEMIPLANAR:
			printf("YCBCR_4_4_4_SEMIPLANAR\n");
			break;
		case PP_PIX_FMT_YCBCR_4_2_2_INTERLEAVED:
			printf("YCBCR_4_2_2_INTERLEAVED\n");
			break;
		case PP_PIX_FMT_YCBCR_4_2_2_SEMIPLANAR:
			printf("YCBCR_4_2_2_SEMIPLANAR\n");
			break;
		case PP_PIX_FMT_YCBCR_4_4_0_SEMIPLANAR:
			printf("YCBCR_4_4_0_SEMIPLANAR\n");
			break;
		case PP_PIX_FMT_YCBCR_4_2_0_PLANAR:
			printf("YCBCR_4_2_0_PLANAR\n");
			break;
		case PP_PIX_FMT_YCBCR_4_2_0_SEMIPLANAR:
			printf("YCBCR_4_2_0_SEMIPLANAR\n");
			break;
		case PP_PIX_FMT_RGB16_5_5_5:
			printf("RGB16_5_5_5\n");
			break;
		case PP_PIX_FMT_RGB16_5_6_5:
			printf("RGB16_5_6_5\n");
			break;
		case PP_PIX_FMT_RGB32:
			printf("RGB32\n");
			break;
		default:
			printf("unsupport!\n");
	}
	printf("\tppInImg.videoRange:%d\n",					pp_cfg->ppInImg.videoRange);
	printf("\tppInImg.bufferBusAddr:0x%08X\n",			pp_cfg->ppInImg.bufferBusAddr);
	printf("\tppInImg.bufferCbBusAddr:0x%08X\n",		pp_cfg->ppInImg.bufferCbBusAddr);
	printf("\tppInImg.bufferCrBusAddr:0x%08X\n",		pp_cfg->ppInImg.bufferCrBusAddr);
	printf("\tppInImg.vc1MultiResEnable:%d\n",			pp_cfg->ppInImg.vc1MultiResEnable);
	printf("\tppInImg.vc1RangeRedFrm:%d\n",				pp_cfg->ppInImg.vc1RangeRedFrm);
	printf("\tppInCrop.enable:%d\n",					pp_cfg->ppInCrop.enable);
	printf("\tppInCrop.originX:%d\n",					pp_cfg->ppInCrop.originX);
	printf("\tppInCrop.originY:%d\n",					pp_cfg->ppInCrop.originY);
	printf("\tppInCrop.width:%d\n",						pp_cfg->ppInCrop.width);
	printf("\tppInCrop.height:%d\n",					pp_cfg->ppInCrop.height);
	printf("\tppInRotation.rotation:%d\n",				pp_cfg->ppInRotation.rotation);
	printf("\tppOutImg.width:%d\n",						pp_cfg->ppOutImg.width);
	printf("\tppOutImg.height:%d\n",					pp_cfg->ppOutImg.height);
//	printf("\tppOutImg.pixFormat:0x%08x\n",				pp_cfg->ppOutImg.pixFormat);
	printf("\tppOutImg.pixFormat:0x%08x, ",				pp_cfg->ppOutImg.pixFormat);
	switch(pp_cfg->ppOutImg.pixFormat)
	{
		case PP_PIX_FMT_YCBCR_4_2_2_INTERLEAVED:
			printf("YCBCR_4_2_2_INTERLEAVED\n");
			break;
		case PP_PIX_FMT_YCBCR_4_2_0_SEMIPLANAR:
			printf("YCBCR_4_2_0_SEMIPLANAR\n");
			break;
		case PP_PIX_FMT_RGB16_CUSTOM:
			printf("RGB16_CUSTOM\n");
			break;
		case PP_PIX_FMT_RGB16_5_5_5:
			printf("RGB16_5_5_5\n");
			break;
		case PP_PIX_FMT_RGB16_5_6_5:
			printf("RGB16_5_6_5\n");
			break;
		case PP_PIX_FMT_BGR16_5_5_5:
			printf("BGR16_5_5_5\n");
			break;
		case PP_PIX_FMT_BGR16_5_6_5:
			printf("BGR16_5_6_5\n");
			break;
		case PP_PIX_FMT_RGB32_CUSTOM:
			printf("RGB32_CUSTOM\n");
			break;
		case PP_PIX_FMT_RGB32:
			printf("RGB32\n");
			break;
		case PP_PIX_FMT_BGR32:
			printf("BGR32\n");
			break;
		default:
			printf("unsupport!\n");
	}
	printf("\tppOutImg.bufferBusAddr:0x%08X\n",			pp_cfg->ppOutImg.bufferBusAddr);
	printf("\tppOutImg.bufferChromaBusAddr:0x%08X\n",	pp_cfg->ppOutImg.bufferChromaBusAddr);
	printf("\tppOutImg.maskAlphaColorDepth:%d\n",		pp_cfg->ppOutImg.maskAlphaColorDepth);
	printf("\tppOutRgb.rgbTransform:%d\n",				pp_cfg->ppOutRgb.rgbTransform);
	printf("\tppOutRgb.brightness:%d\n",				pp_cfg->ppOutRgb.brightness);
	printf("\tppOutRgb.contrast:%d\n",					pp_cfg->ppOutRgb.contrast);
	printf("\tppOutRgb.saturation:%d\n",				pp_cfg->ppOutRgb.saturation);
	printf("\tppOutRgb.alpha:%d\n",						pp_cfg->ppOutRgb.alpha);
	printf("\tppOutRgb.transparency:%d\n",				pp_cfg->ppOutRgb.transparency);
	printf("\tppOutMask1.enable:%d\n",					pp_cfg->ppOutMask1.enable);
	printf("\tppOutMask1.originX:%d\n",					pp_cfg->ppOutMask1.originX);
	printf("\tppOutMask1.originY:%d\n",					pp_cfg->ppOutMask1.originY);
	printf("\tppOutMask1.width:%d\n",					pp_cfg->ppOutMask1.width);
	printf("\tppOutMask1.height:%d\n",					pp_cfg->ppOutMask1.height);
	printf("\tppOutMask1.alphaBlendEna:%d\n",			pp_cfg->ppOutMask1.alphaBlendEna);
	printf("\tppOutMask1.blendComponentBase:0x%08X\n",	pp_cfg->ppOutMask1.blendComponentBase);
	printf("\tppOutMask1.fixedAlpha:%d\n",				pp_cfg->ppOutMask1.fixedAlpha);
	printf("\tppOutMask1.alpha:%d\n",					pp_cfg->ppOutMask1.alpha);
	printf("\tppOutMask2.enable:%d\n",					pp_cfg->ppOutMask2.enable);
	printf("\tppOutMask2.originX:%d\n",					pp_cfg->ppOutMask2.originX);
	printf("\tppOutMask2.originY:%d\n", 				pp_cfg->ppOutMask2.originY);
	printf("\tppOutMask2.width:%d\n",					pp_cfg->ppOutMask2.width);
	printf("\tppOutMask2.height:%d\n",					pp_cfg->ppOutMask2.height);
	printf("\tppOutMask2.alphaBlendEna:%d\n",			pp_cfg->ppOutMask2.alphaBlendEna);
	printf("\tppOutMask2.blendComponentBase:0x%08X\n",	pp_cfg->ppOutMask2.blendComponentBase);
	printf("\tppOutMask2.fixedAlpha:%d\n",				pp_cfg->ppOutMask2.fixedAlpha);
	printf("\tppOutMask2.alpha:%d\n",					pp_cfg->ppOutMask2.alpha);
	printf("\tppOutFrmBuffer.enable:%d\n",				pp_cfg->ppOutFrmBuffer.enable);
	printf("\tppOutFrmBuffer.writeOriginX:%d\n",		pp_cfg->ppOutFrmBuffer.writeOriginX);
	printf("\tppOutFrmBuffer.writeOriginY:%d\n",		pp_cfg->ppOutFrmBuffer.writeOriginY);
	printf("\tppOutFrmBuffer.frameBufferWidth:%d\n",	pp_cfg->ppOutFrmBuffer.frameBufferWidth);
	printf("\tppOutFrmBuffer.frameBufferHeight:%d\n",	pp_cfg->ppOutFrmBuffer.frameBufferHeight);
}


int hantro_jpeg(u8 * jpeg_data, i32 jpeg_size, u8** pixel, int * width, int * height, int * stride, int format)
{
	JpegDecInst dec_inst;
    JpegDecImageInfo DecInfo, *dec_info = &DecInfo;
    JpegDecInput DecInput, *dec_input = &DecInput;
	JpegDecOutput DecOutput, *dec_output = &DecOutput;
	JpegDecRet jpeg_ret;
	JpegDecInitParam_t jpeg_param;
	PPInitParam_t pp_param;

	PPConfig PPConfig, *pp_cfg = &PPConfig;
	PPInst pp_inst = NULL;
	u32 amountOfMCUs, mcuInRow;
	PPResult pp_ret;
	i32 pp_pipeline_flg = 0;
	u32 pp_format;
	u32 w, h;
	u8 *pData=jpeg_data;
	i32 DataSize=jpeg_size;

	/**
	* for heap use.
	*/
	void *jpegheap = NULL;
	u32 jpegheap_bus_addr;
	u32 jpegheap_size;
	void *ph = NULL;

	/**
	* Some jpeg in swf may start with "ff d9 ff d8 ff d8".
	* But we could not decode jpeg with this pattern, so 
	* here we should discard the starting 4bytes.
	*/
	
	if((*pData)==0xFF && (*(pData+1))==0xD9 && \
		(*(pData+2))==0xFF && (*(pData+3))==0xD8 && \
		(*(pData+4))==0xFF && (*(pData+5))==0xD8){
		pData+=4;
		DataSize-=4;
	}

	/**
	* malloc input buffer, must be physical continuous
	*/
	u32 *stream_input = (u32*)SWF_Malloc(DataSize);
	if(stream_input == NULL)
	{
		return 0;
	}

	/**
	* malloc heap for jpeg decode.
	*/
	jpegheap_size = 60*1024;
	jpegheap = SWF_Malloc(jpegheap_size);
	if(jpegheap == NULL){
		SWF_Free(stream_input);
		return 0;
	}
	jpegheap_bus_addr = (u32)(swf_heap_physic_start + ((u32)jpegheap-swf_heap_logic_start))&0x1fffffff;
	ph = OSHCreate(&jpegheap, (unsigned long *)&jpegheap_bus_addr, (int *)&jpegheap_size, 1, 32);
	if(ph == NULL){
		SWF_Free(stream_input);
		SWF_Free(jpegheap);
		return 0;
	}
	jpeg_param.pHeap = ph;
	jpeg_param.hw_reset_enable = 1;
	jpeg_param.mem_usage_trace = 0;
	jpeg_param.reg_rw_trace = 0;

	pp_param.pHeap = ph;
	pp_param.hw_reset_enable = 1;
	pp_param.mem_usage_trace = 0;
	pp_param.reg_rw_trace = 0;

	memset(dec_info, 0, sizeof(JpegDecImageInfo));
	memset(dec_input, 0, sizeof(JpegDecInput));
	memcpy(stream_input, pData, DataSize);

	dec_input->streamBuffer.pVirtualAddress = stream_input;
	dec_input->streamBuffer.busAddress = ((u32)(swf_heap_physic_start + ((u32)stream_input-swf_heap_logic_start)))&0x1fffffff;
	dec_input->streamLength = DataSize;
	dec_input->bufferSize   = (DataSize + 1023) / 1024 * 1024;
	if(dec_input->bufferSize < 128 * 1024)
	{
		dec_input->bufferSize = 128 * 1024;
	}

	memset(&dec_inst, 0, sizeof(JpegDecInst));
	
	jpeg_ret = JpegDecInit(&dec_inst,&jpeg_param);
	if(jpeg_ret != JPEGDEC_OK)
	{
		printf("JPEG DECODER INITIALIZATION FAILED, %d\n", jpeg_ret);
		goto end;
	}

	jpeg_ret = JpegDecGetImageInfo(dec_inst, dec_input, dec_info);
	if(jpeg_ret != JPEGDEC_OK)
	{
		decRet(jpeg_ret);
		printf("get image info error\n");
		goto end;
	}

	w = (*width + 15) / 16 * 16;
	h = (*height + 15) / 16 * 16;


	if(format == SWF_BMP_FMT_YUV422)
	{
		*pixel = SWF_Malloc(w * h * 2);
		pp_format = PP_PIX_FMT_YCBCR_4_2_2_INTERLEAVED;
	}
	else
	{
		*pixel = SWF_Malloc(w * h * 4);
		pp_format = PP_PIX_FMT_RGB32;
	}
	if(*pixel == NULL)
	{
		goto end;
	}

	*stride = w;
	*width  = w * dec_info->displayWidth / dec_info->outputWidth;
	*height = h * dec_info->displayHeight / dec_info->outputHeight;

	memset(dec_output, 0, sizeof(JpegDecOutput));
	memset(pp_cfg, 0, sizeof(PPConfig));

	pp_ret = PPInit(&pp_inst,&pp_param);
	if(pp_ret != PP_OK)
	{
		printf("PP instance init error:%d\n", pp_ret);
		goto end;
	}

	pp_ret = PPDecPipelineEnable(pp_inst, dec_inst, PP_PIPELINED_DEC_TYPE_JPEG);
	if(pp_ret != PP_OK)
	{
		printf("Enable PP Pipeline Error:%d\n", pp_ret);
		goto end;
	}
	pp_pipeline_flg = 1;

	pp_ret = PPGetConfig(pp_inst, pp_cfg);
	if(pp_ret != PP_OK)
	{
		printf("PPGetConfig error:%d\n", pp_ret);
		print_pp_cfg(pp_cfg);
		goto end;
	}


	if(dec_info->outputFormat == JPEGDEC_YCbCr400)
	{
		pp_cfg->ppInImg.pixFormat = PP_PIX_FMT_YCBCR_4_0_0;
		amountOfMCUs = (dec_info->outputWidth * dec_info->outputHeight) / 64;
		mcuInRow = dec_info->outputWidth / 8;
	}
	else if(dec_info->outputFormat == JPEGDEC_YCbCr420_SEMIPLANAR)
	{
		pp_cfg->ppInImg.pixFormat = PP_PIX_FMT_YCBCR_4_2_0_SEMIPLANAR;
		amountOfMCUs = (dec_info->outputWidth * dec_info->outputHeight) / 256;
		mcuInRow = dec_info->outputWidth / 16;
	}
	else if(dec_info->outputFormat == JPEGDEC_YCbCr422_SEMIPLANAR)
	{
		pp_cfg->ppInImg.pixFormat = PP_PIX_FMT_YCBCR_4_2_2_SEMIPLANAR;
		amountOfMCUs = (dec_info->outputWidth * dec_info->outputHeight) / 128;
		mcuInRow = dec_info->outputWidth / 16;
	}
	else if(dec_info->outputFormat == JPEGDEC_YCbCr440_SEMIPLANAR)
	{
		pp_cfg->ppInImg.pixFormat = PP_PIX_FMT_YCBCR_4_4_0_SEMIPLANAR;
		amountOfMCUs = (dec_info->outputWidth * dec_info->outputHeight) / 128;
		mcuInRow = dec_info->outputWidth / 16;
	}
	else if(dec_info->outputFormat == JPEGDEC_YCbCr444_SEMIPLANAR)
	{
		pp_cfg->ppInImg.pixFormat = PP_PIX_FMT_YCBCR_4_4_4_SEMIPLANAR;
		amountOfMCUs = (dec_info->outputWidth * dec_info->outputHeight) / 64;
		mcuInRow = dec_info->outputWidth / 8;
	}
	else
	{
		printf("unknown jpeg output format:%d\n", dec_info->outputFormat);
		goto end;
	}
	dec_input->sliceMbSet = 0;
	dec_input->sliceMbSet = 0;
	if(amountOfMCUs > 4096)
	{
		dec_input->sliceMbSet = 4096/mcuInRow;
	}
	dec_input->decImageType = JPEGDEC_IMAGE;

	pp_cfg->ppInImg.bufferBusAddr = 0;
	pp_cfg->ppInImg.bufferCbBusAddr = 0;
	pp_cfg->ppInImg.bufferCrBusAddr = 0;
	pp_cfg->ppInImg.width = dec_info->outputWidth;
	pp_cfg->ppInImg.height = dec_info->outputHeight;
	pp_cfg->ppInImg.videoRange = 1;
	pp_cfg->ppInCrop.enable = 0;
	pp_cfg->ppInRotation.rotation = PP_ROTATION_NONE;
	pp_cfg->ppOutImg.width = w;
	pp_cfg->ppOutImg.height = h;
	pp_cfg->ppOutImg.pixFormat = pp_format;
	//pp_cfg->ppOutImg.bufferBusAddr = (u32)*pixel & 0xFFFFFFF;
	pp_cfg->ppOutImg.bufferBusAddr = (u32)((swf_heap_physic_start + ((u32)*pixel-swf_heap_logic_start)))&0x1fffffff;
	pp_cfg->ppOutImg.bufferChromaBusAddr = 0;
	pp_cfg->ppOutRgb.rgbTransform = PP_YCBCR2RGB_TRANSFORM_BT_601;
	pp_cfg->ppOutRgb.alpha = 0xFF;
	pp_cfg->ppOutMask1.enable = 0;
	pp_cfg->ppOutMask2.enable = 0;
	pp_cfg->ppOutFrmBuffer.enable = 0;
	pp_ret = PPSetConfig(pp_inst, pp_cfg);
	if(pp_ret != PP_OK)
	{
		printf("PPSetConfig error:%d\n", pp_ret);
		SWF_Free(*pixel);
		print_pp_cfg(pp_cfg);
		*pixel = NULL;
		goto end;
	}
	do 
	{	
		jpeg_ret = JpegDecDecode(dec_inst, dec_input, dec_output);
		

		switch(jpeg_ret)
		{
		case JPEGDEC_FRAME_READY:
			break;
		case JPEGDEC_SLICE_READY:
			break;
		default:
			decRet(jpeg_ret);
			SWF_Free(*pixel);
			*pixel = NULL;
			goto end;
		}
	}
	while(jpeg_ret != JPEGDEC_FRAME_READY);

end:
	
	if(pp_inst != NULL)
	{
		if(pp_pipeline_flg){
			PPDecPipelineDisable(pp_inst, dec_inst);
		}
		PPRelease(pp_inst);
	}
	JpegDecRelease(dec_inst);
	
	SWF_Free(stream_input);
	
	OSHDel(ph);
	SWF_Free(jpegheap);
	
	return 0;
}


