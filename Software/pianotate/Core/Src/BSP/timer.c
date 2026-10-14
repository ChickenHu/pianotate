#include "BSP/timer.h"
#include "BSP/led.h"
#include <limits.h>
volatile uint32_t Timer = 0;
volatile bool Flag1ms=false;
static uint8_t TimerSubTick;
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  if (htim->Instance == TIM4) {
    LED_PWMInterrupt();
    TimerSubTick++;
    if (TimerSubTick >= 10u) {
      TimerSubTick = 0;
      Timer++;
      Flag1ms = true;
    }
  }
}