#ifndef _PID_CONTROL_H_
#define _PID_CONTROL_H_

typedef struct 
{
    float kp;
    float ki;
    float kd;

    float des;
    float input;
    float output;

    float error;
    float error_last;
    float error_all;

    float output_max;
    float error_max;
    float error_all_max;
}pid_control;

int pid_zero(pid_control *pid);
int pid_caculate(pid_control *pid);

#endif
