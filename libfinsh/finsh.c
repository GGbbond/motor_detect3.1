#include "finsh.h"
// #include "stor_elog.h"

static int quit_flag = 0;
#define FINSH_CMD_SIZE 128
#define FINSH_HISTORY_LINES 100
enum input_stat
{
    WAIT_NORMAL,
    WAIT_SPEC_KEY,
    WAIT_FUNC_KEY,
};

struct finsh_shell
{
    enum input_stat stat;

    uint8_t echo_mode: 1;
    uint8_t prompt_mode: 1;

    uint16_t current_history;
    uint16_t history_count;

    char cmd_history[FINSH_HISTORY_LINES][FINSH_CMD_SIZE];

    char line[FINSH_CMD_SIZE + 1];
    uint16_t line_position;
    uint16_t line_curpos;

#ifdef FINSH_USING_AUTH
    char password[6];
#endif
};
struct finsh_shell *shell;
static LIST_HEAD(func_list);

#define FINSH_ARG_MAX    8
typedef int (*cmd_function_t)(int argc, char **argv);

void cmd_list_add(struct finsh_syscall cmd)
{
    syscall_user_space *node = malloc(sizeof(syscall_user_space));
    memcpy(node,&cmd,sizeof(syscall_user_space));
    list_add(&node->link,&func_list);
}
int help(int argc, char *argv[])
{
    syscall_user_space *p;
    struct list_head *pos,*n;
    list_for_each_prev_safe(pos, n, &func_list) {
        p = list_entry(pos,syscall_user_space,link);
        printf("%-32s - %s\n",p->name,p->desc);
    }
    return 0;
}
FINSH_FUNCTION_EXPORT(help,help)

static void shell_push_history(struct finsh_shell *shell)
{
    if (shell->line_position != 0)
    {
        /* push history */
        if (shell->history_count >= FINSH_HISTORY_LINES)
        {
            /* if current cmd is same as last cmd, don't push */
            if (memcmp(&shell->cmd_history[FINSH_HISTORY_LINES - 1], shell->line, FINSH_CMD_SIZE))
            {
                /* move history */
                int index;
                for (index = 0; index < FINSH_HISTORY_LINES - 1; index ++)
                {
                    memcpy(&shell->cmd_history[index][0],
                           &shell->cmd_history[index + 1][0], FINSH_CMD_SIZE);
                }
                memset(&shell->cmd_history[index][0], 0, FINSH_CMD_SIZE);
                memcpy(&shell->cmd_history[index][0], shell->line, shell->line_position);

                /* it's the maximum history */
                shell->history_count = FINSH_HISTORY_LINES;
            }
        }
        else
        {
            /* if current cmd is same as last cmd, don't push */
            if (shell->history_count == 0 || memcmp(&shell->cmd_history[shell->history_count - 1], shell->line, FINSH_CMD_SIZE))
            {
                shell->current_history = shell->history_count;
                memset(&shell->cmd_history[shell->history_count][0], 0, FINSH_CMD_SIZE);
                memcpy(&shell->cmd_history[shell->history_count][0], shell->line, shell->line_position);

                /* increase count and set current history position */
                shell->history_count ++;
            }
        }
    }
    shell->current_history = shell->history_count;
}


static int str_common(const char *str1, const char *str2)
{
    const char *str = str1;

    while ((*str != 0) && (*str2 != 0) && (*str == *str2))
    {
        str ++;
        str2 ++;
    }

    return (str - str1);
}

void msh_auto_complete(char *prefix)
{
    int length, min_length;
    const char *name_ptr, *cmd_name;
    struct finsh_syscall *index;

    min_length = 0;
    name_ptr = NULL;

    if (*prefix == '\0')
    {
        //msh_help(0, RT_NULL);
        return;
    }

    /* checks in internal command */
    //{
    struct list_head *pos,*n;
    list_for_each_prev_safe(pos, n, &func_list) {
        index = list_entry(pos,syscall_user_space,link);
        //for (index = &cmd[0]; index < (struct finsh_syscall *)((char *)&cmd[0]+sizeof(cmd)); index++)
        //{
            /* skip finsh shell function */
            cmd_name = (const char *) index->name;
            if (strncmp(prefix, cmd_name, strlen(prefix)) == 0)
            {
                if (min_length == 0)
                {
                    /* set name_ptr */
                    name_ptr = cmd_name;
                    /* set initial length */
                    min_length = strlen(name_ptr);
                }

                length = str_common(name_ptr, cmd_name);
                if (length < min_length)
                    min_length = length;

                printf("%s\n", cmd_name);
            }
        }
    //}

    /* auto complete string */
    if (name_ptr != NULL)
    {
        strncpy(prefix, name_ptr, min_length);
    }

    return ;
}


static void shell_auto_complete(char *prefix)
{
    printf("\n");
    msh_auto_complete(prefix);

    printf("%s%s", "msh />", prefix);
}
static int shell_handle_history(struct finsh_shell *shell)
{

    printf("\033[2K\r");

    printf("%s%s", "msh />", shell->line);
    return -1;
}

static int msh_split(char *cmd, uint32_t length, char *argv[FINSH_ARG_MAX])
{
    char *ptr;
    uint32_t position;
    uint32_t argc;
    uint32_t i;

    ptr = cmd;
    position = 0;
    argc = 0;

    while (position < length)
    {
        /* strip bank and tab */
        while ((*ptr == ' ' || *ptr == '\t') && position < length)
        {
            *ptr = '\0';
            ptr ++;
            position ++;
        }

        if (argc >= FINSH_ARG_MAX)
        {
            printf("Too many args ! We only Use:\n");
            for (i = 0; i < argc; i++)
            {
                printf("%s ", argv[i]);
            }
            printf("\n");
            break;
        }

        if (position >= length) break;

        /* handle string */
        if (*ptr == '"')
        {
            ptr ++;
            position ++;
            argv[argc] = ptr;
            argc ++;

            /* skip this string */
            while (*ptr != '"' && position < length)
            {
                if (*ptr == '\\')
                {
                    if (*(ptr + 1) == '"')
                    {
                        ptr ++;
                        position ++;
                    }
                }
                ptr ++;
                position ++;
            }
            if (position >= length) break;

            /* skip '"' */
            *ptr = '\0';
            ptr ++;
            position ++;
        }
        else
        {
            argv[argc] = ptr;
            argc ++;
            while ((*ptr != ' ' && *ptr != '\t') && position < length)
            {
                ptr ++;
                position ++;
            }
            if (position >= length) break;
        }
    }

    return argc;
}
static cmd_function_t msh_get_cmd(char *cmds, int size)
{
    struct finsh_syscall *index;
    cmd_function_t cmd_func = NULL;
	//printf("size:%d\n",size);
    struct list_head *pos,*n;
    list_for_each_prev_safe(pos, n, &func_list) {
        index = list_entry(pos,syscall_user_space,link);
    //for (index = &cmd[0]; index < (struct finsh_syscall *)((char *)&cmd[0]+sizeof(cmd)); index++)
    //{
		//printf("1:%s\n",index->name);
        if (strncmp(index->name, cmds, size) == 0 &&
                index->name[size] == '\0')
        {
			//printf("12\n");
            cmd_func = (cmd_function_t)index->func;
            break;
        }
    }
	//printf("func:%s\n",index->name);
    return cmd_func;
}
static int _msh_exec_cmd(char *cmd, uint32_t length, int *retp)
{
    int argc;
    uint32_t cmd0_size = 0;
    cmd_function_t cmd_func;
    char *argv[FINSH_ARG_MAX];

    //RT_ASSERT(cmd);
    //RT_ASSERT(retp);
	//printf("lenth:%d\n",length);
    /* find the size of first command */
    while ((cmd[cmd0_size] != ' ' && cmd[cmd0_size] != '\t') && (cmd0_size < length))
	{
        cmd0_size=cmd0_size+1;;
		//printf("cmdlen:%d\n",cmd0_size);
	}
	//printf("%p\n",&cmd0_size);
	//printf("cmdlen:%d\n",cmd0_size);
	
    if (cmd0_size == 0)
        return -1;
	
	//printf("cmdlen33:%d\n",cmd0_size);
	//printf("%p\n",&cmd0_size);
	
    cmd_func = msh_get_cmd(cmd, cmd0_size);
    if (cmd_func == NULL)
        return -1;
    /* split arguments */
    memset(argv, 0x00, sizeof(argv));
    argc = msh_split(cmd, length, argv);
    if (argc == 0)
        return -1;
	//printf("argc:%d %s\n",argc,argv[1]);
    /* exec this command */
    *retp = cmd_func(argc, argv);
    return 0;
}
int msh_exec(char *cmd, uint32_t length)
{
    int cmd_ret;
	//printf("%s %d\n",cmd,length);
    /* strim the beginning of command */
    while ((length > 0) && (*cmd  == ' ' || *cmd == '\t'))
    {
        cmd++;
        length--;
    }
	//printf("%s %d\n",cmd,length);
    if (length == 0)
        return 0;

    /* Exec sequence:
     * 1. built-in command
     * 2. module(if enabled)
     */
    if (_msh_exec_cmd(cmd, length, &cmd_ret) == 0)
    {
        return cmd_ret;
    }


    /* truncate the cmd at the first space. */
    {
        char *tcmd;
        tcmd = cmd;
        while (*tcmd != ' ' && *tcmd != '\0')
        {
            tcmd++;
        }
        *tcmd = '\0';
    }
    printf("%s: command not found.\n", cmd);
    return -1;
}

struct termios initial_settings;
static int finsh_flag = 0;
//void elog_assert_set_hook(void (*hook)(const char* expr, const char* func, size_t line));
static void process_exit() 
{
    if (tcsetattr(fileno(stdin),TCSANOW,&initial_settings)) {
        printf("tcsetattr failed\n");
    }
}
void finsh_elog_assert_hook(const char* expr, const char* func, size_t line)
{
    if (finsh_flag)
        process_exit();
    //elog_a("elog", "(%s) has assert failed at %s:%ld.", expr, func, line); 
    printf("elog:""(%s) has assert failed at %s:%ld.", expr, func, line); 
    *(int *)0 = 0;
}
int finsh(void)
{
	char ch;
	shell = calloc(1,sizeof(struct finsh_shell));
	struct termios new_settings;
    tcgetattr(fileno(stdin),&initial_settings);
    new_settings = initial_settings;
    new_settings.c_lflag &= ~ICANON;
    new_settings.c_lflag &= ~ECHO;
    new_settings.c_cc[VMIN]  = 1;
    new_settings.c_cc[VTIME] = 0;
    new_settings.c_lflag &= ~ISIG;
    if (tcsetattr(fileno(stdin),TCSANOW,&new_settings)) {
        printf("tcsetattr failed\n");
        return -1;
    }
    finsh_flag = 1;
	shell->echo_mode = 1;
	printf("msh />");
	while (1)
    {
        if(quit_flag) break;
        ch = (int)getchar();
        if (ch < 0)
        {
            continue;
        }
        /*
         * handle control key
         * up key  : 0x1b 0x5b 0x41
         * down key: 0x1b 0x5b 0x42
         * right key:0x1b 0x5b 0x43
         * left key: 0x1b 0x5b 0x44
         */
        if (ch == 0x1b)
        {
            shell->stat = WAIT_SPEC_KEY;
            continue;
        }
        else if (shell->stat == WAIT_SPEC_KEY)
        {
            if (ch == 0x5b)
            {
                shell->stat = WAIT_FUNC_KEY;
                continue;
            }

            shell->stat = WAIT_NORMAL;
        }
        else if (shell->stat == WAIT_FUNC_KEY)
        {
            shell->stat = WAIT_NORMAL;

            if (ch == 0x41) /* up key */
            {
                /* prev history */
                if (shell->current_history > 0)
                    shell->current_history --;
                else
                {
                    shell->current_history = 0;
                    continue;
                }

                /* copy the history command */
                memcpy(shell->line, &shell->cmd_history[shell->current_history][0],
                       FINSH_CMD_SIZE);
                shell->line_curpos = shell->line_position = strlen(shell->line);
                shell_handle_history(shell);
                continue;
            }
            else if (ch == 0x42) /* down key */
            {
                /* next history */
                if (shell->current_history < shell->history_count - 1)
                    shell->current_history ++;
                else
                {
                    /* set to the end of history */
                    if (shell->history_count != 0)
                        shell->current_history = shell->history_count - 1;
                    else
                        continue;
                }

                memcpy(shell->line, &shell->cmd_history[shell->current_history][0],
                       FINSH_CMD_SIZE);
                shell->line_curpos = shell->line_position = strlen(shell->line);
                shell_handle_history(shell);
                continue;
            }
            else if (ch == 0x44) /* left key */
            {
                if (shell->line_curpos)
                {
                    printf("\b");
                    shell->line_curpos --;
                }

                continue;
            }
            else if (ch == 0x43) /* right key */
            {
                if (shell->line_curpos < shell->line_position)
                {
                    printf("%c", shell->line[shell->line_curpos]);
                    shell->line_curpos ++;
                }

                continue;
            }
        }

        /* received null or error */
        if (ch == '\0' || ch == 0xFF) continue;
        /* handle tab key */
        else if (ch == '\t')
        {
            int i;
            /* move the cursor to the beginning of line */
            for (i = 0; i < shell->line_curpos; i++)
                printf("\b");

            /* auto complete */
            shell_auto_complete(&shell->line[0]);
            /* re-calculate position */
            shell->line_curpos = shell->line_position = strlen(shell->line);

            continue;
        }
        /* handle backspace key */
        else if (ch == 0x7f || ch == 0x08)
        {
            /* note that shell->line_curpos >= 0 */
            if (shell->line_curpos == 0)
                continue;

            shell->line_position--;
            shell->line_curpos--;

            if (shell->line_position > shell->line_curpos)
            {
                int i;

                memmove(&shell->line[shell->line_curpos],
                           &shell->line[shell->line_curpos + 1],
                           shell->line_position - shell->line_curpos);
                shell->line[shell->line_position] = 0;

                printf("\b%s  \b", &shell->line[shell->line_curpos]);

                /* move the cursor to the origin position */
                for (i = shell->line_curpos; i <= shell->line_position; i++)
                    printf("\b");
            }
            else
            {
                printf("\b \b");
                shell->line[shell->line_position] = 0;
            }

            continue;
        }

        /* handle end of line, break */
        if (ch == '\r' || ch == '\n')
        {
            shell_push_history(shell);
            if (shell->echo_mode)
                printf("\n");
            msh_exec(shell->line, shell->line_position);

            if (!quit_flag) printf("msh />");
            memset(shell->line, 0, sizeof(shell->line));
            shell->line_curpos = shell->line_position = 0;
            continue;
        }

        /* it's a large line, discard it */
        if (shell->line_position >= FINSH_CMD_SIZE)
            shell->line_position = 0;

        /* normal character */
        if (shell->line_curpos < shell->line_position)
        {
            int i;

            memmove(&shell->line[shell->line_curpos + 1],
                       &shell->line[shell->line_curpos],
                       shell->line_position - shell->line_curpos);
            shell->line[shell->line_curpos] = ch;
            if (shell->echo_mode)
                printf("%s", &shell->line[shell->line_curpos]);

            /* move the cursor to new position */
            for (i = shell->line_curpos; i < shell->line_position; i++)
                printf("\b");
        }
        else
        {
            shell->line[shell->line_position] = ch;
            if (shell->echo_mode)
                printf("%c", ch);
        }

        ch = 0;
        shell->line_position ++;
        shell->line_curpos++;
        if (shell->line_position >= FINSH_CMD_SIZE)
        {
            /* clear command line */
            shell->line_position = 0;
            shell->line_curpos = 0;
        }
    } /* end of device read */
	if (tcsetattr(fileno(stdin),TCSANOW,&initial_settings)) {
        printf("tcsetattr failed\n");
        return -1;
    }
    return 0;
}


static LIST_HEAD(log_list);

static struct spdk_log_flag* 
get_log_flag(const char *name)
{
    struct list_head *pos,*n;
    struct spdk_log_flag *flag;
    list_for_each_prev_safe(pos, n, &log_list) {
        flag = list_entry(pos,struct spdk_log_flag,link);
        if (strcasecmp(name, flag->name) == 0) {
            return flag;
        }
    }
    return NULL;
}
int spdk_log_get_flag(const char *name)
{
	struct spdk_log_flag *flag = get_log_flag(name);

	if (flag && flag->enabled) {
		return 1;
	}

	return 0;
}

// static int
// log_set_flag(const char *name, int value)
// {
// 	struct spdk_log_flag *flag;

// 	flag = get_log_flag(name);
// 	if (flag == NULL) {
//         printf("no find log flag\n");
// 		return -1;
// 	}

// 	flag->enabled = value;

// 	return 0;
// }
// int log_set_flag_cmd(int argc,char *argv[])
// {
//     ASSERT_ERROR_RETURN(argc > 2);
//     uint32_t value = strtoll(argv[2],NULL,0);
//     log_set_flag(argv[1],value);

//     return 0;
// }
// FINSH_FUNCTION_EXPORT(log_set_flag_cmd,log of module on/off)
// int log_flag_show(int argc,char *argv[])
// {
//     struct list_head *pos,*n;
//     struct spdk_log_flag *flag;
//     printf("[\n");
//     list_for_each_prev_safe(pos, n, &log_list) {
//         flag = list_entry(pos,struct spdk_log_flag,link);
//         printf("  %s\n",flag->name);
//     }
//     printf("]\n");
//     return 0;
// }
// FINSH_FUNCTION_EXPORT(log_flag_show,log_flag_show)
int spdk_log_register_flag(const char *name, struct spdk_log_flag *flag)
{
    list_add(&flag->link,&log_list);
    return 0;
}
int quit(int argc,char *argv[])
{
    quit_flag = 1;
    return 0;
}
FINSH_FUNCTION_EXPORT(quit,quit)
int q(int argc,char *argv[])
{
    quit_flag = 1;
    return 0;
}
FINSH_FUNCTION_EXPORT(q,quit)
// static uint16_t global_log_level = 5;
// int global_log_level_set(int argc, char *argv[])
// {
//     ASSERT_ERROR_RETURN(argc > 1);
//     char* log_level[6] = {
//         "LOG_LVL_ASSERT ", 
//         "LOG_LVL_ERROR  ",
//         "LOG_LVL_WARN   ",
//         "LOG_LVL_INFO   ",
//         "LOG_LVL_DEBUG  ",
//         "LOG_LVL_VERBOSE"
//     };
//     int i;
//     uint16_t level = strtoll(argv[1],NULL,0);
//     if (level > 5) {
//         printf("[log level error]:\r\n");
//         for (i=0;i<sizeof(log_level)/sizeof(log_level[0]);i++)
//           printf("%-16s    %d\n",log_level[i],i);
//         return -1;
//     }
//     elog_set_filter_lvl(level);
//     global_log_level = level;
//     printf("set global log level:%s\n",log_level[level]);
//     return 0;
// }
// FINSH_FUNCTION_EXPORT(global_log_level_set,set global log level)
// int global_log_level_get(int argc, char *argv[])
// {
//     char* log_level[6] = {
//         "LOG_LVL_ASSERT ", 
//         "LOG_LVL_ERROR  ",
//         "LOG_LVL_WARN   ",
//         "LOG_LVL_INFO   ",
//         "LOG_LVL_DEBUG  ",
//         "LOG_LVL_VERBOSE"
//     };
//     uint16_t level = global_log_level;
//     if (level <= 5)
//         printf("get global log level:%s\n",log_level[level]);

//     return 0;
// }
// FINSH_FUNCTION_EXPORT(global_log_level_get,get global log level)
// int set_filter_tag(int argc, char *argv[])
// {
//     ASSERT_ERROR_RETURN(argc > 1);
//     elog_set_filter_tag(argv[1]);
//     return 0;
// }
// FINSH_FUNCTION_EXPORT(set_filter_tag,only tag output)
// int set_filter_tag_lvl(int argc, char *argv[])
// {
//     ASSERT_ERROR_RETURN(argc > 2);
//     int level = strtoll(argv[2],NULL,0);
//     elog_set_filter_tag_lvl(argv[1], level);
//     return 0;
// }
// FINSH_FUNCTION_EXPORT(set_filter_tag_lvl,set tag level)
// int get_filter_tag_lvl(int argc, char *argv[])
// {
//     ASSERT_ERROR_RETURN(argc > 1);
//     char* log_level[6] = {
//         "LOG_LVL_ASSERT ", 
//         "LOG_LVL_ERROR  ",
//         "LOG_LVL_WARN   ",
//         "LOG_LVL_INFO   ",
//         "LOG_LVL_DEBUG  ",
//         "LOG_LVL_VERBOSE"
//     };
//     int i;
//     for (i=0;i<sizeof(log_level)/sizeof(log_level[0]);i++)
//       printf("%-16s    %d\n",log_level[i],i);
//     int level  = elog_get_filter_tag_lvl(argv[1]);
//     printf("\r\ntag:%s level:%s\n",argv[1],log_level[level]);
//     return 0;
// }
// FINSH_FUNCTION_EXPORT(get_filter_tag_lvl,get tag level)
