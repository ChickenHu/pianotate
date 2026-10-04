#include "BSP/timer.h"
#include "state.h"
volatile uint32_t Timer = 0;
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  if (htim->Instance == TIM4) {
    Timer++;
    State_Update();
  }
}