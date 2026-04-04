#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <stdint.h>
#include "can_create_socket.h"

int can_creat_tx_socket(char *dev_name, int *socket_fd)
{
    struct sockaddr_can addr;
    struct ifreq ifr;

    *socket_fd = socket(PF_CAN, SOCK_RAW, CAN_RAW); //创建套接字
    if (0 > *socket_fd)
    {
        printf("create can socket failed\n");
        goto error;
    }

    strcpy(ifr.ifr_name, dev_name);
    if (0 < ioctl(*socket_fd, SIOCGIFINDEX, &ifr)) //指定设备
    {
        printf("can ioctl failed\n");
        goto error;
    }

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    if (0 > bind(*socket_fd, (struct sockaddr *)&addr, sizeof(addr)))
    {
        printf("can bind failed\n");
        goto error;
    }

    // if (0 > setsockopt(*socket_fd, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0))
    // {
    //     printf("can setsockopt failed\n");
    //     goto error;
    // }

    return 0;

error:
    if (0 > *socket_fd)
    {
        close(*socket_fd);
        *socket_fd = 0;
    }

    return -1;
}

int can_creat_rx_socket(char *dev_name, int *socket_fd, uint16_t id, uint16_t id_mask)
{
    struct sockaddr_can addr;
    struct ifreq ifr;
    // struct can_filter rfilter[1];

    *socket_fd = socket(PF_CAN, SOCK_RAW, CAN_RAW); //创建套接字
    if (0 > *socket_fd)
        goto error;

    strcpy(ifr.ifr_name, dev_name);
    if (0 < ioctl(*socket_fd, SIOCGIFINDEX, &ifr)) //指定设备
    {
        printf("can ioctl failed\n");
        goto error;
    }

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    if (0 > bind(*socket_fd, (struct sockaddr *)&addr, sizeof(addr)))
    {
        printf("can bind failed\n");
        goto error;
    }

    // rfilter[0].can_id = id & id_mask;
    // rfilter[0].can_mask = CAN_SFF_MASK & id_mask;
    // if (0 > setsockopt(*socket_fd, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, sizeof(rfilter)))
    // {
    //     printf("can setsockopt failed\n");
    //     goto error;
    // }

    return 0;

error:
    if (0 > *socket_fd)
    {
        close(*socket_fd);
        *socket_fd = 0;
    }

    return -1;
}

mHandle can_creat_interface(can_type_t type, char *dev_name, uint16_t id, uint16_t id_mask)
{
    can_handle_t *handle = (can_handle_t *)malloc(sizeof(can_handle_t));
    if (!handle)
    {
        printf("malloc failed\n");
        return NULL;
    }

    memset(handle, 0, sizeof(can_handle_t));

    handle->type = type;
    strcpy(handle->dev_name, dev_name);
    handle->id = id;
    handle->id_mask = id_mask;

   switch (type)
    {
    case CAN_TX:
        /* code */
        if(can_creat_tx_socket(handle->dev_name, &handle->fd))
        {
            printf("create can socket error\n");
            break;
        }
        return handle;   
        
    case CAN_RX:
        if(can_creat_rx_socket(handle->dev_name, &handle->fd,  handle->id, handle->id_mask))
        {
            printf("create can socket error\n");
            break;
        }
        return handle;
    
    //not surport yet
    case CAN_TX_RX:
    default:
        break;
    }
    
    free(handle);
    return NULL;
}

int can_delet_interface(mHandle _handle)
{
    can_handle_t *handle = (can_handle_t *)_handle;

    if (handle)
    {
        if (handle->fd > 0)
            close(handle->fd);

        free(handle);
    }

    return 0;
}

int can_interface_send(mHandle _handle, char *buf, int len)
{
    can_handle_t *handle = (can_handle_t *)_handle;

    // int i;
    // printf("can send:\n");
    // for (i = 0; i < len; i++)
    // {
    //     printf("%02x ", buf[i]);
    // }
    // printf("\n");


    return send(handle->fd, buf, len, 0);
    //return write(handle->fd, buf, len);
}

int can_interface_recv(mHandle _handle, char *buf, int len, int nodelay)
{
    can_handle_t *handle = (can_handle_t *)_handle;

    return recv(handle->fd, buf, len, nodelay ? MSG_DONTWAIT : 0);
    //return read(handle->fd, buf, len);
}
