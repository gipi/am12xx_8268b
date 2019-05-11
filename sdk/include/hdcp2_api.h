#ifndef __HDCP2_API_H
#define __HDCP2_API_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Start the hdcp thread.
 *
 * @param port:the hdcp TCP port,default to 10123.
 */
extern int hdcp2_start(int port);

/**
 * @brief API for data decoding. 
 * @param[in] size - Size of the TS payload to decrypt
 * @param[in] src - Pointer to payload to decrypt.
 * @param[in] dst - Pointer to payload to decrypt.
 * @param[in] input_counter - from MPEG-TS stream
 * @param[out] *result - Result code returned from the SPU function.
 * @return 0 on success, or failure.
 */
extern int hdcp2_decode_data(unsigned int size,unsigned char *src,unsigned char *dst,unsigned long long input_counter,unsigned int *result);

/**
 * @brief Check if hdcp2 authentication has been completed.
 *
 * @return 1-->completed;0-->failure;
 */
extern int hdcp2_status_ok();

/**
 * @brief Rlease some resource when quit.
 */
extern int hdcp2_release();


/**
* @brief Check if the chipkey exists or not.
* @return 1 ---> exist 0 ---> not exist
*/
extern int hdcp2_has_chipkey();

#ifdef __cplusplus
}
#endif

#endif

