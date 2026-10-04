#include "state.h"

void State_Update() {
  // 所有状态机的状态更新都放在此处，统一维护
  Music_UpdateState();
}