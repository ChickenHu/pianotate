#ifndef __KEY_H
#define __KEY_H

#include "main.h"
#include "gpio.h"

typedef struct
{
  uint32_t pendingSince;
  GPIO_TypeDef *gpioPort;
  uint16_t gpioPin;
  bool checkFlag;
  bool keyFlag; // 按键状态变化待处理标志
  GPIO_PinState state;
  GPIO_PinState pendingState;
} Key_HandleTypeDef;

#define KEY_PRESSED   GPIO_PIN_RESET
#define KEY_RELEASED  GPIO_PIN_SET
#define KEY_DEBOUNCE 5

extern void Key_Update(void);
extern Key_HandleTypeDef KeyList[7];

#endif