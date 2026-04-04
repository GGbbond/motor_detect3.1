#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
// #include "common.h"
#include "uart_op.h"

int uart_open_normal(char *devname, int bud , int block)
{
    int fd = open(devname, O_RDWR | O_NOCTTY);
    if (fd < 0)
        return -1;

    int speed = 0;
    struct termios oldtio = {0};
    struct termios newtio = {0};
    tcgetattr(fd, &oldtio);

    switch (bud)
    {
    case 2400:
        speed = B2400;
        break;
    case 4800:
        speed = B4800;
        break;
    case 9600:
        speed = B9600;
        break;
    case 115200:
        speed = B115200;
        break;
    case 460800:
        speed = B460800;
        break;
    default:
        speed = B9600;
        break;
    }

    newtio.c_cflag = speed | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = 0; // IGNPAR | ICRNL
    newtio.c_oflag = 0;
    newtio.c_lflag = 0; // ICANON
    newtio.c_cc[VTIME] = 0;
    newtio.c_cc[VMIN] = 1;

    tcflush(fd, TCIOFLUSH);
    tcsetattr(fd, TCSANOW, &newtio);

    if(!block)
        fcntl(fd, F_SETFL, O_NONBLOCK);

    return fd;
}

int uart_flush(int fd)
{
    return tcflush(fd, TCIOFLUSH);
}


int uart_send(int  fd, char *buf, int len)
{
    // uart_commu_t *handle = (uart_commu_t *)_handle;

    return write(fd, buf, len);
}

int uart_recv(int fd, char *buf, int len)
{
    // uart_commu_t *handle = (uart_commu_t *)_handle;

    return read(fd, buf, len);
}