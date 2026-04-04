#include <stdint.h>
#include <linux/can.h>
#include <string.h>
#include "mit_protocol.h"
// #include "robot_hw.h"
#include "math_ops.h"

void Zero(struct can_frame *msg)
{
    msg->data[0] = 0xFF;
    msg->data[1] = 0xFF;
    msg->data[2] = 0xFF;
    msg->data[3] = 0xFF;
    msg->data[4] = 0xFF;
    msg->data[5] = 0xFF;
    msg->data[6] = 0xFF;
    msg->data[7] = 0xFE;
    //WriteAll(); ?
}

void EnterMotorMode(uint8_t *data)
{
    data[0] = 0xFF;
    data[1] = 0xFF;
    data[2] = 0xFF;
    data[3] = 0xFF;
    data[4] = 0xFF;
    data[5] = 0xFF;
    data[6] = 0xFF;
    data[7] = 0xFC;
    //WriteAll();
}

void ExitMotorMode(uint8_t *data)
{
    data[0] = 0xFF;
    data[1] = 0xFF;
    data[2] = 0xFF;
    data[3] = 0xFF;
    data[4] = 0xFF;
    data[5] = 0xFF;
    data[6] = 0xFF;
    data[7] = 0xFD;
    //WriteAll();
}

void pack_cmd(uint8_t *data, joint_control *joint)
{

    /// limit data to be within bounds ///
    float p_des = fminf(fmaxf(P_MIN, joint->p_des), P_MAX);
    float v_des = fminf(fmaxf(V_MIN, joint->v_des), V_MAX);
    float kp = fminf(fmaxf(KP_MIN, joint->kp), KP_MAX);
    float kd = fminf(fmaxf(KD_MIN, joint->kd), KD_MAX);
    float t_ff = fminf(fmaxf(T_MIN, joint->t_ff), T_MAX);
    /// convert floats to unsigned ints ///
    uint16_t p_int = float_to_uint(p_des, P_MIN, P_MAX, 16);
    uint16_t v_int = float_to_uint(v_des, V_MIN, V_MAX, 12);
    uint16_t kp_int = float_to_uint(kp, KP_MIN, KP_MAX, 12);
    uint16_t kd_int = float_to_uint(kd, KD_MIN, KD_MAX, 12);
    uint16_t t_int = float_to_uint(t_ff, T_MIN, T_MAX, 12);
    /// pack ints into the can buffer ///
    data[0] = p_int >> 8;
    data[1] = p_int & 0xFF;
    data[2] = v_int >> 4;
    data[3] = ((v_int & 0xF) << 4) | (kp_int >> 8);
    data[4] = kp_int & 0xFF;
    data[5] = kd_int >> 4;
    data[6] = ((kd_int & 0xF) << 4) | (t_int >> 8);
    data[7] = t_int & 0xff;
}

void pack_cmd_new(uint8_t *data, joint_control *joint, 
                    float p_max, float p_min, float v_max, float v_min, float t_max, float t_min)
{

    /// limit data to be within bounds ///
    float p_des = fminf(fmaxf(p_min, joint->p_des), p_max);
    float v_des = fminf(fmaxf(v_min, joint->v_des), v_max);
    float kp = fminf(fmaxf(KP_MIN, joint->kp), KP_MAX);
    float kd = fminf(fmaxf(KD_MIN, joint->kd), KD_MAX);
    float t_ff = fminf(fmaxf(t_min, joint->t_ff), t_max);
    /// convert floats to unsigned ints ///
    uint16_t p_int = float_to_uint(p_des, p_min, p_max, 16);
    uint16_t v_int = float_to_uint(v_des, v_min, v_max, 12);
    uint16_t kp_int = float_to_uint(kp, KP_MIN, KP_MAX, 12);
    uint16_t kd_int = float_to_uint(kd, KD_MIN, KD_MAX, 12);
    uint16_t t_int = float_to_uint(t_ff, t_min, t_max, 12);
    /// pack ints into the can buffer ///
    data[0] = p_int >> 8;
    data[1] = p_int & 0xFF;
    data[2] = v_int >> 4;
    data[3] = ((v_int & 0xF) << 4) | (kp_int >> 8);
    data[4] = kp_int & 0xFF;
    data[5] = kd_int >> 4;
    data[6] = ((kd_int & 0xF) << 4) | (t_int >> 8);
    data[7] = t_int & 0xff;
}

void pack_cmd_vel(uint8_t *data, joint_control *joint, float v_max, float v_min)
{


    float v_des = fminf(fmaxf(v_min, joint->v_des), v_max);

    uint8_t *vbuf;
    vbuf=(uint8_t*)&v_des;


    // uint32_t v_int = float_to_uint(v_des, v_min, v_max, 32);

    /// pack ints into the can buffer ///
    data[0] = *vbuf;
    data[1] = *(vbuf+1);
    data[2] = *(vbuf+2);
    data[3] = *(vbuf+3);
    data[4] = 0;
    data[5]= 0;
    data[6] =0;
    data[7] =0;
}


#if 0
int mit_zero(mHandle handle, int index)
{
    robot_hw_t *cassie = (robot_hw_t *)handle;

    int ret = 0;
    struct can_frame msg;
    memset(&msg, 0, sizeof(msg));

    int bus = 0;
    int id = 0;
    index2id(index, &bus, &id);

    msg.can_id = id;
    msg.can_dlc = 8;

    Zero(&msg);
    ret = cassie_can_send(cassie, bus, (char *)&msg, sizeof(msg));

    cassie->joint[index].timestamp_tx = timebase64_get();

    return ret;
}

int mit_enter(mHandle handle, int index)
{
    robot_hw_t *cassie = (robot_hw_t *)handle;

    int ret = 0;
    struct can_frame msg;
    memset(&msg, 0, sizeof(msg));

    int bus = 0;
    int id = 0;
    index2id(index, &bus, &id);

    msg.can_id = id;
    msg.can_dlc = 8;

    EnterMotorMode(&msg);
    ret = cassie_can_send(cassie, bus, (char *)&msg, sizeof(msg));

    cassie->joint[index].timestamp_tx = timebase64_get();

    return ret;
}

int mit_exit(mHandle handle, int index)
{
    robot_hw_t *cassie = (robot_hw_t *)handle;

    int ret = 0;
    struct can_frame msg;
    memset(&msg, 0, sizeof(msg));

    int bus = 0;
    int id = 0;
    index2id(index, &bus, &id);

    msg.can_id = id;
    msg.can_dlc = 8;

    ExitMotorMode(&msg);
    ret = cassie_can_send(cassie, bus, (char *)&msg, sizeof(msg));

    cassie->joint[index].timestamp_tx = timebase64_get();

    return ret;
}

int mit_motor_set_pos(mHandle handle, int index, float pos, float kp, float kd)
{
    robot_hw_t *cassie = (robot_hw_t *)handle;
    struct can_frame msg;
    joint_control joint;

    int bus = 0;
    int id = 0;
    int ret = 0;

    index2id(index, &bus, &id);

    memset(&msg, 0, sizeof(msg));
    memset(&joint, 0, sizeof(joint));

    msg.can_id = id;
    msg.can_dlc = 8;

    // sem_wait(&cassie->lock);

    cassie->joint[index].control.p_des = pos + cassie->joint[index].zero;

    if (-1 != (int)kp)
        cassie->joint[index].control.kp = kp;

    if (-1 != (int)kd)
        cassie->joint[index].control.kd = kd;

    joint.p_des = cassie->joint[index].control.p_des;
    joint.kp = cassie->joint[index].control.kp;
    joint.kd = cassie->joint[index].control.kd;

    pack_cmd(&msg, &joint);
    ret = cassie_can_send(cassie, bus, (char *)&msg, sizeof(msg));

    // sem_post(&cassie->lock);
    cassie->joint[index].timestamp_tx = timebase64_get();

    return ret;
}
#endif