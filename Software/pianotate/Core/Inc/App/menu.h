#pragma once
#include "oled.h"
#include "animation.h"
#include <limits.h>
#include <stdint.h>

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
  int8_t animationDirect; // -1-up,1-down
  int16_t startY;
  int16_t endY;
  int8_t target;
  int8_t modeChoose;      // 1-4K,2-piano
  bool isAnimating;// animation
  int8_t songChoose;      // 1,2,3
  int8_t speedChoose;     // 1-slow 2-fast 3-sonic
} MenuState_t;
extern MenuState_t MenuState;

extern void Menu_Init();
extern void Menu_Test();
extern void Menu_StateUpdate();