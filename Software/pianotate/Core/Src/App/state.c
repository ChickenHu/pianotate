#include "state.h"
#include "menu.h"
#include "oled.h"
#include <stdlib.h>

void State_Update() {
  // 所有状态机的状态更新都放在此处，统一维护
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