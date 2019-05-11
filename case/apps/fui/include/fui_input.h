#ifndef FUI_INPUT
#define FUI_INPUT

/*
* @brief stream input wrapper layer for fui local input.
*
*/


/** get file information */
typedef struct{
	char *extension;
	unsigned int file_len;
}file_info_t;	

/** disk manager seek */
#define		DSEEK_SET		0x01
#define		DSEEK_END		0x02
#define		DSEEK_CUR		0x03

typedef struct stream_input_s{
	int (*read)(struct stream_input_s *input,unsigned int buf,unsigned int len);
	int (*write)(struct stream_input_s *input,unsigned int buf,unsigned int len);
	int (*seek)(struct stream_input_s *input,int offset,int original);		
	int (*tell)(struct stream_input_s *input);
	int (*get_file_info)(struct stream_input_s *input,file_info_t *info);
	int (*dispose)(struct stream_input_s *input);
	int (*testfile)(struct stream_input_s *input);
	int (*seek_64)(struct stream_input_s *input,long long offset,int original);		
	long long (*tell_64)(struct stream_input_s *input);
}stream_input_t;


stream_input_t * create_fui_input(char * filename);

stream_input_t * create_fui_output(char * filename);

#endif
