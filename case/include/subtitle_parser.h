#ifndef _SUBTITLE_PARSER_H_
#define _SUBTITLE_PARSER_H_

/**
*@addtogroup subtitle_parser_lib_s
*@{
*/

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h> 


#define SUBTITLE_UNI16_LIT	0x1000	///< file format: U16 litter endian ff fe
#define SUBTITLE_UNI16_BIG	0x2000	///< file format: U16 big endian fe ff	
#define SUBTITLE_UTF8		0x4000	///< file format: UTF-8 ef bb bf	
#define SUBTITLE_MBCS		0x8000	///< file format: multi byte character set
#define SUBTITLE_FONT_SIZE  32      //the size of subtitle font,setting by user
enum file_type_e{
	FILE_INVAILD,			///< invaild file 
	FILE_SRT,				///< file extension: SRT
	FILE_SSA,				///< file extension: SSA
	FILE_SUB,				///< file extension: SUB/IDX
	FILE_SMI,				///< file extension: SMI
};

typedef struct stream_input{
	void* (*open)(char *path,char* mode);
	int (*close)(void*fp);
	long (*read)(void*fp,unsigned char* buf,unsigned long nbytes);
	long (*write)(void*fp,unsigned char* buf,unsigned long nbytes);
	long (*seek_set)(void*fp,long offset);	
	long (*seek_cur)(void*fp,long offset);	
	long (*seek_end)(void*fp,long offset);	
	long (*tell)(void*fp);
	void* (*malloc)(int size);
	void* (*realloc)(void* buf,unsigned size);
	void (*free)(void* pfree);
	int (*osfeof)(void*fp);
}stream_input_t;		///< file operations

struct time_line{
	unsigned char hour;
	unsigned char min;
	unsigned char sec;
	short unsigned int millsec;
};

typedef struct line_info_s{
	struct time_line t_start;			///<	 the time when the subtitle starts 
	struct time_line t_end;			///<	 the time when the subtitle ends 
	int len;							///<	 the length of string to be show
	int img_w;						///<	 only used in vobsub file
	int img_h;						///<	 only used in vobsub file
	union{
		char* str_show;				///<	 get from middle ware
		int f_pos;					///<	 log where the string to be showed in the file
	};

}line_info_t;

typedef struct memres_s{
	char* memaddr;
	unsigned long memlen;
}memres_t;

typedef struct parser_info_s{
	void *fp;						///< the pointer of the file
	void *fp1;					///< the pointer of the other file
	void *finfo;					///< the info get from the file,maybe include font info and others
	unsigned int ftype;			///< which type the file is,FILE_SRT etc
	stream_input_t fops;			///< the opeartions of file
	void *curline;				///< the current line info
	void *(*get_file_info)(struct parser_info_s *parser_info);	///< for getting the file infomation
	int (*depose_res)(struct parser_info_s *parser_info);		///< for release the resource 
}parser_info_t;


/**
*@brief initial an instance
*
*@param[in] file	: the file to be added
*@param[in] parser_info : the structure of parser_info_t to be filled
*@retval 0 : success
*@retval -1 : error
*@see the definition of parser_info_t structure
*/
int init_instance(char*file,parser_info_t *parser_info);


/**
*@brief remove an instance
*
*@param[in] parser_info : pointer to the parser_info_t structure
*@retval 0
*@see the definition of parser_info_t structure
*/
int remove_instance(parser_info_t *parser_info);	


/**
*@brief parser the file
*
*@param[in] parser_info : pointer to the parser_info_t structure
*@retval 0
*@see the definition of parser_info_t structure
*/
int subtitle_parser(parser_info_t *parser_info);


/**
*@brief get the subtitle of a line
*
*@param[in] parser_info : pointer to the parser_info_t structure
*@param[in] strbuf :  the buffet to store content of the subtitle
*@param[in] len : the length of the buffer 
*@return a pointer to the line_info_t structure
*@see the definition of line_info_t structure
*/
line_info_t get_next_line(parser_info_t *parser_info,char* strbuf,int len);


/**
*@brief query the infomation of a line
*
*@param[in] parser_info : pointer to the parser_info_t structure
*@param[in] time_query :  the time stamp which will be query
*@param[in] strbuf :  the buffet to store content of the subtitle
*@param[in] len : the length of the buffer 
*@return a pointer to the line_info_t structure
*@see the definition of line_info_t structure
*/
line_info_t query_line_info(parser_info_t *parser_info,struct time_line *time_query,char* strbuf,int len);

/**
 *@}
 */

#endif
