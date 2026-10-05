#ifndef __KEY_H
#define __KEY_H

#include "main.h"

#define KEY1_PORT       GPIOA
#define KEY1_PIN        GPIO_PIN_1

#define KEY2_PORT       GPIOA
#define KEY2_PIN        GPIO_PIN_2

#define KEY3_PORT       GPIOA
#define KEY3_PIN        GPIO_PIN_3

#define KEY4_PORT       GPIOA
#define KEY4_PIN        GPIO_PIN_4

#define KEY5_PORT       GPIOA
#define KEY5_PIN        GPIO_PIN_5

#define KEY6_PORT       GPIOA
#define KEY6_PIN        GPIO_PIN_6

#define KEY7_PORT       GPIOA
#define KEY7_PIN        GPIO_PIN_7


#define KEY_INIT_DEF(port, pin, callback) \
{ \
  .key_state = 0, \
  .filter_cnt = 0, \
  .gpio_port = port, \
  .gpio_pin = pin, \
  .press_flag = 0, \
  .press_callback = callback \
}
//之前那个7行的定义太长了，于是我搜了下找了个可以一次性定义的（）

typedef struct
{
  uint8_t key_state;
  uint8_t filter_cnt;
  GPIO_TypeDef *gpio_port;
  uint16_t gpio_pin;
  uint8_t press_flag;
  void (*press_callback)(void);
} Key_HandleTypeDef;

extern int Key_Update();
#endif