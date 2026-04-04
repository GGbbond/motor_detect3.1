#include <pthread.h>
#include <string.h>
#include "pid_control.h"

float pid_limit(float val, float max)
{
    if (val > max)
        return max;
    
    if (val < -max)
        return -max;
    
    return val;
}

int pid_zero(pid_control *pid)
{
    if (!pid)
        return -1;
    
    memset(pid, 0, sizeof(pid_control));

    return 0;
}

int pid_caculate(pid_control *pid)
{
    if (!pid)
        return -1;

    pid->error_last = pid->error;
    pid->error = pid->des - pid->input;
    pid->error = pid_limit(pid->error, pid->error_max);
    pid->error_all += pid->error;
    pid->error_all = pid_limit(pid->error_all, pid->error_all_max);

    pid->output = pid->error * pid->kp + pid->error_all * pid->ki + (pid->error - pid->error_last) * pid->kd;
    pid->output = pid_limit(pid->output, pid->output_max);

    return 0;
}
