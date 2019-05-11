#include "webalbum_api.h"
#include "act_picasa_ioctl.h"
#include "time.h"

picasa_ioctrl_t picasa_cmd_array[PICASA_IOARRAY_LEN];

/**
@brief initialize tthe request queue
@param[in] gdata	: initialize by calling the pacasa_init_gdata()
**/
int picasa_req_init_queue(picasaweb_gdata_t *gdata)
{
	if(gdata==NULL)
		return -1;
	gdata->req_start_idx = 0;
	memset(picasa_cmd_array,0,sizeof(picasa_ioctrl_t)*PICASA_IOARRAY_LEN);
	return 0;
}

/**
@brief enqueue the request
@param[in] gdata	: initialize by calling the pacasa_init_gdata()
@param[in] req	: the pointer to the picasa_ioctrl_t where the request stored
@return
	- -1	: failed
	- others	: the idx of the queue where the request insert
**/
int picasa_req_enqueue(picasaweb_gdata_t *gdata,picasa_ioctrl_t * req){
	int i=0,rtn,tmp_idx;
	if(gdata==NULL)
		return -1;
	picasa_sem_wait(&gdata->syn_lock.semi_req);
	for(i=0;i<PICASA_IOARRAY_LEN;i++){
		/**search the positon to insert the request,**/
		tmp_idx = (gdata->req_start_idx+i)%PICASA_IOARRAY_LEN;
		if(picasa_cmd_array[tmp_idx].active==PICASA_REQ_INVALID){
				break;
		}
	}
	if(i>=PICASA_IOARRAY_LEN){
		picasa_err("Sorry,the Queue is full");
		rtn = -1;
	}
	else{
		memcpy(picasa_cmd_array+tmp_idx,req,sizeof(picasa_ioctrl_t));
		picasa_cmd_array[tmp_idx].active = PICASA_REQ_ENQUEUE;
		picasa_info("REQ: ENQUEUE idx===%d\n",tmp_idx);
		gdata->req_start_idx = tmp_idx;
		rtn  = tmp_idx;
	}
	picasa_sem_post(&gdata->syn_lock.semi_req);
	return rtn;
}

/**
@brief dequeue the request
@param[in] gdata	: initialize by calling the pacasa_init_gdata()
@param[out] req	: the pointer to the picasa_ioctrl_t where the request will be stored
@return
	- -1	: failed
	- others	: the idx of the queue where the request insert
**/
int picasa_req_dequeue(picasaweb_gdata_t *gdata,picasa_ioctrl_t * req){
	int i=0,rtn;
	if(gdata==NULL)
		return -1;
	memset(req,0,sizeof(picasa_ioctrl_t));
	picasa_sem_wait(&gdata->syn_lock.semi_req);
	for(i=0;i<PICASA_IOARRAY_LEN;i++){
		if(picasa_cmd_array[i].active==PICASA_REQ_ENQUEUE){
			break;
		}
	}
	if(i>=PICASA_IOARRAY_LEN){
		rtn = -1;
	}
	else{
		memcpy(req,picasa_cmd_array+i,sizeof(picasa_ioctrl_t));
		picasa_cmd_array[i].active = PICASA_REQ_DEQUEUE;
		rtn  = i;
	}
	picasa_sem_post(&gdata->syn_lock.semi_req);
	return rtn;
}

/**
@brief mark the request that had been done
@param[in] gdata	: initialize by calling the pacasa_init_gdata()
@param[in] req_idx	: which request had been done
@return
	- -1	: failed
	- 0 	: success
**/
int picasa_req_done(picasaweb_gdata_t *gdata,int req_idx)
{
	int rtn=0;
	if(gdata==NULL)
		return -1;
	picasa_sem_wait(&gdata->syn_lock.semi_req);
	if(picasa_cmd_array[req_idx].active==PICASA_REQ_DEQUEUE){
		picasa_info("REQ DONE idx=%d\n",req_idx);
		picasa_cmd_array[req_idx].active = PICASA_REQ_DONE; 
	}
	else
		rtn = -1;
	picasa_sem_post(&gdata->syn_lock.semi_req);
	return rtn;
}


/**
@brief query  the request whether it had been done
@param[in] gdata	: initialize by calling the pacasa_init_gdata()
@param[in] cmd	: see picasa_ioctl_cmd_e
@param[in] para	: if cmd ==PICASA_CMD_DOWNLOADPHOTO
				it is the pointer to the photo_down_info_t, call the picasa_init_download_info() to get it
@return
	- -2	: param is err
	- -1 : the request had not been done
	- others 	: the request had been done
**/
int picasa_req_query(picasaweb_gdata_t *gdata,picasa_ioctl_cmd_e cmd,void *para)
{
	int i=0,rtn=-1;
	if(gdata==NULL || para==NULL)
		return -2;
	picasa_sem_wait(&gdata->syn_lock.semi_req);
	for(i=0;i<PICASA_IOARRAY_LEN;i++){
		if(picasa_cmd_array[i].active==PICASA_REQ_DONE){
			picasa_info("iocmd=%d,iopara=0x%x,cmd=%d,para=0x%x\n",picasa_cmd_array[i].iocmd,picasa_cmd_array[i].para,cmd,para);
			if(picasa_cmd_array[i].iocmd == cmd && picasa_cmd_array[i].para == para)
				break;
		}
	}
	if(i>=PICASA_IOARRAY_LEN){
		rtn = -1;
	}
	else{
		picasa_info("REQ Query idx=%d\n",i);
		picasa_cmd_array[i].active = PICASA_REQ_INVALID; 
		rtn  = i;
	}
	picasa_sem_post(&gdata->syn_lock.semi_req);
	return rtn;
}

