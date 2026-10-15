#pragma once
#include "oled.h"
#include "music.h"
#include <limits.h>
#include <stdint.h>

#define MENU_SONG_COVER_WIDTH 64u
#define MENU_SONG_COVER_HEIGHT 64u
#define MENU_SONG_COVER_BYTES (MENU_SONG_COVER_WIDTH * 8u)

typedef enum {
  MENU_PAGE_NONE = 0,
  MENU_PAGE_MODE = 1,
  MENU_PAGE_SONG = 2,
  MENU_PAGE_SPEED = 3,
} MenuPage_t;

typedef struct {
  bool isActivate;
  int8_t page;            // 0 => no refresh needed
  int16_t animationProcess;
  int16_t startY;
  int16_t endY;
  int8_t target;
  int8_t modeChoose;      // 1-4K,2-piano
  bool isAnimating;// animation
  bool onPageSwitching;
  int8_t songChoose;      // 1,2,3
  int8_t speedChoose;     // 1-slow 2-fast 3-sonic
} MenuState_t;
extern MenuState_t MenuState;

extern void Menu_Init();
extern void Menu_Test();
extern void Menu_StateUpdate();
extern void Menu_ReturnToSong();
extern void Menu_ReturnToMode();
extern void Menu_DrawArrow(int16_t yAxis, int16_t xAxis,
                           OLED_HandleTypeDef *oled);
extern void Menu_DrawArrowByProcess(int16_t process, int16_t startY,
                                    int16_t endY, int16_t xAxis,
                                    OLED_HandleTypeDef *oled);
/* Bitmap is page-major, with columns left-to-right; bit 0 is the page top. */
extern bool Menu_SetSongCover(SongName songName, const uint8_t *bitmap,
                              uint16_t bitmapSize);