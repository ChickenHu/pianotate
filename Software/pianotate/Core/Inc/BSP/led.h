#include "gpio.h"
#include "oled.h"
#include "stm32f103xb.h"
#include "stm32f1xx_hal_gpio.h"
typedef struct{
    GPIO_TypeDef *gpio;
    uint16_t pin;
} LED_HandleTypeDef;
LED_HandleTypeDef LED[7]={
    {.gpio=GPIOB,.pin=GPIO_PIN_12},
    {.gpio=GPIOB,.pin=GPIO_PIN_13},
    {.gpio=GPIOB,.pin=GPIO_PIN_14},
    {.gpio=GPIOB,.pin=GPIO_PIN_15},
    {.gpio=GPIOA,.pin=GPIO_PIN_8},
    {.gpio=GPIOA,.pin=GPIO_PIN_9},
    {.gpio=GPIOA,.pin=GPIO_PIN_10}
};