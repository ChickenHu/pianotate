#pragma once

#include <stdint.h>
#include "oled.h"

void Font_DrawChar(OLED_HandleTypeDef *oled,
                   uint8_t x,
                   uint8_t y,
                   char character);

void Font_DrawText(OLED_HandleTypeDef *oled,
                   uint8_t x,
                   uint8_t y,
                   const char *text);