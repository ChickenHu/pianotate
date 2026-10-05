#pragma once
#include "oled.h"
#include "animation.h"
#include <stdint.h>

typedef struct{
    uint8_t animationProcess;
    uint16_t animationTime;
    uint8_t state;//1 for choose buttton 1;2 for choose button 2;3 for move 1 to 2;4 for move 2 to 1
}MenuState_t;
extern MenuState_t MenuState;

extern void Menu_Init();
extern void Menu_Test();
extern void Menu_StateUpdate();