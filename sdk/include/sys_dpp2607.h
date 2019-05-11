#ifndef __SYS_DPP2607_H__
#define __SYS_DPP2607_H__


#ifdef __cplusplus
extern "C" {
#endif


/**
*@dpp2607 operation
*@{
*/
#define READ_REG_DPP		(0x1)
#define WRITE_REG_DPP		(0x2)
#define CMD_BRIGHT_DPP	(0x3)
#define CMD_CORRECT_DPP	(0x4)
#define RESET_DPP		(0x05)
#define CMD_READ_TMP_DPP	(0x06)
#define CMD_BRIGHT_TRUE_DPP	(0x9)
#define CMD_SET_ICP		(0x14)
#define CMD_GET_ICP	(0x15)


struct dpp2607_data{
	unsigned char reg;
	unsigned char val[4];
};




#ifdef __cplusplus
}
#endif

#endif /*__SYS_DPP2607_H__*/

