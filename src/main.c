#include <unistd.h>
#include <stdio.h>
#include "torque_sensor.h"
#include "common.h"
#include "can_thread.h"
#include "finsh.h"

int main(){

    // printf("motor test \n");
    // if(dy200_init("/dev/ttyUSB0", 115200))
    //     return -1;

    // float torque,speed,power;
        
    // while(1){
    //     usleep(1000*100);
    //     get_dy200_info(&torque, &speed, &power);
    //     printf("torque:%.3f, speed:%.3f, power:%.3f\n", torque, speed, power);
    // }

    extern int motor_test_init();

    motor_test_init();


    return finsh();
}
