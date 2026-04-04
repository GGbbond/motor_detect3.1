#ifndef _MIT_PROTOCOL_H_
#define _MIT_PROTOCOL_H_

#include <stdint.h>
#include <linux/can.h>
// #include "robot_hw.h"
#include "common.h"

/// Value Limits ///
#define P_MIN -12.5f
#define P_MAX 12.5f
#define V_MIN -65.0f
#define V_MAX 65.0f
#define KP_MIN 0.0f
#define KP_MAX 500.0f
#define KD_MIN 0.0f
#define KD_MAX 5.0f
#define T_MIN -18.0f
#define T_MAX 18.0f

// typedef struct
// {
//     float p, v, t; //pos vel torque
// }joint_state;

// typedef struct
// {
//     float p_des, v_des, kp, kd, t_ff;
// }joint_control;

// typedef struct 
// {
//     joint_state hy, hr, hp, kp, ss, hs, ap;
//     // uint32_t touch;
// }leg_state;

// typedef struct 
// {
//     joint_control hy, hr, hp, kp, ap;
// }leg_control;


// struct spi_data_t
// {
//     // float q_abad[2];
//     // float q_hip[2];
//     // float q_knee[2];
//     // float qd_abad[2];
//     // float qd_hip[2];
//     // float qd_knee[2];
//     joint_state leg[2];
//     int32_t flags[2];
//     int32_t checksum;
// };

// // 132 bytes
// // 66 16-bit words
// struct spi_command_t
// {
//     // float q_des_abad[2];
//     // float q_des_hip[2];
//     // float q_des_knee[2];
//     // float qd_des_abad[2];
//     // float qd_des_hip[2];
//     // float qd_des_knee[2];
//     // float kp_abad[2];
//     // float kp_hip[2];
//     // float kp_knee[2];
//     // float kd_abad[2];
//     // float kd_hip[2];
//     // float kd_knee[2];
//     // float tau_abad_ff[2];
//     // float tau_hip_ff[2];
//     // float tau_knee_ff[2];
//     leg_control control[2];
//     int32_t flags[2];
//     int32_t checksum;
// };

/*

//�����ؽ�   
hip yaw
hip roll
hip pitch
kenn pitch

// �����ؽ�
skin spring
hell spring

ankle
*/

// typedef enum 
// {
//     joint_hy=0,
//     joint_hr,
//     joint_hp,
//     joint_kp,
//     joint_ap
// }Joint_Name;

typedef struct
{
    float p_des, v_des, kp, kd, t_ff;
}joint_control;

typedef struct
{
    float p, v, t;
}joint_state;

void Zero(struct can_frame *msg);
void EnterMotorMode(uint8_t *data);
void ExitMotorMode(uint8_t *data);
void pack_cmd(uint8_t *data, joint_control *joint);
void pack_cmd_new(uint8_t *data, joint_control *joint, 
                    float p_max, float p_min, float v_max, float v_min, float t_max, float t_min);
void pack_cmd_vel(uint8_t *data, joint_control *joint, float v_max, float v_min);

int mit_zero(mHandle handle, int index);
int mit_enter(mHandle handle, int index);
int mit_exit(mHandle handle, int index);
int mit_motor_set_pos(mHandle handle, int index, float pos, float kp, float kd);

#endif