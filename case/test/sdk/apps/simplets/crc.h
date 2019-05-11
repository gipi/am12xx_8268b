

#ifndef AVUTIL_CRC_H
#define AVUTIL_CRC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ez_avutil.h"

typedef av_u32 AVCRC;

typedef enum {
    AV_CRC_8_ATM,
    AV_CRC_16_ANSI,
    AV_CRC_16_CCITT,
    AV_CRC_32_IEEE,
    AV_CRC_32_IEEE_LE,  /*< reversed bitorder version of AV_CRC_32_IEEE */
    AV_CRC_MAX,         /*< Not part of public API! Do not use outside libavutil. */
}AVCRCId;

av_u32 av_crc(const AVCRC *ctx, av_u32 crc, const av_u8 *buffer, unsigned int length);
const AVCRC *av_crc_get_table(AVCRCId crc_id);

#ifdef __cplusplus
}
#endif
#endif /* AVUTIL_CRC_H */

