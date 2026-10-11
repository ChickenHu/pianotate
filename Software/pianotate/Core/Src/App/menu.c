#include "menu.h"
#include "animation.h"
#include "font.h"
#include "fourkeys.h"
#include "key.h"
#include "oled.h"
#include <stdint.h>

const int8_t delta = 16;
const uint8_t LUT[256] = {
#embed "../../Inc/Data/LUT.bin"
};
const int16_t PageLayout3[3] = {12, 28, 44};

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
      .modeChoose = 1,
      .songChoose = 1,
      .speedChoose = 1,
      .isAnimating = false,
  };
  ;
}

void Menu_DrawArrowByProcess(int16_t process, int16_t startY, int16_t endY,
                             int16_t xAxis,
                             OLED_HandleTypeDef *oled) { // 给出的左上顶点的坐标
  int16_t yAxis = Menu_GetArrowY(process, startY, endY);
  OLED_Clear(oled);

  for (int16_t y = yAxis; y < yAxis + 8; y++) {
    if (y >= 64) {
      break;
    }
    OLED_DrawLine(oled->FrameBufferPtr, xAxis, y, xAxis + y - yAxis + 1, y);
  }

  for (int16_t y = yAxis + 8; y < yAxis + 16; y++) {
    if (y >= 64) {
      break;
    }
    OLED_DrawLine(oled->FrameBufferPtr, xAxis, y, xAxis + yAxis - y + 16, y);
  }
}
void Menu_DrawArrow(int16_t yAxis, int16_t xAxis,
                    OLED_HandleTypeDef *oled) { // 给出的左上顶点的坐标
  OLED_Clear(oled);

  for (int16_t y = yAxis; y < yAxis + 8; y++) {
    if (y >= 64) {
      break;
    }
    OLED_DrawLine(oled->FrameBufferPtr, xAxis, y, xAxis + y - yAxis + 1, y);
  }

  for (int16_t y = yAxis + 8; y < yAxis + 16; y++) {
    if (y >= 64) {
      break;
    }
    OLED_DrawLine(oled->FrameBufferPtr, xAxis, y, xAxis + yAxis - y + 16, y);
  }
}

static void Menu_DrawModePage(void) {
  int16_t phase = MenuState.animationProcess;
  if (MenuState.isAnimating) {
    int16_t phase = MenuState.animationProcess;
    phase += delta;
    if (phase > 256) {
      MenuState.isAnimating = false;
      MenuState.modeChoose = MenuState.target;
      phase = 0;
    }

    MenuState.animationProcess = phase;
  }
  if (MenuState.isAnimating) {
    Menu_DrawArrowByProcess(phase, MenuState.startY, MenuState.endY, 4, &OLED1);
  } else {
    Menu_DrawArrow((MenuState.modeChoose == 1 ? 8 : 48), 4, &OLED1);
  }
  Font_DrawText(&OLED1, 32, 8, "Four Keys");
  Font_DrawText(&OLED1, 32, 48, "Piano");
}

static void Menu_DrawSongPage(void) {
  int16_t phase = MenuState.animationProcess;
  if (MenuState.isAnimating) {
    phase += delta;
    if (phase > 256) {
      MenuState.isAnimating = false;
      MenuState.songChoose = MenuState.target;
      phase = 0;
    }

    MenuState.animationProcess = phase;
  }
  if (MenuState.isAnimating) {
    Menu_DrawArrowByProcess(phase, MenuState.startY, MenuState.endY, 4, &OLED1);
  } else {
    Menu_DrawArrow(PageLayout3[MenuState.songChoose - 1], 4, &OLED1);
  }
  Font_DrawText(&OLED1, 32, PageLayout3[0], "Bad Apple!");
  Font_DrawText(&OLED1, 32, PageLayout3[1], "fish");
  Font_DrawText(&OLED1, 32, PageLayout3[2], "unknow");
}

static void Menu_DrawSpeedPage(void) {
  int16_t phase = MenuState.animationProcess;
  if (MenuState.isAnimating) {
    phase += delta;
    if (phase > 256) {
      MenuState.isAnimating = false;
      MenuState.speedChoose = MenuState.target;
      phase = 0;
    }

    MenuState.animationProcess = phase;
  }
  if (MenuState.isAnimating) {
    Menu_DrawArrowByProcess(phase, MenuState.startY, MenuState.endY, 4, &OLED2);
  } else {
    Menu_DrawArrow(PageLayout3[MenuState.speedChoose - 1], 4, &OLED2);
  }

  Font_DrawText(&OLED1, 32, PageLayout3[0], "Bad Apple!");
  Font_DrawText(&OLED1, 32, PageLayout3[1], "fish");
  Font_DrawText(&OLED1, 32, PageLayout3[2], "unknow");

  Font_DrawText(&OLED2, 70, PageLayout3[0], "Low");
  Font_DrawText(&OLED2, 70, PageLayout3[1], "High");
  Font_DrawText(&OLED2, 70, PageLayout3[2], "Sonic");
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
      MenuState.startY = 48;
      MenuState.endY = 8;
      MenuState.target = 1;
    } else if (KeyList[2].keyFlag && MenuState.isAnimating &&
               MenuState.target == 2) { // 打断动画，不修改start/end
      // MenuState.modeChoose = 1;
      MenuState.animationProcess = 256 - MenuState.animationProcess;
      MenuState.isAnimating = true;
      MenuState.startY = 48;
      MenuState.endY = 8;
      MenuState.target = 1;
    } else if (KeyList[3].keyFlag && !MenuState.isAnimating &&
               MenuState.modeChoose == 1) { // 开始动画，从1到2
      // MenuState.modeChoose = 1;
      MenuState.animationProcess = 1;
      MenuState.isAnimating = true;
      MenuState.startY = 8;
      MenuState.endY = 48;
      MenuState.target = 2;
    } else if (KeyList[3].keyFlag && MenuState.isAnimating &&
               MenuState.target == 1) {
      // MenuState.modeChoose = 1;
      MenuState.animationProcess = 256 - MenuState.animationProcess;
      MenuState.isAnimating = true;
      MenuState.startY = 8;
      MenuState.endY = 48;
      MenuState.target = 2;
    }
    if (KeyList[4].keyFlag) {
      if (MenuState.modeChoose == 1) {
        MenuState.page = MENU_PAGE_SONG;
        MenuState.isAnimating = 0;
        MenuState.songChoose = 1;
        MenuState.animationProcess = 0;
        MenuState.onPageSwitching = true;
        OLED_Clear(&OLED1);
      } else {
        MenuState.page = MENU_PAGE_NONE;
      }
    }
    Menu_DrawModePage();
    break;
  }
  case MENU_PAGE_SONG: {
    if (!KeyList[4].keyFlag && MenuState.onPageSwitching) {
      MenuState.onPageSwitching = false;
    }
    if (KeyList[2].keyFlag) { // 上移
      // MenuState.modeChoose = 1;
      if (MenuState.isAnimating) {
        if (MenuState.target > MenuState.songChoose) { // 正在下移
          // 反转
          MenuState.animationProcess = 256 - MenuState.animationProcess;
          MenuState.target = MenuState.songChoose;
          int16_t temp = MenuState.startY;
          MenuState.startY = MenuState.endY;
          MenuState.endY = temp;
        }
      } else {
        if (MenuState.songChoose != 1) { // 未到顶
          MenuState.isAnimating = true;
          MenuState.animationProcess = 1;
          MenuState.target = MenuState.songChoose - 1;
          MenuState.startY = PageLayout3[MenuState.songChoose - 1];
          MenuState.endY = PageLayout3[MenuState.target - 1];
        }
      }
    }
    if (KeyList[3].keyFlag) { // 下移
      // MenuState.modeChoose = 1;
      if (MenuState.isAnimating) {
        if (MenuState.target < MenuState.songChoose) { // 正在上移
          // 反转
          MenuState.animationProcess = 256 - MenuState.animationProcess;
          MenuState.target = MenuState.songChoose;
          int16_t temp = MenuState.startY;
          MenuState.startY = MenuState.endY;
          MenuState.endY = temp;
        }
      } else {
        if (MenuState.songChoose != 3) { // 未到底
          MenuState.isAnimating = true;
          MenuState.animationProcess = 1;
          MenuState.target = MenuState.songChoose + 1;
          MenuState.startY = PageLayout3[MenuState.songChoose - 1];
          MenuState.endY = PageLayout3[MenuState.target - 1];
        }
      }
    }
    if (!MenuState.onPageSwitching && KeyList[4].keyFlag) {
      MenuState.page = MENU_PAGE_SPEED;
      MenuState.isAnimating = 0;
      MenuState.speedChoose = 1;
      MenuState.animationProcess = 0;
      MenuState.onPageSwitching = true;
    }
    Menu_DrawSongPage();
    break;
  }
  case MENU_PAGE_SPEED: {
    if (!KeyList[4].keyFlag && MenuState.onPageSwitching) {
      MenuState.onPageSwitching = false;
    }
    if (KeyList[2].keyFlag) { // 上移
      // MenuState.modeChoose = 1;
      if (MenuState.isAnimating) {
        if (MenuState.target > MenuState.speedChoose) { // 正在下移
          // 反转
          MenuState.animationProcess = 256 - MenuState.animationProcess;
          MenuState.target = MenuState.speedChoose;
          int16_t temp = MenuState.startY;
          MenuState.startY = MenuState.endY;
          MenuState.endY = temp;
        }
      } else {
        if (MenuState.speedChoose != 1) { // 未到顶
          MenuState.isAnimating = true;
          MenuState.animationProcess = 1;
          MenuState.target = MenuState.speedChoose - 1;
          MenuState.startY = PageLayout3[MenuState.speedChoose - 1];
          MenuState.endY = PageLayout3[MenuState.target - 1];
        }
      }
    }
    if (KeyList[3].keyFlag) { // 下移
      // MenuState.modeChoose = 1;
      if (MenuState.isAnimating) {
        if (MenuState.target < MenuState.speedChoose) { // 正在上移
          // 反转
          MenuState.animationProcess = 256 - MenuState.animationProcess;
          MenuState.target = MenuState.speedChoose;
          int16_t temp = MenuState.startY;
          MenuState.startY = MenuState.endY;
          MenuState.endY = temp;
        }
      } else {
        if (MenuState.speedChoose != 3) { // 未到底
          MenuState.isAnimating = true;
          MenuState.animationProcess = 1;
          MenuState.target = MenuState.speedChoose + 1;
          MenuState.startY = PageLayout3[MenuState.speedChoose - 1];
          MenuState.endY = PageLayout3[MenuState.target - 1];
        }
      }
    }
    if (!MenuState.onPageSwitching && KeyList[4].keyFlag) {
      MenuState.page = MENU_PAGE_NONE;
      MenuState.animationProcess = 0;
      MenuState.isActivate = false;
      int32_t speed = 0;
      switch (MenuState.speedChoose) {
      case 1: {
        speed = 4;
        break;
      }
      case 2: {
        speed = 8;
        break;
      }
      case 3: {
        speed = 32;
        break;
      }
      }
      FourKeys_Init(speed, MenuState.songChoose);
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
