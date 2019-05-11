#include "webalbum_api.h"
#include "facebook_ioctl.h"
#include "time.h"

facebook_ioctrl facebook_cmd_array[FACEBOOK_IOARRAY_LEN];

int facebook_req_init_queue(facebook_data *f_data)
{
	if(f_data==NULL)
		return -1;
	f_data->req_start_idx = 0;
	memset(facebook_cmd_array,0,sizeof(facebook_ioctrl)*FACEBOOK_IOARRAY_LEN);
	return 0;
}

int facebook_req_enqueue(facebook_data *f_data,facebook_ioctrl * req){
	int i=0,rtn,tmp_idx;
	if(f_data==NULL)
		return -1;
	facebook_sem_wait(&f_data->syn_lock.semi_req);
	for(i=0;i<FACEBOOK_IOARRAY_LEN;i++){
		/**search the positon to insert the request,**/
		tmp_idx = (f_data->req_start_idx+i)%FACEBOOK_IOARRAY_LEN;
		if(facebook_cmd_array[tmp_idx].active==FACEBOOK_REQ_INVALID){
				break;
		}
	}
	if(i>=FACEBOOK_IOARRAY_LEN){
		facebook_dbg("Sorry,the Queue is full");
		rtn = -1;
	}
	else{
		memcpy(facebook_cmd_array+tmp_idx,req,sizeof(facebook_ioctrl));
		facebook_cmd_array[tmp_idx].active = FACEBOOK_REQ_ENQUEUE;
		facebook_dbg("REQ: ENQUEUE idx===%d\n",tmp_idx);
		f_data->req_start_idx = tmp_idx;
		rtn  = tmp_idx;
	}
	facebook_sem_post(&f_data->syn_lock.semi_req);
	return rtn;
}

int facebook_req_dequeue(facebook_data *f_data,facebook_ioctrl * req){
	int i=0,rtn;
	if(f_data==NULL)
		return -1;
	memset(req,0,sizeof(facebook_ioctrl));
	facebook_sem_wait(&f_data->syn_lock.semi_req);
	for(i=0;i<FACEBOOK_IOARRAY_LEN;i++){
		if(facebook_cmd_array[i].active==FACEBOOK_REQ_ENQUEUE){
			break;
		}
	}
	if(i>=FACEBOOK_IOARRAY_LEN){
		rtn = -1;
	}
	else{
		memcpy(req,facebook_cmd_array+i,sizeof(facebook_ioctrl));
		facebook_cmd_array[i].active = FACEBOOK_REQ_DEQUEUE;
		rtn  = i;
	}
	facebook_sem_post(&f_data->syn_lock.semi_req);
	return rtn;
}

int facebook_req_done(facebook_data *f_data,int req_idx)
{
	int rtn=0;
	if(f_data==NULL)
		return -1;
	facebook_sem_wait(&f_data->syn_lock.semi_req);
	if(facebook_cmd_array[req_idx].active==FACEBOOK_REQ_DEQUEUE){
		facebook_dbg("REQ DONE idx=%d\n",req_idx);
		facebook_cmd_array[req_idx].active = FACEBOOK_REQ_DONE; 
	}
	else
		rtn = -1;
	facebook_sem_post(&f_data->syn_lock.semi_req);
	return rtn;
}

int facebook_req_query(facebook_data *f_data,facebook_ioctl_cmd cmd,void *para)
{
	int i=0,rtn=-1;
	if(f_data==NULL || para==NULL)
		return -2;
	facebook_sem_wait(&f_data->syn_lock.semi_req);
	for(i=0;i<FACEBOOK_IOARRAY_LEN;i++){
		if(facebook_cmd_array[i].active==FACEBOOK_REQ_DONE ){
			facebook_dbg("iocmd=%d,iopara=0x%x,cmd=%d,para=0x%x\n",facebook_cmd_array[i].iocmd,facebook_cmd_array[i].para,cmd,para);
			if(facebook_cmd_array[i].iocmd == cmd && facebook_cmd_array[i].para == para)
				break;
		}
	}
	if(i>=FACEBOOK_IOARRAY_LEN){
		rtn = -1;
	}
	else{
		facebook_dbg("REQ Query idx=%d\n",i);
		facebook_cmd_array[i].active = FACEBOOK_REQ_INVALID; 
		rtn  = i;
	}
	facebook_sem_post(&f_data->syn_lock.semi_req);
	return rtn;
}

