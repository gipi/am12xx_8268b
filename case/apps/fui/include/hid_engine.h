
#include "swf_ext.h"
#include "file_list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>

typedef struct HID_PC_value{
	char magic_string[4];
	long packet_length;
	long type;	// reserved 3 bytes
	short checksum;
	short pointers_dot;
	char pointer_ID ;
	char status;
	short X_position1; 
	short y_position1; 
}PC_Data_value;

struct _hiddev_report_info {
	int report_type;
	int report_id;
	int num_fields;
};

struct _hiddev_field_info {
	int report_type;
	int report_id;
	int field_index;
	int maxusage;
	int flags;
	int physical;		/* physical usage for this field */
	int logical;		/* logical usage for this field */
	int application;		/* application usage for this field */
	int logical_minimum;
	int logical_maximum;
	int physical_minimum;
	int physical_maximum;
	int unit_exponent;
	int unit;
};

struct _hiddev_usage_ref {
	int report_type;
	int report_id;
	int field_index;
	int usage_index;
	int usage_code;
	int value;
};
extern PC_Data_value PC_Data;
/*struct HID_PC_value {
	char magic_string[4];
	long packet_length;
	long type;	// reserved 3 bytes
	short checksum;
	short pointers_dot;
	char pointer_ID ;
	char status;
	short X_position1; 
	short y_position1; 

}PC_Data_value;*/

int pos = 0;
int HID_value[64]={0x00};//shane

long current_params_x_a = 0;
long current_params_x_b = 0;
long current_params_y_a = 0;
long current_params_y_b = 0;

static int shift_x, shift_y, handle_cal_error =0, pen_down=0;
static int pen_for_pc=0;

#define HID_REPORT_ID_UNKNOWN 0xffffffff
#define HID_REPORT_ID_FIRST   0x00000100
#define HID_REPORT_ID_NEXT    0x00000200
#define HID_REPORT_ID_MASK    0x000000ff
#define HID_REPORT_ID_MAX     0x000000ff

#define HID_REPORT_TYPE_INPUT	1
#define HID_REPORT_TYPE_OUTPUT	2
#define HID_REPORT_TYPE_FEATURE	3
#define HID_REPORT_TYPE_MIN     1
#define HID_REPORT_TYPE_MAX     3
/*ioctl command*/
#define HIDIOCGVERSION	0x40044801	
#define HIDIOCAPPLICATION	0x20004802
#define HIDIOCGDEVINFO	 0x401c4803	
#define HIDIOCGSTRING	 0x41044804	
#define HIDIOCINITREPORT	 0x20004805
#define HIDIOCGNAME
#define HIDIOCGREPORT	0x800c4807	
#define HIDIOCSREPORT	800c4808	
#define HIDIOCGREPORTINFO  0xc00c4809	
#define HIDIOCGFIELDINFO	    0xc038480a
#define HIDIOCGUSAGE		    0xc018480b
#define HIDIOCSUSAGE		    0x8018480c
#define HIDIOCGUCODE	  0xc018480d	
#define HIDIOCGFLAG	  0x4004480e	
#define HIDIOCSFLAG	  0x8004480f	
#define HIDIOCGCOLLECTIONINDEX	
#define HIDIOCGCOLLECTIONINFO   0xc0104811	

