#ifndef _TORQUE_SENSOR_H_
#define _TORQUE_SENSOR_H_

int dy200_init(char *dev, int bud);
int get_dy200_info(float *torque, float *speed, float *power);
// int get_dy200_info(float *torque, float *speed, float *power);

#endif