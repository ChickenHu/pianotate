#include "BSP/oled.h"

void OLED_Send(uint8_t *data, uint8_t len) {
  HAL_I2C_Master_Transmit(&OLED, OLED_ADDRESS, data, len, HAL_MAX_DELAY);
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
//   OLED_SendCmd(0xA5);
}
void OLED_Clear(void) {
  uint8_t clearBuffer[129] = {0x40};
  for (uint8_t page = 0; page < 8; page++) {
    OLED_SendCmd(0xB0 + page);
    OLED_SendCmd(0x00);
    OLED_SendCmd(0x10);
    OLED_Send(clearBuffer, sizeof(clearBuffer));
  }
}
void OLED_FreshFrame(uint8_t *frameBuffer) {
  for (uint8_t page = 0; page < 4; page++) {
    OLED_SendCmd(0xB0 + page);
    OLED_SendCmd(0x00);
    OLED_SendCmd(0x10);
    OLED_Send(&frameBuffer[0], 129);
  }
}