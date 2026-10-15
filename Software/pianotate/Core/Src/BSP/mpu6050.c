#include "BSP/mpu6050.h"

/* 驱动使用的全局设备对象。 */
MPU6050_t MPU6050_Device;

/* 这些变量会被主循环和应用层读取，因此声明为 volatile。 */
volatile bool MPU6050_Ready = false;
volatile bool MPU6050_AccelDetectedFlag = false;
volatile bool MPU6050_RotatingFlag = false;

/* 初始化失败原因：1=无应答，2=WHO_AM_I读取失败，3=ID错误，4=写寄存器失败。 */
volatile uint8_t MPU6050_ErrorCode = 0;
/* 保存实际读到的 WHO_AM_I，便于判断板上芯片是否真的是 MPU6050。 */
volatile uint8_t MPU6050_WhoAmI = 0;

static HAL_StatusTypeDef MPU6050_WriteReg(MPU6050_t *dev,
                                          uint8_t reg,
                                          uint8_t value)
{
    /* MPU6050 寄存器地址是 8 位，因此使用 I2C_MEMADD_SIZE_8BIT。 */
    return HAL_I2C_Mem_Write(dev->hi2c,
                             dev->address,
                             reg,
                             I2C_MEMADD_SIZE_8BIT,
                             &value,
                             1,
                             5);
}

static HAL_StatusTypeDef MPU6050_ReadRegs(MPU6050_t *dev,
                                          uint8_t reg,
                                          uint8_t *data,
                                          uint16_t length)
{
    /* 从指定寄存器开始连续读取 length 个字节。 */
    return HAL_I2C_Mem_Read(dev->hi2c,
                            dev->address,
                            reg,
                            I2C_MEMADD_SIZE_8BIT,
                            data,
                            length,
                            5);
}

HAL_StatusTypeDef MPU6050_Init(MPU6050_t *dev,
                               I2C_HandleTypeDef *hi2c,
                               uint16_t address)
{
    uint8_t who_am_i = 0;

    /* 保存本次初始化使用的 I2C 外设和设备地址。 */
    dev->hi2c = hi2c;
    dev->address = address;
    dev->accel_detected = false;
    dev->rotating = false;
    dev->quiet_ms = 0;

    MPU6050_Ready = false;
    MPU6050_AccelDetectedFlag = false;
    MPU6050_RotatingFlag = false;
    MPU6050_ErrorCode = 0;
    MPU6050_WhoAmI = 0;

    /* 先发送设备地址，确认 MPU6050 能够应答。 */
    if (HAL_I2C_IsDeviceReady(dev->hi2c, dev->address, 3, 100) != HAL_OK)
    {
        MPU6050_ErrorCode = 1; /* No I2C acknowledge. */
        return HAL_ERROR;
    }

    /* 6050 返回 0x68；同寄存器兼容的 6500 返回 0x70。 */
    if (MPU6050_ReadRegs(dev, MPU6050_REG_WHO_AM_I, &who_am_i, 1) != HAL_OK)
    {
        MPU6050_ErrorCode = 2; /* WHO_AM_I read failed. */
        return HAL_ERROR;
    }

    /* 先保存原始值，即使后面的型号检查失败也可以从 RAM 读出。 */
    MPU6050_WhoAmI = who_am_i;

    if (who_am_i != MPU6050_WHO_AM_I_VALUE &&
        who_am_i != MPU6500_WHO_AM_I_VALUE)
    {
        MPU6050_ErrorCode = 3; /* Device responded with an unexpected ID. */
        return HAL_ERROR;
    }

    /* 清除睡眠位，唤醒芯片并使用内部时钟。 */
    if (MPU6050_WriteReg(dev, MPU6050_REG_PWR_MGMT_1, 0x00u) != HAL_OK)
    {
        MPU6050_ErrorCode = 4; /* Register write failed. */
        return HAL_ERROR;
    }

    /*
     * DLPF 开启后内部采样率为 1 kHz：
     * 1 kHz / (1 + 9) = 100 Hz。
     */
    if (MPU6050_WriteReg(dev, MPU6050_REG_SMPLRT_DIV, 9u) != HAL_OK ||
        MPU6050_WriteReg(dev, MPU6050_REG_CONFIG, 3u) != HAL_OK ||
        /* FS_SEL=1：陀螺仪量程为 ±500 dps，灵敏度为 65.5 LSB/dps。 */
        MPU6050_WriteReg(dev, MPU6050_REG_GYRO_CONFIG, 0x08u) != HAL_OK ||
        /* AFS_SEL=0：加速度量程为 ±2 g，灵敏度为 16384 LSB/g。 */
        MPU6050_WriteReg(dev, MPU6050_REG_ACCEL_CONFIG, 0x00u) != HAL_OK)
    {
        MPU6050_ErrorCode = 4; /* Register write failed. */
        return HAL_ERROR;
    }

    /* 所有初始化步骤成功，允许主循环读取传感器。 */
    MPU6050_Ready = true;
    return HAL_OK;
}

HAL_StatusTypeDef MPU6050_Read(MPU6050_t *dev)
{
    /* 0x3B 开始的 14 字节依次为：加速度 6 字节、温度 2 字节、陀螺仪 6 字节。 */
    uint8_t data[14];

    if (MPU6050_ReadRegs(dev,
                         MPU6050_REG_ACCEL_XOUT_H,
                         data,
                         sizeof(data)) != HAL_OK)
    {
        /* 保持初始化状态，允许主循环在瞬时总线错误后重试。 */
        MPU6050_ErrorCode = 5;
        MPU6050_AccelDetectedFlag = false;
        MPU6050_RotatingFlag = false;
        dev->accel_detected = false;
        dev->rotating = false;
        dev->quiet_ms = 0;
        return HAL_ERROR;
    }

    /* MPU6050 数据按高字节在前传输，需要组合成有符号 16 位整数。 */
    dev->ax = (int16_t)((((uint16_t)data[0]) << 8) | data[1]);
    dev->ay = (int16_t)((((uint16_t)data[2]) << 8) | data[3]);
    dev->az = (int16_t)((((uint16_t)data[4]) << 8) | data[5]);
    dev->gx = (int16_t)((((uint16_t)data[8]) << 8) | data[9]);
    dev->gy = (int16_t)((((uint16_t)data[10]) << 8) | data[11]);
    dev->gz = (int16_t)((((uint16_t)data[12]) << 8) | data[13]);

    MPU6050_ErrorCode = 0;
    return HAL_OK;
}

static int32_t MPU6050_Abs16(int16_t value)
{
    /* 先扩展为 32 位，避免对 -32768 求绝对值时溢出。 */
    int32_t result = value;
    return result < 0 ? -result : result;
}

void MPU6050_UpdateFlags(MPU6050_t *dev, uint16_t sample_period_ms)
{
    int32_t gyro_max;
    int32_t g_abs;
    int64_t accel_squared;
    const int64_t accel_low = (int64_t)13107 * 13107;  /* 0.8 g */
    const int64_t accel_high = (int64_t)19661 * 19661; /* 1.2 g */

    accel_squared =
        (int64_t)dev->ax * dev->ax +
        (int64_t)dev->ay * dev->ay +
        (int64_t)dev->az * dev->az;

    /*
     * 静止时 |a| 约等于 1 g。
     * 这里使用平方值，避免调用 sqrtf；超出 0.8 g～1.2 g 认为发生动态加速度。
     */
    dev->accel_detected =
        (accel_squared < accel_low || accel_squared > accel_high);
    MPU6050_AccelDetectedFlag = dev->accel_detected;

    /*
     * 同时判断 GX、GY、GZ 三个轴向，任意一个角速度过大即认为在旋转。
     * 取三者绝对值的最大值作为判定依据。
     */
    gyro_max = MPU6050_Abs16(dev->gx);
    g_abs = MPU6050_Abs16(dev->gy);
    if (g_abs > gyro_max)
        gyro_max = g_abs;
    g_abs = MPU6050_Abs16(dev->gz);
    if (g_abs > gyro_max)
        gyro_max = g_abs;

    /*
     * 当前陀螺仪量程为 ±500 dps，灵敏度为 65.5 LSB/dps：
     * 30 dps 约等于 2000 raw，作为开始旋转阈值（原 50 dps/3275）；
     * 15 dps 约等于 1000 raw，作为停止旋转阈值（原 25 dps/1638）。
     */
    if (!dev->rotating && gyro_max > 2000)
    {
        dev->rotating = true;
        dev->quiet_ms = 0;
    }
    else if (dev->rotating)
    {
        if (gyro_max < 1000)
        {
            if (dev->quiet_ms < 1000)
                dev->quiet_ms += sample_period_ms;

            /* 低于停止阈值持续 100 ms 后才清除 Flag，防止抖动。 */
            if (dev->quiet_ms >= 100)
            {
                dev->rotating = false;
                dev->quiet_ms = 0;
            }
        }
        else
        {
            dev->quiet_ms = 0;
        }
    }

    MPU6050_RotatingFlag = dev->rotating;
}
