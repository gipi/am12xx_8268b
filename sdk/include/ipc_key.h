#ifndef __IPC_KEY__
#define __IPC_KEY__
/**
*@file ipc_key.h
*@brief This head file represents the key values for IPC resource\n
*All the values are sorted by group, as a rule of IPC key.
*
*@author yekai
*@date 2010-03-25
*@version 0.1
*/

/**magic number of id key*/
#define AM_IPC_KEY_BASE	0x414D3758	//"AM7X"
/**offset between each key*/
#define UNIT_KEY_NUM		0xff

/**
*@name key base for each module
*@{
*/
#define MANAGER_KEY_BASE		(AM_IPC_KEY_BASE+UNIT_KEY_NUM*1)
#define MNAVI_KEY_BASE			(AM_IPC_KEY_BASE+UNIT_KEY_NUM*2)
#define MIDWARE_KEY_BASE		(AM_IPC_KEY_BASE+UNIT_KEY_NUM*3)
#define GUI_KEY_BASE			(AM_IPC_KEY_BASE+UNIT_KEY_NUM*4)
#define COMMON_KEY_BASE		(AM_IPC_KEY_BASE+UNIT_KEY_NUM*5)
#define APP_KEY_BASE			(AM_IPC_KEY_BASE+UNIT_KEY_NUM*6)
/**
*@}
*/

#endif // __IPC_KEY__