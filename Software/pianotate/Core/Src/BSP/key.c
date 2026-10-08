#include "key.h"
#include "stm32f1xx_hal_gpio.h"
#include <stdint.h>

Key_HandleTypeDef KeyList[7] = {
    {
        .filterCount = 0,
        .gpioPort = GPIOA,
        .gpioPin = GPIO_PIN_0,
        .checkFlag = 0,
        .state = KEY_RELEASED,
        .keyFlag = false,
    },
    {
        .filterCount = 0,
        .gpioPort = GPIOA,
        .gpioPin = GPIO_PIN_1,
        .checkFlag = 0,
        .state = KEY_RELEASED,
        .keyFlag = false,
    },
    {
        .filterCount = 0,
        .gpioPort = GPIOA,
        .gpioPin = GPIO_PIN_2,
        .checkFlag = 0,
        .state = KEY_RELEASED,
        .keyFlag = false,
    },
    {
        .filterCount = 0,
        .gpioPort = GPIOA,
        .gpioPin = GPIO_PIN_3,
        .checkFlag = 0,
        .state = KEY_RELEASED,
        .keyFlag = false,
    },
    {
        .filterCount = 0,
        .gpioPort = GPIOA,
        .gpioPin = GPIO_PIN_4,
        .checkFlag = 0,
        .state = KEY_RELEASED,
        .keyFlag = false,
    },
    {
        .filterCount = 0,
        .gpioPort = GPIOA,
        .gpioPin = GPIO_PIN_5,
        .checkFlag = 0,
        .state = KEY_RELEASED,
        .keyFlag = false,
    },
    {
        .filterCount = 0,
        .gpioPort = GPIOA,
        .gpioPin = GPIO_PIN_6,
        .checkFlag = 0,
        .state = KEY_RELEASED,
        .keyFlag = false,
    },
};

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    uint8_t pin = 0;

    switch (GPIO_Pin)
    {
    case GPIO_PIN_0:
        pin = 0;
        break;

    case GPIO_PIN_1:
        pin = 1;
        break;

    case GPIO_PIN_2:
        pin = 2;
        break;

    case GPIO_PIN_3:
        pin = 3;
        break;

    case GPIO_PIN_4:
        pin = 4;
        break;

    case GPIO_PIN_5:
        pin = 5;
        break;

    case GPIO_PIN_6:
        pin = 6;
        break;

    default:
        return;
    }

    KeyList[pin].state =
        HAL_GPIO_ReadPin(
            KeyList[pin].gpioPort,
            KeyList[pin].gpioPin
        );

    KeyList[pin].filterCount = 0;
    KeyList[pin].checkFlag = true;
}

void Key_Update(void)
{
    for (uint8_t pin = 0; pin < 7; pin++)
    {
        if (!KeyList[pin].checkFlag)
        {
            continue;
        }

        GPIO_PinState current =
            HAL_GPIO_ReadPin(
                KeyList[pin].gpioPort,
                KeyList[pin].gpioPin
            );

        /*
         * 当前电平已经和目标状态不同，
         * 说明按键仍在抖动。
         */
        if (current != KeyList[pin].state)
        {
            KeyList[pin].filterCount = 0;
            continue;
        }

        /*
         * 当前电平与目标状态一致，
         * 连续稳定计数。
         */
        KeyList[pin].filterCount++;

        if (KeyList[pin].filterCount >= KEY_DEBOUNCE)
        {
            /*
             * 确认状态改变
             */
            KeyList[pin].keyFlag = (KeyList[pin].state == KEY_PRESSED);

            KeyList[pin].filterCount = 0;
            KeyList[pin].checkFlag = false;
        }
    }
}