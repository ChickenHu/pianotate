#include "menu.h"
#include "animation.h"
#include "key.h"
#include "oled.h"
#include <stdint.h>

const int8_t delta = 16;
const uint8_t LUT[256] = {
#embed "../../Inc/Data/LUT.bin"
};

MenuState_t MenuState = {false, MENU_PAGE_MODE, 0, 1, 1, 1, 1};

static int16_t Menu_GetArrowY(int16_t process, int8_t startY, int8_t endY) {
  int16_t span = (endY - startY);

  if (process < 1) {
    return startY;
  }
  if (process > 256) {
    return endY;
  }

  int16_t ratio = LUT[process - 1];
  int16_t y = (int16_t)(ratio * (int32_t)span / 256);
  return (int16_t)(y + startY);
}

void Menu_Init() {
  MenuState = (MenuState_t){
      .isActivate = false,
      .page = MENU_PAGE_MODE,
      .animationProcess = 0,
      .animationDirect = 0,
      .modeChoose = 1,
      .songChoose = 1,
      .speedChoose = 1,
      .isAnimating = false,
  };
  ;
}

void Menu_DrawArrowByProcess(int16_t process, int16_t startY, int16_t endY,
                             int16_t xAxis,
                             OLED_HandleTypeDef oled) { // 给出的左上顶点的坐标
  int16_t yAxis = Menu_GetArrowY(process, startY, endY);
  OLED_Delete(oled.FrameBufferPtr, xAxis, 0, 9, 64);

  for (int16_t y = yAxis; y < yAxis + 8; y++) {
    if (y >= 64) {
      break;
    }
    OLED_DrawLine(oled.FrameBufferPtr, xAxis, y, xAxis + y - yAxis + 1, y);
  }

  for (int16_t y = yAxis + 8; y < yAxis + 16; y++) {
    if (y >= 64) {
      break;
    }
    OLED_DrawLine(oled.FrameBufferPtr, xAxis, y, xAxis + yAxis - y + 16, y);
  }
}
void Menu_DrawArrow(int16_t yAxis, int16_t xAxis,
                    OLED_HandleTypeDef oled) { // 给出的左上顶点的坐标
  OLED_Delete(oled.FrameBufferPtr, xAxis, 0, 9, 64);

  for (int16_t y = yAxis; y < yAxis + 8; y++) {
    if (y >= 64) {
      break;
    }
    OLED_DrawLine(oled.FrameBufferPtr, xAxis, y, xAxis + y - yAxis + 1, y);
  }

  for (int16_t y = yAxis + 8; y < yAxis + 16; y++) {
    if (y >= 64) {
      break;
    }
    OLED_DrawLine(oled.FrameBufferPtr, xAxis, y, xAxis + yAxis - y + 16, y);
  }
}

static void Menu_DrawModePage(void) {
  int16_t phase = MenuState.animationProcess;
  if (MenuState.isAnimating) {
    int16_t phase = MenuState.animationProcess;
    phase += delta * MenuState.animationDirect;
    if (phase > 256) {
      MenuState.isAnimating = false;
      MenuState.modeChoose = MenuState.target;
      phase = 0;
      MenuState.animationDirect = 0;
    }

    MenuState.animationProcess = phase;
  }
  if (MenuState.isAnimating) {
    Menu_DrawArrowByProcess(phase, MenuState.startY, MenuState.endY, 4, OLED1);
  } else {
    Menu_DrawArrow((MenuState.modeChoose == 1 ? 8 : 48), 4, OLED1);
  }
}

static void Menu_DrawSongPage(void) {
  int8_t rowY[] = {12, 28, 44};
  int8_t targetY = rowY[MenuState.songChoose - 1];
  Menu_DrawArrowByProcess(MenuState.animationProcess, 8, 48, 4, OLED1);
  (void)targetY;
}

static void Menu_DrawSpeedPage(void) {
  int8_t rowY[] = {12, 28, 44};
  int8_t targetY = rowY[MenuState.speedChoose - 1];
  Menu_DrawArrowByProcess(MenuState.animationProcess, 8, 48, 4, OLED2);
  (void)targetY;
}

void Menu_StateUpdate() {
  if (MenuState.page == MENU_PAGE_NONE) {
    return;
  }

  switch (MenuState.page) {
  case MENU_PAGE_MODE: {
    if (KeyList[2].keyFlag && !MenuState.isAnimating &&
        MenuState.modeChoose == 2) { // 开始动画，从2到1
      // MenuState.modeChoose = 1;
      MenuState.animationProcess = 1;
      MenuState.isAnimating = true;
      MenuState.animationDirect = 1;
      MenuState.startY = 48;
      MenuState.endY = 8;
      MenuState.target = 1;
      // } else if (KeyList[2].keyFlag && MenuState.isAnimating &&
      //            MenuState.target == 2) { // 打断动画，不修改start/end
      //   // MenuState.modeChoose = 1;
      //   MenuState.animationDirect = -1;
    } else if (KeyList[3].keyFlag && !MenuState.isAnimating &&
               MenuState.modeChoose == 1) { // 开始动画，从1到2
      // MenuState.modeChoose = 1;
      MenuState.animationProcess = 1;
      MenuState.isAnimating = true;
      MenuState.animationDirect = 1;
      MenuState.startY = 8;
      MenuState.endY = 48;
      MenuState.target = 2;
      // } else if (KeyList[3].keyFlag && MenuState.isAnimating &&
      //            MenuState.target == 1) {
      //   // MenuState.modeChoose = 1;
      //   MenuState.animationDirect = -1;
    }
    if (KeyList[4].keyFlag) {
      if (MenuState.modeChoose == 1) {
        MenuState.page = MENU_PAGE_SONG;
        MenuState.songChoose = 1;
        MenuState.animationProcess = 0;
        MenuState.animationDirect = 1;
      } else {
        MenuState.page = MENU_PAGE_NONE;
      }
    }
    Menu_DrawModePage();
    break;
  }
  case MENU_PAGE_SONG: {
    if (KeyList[2].keyFlag && MenuState.songChoose > 1) {
      MenuState.songChoose--;
    }
    if (KeyList[3].keyFlag && MenuState.songChoose < 3) {
      MenuState.songChoose++;
    }
    if (KeyList[4].keyFlag) {
      MenuState.page = MENU_PAGE_SPEED;
      MenuState.speedChoose = 1;
      MenuState.animationProcess = 0;
      MenuState.animationDirect = 1;
    }
    Menu_DrawSongPage();
    break;
  }
  case MENU_PAGE_SPEED: {
    if (KeyList[2].keyFlag && MenuState.speedChoose > 1) {
      MenuState.speedChoose--;
    }
    if (KeyList[3].keyFlag && MenuState.speedChoose < 3) {
      MenuState.speedChoose++;
    }
    if (KeyList[4].keyFlag) {
      MenuState.page = MENU_PAGE_NONE;
      MenuState.animationProcess = 0;
      MenuState.animationDirect = 1;
      // game entry is intentionally left for later.
    }
    Menu_DrawSpeedPage();
    break;
  }
  default:
    break;
  }
}

void Menu_Test() {
  // no-op placeholder, left for later text rendering.
}
