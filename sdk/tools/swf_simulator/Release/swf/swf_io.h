#ifndef SWF_IO_H
#define SWF_IO_H

#include "swf_base.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UI_TO_SI(SI,UI,shiftbits) \
{\
	SI=UI<<shiftbits;\
	SI=SI>>shiftbits;\
}

#define FLOAT_TO_FIXED(FixedVal,FloatVal,shiftbits)\
{\
	FixedVal=FloatVal<<shiftbits;\
	FixedVal>>=shiftbits;\
}

#define ZGET_RGB(val, s)\
{\
	unsigned char c[3];\
	gzread((s)->file, c, 3);\
	val = 0xFF000000 | (c[0]<<16) | (c[1]<<8) | c[2];\
}
#define ZGET_BLOCK(val,size,s)\
{\
	gzread((s)->file, val, size);\
}
#define ZGET_DWORD(val,s)\
{\
	unsigned int c;\
	gzread((s)->file, &c, 4);\
	val = c;\
}
#define ZGET_WORD(val,s)\
{\
	unsigned short c;\
	gzread((s)->file, &c, 2);\
	val = c;\
}
#define ZGET_BYTE(val,s)\
{\
	unsigned char c;\
	gzread((s)->file, &c, 1);\
	val = c;\
}
#define ZBITS(s, n)\
((n == 0) ? 0 : gzreadbits((s)->file, n, ORDER_MSB))

#define ZBITS_CLEAR(s)\
	gzreadbits((s)->file, 0, ORDER_MSB)

/************************************************************************
#define ZGET_ARGB(val, s)\
{\
	unsigned char c[4];\
	gzread((s)->file, c, 4);\
	val = (c[1]<<24) | (c[2]<<16) | (c[3]<<8) | c[0];\
}
#define ZGET_RGBA(val, s)\
{\
	unsigned char c[4];\
	gzread((s)->file, c, 4);\
	val = (c[0]<<24) | (c[1]<<16) | (c[2]<<8) | c[3];\
}
************************************************************************/

#define SKIP_N(n, s)\
	(s)->ptr += n

#define GET_BLOCK(val,size,s)\
{\
	memcpy(val, &(s)->data[(s)->ptr], size);\
	(s)->ptr += size;\
}
#define GET_DWORD(val,s)\
{\
	val = (s)->data[(s)->ptr] | ((s)->data[(s)->ptr+1] << 8) | ((s)->data[(s)->ptr+2] << 16) | ((s)->data[(s)->ptr+3] << 24);\
	(s)->ptr += 4;\
}
#define GET_WORD(val,s)\
{\
	val = (s)->data[(s)->ptr] | ((s)->data[(s)->ptr+1] << 8);\
	(s)->ptr += 2;\
}
#define GET_BYTE(val,s)\
{\
	val = (s)->data[(s)->ptr++];\
}

#define GET_DWORD_BE(val,s)\
{\
	val = ((s)->data[(s)->ptr] << 24) | ((s)->data[(s)->ptr+1] << 16) | ((s)->data[(s)->ptr+2] << 8) | ((s)->data[(s)->ptr+3]);\
	(s)->ptr += 4;\
}
#define GET_WORD_BE(val,s)\
{\
	val = ((s)->data[(s)->ptr] << 8 )| ((s)->data[(s)->ptr+1] );\
	(s)->ptr += 2;\
}


#define GET_RGB(val, s)\
{\
	val = 0xFF000000 | ((s)->data[(s)->ptr]<<16) | ((s)->data[(s)->ptr+1]<<8) | ((s)->data[(s)->ptr+2]);\
	(s)->ptr += 3;\
}

#define GET_ARGB(val, s)\
{\
	val = ((s)->data[(s)->ptr]<<24) | ((s)->data[(s)->ptr+1]<<16) | ((s)->data[(s)->ptr+2]<<8) | (s)->data[(s)->ptr+3];\
	(s)->ptr += 4;\
}

#define GET_RGBA(val, s)\
{\
	val = ((s)->data[(s)->ptr]<<16) | ((s)->data[(s)->ptr+1]<<8) | ((s)->data[(s)->ptr+2]) | ((s)->data[(s)->ptr+3]<<24);\
	(s)->ptr += 4;\
}

#define GET_STRING(str, s)\
{\
	int _i = 0;\
	while(_i < AS_MAX_TEXT && ((str)[_i++] = (s)->data[(s)->ptr++]) != 0);\
	if(_i == AS_MAX_TEXT) (str)[AS_MAX_TEXT-1] = 0;\
}

#define CONV_FLOAT(val, data)\
{\
	union {int i;float f;} conv;\
	conv.i = ((data)[3] << 24) | ((data)[2] << 16) | ((data)[1] << 8) | (data)[0];\
	val = conv.f;\
}

#define CONV_DOUBLE(val, data)\
{\
	union {int i[2];double d;} conv;\
	conv.i[1] = ((data)[3] << 24) | ((data)[2] << 16) | ((data)[1] << 8) | (data)[0];\
	conv.i[0] = ((data)[7] << 24) | ((data)[6] << 16) | ((data)[5] << 8) | (data)[4];\
	val = conv.d;\
}

#define CONV_DOUBLE_BE(val, data)\
{\
	union {int i[2];double d;} conv;\
	conv.i[1] = ((data)[0] << 24) | ((data)[1] << 16) | ((data)[2] << 8) | (data)[3];\
	conv.i[0] = ((data)[4] << 24) | ((data)[5] << 16) | ((data)[6] << 8) | (data)[7];\
	val = conv.d;\
}


#ifndef SEEK_SET
#define SEEK_SET        0               /* seek to an absolute position */
#define SEEK_CUR        1               /* seek relative to current position */
#define SEEK_END        2               /* seek relative to end of file */
#endif

#define PUT_BYTE(pb, val) \
{\
	INT8U i = (val) & 0xFF;\
	(pb)->write((pb), (unsigned int)&i, 1);\
}

#define PUT_WORD_BE(pb, val) \
{\
	INT8U i[2];\
	i[0] = ((val) >> 8) & 0xFF;\
	i[1] = (val) & 0xFF;\
	(pb)->write((pb), (unsigned int)&i[0], 2);\
}

#define PUT_DWORD_BE(pb, val) \
{\
	INT8U i[4];\
	i[0] = ((val) >> 24) & 0xFF;\
	i[1] = ((val) >> 16) & 0xFF;\
	i[2] = ((val) >> 8) & 0xFF;\
	i[3] = (val) & 0xFF;\
	(pb)->write((pb),(unsigned int) &i[0], 4);\
}

#define PUT_DOUBLE_BE(pb, val)\
{\
	union {INT8U i[8];double d;} conv;\
	int i;\
	INT8U temp;	\
	conv.d = val;\
	for(i = 0; i < 4; i++)\
	{\
	temp = conv.i[i];\
	conv.i[i]= conv.i[7 - i];\
	conv.i[7 - i] = temp;\
	}\
	(pb)->write((pb), (unsigned int)&conv.i[0], 8);\
}

#define PUT_STRING(pb, str)\
{\
	(pb)->write((pb), (unsigned int)&(str)[0], strlen(str));\
}

#define GET_U32(data)\
	(((unsigned char*)(data))[0] | (((unsigned char*)(data))[1] << 8) | (((unsigned char*)(data))[2] << 16) | (((unsigned char*)(data))[3] << 24))

#define GET_U16(data)\
	(((unsigned char*)(data))[0] | (((unsigned char*)(data))[1] << 8))

#ifdef MIPS_VERSION
#define INLINE static inline
#else
#define INLINE static
#endif

INLINE int Get_BITS(TAG * s, unsigned n)
{
	int val;
	while((s)->bits < n) {
		(s)->hold = ((s)->hold << 8) | (s)->data[(s)->ptr++];
		(s)->bits += 8;
	};
	(s)->bits -= n;
	val = (s)->hold >> (s)->bits;
	(s)->hold &= ((1U << ((s)->bits)) - 1);
	return val;
}

INLINE int Peek_BITS(TAG * s, unsigned n)
{
	int val;
	while((s)->bits < n) {
		(s)->hold = ((s)->hold << 8) | (s)->data[(s)->ptr++];
		(s)->bits += 8;
	};
	val = (s)->hold >> ((s)->bits - n);
	return val;
}


#define BITS(s,n)\
	Get_BITS(s, n)

#define BITS_CLEAR(s)\
{\
	(s)->hold = 0;\
	(s)->bits = 0;\
}

INLINE void GET_RECT(SWF_RECT * pRect, TAG * s)
{
	int nbits;
	int shiftbits;
	BITS_CLEAR(s);
	nbits=BITS(s,5);
	shiftbits=32-nbits;
	UI_TO_SI((pRect)->Xmin,BITS(s,nbits),shiftbits);
	UI_TO_SI((pRect)->Xmax,BITS(s,nbits),shiftbits);
	UI_TO_SI((pRect)->Ymin,BITS(s,nbits),shiftbits);
	UI_TO_SI((pRect)->Ymax,BITS(s,nbits),shiftbits);
}

INLINE void GET_MATRIX(SWF_MATRIX * pMatrix, TAG * s)
{
	int HasScale,HasRotate,nbits;
	int shiftbits;
	BITS_CLEAR(s);
	HasScale = BITS(s, 1);
	if(HasScale==1)
	{
		nbits = BITS(s, 5);
		shiftbits=32-nbits;
		FLOAT_TO_FIXED((pMatrix)->ScaleX,BITS(s, nbits),shiftbits);
		FLOAT_TO_FIXED((pMatrix)->ScaleY,BITS(s, nbits),shiftbits);
	}
	else
	{
		(pMatrix)->ScaleX=0x10000;
		(pMatrix)->ScaleY=0x10000;
	}
	HasRotate = BITS(s, 1);
	if(HasRotate==1)
	{
		nbits = BITS(s, 5);
		shiftbits=32-nbits;
		FLOAT_TO_FIXED((pMatrix)->RotateSkew0,BITS(s, nbits),shiftbits);
		FLOAT_TO_FIXED((pMatrix)->RotateSkew1,BITS(s, nbits),shiftbits);
	}
	else
	{
		(pMatrix)->RotateSkew0=0;
		(pMatrix)->RotateSkew1=0;
	}
	nbits = BITS(s, 5);
	shiftbits=32-nbits;
	UI_TO_SI((pMatrix)->TranslateX,BITS(s, nbits),shiftbits);
	UI_TO_SI((pMatrix)->TranslateY,BITS(s, nbits),shiftbits);
}

INLINE void GET_CXFORM(SWF_CXFORM * colortransform, TAG * s, int withAlpha)
{
	int HasAddTerms,HasMultTerms;
	int Nbits,shiftbits;
	BITS_CLEAR(s);
	HasAddTerms = BITS(s, 1);
	HasMultTerms = BITS(s, 1);
	Nbits = BITS(s, 4);
	shiftbits=32-Nbits;
	if(HasMultTerms ==1 && Nbits!=0)
	{
		UI_TO_SI((colortransform)->RedMultTerm,BITS(s,Nbits),shiftbits);
		UI_TO_SI((colortransform)->GreenMultTerm,BITS(s,Nbits),shiftbits);
		UI_TO_SI((colortransform)->BlueMultTerm,BITS(s,Nbits),shiftbits);
		if(withAlpha)
		{
			UI_TO_SI((colortransform)->AlphaMultTerm,BITS(s,Nbits),shiftbits);
		}
	}
	else
	{
		(colortransform)->RedMultTerm = 256;
		(colortransform)->GreenMultTerm = 256;
		(colortransform)->BlueMultTerm =256;
		(colortransform)->AlphaMultTerm =256;
	}
	if(HasAddTerms==1 && Nbits!=0)
	{
		UI_TO_SI((colortransform)->RedAddTerm,BITS(s,Nbits),shiftbits);
		UI_TO_SI((colortransform)->GreenAddTerm,BITS(s,Nbits),shiftbits);
		UI_TO_SI((colortransform)->BlueAddTerm,BITS(s,Nbits),shiftbits);
		if(withAlpha)
		{
			UI_TO_SI((colortransform)->AlphaAddTerm,BITS(s,Nbits),shiftbits);	
		}
	}
	else
	{
		(colortransform)->RedAddTerm = 0;
		(colortransform)->GreenAddTerm =0;
		(colortransform)->BlueAddTerm = 0;
		(colortransform)->AlphaAddTerm = 0;
	}
}

/*
#define GET_MATRIX(pMatrix, s)\
{\
	int HasScale,HasRotate,nbits;\
	int shiftbits;\
	BITS_CLEAR(s);\
	HasScale = BITS(s, 1);\
	if(HasScale==1)\
	{\
		nbits = BITS(s, 5);\
		shiftbits=32-nbits;\
		FLOAT_TO_FIXED((pMatrix)->ScaleX,BITS(s, nbits),shiftbits);\
		FLOAT_TO_FIXED((pMatrix)->ScaleY,BITS(s, nbits),shiftbits);\
	}\
	else\
	{\
		(pMatrix)->ScaleX=0x10000;\
		(pMatrix)->ScaleY=0x10000;\
	}\
	HasRotate = BITS(s, 1);\
	if(HasRotate==1)\
	{\
		nbits = BITS(s, 5);\
		shiftbits=32-nbits;\
		FLOAT_TO_FIXED((pMatrix)->RotateSkew0,BITS(s, nbits),shiftbits);\
		FLOAT_TO_FIXED((pMatrix)->RotateSkew1,BITS(s, nbits),shiftbits);\
	}\
	else\
	{\
		(pMatrix)->RotateSkew0=0;\
		(pMatrix)->RotateSkew1=0;\
	}\
	nbits = BITS(s, 5);\
	shiftbits=32-nbits;\
	UI_TO_SI((pMatrix)->TranslateX,BITS(s, nbits),shiftbits);\
	UI_TO_SI((pMatrix)->TranslateY,BITS(s, nbits),shiftbits);\
}

#define GET_CXFORM(colortransform,s,tagType)\
{\
	int HasAddTerms,HasMultTerms;\
	int Nbits,shiftbits;\
	BITS_CLEAR(s);\
	HasAddTerms = BITS(s, 1);\
	HasMultTerms = BITS(s, 1);\
	Nbits = BITS(s, 4);\
	shiftbits=32-Nbits;\
	if(HasMultTerms ==1 && Nbits!=0)\
	{\
		UI_TO_SI((colortransform)->RedMultTerm,BITS(s,Nbits),shiftbits);\
		UI_TO_SI((colortransform)->GreenMultTerm,BITS(s,Nbits),shiftbits);\
		UI_TO_SI((colortransform)->BlueMultTerm,BITS(s,Nbits),shiftbits);\
		if(tagType == PLACEOBJECT2)\
		{\
			UI_TO_SI((colortransform)->AlphaMultTerm,BITS(s,Nbits),shiftbits);\
		}\
	}\
	else\
	{\
		(colortransform)->RedMultTerm = 256;\
		(colortransform)->GreenMultTerm = 256;\
		(colortransform)->BlueMultTerm =256;\
		(colortransform)->AlphaMultTerm =256;\
		if(tagType == PLACEOBJECT)\
		{\
			(colortransform)->AlphaMultTerm =257;\
		}\
	}\
	if(HasAddTerms==1 && Nbits!=0)\
	{\
		UI_TO_SI((colortransform)->RedAddTerm,BITS(s,Nbits),shiftbits);\
		UI_TO_SI((colortransform)->GreenAddTerm,BITS(s,Nbits),shiftbits);\
		UI_TO_SI((colortransform)->BlueAddTerm,BITS(s,Nbits),shiftbits);\
		if(tagType == PLACEOBJECT2)\
		{\
			UI_TO_SI((colortransform)->AlphaAddTerm,BITS(s,Nbits),shiftbits);	\
		}\
	}\
	else\
	{\
		(colortransform)->RedAddTerm = 0;\
		(colortransform)->GreenAddTerm =0;\
		(colortransform)->BlueAddTerm = 0;\
		(colortransform)->AlphaAddTerm = 0;\
		if(tagType == PLACEOBJECT)\
		{\
			(colortransform)->AlphaAddTerm = 257;\
		}\
	}\
}
*/

#define GET_BITS_READER(s)\
	(BITS_READER*)&(s)->data

#define ALLOC_TAG(s) \
	if((s)->TagLength > 0)\
	{\
	(s)->data = (INT8U*)SWF_MALLOC((s)->TagLength);\
	(s)->ptr = 0;\
	(s)->hold = 0;\
	(s)->bits = 0;\
	}

#define FREE_TAG(s) \
	if((s)->TagLength > 0)\
	{\
	SWF_FREE((s)->data);\
	(s)->data = NULL;\
	}

#ifdef __cplusplus
}
#endif
#endif

