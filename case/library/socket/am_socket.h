#ifndef __WEBSETTING_CGI_H__
#define __WEBSETTING_CGI_H__
#include <sys/un.h> 
#include <sys/socket.h>
int socket_cli_fd;
struct sockaddr_un unc;
void websetting_client_write(char *cmd_string);
int websetting_client_read(char *receiveline );
int create_websetting_client(char *cmd_string);
void close_websetting_socket_cli_fd(void);
#endif
