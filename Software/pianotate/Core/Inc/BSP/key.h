#ifndef __KEY_H
#define __KEY_H

#include "main.h"
#include "gpio.h"

typedef struct
{
  uint8_t key_state;
  uint8_t filter_cnt;
  GPIO_TypeDef *gpio_port;
  uint16_t gpio_pin;
  uint8_t press_flag;
  void (*press_callback)(void);
} Key_HandleTypeDef;





extern void Key_Update(void);
extern Key_HandleTypeDef key_list[7];

#endif