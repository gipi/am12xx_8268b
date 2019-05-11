#include <time.h>
#include <string.h>
#include <errno.h>
#include <syscall.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/time.h>

#include "linux-alien.h"

#include "fileviewer.h"


int gettid()
{
    return syscall(SYS_gettid);
}

void OSSleep(unsigned long time_ms)
{
    struct timespec req;
    struct timespec rem;
    int ret;

    req.tv_sec = time_ms/1000;
    req.tv_nsec = (time_ms%1000)*1000*1000;

    for(;;)
    {
        ret = nanosleep(&req, &rem);
        if(ret == -1)
        {
            int errsv = errno;
            if(errsv == EINTR)
            {
                req = rem;
                continue;   //Restart if interrupted by handler
            }
            else
            {
                printf("nanosleep: errno:%d, %s\n", errsv, strerror(errsv));
            }
        }
        break;
    }
}

int OSGetTime(unsigned long *psec, unsigned long *pusec)
{
    if(psec || pusec)
    {
        struct timeval tv;
        int ret;
        ret = gettimeofday(&tv, NULL);
        if(ret)
        {
            perror("gettimeofday");
            return -1;
        }
        if(psec)
            *psec = tv.tv_sec;
        if(pusec)
            *pusec = tv.tv_usec;
    }
    return 0;
}


void *timer_init()
{
    TIMER_CONTEXT *ptc = NULL;

    ptc = (TIMER_CONTEXT*)calloc(sizeof(TIMER_CONTEXT), 1);
    if(!ptc)
    {
        printf("timer_init(), calloc error\n");
        goto fail;
    }

    if(pthread_mutex_init(&ptc->mutex, NULL) != 0)
    {
        printf("pthread_mutex_init() failed\n");
        goto fail;
    }

    return (void*)ptc;

fail:
    if(ptc)
        free(ptc);
    return NULL;
}

int timer_add(void *timer_handle,
                     int ms,
                     void (*proc)(void *arg, int timer_id),
                     void *arg)
{
    TIMER_CONTEXT *ptc = (TIMER_CONTEXT*)timer_handle;
    TIMER_EVENT *pte = NULL;
    int timer_id = 0;

    pte = (TIMER_EVENT*)calloc(sizeof(TIMER_EVENT), 1);
    if(!pte)
        goto end;
    pte->ms = ms;
    pte->proc = proc;
    pte->arg = arg;
    OSGetTime(&pte->time_s, &pte->time_us);

    pthread_mutex_lock(&ptc->mutex);

    timer_id = ptc->timer_id;
    ptc->timer_id++;
    pte->timer_id = timer_id;

    if(!ptc->pevents)
        ptc->pevents = pte;
    else
    {
        TIMER_EVENT *p = ptc->pevents;
        while(p->next)
            p = p->next;
        p->next = pte;
    }
    pthread_mutex_unlock(&ptc->mutex);
end:
    return timer_id;
}

int timer_remove(void *timer_handle,
                        int timer_id)
{
    TIMER_CONTEXT *ptc = (TIMER_CONTEXT*)timer_handle;
    TIMER_EVENT *curr;
    TIMER_EVENT *prev;

    pthread_mutex_lock(&ptc->mutex);

    prev = NULL;
    curr = ptc->pevents;
    while(curr)
    {
        if(curr->timer_id > timer_id)
        {
            break;
        }
        else if(curr->timer_id == timer_id)
        {
            if(!prev)
                ptc->pevents = curr->next;
            else
                prev->next = curr->next;
            free(curr);
        }
        prev = curr;
        curr = curr->next;
    }

    pthread_mutex_unlock(&ptc->mutex);

    return 0;
}

void timer_release(void *handle)
{
    if(handle)
    {
        TIMER_CONTEXT *s = (TIMER_CONTEXT*)handle;
        TIMER_EVENT *pte = s->pevents;
        while(pte)
        {
            TIMER_EVENT *next = pte->next;
            free(pte);
            pte = next;
        }
        free(s);
    }
}

void userRequestIdleCallback(void *data, int timer_id)
{
    AlienUserRequest_Request *alienRequest = (AlienUserRequest_Request*)data;

    PicselUserRequest_notify(alienRequest->ac->picselContext, alienRequest->picselRequest);
    free(alienRequest);
}

void requestPasswordCallback(void *data, int timer_id)
{
    char *password = NULL;
    AlienUserRequest_Request *alienRequest;
    PicselUserRequest_Request *picselRequest;

//  PRINTF("call %s()\n", __FUNCTION__);

    alienRequest = (AlienUserRequest_Request*)data;
    picselRequest = alienRequest->picselRequest;

    password = strdup("123");
//printf("================> the password is %s\n",password);
    if (password != NULL && strlen(password) > 0)
    {
        picselRequest->requestData->password.password = password;
        picselRequest->result = PicselUserRequest_Result_Accepted;
    }
    else
    {
        picselRequest->requestData->password.password = NULL;
        picselRequest->result = PicselUserRequest_Result_Rejected;
    }

    PicselUserRequest_notify(alienRequest->ac->picselContext, alienRequest->picselRequest);

    if(password)
        free(password);
    free(alienRequest);
}


