#include "i2c.h"
#include <stdlib.h>
#include <string.h>

#define OLED hi2c1
#define OLED_ADDRESS 0x78
#define SSD1315_CMD_LOWER_COLUMN_START_ADDRESS              0x00        /**< command lower column start address */
#define SSD1315_CMD_HIGHER_COLUMN_START_ADDRESS             0x10        /**< command higher column start address */
#define SSD1315_CMD_MEMORY_ADDRESSING_MODE                  0x20        /**< command memory addressing mode */
#define SSD1315_CMD_SET_COLUMN_ADDRESS                      0x21        /**< command set column address */
#define SSD1315_CMD_SET_PAGE_ADDRESS                        0x22        /**< command set page address */
#define SSD1315_CMD_SET_FADE_OUT_AND_BLINKING               0x23        /**< command set fade out and blinking */
#define SSD1315_CMD_RIGHT_HORIZONTAL_SCROLL                 0x26        /**< command right horizontal scroll */
#define SSD1315_CMD_LEFT_HORIZONTAL_SCROLL                  0x27        /**< command left horizontal scroll */
#define SSD1315_CMD_VERTICAL_RIGHT_HORIZONTAL_SCROLL        0x29        /**< command vertical right horizontal scroll */
#define SSD1315_CMD_VERTICAL_LEFT_HORIZONTAL_SCROLL         0x2A        /**< command vertical left horizontal scroll */
#define SSD1315_CMD_RIGHT_HORIZONTAL_SCROLL_SETUP           0x2C        /**< command right horizontal scroll setup */
#define SSD1315_CMD_LEFT_HORIZONTAL_SCROLL_SETUP            0x2D        /**< command left horizontal scroll setup */
#define SSD1315_CMD_DEACTIVATE_SCROLL                       0x2E        /**< command deactivate scroll */
#define SSD1315_CMD_ACTIVATE_SCROLL                         0x2F        /**< command activate scroll */
#define SSD1315_CMD_DISPLAY_START_LINE                      0x40        /**< command display start line */
#define SSD1315_CMD_CONTRAST_CONTROL                        0x81        /**< command contrast control */
#define SSD1315_CMD_CHARGE_PUMP_SETTING                     0x8D        /**< command charge pump setting */
#define SSD1315_CMD_COLUMN_0_MAPPED_TO_SEG0                 0xA0        /**< command column 0 mapped to seg 0 */
#define SSD1315_CMD_COLUMN_127_MAPPED_TO_SEG0               0xA1        /**< command column 127 mapped to seg 0 */
#define SSD1315_CMD_VERTICAL_SCROLL_AREA                    0xA3        /**< command vertical scroll area */
#define SSD1315_CMD_ENTIRE_DISPLAY_OFF                      0xA4        /**< command entire display off */ 
#define SSD1315_CMD_ENTIRE_DISPLAY_ON                       0xA5        /**< command entire display on */ 
#define SSD1315_CMD_NORMAL_DISPLAY                          0xA6        /**< command normal display */ 
#define SSD1315_CMD_INVERSE_DISPLAY                         0xA7        /**< command inverse display */ 
#define SSD1315_CMD_MULTIPLEX_RATIO                         0xA8        /**< command multiplex ratio */ 
#define SSD1315_CMD_INTERNAL_IREF_SETTING                   0xAD        /**< command internal iref setting */
#define SSD1315_CMD_DISPLAY_OFF                             0xAE        /**< command display off */ 
#define SSD1315_CMD_DISPLAY_ON                              0xAF        /**< command display on */ 
#define SSD1315_CMD_PAGE_ADDR                               0xB0        /**< command page address */ 
#define SSD1315_CMD_SCAN_DIRECTION_COM0_START               0xC0        /**< command scan direction com 0 start */ 
#define SSD1315_CMD_SCAN_DIRECTION_COMN_1_START             0xC8        /**< command scan direction com n-1 start */ 
#define SSD1315_CMD_DISPLAY_OFFSET                          0xD3        /**< command display offset */ 
#define SSD1315_CMD_DISPLAY_CLOCK_DIVIDE                    0xD5        /**< command display clock divide */ 
#define SSD1315_CMD_SET_ZOOM_IN                             0xD6        /**< command set zoom in */ 
#define SSD1315_CMD_PRE_CHARGE_PERIOD                       0xD9        /**< command pre charge period */ 
#define SSD1315_CMD_COM_PINS_CONF                           0xDA        /**< command com pins conf */ 
#define SSD1315_CMD_COMH_DESELECT_LEVEL                     0xDB        /**< command comh deselect level */ 
#define SSD1315_CMD_NOP                                     0xE3        /**< command nop */ 

extern uint8_t FrameBuffer[129 * 8];

extern void OLED_Send(uint8_t *data, uint8_t len);
extern void OLED_Init();
extern void OLED_SendCmd(uint8_t cmd);
extern void OLED_Clear();
extern void OLED_FreshFrame(uint8_t *frameBuffer);
extern void OLED_DrawPixel(uint8_t *frameBuffer, uint8_t x, uint8_t y);
extern void OLED_DrawLine(uint8_t *frameBuffer, uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
extern void OLED_AddLayer(uint8_t *frameBuffer, uint8_t *layer);