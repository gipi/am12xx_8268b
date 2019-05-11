
#ifndef __BUF_ENCODER_H__
#define __BUF_ENCODER_H__

#ifdef __cplusplus
extern "C" {
#endif

//编码
/******************************************************************************
//支持的编码数据的格式
#define JPEG_INPUT_PIX_FMT_YUV_420_PLANAR		1
#define JPEG_INPUT_PIX_FMT_YUV_420_SEMIPLANAR	2
#define JPEG_INPUT_PIX_FMT_RGB16_565			3
#define JPEG_INPUT_PIX_FMT_YCbCr422_INTERLEAVED 4
#define JPEG_INPUT_PIX_FMT_RGB24_888			5

int jpeg_encode(void *buffer, void *JPGFileBuffer, char Level, int *size, int w, int h, int type, int dest)

Input:    buffer          -要编码的数据地址
          Level		      -编码的jpg质量[0-5]
          size		      -生成的jpg的大小
          x			      -buffer(生成图片)的宽(像素为单位)
          y			      -buffer(生成图片)的高(像素为单位)
          type		      -参见上面宏定义的数据类型
	      dest            -0，输出到buffer，1，输出到文件。
	   			          相应的JPGFileBuffer分别为输出buffer地址和输出文件路径
          
Return：0-成功   -1-失败
*******************************************************************************/

__attribute__ ((visibility("default"))) 
int jpeg_encode(void *JPGFileBuffer,
				void *buffer,
				void *buffer_u,
				void *buffer_v,
				int buffer_stride,
				int pixel_format,
				int crop_x,
				int crop_y,
				int crop_width,
				int crop_height,
				char Level,
				int *size,
				int dest);

#ifdef __cplusplus
}
#endif

#endif//__BUF_ENCODER_H__
