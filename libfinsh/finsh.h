#ifndef __FINSH_H__
#define __FINSH_H__
#include <stdio.h>
#include <stdint.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef long (*syscall_func)(void);
typedef struct finsh_syscall
{
    const char*     name;
    const char*     desc;
    syscall_func    func;
    struct list_head link;
}syscall_user_space;

#define __CMD_MODULE_REGISTER(line)     __CMD_MODULE_REGISTER_(line)
#define __CMD_MODULE_REGISTER_(line)    cmd_register_##line
#define FINSH_FUNCTION_EXPORT_CMD(name, cmd, desc)      \
    extern void cmd_list_add(struct finsh_syscall cmd); \
    const char __fsym_##cmd##_name[] = #cmd;            \
    const char __fsym_##cmd##_desc[] = #desc;           \
    const struct finsh_syscall __fsym_##cmd =           \
    {                                                   \
        __fsym_##cmd##_name,                            \
        __fsym_##cmd##_desc,                            \
        (syscall_func)&name                             \
    };                                                  \
    __attribute__((constructor(101))) static void       \
    __CMD_MODULE_REGISTER(__LINE__)(void)              \
    {                                                   \
        cmd_list_add(__fsym_##cmd);                     \
    }

#define FINSH_FUNCTION_EXPORT(name, desc)   \
    FINSH_FUNCTION_EXPORT_CMD(name, name, desc)

void finsh_elog_assert_hook(const char* expr, const char* func, size_t line);
int finsh(void);

struct spdk_log_flag {
    const char *name;
    int   enabled;
    struct list_head link;
};
#define SPDK_LOG_REGISTER_COMPONENT(FLAG) \
struct spdk_log_flag SPDK_LOG_##FLAG = { \
        .enabled = 0, \
        .name = #FLAG, \
}; \
__attribute__((constructor)) static void register_flag_##FLAG(void) \
{ \
    extern int spdk_log_register_flag(const char *name, struct spdk_log_flag *flag);\
    spdk_log_register_flag(#FLAG, &SPDK_LOG_##FLAG); \
}


#if 0
#define SF_E_SUCCESS(rv)              ((rv) >= 0)
#define SF_E_FAILURE(rv)              ((rv) < 0)

#define SF_E_SUCCESS_RETURN(op) do { \
    int32_t __rv__; \
    if ((__rv__ = (op)) >= 0) { \
        return(__rv__); \
    } } while (0)

#define SF_E_ERROR_RETURN(op) do { \
    int32_t __rv__; \
    if ((__rv__ = (op)) < 0) { \
        return(__rv__); \
    } } while (0)

#define SF_ERROR_GOTO(op, ret, label) do { \
    ret = (op); \
    if (ret < 0) { \
        goto label; \
    } } while (0)

#define SF_IF_NULL_RETURN(ptr) do { \
    if (NULL == (ptr)) { \
        log_e("NULL error"); \
        return -1; \
    } } while (0)

#define SF_GOTO_LABEL_IF_NULL(ptr, ret, label) do { \
    if (NULL == (ptr)) { \
        log_e("NULL error"); \
        goto label; \
    } } while (0)
		
#endif

#ifdef __cplusplus
}
#endif

#endif
