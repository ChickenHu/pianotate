#include "BSP/oled.h"
#include "stm32f1xx_hal_i2c.h"
#include <stdint.h>
const uint8_t FreshPeriod = 30; // 刷新周期，单位为ms
uint8_t FrameBuffer[129 * 16] = {
    0x40}; // 1 byte for control byte, 128 bytes for pixel data
uint8_t *FrameBufferPtr = FrameBuffer;
uint8_t *FrameBufferPrevPtr =
    &FrameBuffer[129 * 8]; // 1 byte for control byte, 128 bytes for pixel data
void OLED_Send(uint8_t *data, uint8_t len) {
  HAL_I2C_Master_Transmit(&OLED, OLED_ADDRESS, data, len,HAL_MAX_DELAY);
}
void OLED_SendCmd(uint8_t cmd) {
  static uint8_t sendBuffer[2] = {0};
  sendBuffer[1] = cmd;
  OLED_Send(sendBuffer, 2);
}
void OLED_Init(void) {
  OLED_SendCmd(0xAE); /*关闭显示 display off*/

  OLED_SendCmd(0x20);
  OLED_SendCmd(0x10);

  OLED_SendCmd(0xB0);

  OLED_SendCmd(0xC8);

  OLED_SendCmd(0x00);
  OLED_SendCmd(0x10);

  OLED_SendCmd(0x40);

  OLED_SendCmd(0x81);

  OLED_SendCmd(0xDF);
  OLED_SendCmd(0xA1);

  OLED_SendCmd(0xA6);
  OLED_SendCmd(0xA8);

  OLED_SendCmd(0x3F);

  OLED_SendCmd(0xA4);

  OLED_SendCmd(0xD3);
  OLED_SendCmd(0x00);

  OLED_SendCmd(0xD5);
  OLED_SendCmd(0xF0);

  OLED_SendCmd(0xD9);
  OLED_SendCmd(0x22);

  OLED_SendCmd(0xDA);
  OLED_SendCmd(0x12);

  OLED_SendCmd(0xDB);
  OLED_SendCmd(0x20);

  OLED_SendCmd(0x8D);
  OLED_SendCmd(0x14);

  OLED_SendCmd(0xAF);
  // OLED_SendCmd(0xA5);
  for (uint8_t page = 0; page < 16; page++) {
    memset(&FrameBuffer[page * 129], 0x00,
           sizeof(uint8_t) * 129);  // Clear each page
    FrameBuffer[page * 129] = 0x40; // Set control byte for each page
  }
  for (uint8_t page = 0; page < 8; page++) {
    OLED_SendCmd(0xB0 + page);
    OLED_SendCmd(0x00);
    OLED_SendCmd(0x10);
    OLED_Send(&FrameBufferPtr[page * 129], 129);
  }
}
void OLED_Clear(void) {
  memset(FrameBufferPtr, 0x00, sizeof(FrameBufferPtr[0]) * 129 * 8);
  for (uint8_t page = 0; page < 8; page++) {
    FrameBufferPtr[page * 129] = 0x40; // Clear each page
  }
  // OLED_FreshFrame(FrameBuffer);
}
void OLED_FreshFrame() {
  for (uint8_t page = 0; page < 8; page++) {
    if (memcmp(&FrameBufferPtr[page * 129], &FrameBufferPrevPtr[page * 129],
               129)) {
      OLED_SendCmd(0xB0 + page);
      OLED_SendCmd(0x00);
      OLED_SendCmd(0x10);
      OLED_Send(&FrameBufferPtr[page * 129], 129);
    }
  }
  // swap
  uint8_t *temp = FrameBufferPtr;
  FrameBufferPtr = FrameBufferPrevPtr;
  FrameBufferPrevPtr = temp;
}
void OLED_DrawPixel(uint8_t *frameBuffer, uint8_t x, uint8_t y) {
  if (x >= 128 || y >= 64) {
    return; // 超出范围
  }
  uint8_t page = y / 8;
  uint8_t bitPosition = y % 8;
  frameBuffer[page * 129 + x + 1] |= (1 << bitPosition);
  // OLED_FreshFrame(frameBuffer);
}
void OLED_DrawLine(uint8_t *frameBuffer, uint8_t x0, uint8_t y0, uint8_t x1,
                   uint8_t y1) {
  int dx = abs(x1 - x0);
  int dy = abs(y1 - y0);
  int sx = (x0 < x1) ? 1 : -1;
  int sy = (y0 < y1) ? 1 : -1;
  int err = dx - dy;

  while (true) {
    OLED_DrawPixel(frameBuffer, x0, y0);
    if (x0 == x1 && y0 == y1)
      break;
    int err2 = err * 2;
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
void OLED_UpdateState() { OLED_FreshFrame(); }