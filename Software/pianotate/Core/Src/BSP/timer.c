#include "BSP/timer.h"
#include "stm32f103xb.h"
volatile uint32_t Timer = 0;
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  if (htim->Instance == TIM4) {
    Timer++;
  }
}