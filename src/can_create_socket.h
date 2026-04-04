#ifndef _CAN_CREATE_SOCKET_H_
#define _CAN_CREATE_SOCKET_H_

#include <stdint.h>
#include "common.h"

typedef enum {
    CAN_TX,
    CAN_RX,
    CAN_TX_RX
}can_type_t; 

typedef struct 
{
    int fd;
    char dev_name[64];
    can_type_t type;
    uint16_t id;
    uint16_t id_mask;
}can_handle_t;

int can_creat_tx_socket(char *dev_name, int *socket_fd);
int can_creat_rx_socket(char *dev_name, int *socket_fd, uint16_t id, uint16_t id_mask);

mHandle can_creat_interface(can_type_t type, char *dev_name, uint16_t id, uint16_t id_mask);
int can_delet_interface(mHandle _handle);
int can_interface_send(mHandle _handle, char *buf, int len);
int can_interface_recv(mHandle _handle, char *buf, int len, int nodelay);

#endif