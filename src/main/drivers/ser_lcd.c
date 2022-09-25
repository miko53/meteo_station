#include <string.h>
#include "common.h"
#include "drivers/io.h"
#include "drivers/ser_lcd.h"
#include "drivers/i2c.h"
#include "os/os.h"

#define SER_LCD_POWER_GPIO                          (32)
#define SER_LCD_I2C_ID                              (I2C_LCD_ID)
#define SER_LCD_I2C_ADDRESS                         (0x72)

#define SER_LCD_MAX_ROWS                            (2)
#define SER_LCD_MAX_COLUMNS                         (16)

///SPECIAL COMMAND
#define SER_LCD_SPECIAL_COMMAND                     (0xFE)  //magic number for sending a special command

#define SER_LCD_RETURN_HOME                         (0x02)
#define SER_LCD_ENTRY_MODE_SET                      (0x04)
#define SER_LCD_DISPLAY_CONTROL                     (0x08)
#define SER_LCD_CURSOR_SHIFT                        (0x10)
#define SER_LCD_SET_DDRAM_ADDR                      (0x80)

//Display ctrl
#define SER_LCD_BLINK_ON                            (0x01)
#define SER_LCD_CURSOR_ON                           (0x02)
#define SER_LCD_DISPLAY_ON                          (0x04)
#define SER_LCD_CURSOR_OFF                          (0x00)
#define SER_LCD_BLINK_OFF                           (0x00)


//Display mode
#define SER_LCD_ENTRY_RIGHT                         (0x00)
#define SER_LCD_ENTRY_LEFT                          (0x02)
#define SER_LCD_ENTRY_SHIFT_INCREMENT               (0x01)
#define SER_LCD_ENTRY_SHIFT_DECREMENT               (0x00)


#define SER_LCD_SETTING_COMMAND                     (0x7C)  //magic number to change settings: baud, lines, width, backlight, splash, etc

#define SER_LCD_CLEAR_COMMAND                       (0x2D)  //command to clear and home the display
#define SER_LCD_CONTRAST_COMMAND                    (0x18)  //command to change the contrast setting
#define SER_LCD_ADDRESS_COMMAND                     (0x19)  //command to change the i2c address
#define SER_LCD_SET_RGB_COMMAND                     (0x2B)  //command to set backlight RGB value
#define SER_LCD_ENABLE_SYSTEM_MESSAGE_DISPLAY       (0x2E)  //command to enable system messages being displayed
#define SER_LCD_DISABLE_SYSTEM_MESSAGE_DISPLAY      (0x2F)  //command to disable system messages being displayed
#define SER_LCD_ENABLE_SPLASH_DISPLAY               (0x30)  //command to enable splash screen at power on
#define SER_LCD_DISABLE_SPLASH_DISPLAY              (0x31)  //command to disable splash screen at power on
#define SER_LCD_SAVE_CURRENT_DISPLAY_AS_SPLASH      (0x0A)  //command to save current text on display as splash

typedef struct
{
  uint8_t displayControl;
  uint8_t displayMode;
} ser_lcd_display_state;

static ser_lcd_display_state ser_lcd_state;

static STATUS ser_lcd_transmit_command(uint8_t command, uint8_t data);


STATUS ser_lcd_init(void)
{
  STATUS status;

  status = io_configure_output(SER_LCD_POWER_GPIO, true);
  thread_msleep(2000);

  ser_lcd_state.displayControl = SER_LCD_DISPLAY_ON | SER_LCD_CURSOR_OFF | SER_LCD_BLINK_OFF;
  ser_lcd_state.displayMode = SER_LCD_ENTRY_LEFT | SER_LCD_ENTRY_SHIFT_DECREMENT;

  //status = ser_lcd_transmit_command(SER_LCD_SETTING_COMMAND, SER_LCD_DISABLE_SPLASH_DISPLAY);
  status = ser_lcd_transmit_command(SER_LCD_SPECIAL_COMMAND, SER_LCD_DISPLAY_CONTROL | ser_lcd_state.displayControl);
  status = ser_lcd_transmit_command(SER_LCD_SPECIAL_COMMAND, SER_LCD_ENTRY_MODE_SET | ser_lcd_state.displayMode);
  ser_lcd_clear_screen();
  thread_msleep(30);
  status = ser_lcd_set_backlight(255, 255, 255);

  //status = STATUS_ERROR;

  return status;
}

STATUS ser_lcd_set_backlight(uint8_t r, uint8_t g, uint8_t b)
{
  STATUS s;

  uint8_t red = 128 + (r * 29) / 255;
  uint8_t green = 158 + (g * 29) / 255;
  uint8_t blue = 188 + (b * 29) / 255;

  ser_lcd_state.displayControl &= ~SER_LCD_DISPLAY_ON;
  s = ser_lcd_transmit_command(SER_LCD_SPECIAL_COMMAND, SER_LCD_DISPLAY_CONTROL | ser_lcd_state.displayControl);
  s = ser_lcd_transmit_command(SER_LCD_SETTING_COMMAND, red);
  s = ser_lcd_transmit_command(SER_LCD_SETTING_COMMAND, green);
  s = ser_lcd_transmit_command(SER_LCD_SETTING_COMMAND, blue);
  ser_lcd_state.displayControl |= SER_LCD_DISPLAY_ON;
  s = ser_lcd_transmit_command(SER_LCD_SPECIAL_COMMAND, SER_LCD_DISPLAY_CONTROL | ser_lcd_state.displayControl);

  return s;
}

STATUS ser_lcd_home(void)
{
  STATUS s;
  s = ser_lcd_transmit_command(SER_LCD_SPECIAL_COMMAND, SER_LCD_RETURN_HOME);
  return s;
}

STATUS ser_lcd_write_screen(char* string)
{
  STATUS s;
  s = ser_lcd_home();
  s = i2c_write(SER_LCD_I2C_ID, SER_LCD_I2C_ADDRESS, (uint8_t*) string, strlen(string));
  return s;
}


STATUS ser_lcd_write_line(uint32_t noLine, char* string)
{
  STATUS s;
  s = ser_lcd_set_cursor(noLine, 0);
  s = ser_lcd_write("                ");
  s = ser_lcd_set_cursor(noLine, 0);
  s = ser_lcd_write(string);
  return s;
}

STATUS ser_lcd_write(char* string)
{
  STATUS s;
  s = i2c_write(SER_LCD_I2C_ID, SER_LCD_I2C_ADDRESS, (uint8_t*) string, strlen(string));
  return s;
}

STATUS ser_lcd_set_cursor(uint8_t row, uint8_t col)
{
  STATUS s;
  const int row_offsets[] = {0x00, 0x40, 0x14, 0x54};

  //kepp variables in bounds
  row = MIN(row, (uint8_t)(SER_LCD_MAX_ROWS - 1));
  s = ser_lcd_transmit_command(SER_LCD_SPECIAL_COMMAND, SER_LCD_SET_DDRAM_ADDR | (col + row_offsets[row]));
  return s;
}

STATUS ser_lcd_clear_screen(void)
{
  STATUS s;
  s = ser_lcd_transmit_command(SER_LCD_SETTING_COMMAND, SER_LCD_CLEAR_COMMAND); //Set green backlight amount
  thread_msleep(10);
  return s;
}

STATUS ser_lcd_display_on()
{
  STATUS s;
  ser_lcd_state.displayControl |= SER_LCD_DISPLAY_ON;
  s = ser_lcd_transmit_command(SER_LCD_SPECIAL_COMMAND, SER_LCD_DISPLAY_CONTROL | ser_lcd_state.displayControl);
  return s;
}

STATUS ser_lcd_display_off()
{
  STATUS s;
  ser_lcd_state.displayControl &= ~SER_LCD_DISPLAY_ON;
  s = ser_lcd_transmit_command(SER_LCD_SPECIAL_COMMAND, SER_LCD_DISPLAY_CONTROL | ser_lcd_state.displayControl);
  return s;
}

STATUS ser_lcd_cursor_on()
{
  STATUS s;
  ser_lcd_state.displayControl |= SER_LCD_CURSOR_ON;
  s = ser_lcd_transmit_command(SER_LCD_SPECIAL_COMMAND, SER_LCD_DISPLAY_CONTROL | ser_lcd_state.displayControl);
  return s;
}

STATUS ser_lcd_cursor_off()
{
  STATUS s;
  ser_lcd_state.displayControl &= ~SER_LCD_CURSOR_ON;
  s = ser_lcd_transmit_command(SER_LCD_SPECIAL_COMMAND, SER_LCD_DISPLAY_CONTROL | ser_lcd_state.displayControl);
  return s;
}

STATUS ser_lcd_blink_on()
{
  STATUS s;
  ser_lcd_state.displayControl |= SER_LCD_BLINK_ON;
  s = ser_lcd_transmit_command(SER_LCD_SPECIAL_COMMAND, SER_LCD_DISPLAY_CONTROL | ser_lcd_state.displayControl);
  return s;
}

STATUS ser_lcd_blink_off()
{
  STATUS s;
  ser_lcd_state.displayControl &= ~SER_LCD_BLINK_ON;
  s = ser_lcd_transmit_command(SER_LCD_SPECIAL_COMMAND, SER_LCD_DISPLAY_CONTROL | ser_lcd_state.displayControl);
  return s;
}


static STATUS ser_lcd_transmit_command(uint8_t command, uint8_t data)
{
  STATUS s;
  uint8_t buffer[2];
  buffer[0] = command;
  buffer[1] = data;

  s = i2c_write(SER_LCD_I2C_ID, SER_LCD_I2C_ADDRESS, buffer, 2);
  return s;
}

