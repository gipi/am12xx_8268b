/*
 * WPA Supplicant / UNIX domain socket -based control interface
 * Copyright (c) 2004-2005, Jouni Malinen <j@w1.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 */

#ifndef WPA_CLI_H
#define WPA_CLI_H
typedef struct _wpa_cmd_result
{
	int results;
	char buf[2048];
}wpa_cmd_result;

int wpa_cli_cmd_main(int argc,char* argv[],wpa_cmd_result *rel);


#endif /* CTRL_IFACE_H */

