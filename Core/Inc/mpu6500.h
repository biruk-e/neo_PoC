#pragma once

#include "main.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// MPU6500 default 7 bit slave address (AD0 = 0)
#define MPU_SLAVE_ADDR (0x68) << 1 // must be shifted left by one for STM32 HAL calls

// register addresses
#define MPU_WHO_AM_I_REG 0x75
#define MPU_CONFIG_REG 0x1A
#define MPU_SMPLRT_DIV_REG 0x19
#define MPU_INT_ENABLE_REG 0x38

#define MPU_DATA_START_ADDR 0x3B


typedef struct
{
	int16_t accel_x;
	int16_t accel_y;
	int16_t accel_z;
	int16_t die_temp;
	int16_t gyro_x;
	int16_t gyro_y;
	int16_t gyro_z;
} mpu6500_data_t;

typedef struct
{
	I2C_HandleTypeDef* hi2c;
	uint8_t dma_rx_buf[14];
	mpu6500_data_t data;
	volatile bool dma_busy;
	volatile bool data_ready;
} mpu6500_t;

/**
 * @brief Configures MPU6500 settings
 * @param mpu: a pointer to an instance of mpu6500_t
 * @param hi2c: a pointer to an instance of I2C_HandleTypeDef
 * @retval HAL_ERROR or HAL_OK based on failure or success
 */
HAL_StatusTypeDef mpu6500_init(mpu6500_t* mpu, I2C_HandleTypeDef* hi2c);

/**
 * @brief Triggers DMA driven I2C reading of MPU6500 data
 * @note Called from HAL_GPIO_EXTI_Callback in main.c when INT pin of MPU6500 rises
 * @param mpu: a pointer to an instance of mpu6500_t
 * @retval None
 */
void mpu6500_on_gpio_interrupt(mpu6500_t* mpu);

/**
 * @brief Processes big-endian data in DMA receiving buffer
 * @note Called from HAL_I2C_MemRxCpltCallback in main.c when DMA transfer successfully finishes
 * @param mpu: a pointer to an instance of mpu6500_t
 * @retval None
 */
void mpu6500_on_dma_complete(mpu6500_t* mpu);

#ifdef __cplusplus
}
#endif
