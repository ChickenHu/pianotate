#ifndef __KEY_H
#define __KEY_H

#include "main.h"
#include "gpio.h"

typedef struct
{
  uint32_t pendingSince;
  GPIO_TypeDef *gpioPort;
  uint16_t gpioPin;
  bool checkFlag; // 消抖状态待确认
  bool keyFlag;   // 按键状态变化事件待消费
  bool pressedEventPending; // 稳定按下事件待消费
  GPIO_PinState state;
  GPIO_PinState pendingState;
} Key_HandleTypeDef;

#define KEY_PRESSED   GPIO_PIN_RESET
#define KEY_RELEASED  GPIO_PIN_SET
#define KEY_DEBOUNCE 5

extern void Key_Update(void);
extern void Key_ConsumeEvent(uint8_t key);
extern void Key_ConsumeAllEvents(void);
extern Key_HandleTypeDef KeyList[7];

#endif