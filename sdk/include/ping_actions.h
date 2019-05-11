#ifndef _PING_ACTIONS_H
#define _PING_ACTIONS_H

#ifdef __cplusplus 
extern "C" {
#endif

int ping_network(const char *ip, int timeoutMs);

#ifdef __cplusplus 
}
#endif

#endif
