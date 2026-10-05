#pragma once
#include "tim.h"
#include <stdint.h>

#define BEEPER1 htim2
#define BEEPER2 htim3

extern void Beeper_Set(TIM_HandleTypeDef *beeper, uint32_t prescaler,
                       uint32_t freq);
extern void Beeper_Start(TIM_HandleTypeDef *beeper);
extern void Beeper_Stop(TIM_HandleTypeDef *beeper);
