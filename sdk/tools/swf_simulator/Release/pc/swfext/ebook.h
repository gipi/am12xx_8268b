#ifndef SWF_EBOOK_H
#define SWF_EBOOK_H

#include "fui_input.h"

#define MAX_BUF_SIZE  512




typedef enum
{
	EBOOK_MBCS = 0,
	EBOOK_U16_BIG,//unicode big endian
	EBOOK_U16_LIT,//unicode little endian
	EBOOK_UTF8
}txt_type_t;


typedef struct{
	stream_input_t *pb;
	txt_type_t  type;
	INT32U		nMagicLen;   

	INT32U	TxtW;
	INT8U	*lpszShowBuf;		

	INT32S nLinePerPage;
	INT32S nTotalLine;
	INT32S nCurLine;
}ebook_info_t;



#endif
