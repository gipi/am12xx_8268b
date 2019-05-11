
#include "crc.h"

typedef struct {
    av_u8  le;
    av_u8  bits;
    av_u32 poly;
} AV_CRC_TABLE_PARM;
AV_CRC_TABLE_PARM av_crc_table_params[AV_CRC_MAX];

static AVCRC av_crc_table[AV_CRC_MAX][257];

//#define NULL    0
/**
 * Initializes a CRC table.
 * @param ctx must be an array of size sizeof(AVCRC)*257 or sizeof(AVCRC)*1024
 * @param cts_size size of ctx in bytes
 * @param le If 1, the lowest bit represents the coefficient for the highest
 *           exponent of the corresponding polynomial (both for poly and
 *           actual CRC).
 *           If 0, you must swap the CRC parameter and the result of av_crc
 *           if you need the standard representation (can be simplified in
 *           most cases to e.g. bswap16):
 *           bswap_32(crc << (32-bits))
 * @param bits number of bits for the CRC
 * @param poly generator polynomial without the x**bits coefficient, in the
 *             representation as specified by le
 * @return <0 on failure
 */
int av_crc_init(AVCRC *ctx, int le, int bits, av_u32 poly, int ctx_size){
    int i, j;
    av_u32 c;
	
#ifdef WIN32
	if (bits < 8 || bits > 32 || poly >= ((av_u64)(1)<<bits))
#else
	if (bits < 8 || bits > 32 || poly >= (1LL<<bits))
#endif
        return -1;
    if (ctx_size != sizeof(AVCRC)*257 && ctx_size != sizeof(AVCRC)*1024)
        return -1;

    for (i = 0; i < 256; i++) {
        if (le) {
            for (c = i, j = 0; j < 8; j++)
                c = (c>>1)^(poly & (-(c&1)));
            ctx[i] = c;
        } else {
            for (c = i << 24, j = 0; j < 8; j++)
                c = (c<<1) ^ ((poly<<(32-bits)) & (((av_i32)c)>>31) );
            ctx[i] = bswap_32(c);
        }
    }
    ctx[256]=1;

    if(ctx_size >= sizeof(AVCRC)*1024)
        for (i = 0; i < 256; i++)
            for(j=0; j<3; j++)
                ctx[256*(j+1) + i]= (ctx[256*j + i]>>8) ^ ctx[ ctx[256*j + i]&0xFF ];

    return 0;
}

/**
 * Gets an initialized standard CRC table.
 * @param crc_id ID of a standard CRC
 * @return a pointer to the CRC table or NULL on failure
 */
#define FF_ARRAY_ELEMS(a) (sizeof(a) / sizeof((a)[0]))

const AVCRC *av_crc_get_table(AVCRCId crc_id){
#if 1
	av_crc_table_params[0].le = 0;
	av_crc_table_params[0].bits = 8;
	av_crc_table_params[0].poly = 0x07;

	av_crc_table_params[1].le = 0;
	av_crc_table_params[1].bits = 16;
	av_crc_table_params[1].poly = 0x8005;

	av_crc_table_params[2].le = 0;
	av_crc_table_params[2].bits = 16;
	av_crc_table_params[2].poly = 0x1021;

	av_crc_table_params[3].le = 0;
	av_crc_table_params[3].bits = 32;
	av_crc_table_params[3].poly = 0x04C11DB7;

	av_crc_table_params[4].le = 0;
	av_crc_table_params[4].bits = 32;
	av_crc_table_params[4].poly = 0xEDB88320;
#endif
	if (!av_crc_table[crc_id][FF_ARRAY_ELEMS(av_crc_table[crc_id])-1])
		if (av_crc_init(av_crc_table[crc_id],
			av_crc_table_params[crc_id].le,
			av_crc_table_params[crc_id].bits,
			av_crc_table_params[crc_id].poly,
			sizeof(av_crc_table[crc_id])) < 0)
			return NULL;

	return av_crc_table[crc_id];
}
/**
 * Calculates the CRC of a block.
 * @param crc CRC of previous blocks if any or initial value for CRC
 * @return CRC updated with the data from the given block
 *
 * @see av_crc_init() "le" parameter
 */
#define le2me_32(x) (x)
av_u32 av_crc(const AVCRC *ctx, av_u32 crc, const av_u8 *buffer, unsigned int length){//size_t
    const av_u8 *end= buffer+length;

	if (ctx)
	{
    if(!ctx[256])
        while(buffer<end-3){
            crc ^= le2me_32(*(const av_u32*)buffer); buffer+=4;
            crc =  ctx[3*256 + ( crc     &0xFF)]
                  ^ctx[2*256 + ((crc>>8 )&0xFF)]
                  ^ctx[1*256 + ((crc>>16)&0xFF)]
                  ^ctx[0*256 + ((crc>>24)     )];
        }

    while(buffer<end)
        crc = ctx[((av_u8)crc) ^ *buffer++] ^ (crc >> 8);
	}

    return crc;
}
