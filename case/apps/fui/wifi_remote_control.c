#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/msg.h>
#include <sys_pmu.h>
#include <sys_rtc.h>

#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <swf_types.h>


#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h> /* close() */
#include <string.h> /* memset() */

#include "wifi_remote_control.h"
#include "swf_ext.h"
#include "wifi_engine.h"

extern current_status wlan_current_status;

#if 0 //Move to LSDK/sdk/library/wifi_subdisplay/ez_remote.c
static pthread_t recv_stream_tid = -1;
static pthread_t parse_stream_tid = -1;
static int connection_interval = 0;

int parseMessage(char *);
static struct sockaddr_in cliAddr, servAddr;
void processKeyEvent(int msg_type);
extern int wifi_remote_control_started;

static int _inform_remote_control_change()
{
	SWF_Message(NULL,SWF_REMOTE_MSG_CHANGE,NULL);
	SWF_Message(NULL, SWF_MSG_PLAY, NULL);

	return 0;
}


int _inform_remote_control_connected()
{
	SWF_Message(NULL,SWF_REMOTE_MSG_CONNECTED,NULL);
	SWF_Message(NULL, SWF_MSG_PLAY, NULL);
	return 0;
}

int _inform_remote_control_disconnected()
{
	SWF_Message(NULL,SWF_REMOTE_MSG_DISCONNECTED,NULL);
	SWF_Message(NULL, SWF_MSG_PLAY, NULL);
	return 0;
}

/**
* Remote Control Entry
**/
void *remote_control_main(void *arg)
{
	int fd,ret,i;
	char test_buf[128];
	char fpath[128];
	int mgr_msgq_id;
	int err;
	int msgsize;
	int key=0;
	int opt;
	
	int sd, rc, n, cliLen, flags;
	struct sockaddr_in cliAddr, servAddr;
	char msg[MAX_MSG];

  //Save client addr
  struct addrinfo addrC;
  struct addrinfo *addrL;
  struct addrinfo *temp;

  memset(&addrC, 0, sizeof(addrC));
  addrC.ai_family = AF_INET;
  addrC.ai_socktype = SOCK_STREAM;
  addrC.ai_protocol = IPPROTO_TCP;
  char addrBuf[MAX_MSG];
  //Save client addr

	printf("##### [Wifi Remote Control] This test code is mainly for wifi remote control\n");

    /* socket creation */
	printf("##### [Wifi Remote Control] create socket\n");

    sd=socket(AF_INET, SOCK_DGRAM, 0);
    if(sd<0) {
      printf("##### [Wifi Remote Control]: cannot open socket \n");
      //exit(1);
    }

    /* bind local server port */
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(LOCAL_SERVER_PORT);

	// address reuse.
	opt = 1;
	ret = setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	if(ret<0){
		printf("set SO_REUSEADDR error!! ret = %d\n",ret);
	}

    rc = bind (sd, (struct sockaddr *) &servAddr,sizeof(servAddr));
    if(rc<0) {
      printf("##### [Wifi Remote Control]: cannot bind port number %d \n", 
  	   LOCAL_SERVER_PORT);
      exit(1);
    }

    printf("##### [Wifi Remote Control]: waiting for data on port UDP %u\n", 
  	   LOCAL_SERVER_PORT);

    flags = 0;

	/** create the mainloop message queue */
	mgr_msgq_id = msgget(IPC_PRIVATE,IPC_CREAT);
	if(mgr_msgq_id == -1){
		printf("##### [Wifi Remote Control] create manager msg queue err\n");
		exit(1);
	}

    /* server infinite loop */
    while(1) {
      /* init buffer */
      memset(msg,0x0,MAX_MSG);

      /* receive message */
      cliLen = sizeof(cliAddr);
      n = recvfrom(sd, msg, MAX_MSG, flags, (struct sockaddr *) &cliAddr, &cliLen);

      if(n<0) {
        printf("##### [Wifi Remote Control]: cannot receive data \n");
        continue;
      }
      else{
      	connection_interval = 0; // temporary reset, need set timeout somewhere
      }
      
      /* print received message */
      printf("##### [Wifi Remote Control]: from %s:UDP %u : %s\n", 
  	   inet_ntoa(cliAddr.sin_addr),
  	   ntohs(cliAddr.sin_port),msg);

   // parse incoming message and parse
    sleep(0.5);
    connection_interval+=0.5;

     int (*my_func_ptr)(char *);
     my_func_ptr = parseMessage(msg);

     //sendto(sd,msg,n,flags,(struct sockaddr *)&cliAddr,cliLen);
		// Echo to client ther server address anyway...
     {
			#ifdef _DESKTOP_ // in DESKTOP, use getaddrinfo()
				
       if (getaddrinfo(argv[1], "http", &addrC, &addrL) != 0)
       {
           perror("getaddrinfo!");
           exit(1);
       }

       for (temp = addrL; temp != NULL; temp = temp->ai_next)
       {
           void *addrCount = &((struct sockaddr_in*)temp->ai_addr)->sin_addr;             
          inet_ntop(temp->ai_addr->sa_family, addrCount, addrBuf, sizeof(addrBuf));
           //printf("\n\n\n#######%s\n", addrBuf);
       }
       for (temp = addrL; temp != NULL; temp = addrL)
       {
           addrL = temp->ai_next;
           free(temp);
       }
			#endif // _DESKTOP_       
#if 1 //[Sanders.121221] - Replace Ez remote message format.
		char sndBuf[1000] = { 0, };
		int sndLen;
		ezMsgGetRemote(sndBuf, sizeof(sndBuf));
		sndLen = strlen(sndBuf) + 1;
		sendto(sd,sndBuf,sndLen,flags,(struct sockaddr *)&cliAddr,cliLen);
#else
				int rtn = 0;
				char ip_addr[18] = {0};
				printf("##### [Wifi Remote Control]: ip address is %s\n",wlan_current_status.ip_address);
				memcpy(ip_addr,wlan_current_status.ip_address,strlen(wlan_current_status.ip_address));

	     sendto(sd,ip_addr,strlen(ip_addr)+1,flags,(struct sockaddr *)&cliAddr,cliLen);
#endif				
     }

     // Maybe echo to client about server address with server device name?
     /*
     sleep(0.5);
     printf("\n\n%s\n\n", getDeviceName());
     sendto(sd,getDeviceName(),strlen(getDeviceName())+1,flags,(struct sockaddr *)&cliAddr,cliLen);     
     */
     //sendto(sd,msg,strlen(msg)+1,flags,(struct sockaddr *)&cliAddr,cliLen);

     // send swf messge : connected/disconnected
     //_inform_remote_control_connected();

      printf("##### [Wifi Remote Control]: connection_interval = %d\n", 
  	   connection_interval);

     if(connection_interval > REMOTE_CONTROL_TIMEOUT)
     {
	    printf("##### [Wifi Remote Control]: _inform_remote_control_disconnected\n");
     	_inform_remote_control_disconnected();
     }
     else
     {
	    printf("##### [Wifi Remote Control]: _inform_remote_control_connected\n");
		_inform_remote_control_connected();
     }
    }/* end of server infinite loop */

}

void parseMsgCommand(int msg_command, int msg_type)
{	
  switch(msg_command){
    case COMMAND_CONNECT:
    {
      // Since we echo to client everytime, no need to process connect command.
      // Echo server info (ip, hostname), status if available
      printf("[parseMsgCommand] = CONNECT\n");
      //processConnEvent(msg_type);
      break;
    }
    case COMMAND_KEY:
    {
      // Pass key events
      printf("[parseMsgCommand] = KEY\n");
      processKeyEvent(msg_type);
      break;
    }
    case COMMAND_MOUSE:
    {
      printf("[parseMsgCommand] = MOUSE\n");
      //processMouseEvent(msg_type);
      break;
    }
  }
  
}

void processConnEvent(int msg_type)
{
  switch(msg_type)
  {
    case CONN_LOOKUP:
    case CONN_FOUND:
    case CONN_CONNECT:
    case CONN_DISCONNECT:
    {	
      break;
    }
  }
}

void processKeyEvent(int msg_type)
{
  printf("[processKeyEvent] = %d\n", msg_type);
  switch(msg_type)
  {
    case KEY_LEFT:
    {
      sendKey(0x125);      
      break;
    }
    case KEY_UP:
    {
      sendKey(0x126);      
      break;
    }
    case KEY_RIGHT:
    {
      sendKey(0x127);
      break;
    }
    case KEY_DOWN:
    {
      sendKey(0x128);
      break;
    }
    case KEY_ENTER:
    {
      sendKey(0x10d);
      break;
    }
    case KEY_ESC:
    {
      sendKey(0x11b);
      break;
    }
  }
  //return;
}

void processKeyboardEvent(char* msg)
{
	char str_a[] = "a";
	char str_A[] = "A";
	char str_0[] = "0";
	
	printf("[processKeyboardEvent] %s\n", msg);
	int compareResult = 0;
	int compareToA = 0;
	int compareToNumber = 0;

	compareResult = strcmp(msg, str_a);
	compareToNumber = strcmp(msg, str_0);
	compareToA = strcmp(msg, str_A);
	printf("####### compare with 0 = %d\n", strcmp(msg, str_0));
	printf("####### compare with A = %d\n", compareToA);
	printf("####### compare with a = %d\n", compareResult);
	
	if(compareToNumber>=0 && compareToNumber <=9)
	{
		switch(compareToNumber){
			case 0:
			{
				sendKey(REMOTE_MSG_KEY_NUMPAD0);
				break;
			}
			case 1:
			{
				sendKey(REMOTE_MSG_KEY_NUMPAD1);
				break;
			}
			case 2:
			{
				sendKey(REMOTE_MSG_KEY_NUMPAD2);
				break;
			}
			case 3:
			{
				sendKey(REMOTE_MSG_KEY_NUMPAD3);
				break;
			}
			case 4:
			{
				sendKey(REMOTE_MSG_KEY_NUMPAD4);
				break;
			}
			case 5:
			{
				sendKey(REMOTE_MSG_KEY_NUMPAD5);
				break;
			}
			case 6:
			{
				sendKey(REMOTE_MSG_KEY_NUMPAD6);
				break;
			}
			case 7:
			{
				sendKey(REMOTE_MSG_KEY_NUMPAD7);
				break;
			}
			case 8:
			{
				sendKey(REMOTE_MSG_KEY_NUMPAD8);
				break;
			}
			case 9:
			{
				sendKey(REMOTE_MSG_KEY_NUMPAD9);
				break;
			}
			default:
				break;
		}		
	}

	if(compareToA>=0)
	{
		switch(compareToA){
			case 0:
			{
				sendKey(REMOTE_MSG_KEY_A);
				break;
			}
			case 1:
			{
				sendKey(REMOTE_MSG_KEY_B);
				break;
			}
			case 2:
			{
				sendKey(REMOTE_MSG_KEY_C);
				break;
			}
			case 3:
			{
				sendKey(REMOTE_MSG_KEY_D);
				break;
			}
			case 4:
			{
				sendKey(REMOTE_MSG_KEY_E);
				break;
			}
			case 5:
			{
				sendKey(REMOTE_MSG_KEY_F);
				break;
			}
			case 6:
			{
				sendKey(REMOTE_MSG_KEY_G);
				break;
			}
			case 7:
			{
				sendKey(REMOTE_MSG_KEY_H);
				break;
			}
			case 8:
			{
				sendKey(REMOTE_MSG_KEY_I);
				break;
			}
			case 9:
			{
				sendKey(REMOTE_MSG_KEY_J);
				break;
			}
			case 10:
			{
				sendKey(REMOTE_MSG_KEY_K);
				break;
			}
			case 11:
			{
				sendKey(REMOTE_MSG_KEY_L);
				break;
			}
			case 12:
			{
				sendKey(REMOTE_MSG_KEY_M);
				break;
			}
			case 13:
			{
				sendKey(REMOTE_MSG_KEY_N);
				break;
			}
			case 14:
			{
				sendKey(REMOTE_MSG_KEY_O);
				break;
			}
			case 15:
			{
				sendKey(REMOTE_MSG_KEY_P);
				break;
			}
			case 16:
			{
				sendKey(REMOTE_MSG_KEY_Q);
				break;
			}
			case 17:
			{
				sendKey(REMOTE_MSG_KEY_R);
				break;
			}
			case 18:
			{
				sendKey(REMOTE_MSG_KEY_S);
				break;
			}
			case 19:
			{
				sendKey(REMOTE_MSG_KEY_T);
				break;
			}
			case 20:
			{
				sendKey(REMOTE_MSG_KEY_U);
				break;
			}
			case 21:
			{
				sendKey(REMOTE_MSG_KEY_V);
				break;
			}
			case 22:
			{
				sendKey(REMOTE_MSG_KEY_W);
				break;
			}
			case 23:
			{
				sendKey(REMOTE_MSG_KEY_X);
				break;
			}
			case 24:
			{
				sendKey(REMOTE_MSG_KEY_Y);
				break;
			}
			case 25:
			{
				sendKey(REMOTE_MSG_KEY_Z);
				break;
			}
			default:
				break;
		}
	}
// sendKey(0x141);      // A
	
	if(compareResult>=0)
	{
		switch(compareResult){
			case 0:
			{
				sendKey(REMOTE_MSG_KEY_a);
				break;
			}
			case 1:
			{
				sendKey(REMOTE_MSG_KEY_b);
				break;
			}
			case 2:
			{
				sendKey(REMOTE_MSG_KEY_c);
				break;
			}
			case 3:
			{
				sendKey(REMOTE_MSG_KEY_d);
				break;
			}
			case 4:
			{
				sendKey(REMOTE_MSG_KEY_e);
				break;
			}
			case 5:
			{
				sendKey(REMOTE_MSG_KEY_f);
				break;
			}
			case 6:
			{
				sendKey(REMOTE_MSG_KEY_g);
				break;
			}
			case 7:
			{
				sendKey(REMOTE_MSG_KEY_h);
				break;
			}
			case 8:
			{
				sendKey(REMOTE_MSG_KEY_i);
				break;
			}
			case 9:
			{
				sendKey(REMOTE_MSG_KEY_j);
				break;
			}
			case 10:
			{
				sendKey(REMOTE_MSG_KEY_k);
				break;
			}
			case 11:
			{
				sendKey(REMOTE_MSG_KEY_l);
				break;
			}
			case 12:
			{
				sendKey(REMOTE_MSG_KEY_m);
				break;
			}
			case 13:
			{
				sendKey(REMOTE_MSG_KEY_n);
				break;
			}
			case 14:
			{
				sendKey(REMOTE_MSG_KEY_o);
				break;
			}
			case 15:
			{
				sendKey(REMOTE_MSG_KEY_p);
				break;
			}
			case 16:
			{
				sendKey(REMOTE_MSG_KEY_q);
				break;
			}
			case 17:
			{
				sendKey(REMOTE_MSG_KEY_r);
				break;
			}
			case 18:
			{
				sendKey(REMOTE_MSG_KEY_s);
				break;
			}
			case 19:
			{
				sendKey(REMOTE_MSG_KEY_t);
				break;
			}
			case 20:
			{
				sendKey(REMOTE_MSG_KEY_u);
				break;
			}
			case 21:
			{
				sendKey(REMOTE_MSG_KEY_v);
				break;
			}
			case 22:
			{
				sendKey(REMOTE_MSG_KEY_w);
				break;
			}
			case 23:
			{
				sendKey(REMOTE_MSG_KEY_x);
				break;
			}
			case 24:
			{
				sendKey(REMOTE_MSG_KEY_y);
				break;
			}
			case 25:
			{
				sendKey(REMOTE_MSG_KEY_z);
				break;
			}
			default:
				break;
		}
	}
	
}

void virtualKeyboardEvent(int msg)
{
	int msg_num = 0;
	char *newInfo;
	newInfo = SWF_Malloc(2);
	memset(newInfo, 0, 2);
	
	
	
	printf("[processKeyboardEvent] %d\n", msg);

	if(msg > 0 && msg < 128){
		printf("it is char %c\n", (char)msg);
		newInfo[0] = (char)msg;
		msg_num =48 + (msg - '0');
		msg_num |= SWF_MSG_IME;
		
		printf("send var is 0x%x\n", msg_num);
//		printf("newInfo = %s\n", newInfo);
//		printf("len of newInfo is %d\n", strlen(newInfo));
		SWF_Message(NULL, msg_num, newInfo);
		//sendKey(msg_num);
	}else if(msg == -1){
		printf("^^^^^^^^^Backspace\n");
		msg_num =8;
		msg_num |= SWF_MSG_IME;
		printf("send var is 0x%x\n", msg_num);
//		printf("newInfo = %s\n", newInfo);
//		printf("len of newInfo is %d\n", strlen(newInfo));
		SWF_Message(NULL, msg_num, newInfo);
	}

			
}

int parseMessage(char *recv_msg)
{
  int ret = 0;
  char *p1, *p2;
  //printf("[parseMessage] msg = %s\n", recv_msg);
  //if(strlen(recv_msg)>5)
  if(strlen(recv_msg)>20)
  {
    printf("error! unexpected msg!\n");
  }
  else{
    printf("parsing...\n");
    p1 = strtok(recv_msg, ":");
    if (p1){
      p2 = strtok(NULL, ":");
      if (p2)
      {
      	switch(atoi(p1))
      	{
					case COMMAND_CONNECT:
					case COMMAND_KEY:
					{
					 	printf("p1 = %d, p2 = %d\n", atoi(p1), atoi(p2));
					  parseMsgCommand( atoi(p1), atoi(p2) );
					  break;
					}
					case COMMAND_KEYBOARD:
					{
	        	printf("processKeyboardEvent\n");
						printf("p1 = %d, p2 = %s\n", atoi(p1), p2);
						processKeyboardEvent( p2 );
			 			break;
					}
					case 10:		// for virtual keyboard from android or ios
						printf("processKeyboardEvent\n");
						printf("p1 = %d, p2 = %s\n", atoi(p1), p2);
						virtualKeyboardEvent(atoi(p2));
						break;
					default:
						break;
				}
      }
    }
  }

  return ret;
}


int wifi_remote_control_init(void){
	int ret = 0;
	char *message1 = "########## [Wifi Remote Control] Thread message.";
	
	//ret = pthread_create(&recv_stream_tid,NULL, wifi_remote_control_recv_msg, (void*) message1);
	ret = pthread_create(&recv_stream_tid,NULL, remote_control_main, (void*) message1);
	if(ret){
		printf("##### [Wifi Remote Control]: Create recv msg thread error!\n");
		//goto start_out;
	}
	else{
		printf("##### [Wifi Remote Control]: Create recv msg thread success!\n");
	}

	return ret;
}

int wifi_remote_control_exit(void){
	int ret = 0;
	char *message_exit = "########## [Wifi Remote Control] Exit Thread message.";

//	ret = pthread_exit(&recv_stream_tid,NULL, remote_control_main, (void*) message_exit);
	ret = pthread_cancel(recv_stream_tid);
	if(ret){
		printf("##### [Wifi Remote Control]: exit thread error!\n");
	}
	else{
		wifi_remote_control_started = 0;
		printf("##### [Wifi Remote Control]: exit thread success!\n");
	}



	return ret;
}

int sendKey(int keycode){
	int result = 0;
  printf("########## [sendKey]: keycode = %d\n", keycode);
	result = SWF_Message(NULL,keycode,NULL);
  printf("########## [sendKey]: keycode");
	return result;
}

void wifi_remote_control_recv_msg(void* arg){
    char *message;
    message = (char*) arg;
		printf("##### [Wifi Remote Control] wifi_remote_control_recv_msg: %s\n", message);
}

void wifi_remote_control_parse_image(void* arg){
	
}
#else

extern int wifi_remote_control_started;

int wifi_remote_control_init(void){
	int ret = 0;
	ret = ezFuiRemoteStart();
	return ret;
}

int wifi_remote_control_exit(void){
	int ret = 0;
	ret = ezFuiRemoteStop();
	if (ret == 0)
	{
		wifi_remote_control_started = 0;
	}
	return ret;
}

#endif

