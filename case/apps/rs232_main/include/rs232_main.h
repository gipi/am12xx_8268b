#ifndef __RS232_MAIN_H__
#define __RS232_MAIN_H__

#define MSG_MAX_LEN 1024
#define CMD_MAX_LEN 50

#define READ_MSG_KEY 500
#define WRITE_MSG_KEY 501

#define SEND_RS232_CMD 19
#define CMD_GET_RESULT 20

#ifdef __cplusplus
extern "C" {
#endif

#include "uthash.h"
struct s_query {
	char uart_cmd[30];				/* cmd key (string is WITHIN the structure) */
	char web_cmd[30];
    char query_result[30];
    UT_hash_handle hh1,hh2;         /* makes this structure hashable, hh1 for uart_cmd, hh2 for web_cmd */
};


struct msg_t{
	long int type;
	char buf[MSG_MAX_LEN];
	char NonCached;
};

struct hash_data{
	int catelog;
	int item;
	int value;
	char cmd[CMD_MAX_LEN];
};

struct dehash_data{
	int hash;
	int catelog;
	int item;
	int value;
};

typedef struct hash_data* HASH_DATA;
typedef struct hash_dehash* DEHASH_DATA;

//#define printf(...)

#ifdef __cplusplus
}
#endif


#endif
