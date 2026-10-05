#include "menu.h"
#include "animation.h"
#include "oled.h"
#include <stdint.h>
const uint8_t LUT[256] = {
#embed "../../Inc/Data/LUT.bin"
};
MenuState_t MenuState = {0, 0, 1};

void Menu_Init() {
  MenuState.state = 0;
  MenuState.animationProcess = 0;
  MenuState.animationTime = 0;
}
void Menu_Draw(uint8_t process) { // process:0~255
  uint8_t yAxis = process >> 3;   // 上边线位置
  OLED_Clear();
  for (uint8_t i = 0; i < 32; i++) {
    OLED_DrawLine(FrameBufferPtr, 0, i + yAxis, 127, i + yAxis);
  }
  OLED_UpdateState();
}

void Menu_Test() {
  // Menu_Draw(0);
  if (MenuState.state == 1) {
    MenuState.state = 3;
    MenuState.animationTime=0;
  } else if (MenuState.state == 2) {
    // MenuState.state = 4;
    // MenuState.animationTime=7;
  } else if (MenuState.state == 3) {
    if (MenuState.animationTime > 3) {
      MenuState.state = 4;
    }
  } else {
    if (MenuState.animationTime <= 3) {
    //   MenuState.state = 3;
    }
  }
}
void Menu_StateUpdate() {
  switch (MenuState.state) {
  case 1: {
    Menu_Draw(0);
    break;
  }
  case 2: {
    Menu_Draw(255);
    break;
  }
  case 3: {
    Menu_Draw(MenuState.animationProcess);
    if (MenuState.animationTime == 7) {
      MenuState.state = 2;
      MenuState.animationProcess = 255;
      MenuState.animationTime = 7;
      break;
    }
    MenuState.animationTime++;
    MenuState.animationProcess = LUT[MenuState.animationTime << 5];
    break;
  }
  case 4: {
    Menu_Draw(MenuState.animationProcess);
    if (MenuState.animationTime == 0) {
      MenuState.state = 1;
      MenuState.animationProcess = 0;
      MenuState.animationTime = 0;
      break;
    }
    MenuState.animationTime--;
    MenuState.animationProcess = LUT[255 - (MenuState.animationTime << 5)];
    break;
  }
  default: {
    break;
  }
  }
}