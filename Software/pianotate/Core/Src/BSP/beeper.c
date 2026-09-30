#include "BSP/beeper.h"
#include "stm32f1xx_hal_tim.h"
#include "tim.h"
#include <stdint.h>

void Beeper_Set(TIM_HandleTypeDef *beeper, uint32_t prescaler, uint32_t freq) {
  beeper->Instance->PSC = prescaler;
  beeper->Instance->ARR = freq;
  beeper->Instance->CCR1 = freq >> 2;
}

void Beeper_Start(TIM_HandleTypeDef *beeper) {
  HAL_TIM_PWM_Start(beeper, TIM_CHANNEL_1);
}

void Beeper_Stop(TIM_HandleTypeDef *beeper) {
  HAL_TIM_PWM_Stop(beeper, TIM_CHANNEL_1);
}