#include "menu.h"
#include "font.h"
#include "fourkeys.h"
#include "key.h"
#include "rank.h"
#include "oled.h"
#include "piano.h"
#include <stdint.h>

const int8_t delta = 16;
const uint8_t LUT[256] = {
#embed "../../Inc/Data/LUT.bin"
};
const int16_t PageLayout3[3] = {12, 28, 44};

MenuState_t MenuState = {false, MENU_PAGE_MODE, 0, 1, 1, 1, 1};
static const uint8_t *SongCovers[4];

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
  Rank_Init();
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

void Menu_ReturnToSong() {
  MenuState.page = MENU_PAGE_SONG;
  MenuState.isActivate = true;
  MenuState.isAnimating = false;
  MenuState.animationProcess = 0;
  MenuState.onPageSwitching = false;
  OLED_Clear(&OLED2);
}

void Menu_ReturnToMode() {
  MenuState.page = MENU_PAGE_MODE;
  MenuState.isActivate = true;
  MenuState.isAnimating = false;
  MenuState.animationProcess = 0;
  MenuState.onPageSwitching = false;
  OLED_Clear(&OLED2);
  OLED_Clear(&OLED1);
}

bool Menu_SetSongCover(SongName songName, const uint8_t *bitmap,
                       uint16_t bitmapSize) {
  if (songName < BADAPPLE || songName > NONE ||
  (bitmap != NULL && bitmapSize != MENU_SONG_COVER_BYTES)) {
    return false;
  }
  SongCovers[songName] = bitmap;
  return true;
}

static void Menu_FormatScore(uint32_t score, char output[11]) {
  char reversedDigits[10];
  uint8_t digitCount = 0;
  do {
    reversedDigits[digitCount++] = (char)('0' + score % 10u);
    score /= 10u;
  } while (score != 0 && digitCount < sizeof(reversedDigits));

  for (uint8_t digit = 0; digit < digitCount; digit++) {
    output[digit] = reversedDigits[digitCount - digit - 1];
  }
  output[digitCount] = '\0';
}

static void Menu_DrawSongInfo(void) {
  SongName songName = (SongName)MenuState.songChoose;
  const char *songTitle = "ROTATE TEST";
  if (songName == BADAPPLE) {
    songTitle = "Bad Apple!";
  } else if (songName == FISH) {
    songTitle = "Fish";
  }

  OLED_Clear(&OLED2);
  const uint8_t *cover = SongCovers[songName];
  if (cover != NULL) {
    for (uint8_t x = 0; x < MENU_SONG_COVER_WIDTH; x++) {
      for (uint8_t y = 0; y < MENU_SONG_COVER_HEIGHT; y++) {
        if (cover[x * 8u + (y >> 3)] & (1u << (y & 7u))) {
          OLED_DrawPixel(OLED2.FrameBufferPtr, x + 2u, y + 2u);
        }
      }
    }
  }
  Font_DrawText(&OLED2, 64, 0, songTitle);
  for (uint8_t rank = 0; rank < RANK_ENTRIES; rank++) {
    char rankText[3] = {'0', (char)('1' + rank), '\0'};
    char scoreText[11];
    Menu_FormatScore(Rank_GetScore(songName, rank), scoreText);
    uint8_t y = (uint8_t)((rank + 1u) * 16u);
    Font_DrawText(&OLED2, 64, y, rankText);
    Font_DrawText(&OLED2, 82, y, scoreText);
  }
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
  OLED_Clear(&OLED2);
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
  Font_DrawText(&OLED1, 32, PageLayout3[2], "ROTATE TEST");
  Menu_DrawSongInfo();
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
    Menu_DrawArrowByProcess(phase, MenuState.startY, MenuState.endY, 4, &OLED1);
  } else {
    Menu_DrawArrow(PageLayout3[MenuState.speedChoose - 1], 4, &OLED1);
  }

  Font_DrawText(&OLED1, 32, PageLayout3[0], "Low");
  Font_DrawText(&OLED1, 32, PageLayout3[1], "High");
  Font_DrawText(&OLED1, 32, PageLayout3[2], "Sonic");
  Menu_DrawSongInfo();
}

void Menu_StateUpdate() {
  if (MenuState.page == MENU_PAGE_NONE) {
    return;
  }

  bool backPressed = false;
  const uint8_t backKeys[] = {0, 1, 5, 6};
  for (uint8_t key = 0; key < sizeof(backKeys); key++) {
    uint8_t keyIdx = backKeys[key];
    if (KeyList[keyIdx].keyFlag && KeyList[keyIdx].state == KEY_PRESSED) {
      backPressed = true;
      break;
    }
  }
  if (backPressed) {
    if (MenuState.page == MENU_PAGE_SPEED) {
      MenuState.page = MENU_PAGE_SONG;
      OLED_Clear(&OLED2);
    } else if (MenuState.page == MENU_PAGE_SONG) {
      MenuState.page = MENU_PAGE_MODE;
    }
    MenuState.isActivate = true;
    MenuState.isAnimating = false;
    MenuState.animationProcess = 0;
  }

  switch (MenuState.page) {
  case MENU_PAGE_MODE: {
    if (KeyList[2].keyFlag && KeyList[2].state == KEY_PRESSED &&
      !MenuState.isAnimating &&
        MenuState.modeChoose == 2) { // 开始动画，从2到1
      // MenuState.modeChoose = 1;
      MenuState.animationProcess = 1;
      MenuState.isAnimating = true;
      MenuState.startY = 48;
      MenuState.endY = 8;
      MenuState.target = 1;
    } else if (KeyList[2].keyFlag && KeyList[2].state == KEY_PRESSED &&
           MenuState.isAnimating &&
               MenuState.target == 2) { // 打断动画，不修改start/end
      // MenuState.modeChoose = 1;
      MenuState.animationProcess = 256 - MenuState.animationProcess;
      MenuState.isAnimating = true;
      MenuState.startY = 48;
      MenuState.endY = 8;
      MenuState.target = 1;
    } else if (KeyList[3].keyFlag && KeyList[3].state == KEY_PRESSED &&
           !MenuState.isAnimating &&
               MenuState.modeChoose == 1) { // 开始动画，从1到2
      // MenuState.modeChoose = 1;
      MenuState.animationProcess = 1;
      MenuState.isAnimating = true;
      MenuState.startY = 8;
      MenuState.endY = 48;
      MenuState.target = 2;
    } else if (KeyList[3].keyFlag && KeyList[3].state == KEY_PRESSED &&
           MenuState.isAnimating &&
               MenuState.target == 1) {
      // MenuState.modeChoose = 1;
      MenuState.animationProcess = 256 - MenuState.animationProcess;
      MenuState.isAnimating = true;
      MenuState.startY = 8;
      MenuState.endY = 48;
      MenuState.target = 2;
    }
    if (KeyList[4].keyFlag && KeyList[4].state == KEY_PRESSED) {
      if (MenuState.modeChoose == 1) {
        MenuState.page = MENU_PAGE_SONG;
        MenuState.isAnimating = 0;
        MenuState.songChoose = 1;
        MenuState.animationProcess = 0;
        MenuState.onPageSwitching = true;
        OLED_Clear(&OLED1);
      } else {
        MenuState.page = MENU_PAGE_NONE;
        MenuState.isAnimating = 0;
        MenuState.animationProcess = 0;
        MenuState.isActivate = false;
        Piano_Init();
      }
    }
    Menu_DrawModePage();
    break;
  }
  case MENU_PAGE_SONG: {
    if (KeyList[4].state == KEY_RELEASED && MenuState.onPageSwitching) {
      MenuState.onPageSwitching = false;
    }
    if (KeyList[2].keyFlag && KeyList[2].state == KEY_PRESSED) { // 上移
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
    if (KeyList[3].keyFlag && KeyList[3].state == KEY_PRESSED) { // 下移
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
    if (!MenuState.onPageSwitching && KeyList[4].keyFlag &&
      KeyList[4].state == KEY_PRESSED) {
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
    if (KeyList[4].state == KEY_RELEASED && MenuState.onPageSwitching) {
      MenuState.onPageSwitching = false;
    }
    if (KeyList[2].keyFlag && KeyList[2].state == KEY_PRESSED) { // 上移
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
    if (KeyList[3].keyFlag && KeyList[3].state == KEY_PRESSED) { // 下移
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
    if (!MenuState.onPageSwitching && KeyList[4].keyFlag &&
      KeyList[4].state == KEY_PRESSED) {
      MenuState.page = MENU_PAGE_NONE;
      MenuState.animationProcess = 0;
      MenuState.isActivate = false;
      int32_t speed = 0;
      switch (MenuState.speedChoose) {
      case 1: {
        speed = 10;
        break;
      }
      case 2: {
        speed = 16;
        break;
      }
      case 3: {
        speed = 128;
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
  for (uint8_t key = 0; key < 7; key++) {
    KeyList[key].keyFlag = false;
  }
}

void Menu_Test() {
  // no-op placeholder, left for later text rendering.
}
