#ifndef _MPU6050_H
#define _MPU6050_H
#include "MahonyAHRS.h"
#include "mpu6050reg.h"
#include "stdint.h"

typedef struct imu_data_t {
    float accel[3];
    float gyro[3];
    float euler[3];
    float euler_deg[3];
    int round;
    float yaw;
} imu_data;

typedef struct MPU6050_config_t {
    uint8_t bsp_i2c_index;
    uint16_t addr;
    float accelbias[3];
} MPU6050_config;

typedef struct MPU6050_imu_t {
    uint8_t init_error;
    uint8_t bias_init_success;
    
    imu_data data;
    float temp;
    float gyrobias[3];
    
    uint16_t read_addr;
    uint16_t write_addr;
    
    MPU6050_config config;
    MahonyAHRS mahony_solver;
} MPU6050_imu;


void MPU6050_Create(MPU6050_imu *obj, MPU6050_config* config);
void MPU6050_Update(MPU6050_imu* obj);
#endif
