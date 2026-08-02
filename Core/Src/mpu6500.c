#include "mpu6500.h"

HAL_StatusTypeDef mpu6500_init(mpu6500_t* mpu, I2C_HandleTypeDef* hi2c) {
	if (mpu == NULL || hi2c == NULL) return HAL_ERROR; // guard against null pointers

	mpu->hi2c = hi2c; // bind mpu->hi2c to the passed i2c handle
	mpu->dma_busy = false;
	mpu->data_ready = false;

	// read WHO_AM_I register contents into checkVal
	uint8_t checkVal = 0;
	if (HAL_I2C_Mem_Read(hi2c, MPU_SLAVE_ADDR, MPU_WHO_AM_I_REG, 1, &checkVal, 1, 100) != HAL_OK) {
		return HAL_ERROR;
	}
	// check if checkVal holds a valid ID
	if (checkVal != 0x70 && checkVal != 0x71) {
		return HAL_ERROR;
	}

	// write to DLPF_CONFIG section of CONFIG register to reduce internal sampling rate to 1 kHz
	uint8_t dlpf_data = 0x03; // sets internal sampling rate = 1 kHz
	if (HAL_I2C_Mem_Write(hi2c, MPU_SLAVE_ADDR, MPU_CONFIG_REG, 1, &dlpf_data, 1, 100) != HAL_OK) {
		return HAL_ERROR;
	}

	// write to SMPLRT_DIV register to reduce sampling rate to 50Hz
	uint8_t smplrt_div_data = 19; // sampling rate = internal sampling rate / 1 + smplrt_div
	if (HAL_I2C_Mem_Write(hi2c, MPU_SLAVE_ADDR, MPU_SMPLRT_DIV_REG, 1, &smplrt_div_data, 1, 100) != HAL_OK) {
			return HAL_ERROR;
	}

	// write to INT_ENABLE register to propogate data ready interrupts to INT pin
	uint8_t int_enable_data = 0x01; // sets RAW_RDY_EN bit of INT_ENABLE register to 1
	if (HAL_I2C_Mem_Write(hi2c, MPU_SLAVE_ADDR, MPU_INT_ENABLE_REG, 1, &int_enable_data, 1, 100) != HAL_OK) {
			return HAL_ERROR;
	}

	return HAL_OK;
}

void mpu6500_on_gpio_interrupt(mpu6500_t* mpu) {
	if (mpu == NULL) return; // guard against null pointer

	// if dma is free -> start DMA transfer
	if (mpu->dma_busy != true) {
		HAL_I2C_Mem_Read_DMA(mpu->hi2c, MPU_SLAVE_ADDR, MPU_DATA_START_ADDR, 1, mpu->dma_rx_buf, 14);
		mpu->dma_busy = true; // set dma_busy flag to true
	}
}

void mpu6500_on_dma_complete(mpu6500_t* mpu) {
	if (mpu == NULL) return; // guard against null pointer

	mpu->data.accel_x = (int16_t) ( (mpu->dma_rx_buf[0] << 8) | (mpu->dma_rx_buf[1]) );
	mpu->data.accel_y = (int16_t) ( (mpu->dma_rx_buf[2] << 8) | (mpu->dma_rx_buf[3]) );
	mpu->data.accel_z = (int16_t) ( (mpu->dma_rx_buf[4] << 8) | (mpu->dma_rx_buf[5]) );
	mpu->data.die_temp = (int16_t) ( (mpu->dma_rx_buf[6] << 8) | (mpu->dma_rx_buf[7]) );
	mpu->data.gyro_x = (int16_t) ( (mpu->dma_rx_buf[8] << 8) | (mpu->dma_rx_buf[9]) );
	mpu->data.gyro_y = (int16_t) ( (mpu->dma_rx_buf[10] << 8) | (mpu->dma_rx_buf[11]) );
	mpu->data.gyro_z = (int16_t) ( (mpu->dma_rx_buf[12] << 8) | (mpu->dma_rx_buf[13]) );

	mpu->dma_busy = false; // allow DMA transfer to be started on next ISR
	mpu->data_ready = true; // lets main loop know that data is ready for use
}


