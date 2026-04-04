#ifndef _UART_OP_H_
#define _UART_OP_H_

int uart_open_normal(char *devname, int bud, int block);
int uart_flush(int fd);
int uart_send(int  fd, char *buf, int len);
int uart_recv(int fd, char *buf, int len);

typedef struct
{
    int fd;
    int bud;
    char dev[64];
}uart_commu_t;


#endif