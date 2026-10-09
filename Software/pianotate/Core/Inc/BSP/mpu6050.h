#ifndef __MPU6050_H
#define __MPU6050_H

#include "stm32f1xx_hal.h"
#include <stdbool.h>
#include <stdint.h>

/*
 * MPU6050 的 7 位 I2C 地址由 AD0 决定：
 *   AD0 接 GND -> 0x68
 *   AD0 接 3.3V -> 0x69
 * STM32 HAL 接口要求传入左移一位后的地址，因此 0x68 要写成 0xD0。
 */
#define MPU6050_ADDR_7BIT  0x68u
#define MPU6050_ADDR       (MPU6050_ADDR_7BIT << 1)

#define MPU6050_REG_SMPLRT_DIV    0x19u
#define MPU6050_REG_CONFIG        0x1Au
#define MPU6050_REG_GYRO_CONFIG   0x1Bu
#define MPU6050_REG_ACCEL_CONFIG  0x1Cu
#define MPU6050_REG_ACCEL_XOUT_H  0x3Bu
#define MPU6050_REG_PWR_MGMT_1    0x6Bu
#define MPU6050_REG_WHO_AM_I      0x75u

/* WHO_AM_I：MPU6050 为 0x68，MPU6500 为 0x70。两者的基础六轴数据寄存器兼容。 */
#define MPU6050_WHO_AM_I_VALUE    0x68u
#define MPU6500_WHO_AM_I_VALUE    0x70u

/* 保存 MPU6050 总线句柄、原始数据和应用层状态。 */
typedef struct
{
    I2C_HandleTypeDef *hi2c; /* 当前使用的 I2C 外设，例如 &hi2c1。 */
    uint16_t address;        /* HAL 格式的设备地址，例如 0xD0。 */

    /* 加速度和角速度的 16 位原始值。 */
    int16_t ax;
    int16_t ay;
    int16_t az;
    int16_t gx;
    int16_t gy;
    int16_t gz;

    bool accel_detected; /* 加速度合成值明显偏离 1 g。 */
    bool rotating;       /* 当前是否处于旋转状态。 */
    uint16_t quiet_ms;   /* 低角速度持续时间，用于清除 rotating。 */
} MPU6050_t;

/* 全局设备对象和应用层可直接读取的状态 Flag。 */
extern MPU6050_t MPU6050_Device;
extern volatile bool MPU6050_Ready;
extern volatile bool MPU6050_AccelDetectedFlag;
extern volatile bool MPU6050_RotatingFlag;
extern volatile uint8_t MPU6050_ErrorCode;
/* 初始化时从传感器读取到的 WHO_AM_I 值，便于用调试器确认实际芯片。 */
extern volatile uint8_t MPU6050_WhoAmI;

/* 检测设备、唤醒芯片并配置采样参数。 */
HAL_StatusTypeDef MPU6050_Init(MPU6050_t *dev,
                               I2C_HandleTypeDef *hi2c,
                               uint16_t address);

/* 从 0x3B 开始连续读取加速度、温度和陀螺仪原始数据。 */
HAL_StatusTypeDef MPU6050_Read(MPU6050_t *dev);

/* 根据最近一次采样更新加速度检测和旋转状态。 */
void MPU6050_UpdateFlags(MPU6050_t *dev, uint16_t sample_period_ms);

#endif
