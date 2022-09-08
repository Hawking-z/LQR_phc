#include "motor.h"
#include "math.h"
#define PPR 500
#define REDUCTION_RATIO 30
#define SOFT_RATIO 4

#define RPM2RADS 0.10471975

void motor_init(motor*obj, PID_config pid_config,hardware_config hw_config)
{
    obj->fdb_v = 0;
    obj->fdb_rpm = 0;
    obj->code.round = 0;
    obj->code.encoder_cnt = 0;
    obj->stop = 1;
    obj->pid.config = pid_config;
    obj->hw_config = hw_config;
}

void motor_pwm_setccr(motor * obj,uint16_t ccr)
{
    __HAL_TIM_SET_COMPARE(obj->hw_config.pwm_htim_handle,obj->hw_config.pwm_channel,ccr);
}

void motor_stop(motor * obj)
{
    motor_pwm_setccr(obj,0);
    obj->ref_v = 0;
    obj->ref_rpm = 0;
    HAL_GPIO_WritePin(obj->hw_config.GPIO,obj->hw_config.PIN1,GPIO_PIN_RESET);
    HAL_GPIO_WritePin(obj->hw_config.GPIO,obj->hw_config.PIN2,GPIO_PIN_RESET);
}

void motor_control(motor * obj, float output)
{
    if(obj->stop)
    {
        motor_stop(obj);
        return;
    }
    if(output > 0)
    {
        HAL_GPIO_WritePin(obj->hw_config.GPIO,obj->hw_config.PIN1,GPIO_PIN_RESET);
        HAL_GPIO_WritePin(obj->hw_config.GPIO,obj->hw_config.PIN2,GPIO_PIN_SET);
    }
    else
    {
        HAL_GPIO_WritePin(obj->hw_config.GPIO,obj->hw_config.PIN1,GPIO_PIN_SET);
        HAL_GPIO_WritePin(obj->hw_config.GPIO,obj->hw_config.PIN2,GPIO_PIN_RESET);
    }
    motor_pwm_setccr(obj,fabsf(output));
}

void motor_calc(motor * obj)
{
    if(obj->stop)
    {
        motor_stop(obj);
        return;
    }
    //计算
    obj->pid.fdb = obj->fdb_v;
    obj->pid.ref = obj->ref_v;
    PID_Calc(&obj->pid);
    
    motor_control(obj,obj->pid.output);
}


void motor_getvelocity(motor * obj)
{
    obj->code.last_encoder_cnt = obj->code.encoder_cnt;
    obj->code.encoder_cnt = __HAL_TIM_GET_COUNTER(obj->hw_config.encoder_htim_handle) + obj->code.round * 65536;
    obj->fdb_rpm = 0.95*((float)(obj->code.encoder_cnt-obj->code.last_encoder_cnt))*(1000.0f/TIME)*60.0f/(SOFT_RATIO*PPR*REDUCTION_RATIO) + 0.05*obj->fdb_rpm;
    obj->fdb_v = obj->fdb_rpm *RPM2RADS;
}