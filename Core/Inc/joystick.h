#pragma once

#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

#define JOYSTICK_ADC_CHANNELS 2 // we are reading from 2 adc channels
#define JOYSTICK_ADC_UNREACHABLE_BAND 163.8f // upper region of ADC voltage that can't be reached (with current setup)
#define JOYSTICK_ADC_MAX_RAW 4095.0f // can be used for conversion math later on (float to prevent unwanted rounding down)
#define JOYSTICK_DEFAULT_DEADZONE 150 // band in adc readings in which any fluctuation is ignored
#define JOYSTICK_DEFAULT_MIDPOINT 2047 // default midpoint of adc range (4095/2)

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
	volatile float x_norm;
	volatile float y_norm;

	// raw ADC values that can be accessed if needed for debugging
	volatile uint16_t x_raw;
	volatile uint16_t y_raw;
} Joystick_t;

// public API
HAL_StatusTypeDef Joystick_Init(Joystick_t *joystick, ADC_HandleTypeDef *hadc);
void Joystick_Calibrate(Joystick_t *joystick);
void Joystick_Update(Joystick_t *joystick);

#ifdef __cplusplus
}
#endif
