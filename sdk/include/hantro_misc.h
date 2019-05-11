#ifndef __HANTRO_MISC__
#define __HANTRO_MISC__

#ifdef __cplusplus
extern "C" {
#endif

#define HM_OUT_FMT_YCBCR_420_SEMIPLANAR      0x020001U
#define HM_OUT_FMT_YCBCR_422_INTERLEAVED     0x010001U

typedef struct tagHantroMiscFunc
{
	int (*pp_copy)(unsigned char *src_buf,
	               unsigned long src_bus_addr,
	               unsigned long src_buf_stride,
	               long src_x,
	               long src_y,
	               unsigned long src_width,
	               unsigned long src_height,
	               unsigned char *dst_buf,
	               unsigned long dst_bus_addr,
	               unsigned long dst_buf_stride,
	               long dst_x,
	               long dst_y);

	int (*pp_ezcopy)(unsigned char *src,
				 	unsigned long src_bus_addr,
				 	unsigned long src_size,
					unsigned long src_width,
					unsigned long src_height,
					unsigned long src_pix_fmt,
					unsigned long crop_x,
					unsigned long crop_y,
					unsigned long crop_width,
					unsigned long crop_height,
					unsigned long dst_bus_addr,
					unsigned long dst_x,
					unsigned long dst_y,
					unsigned long dst_width,
					unsigned long dst_height,
					unsigned long dst_buf_width,
					unsigned long dst_buf_height,
					unsigned long dst_pix_fmt);
	               
	int (*pp_rotate_yuv422)(unsigned char *src_img,
							unsigned long src_bus_addr,
							unsigned long src_width,
							unsigned long src_height,
							unsigned char *dst_img,
							unsigned long dst_bus_addr,
							unsigned long angle);
							
	int (*jpeg_decode)(unsigned char *p_heap_buf, 
					   unsigned long p_heap_bus_addr, 
					   unsigned long p_heap_size, 
					   unsigned char *stream_buf, 
					   unsigned long stream_bus_addr, 
					   unsigned long stream_len,
					   unsigned char *out_buf, 
					   unsigned long out_bus_addr, 
					   unsigned char *out_buf_uv, 
					   unsigned long out_bus_addr_uv, 
					   unsigned long out_buf_width,
					   unsigned long out_buf_height,
					   long out_img_x,
					   long out_img_y,
					   unsigned long out_img_width, 
					   unsigned long out_img_height, 
					   unsigned long out_pix_fmt);
					   
	int (*jpeg_decode_soft)(unsigned char *stream_buf,
	                        int stream_len,
						    unsigned char *out_buf, 
						    unsigned char *out_buf_uv, 
						    int out_buf_width,
						    int out_buf_height,
						    int out_img_x,
						    int out_img_y,
						    int out_pix_fmt);

	int (*jpeg_decode_sw)(unsigned char *stream_buf,
						  int stream_len,
						  unsigned char *out_buf,
						  unsigned char *out_buf_uv,
						  int out_buf_stride,
						  int out_width,
						  int out_height,
						  int out_pix_fmt,
						  int zoom_flg);

	int (*img_cpoy)(unsigned char *src,
				 	unsigned long src_bus_addr,
					unsigned long src_x,
					unsigned long src_y,
					unsigned long src_width,
					unsigned long src_height,
					unsigned long src_buf_width,
					unsigned long src_buf_height,
					unsigned char *dst,
					unsigned long dst_bus_addr,
					unsigned long dst_x,
					unsigned long dst_y,
					unsigned long dst_buf_width,
					unsigned long dst_buf_height);

	void *(*ezcodec_dec_open)(unsigned char *p_heap_buf,
							unsigned long p_heap_bus_addr, 
							unsigned long p_heap_size);

	void (*ezcodec_dec_close)(void *inst);

	int (*ezcodec_decode)(void *inst,
						unsigned char *stream_buf, 
						unsigned long stream_bus_addr,
						unsigned long stream_len,
						unsigned char *out_buf,
						unsigned long out_bus_addr,
						unsigned char *out_buf_uv,
						unsigned long out_bus_addr_uv,
						unsigned long out_buf_width,
						unsigned long out_buf_height,
						long out_img_x,
						long out_img_y,
						unsigned long out_img_width,
						unsigned long out_img_height,
						unsigned long out_pix_fmt);

	void *(*mpeg4_dec_open)(unsigned char *p_heap_buf,
							unsigned long p_heap_bus_addr, 
							unsigned long p_heap_size);

	void (*mpeg4_dec_close)(void *inst);

	int (*mpeg4_decode)(void *inst,
						unsigned char *stream_buf, 
						unsigned long stream_bus_addr,
						unsigned long stream_len,
						unsigned char *out_buf,
						unsigned long out_bus_addr,
						unsigned char *out_buf_uv,
						unsigned long out_bus_addr_uv,
						unsigned long out_buf_width,
						unsigned long out_buf_height,
						long out_img_x,
						long out_img_y,
						unsigned long out_img_width,
						unsigned long out_img_height,
						unsigned long out_pix_fmt);
	
	void *(*h264_dec_open)(unsigned char *p_heap_buf,
							unsigned long p_heap_bus_addr, 
							unsigned long p_heap_size);

	void (*h264_dec_close)(void *inst);

	int (*h264_decode)(void *inst,
						unsigned char *stream_buf, 
						unsigned long stream_bus_addr,
						unsigned long stream_len,
						unsigned char *out_buf,
						unsigned long out_bus_addr,
						unsigned char *out_buf_uv,
						unsigned long out_bus_addr_uv,
						unsigned long out_buf_width,
						unsigned long out_buf_height,
						long out_img_x,
						long out_img_y,
						unsigned long out_img_width,
						unsigned long out_img_height,
						unsigned long out_pix_fmt);
	
	int (*pp_rotate_yuv420)(unsigned char *src_img,
							unsigned long src_bus_addr,
							unsigned long src_width,
							unsigned long src_height,
							unsigned char *dst_img,
							unsigned long dst_bus_addr,
							unsigned long angle);


}HANTRO_MISC_FUNC;

extern HANTRO_MISC_FUNC hm_func;

#ifdef __cplusplus
}
#endif
	          
#endif//__HANTRO_MISC__
