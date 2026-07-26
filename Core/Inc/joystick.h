#pragma once

#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

#define JOYSTICK_ADC_CHANNELS 2 // we are reading from 2 adc channels
#define JOYSTICK_ADC_MAX_RAW 4095.0f // can be used for conversion math later on (float to prevent unwanted rounding down)
#define JOYSTICK_DEFAULT_DEADZONE 150 // band in adc readings in which any fluctuation is ignored

// typedef struct for holding calibration values
typedef struct {
	uint16_t center_x;
	uint16_t center_y;
	uint16_t deadzone;
} Joystick_Calib_t;

// typedef struct that represents the joystick and its state

typedef struct {
	ADC_HandleTypeDef *hadc; // pointer to ADC peripheral
	volatile uint16_t raw_dma[JOYSTICK_ADC_CHANNELS]; // target DMA buffer

	Joystick_Calib_t calib; // calibration parameters

	// normalized values that are exposed to application layer
	float x_norm;
	float y_norm;

	// raw ADC values that can be accessed if needed for debugging
	uint16_t x_raw;
	uint16_t y_raw;
}Joystick_t;

// public API
HAL_StatusTypeDef Joystick_Init(Joystick_t *joystick, ADC_HandleTypeDef *hadc);
void Joystick_Calibrate(Joystick_t *joystick);
void Joystick_Update(Joystick_t *joystick);

#ifdef __cplusplus
}
#endif
