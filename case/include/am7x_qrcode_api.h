#ifndef _AM7X_QRCODE_API_H
#define _AM7X_QRCODE_API_H

#if defined(__cplusplus)
extern "C" {
#endif

typedef enum{
	QR_STAT_INIT=0,
	QR_STAT_ENCODED,
	QR_STAT_ERROR
}QR_STATUS;

struct qrc_code{
	void *qrcode; ///> wrapper coder
	int width;    ///> the QR code width after encoding
	int status;   ///> the QR code status
};


/**
* @brief Do some init.
*/
extern struct qrc_code *qrc_open();

/**
* @brief Do some destruction.
*/
extern void qrc_close(struct qrc_code *coder);


/**
* @brief QR encode the input string.
*/
extern int qrc_encode(struct qrc_code *coder,unsigned char *str);


/**
* @brief Get the QR code width for displaying.
*/
extern int qrc_get_width(struct qrc_code *coder);


/**
* @brief Get the code at position (x,y) relative to the QR code area.
* 
* @return 1 for black and 0 for white.
*/
extern int qrc_get_code_at_pos(struct qrc_code *coder,int x,int y);


#if defined(__cplusplus)
}
#endif

#endif /** _AM7X_QRCODE_API_H */

