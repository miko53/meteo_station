#ifndef __SER_LCD_H__
#define __SER_LCD_H__

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

extern STATUS ser_lcd_init(void);
extern STATUS ser_lcd_set_backlight(uint8_t r, uint8_t g, uint8_t b);
extern STATUS ser_lcd_write_screen(char* string);
extern STATUS ser_lcd_write(char* string);
extern STATUS ser_lcd_set_cursor(uint8_t row, uint8_t col);
extern STATUS ser_lcd_write_line(uint32_t noLine, char* string);
extern STATUS ser_lcd_clear_screen(void);

#ifdef __cplusplus
}
#endif

#endif /* __SER_LCD_H__ */

