#ifndef __H_ROBOT_H__
#define __H_ROBOT_H__

#include "main.h"
#include "tim.h"
#include "adc.h"
#include "bsp_softreset.h"
#include "MPU6050.h"
#include "motor.h"
#include "oled.h"
#include "crc16.h"
#include "bsp_i2c.h"
#include "bsp_delay.h"
#include "bsp_uart.h"
#include "bluetooth.h"
#include "BalanceCalc.h"

typedef struct  robot_t
{
    OLED oled;
    MPU6050_imu imu;
    bt bluetooth;
    motor motor_right;
    motor motor_left;
    LQR lqr;
    uint8_t mode;
    uint8_t last_mode;
    uint16_t adc_data;
    float voltage;
    float speed;
    float displacement;
}robot;


void robot_init();
void Robot_Encoder(uint8_t choice);
void default_loop();
void Robot_Command();

#endif

