#include "robot.h"
#define CHASSIS_RADIUS 0.0325

robot PHC;

void robot_init()
{
    BSP_I2C_Init();
    BSP_UART_Init();
    CRC16_Init();
    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)&PHC.adc_data, 1);
    //延时200毫秒
    HAL_Delay(200);
    //进行初始化
    // Olde初始化
    Oled_Init(&PHC.oled);
    HAL_Delay(200);
    //陀螺仪初始化
    MPU6050_config imu_config;
    imu_config.addr = 0x68;
    imu_config.bsp_i2c_index = 0;
    imu_config.accelbias[0] = 0;
    imu_config.accelbias[1] = 0;
    imu_config.accelbias[2] = 0;
    MPU6050_Create(&PHC.imu, &imu_config);
    //蓝牙初始化
    bt_config BTconfig;
    BTconfig.bsp_uart_index = 0;
    BTconfig.rx_len = 20;
    BlueTooth_Init(&PHC.bluetooth, &BTconfig);
    //电机初始化
    PID_config pid_motor_left_config;
    PID_config pid_motor_right_config;
    PID_SetConfig(&pid_motor_left_config, 5000, 1000, 3000, 1000, 65535);
    PID_SetConfig(&pid_motor_right_config, 5000, 1000, 3000, 1000, 65535);
    hardware_config hw_left_motor_config;
    hardware_config hw_right_motor_config;
    hw_left_motor_config.GPIO = GPIOB;
    hw_left_motor_config.PIN1 = GPIO_PIN_12;
    hw_left_motor_config.PIN2 = GPIO_PIN_13;
    hw_left_motor_config.encoder_htim_handle = &htim2;
    hw_left_motor_config.pwm_htim_handle = &htim1;
    hw_left_motor_config.pwm_channel = TIM_CHANNEL_1;
    hw_right_motor_config.GPIO = GPIOB;
    hw_right_motor_config.PIN1 = GPIO_PIN_14;
    hw_right_motor_config.PIN2 = GPIO_PIN_15;
    hw_right_motor_config.encoder_htim_handle = &htim3;
    hw_right_motor_config.pwm_htim_handle = &htim1;
    hw_right_motor_config.pwm_channel = TIM_CHANNEL_2;
    motor_init(&PHC.motor_left, pid_motor_left_config, hw_left_motor_config);
    motor_init(&PHC.motor_right, pid_motor_right_config, hw_right_motor_config);

    float matrix[12] = 
    {
        -66.0087,-110.0825 ,-232.8395  ,-41.4499  , 56.6947 ,  26.7274,
        -66.0087,-110.0825 ,-232.8395  ,-41.4499  ,-56.6947 , -26.7274,
    };
    lqr_set_matrix_K(&PHC.lqr,matrix);
    PHC.lqr.Outputmax = 100;             
}

void StateCalc(robot * obj)
{
    obj->speed = (-obj->motor_left.fdb_v + obj->motor_right.fdb_v)/2*CHASSIS_RADIUS;
    obj->displacement +=obj->speed*TIME/1000;
    if(obj->bluetooth.rxdata.cd.ch1!=0)
    {
        obj->displacement = 0;
    }
}


void Robot_Command()
{
    PHC.last_mode = PHC.mode;
    PHC.mode = PHC.bluetooth.rxdata.cd.stop & PHC.bluetooth.isConnect & PHC.imu.bias_init_success;
    if (PHC.mode != PHC.last_mode)
        PHC.oled.update2 = 1;
    PHC.motor_left.stop = !PHC.mode;
    PHC.motor_right.stop = !PHC.mode;
    if(PHC.bluetooth.rxdata.cd.reset)
    {
        bsp_soft_rest();
    }
    if(PHC.mode)
    {
        StateCalc(&PHC);
        PHC.lqr.StateFeedBack[0] = PHC.displacement;
        PHC.lqr.StateFeedBack[1] = PHC.speed;
        PHC.lqr.StateFeedBack[2] = -PHC.imu.data.euler[1];
        PHC.lqr.StateFeedBack[3] = -PHC.imu.data.gyro[1];
        PHC.lqr.StateFeedBack[4] = -PHC.imu.data.yaw;
        PHC.lqr.StateFeedBack[5] = -PHC.imu.data.gyro[0];
        PHC.lqr.StateRef[0] = 0;
        PHC.lqr.StateRef[1] = (float)PHC.bluetooth.rxdata.cd.ch1/200.0f;
        PHC.lqr.StateRef[2] = 0.021;
        PHC.lqr.StateRef[3] = 0;
        PHC.lqr.StateRef[4] += (float)PHC.bluetooth.rxdata.cd.ch0/4000.0f;
        PHC.lqr.StateRef[5] = 0;
        lqr_calc(&PHC.lqr);
        float ccr_l,ccr_r;
        ccr_l = -PHC.lqr.Output[0] / PHC.lqr.Outputmax * 65535 ;
        ccr_r = PHC.lqr.Output[1] / PHC.lqr.Outputmax * 65535 ;
        motor_control(&PHC.motor_left,ccr_l);
        motor_control(&PHC.motor_right,ccr_r);
    }
    else{
        motor_stop(&PHC.motor_left);
        motor_stop(&PHC.motor_right);
        PHC.speed = 0;
        PHC.displacement = 0;
        PHC.lqr.StateRef[4] = -PHC.imu.data.yaw;
    }
}

void Robot_Encoder(uint8_t choice)
{
    if (choice)
    {
        if (__HAL_TIM_IS_TIM_COUNTING_DOWN(&htim2))
        {
            PHC.motor_left.code.round--;
        }
        else
        {
            PHC.motor_left.code.round++;
        }
    }
    else
    {
        if (__HAL_TIM_IS_TIM_COUNTING_DOWN(&htim3))
        {
            PHC.motor_right.code.round--;
        }
        else
        {
            PHC.motor_right.code.round++;
        }
    }
}

void default_loop()
{
    static uint8_t count1 = 0;
    count1++;
    if (count1 > 100)
    {
        count1 = 0;
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_1);
    if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13))
    {
        if (PHC.bluetooth.isConnect == 0)
            PHC.oled.update4 = 1;
        PHC.bluetooth.isConnect = 1;
    }
    else
    {
        if (PHC.bluetooth.isConnect == 1)
            PHC.oled.update4 = 1;
        PHC.bluetooth.isConnect = 0;
    }
    }
    static uint8_t count2 = 0;
    count2++;
    if (count2 > 40)
    {
        count2 = 0;
        PHC.voltage = (float)PHC.adc_data / 4096 * 3.3 * 90 / 22;
        PHC.oled.update1 = 1;
    }
    
    if (!PHC.bluetooth.isConnect)
        memset(&PHC.bluetooth.rxdata.cd,0,sizeof(PHC.bluetooth.rxdata.cd));
}
