#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "string.h"
#include "qrspec.h"
#include "qrinput.h"
#include "split.h"
#include "qrencode.h"
#include "qrencode_inner.h"
#include "am7x_qrcode_api.h"

static int qr_enc_version = 3;
static int qr_ecc_level = QR_ECLEVEL_L;

__attribute__ ((visibility("default"))) struct qrc_code *qrc_open()
{
	struct qrc_code *coder;

	coder = (struct qrc_code *)malloc(sizeof(struct qrc_code));
	if(coder == NULL){
		printf("%s:not enough memory\n",__FUNCTION__);
		return NULL;
	}

	coder->qrcode =NULL;
	coder->status = QR_STAT_INIT;
	coder->width = 0;

	return coder;
}

__attribute__ ((visibility("default"))) void qrc_close(struct qrc_code *coder)
{
	QRcode *qrcode;
	
	if(coder == NULL){
		return;
	}

	/** release the internal structures */
	qrcode = (QRcode *)coder->qrcode;
	if(qrcode){
		QRcode_free(qrcode);
	}
	
	free(coder);
	coder = NULL;
}


__attribute__ ((visibility("default"))) int qrc_encode(struct qrc_code *coder,unsigned char *str)
{
	int len;
	int ret;
	QRinput *input;
	QRcode *qrcode,*oldqrcode;
	
	if(coder==NULL || str==NULL){
		printf("%s:param error\n",__FUNCTION__);
		goto error_out_2;
	}

	len = strlen((char *)str);
	if(len <=0){
		printf("%s:string error\n",__FUNCTION__);
		goto error_out_2;
	}

	input = QRinput_new2(qr_enc_version, qr_ecc_level);
	if(input == NULL) {
		printf("%s:input init error\n",__FUNCTION__);
		goto error_out_2;
	}

	ret = QRinput_append(input, QR_MODE_8, len, str);
	if(ret < 0) {
		printf("%s:input append error\n",__FUNCTION__);
		goto error_out_1;
	}

	QRinput_setVersionAndErrorCorrectionLevel(input, qr_enc_version, qr_ecc_level);

	qrcode = QRcode_encodeMask(input, -1);
	if(qrcode == NULL) {
		printf("%s:QR encode error\n",__FUNCTION__);
		goto error_out_1;
	}

	/** replace the old QR code */
	oldqrcode = (QRcode *)coder->qrcode;
	if(oldqrcode){
		QRcode_free(oldqrcode);
		coder->qrcode = NULL;
	}

	coder->qrcode = qrcode;
	coder->status = QR_STAT_ENCODED;
	coder->width = qrcode->width;

	return 0;
	
error_out_1:
	QRinput_free(input);
error_out_2:
	return -1;
}


__attribute__ ((visibility("default"))) int qrc_get_width(struct qrc_code *coder)
{
	if(coder && (coder->status == QR_STAT_ENCODED)){
		return coder->width;
	}

	return 0;
}


__attribute__ ((visibility("default"))) int qrc_get_code_at_pos(struct qrc_code *coder,int x,int y)
{
	unsigned char *p;
	QRcode *qrcode;

	if(coder && (coder->status == QR_STAT_ENCODED) && coder->qrcode){
		qrcode = (QRcode *)coder->qrcode;
		p = qrcode->data;

		if(*(p+y*qrcode->width+x)&1){
			return 1;
		}
		else{
			return 0;
		}
	}

	return 0;
}

