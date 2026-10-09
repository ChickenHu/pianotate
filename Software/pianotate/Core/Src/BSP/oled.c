#include "BSP/oled.h"
#include "i2c.h"
#include "stm32f1xx_hal_i2c.h"
#include <stdint.h>
#include <string.h>
#include <sys/_intsup.h>
const uint8_t FreshPeriod = 25; // 刷新周期，单位为ms
uint8_t InitCmd[] = {
    0x00,       // Control byte: following bytes are commands
    0xAE,       // Display OFF
    0x20, 0x10, // Set Memory Addressing Mode
    0xB0,       // Set Page Start Address
    0xC8,       // COM Output Scan Direction
    0x00,       // Set lower column start address
    0x10,       // Set higher column start address
    0x40,       // Set Display Start Line
    0x81, 0xDF, // Set Contrast Control
    0xA1,       // Set Segment Re-map
    0xA6,       // Normal display
    0xA8, 0x3F, // Set Multiplex Ratio
    0xA4,       // Entire Display ON resume
    0xD3, 0x00, // Set Display Offset
    0xD5, 0xF0, // Set Display Clock Divide Ratio / Oscillator Frequency
    0xD9, 0x22, // Set Pre-charge Period
    0xDA, 0x12, // Set COM Pins Hardware Configuration
    0xDB, 0x20, // Set VCOMH Deselect Level
    0x8D, 0x14, // Charge Pump Setting
    0xAF        // Display ON
};
OLED_HandleTypeDef OLED1 = {}, OLED2 = {};
void OLED_Send(OLED_HandleTypeDef *oled, uint8_t *data, uint8_t len) {
  HAL_I2C_Master_Transmit(oled->OLED, OLED_ADDRESS, data, len, HAL_MAX_DELAY);
}
void OLED_SendCmd(OLED_HandleTypeDef *oled, uint8_t cmd) {
  static uint8_t sendBuffer[2] = {0};

  sendBuffer[0] = 0x00;
  sendBuffer[1] = cmd;

  OLED_Send(oled, sendBuffer, 2);
}
void OLED_Init() {
  OLED1.OLED = &hi2c1;
  OLED2.OLED = &hi2c2;
  memset(OLED1.FrameBuffer, 0, sizeof(uint8_t) * 129 * 16);
  memset(OLED2.FrameBuffer, 0, sizeof(uint8_t) * 129 * 16);
  for (uint8_t i = 0; i < 16; i++) {
    OLED1.FrameBuffer[i * 129] = 0x40;
    OLED2.FrameBuffer[i * 129] = 0x40;
  }
  OLED1.FrameBufferPtr = OLED1.FrameBuffer;
  OLED2.FrameBufferPtr = OLED2.FrameBuffer;
  OLED1.FrameBufferPrevPtr = &OLED1.FrameBuffer[8 * 129];
  OLED2.FrameBufferPrevPtr = &OLED2.FrameBuffer[8 * 129];

  /*OLED1*/
  OLED_Send(&OLED1, InitCmd, sizeof(InitCmd));
  for (uint16_t page = 0; page < 8; page++) {
    OLED_SendCmd(&OLED1, 0xB0 + page);
    OLED_SendCmd(&OLED1, 0x00);
    OLED_SendCmd(&OLED1, 0x10);
    OLED_Send(&OLED1, &OLED1.FrameBufferPtr[((page << 7) | page)], 129);
  }

  /*&OLED2*/
  OLED_Send(&OLED2, InitCmd, sizeof(InitCmd));
  for (uint16_t page = 0; page < 8; page++) {
    OLED_SendCmd(&OLED2, 0xB0 | page);
    OLED_SendCmd(&OLED2, 0x00);
    OLED_SendCmd(&OLED2, 0x10);
    OLED_Send(&OLED2, &OLED2.FrameBufferPtr[((page << 7) | page)], 129);
  }
}
void OLED_Clear(OLED_HandleTypeDef *oled) {
  memset(oled->FrameBufferPtr, 0x00, sizeof(uint8_t) * 129 * 8);
  for (uint16_t page = 0; page < 8; page++) {
    oled->FrameBufferPtr[((page << 7) | page)] = 0x40; // Clear each page
  }
}
void OLED_FreshFrame(OLED_HandleTypeDef *oled) {
  for (uint16_t page = 0; page < 8; page++) {
    if (memcmp(&oled->FrameBufferPtr[((page << 7) | page)],
               &oled->FrameBufferPrevPtr[((page << 7) | page)], 129)) {
      uint8_t cmd[] = {0x00, 0xB0 | page, 0x00, 0x10};
      OLED_Send(oled, cmd, 3);

      OLED_Send(oled, &oled->FrameBufferPtr[((page << 7) | page)], 129);
    }
  }
  // swap
  uint8_t *temp = oled->FrameBufferPtr;
  oled->FrameBufferPtr = oled->FrameBufferPrevPtr;
  oled->FrameBufferPrevPtr = temp;
}
void OLED_DrawPixel(uint8_t *frameBuffer, uint8_t x, uint8_t y) {
  if (x >= 128 || y >= 64) {
    return; // 超出范围
  }
  uint16_t page = y >> 3;
  uint8_t bitPosition = y & 0b0111;
  frameBuffer[((page << 7) | page) + x + 1] |= (1 << bitPosition);
  // OLED_FreshFrame(frameBuffer);
}
void OLED_Delete(uint8_t *frameBuffer, uint8_t x, uint8_t y, uint8_t width,
                uint8_t height) {
  if (frameBuffer == NULL || width == 0 || height == 0) {
    return;
  }

  uint8_t xEnd = x + width;
  uint8_t yEnd = y + height;

  for (uint8_t py = y; py < yEnd; py++) {
    if (py >= 64) {
      break;
    }
    for (uint8_t px = x; px < xEnd; px++) {
      if (px >= 128) {
        break;
      }
      uint16_t page = py >> 3;
      uint8_t bitPosition = py & 0b0111;
      frameBuffer[((page << 7) | page) + px + 1] &= ~(1u << bitPosition);
    }
  }
}
void OLED_DrawLine(uint8_t *frameBuffer, uint8_t x0, uint8_t y0, uint8_t x1,
                   uint8_t y1) {
  int dx = abs((int8_t)x1 - (int8_t)x0);
  int dy = abs((int8_t)y1 - (int8_t)y0);
  int sx = (x0 < x1) ? 1 : -1;
  int sy = (y0 < y1) ? 1 : -1;
  int err = dx - dy;

  while (true) {
    OLED_DrawPixel(frameBuffer, x0, y0);
    if (x0 == x1 && y0 == y1)
      break;
    int err2 = err << 1;
    if (err2 > -dy) {
      err -= dy;
      x0 += sx;
    }
    if (err2 < dx) {
      err += dx;
      y0 += sy;
    }
  }
}
void OLED_AddLayer(uint8_t *frameBuffer, uint8_t *layer) {
  for (uint16_t i = 0; i < 129 * 8; i++) {
    frameBuffer[i] |= layer[i];
  }
}
void OLED_UpdateState(OLED_HandleTypeDef *oled) { OLED_FreshFrame(oled); }