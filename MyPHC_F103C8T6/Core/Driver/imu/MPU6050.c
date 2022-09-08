#include "MPU6050.h"
#include "i2c.h"
#include "bsp_i2c.h"
#include "cvector.h"
#include "stdlib.h"
#include "string.h"
#include "main.h"
#include "usart.h"
#define PI 3.1415926
#define DEG2RAD 0.0174532922
#define RAD2DEG 57.295780490442965
//量程/65536*pi/180
#define MPU6050_GYRO_SEN0  0.00013315805450396191
#define MPU6050_GYRO_SEN1  0.00026631610900792382
#define MPU6050_GYRO_SEN2  0.00053263221801584765
#define MPU6050_GYRO_SEN3  0.00106526443603169530
//量程/65536/9.8
#define MPU6050_ACCEL_SEN0 0.00059814453125
#define MPU6050_ACCEL_SEN1 0.0011962890625
#define MPU6050_ACCEL_SEN2 0.002392578125
#define MPU6050_ACCEL_SEN3 0.00478515625
#define bsp_delay_ms HAL_Delay
#define MPU6050_BIAS_INIT_DISCARD 1000
#define MPU6050_BIAS_INIT_COUNT 2000

uint8_t MPU_Write_Len(MPU6050_imu* obj, uint8_t reg, uint8_t len, uint8_t* buf);
uint8_t MPU_Read_Len(MPU6050_imu* obj, uint8_t reg, uint8_t len, uint8_t* buf);
uint8_t MPU_Write_Byte(MPU6050_imu* obj, uint8_t reg, uint8_t data);
uint8_t MPU_Read_Byte(MPU6050_imu* obj, uint8_t reg);
uint8_t MPU_Set_Gyro_Fsr(MPU6050_imu* obj, uint8_t fsr);
uint8_t MPU_Set_Accel_Fsr(MPU6050_imu* obj, uint8_t fsr);
uint8_t MPU_Set_LPF(MPU6050_imu* obj, uint16_t lpf);
uint8_t MPU_Set_Rate(MPU6050_imu* obj, uint16_t rate);
uint8_t MPU_Set_Fifo(MPU6050_imu* obj, uint8_t sens);
float MPU_Get_Temperature(MPU6050_imu* obj);
uint8_t MPU_Get_Gyroscope(MPU6050_imu* obj, short* gx, short* gy, short* gz);
uint8_t MPU_Get_Accelerometer(MPU6050_imu* obj, short* ax, short* ay, short* az);
uint8_t MPU6050_init(MPU6050_imu* obj);


void MPU6050_Create(MPU6050_imu * obj , MPU6050_config* config) {
    obj->config = *config;
    if (obj->config.addr == 0x68) {
        obj->read_addr = 0xD1;
        obj->write_addr = 0xD0;
    } else {
        obj->read_addr = 0xD3;
        obj->write_addr = 0xD2;
    }
    obj->bias_init_success = 0;
    obj->init_error = 0;
    obj->temp = 0;
    memset(obj->gyrobias, 0, sizeof(float) * 3);
    while (MPU6050_init(obj));
    MahonyAHRS_init(&obj->mahony_solver, 2 * 1.0, 2 * 0.0000, 200.0f);
}

uint8_t MPU6050_init(MPU6050_imu* obj) {
    MPU_Write_Byte(obj, MPU_PWR_MGMT1_REG, 0x80);
    bsp_delay_ms(100);
    MPU_Write_Byte(obj, MPU_PWR_MGMT1_REG, 0X00);
    bsp_delay_ms(50);
    MPU_Set_Gyro_Fsr(obj, 3);
    MPU_Set_Accel_Fsr(obj, 1);
    MPU_Set_Rate(obj, 200);
    MPU_Write_Byte(obj, MPU_INT_EN_REG, 0X00);     //关闭所有中断
    MPU_Write_Byte(obj, MPU_USER_CTRL_REG, 0X00);  // I2C主模式关闭
    MPU_Write_Byte(obj, MPU_FIFO_EN_REG, 0X00);    //关闭FIFO
    MPU_Write_Byte(obj, MPU_INTBP_CFG_REG, 0X80);  // INT引脚低电平有效
	MPU_Write_Byte(obj, MPU_INT_EN_REG,0x00);      //关闭中断
	MPU_Write_Byte(obj, MPU_PWR_MGMT1_REG, 0X01);
    uint8_t res = MPU_Read_Byte(obj, MPU_DEVICE_ID_REG);
    if (res == obj->config.addr) {
        MPU_Write_Byte(obj, MPU_PWR_MGMT1_REG, 0X01);  //设置CLKSEL,PLL X轴为参考
        MPU_Write_Byte(obj, MPU_PWR_MGMT2_REG, 0X00);  //加速度与陀螺仪都工作
        MPU_Set_Rate(obj, 200);                        //设置采样率为100Hz
        return 0;
    } else {
        bsp_delay_ms(10);
        return 1;
    }
}

short raw_g[3], raw_a[3];
uint8_t readBuff[6];
uint8_t flag = 0;
void MPU6050_Update(MPU6050_imu* obj) {
    MPU_Get_Gyroscope(obj, &raw_g[0], &raw_g[1], &raw_g[2]);
    MPU_Get_Accelerometer(obj, &raw_a[0], &raw_a[1], &raw_a[2]);
	obj->temp = MPU_Get_Temperature(obj);
    obj->data.gyro[0] = raw_g[0] * MPU6050_GYRO_SEN2;
    obj->data.gyro[1] = raw_g[1] * MPU6050_GYRO_SEN2;
    obj->data.gyro[2] = raw_g[2] * MPU6050_GYRO_SEN2;
	
    obj->data.accel[0] = raw_a[0] * MPU6050_ACCEL_SEN1;
    obj->data.accel[1] = raw_a[1] * MPU6050_ACCEL_SEN1;
    obj->data.accel[2] = raw_a[2] * MPU6050_ACCEL_SEN1;
    if (!obj->bias_init_success) {
        static uint16_t init_count = 0;
        ++init_count;
        if (init_count < MPU6050_BIAS_INIT_DISCARD)
            return;
        else if (init_count <
                 MPU6050_BIAS_INIT_COUNT + MPU6050_BIAS_INIT_DISCARD) {
            obj->gyrobias[0] += obj->data.gyro[0];
            obj->gyrobias[1] += obj->data.gyro[1];
            obj->gyrobias[2] += obj->data.gyro[2];
        } else if (init_count ==
                   MPU6050_BIAS_INIT_COUNT + MPU6050_BIAS_INIT_DISCARD) {
            obj->gyrobias[0] /= MPU6050_BIAS_INIT_COUNT;
            obj->gyrobias[1] /= MPU6050_BIAS_INIT_COUNT;
            obj->gyrobias[2] /= MPU6050_BIAS_INIT_COUNT;
            obj->bias_init_success = 1;

        } else {
            init_count = 0;
        }
        return;
    }

    //校准静态误差
    obj->data.gyro[0] -= obj->gyrobias[0];
    obj->data.gyro[1] -= obj->gyrobias[1];
    obj->data.gyro[2] -= obj->gyrobias[2];
    obj->data.accel[0] -= obj->config.accelbias[0];
    obj->data.accel[1] -= obj->config.accelbias[1];
    obj->data.accel[2] -= obj->config.accelbias[2];

    // Mahony算法姿态解算
    MahonyAHRS_update(&obj->mahony_solver, obj->data.gyro[0], obj->data.gyro[1],
                      obj->data.gyro[2], obj->data.accel[0], obj->data.accel[1],
                      obj->data.accel[2]);
    memcpy(obj->data.euler, obj->mahony_solver.euler, sizeof(float) * 3);
    obj->data.euler_deg[0] = obj->data.euler[0] * RAD2DEG;
    obj->data.euler_deg[1] = obj->data.euler[1] * RAD2DEG;
    obj->data.euler_deg[2] = obj->data.euler[2] * RAD2DEG;

    static float last_yaw = 0;
    if ((obj->data.euler[0] - last_yaw) > 3.141592) obj->data.round--;
    if ((obj->data.euler[0] - last_yaw) < -3.141592) obj->data.round++;
    obj->data.yaw = obj->data.euler[0] + obj->data.round * 3.1415926*2;
    last_yaw = obj->data.euler[0];
}



// IIC连续写
uint8_t MPU_Write_Len(MPU6050_imu* obj, uint8_t reg, uint8_t len, uint8_t* buf) {
    BSP_I2C_Mem_Write(0, obj->write_addr, reg, BSP_I2C_MEMADD_SIZE_8BIT, buf, len, 0xfff);
    //bsp_delay_ms(1);
    return 0;
}
// IIC连续读
// addr:器件地址
// reg:要读取的寄存器地址
// len:要读取的长度
// buf:读取到的数据存储区
// 返回值:0,正常
// 其他,错误代码
uint8_t MPU_Read_Len(MPU6050_imu* obj, uint8_t reg, uint8_t len, uint8_t* buf) {
    BSP_I2C_Mem_Read(obj->config.bsp_i2c_index, obj->read_addr, reg, BSP_I2C_MEMADD_SIZE_8BIT, buf, len, 0xfff);
    //bsp_delay_ms(1);
    return 0;
}
// IIC写一个字节
// reg:寄存器地址
// data:数据
// 返回值:0,正常
//     其他,错误代码
uint8_t MPU_Write_Byte(MPU6050_imu* obj, uint8_t reg, uint8_t data) {
    unsigned char W_Data = 0;

    W_Data = data;
    BSP_I2C_Mem_Write(obj->config.bsp_i2c_index, obj->write_addr, reg, BSP_I2C_MEMADD_SIZE_8BIT, &W_Data, 1, 0xfff);
    //bsp_delay_ms(1);

    return 0;
}
// IIC读一个字节
// reg:寄存器地址
//返回值:读到的数据
uint8_t MPU_Read_Byte(MPU6050_imu* obj, uint8_t reg) {
    unsigned char R_Data = 0;
    BSP_I2C_Mem_Read(obj->config.bsp_i2c_index, obj->read_addr, reg, BSP_I2C_MEMADD_SIZE_8BIT, &R_Data, 1, 0xfff);
    //bsp_delay_ms(1);
    return R_Data;
}

//设置MPU6050陀螺仪传感器满量程范围
// fsr:0,±250dps;1,±500dps;2,±1000dps;3,±2000dps
//返回值:0,设置成功
//    其他,设置失败
uint8_t MPU_Set_Gyro_Fsr(MPU6050_imu* obj, uint8_t fsr) {
    return MPU_Write_Byte(obj, MPU_GYRO_CFG_REG, fsr << 3);  //设置陀螺仪满量程范围
}
//设置MPU6050加速度传感器满量程范围
// fsr:0,±2g;1,±4g;2,±8g;3,±16g
//返回值:0,设置成功
//    其他,设置失败
uint8_t MPU_Set_Accel_Fsr(MPU6050_imu* obj, uint8_t fsr) {
    return MPU_Write_Byte(obj, MPU_ACCEL_CFG_REG, fsr << 3);  //设置加速度传感器满量程范围
}
//设置MPU6050的数字低通滤波器
// lpf:数字低通滤波频率(Hz)
//返回值:0,设置成功
//    其他,设置失败
uint8_t MPU_Set_LPF(MPU6050_imu* obj, uint16_t lpf) {
    uint8_t data = 0;
    if (lpf >= 188)
        data = 1;
    else if (lpf >= 98)
        data = 2;
    else if (lpf >= 42)
        data = 3;
    else if (lpf >= 20)
        data = 4;
    else if (lpf >= 10)
        data = 5;
    else
        data = 6;
    return MPU_Write_Byte(obj, MPU_CFG_REG, data);  //设置数字低通滤波器
}
//设置MPU6050的采样率(假定Fs=1KHz)
// rate:4~1000(Hz)
//返回值:0,设置成功
//    其他,设置失败
uint8_t MPU_Set_Rate(MPU6050_imu* obj, uint16_t rate) {
    uint8_t data;
    if (rate > 1000) rate = 1000;
    if (rate < 4) rate = 4;
    data = 1000 / rate - 1;
    data = MPU_Write_Byte(obj, MPU_SAMPLE_RATE_REG, data);
    //设置数字低通滤波器
    //自动设置LPF为采样率的一半
    return MPU_Set_LPF(obj, rate / 2);
}

//得到温度值
//返回值:温度值(扩大了100倍)
float MPU_Get_Temperature(MPU6050_imu* obj) {
    unsigned char buf[2];
    short raw;
    float temp;

    MPU_Read_Len(obj, MPU_TEMP_OUTH_REG, 2, buf);
    raw = (buf[0] << 8) | buf[1];
    temp = (36.53 + ((double)raw) / 340) * 100;
    //  temp = (long)((35 + (raw / 340)) * 65536L);
    return temp / 100.0f;
}
//得到陀螺仪值(原始值)
// gx,gy,gz:陀螺仪x,y,z轴的原始读数(带符号)
//返回值:0,成功
//    其他,错误代码
uint8_t MPU_Get_Gyroscope(MPU6050_imu* obj, short* gx, short* gy, short* gz) {
    uint8_t buf[6], res;
    res = MPU_Read_Len(obj, MPU_GYRO_XOUTH_REG, 6, buf);
    if (res == 0) {
        *gx = ((uint16_t)buf[0] << 8) | buf[1];
        *gy = ((uint16_t)buf[2] << 8) | buf[3];
        *gz = ((uint16_t)buf[4] << 8) | buf[5];
    }
    return res;
}
//得到加速度值(原始值)
// gx,gy,gz:陀螺仪x,y,z轴的原始读数(带符号)
//返回值:0,成功
//    其他,错误代码
uint8_t MPU_Get_Accelerometer(MPU6050_imu* obj, short* ax, short* ay, short* az) {
    uint8_t buf[6], res;
    res = MPU_Read_Len(obj, MPU_ACCEL_XOUTH_REG, 6, buf);
    if (res == 0) {
        *ax = ((uint16_t)buf[0] << 8) | buf[1];
        *ay = ((uint16_t)buf[2] << 8) | buf[3];
        *az = ((uint16_t)buf[4] << 8) | buf[5];
    }
    return res;
    ;
}

