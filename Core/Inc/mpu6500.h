#pragma once

#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MPU6500_I2C_ADDR         (0x68 << 1) // default I2C address (shifted for HAL)
#define MPU6500_REG_WHO_AM_I     0x75
#define MPU6500_REG_PWR_MGMT_1   0x6B
#define MPU6500_REG_INT_ENABLE   0x38
#define MPU6500_DATA_START_ADDR  0x3B // start register for 14 continuous bytes

#define MPU6500_REG_SMPLRT_DIV   0x19
#define MPU6500_REG_CONFIG       0x1A

typedef struct {
    volatile int16_t accel_x;
    volatile int16_t accel_y;
    volatile int16_t accel_z;
    volatile int16_t temp_raw;
    volatile int16_t gyro_x;
    volatile int16_t gyro_y;
    volatile int16_t gyro_z;
} MPU6500_Data_t;

typedef struct {
    I2C_HandleTypeDef *hi2c;
    uint8_t dma_rx_buf[14]; // 6 Accel + 2 Temp + 6 Gyro bytes
    MPU6500_Data_t data;
    volatile uint8_t data_ready;
    volatile uint8_t dma_busy;
} MPU6500_t;

HAL_StatusTypeDef MPU6500_Init(MPU6500_t *dev, I2C_HandleTypeDef *hi2c);
void MPU6500_OnGpioInterrupt(MPU6500_t *dev);
void MPU6500_OnDmaComplete(MPU6500_t *dev);
void MPU6500_ProcessData(MPU6500_t *dev);

#ifdef __cplusplus
}
#endif
