#include "state.h"
#include "key.h"
#include "led.h"
#include "menu.h"
#include "oled.h"
#include "stm32f103xb.h"
#include "stm32f1xx_hal_gpio.h"
#include <stdint.h>
#include <stdlib.h>


void State_Update() {
  // 所有状态机的状态更新都放在此处，统一维护
  Key_Update();
  for (uint8_t i = 0; i < 7; i++) {
    if (KeyList[i].keyFlag) {
      HAL_GPIO_WritePin(LED[i].gpio, LED[i].pin, GPIO_PIN_SET);
    } else {
      HAL_GPIO_WritePin(LED[i].gpio, LED[i].pin, GPIO_PIN_RESET);
    }
  }
  Music_UpdateState();
  if (Timer % (FreshPeriod * 2) == 0) {
    // Animation_Test();
    Menu_StateUpdate();
    FourKeys_UpdateState();
    OLED_UpdateState(&OLED2);
    OLED_UpdateState(&OLED1);
    // FourKeys_UpdateState();
  } else if (Timer % FreshPeriod == 0) {
    // Menu_Test();
    // OLED_UpdateState(&OLED2);
  }
}