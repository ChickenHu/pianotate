#include "BSP/timer.h"
#include "state.h"
#include <limits.h>
volatile uint32_t Timer = 0;
volatile bool Flag1ms=false;
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  if (htim->Instance == TIM4) {
    Timer++;
    // if (Timer > 10000) {
    Flag1ms=true;
    // }
    
  }
}