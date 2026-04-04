#include <pthread.h>
#include <string.h>
#include <malloc.h>
#include "can_thread.h"
#include "can_create_socket.h"
#include "common.h"

void* can_rx_func(void *_arg)
{
    int len = 0;
    char *buf = NULL;
    can_commu_t *handle = (can_commu_t *)_arg;

    buf = malloc(CAN_BUF_SIZE);
    if (!buf)
    {
        printf("malloc error\n");
        return NULL;
    }
    memset(buf, 0, CAN_BUF_SIZE);
    
    //FIXME 阻塞接受这里不会退出
    while (handle->run)
    {        
        len = can_interface_recv(handle->can_rx, buf, CAN_BUF_SIZE, handle->flag & COMMU_FLAG_NODELAY);
        if(len <= 0) //FIXME 无法识别接收超时和接收失败
        {
            continue;
        }

        handle->func(handle->arg, handle->arg2, buf, len);
        memset(buf, 0, CAN_BUF_SIZE);
    }

    free(buf);

    handle->run = 2;

    return NULL;
}

int can_commu_del(mHandle _handle)
{
    can_commu_t *handle = (can_commu_t *)_handle;

    handle->run = 0;

    can_delet_interface(handle->can_tx);
    can_delet_interface(handle->can_rx);

    free(handle);

    return 0;
}

int can_commu_send(mHandle _handle, char *buf, int len)
{
    can_commu_t *handle = (can_commu_t *)_handle;

    //return can_interface_send(handle->can_tx, buf, len);
    return can_interface_send(handle->can_rx, buf, len);
}

mHandle can_commu_init(char *dev_rc, char *dev_sd, uint32_t id_rc, uint32_t mask_rc, usr_call2 func, void *arg, void *arg2,int flag)
{
    can_commu_t *handle = (can_commu_t *)malloc(sizeof(can_commu_t));
    if(!handle)
    {
        printf("malloc error\n");
        return NULL;
    }

    memset(handle, 0, sizeof(can_commu_t));
    handle->func = func;
    handle->arg = arg;
    handle->arg2 = arg2;
    handle->flag = flag;
    handle->run = 1;

    // handle->can_tx = can_creat_interface(CAN_TX, dev_sd, 0, 0);
    // if(!handle->can_tx)
    // {
    //     printf("udp interface tx create failed\n");
    //     return NULL;
    // }

    handle->can_rx = can_creat_interface(CAN_RX, dev_rc, id_rc, mask_rc);
    if(!handle->can_rx)
    {
        printf("can interface rx create failed\n");
        can_delet_interface(handle->can_rx);
        return NULL;
    }

    if (-1 == pthread_create(&handle->t_id_rx, NULL, can_rx_func, (void *)handle))
    {
        DBG_ERROR("can rx thread create failed.\n\r");
        can_delet_interface(handle->can_tx);
        can_delet_interface(handle->can_rx);
        return NULL;
    }

    return handle;
}