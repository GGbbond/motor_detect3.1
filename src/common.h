#ifndef _COMM_H_
#define _COMM_H_

#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif


typedef void * mHandle;

/*
 * Copied from include/linux/...
 */

#undef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:        the pointer to the member.
 * @type:       the type of the container struct this is embedded in.
 * @member:     the name of the member within the struct.
 *
 */
#define container_of(ptr, type, member) ({                      \
	const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
	(type *)( (char *)__mptr - offsetof(type,member) );})


typedef void * mHandle;

/** @brief 成功返回值*/
#define SUCCESS         0
/** @brief 失败返回值*/
#define FAILURE         (-1)
/** @brief 非法值*/
#define INVALID         (-1)

typedef union
{
	char c[4];
	int i;
}ctoi;

typedef struct
{
    uint32_t cmd;
    uint32_t param_1;
    uint32_t param_2;
    uint32_t param_3;
    uint32_t buf_len;
    char *buf;
}msg_t;

uint64_t timebase64_get(void);
uint64_t timebase64_diff_us(uint64_t timeBase);
uint64_t timebase64_diff_ms(uint64_t timeBase);

void delay_us(uint32_t us);
void delay_ms(uint32_t ms);

void data_dump(char *prefix, char *buf, int len);

#define GBL_ERROR printf
#define GBL_NOTICE printf
#define DBG_ERROR printf

#define ANG2RAD (3.1415926f/180.0f)

#ifdef __cplusplus
}
#endif

#endif