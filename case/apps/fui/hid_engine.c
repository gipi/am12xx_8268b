/*
* USB HID FUNCTIONS 
*/
#include "hid_engine.h"
#include "swf_ext.h"
//#include "swfdec.h"
//#include "swf_types.h"
#include "sys_msg.h"
//#include "key_map.h"
//#include "sys_cfg.h"
#include <sys/msg.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h> /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include<pthread.h> 
#include <stdlib.h>
#include <signal.h>

#define HidInfo(fmt, arg...) printf("[%s %d]:"fmt"\n",__func__,__LINE__,##arg)
#define HidErr(fmt, arg...) printf("[%s %d]:"fmt"\n",__func__,__LINE__,##arg)
#define 	HID_EVENT_DEBUG	
#ifdef HID_EVENT_DEBUG
	#define HidDbg(fmt, arg...) printf("[%s %d]:"fmt"\n",__func__,__LINE__,##arg)
#else
	#define HidDbg(fmt, arg...)
#endif
pthread_t hid=-1;
int hid_fd=-1;
int hid_fd1=-1;
int hid_fd2=-1;
//static int hid_mainloop(const  int fd);
void hid_process_report(int fd);
#if 0
static void hid_handle_event()
{
	HidDbg("process hiddev event\n");
	hid_process_report(hid_fd1);
}
#else

static int hid_mainloop(const  int fd)
{
	  fd_set hid_fds; 
	  int HidRet=-1, l;
	  int maxfds=fd+1;
	  char buf[128]={0x00};
	  struct timeval tv;
	   pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL); 
	  HidInfo("fd=%d\n",fd);
	  while(1){
		FD_ZERO(&hid_fds);
		FD_SET(fd,&hid_fds);
		tv.tv_sec=0;
		tv.tv_usec=250;
		HidRet=select(maxfds,&hid_fds,NULL,NULL,&tv);
		//HidInfo("HidRet:%d\n",HidRet);
		if( HidRet>0){
			if(FD_ISSET(fd,&hid_fds)){
				 for(l=1; l<=5; l++) {
		 			if(l == 5) {
		 				hid_process_report(fd);	 					
		 			} else {	 	
						usleep(1000);
					}
				}
			}
		}else if(HidRet<0){
			HidErr("select error:%d\n",errno);
		 }		
	  }
	  pthread_exit(NULL);
	  return 0;
}
void  process_hid(void *arg)
{
	int fd=*(int*)arg;
	HidDbg("fd:%d\n",fd);
	hid_mainloop(fd);
}
#endif
static int hid_open()
{
	int HidFlag;
	//hid_fd=open("/dev/event2",O_RDONLY);
	if(0==access("/dev/hiddev0",F_OK)){
		hid_fd=open("/dev/hiddev0",O_RDONLY);
		HidDbg("hid_fd:%d\n",hid_fd);
		if(hid_fd<0){
			HidErr("open device hiddev0 failured %d\n",errno);
			return -1;
		}
	}

	if(0==access("/dev/hiddev1",F_OK)){
		hid_fd1=open("/dev/hiddev1",O_RDONLY);
		HidDbg("hid_fd1:%d\n",hid_fd1);
		if(hid_fd1<0){
			HidErr("open device hiddev1 failured %d\n",errno);
			return -1;
		}
	}

	if(0==access("/dev/hiddev2",F_OK)){
		hid_fd2=open("/dev/hiddev2",O_RDONLY);
		HidDbg("hid_fd2:%d\n",hid_fd2);
		if(hid_fd2<0){
			HidErr("open device hiddev2 failured %d\n",errno);
			return -1;
		}
	}
#if 1	
	if(pthread_create(&hid, NULL, (void *(*)(void *))process_hid, &hid_fd1)!=0){
		HidErr("pthread error errno:%d\n",errno);
		return -1;
	}
#else
	signal(SIGIO,hid_handle_event);
	fcntl(hid_fd1,F_SETOWN,getpid());
	HidFlag=fcntl(hid_fd1,F_GETFL);
	fcntl(hid_fd1,F_SETFL,HidFlag|FASYNC);
#endif
	return hid_fd;
}

//void handle_cal(int new_x, int new_y, int new_down) 
static int x1, y1, x2, y2, x3, y3, x4, y4, x5, y5, dx, dy, cal_time=0;

static int handle_cal(int new_x, int new_y,int cal_state)
{
#define		TEST_X		640
#define		TEST_Y		400
#define		TORANCE_X		40
#define		TORANCE_y		40
	
	static float tor_x, tor_y;
	
		printf("Calibration: %d,%d,%d\n", new_x, new_y, cal_state);

			switch (cal_state) {
				case 1:	// 取得左上角點位 
					x1 = new_x;
					y1 = new_y;
					break;

				case 2:	// 取得右上角點位 
					x2 = new_x;
					y2 = new_y;
					break;
				
				case 3:	// 取得右下角點位 
					x3 = new_x;
					y3 = new_y;
					break;	
					
				case 4:	// 取得左下角點位 
					x4 = new_x;
					y4 = new_y;

					shift_x = (x4 - x1);
					shift_y = (y2 - y1);

					y2 -= shift_y;

					x3 -= shift_x;
					y3 -= shift_y;

					x4 -= shift_x;
					break;	

				case 5:	// 計算校正及驗證第五點							
					x5 = new_x - (shift_x >> 1);	// shift_x * (x5 - x1) / (x2 - x1)
					y5 = new_y - (shift_y >> 1);
					//x5 = new_x ;	
					//y5 = new_y ;				
									
					// transfer to right side
					/*if (x2 < x1) {
						if (y2 < y1) {
							mirror_side = 2;	// upside down & mirror
							x2 = x1;
							y2 = y1;
							x1 = 32767 - x1;	//x_trans
							y1 = 32767 - y1;	//y_trans
							x3 = 32767 - x3;	//x_trans
							y3 = 32767 - y3;	//y_trans
							
						} else {
							mirror_side = 1;	// x side mirror
							x2 = x1;
							x1 = 32767 - x1;	//x_trans
							x3 = 32767 - x3;	//x_trans
						}
					} else {
						if (y2 < y1) {
							mirror_side = 3;	// y side mirror
							y2 = y1;
							y1 = 32767 - y1;	//y_trans
							y3 = 32767 - y3;	//y_trans
						} else {
							mirror_side = 0;	// normal
						}
					}*/
					current_params_x_a = (long)(1230 * 65536.0 / (x3 - x1));
					current_params_y_a = (long)(750 * 65536.0 / (y3 - y1));
					current_params_x_b = 25 * 65536 - (long)x1 * current_params_x_a;
					current_params_y_b = 25 * 65536 - (long)y1 * current_params_y_a;
					
					dx = (short)(((long)x5 * current_params_x_a + current_params_x_b) >> 16) - (TEST_X/* - shift_x*/);
					dy = (short)(((long)y5 * current_params_y_a + current_params_y_b) >> 16) - (TEST_Y/* - shift_y*/);
					tor_x = (float)TORANCE_X * (x3 - x1) / (1280 - 50);	 
					tor_y = (float)TORANCE_y * (y3 - y1) / (800 - 50);	 
					
					printf("set cal to x1=%ld, x2=%ld, x3=%ld, x4=%ld, x5=%ld, y1=%ld, y2=%ld,  y3=%ld , y4=%ld, y5=%ld\n",
						   x1,x2,x3,x4,x5,					   
						   y1,y2,y3,y4,y5);
						  
						   
					printf("set cal to x_a=%ld, x_b=%ld, y_a=%ld, y_b=%ld, dx=%d, dy=%d, torance x=%f, torance y=%f\n",
						   current_params_x_a,  current_params_x_b,
						   current_params_y_a,  current_params_y_b,
						   dx, dy,
						   tor_x, tor_y);
					if (dx < (0-tor_x) || dx > tor_x || dy < (0-tor_y) || dy > tor_y) {						// 判定校正失敗
							printf("Calibrating ERROR\n");
							handle_cal_error = -1;
							//cal_time = 1;
					    break;
					}
					handle_cal_error = 1;
					cal_time = 6;
					pen_down = 0;
					break;
				default:
					break;
			}
	return 0;
}

static int x_out, y_out, x_old, y_old;

void calcluate_new_position(int new_x, int new_y)
{ 
		int x_tmp, y_tmp, check_tmp;

		x_tmp = new_x;
		y_tmp = new_y;
		x_tmp -= (shift_x + dx) * (new_y - y1) / (y4 - y1);
		y_tmp -= (shift_y + dy) * (new_x - x1) / (x2 - x1);
		printf("Calibration After x_tmp ,y_tmp: %d,%d\n", x_tmp, y_tmp);
		x_old = x_out;
		y_old = y_out;
		x_out = (short)(((long)x_tmp * current_params_x_a + current_params_x_b) >> 16) - (dx * (x2 - x1) / ((new_x - x1) * 2));
		y_out = (short)(((long)y_tmp * current_params_y_a + current_params_y_b) >> 16) - (dy * (y4 - y1) / (new_y - y1));


		if (pen_for_pc == 0) {
			pos = (x_out<<16)| (y_out&0xffff);
			printf("Calibration After x_out y_out:%d,%d; %d,%d\n", x_out, y_out, x_old, y_old);
		} else {	// 座標轉換 (取得 PC 端的實際解析度, 這邊假設為 1024x768, 而 AM8250 輸出為 1280x800
			x_out = (int) (1024 * x_out) / 1280;
			y_out = (int) (768 * x_out) / 800;
			check_tmp = x_out + y_out + 1;
			printf("Output PC: %d,%d\n", x_out, y_out);
			strcpy(PC_Data.magic_string, "LPEN");
			PC_Data.packet_length = 22;
			PC_Data.type = 0;
			PC_Data.checksum = check_tmp;
			PC_Data.pointers_dot = 1;
			PC_Data.pointer_ID = 0;
			PC_Data.status = pen_down;
			PC_Data.X_position1 = x_out; 
			PC_Data.y_position1 = y_out; 
		}
}

void hid_process_report(int fd)
{
	struct am_sys_msg msg;
	struct _hiddev_report_info rinfo;
	struct _hiddev_field_info finfo;
	struct _hiddev_usage_ref uref;
	int rtype, i, j;
	static int Xpos = 0, Ypos = 0, old_pos=0;
	char *rtype_str;
	rtype = HID_REPORT_TYPE_INPUT;
	// for (rtype = HID_REPORT_TYPE_MIN; rtype <= HID_REPORT_TYPE_MAX; rtype++) {

	switch (rtype) {
		 case HID_REPORT_TYPE_INPUT: rtype_str = "Input"; break;
		 case HID_REPORT_TYPE_OUTPUT: rtype_str = "Output"; break;
		 case HID_REPORT_TYPE_FEATURE: rtype_str = "Feature"; break;
		 default: rtype_str = "Unknown"; break;
	}

	//HidDbg("[%s %d]Reports of type %s (%d):\n", __func__,__LINE__,rtype_str, rtype);
	 rinfo.report_type = rtype;

	rinfo.report_id = HID_REPORT_ID_FIRST;
	 while (ioctl(fd, HIDIOCGREPORTINFO, &rinfo) >= 0) {
		// HidDbg("[%s %d] Report id: %d (%d fields)\n", __func__,__LINE__, rinfo.report_id, rinfo.num_fields);

			for (i = 0; i < rinfo.num_fields; i++) { 
			 		memset(&finfo, 0, sizeof(struct _hiddev_field_info));
			 		finfo.report_type = rinfo.report_type;
					finfo.report_id = rinfo.report_id;
					finfo.field_index = i;
					ioctl(fd, HIDIOCGFIELDINFO, &finfo);
					//	HidDbg("[%s %d] Field: %d: app: %04x phys %04x " "flags %x (%d usages) unit %x exp %d\n",  __func__,__LINE__,
					//	 i, finfo.application, finfo.physical, finfo.flags,
					//	 finfo.maxusage, finfo.unit, finfo.unit_exponent);
			
					memset(&uref, 0, sizeof(uref));
					memset(HID_value,0,64);
					for (j = 0; j < finfo.maxusage; j++) {   
	            uref.report_type = finfo.report_type;
	            uref.report_id = finfo.report_id;
	            uref.field_index = i;
	            uref.usage_index = j;
	           /*
	            *HIDIOCGUSAGE <HIDIOCGUCODE>
	            *obtain hid device report data ,because hid data stored  in field ->value 
	            */
	            ioctl(fd, HIDIOCGUSAGE, &uref);
	            //printf("  %02x", uref.value);
	            HID_value[j] = uref.value;
					}

	        if(handle_cal_error == 0) {
	        	if(HID_value[3] != 0) {
	        		Xpos = (((HID_value[7] | 0x0000) << 8) | HID_value[6]);
	        		Ypos = (((HID_value[9] | 0x0000) << 8)| HID_value[8]);
	        		printf("Xpos=%x Ypos=%x\n", Xpos,Ypos);	
	        		handle_cal(Xpos,Ypos,cal_time);
	      		} else {
	      			cal_time++;
	      			if (cal_time > 5 )
	     		  			cal_time = 1;
	      		}
	      	}else if(handle_cal_error == 1){
      				Xpos = (((HID_value[7] | 0x0000) << 8) | HID_value[6]);
        			Ypos = (((HID_value[9] | 0x0000) << 8)| HID_value[8]);
        			//printf("pen Xpos=%x Ypos=%x\n", Xpos,Ypos);
        			calcluate_new_position(Xpos,Ypos);
	   					old_pos = (x_old<<16)| (y_old&0xffff);

	        		if(HID_value[3] != 0) {	// pen down
	        			if (pen_down == 0) {
										pen_down = 1;
				      	    SWF_Message(NULL, SWF_MSG_LBUTTONDOWN, (void *)pos);
		        				printf("pen down\n");
	        			} else if (old_pos != pos) {
	        					pen_down = 2;
				        	  SWF_Message(NULL, SWF_MSG_MOVE, (void *)pos);
		        				printf("pen move\n");
	        			}
	        		} else if ((pen_down != 0) && (HID_value[3] == 0)) {	// pen up
	        				pen_down = 0;
			        		SWF_Message(NULL, SWF_MSG_LBUTTONUP, (void *)old_pos);
		       				printf("pen up\n");
	        		}
	      	}else{
	      	  	pos = 0;
	      	  	pen_down = 0;
	      	  	handle_cal_error = 0;
	      	  	cal_time = 1;
	      	  	printf("Cal_ERROR\n");
	      	}
					
				rinfo.report_id |= HID_REPORT_ID_NEXT;
			}
	 }

}

static int hid_handle_cal(void * handle)
{

	SWFEXT_FUNC_BEGIN(handle);
	
	handle_cal_error = Swfext_GetNumber();
	printf("handle_cal_error=%d\n", handle_cal_error);
	cal_time = 1;
	SWFEXT_FUNC_END();
}

static int hid_cal_time(void * handle)
{

	SWFEXT_FUNC_BEGIN(handle);
	printf("cal_time=%d\n", cal_time);
	Swfext_PutNumber(cal_time);
	
	SWFEXT_FUNC_END();
}

int hid_start()
{
	return ( ezFuiHidStart() );
	
	HidInfo("hid\n");
	if(hid_open()>0)
		return 0;
	return -1;
}
int hid_stop()
{
	return ( ezFuiHidStop() );

	HidInfo();
	pthread_cancel(hid);
	if(hid_fd2!=-1){
		if(close(hid_fd2)!=0)
			goto err1;
	}
err1:
	if(hid_fd1!=-1){
		if(close(hid_fd1)!=0)
			goto err2;
	}
err2:
	if(hid_fd!=-1){
		if(close(hid_fd)!=0)
			return -1;
	}
	//hid_fd=hid_fd1=hid_fd2=-1
	return 0;
}

int swfext_hid_register(void)
{
	SWFEXT_REGISTER("hid_handlecal", hid_handle_cal);
	SWFEXT_REGISTER("hid_caltime", hid_cal_time);
	return 0;
}