#define MIRACAST_ENALBE

#ifdef MIRACAST_ENALBE

#include <pthread.h>
#include <pwd.h> 
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <time.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include "swf_ext.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <stdarg.h>

#include "video_engine.h"
#include "display.h"
#include "fui_common.h"

pthread_t miracast_thread_id;
static int loop_exit = 0;

#define RTP_PORT_DEFAULT  42030

#ifdef MODULE_CONFIG_HDCP_ENABLE
#define MIRACAST_HDCP_SUPPORT
#endif

#ifdef MIRACAST_HDCP_SUPPORT
#include "hdcp2_api.h"
pthread_t miracast_hdcp_thread_id = -1;
#endif

#define WIFI_DIRECT_JSON 1

#ifdef WIFI_DIRECT_JSON
extern struct p2p action_p2p;
extern int writeToJsonFile(struct p2p * p2p,char * error_tmp);
static int write_json_file_flag = 0;
#endif

enum{
	MIRACAST_STAT_IDLE=0,
	MIRACAST_STAT_CONNECTING,
	MIRACAST_STAT_CONNECTED,
};
static int miracast_stat=MIRACAST_STAT_IDLE;

enum{
	MIRACAST_RTSP_STAT_IDLE=0,
	MIRACAST_RTSP_STAT_PLAYING=1,
	MIRACAST_RTSP_STAT_STOP=2,
};
static int miracast_rtsp_stat=MIRACAST_RTSP_STAT_IDLE;
static int miracast_wifi_drop_flag=0;

enum{
	MIRACAST_CMD_PLAY=0,
	MIRACAST_CMD_PAUSE,
	MIRACAST_CMD_STOP,
	MIRACAST_CMD_P2P_FAILED
};

typedef struct miracast_cmd_s miracast_cmd_t;
struct miracast_cmd_s
{
	int cmd;
	int param;
};

struct miracast_rtsp_control
{
	/** server and client socket */
	int sfd,cfd;

	/** rtsp process id*/
	pid_t rtsp_pid;
    //pthread_mutex_t ctrl_locker;
};
struct miracast_rtsp_control mcc;


/**
* for DE config.
*/
extern void *deinst;
static DE_config miracast_de_saved_conf;
static int _miracast_set_de_defaultcolor()
{
	DE_config ds_conf;
	
	de_get_config(deinst,&ds_conf,DE_CFG_ALL);
	ds_conf.input.enable=0;
	de_set_Config(deinst,&ds_conf,DE_CFG_ALL);
	
	return 0;
}

static int  _miracast_save_de_config()
{
	de_get_config(deinst,&miracast_de_saved_conf,DE_CFG_ALL);
	
	return 0;
}

static int  _miracast_restore_de_config(int setdefault)
{
	if(setdefault){
		miracast_de_saved_conf.input.enable=0;
	}
	de_set_Config(deinst,&miracast_de_saved_conf,DE_CFG_ALL);
	
	return 0;
}

/**
* @brief Function for execute a shell script.
* 
* @note The last arg for arglist must be NULL.
*/
static int miracast_exec_shell_script(char *script,char *arglist,...)
{
#define INITIAL_ARGV_MAX 1024

	const char *argv[INITIAL_ARGV_MAX];
	pid_t pcid;
	pid_t wait_pid;
	int execstat;
	int status;
	va_list args;
	unsigned int i;

	/**
	* process the args.
	*/
	argv[0] = "sh";
	argv[1] = script;
	argv[2] = arglist;

	va_start (args, arglist);

	i = 2;
	while (argv[i] != NULL)
	{
		i++;
		argv[i] = va_arg (args, const char *);
	}
	
	va_end (args);
	
	/**
	* exec sh.
	*/
	pcid = fork();
	if(pcid == -1){
		printf("[%s]: fork error\n",__FUNCTION__);
		return -1;
	}
	else if(pcid == 0){
		execstat = execv("/bin/sh", (char *const *)argv);
		if(execstat == -1){
			printf("[%s]: execl error\n",__FUNCTION__);
			exit(1);
		}
	}
	else{
		wait_pid = wait(&status);
		if(wait_pid == -1){
			printf("[%s]:wait for child error\n",__FUNCTION__);
			return -1;
		}
		if(WIFEXITED(status)){
			printf("[%s]:normal termination\n",__FUNCTION__);
		}	
		if(WIFSIGNALED(status)){
			printf("[%s]:signal exit\n",__FUNCTION__);
		}
		if(WIFSTOPPED(status)){
			printf("[%s]:stopped\n",__FUNCTION__);
		}
		return 0;
	}

	return 0;
	
}


static int miracast_get_player_heap(unsigned char **pvbus,unsigned int *pbus,unsigned int *psize)
{
	if(pvbus == NULL || pbus == NULL || psize == NULL){
		return -1;
	}

	*pvbus = (unsigned char *)SWF_Malloc(_VIDEO_HEAP1_SIZE);
	if(*pvbus != NULL){
		*psize = _VIDEO_HEAP1_SIZE;
		goto __get_miracast_player_physics;
	}

	*pvbus = (unsigned char *)SWF_Malloc(_VIDEO_HEAP2_SIZE);
	if(*pvbus != NULL){
		*psize = _VIDEO_HEAP2_SIZE;
		goto __get_miracast_player_physics;
	}

	*pvbus = (unsigned char *)SWF_Malloc(_VIDEO_HEAP3_SIZE);
	if(*pvbus != NULL){
		*psize = _VIDEO_HEAP3_SIZE;
		goto __get_miracast_player_physics;
	}

	return -1;
	
__get_miracast_player_physics:

	*pbus = (unsigned int)fui_get_bus_address((unsigned long)*pvbus);

	return 0;
	
}

static int miracast_rtsp_alive()
{
	return 1;
}

static int miracast_get_p2p_status()
{
	FILE *fp;
	int stat = MIRACAST_STAT_IDLE;
	char *ptr;
	char buf[16];

	fp = fopen("/tmp/p2pstatus","r");
	if(fp){
		ptr = fgets(buf, 16, fp);
		if(ptr){
			if(strstr(buf,"init")){
				stat = MIRACAST_STAT_IDLE;
			}
			else{
				stat = MIRACAST_STAT_CONNECTING;
			}
		}
		fclose(fp);
	}

	return stat;
}

static void *miracast_loop(void *arg)
{
	
	int sfd=-1, size;
	int cfd=-1;
	socklen_t csize;
    struct sockaddr_un un,un1;
	pid_t mira_app_id;
	pid_t wait_pid;
	int status;
	int execstat;
	fd_set rfd_set;
	int cmd;
	struct timeval tv;
	int sret;
	unsigned char *heap1=NULL;
	unsigned int bus;
	unsigned int hsize;

	loop_exit = 0;

	/**
	*********************************************
	* First create the IPC channel.
	*********************************************
	*/

    un.sun_family = AF_UNIX;
    strcpy(un.sun_path, "/tmp/miracast_server");
	if ((sfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0){
		printf("[%s]: socket create error\n",__FUNCTION__);
		goto miracast_loop_out;
	}

	/** before bind, must make sure the path file not exist */
	unlink(un.sun_path);
	
	size = offsetof(struct sockaddr_un, sun_path) + strlen(un.sun_path);
	if (bind(sfd, (struct sockaddr *)&un, size) < 0){
		printf("[%s]: bind error\n",__FUNCTION__);
		goto miracast_loop_out;
	}

	if (listen(sfd, 5) < 0){
		printf("[%s]: listen error\n",__FUNCTION__);
		goto miracast_loop_out;
	}

	printf("[%s]: IPC channel ok\n",__FUNCTION__);

	/**
	*********************************************
	* Start the P2P and RTSP app.
	*********************************************
	*/

miracast_loop_start_rtsp:

	miracast_stat=MIRACAST_STAT_IDLE;

	mira_app_id = fork();
	if(mira_app_id == -1){
		printf("[%s]: fork error\n",__FUNCTION__);
		goto miracast_loop_out;
	}
	else if(mira_app_id == 0){
		
		execstat = execl("/am7x/bin/miracast.app","miracast.app",NULL);
		if(execstat == -1){
			printf("[%s]: execl error\n",__FUNCTION__);
			exit(1);
		}
	}
	else{
		/** accept the miracast app connection */
		/** often errno=EINTR, if signal caught */
	miracast_loop_accept:
		csize = sizeof(un1);
		cfd = accept(sfd, (struct sockaddr *)&un1, &csize);
		if(cfd < 0){
			int errsv = errno;
			if(errsv == EINTR){
				//Restart if interrupted by handler
				goto miracast_loop_accept;	
			}
			else{
				printf("[%s]: accept error %d\n",__FUNCTION__,errsv);
				kill((int)mira_app_id,SIGKILL);
				goto miracast_loop_out;
			}
		}
		else{
			/** 
			* we get the connection from miracast.app
			* and will begin to process the messages.
			*/
			printf("[%s]: accept ok, %d\n",__FUNCTION__,cfd);

			while(loop_exit == 0){
				
				FD_ZERO(&rfd_set);
				FD_SET(cfd, &rfd_set);

				tv.tv_sec = 1;
				tv.tv_usec = 0;

				/** if connected, p2p status has no meaning */
				if(miracast_stat!=MIRACAST_STAT_CONNECTED){
					miracast_stat = miracast_get_p2p_status();
				}

				/** timeout each second for get the chance to check loop_exit */
				sret = select(cfd+1,&rfd_set,NULL,NULL,&tv);

				if(sret>0){
					
					if(FD_ISSET(cfd,&rfd_set)){
						miracast_cmd_t cmd;
						int cmdsize;
						cmdsize = read(cfd,&cmd,sizeof(cmd));
						if(cmdsize == sizeof(cmd)){
							switch(cmd.cmd){
								case MIRACAST_CMD_PLAY:

								{
									
									miracast_stat=MIRACAST_STAT_CONNECTED;
									
									printf("[%s]: receive MIRACAST_CMD_PLAY command,port:%d\n",__FUNCTION__,cmd.param);
									/** sleep SWF */
									if(SWF_GetState(SWF_GetActiveInst()) & SWF_STATE_ACTIVE)
									{
										SWF_SetState(SWF_GetActiveInst(), SWF_STATE_SLEEP);
									}

									if(miracast_get_player_heap(&heap1,&bus,&hsize) == 0){
										printf("[%s]: get heap ok,0x%x,0x%x,0x%x\n",__FUNCTION__,heap1,bus,hsize);
										mirartpStreamStart(cmd.param,heap1,bus,hsize,IMAGE_WIDTH_E,IMAGE_HEIGHT_E);
									}
									else{
										printf("[%s]: get heap error\n",__FUNCTION__);
										if((SWF_GetState(SWF_GetActiveInst()) & SWF_STATE_ACTIVE) == 0){
											SWF_SetState(SWF_GetActiveInst(), SWF_STATE_ACTIVE);
										}
									}
								}
									
								break;

								case MIRACAST_CMD_PAUSE:
									printf("[%s]: receive MIRACAST_CMD_PAUSE command\n",__FUNCTION__);
								break;

								case MIRACAST_CMD_P2P_FAILED:
									printf("[%s]: receive MIRACAST_CMD_P2P_FAILED command\n",__FUNCTION__);
									if(mira_app_id > 0){
										wait_pid = wait(&status);
										if(wait_pid == mira_app_id){
											printf("[%s]: Kill the RTSP\n",__FUNCTION__);
										}
									}
									printf(">>>>>>> %d,%s\n",__LINE__,__FILE__);
									/** start the next time P2P and RTSP */
									goto miracast_loop_start_rtsp;
									
								break;

								case MIRACAST_CMD_STOP:
									printf("[%s]: receive MIRACAST_CMD_STOP command\n",__FUNCTION__);

									/** stop the streaming */
									mirartpStreamStop();

									printf(">>>>>>> %d,%s\n",__LINE__,__FILE__);

									/** stop the RTSP process */
									if(mira_app_id > 0 && miracast_rtsp_alive()){
										kill((int)mira_app_id,SIGKILL);
										wait_pid = wait(&status);
										if(wait_pid == mira_app_id){
											printf("[%s]: Kill the RTSP\n",__FUNCTION__);
										}
									}

									printf(">>>>>>> %d,%s\n",__LINE__,__FILE__);

									/** close the RTSP communication channel */
									if(cfd > 0){
										close(cfd);
										cfd = -1;
									}

									printf(">>>>>>> %d,%s\n",__LINE__,__FILE__);

									/** release the heap memory */
									if(heap1){
										SWF_Free((void *)heap1);
										heap1 = NULL;
									}

									printf(">>>>>>> %d,%s\n",__LINE__,__FILE__);

									/** wake up the FUI process again */
									if((SWF_GetState(SWF_GetActiveInst()) & SWF_STATE_ACTIVE) == 0)
									{
										SWF_SetState(SWF_GetActiveInst(), SWF_STATE_ACTIVE);
									}

									printf(">>>>>>> %d,%s\n",__LINE__,__FILE__);
									
									if(loop_exit){
										/** someone asked us to quit */
										printf(">>>>>>> %d,%s\n",__LINE__,__FILE__);
										goto miracast_loop_out;
									}
									else{
										/** RTSP out and we contiue to do the next Miracast process */
										printf(">>>>>>> %d,%s\n",__LINE__,__FILE__);
										goto miracast_loop_start_rtsp;
									}
								break;

								default:
								break;
							}
						}
					}
					
				}

			}
			
			/** loop exit, we still need to release the resources */
			printf(">>>>>>> %d,%s\n",__LINE__,__FILE__);
			mirartpStreamStop();
			printf(">>>>>>> %d,%s\n",__LINE__,__FILE__);
			
			if(mira_app_id > 0 && miracast_rtsp_alive()){
				kill((int)mira_app_id,SIGKILL);
				wait_pid = wait(&status);
				if(wait_pid == mira_app_id){
					printf("[%s]: Kill the RTSP\n",__FUNCTION__);
				}
			}
			printf(">>>>>>> %d,%s\n",__LINE__,__FILE__);

			if(heap1){
				SWF_Free((void *)heap1);
				heap1 = NULL;
			}
			printf(">>>>>>> %d,%s\n",__LINE__,__FILE__);
			if((SWF_GetState(SWF_GetActiveInst()) & SWF_STATE_ACTIVE) == 0)
			{
				SWF_SetState(SWF_GetActiveInst(), SWF_STATE_ACTIVE);
			}
			printf(">>>>>>> %d,%s\n",__LINE__,__FILE__);
		}
		

	}
	

miracast_loop_out:

	if(sfd > 0){
		close(sfd);
	}

	if(cfd > 0){
		close(cfd);
	}
	
	pthread_exit(NULL);

	return NULL;
	
}

static int miracast_start()
{
	/**
	* The start function only create the miracast thread.
	* All things done in the thread.
	*/
	if (pthread_create(&miracast_thread_id, NULL, miracast_loop, NULL) == 0){
		return 1;
	}

	printf("[%s],create miracast loop thread error\n",__FUNCTION__);

	return 0;
}

static int miracast_stop()
{
	loop_exit = 1;

	pthread_join(miracast_thread_id, NULL);
	
	miracast_exec_shell_script("/am7x/case/scripts/miracast_exit.sh",NULL);

	return 0;
}

#ifdef MIRACAST_HDCP_SUPPORT

static void _miracast_hdcp_release(void *arg)
{
	hdcp2_release();
}
static void *_miracast_hdcp_loop(void *arg)
{
	/* enter the hdcp loop */
	pthread_cleanup_push(_miracast_hdcp_release,NULL);
	
	hdcp2_start(10123);
	
	pthread_cleanup_pop(1);
	
	pthread_exit(NULL);
}
#endif

static void *_miracast_rtsp_loop(void *arg)
{	
	int size;
	socklen_t csize;
	struct sockaddr_un un,un1;
	fd_set rfd_set;
	int cmd;
	struct timeval tv;
	int sret;
	unsigned char *heap1=NULL;
	unsigned int bus;
	unsigned int hsize;
	struct miracast_rtsp_control *ctrl = (struct miracast_rtsp_control *)arg;
	int play_started = 0;

	loop_exit = 0;
	
_loop_accept:
	play_started = 0;	
	if(loop_exit){
		goto _loop_out;
	}
	csize = sizeof(un1);
	ctrl->cfd = accept(ctrl->sfd, (struct sockaddr *)&un1, &csize);
	if(ctrl->cfd < 0){
		int errsv = errno;
		if(errsv == EINTR){
			//Restart if interrupted by handler
			goto _loop_accept;	
		}
		else{
			printf("[%s]: accept error %d\n",__FUNCTION__,errsv);
			goto _loop_accept;
		}
	}
	else{
		/** 
		* we get the connection from miracast.app
		* and will begin to process the messages.
		*/
		miracast_rtsp_stat = MIRACAST_RTSP_STAT_IDLE;
		
		while(1){
			FD_ZERO(&rfd_set);
			FD_SET(ctrl->cfd, &rfd_set);
			tv.tv_sec = 1;
			tv.tv_usec = 0;

			if(loop_exit){
				goto _loop_out;
			}

			/** timeout each second for get the chance to check loop_exit */
			sret = select(ctrl->cfd+1,&rfd_set,NULL,NULL,&tv);
			if(sret>0){
				if(FD_ISSET(ctrl->cfd,&rfd_set)){
					miracast_cmd_t cmd;
					int cmdsize;
					cmdsize = read(ctrl->cfd,&cmd,sizeof(cmd));
					if(cmdsize == sizeof(cmd)){
						switch(cmd.cmd){
							case MIRACAST_CMD_PLAY:
							{
								printf("[%s]: receive MIRACAST_CMD_PLAY command,port:%d\n",__FUNCTION__,cmd.param);
								if(play_started){
									break;
								}
								play_started = 1;
								
								//pthread_mutex_lock(&ctrl->ctrl_locker);
								
								/** sleep SWF */
								if(SWF_GetState(SWF_GetActiveInst()) & SWF_STATE_ACTIVE)
								{
									SWF_SetState(SWF_GetActiveInst(), SWF_STATE_SLEEP);
								}
								
								if(miracast_get_player_heap(&heap1,&bus,&hsize) == 0){
									printf("[%s]: get heap ok,0x%x,0x%x,0x%x\n",__FUNCTION__,heap1,bus,hsize);
									_miracast_set_de_defaultcolor();
									printf("###defualt color!\n");
									mirartpStreamStart(cmd.param,heap1,bus,hsize,IMAGE_WIDTH_E,IMAGE_HEIGHT_E);
								}
								else{
									printf("[%s]: get heap error\n",__FUNCTION__);
									if((SWF_GetState(SWF_GetActiveInst()) & SWF_STATE_ACTIVE) == 0){
										SWF_SetState(SWF_GetActiveInst(), SWF_STATE_ACTIVE);
									}
								}
								
								miracast_rtsp_stat = MIRACAST_RTSP_STAT_PLAYING;

								//pthread_mutex_unlock(&ctrl->ctrl_locker);
							}
								
							break;

							case MIRACAST_CMD_PAUSE:
								printf("[%s]: receive MIRACAST_CMD_PAUSE command\n",__FUNCTION__);
							break;

							case MIRACAST_CMD_STOP:
								printf("[%s]: receive MIRACAST_CMD_STOP command\n",__FUNCTION__);
							stop_and_go_next_run:
								printf("[%s,%d]:miracast_rtsp_stat is %d\n",__FUNCTION__,__LINE__,miracast_rtsp_stat);
							#ifdef WIFI_DIRECT_JSON
								struct p2p *p =NULL;
								p = &action_p2p;
								if(miracast_rtsp_stat  != MIRACAST_RTSP_STAT_PLAYING){
									writeToJsonFile(p,"rtsp connection error");
								}	
								else if(!write_json_file_flag){
									write_json_file_flag = 1;
									writeToJsonFile(p,"no error");									
								}
							#endif
								//pthread_mutex_lock(&ctrl->ctrl_locker);
								
								/** stop the streaming */
								mirartpStreamStop();

								_miracast_restore_de_config(1);

								/** close the RTSP communication channel */
								if(ctrl->cfd > 0){
									close(ctrl->cfd);
									ctrl->cfd = -1;
								}
								/** release the heap memory */
								if(heap1){
									SWF_Free((void *)heap1);
									heap1 = NULL;
								}
								/** wake up the FUI process again */
								if((SWF_GetState(SWF_GetActiveInst()) & SWF_STATE_ACTIVE) == 0)
								{
									SWF_SetState(SWF_GetActiveInst(), SWF_STATE_ACTIVE);
								}
								miracast_rtsp_stat = MIRACAST_RTSP_STAT_STOP;
								miracast_wifi_drop_flag = 0;

								//pthread_mutex_unlock(&ctrl->ctrl_locker);
								
								/** after stop, we do another run */
								goto _loop_accept;
								
							break;

							default:
								printf("%d,%s,%d\n",__LINE__,__FILE__,cmd.cmd);
							break;
						}
					}
					else if(cmdsize == 0){
						/** if socket close, read will return 0 */
						if(miracast_wifi_drop_flag){
							printf("[%s]: receive Wifi drop command2\n",__FUNCTION__);
							miracast_wifi_drop_flag = 0;
							goto stop_and_go_next_run;
						}
					}
				}
			}
			else if(sret == 0){
				/** 
				* check if request to disconnect because of Wifi drop.
				*/
				if(miracast_wifi_drop_flag){
					printf("[%s]: receive Wifi drop command\n",__FUNCTION__);
					miracast_wifi_drop_flag = 0;
					goto stop_and_go_next_run;
				}
			}
		}
	}

_loop_out:

	mirartpStreamStop();

	if(play_started){
		_miracast_restore_de_config(1);
	}

	if(ctrl->cfd > 0){
		close(ctrl->cfd);
		ctrl->cfd = -1;
	}
	
	if(heap1){
		SWF_Free((void *)heap1);
		heap1 = NULL;
	}
	
	if((SWF_GetState(SWF_GetActiveInst()) & SWF_STATE_ACTIVE) == 0)
	{
		SWF_SetState(SWF_GetActiveInst(), SWF_STATE_ACTIVE);
	}
	
	pthread_exit(NULL);
	return NULL;
}


/**
* Preparation for RTSP.
*/
static int _miracast_rtsp_init(struct miracast_rtsp_control *ctrl)
{
	struct sockaddr_un un;
	int size;
	socklen_t csize;

	if(ctrl == NULL){
		return -1;
	}

	ctrl->sfd = -1;
	ctrl->cfd = -1;
	ctrl->rtsp_pid = -1;
	
	un.sun_family = AF_UNIX;
    strcpy(un.sun_path, "/tmp/miracast_server");
	if ((ctrl->sfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0){
		printf("[%s]: socket create error\n",__FUNCTION__);
		return -1;
	}

	/** before bind, must make sure the path file not exist */
	unlink(un.sun_path);
	
	size = offsetof(struct sockaddr_un, sun_path) + strlen(un.sun_path);
	if (bind(ctrl->sfd, (struct sockaddr *)&un, size) < 0){
		printf("[%s]: bind error\n",__FUNCTION__);
		close(ctrl->sfd);
		ctrl->sfd = -1;
		return -1;
	}

	if (listen(ctrl->sfd, 5) < 0){
		printf("[%s]: listen error\n",__FUNCTION__);
		close(ctrl->sfd);
		ctrl->sfd = -1;
		return -1;
	}

	if (pthread_create(&miracast_thread_id, NULL, _miracast_rtsp_loop, (void *)ctrl) != 0){
		printf("[%s]: rtsp loop create error\n",__FUNCTION__);
		close(ctrl->sfd);
		ctrl->sfd = -1;
		return -1;
	}

	//pthread_mutex_init(&ctrl->ctrl_locker, NULL);

	_miracast_save_de_config();
	
	return 0;
}

/**
* start the rtsp service.
*/
static int _miracast_rtsp_start(struct miracast_rtsp_control *ctrl)
{
	pid_t rtsp_pid;
	int execstat;
	write_json_file_flag = 0;
	rtsp_pid = fork();
	if(rtsp_pid == -1){
		printf("[%s]: fork error\n",__FUNCTION__);
		return -1;
	}
	else if(rtsp_pid == 0){
		//execstat = execl("/mnt/udisk/miracast.app","miracast.app",NULL);
		execstat = execl("/am7x/bin/miracast.app","miracast.app",NULL);

		if(execstat == -1){
			printf("[%s]: execl error\n",__FUNCTION__);
			exit(1);
		}
	}
	else{
		ctrl->rtsp_pid = rtsp_pid;
	}

	mirartpStreamInit(RTP_PORT_DEFAULT);

#ifdef MIRACAST_HDCP_SUPPORT
	/** remove the hdcp.txt */
	unlink("/tmp/hdcp.txt");
	unlink("/tmp/hdcp_decl_support.txt");
	unlink("/tmp/hdcp_nego_over.txt");
	unlink("/tmp/hdcp_key_source.txt");

	if (pthread_create(&miracast_hdcp_thread_id, NULL, _miracast_hdcp_loop, NULL) != 0){
		printf("[%s]: HDCP loop create error\n",__FUNCTION__);
		miracast_hdcp_thread_id = -1;
	}
#endif
	return 0;
}

/**
* stop the rtsp service.
*/
static int _miracast_rtsp_stop(struct miracast_rtsp_control *ctrl)
{
	pid_t wait_pid;
	int status;
#ifdef WIFI_DIRECT_JSON
	struct p2p *p =NULL;
	p = &action_p2p;
#endif
	if(ctrl->rtsp_pid > 0){
		kill((int)ctrl->rtsp_pid,SIGKILL);
		printf("[%s]: wait the RTSP over\n",__FUNCTION__);
		//wait_pid = wait(&status);
		wait_pid = waitpid(ctrl->rtsp_pid,&status,0);
		if(wait_pid == ctrl->rtsp_pid){
			printf("[%s]: Kill the RTSP,wait_pid=%d\n",__FUNCTION__,wait_pid);
		}
		printf(">>>>> wait pid is %d,rtsp pid is %d\n",wait_pid,ctrl->rtsp_pid);
		ctrl->rtsp_pid = -1;
	}

	if(miracast_rtsp_stat == MIRACAST_RTSP_STAT_PLAYING){
		_miracast_restore_de_config(1);
#ifdef WIFI_DIRECT_JSON
		if(!write_json_file_flag){
			write_json_file_flag = 1;
			writeToJsonFile(p,"no error");
		}
#endif
	}
	/** after RTSP stop, set status immediately */
	miracast_rtsp_stat = MIRACAST_RTSP_STAT_IDLE;

#ifdef MIRACAST_HDCP_SUPPORT
	if (miracast_hdcp_thread_id != -1){
		printf("[%s]: HDCP is runnig, kill it\n",__FUNCTION__);
		pthread_cancel(miracast_hdcp_thread_id);
		pthread_join(miracast_hdcp_thread_id, NULL);
		miracast_hdcp_thread_id = -1;
	}
#endif

	return 0;
}

static int _miracast_rtsp_destroy(struct miracast_rtsp_control *ctrl)
{
	pid_t wait_pid;
	int status;
	
	/** stop the rtsp looper */
	loop_exit = 1;

	//pthread_mutex_lock(&ctrl->ctrl_locker);
	
	if(ctrl->cfd <= 0){
		pthread_cancel(miracast_thread_id);
	}
	pthread_join(miracast_thread_id, NULL);
	
	//pthread_mutex_unlock(&ctrl->ctrl_locker);
	//pthread_mutex_destroy(&ctrl->ctrl_locker);

	_miracast_rtsp_stop(ctrl);

	/** close the server socket */
	if(ctrl->sfd > 0){
		close(ctrl->sfd);
		ctrl->sfd = -1;
	}

	return 0;
}

static int miracast_engine_rtsp_init(void * handle)
{
	int ret;
	
	SWFEXT_FUNC_BEGIN(handle);

	ret = _miracast_rtsp_init(&mcc);

	Swfext_PutNumber(!!ret);

	SWFEXT_FUNC_END();	
}

static int miracast_engine_rtsp_destroy(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);

	_miracast_rtsp_destroy(&mcc);

	SWFEXT_FUNC_END();	
}

static int miracast_engine_rtsp_start(void * handle)
{
	int ret;
	
	SWFEXT_FUNC_BEGIN(handle);

	ret = _miracast_rtsp_start(&mcc);

	Swfext_PutNumber(!!ret);

	SWFEXT_FUNC_END();	
}

static int miracast_engine_rtsp_stop(void * handle)
{	
	SWFEXT_FUNC_BEGIN(handle);

	_miracast_rtsp_stop(&mcc);

	SWFEXT_FUNC_END();	
}

static int miracast_engine_rtsp_drop_mira(void * handle)
{	
	SWFEXT_FUNC_BEGIN(handle);

	/** only valid in playing status */
	if(miracast_rtsp_stat == MIRACAST_RTSP_STAT_PLAYING){
		printf("[%s]: miracast rtsp drop\n",__FUNCTION__);
		miracast_wifi_drop_flag = 1;
	}

	SWFEXT_FUNC_END();	
}

static int miracast_engine_rtsp_get_stat(void * handle)
{
	
	SWFEXT_FUNC_BEGIN(handle);

	Swfext_PutNumber(miracast_rtsp_stat);

	SWFEXT_FUNC_END();	
}

static int miracast_engine_start(void * handle)
{
	int ret;
	
	SWFEXT_FUNC_BEGIN(handle);

	ret = miracast_start();

	Swfext_PutNumber(ret);

	SWFEXT_FUNC_END();	
}

static int miracast_engine_stop(void * handle)
{
	
	SWFEXT_FUNC_BEGIN(handle);

	miracast_stop();

	SWFEXT_FUNC_END();	
}

static int miracast_engine_get_stat(void * handle)
{
	
	SWFEXT_FUNC_BEGIN(handle);

	Swfext_PutNumber(miracast_stat);

	SWFEXT_FUNC_END();	
}

/***
* Next functions for Miracast Sigma verification.
*/
#define P2P_STATUS_LOG "/tmp/p2p_status.txt"
#define MIRACAST_SIGMA_CMD "/tmp/sigmatrigger"
enum{
	SIGMA_CMD_NONE=-1,
	SIGMA_CMD_PLAY=0,
	SIGMA_CMD_PAUSE=1,
	SIGMA_CMD_RESUME=2,
	SIGMA_CMD_STOP=3,
	SIGMA_CMD_TEARDOWN=4,
};

/**
* Get the Miracast source IP address.
*/
struct dyn_lease {
	/* "nip": IP in network order */
	/* Unix time when lease expires. Kept in memory in host order.
	 * When written to file, converted to network order
	 * and adjusted (current time subtracted) */
	unsigned int expires;
	unsigned int lease_nip;
	/* We use lease_mac[6], since e.g. ARP probing uses
	 * only 6 first bytes anyway. We check received dhcp packets
	 * that their hlen == 6 and thus chaddr has only 6 significant bytes
	 * (dhcp packet has chaddr[16], not [6])
	 */
	unsigned char lease_mac[6];
	char hostname[20];
	unsigned char pad[2];
	/* total size is a multiply of 4 */
};

static int _miracast_sigma_to_upper_case(char *str)
{
	int len;
	int i;
	
	if(str == NULL){
		return -1;
	}

	len = strlen(str);
	for(i=0;i<len;i++){
		if(*(str+i) >= 'a' && *(str+i) <= 'f'){
			*(str+i) = *(str+i) - 'a' + 'A';
		}
	}

	return 0;
}

static int _miracast_sigma_get_source_ip_address()
{
	char mac[32]={0};
	unsigned char mac_tmp[32]={0};
	FILE *fp;
	char line[256]={0};
	int ip_get_ok=0;
	char ip[32]={0};
	unsigned char ip_tmp[16]={0};
	int i,cnt=0;
	int file_len;
	char nouse[12];
	int ret=-1;
	int role=0;
	char *find_role;

	/** get role */
	fp = fopen("/tmp/p2p_role.txt", "r");
	if (fp){
		while(!feof(fp)){
			memset( line, 0x00, 256 );
			fgets( line, 256, fp );
			find_role=strstr(line,"Role=");
			if(find_role){
				role = atoi(find_role+5);
				break;
			}	
		}
		fclose(fp);
	}

	if(role!=2 && role!=3){
		printf(">>>> role=%d error\n",role);
		return -1;
	}

	/** get mac */
	fp = fopen("/tmp/p2p_peer_mac.txt", "r");
	if(fp){
		fgets(mac,32,fp);
		fclose(fp);
	}
	else{
		printf(">>>> could not fine mac\n");
		return -1;
	}
	_miracast_sigma_to_upper_case(mac);
	printf(">>>> role is %d, mac is %s,mac len is %d\n",role,mac,strlen(mac));
	if(strlen(mac)>17){
		printf(">>>> mac with tail\n");
		mac[17] = 0;
		printf(">>>> trancate mac is %s\n",mac);
	}
	
	if( role == 2 ){
		/** client mode, use the /proc/net/arp to get the address */
		cnt=0;
		while(ip_get_ok==0){
			fp = fopen("/proc/net/arp","r");
			if(fp){
				int xx=0;
				while(!feof(fp)){
					memset(line, 0x00, 256);
					fgets(line, 256, fp);
					_miracast_sigma_to_upper_case(line);
					xx++;
					if(xx > 1 && strlen(line)>17){
						printf("arp:%s\n",line);
					}
					if(strstr(line,mac) != NULL){
						i=0;
						while(line[i]!=' '){
							ip[i]=line[i];
							i++;
						}
						printf(">>>> found peer ip address:%s\n",ip);
						ip_get_ok = 1;
						break;
					}	
				}	
				fclose(fp);
			}

			if(ip_get_ok == 0){
				sleep(1);
				cnt++;
				if(cnt >= 50){
					break;
				}
			}
		}
	}
	else if( role == 3 ){
		/** group owner mode */
		cnt=0;
		while(ip_get_ok==0){
			fp = fopen("/tmp/udhcpd.leases","rb");
			if(fp == NULL){
				sleep(1);
				cnt++;
				if(cnt >= 100){
					break;
				}
				else{
					continue;
				}
			}

			fseek(fp, 0, SEEK_END);
			file_len = ftell(fp);

			/**
			* begine of each least is int64 which means 8 bytes.
			*/
			if(file_len != (sizeof(struct dyn_lease)+8)){
				printf("---->lease file len error:%d,but expected:%d\n",file_len,sizeof(struct dyn_lease)+8);
				fclose(fp);
				sleep(1);
				cnt++;
				if(cnt >= 50){
					break;
				}
				else{
					continue;
				}
			}
			fseek(fp, 0, SEEK_SET);

			/**
			* no use for the first 12 bytes.
			*/
			fread(nouse, 1,12, fp);

			/**
			* ip address
			*/
			for(i=0;i<4;i++){
				fread(&ip_tmp[i], 1,1, fp);
			}
			sprintf(ip,"%d.%d.%d.%d",ip_tmp[0],ip_tmp[1],ip_tmp[2],ip_tmp[3]);
			
			/**
			* mac address
			*/
			for(i=0;i<6;i++){
				fread(&mac_tmp[i], 1,1, fp);
			}
			sprintf(line,"%02x:%02x:%02x:%02x:%02x:%02x",mac_tmp[0],mac_tmp[1],mac_tmp[2],mac_tmp[3],mac_tmp[4],mac_tmp[5]);
			fclose(fp);
			_miracast_sigma_to_upper_case(line);
			printf(">>> host mode:%s,%s,mac=%s,strlen(mac)=%d,strlen(line)=%d\n",ip,line,mac,strlen(mac),strlen(line));
			if(strncmp(line,mac,17)==0){
				ip_get_ok = 1;
				break;
			}
		}
	}

	if(ip_get_ok){
		printf(">>> %s:peer ip is %s\n",__FUNCTION__,ip);
		fp = fopen("/tmp/sourceip.log","w");
		if(fp){
			fputs(ip,fp);
			fclose(fp);
		}
		ret = 0;
	}

	return ret;
}

#include "am7x_dac.h"

static void _miracast_sigma_vol_ctrl(int cmd)
{
	int vol;
	int fd;
	int err;

	fd = open("/dev/DAC",2);

	if (fd < 0) {
		printf("open dac error\n");
		return;	
	}

	if(cmd==1){
		/* for pause*/
		err = ioctl(fd,DACIO_SET_PAUSE,NULL);
		if(err < 0){
			printf("set audio dac pause error\n");
			close(fd);
			return ;
		}
	}
	else if(cmd==0){
		/* for play*/
		err = ioctl(fd,DACIO_SET_CONTINUE,NULL);
		if(err < 0){
			printf("set audio dac play error\n");
			close(fd);
			return ;
		}
	}

	close(fd);
}

static void *_miracast_sigma_loop(void *arg)
{	
	int size;
	socklen_t csize;
	struct sockaddr_un un,un1;
	fd_set rfd_set;
	int cmd;
	struct timeval tv;
	int sret;
	unsigned char *heap1=NULL;
	unsigned int bus;
	unsigned int hsize;
	struct miracast_rtsp_control *ctrl = (struct miracast_rtsp_control *)arg;
	FILE *fp;
	char parse[64]={0};
	int p2p_connect_success=0;
	pid_t mira_app_id;
	pid_t wait_pid;
	int status;
	int execstat;
	int sigma_cmd = SIGMA_CMD_NONE,sigma_cmd_prev=SIGMA_CMD_NONE;
	int play_started = 0;

	/*************************************************************
	* Get the P2P connection status.
	* 1. get p2p status from /tmp/p2p_status.txt with success for p2p ok and
	*    others will be ignored.
	* 2. get p2p role form /tmp/p2p_role.txt with 2-->client 3-->go
	**************************************************************/
__stm_p2p:

	while(p2p_connect_success==0){
		fp = fopen(P2P_STATUS_LOG,"r");
		if(fp){
			while(!feof(fp)){
				memset(parse,0x00,64);
				fgets(parse,64,fp);
				if(strstr(parse,"success")){
					printf(">>> p2p connect success\n");
					p2p_connect_success = 1;
					break;
				}
			}
			fclose(fp);
		}
		
		if(p2p_connect_success){
			break;
		}
		sleep(1);
	}
	p2p_connect_success = 0;
	sprintf(parse,"rm -rf %s",P2P_STATUS_LOG);
	system(parse);
	
	/**
	* connect success. Obtain the IP address and source port.
	*/
	//miracast_exec_shell_script("/am7x/case/scripts/miracast_sigma_source_ip.sh",NULL);
	if(_miracast_sigma_get_source_ip_address()!=0){
		printf(">>>> get source ip error\n");
		goto __stm_p2p;
	}

	play_started = 0;
	/*************************************************************
	* Start the RTSP engine.
	**************************************************************/
	mira_app_id = fork();
	if(mira_app_id == -1){
		printf("[%s]: fork error\n",__FUNCTION__);
		goto _stm_loop_out;
	}
	else if(mira_app_id == 0){
		
		execstat = execl("/am7x/bin/miracast.app","miracast.app",NULL);
		if(execstat == -1){
			printf("[%s]: execl error\n",__FUNCTION__);
			exit(1);
		}
	}
	else{
		
__stm_accept:
		csize = sizeof(un1);
		ctrl->cfd = accept(ctrl->sfd, (struct sockaddr *)&un1, &csize);
		if(ctrl->cfd < 0){
			int errsv = errno;
			if(errsv == EINTR){
				//Restart if interrupted by handler
				goto __stm_accept;	
			}
			else{
				printf("[%s]: accept error %d\n",__FUNCTION__,errsv);
				goto __stm_accept;
			}
		}
		else{
			/** 
			* we get the connection from miracast.app
			* and will begin to process the messages.
			*/
			printf("[%s]: accept ok, %d\n",__FUNCTION__,ctrl->cfd);

			while(loop_exit == 0){
				
				FD_ZERO(&rfd_set);
				FD_SET(ctrl->cfd, &rfd_set);

				tv.tv_sec = 1;
				tv.tv_usec = 0;

				/** timeout each second for get the chance to check loop_exit */
				sret = select(ctrl->cfd+1,&rfd_set,NULL,NULL,&tv);

				if(sret>0){
					
					if(FD_ISSET(ctrl->cfd,&rfd_set)){
						miracast_cmd_t cmd;
						int cmdsize;
						cmdsize = read(ctrl->cfd,&cmd,sizeof(cmd));
						if(cmdsize == sizeof(cmd)){
							switch(cmd.cmd){
								case MIRACAST_CMD_PLAY:
								{	
									printf("[%s]: receive MIRACAST_CMD_PLAY command,port:%d\n",__FUNCTION__,cmd.param);
									//_miracast_sigma_vol_ctrl(0);
									if(play_started){
										_miracast_sigma_vol_ctrl(0);
										printf("[%s]: play started, ignore\n",__FUNCTION__);
										break;
									}
									play_started = 1;
									/** sleep SWF */
									if(SWF_GetState(SWF_GetActiveInst()) & SWF_STATE_ACTIVE)
									{
										SWF_SetState(SWF_GetActiveInst(), SWF_STATE_SLEEP);
									}

									if(miracast_get_player_heap(&heap1,&bus,&hsize) == 0){
										printf("[%s]: get heap ok,0x%x,0x%x,0x%x\n",__FUNCTION__,heap1,bus,hsize);
										mirartpStreamStart(cmd.param,heap1,bus,hsize,IMAGE_WIDTH_E,IMAGE_HEIGHT_E);
									}
									else{
										printf("[%s]: get heap error\n",__FUNCTION__);
										if((SWF_GetState(SWF_GetActiveInst()) & SWF_STATE_ACTIVE) == 0){
											SWF_SetState(SWF_GetActiveInst(), SWF_STATE_ACTIVE);
										}
									}
								}
									
								break;

								case MIRACAST_CMD_PAUSE:
									_miracast_sigma_vol_ctrl(1);
									printf("[%s]: receive MIRACAST_CMD_PAUSE command\n",__FUNCTION__);
								break;

								case MIRACAST_CMD_STOP:
									printf("[%s]: receive MIRACAST_CMD_STOP command\n",__FUNCTION__);

									/** stop the streaming */
									mirartpStreamStop();

									/** stop the RTSP process */
									if(mira_app_id > 0 && miracast_rtsp_alive()){
										kill((int)mira_app_id,SIGKILL);
										wait_pid = wait(&status);
										if(wait_pid == mira_app_id){
											printf("[%s]: Kill the RTSP\n",__FUNCTION__);
										}
										mira_app_id = 0;
									}

									/** close the RTSP communication channel */
									if(ctrl->cfd > 0){
										close(ctrl->cfd);
										ctrl->cfd = -1;
									}

									printf(">>>>>>> %d,%s\n",__LINE__,__FILE__);

									/** release the heap memory */
									if(heap1){
										SWF_Free((void *)heap1);
										heap1 = NULL;
									}

									printf(">>>>>>> %d,%s\n",__LINE__,__FILE__);

									/** wake up the FUI process again */
									if((SWF_GetState(SWF_GetActiveInst()) & SWF_STATE_ACTIVE) == 0)
									{
										SWF_SetState(SWF_GetActiveInst(), SWF_STATE_ACTIVE);
									}

									play_started = 0;

									printf(">>>>>>> %d,%s\n",__LINE__,__FILE__);
									
									if(loop_exit){
										/** someone asked us to quit */
										printf(">>>>>>> %d,%s\n",__LINE__,__FILE__);
										goto _stm_loop_out;
									}
									else{
										/** RTSP out and we contiue to do the next Miracast process */
										printf(">>>>>>> %d,%s\n",__LINE__,__FILE__);
										goto __stm_p2p;
									}
								break;

								default:
								break;
							}
						}
					}
					
				}
				else if(sret == 0){
					/** check sigma command */
					fp = fopen(MIRACAST_SIGMA_CMD,"r");
					if(fp){
						while(!feof(fp)){
							memset(parse,0x00,64);
							fgets(parse,64,fp);
							if(strncmp(parse, "PLAY",4)==0){
								sigma_cmd = SIGMA_CMD_PLAY;
								break;
							}
							else if(strncmp(parse, "PAUSE",5)==0){
								sigma_cmd = SIGMA_CMD_PAUSE;
								break;
							}
							else if(strncmp(parse, "TEARDOWN",8)==0){
								sigma_cmd = SIGMA_CMD_TEARDOWN;
								break;
							}
							else{
								sigma_cmd = SIGMA_CMD_NONE;
							}
						}

						if((sigma_cmd != sigma_cmd_prev) && (sigma_cmd != SIGMA_CMD_NONE)){
							/** new sigma command */
							printf(">>>>>> send commad %d to RTSP engine\n",sigma_cmd);
							if(ctrl->cfd > 0){
								int wret=0;
								wret = write(ctrl->cfd,&sigma_cmd,sizeof(int));
								if(sigma_cmd == SIGMA_CMD_PLAY){
									_miracast_sigma_vol_ctrl(0);
								}
								else if(sigma_cmd == SIGMA_CMD_PAUSE){
									_miracast_sigma_vol_ctrl(1);
								}
							}
						}
						sigma_cmd_prev = sigma_cmd;
						fclose(fp);
					}
				}

			}
			
			/** loop exit, we still need to release the resources */
			printf(">>>>>>> %d,%s\n",__LINE__,__FILE__);
			mirartpStreamStop();
			printf(">>>>>>> %d,%s\n",__LINE__,__FILE__);
			
			if(mira_app_id > 0){
				kill((int)mira_app_id,SIGKILL);
				wait_pid = wait(&status);
				if(wait_pid == mira_app_id){
					printf("[%s]: Kill the RTSP\n",__FUNCTION__);
				}
				mira_app_id = 0;
			}
			printf(">>>>>>> %d,%s\n",__LINE__,__FILE__);

			if(heap1){
				SWF_Free((void *)heap1);
				heap1 = NULL;
			}
			printf(">>>>>>> %d,%s\n",__LINE__,__FILE__);
			if((SWF_GetState(SWF_GetActiveInst()) & SWF_STATE_ACTIVE) == 0)
			{
				SWF_SetState(SWF_GetActiveInst(), SWF_STATE_ACTIVE);
			}
			printf(">>>>>>> %d,%s\n",__LINE__,__FILE__);
		}
		

	}
	
_stm_loop_out:

	if(ctrl->cfd > 0){
		close(ctrl->cfd);
		ctrl->cfd = -1;
	}
	
	pthread_exit(NULL);

	return NULL;
}

static int _miracast_sigma_init(struct miracast_rtsp_control *ctrl)
{
	struct sockaddr_un un;
	int size;
	socklen_t csize;

	if(ctrl == NULL){
		return -1;
	}

	ctrl->sfd = -1;
	ctrl->cfd = -1;
	ctrl->rtsp_pid = -1;
	
	un.sun_family = AF_UNIX;
    strcpy(un.sun_path, "/tmp/miracast_server");
	if ((ctrl->sfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0){
		printf("[%s]: socket create error\n",__FUNCTION__);
		return -1;
	}

	/** before bind, must make sure the path file not exist */
	unlink(un.sun_path);
	
	size = offsetof(struct sockaddr_un, sun_path) + strlen(un.sun_path);
	if (bind(ctrl->sfd, (struct sockaddr *)&un, size) < 0){
		printf("[%s]: bind error\n",__FUNCTION__);
		close(ctrl->sfd);
		ctrl->sfd = -1;
		return -1;
	}

	if (listen(ctrl->sfd, 5) < 0){
		printf("[%s]: listen error\n",__FUNCTION__);
		close(ctrl->sfd);
		ctrl->sfd = -1;
		return -1;
	}

	if (pthread_create(&miracast_thread_id, NULL, _miracast_sigma_loop, (void *)ctrl) != 0){
		printf("[%s]: rtsp loop create error\n",__FUNCTION__);
		close(ctrl->sfd);
		ctrl->sfd = -1;
		return -1;
	}
	
	return 0;
}

static int _miracast_sigma_destroy(struct miracast_rtsp_control *ctrl)
{
	_miracast_rtsp_destroy(ctrl);

	return 0;
}


static int miracast_engine_sigma_start(void * handle)
{
	int ret;
	
	SWFEXT_FUNC_BEGIN(handle);

	ret = _miracast_sigma_init(&mcc);

	Swfext_PutNumber(!!ret);

	SWFEXT_FUNC_END();	
}

static int miracast_engine_sigma_destory(void * handle)
{
	
	SWFEXT_FUNC_BEGIN(handle);

	_miracast_sigma_destroy(&mcc);

	SWFEXT_FUNC_END();	
}



int swfext_miracast_engine_register(void)
{
	SWFEXT_REGISTER("miracastEnigneStart", miracast_engine_start);
	SWFEXT_REGISTER("miracastEnigneStop", miracast_engine_stop);
	SWFEXT_REGISTER("miracastEnigneGetStatus", miracast_engine_get_stat);
	SWFEXT_REGISTER("miracast_engine_rtsp_init_as", miracast_engine_rtsp_init);
	SWFEXT_REGISTER("miracast_engine_rtsp_destroy_as", miracast_engine_rtsp_destroy);
	SWFEXT_REGISTER("miracast_engine_rtsp_start_as", miracast_engine_rtsp_start);
	SWFEXT_REGISTER("miracast_engine_rtsp_stop_as", miracast_engine_rtsp_stop);	
	SWFEXT_REGISTER("miracast_engine_rtsp_get_stat_as", miracast_engine_rtsp_get_stat);
	SWFEXT_REGISTER("miracast_engine_rtsp_drop_mira_as", miracast_engine_rtsp_drop_mira);
	SWFEXT_REGISTER("miracast_engine_sigma_start_as", miracast_engine_sigma_start);
	SWFEXT_REGISTER("miracast_engine_sigma_destory_as", miracast_engine_sigma_destory);
	return 0;
}


#endif 

