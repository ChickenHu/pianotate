#pragma once
#include "i2c.h"
#include "stm32f1xx_hal_i2c.h"
#include <stdlib.h>
#include <string.h>

#define OLED_ADDRESS 0x78

typedef struct {
  const I2C_HandleTypeDef *OLED;
  uint8_t FrameBuffer[129 *
                      16]; // 1 byte for control byte, 128 bytes for pixel data
  uint8_t *FrameBufferPtr;
  uint8_t
      *FrameBufferPrevPtr; // 1 byte for control byte, 128 bytes for pixel data
} OLED_HandleTypeDef;

extern OLED_HandleTypeDef OLED1, OLED2;
extern const uint8_t FreshPeriod;

extern void OLED_Send(OLED_HandleTypeDef *oled, uint8_t *data, uint8_t len);

extern void OLED_Init();

extern void OLED_SendCmd(OLED_HandleTypeDef *oled, uint8_t cmd);

extern void OLED_Clear(OLED_HandleTypeDef *oled);

extern void OLED_FreshFrame(OLED_HandleTypeDef *oled);

extern void OLED_DrawPixel(uint8_t *frameBuffer, uint8_t x, uint8_t y);

extern void OLED_DrawLine(uint8_t *frameBuffer, uint8_t x0, uint8_t y0,
                          uint8_t x1, uint8_t y1);

extern void OLED_AddLayer(uint8_t *frameBuffer, uint8_t *layer);

extern void OLED_UpdateState(OLED_HandleTypeDef *oled);