#include "gpio.h"
#include "stm32f1xx_hal_gpio.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct {
  GPIO_TypeDef *gpio;
  uint16_t pin;
} LED_HandleTypeDef;

extern LED_HandleTypeDef LED[7];

void LED_BreathingStart(void);
void LED_BreathingStop(void);
void LED_PWMInterrupt(void);