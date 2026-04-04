#ifndef _CAN_THREAD_H_
#define _CAN_THREAD_H_

#include <pthread.h>
#include "common.h"

#define COMMU_FLAG_NODELAY  (1 << 0)
typedef int (*usr_call2)(void *arg, void *arg2, char *buf, int len); 

#define CAN_BUF_SIZE    (16)

typedef struct 
{
    int run;
    int flag;
    pthread_t t_id_tx;
    pthread_t t_id_rx;
    mHandle can_tx;
    mHandle can_rx;
    usr_call2 func;
    void *arg;
    void *arg2;
}can_commu_t;

int can_commu_del(mHandle _handle);
int can_commu_send(mHandle _handle, char *buf, int len);
mHandle can_commu_init(char *dev_rc, char *dev_sd, uint32_t id_rc, uint32_t mask_rc, usr_call2 func, void *arg, void *arg2, int flag);

#endif