#ifndef SWF_UTIL_H
#define SWF_UTIL_H

#ifndef MAX
#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define ABS(a) ((a) < 0 ? -(a) : (a))
#endif

//#ifdef MIPS_VERSION
#if 1
#define SWF_MALLOC(size) swf_malloc_basic(size)
#define SWF_MALLOC_SHARE(size) swf_malloc_share(size)
#define SWF_FREE(p) swf_free(p)
#else
#define SWF_MALLOC(size) swf_dbg_malloc(swf_malloc_basic, size, __FILE__, __LINE__)
#define SWF_MALLOC_SHARE(size) swf_dbg_malloc(swf_malloc_share, size, __FILE__, __LINE__)
#define SWF_FREE(p) swf_dbg_free(p)
#endif

#define ALLOC_MEM2D(array2D,type,dim0,dim1)\
{\
	int i;\
	array2D = (type**)SWF_MALLOC((dim0) * sizeof(type*));\
	array2D[0] = (type*)SWF_MALLOC((dim0) * (dim1) * sizeof(type));\
	for(i = 1; i < (dim0); i++)\
	array2D[i] = array2D[i - 1] + (dim1);\
}

#define FREE_MEM2D(array2D)\
{\
	if(array2D != NULL)\
	{\
		SWF_FREE(array2D[0]);\
		SWF_FREE(array2D);\
	}\
}

#define INIT_LIST_ARRAY(listArray, size)\
{\
	int i;\
	for(i = 0; i < size; i++)\
	{\
	BLIST_INIT(&listArray[i]);\
	}\
}

#define GET_SCREEN_COOR(x)\
	(((x) + 8) >> 4)

#define GET_SPACE_COOR(x)\
	((x) << 4)

#define IS_SHAPE_UNCHANGED(a,b) (\
	(a)->RotateSkew0 == (b)->RotateSkew0 &&\
	(a)->RotateSkew1 == (b)->RotateSkew1 &&\
	(a)->ScaleX == (b)->ScaleX &&\
	(a)->ScaleY == (b)->ScaleY)

#define IS_MATRIX_UNCHANGED(a,b) (\
	(a)->RotateSkew0 == (b)->RotateSkew0 &&\
	(a)->RotateSkew1 == (b)->RotateSkew1 &&\
	(a)->ScaleX == (b)->ScaleX &&\
	(a)->ScaleY == (b)->ScaleY &&\
	(a)->TranslateX == (b)->TranslateX &&\
	(a)->TranslateY == (b)->TranslateY)

#define IS_SHAPE_OVERLAPED(r1, r2) (\
	((r1)->Xmax > (r2)->Xmin) &&\
	((r2)->Xmax > (r1)->Xmin) &&\
	((r1)->Ymax > (r2)->Ymin) &&\
	((r2)->Ymax > (r1)->Ymin))

#define IS_SHAPE_OVERLAPED2(r1, r2, t1, t2) (\
	((r1)->Xmax + (t1)->TranslateX > (r2)->Xmin + (t2)->TranslateX) &&\
	((r2)->Xmax + (t2)->TranslateX > (r1)->Xmin + (t1)->TranslateX) &&\
	((r1)->Ymax + (t1)->TranslateY > (r2)->Ymin + (t2)->TranslateY) &&\
	((r2)->Ymax + (t2)->TranslateY > (r1)->Ymin + (t1)->TranslateY))

#define IS_COLOR_EMPTY(a) (\
	((a)->AlphaAddTerm | (a)->BlueAddTerm | (a)->GreenAddTerm | (a)->RedAddTerm |\
	((a)->AlphaMultTerm-0x100) | ((a)->BlueMultTerm-0x100) | ((a)->GreenMultTerm-0x100) | ((a)->RedMultTerm-0x100)) == 0)

#define IS_ALPHA_FULL(a)\
    ((a & 0xFF000000)  == 0xFF000000)


#define RGB2YUV(y,u,v,r,g,b)\
{\
	y = (((66 * r + 129 * g + 25 * b) + 128) >> 8) + 16;\
	u = (((-38 * r - 74 * g + 112 * b) + 128) >> 8) + 128;\
	v = (((112 * r - 94 * g - 18 * b) + 128) >> 8) + 128;\
}

#define YUV2RGB(r, g, b, y, u, v)\
{\
	int c,d,e;\
	c = y - 16;\
	d = u - 128;\
	e = v - 128;\
	r = MAX(0,MIN(255,(298 * c + 409 * e + 128) >> 8));\
	g = MAX(0,MIN(255,(298 * c - 100 * d - 208 * e + 128) >> 8));\
	b = MAX(0,MIN(255,(298 * c + 516 * d + 128) >> 8));\
}

#define ARGB_MUX(a,r,g,b)\
    (((a) << 24) | ((r) << 16) | ((g) << 8) | (b))

#define ARGB_DEMUX(v,a,r,g,b)\
{\
    a = (v) >> 24;\
    r = (v) >> 16;\
    g = (v) >> 8;\
    b = (v);\
}

#define LOWER_CASE(c) (((c) >= 'A' && (c) <= 'Z') ? ((c) + 'a' - 'A') : (c))
#define UPPER_CASE(c) (((c) >= 'a' && (c) <= 'z') ? ((c) + 'A' - 'a') : (c))

//#ifdef MIPS_VERSION
#if 0
#define FPMUL(rd,f,m)\
{\
	asm volatile("MULT %1,%2;"\
	"MFHI $8;"\
	"MFLO $9;"\
	"SLL $8,$8,16;"\
	"SRL $9,$9,16;"\
	"OR %0,$8,$9;"\
	:"=d"(rd)\
	:"d"(f),"d"(m)\
	:"$8","$9"\
	);\
}
#else
#define FPMUL(rd,f,m)\
	rd = fpMul(f,m)
#endif


//#ifdef MIPS_VERSION
#if 0
#define FPMUL2(rd,f1,m1,f2,m2)\
{\
	asm volatile("MULT %1,%2;"\
	"MFHI $8;"\
	"MFLO $9;"\
	"SLL $8,$8,16;"\
	"SRL $9,$9,16;"\
	"OR $10,$8,$9;"\
	"MULT %3,%4;"\
	"MFHI $8;"\
	"MFLO $9;"\
	"SLL $8,$8,16;"\
	"SRL $9,$9,16;"\
	"OR $11,$8,$9;"\
	"ADD %0,$10,$11;"\
	:"=d"(rd)\
	:"d"(f1),"d"(m1),"d"(f2),"d"(m2)\
	:"$8","$9","$10","$11"\
	);\
}
#else
#define FPMUL2(rd,f1,m1,f2,m2)\
	rd = fpMul(f1,m1) + fpMul(f2,m2)
#endif

//#ifdef MIPS_VERSION
#if 0
#define FPMUL3(rd,f,m,a)\
{\
	asm volatile("MULT %1,%2;"\
	"MFHI $8;"\
	"MFLO $9;"\
	"SLL $8,$8,16;"\
	"SRL $9,$9,16;"\
	"OR $10,$8,$9;"\
	"ADD %0,$10,%3;"\
	:"=d"(rd)\
	:"d"(f),"d"(m),"d"(a)\
	:"$8","$9","$10"\
	);\
}
#else
#define FPMUL3(rd,f,m,a)\
	rd = (a) + fpMul(f,m)
#endif

//#ifdef MIPS_VERSION
#if 0
#define CoorTrans(rd,f1,m1,f2,m2,a)\
{\
	asm volatile("MULT %1,%2;"\
	"MFHI $8;"\
	"MFLO $9;"\
	"SLL $8,$8,16;"\
	"SRL $9,$9,16;"\
	"OR $10,$8,$9;"\
	"ADD $10,$10,%5;"\
	"MULT %3,%4;"\
	"MFHI $8;"\
	"MFLO $9;"\
	"SLL $8,$8,16;"\
	"SRL $9,$9,16;"\
	"OR $11,$8,$9;"\
	"ADD %0,$10,$11;"\
	:"=d"(rd)\
	:"d"(f1),"d"(m1),"d"(f2),"d"(m2),"d"(a)\
	:"$8","$9","$10","$11"\
	);\
}
#else
#define CoorTrans(rd,f1,m1,f2,m2,a)\
	rd = (a) + fpMul(f1,m1) + fpMul(f2,m2)
#endif

extern unsigned char Sqrt[];

#endif
