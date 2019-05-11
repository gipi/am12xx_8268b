/*
 *  tslib/tests/ts_calibrate.c
 *
 *  Copyright (C) 2001 Russell King.
 *
 * This file is placed under the GPL.  Please see the file
 * COPYING for more details.
 *
 * $Id: ts_calibrate.c,v 1.8 2004/10/19 22:01:27 dlowder Exp $
 *
 * Basic test program for touchscreen library.
 */
//#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include "tslib.h"

int xres=800, yres=480;
int main()
{
	struct tsdev *ts;
	calibration cal;
	int cal_fd;
	char cal_buffer[256];
	char *tsdevice = NULL;
	char *calfile = NULL;
	unsigned int i;

	//signal(SIGSEGV, sig);
	//signal(SIGINT, sig);
	//signal(SIGTERM, sig);
	
	if( (tsdevice = getenv("TSLIB_TSDEVICE")) != NULL ) {
		ts = ts_open(tsdevice,0);
	} else {
		ts = ts_open("/dev/event1", 0);
	}

	if (!ts) {
		perror("ts_open");
		exit(1);
	}
	if (ts_config(ts)) {
		perror("ts_config");
		exit(1);
	}
	

	get_sample (ts, &cal, 0,50,			50,		"Top left");
	get_sample (ts, &cal, 1,xres-50,	50,		"Top right");
	get_sample (ts, &cal, 2,xres-50,	yres-50,	"Bot right");
	get_sample (ts, &cal, 3,50,			yres-50,	"Bot left");
	get_sample (ts, &cal, 4,xres/2,		yres/2,	"Center");

	if (perform_calibration (&cal)) {
		printf ("Calibration constants: ");
		for (i = 0; i < 7; i++) printf("%d ", cal.a [i]);
		printf("\n");
		if ((calfile = getenv("TSLIB_CALIBFILE")) != NULL) {
			cal_fd = open (calfile, O_CREAT | O_RDWR);
		} else {
			cal_fd = open ("/mnt/udisk/pointercal", O_CREAT | O_RDWR | O_TRUNC);
		}
		
		sprintf (cal_buffer,"%d %d %d %d %d %d %d",
				cal.a[1], cal.a[2], cal.a[0],
				cal.a[4], cal.a[5], cal.a[3], cal.a[6]);
		write (cal_fd, cal_buffer, strlen (cal_buffer) + 1);
		fsync(cal_fd);
		close (cal_fd);
		i = 0;
	} else {
		printf("Calibration failed.\n");
		i = -1;
	}

	while(1)
	{
		struct ts_sample samp;
		ts_read(ts, &samp, 1);

		printf("curXX=%d,curYY=%d\n",samp.x,samp.y);
		
	}
	//close_framebuffer();
	return i;
}
