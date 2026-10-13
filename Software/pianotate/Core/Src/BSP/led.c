#include "led.h"
#include "timer.h"

LED_HandleTypeDef LED[7] = {
		{.gpio = GPIOA, .pin = GPIO_PIN_10}, {.gpio = GPIOA, .pin = GPIO_PIN_9},
		{.gpio = GPIOA, .pin = GPIO_PIN_8},  {.gpio = GPIOB, .pin = GPIO_PIN_15},
		{.gpio = GPIOB, .pin = GPIO_PIN_14}, {.gpio = GPIOB, .pin = GPIO_PIN_13},
		{.gpio = GPIOB, .pin = GPIO_PIN_12},
};

static volatile uint8_t LED_Brightness[7];
static volatile uint8_t LED_PWMPhase;
static volatile bool LED_BreathingActive;
static uint32_t LED_BreathingStartTick;

void LED_BreathingStart(void) {
	for (uint8_t index = 0; index < 7; index++) {
		LED_Brightness[index] = 0;
		HAL_GPIO_WritePin(LED[index].gpio, LED[index].pin, GPIO_PIN_RESET);
	}
	LED_PWMPhase = 0;
	LED_BreathingStartTick = Timer;
	LED_BreathingActive = true;
}

void LED_BreathingStop(void) {
	LED_BreathingActive = false;
	for (uint8_t index = 0; index < 7; index++) {
		LED_Brightness[index] = 0;
		HAL_GPIO_WritePin(LED[index].gpio, LED[index].pin, GPIO_PIN_RESET);
	}
}

void LED_PWMInterrupt(void) {
	if (!LED_BreathingActive) {
		return;
	}

	LED_PWMPhase = (LED_PWMPhase + 1u) & 0x0Fu;
	if (LED_PWMPhase == 0) {
		uint32_t elapsed = (uint32_t)(Timer - LED_BreathingStartTick);
		if (elapsed >= 5000u) {
			LED_BreathingStop();
			return;
		}

		uint32_t sequencePosition = elapsed * 7u;
		uint8_t activeLED = (uint8_t)(sequencePosition / 5000u);
		uint32_t fadePosition = sequencePosition % 5000u;
		uint32_t brightness = fadePosition <= 2500u
								  ? fadePosition * 16u / 2500u
								  : (5000u - fadePosition) * 16u / 2500u;
		for (uint8_t index = 0; index < 7; index++) {
			LED_Brightness[index] = index == activeLED ? (uint8_t)brightness : 0;
		}
	}

	for (uint8_t index = 0; index < 7; index++) {
		GPIO_PinState pinState = LED_PWMPhase < LED_Brightness[index]
																 ? GPIO_PIN_SET
																 : GPIO_PIN_RESET;
		HAL_GPIO_WritePin(LED[index].gpio, LED[index].pin, pinState);
	}
}
