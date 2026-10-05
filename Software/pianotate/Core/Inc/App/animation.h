#pragma once
#include "BSP/oled.h"
typedef struct
{
    uint8_t x;
    uint8_t y;
} Point;
extern const Point projection_table[256*2][4];
extern uint8_t UILayer[129 * 8];
extern void Animation_Test();
extern void Animation_SetUI();
// extern inline uint8_t Animation_GetLUT(uint8_t state);