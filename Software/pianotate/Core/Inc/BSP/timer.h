#include "tim.h"
#include <stdint.h>

extern volatile uint32_t Timer;
extern void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);