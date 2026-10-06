#include "state.h"
#include "menu.h"
#include <stdlib.h>

void State_Update() {
  // 所有状态机的状态更新都放在此处，统一维护
  Music_UpdateState();
  if (Timer % FreshPeriod == 0) {
    // Animation_Test();
    FourKeys_UpdateState();
    // Menu_Test();
    // Menu_StateUpdate();
    OLED_UpdateState(&OLED1);
    // FourKeys_UpdateState();
  }
}