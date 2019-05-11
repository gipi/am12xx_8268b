#include "flickr_main.h"
#include "time.h"
#include "flickr_req_ioctl.h"
flickr_ioctrl_t flickr_cmd_array[FLICKR_IOARRAY_LEN];

/**
@brief initialize tthe request queue
@param[in] gdata	: initialize by calling the pacasa_init_gdata()
**/
int flickr_req_init_queue(flickr_task_info_t *task_info)
{
	if(task_info==NULL)
		return -1;
	task_info->req_start_idx = 0;
	memset(flickr_cmd_array,0,sizeof(flickr_ioctrl_t)*FLICKR_IOARRAY_LEN);
	return 0;
}

/**
@brief enqueue the request
@param[in] task_info	: pointer to the task info
@param[in] req	: the pointer to the picasa_ioctrl_t where the request stored
@return
	- -1	: failed
	- others	: the idx of the queue where the request insert
**/
int flickr_req_enqueue(flickr_task_info_t *task_info,flickr_ioctrl_t * req){
	int i=0,rtn,tmp_idx;
	if(task_info==NULL)
		return -1;
	flickr_sem_wait(&task_info->syn_lock.semi_req);
	for(i=0;i<FLICKR_IOARRAY_LEN;i++){
		/**search the positon to insert the request,**/
		tmp_idx = (task_info->req_start_idx+i)%FLICKR_IOARRAY_LEN;
		if(flickr_cmd_array[tmp_idx].active==FLICKR_REQ_INVALID){
				break;
		}
	}
	if(i>=FLICKR_IOARRAY_LEN){
		flickr_err("Sorry,the Queue is full");
		rtn = -1;
	}
	else{
		memcpy(flickr_cmd_array+tmp_idx,req,sizeof(flickr_ioctrl_t));
		flickr_cmd_array[tmp_idx].active = FLICKR_REQ_ENQUEUE;
		flickr_info("REQ: ENQUEUE idx===%d\n",tmp_idx);
		task_info->req_start_idx = tmp_idx;
		rtn  = tmp_idx;
	}
	flickr_sem_post(&task_info->syn_lock.semi_req);
	return rtn;
}

/**
@brief dequeue the request
@param[in] task_info	: pointer to the task info
@param[out] req	: the pointer to the flickr_ioctrl_t where the request will be stored
@return
	- -1	: failed
	- others	: the idx of the queue where the request insert
**/
int flickr_req_dequeue(flickr_task_info_t *task_info,flickr_ioctrl_t * req){
	int i=0,rtn;
	if(task_info==NULL)
		return -1;
	memset(req,0,sizeof(flickr_ioctrl_t));
	flickr_sem_wait(&task_info->syn_lock.semi_req);
	for(i=0;i<FLICKR_IOARRAY_LEN;i++){
		if(flickr_cmd_array[i].active==FLICKR_REQ_ENQUEUE){
			break;
		}
	}
	if(i>=FLICKR_IOARRAY_LEN){
		rtn = -1;
	}
	else{
		memcpy(req,flickr_cmd_array+i,sizeof(flickr_ioctrl_t));
		flickr_cmd_array[i].active = FLICKR_REQ_DEQUEUE;
		rtn  = i;
	}
	flickr_sem_post(&task_info->syn_lock.semi_req);
	return rtn;
}

/**
@brief mark the request that had been done
@param[in] task_info	: pointer to the task info
@param[in] req_idx	: which request had been done
@return
	- -1	: failed
	- 0 	: success
**/
int flickr_req_done(flickr_task_info_t *task_info,int req_idx)
{
	int rtn=0;
	if(task_info==NULL)
		return -1;
	flickr_sem_wait(&task_info->syn_lock.semi_req);
	if(flickr_cmd_array[req_idx].active==FLICKR_REQ_DEQUEUE){
		flickr_info("REQ DONE idx=%d\n",req_idx);
		flickr_cmd_array[req_idx].active = FLICKR_REQ_DONE; 
	}
	else
		rtn = -1;
	flickr_sem_post(&task_info->syn_lock.semi_req);
	return rtn;
}


/**
@brief query  the request whether it had been done
@param[in] task_info	: pointer to the task info
@param[in] cmd	: see flickr_ioctl_cmd_e
@param[in] para	: if cmd ==FLICKR_CMD_DOWNLOADPHOTO
				it is the pointer to the flickr_download_info_t, call the flickr_init_download_info() to get it
@return
	- -2	: param is err
	- -1 : the request had not been done
	- others 	: the request had been done
**/
int flickr_req_query(flickr_task_info_t *task_info,flickr_ioctl_cmd_e cmd,void *para)
{
	int i=0,rtn=-1;
	if(task_info==NULL || para==NULL){
		flickr_err("Carzy ParaError:task_info==0x%x,para=0x%x",task_info,para);
		return -2;
	}
	flickr_sem_wait(&task_info->syn_lock.semi_req);
	for(i=0;i<FLICKR_IOARRAY_LEN;i++){
		if(flickr_cmd_array[i].active==FLICKR_REQ_DONE){
			flickr_info("iocmd=%d,iopara=0x%x,cmd=%d,para=0x%x\n",flickr_cmd_array[i].iocmd,flickr_cmd_array[i].para,cmd,para);
			if(flickr_cmd_array[i].iocmd == cmd && flickr_cmd_array[i].para == para)
				break;
		}
	}
	if(i>=FLICKR_IOARRAY_LEN){
		rtn = -1;
	}
	else{
		flickr_info("REQ Query idx=%d\n",i);
		flickr_cmd_array[i].active = FLICKR_REQ_INVALID; 
		rtn  = i;
	}
	flickr_sem_post(&task_info->syn_lock.semi_req);
	return rtn;
}

