#include "state.h"
#include "key.h"
#include "menu.h"
#include "oled.h"
#include "stm32f103xb.h"
#include "stm32f1xx_hal_gpio.h"
#include <stdlib.h>

void State_Update() {
  // 所有状态机的状态更新都放在此处，统一维护
  Key_Update();
  if (KeyList[3].keyFlag) {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
  } else {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
  }
  Music_UpdateState();
  if (Timer % (FreshPeriod * 2) == 0) {
    // Animation_Test();
    FourKeys_UpdateState();
    OLED_UpdateState(&OLED1);
    // FourKeys_UpdateState();
  } else if (Timer % FreshPeriod == 0) {
    Menu_Test();
    Menu_StateUpdate();
    OLED_UpdateState(&OLED2);
  }
}