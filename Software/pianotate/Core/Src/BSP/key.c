#include "key.h"
#include "stm32f1xx_hal_gpio.h"
#include "timer.h"
#include <stdint.h>

Key_HandleTypeDef KeyList[7] = {
    
        
    {
        .pendingSince = 0,
        .gpioPort = GPIOA,
        .gpioPin = GPIO_PIN_1,
        .checkFlag = 0,
        .state = KEY_RELEASED,
        .pendingState = KEY_RELEASED,
        .keyFlag = false,
        .pressedEventPending = false,
    },
    {
        .pendingSince = 0,
        .gpioPort = GPIOA,
        .gpioPin = GPIO_PIN_2,
        .checkFlag = 0,
        .state = KEY_RELEASED,
        .pendingState = KEY_RELEASED,
        .keyFlag = false,
        .pressedEventPending = false,
    },
    {
        .pendingSince = 0,
        .gpioPort = GPIOA,
        .gpioPin = GPIO_PIN_3,
        .checkFlag = 0,
        .state = KEY_RELEASED,
        .pendingState = KEY_RELEASED,
        .keyFlag = false,
        .pressedEventPending = false,
    },
    {
        .pendingSince = 0,
        .gpioPort = GPIOA,
        .gpioPin = GPIO_PIN_4,
        .checkFlag = 0,
        .state = KEY_RELEASED,
        .pendingState = KEY_RELEASED,
        .keyFlag = false,
        .pressedEventPending = false,
    },
    {
        .pendingSince = 0,
        .gpioPort = GPIOA,
        .gpioPin = GPIO_PIN_5,
        .checkFlag = 0,
        .state = KEY_RELEASED,
        .pendingState = KEY_RELEASED,
        .keyFlag = false,
        .pressedEventPending = false,
    },
    {
        .pendingSince = 0,
        .gpioPort = GPIOA,
        .gpioPin = GPIO_PIN_6,
        .checkFlag = 0,
        .state = KEY_RELEASED,
        .pendingState = KEY_RELEASED,
        .keyFlag = false,
        .pressedEventPending = false,
    },
    {
        .pendingSince = 0,
        .gpioPort = GPIOA,
        .gpioPin = GPIO_PIN_7,
        .checkFlag = 0,
        .state = KEY_RELEASED,
        .pendingState = KEY_RELEASED,
        .keyFlag = false,
        .pressedEventPending = false,
    },
};

void Key_ConsumeEvent(uint8_t key)
{
    if (key >= 7)
    {
        return;
    }

    KeyList[key].keyFlag = false;
    KeyList[key].pressedEventPending = false;
}

void Key_ConsumeAllEvents(void)
{
    for (uint8_t key = 0; key < 7; key++)
    {
        Key_ConsumeEvent(key);
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    uint8_t pin = 0;

    switch (GPIO_Pin)
    {
    case GPIO_PIN_1:
        pin = 0;
        break;

    case GPIO_PIN_2:
        pin = 1;
        break;

    case GPIO_PIN_3:
        pin = 2;
        break;

    case GPIO_PIN_4:
        pin = 3;
        break;

    case GPIO_PIN_5:
        pin = 4;
        break;

    case GPIO_PIN_6:
        pin = 5;
        break;

    case GPIO_PIN_7:
        pin = 6;
        break;

    default:
        return;
    }

    KeyList[pin].pendingState =
        HAL_GPIO_ReadPin(
            KeyList[pin].gpioPort,
            KeyList[pin].gpioPin
        );

    KeyList[pin].pendingSince = Timer;
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
        if (current != KeyList[pin].pendingState)
        {
            KeyList[pin].pendingState = current;
            KeyList[pin].pendingSince = Timer;
            continue;
        }

        if ((uint32_t)(Timer - KeyList[pin].pendingSince) >= KEY_DEBOUNCE)
        {
            /*
             * 确认状态改变
             */
            if (KeyList[pin].state != KeyList[pin].pendingState)
            {
                KeyList[pin].state = KeyList[pin].pendingState;
                KeyList[pin].keyFlag = true;
                if (KeyList[pin].state == KEY_PRESSED)
                {
                    KeyList[pin].pressedEventPending = true;
                }
            }

            KeyList[pin].checkFlag = false;
        }
    }
}