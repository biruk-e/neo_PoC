#include "mpu6500.h"

HAL_StatusTypeDef MPU6500_Init(MPU6500_t *dev, I2C_HandleTypeDef *hi2c) {
    if (dev == NULL || hi2c == NULL) return HAL_ERROR;

    dev->hi2c = hi2c;
    dev->data_ready = 0;
    dev->dma_busy = 0;

    // 1. Verify device ID
    uint8_t who_am_i = 0;
    if (HAL_I2C_Mem_Read(dev->hi2c, MPU6500_I2C_ADDR, MPU6500_REG_WHO_AM_I,
                         I2C_MEMADD_SIZE_8BIT, &who_am_i, 1, 100) != HAL_OK) {
        return HAL_ERROR;
    }
    if (who_am_i != 0x70 && who_am_i != 0x71) { // MPU6500 / MPU9250 standard IDs
        return HAL_ERROR;
    }

    // 2. Wake up MPU6500 (clear SLEEP bit)
    uint8_t pwr_mgmt = 0x00;
    if (HAL_I2C_Mem_Write(dev->hi2c, MPU6500_I2C_ADDR, MPU6500_REG_PWR_MGMT_1,
                          I2C_MEMADD_SIZE_8BIT, &pwr_mgmt, 1, 100) != HAL_OK) {
        return HAL_ERROR;
    }

    // 3. Enable Data Ready Interrupt on INT pin
    uint8_t int_enable = 0x01; // DATA_RDY_INT_EN
    if (HAL_I2C_Mem_Write(dev->hi2c, MPU6500_I2C_ADDR, MPU6500_REG_INT_ENABLE,
                          I2C_MEMADD_SIZE_8BIT, &int_enable, 1, 100) != HAL_OK) {
        return HAL_ERROR;
    }

    // 4. Configure DLPF in Register 0x1A (CONFIG)
    // Setting DLPF_CFG = 3 sets Gyro Bandwidth to ~41Hz and Internal Sample Rate to 1 kHz
    uint8_t config = 0x03;
    if (HAL_I2C_Mem_Write(dev->hi2c, MPU6500_I2C_ADDR, MPU6500_REG_CONFIG,
                          I2C_MEMADD_SIZE_8BIT, &config, 1, 100) != HAL_OK) {
        return HAL_ERROR;
    }

    // 5. Set SMPLRT_DIV to 19 (Yields 1000Hz / (1 + 19) = 50Hz ODR)
    uint8_t smplrt_div = 19;
    if (HAL_I2C_Mem_Write(dev->hi2c, MPU6500_I2C_ADDR, MPU6500_REG_SMPLRT_DIV,
                          I2C_MEMADD_SIZE_8BIT, &smplrt_div, 1, 100) != HAL_OK) {
        return HAL_ERROR;
    }

    return HAL_OK;
}

/**
 * @brief Called from HAL_GPIO_EXTI_Callback when MPU_INT goes high
 */
void MPU6500_OnGpioInterrupt(MPU6500_t *dev) {
    if (dev == NULL) return;

    // Start non-blocking DMA read of all 14 registers if DMA is idle
    if (!dev->dma_busy) {
        dev->dma_busy = 1;
        if (HAL_I2C_Mem_Read_DMA(dev->hi2c, MPU6500_I2C_ADDR, MPU6500_DATA_START_ADDR,
                             I2C_MEMADD_SIZE_8BIT, dev->dma_rx_buf, 14) != HAL_OK) {
        	dev->dma_busy = 0;
        }
    }
}

/**
 * @brief Called from HAL_I2C_MemRxCpltCallback when DMA transfer finishes
 */
void MPU6500_OnDmaComplete(MPU6500_t *dev) {
    if (dev == NULL) return;

    // Reassemble big-endian byte pairs into 16-bit signed values
    dev->data.accel_x = (int16_t)((dev->dma_rx_buf[0]  << 8) | dev->dma_rx_buf[1]);
    dev->data.accel_y = (int16_t)((dev->dma_rx_buf[2]  << 8) | dev->dma_rx_buf[3]);
    dev->data.accel_z = (int16_t)((dev->dma_rx_buf[4]  << 8) | dev->dma_rx_buf[5]);
    dev->data.temp_raw= (int16_t)((dev->dma_rx_buf[6]  << 8) | dev->dma_rx_buf[7]);
    dev->data.gyro_x  = (int16_t)((dev->dma_rx_buf[8]  << 8) | dev->dma_rx_buf[9]);
    dev->data.gyro_y  = (int16_t)((dev->dma_rx_buf[10] << 8) | dev->dma_rx_buf[11]);
    dev->data.gyro_z  = (int16_t)((dev->dma_rx_buf[12] << 8) | dev->dma_rx_buf[13]);

    dev->dma_busy = 0;
    dev->data_ready = 1; // Signal main loop that new processing can occur
}
