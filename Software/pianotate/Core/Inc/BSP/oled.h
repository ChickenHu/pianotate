#pragma once
#include "i2c.h"
#include <stdlib.h>
#include <string.h>

#define OLED hi2c1
#define OLED_ADDRESS 0x78


extern uint8_t *FrameBufferPtr;
extern uint8_t *FrameBufferPrevPtr;
extern const uint8_t FreshPeriod;

extern void OLED_Send(uint8_t *data, uint8_t len);
extern void OLED_Init();
extern void OLED_SendCmd(uint8_t cmd);
extern void OLED_Clear();
extern void OLED_FreshFrame();
extern void OLED_DrawPixel(uint8_t *frameBuffer, uint8_t x, uint8_t y);
extern void OLED_DrawLine(uint8_t *frameBuffer, uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
extern void OLED_AddLayer(uint8_t *frameBuffer, uint8_t *layer);
extern void OLED_UpdateState();