#include "joystick.h"
#include "main.h"
#include <stdlib.h>

/**
 * @brief Initializes a joystick instance, binds hardware handle,
 * initializes joystick parameters and dma buffer, and starts DMA sampling
 * @param joystick: pointer to an instance of Joystick_t
 * @param hadc: pointer to the ADC handle
 * @retval HAL_StatusTypeDef: HAL Status of the DMA start operation
 */
HAL_StatusTypeDef Joystick_Init(Joystick_t *joystick, ADC_HandleTypeDef *hadc) {
	// returns an error flag if either Joystick_t or ADC_HandleTypeDef handles have not been initialized in main
	if (joystick == NULL || hadc == NULL) {
		return HAL_ERROR;
	}

	joystick->hadc = hadc; // bind hardware handle

	// set default midpoints and deadzones
	joystick->calib.center_x = JOYSTICK_DEFAULT_MIDPOINT;
	joystick->calib.center_y = JOYSTICK_DEFAULT_MIDPOINT;
	joystick->calib.deadzone = JOYSTICK_DEFAULT_DEADZONE;

	// initialize x_norm and y_norm to 0.0
	joystick->x_norm = 0.0f;
	joystick->y_norm = 0.0f;

	// initialize x_raw and y_raw to 0
	joystick->x_raw = 0;
	joystick->y_raw = 0;

	// initialize dma_buffer[0] and dma_buffer[1] to 0
	joystick->raw_dma[0] = 0;
	joystick->raw_dma[1] = 0;

	// start ADC conversion in DMA mode and return HAL status
	return HAL_ADC_Start_DMA(joystick->hadc, (uint32_t*)joystick->raw_dma, JOYSTICK_ADC_CHANNELS);
}

/**
 * @brief Samples resting ADC readings and computes avg to set dynamic zero points for x and y axes
 * @param joystick: pointer to an instance of Joystick_t
 */
void Joystick_Calibrate(Joystick_t *joystick) {
	// immediately returns if joystick pointer is NULL
	if (joystick == NULL) {
		return;
	}

	// sample ADC readings, compute average, and set as the center for each axis (VRy = x, and VRx = y for current orientation of breakout)
	uint32_t sum_x = 0;
	uint32_t sum_y = 0;
	const uint16_t samples = 32;

	for (uint16_t i = 0; i < samples; i++) {
		sum_x += joystick->raw_dma[1];
		sum_y += joystick->raw_dma[0];
		HAL_Delay(2);
	}

	joystick->calib.center_x = (uint16_t)(sum_x / samples);
	joystick->calib.center_y = (uint16_t)(sum_y / samples);
}

/**
 * @brief Reads latest DMA values, calculates axis delta, applies deadzone, and normalizes output to [-1.0f, +1.0f]
 * @param joystick: pointer to an instance of Joystick_t
 */
void Joystick_Update(Joystick_t *joystick) {
	// immediately returns if joystick pointer is NULL
		if (joystick == NULL) {
			return;
		}

	// fetch raw values from DMA buffer (VRy = x, and VRx = y for current orientation of breakout)
	joystick->x_raw = joystick->raw_dma[1];
	joystick->y_raw = joystick->raw_dma[0];

	// calculate change in x and y from resting center
	int32_t delta_x = (int32_t)joystick->x_raw - (int32_t)joystick->calib.center_x; // delta_x = x_raw - center_x
	int32_t delta_y = (int32_t)joystick->y_raw - (int32_t)joystick->calib.center_y; // delta_y = y_raw - center_y

	// x-axis normalization
	if (labs(delta_x) <= joystick->calib.deadzone) {
	        joystick->x_norm = 0.0f;
	    } else if (delta_x > 0) {
	        float effective_delta = (float)delta_x - (float)joystick->calib.deadzone;
	        float max_span = JOYSTICK_ADC_MAX_RAW - JOYSTICK_ADC_UNREACHABLE_BAND - (float)joystick->calib.center_x - (float)joystick->calib.deadzone;
	        joystick->x_norm = (max_span >= 0.0f) ? (effective_delta / max_span) : 0.0f;
	    } else {
	        float effective_delta = (float)delta_x + (float)joystick->calib.deadzone;
	        float min_span = (float)joystick->calib.center_x - (float)joystick->calib.deadzone;
	        joystick->x_norm = (min_span >= 0.0f) ? (effective_delta / min_span) : 0.0f;
	    }

	// y-axis normalization
	if (labs(delta_y) <= joystick->calib.deadzone) {
	        joystick->y_norm = 0.0f;
	    } else if (delta_y > 0) {
	        float effective_delta = (float)delta_y - (float)joystick->calib.deadzone;
	        float max_span = JOYSTICK_ADC_MAX_RAW - JOYSTICK_ADC_UNREACHABLE_BAND - (float)joystick->calib.center_y - (float)joystick->calib.deadzone;
	        joystick->y_norm = (max_span >= 0.0f) ? (effective_delta / max_span) : 0.0f;
	    } else {
	        float effective_delta = (float)delta_y + (float)joystick->calib.deadzone;
	        float min_span = (float)joystick->calib.center_y - (float)joystick->calib.deadzone;
	        joystick->y_norm = (min_span >= 0.0f) ? (effective_delta / min_span) : 0.0f;
	    }

		// clamp x_norm and y_norm to be within -1.0 to +1.0 (accounts for slightly-off center midpoint during calibration)
		if (joystick->x_norm > 1.0f) {
			joystick->x_norm = 1.0f;
		} else if (joystick->x_norm < -1.0f) {
			joystick->x_norm = -1.0f;
		}

		if (joystick->y_norm > 1.0f) {
			joystick->y_norm = 1.0f;
		} else if (joystick->y_norm < -1.0f) {
			joystick->y_norm = -1.0f;
		}

}
