#ifndef __H_MOTOR_H__
#define __H_MOTOR_H__

#include "pid.h"
#include "tim.h"
#include "main.h"
#define TIME 5  //ms
typedef struct encoder_t
{
    int32_t encoder_cnt;
    int32_t last_encoder_cnt;
    int round;
}encoder;

typedef struct hardware_config_t
{
    //编码器设置
    TIM_HandleTypeDef * encoder_htim_handle;
    //pwm设置
    uint16_t pwm_channel;
    TIM_HandleTypeDef * pwm_htim_handle;
    //gpio设置
    uint16_t PIN1;
    uint16_t PIN2;
    GPIO_TypeDef* GPIO;

}hardware_config;

typedef struct motor_t
{
    hardware_config hw_config;
    float fdb_v;    //  rad/s
    float fdb_rpm;  //  r/min
    float ref_v;  
    float ref_rpm;
    uint8_t stop;
    PID pid;
    encoder code;
}motor;

void motor_init(motor*obj, PID_config pid_config,hardware_config hw_config);
void motor_getvelocity(motor*obj);
void motor_stop(motor * obj);
void motor_calc(motor * obj);
void motor_control(motor * obj, float output);
#endif