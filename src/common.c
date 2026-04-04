#include <sys/time.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include "common.h"

uint64_t timebase64_get(void)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000000ULL + (uint64_t)tv.tv_usec;
}

uint64_t timebase64_diff_us(uint64_t timeBase)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    return ((uint64_t)tv.tv_sec * 1000000ULL + (uint64_t)tv.tv_usec) - timeBase;
}

uint64_t timebase64_diff_ms(uint64_t timeBase)
{
    return timebase64_diff_us(timeBase) / 1000ULL;
}

void delay_us(uint32_t us)
{
    // usleep(us);
    uint64_t time_st = timebase64_get();

    while (timebase64_diff_us(time_st) < us);
}

void delay_ms(uint32_t ms)
{
    // while (ms--)
    // {
    //     usleep(1000);
    // }
    delay_us(ms * 1000);
}

void data_dump(char *prefix, char *buf, int len)
{
    int i;

    printf("%s", prefix);
    for (i = 0; i < len; i++)
    {
        printf("0x%02x ", buf[i]);
    }
    printf("\n");
}