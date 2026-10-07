#include "animation.h"
#include "oled.h"
#include <stdint.h>

//0-1 2-3是长边
const Point projection_table[256 * 2][4] = {
#embed "../../Inc/Data/projection_table.bin"
};
uint16_t projection_table_index = 0;
uint8_t UILayer[129 * 8] = {0};
void Animation_SetUI() {
  // border
  //   OLED_DrawLine(UILayer, 0, 0, 127, 0);
  //   OLED_DrawLine(UILayer, 0, 63, 127, 63);
  OLED_DrawLine(UILayer, 0, 0, 0, 63);
  OLED_DrawLine(UILayer, 127, 0, 127, 63);
  for (uint16_t i = 0; i < 4; i++) {
    OLED_DrawLine(
        UILayer, projection_table[0][i % 4].x, projection_table[0][i % 4].y,
        projection_table[0][(i + 1) % 4].x, projection_table[0][(i + 1) % 4].y);
  }
  // lanes
  OLED_DrawLine(UILayer, 0, 32, projection_table[0][1].x, 32);
  OLED_DrawLine(UILayer, 127, 32, projection_table[0][3].x, 32);
  for (uint16_t i = 0; i < 512; i++) {
    OLED_DrawPixel(UILayer, projection_table[i][0].x, projection_table[i][0].y);
    OLED_DrawPixel(UILayer, projection_table[i][1].x, projection_table[i][1].y);
    OLED_DrawPixel(UILayer, projection_table[i][2].x, projection_table[i][2].y);
    OLED_DrawPixel(UILayer, projection_table[i][3].x, projection_table[i][3].y);
  }
  // hit box
  OLED_DrawLine(UILayer, projection_table[511 - 8][1].x,
                projection_table[511 - 8][1].y, projection_table[511 - 8][2].x,
                projection_table[511 - 8][2].y);
  OLED_DrawLine(UILayer, projection_table[511 - 8][0].x,
                projection_table[511 - 8][0].y, projection_table[511 - 8][3].x,
                projection_table[511 - 8][3].y);
}
void Animation_Test() {
  OLED_Clear(&OLED1);
  for (uint16_t size = 0; size < 8; size++) {

    if (projection_table_index + size >= 256 * 2) {
      break;
    }
    for (uint16_t i = 1; i < 4; i += 2) {
      OLED_DrawLine(
          OLED1.FrameBufferPtr,
          projection_table[projection_table_index + size][i % 4].x,
          projection_table[projection_table_index + size][i % 4].y,
          projection_table[projection_table_index + size][(i + 1) % 4].x,
          projection_table[projection_table_index + size][(i + 1) % 4].y);
    }
  }
  projection_table_index+=4;
  projection_table_index %= 512;
  OLED_AddLayer(OLED1.FrameBufferPtr, UILayer);
}
// inline uint8_t Animation_GetLUT(uint8_t state){
//     return LUT[state];
// }