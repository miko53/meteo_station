#include "common.h"
#include "drivers/i2c.h"
#include "drivers/pcf_8523.h"
#include "log.h"
#include <stdio.h>

#define PCF8523_I2C_ADDR           (0x68)
#define PCF8523_CTRL1_REG             (0)
#define PCF8523_CTRL2_REG             (1)
#define PCF8523_CTRL3_REG             (2)
#define PCF8523_STATUS_REG            (3)
#define PCF8523_SECOND_REG            (3)

static void pcf8523_set_switchOverInStandardMode(void);
static STATUS pcff8523_write_reg(uint8_t reg, uint8_t value);
static uint8_t pcf8523_read_reg(uint8_t reg);
static uint8_t bcd2bin(uint8_t bcdValue);
static uint8_t bin2bcd(uint8_t binValue);


STATUS pcf_8523_init(void)
{
  STATUS status;
  status = STATUS_OK;

  fprintf(stdout, "CTR1 = 0x%x\n", pcf8523_read_reg(PCF8523_CTRL1_REG));
  fprintf(stdout, "CTR2 = 0x%x\n", pcf8523_read_reg(PCF8523_CTRL2_REG));
  fprintf(stdout, "CTR3 = 0x%x\n", pcf8523_read_reg(PCF8523_CTRL3_REG));
  fprintf(stdout, "STATUS = 0x%x\n", pcf8523_read_reg(PCF8523_STATUS_REG));

  pcf8523_set_switchOverInStandardMode();
  return status;
}

static void pcf8523_set_switchOverInStandardMode(void)
{
  uint8_t ctrl3;
  ctrl3 = pcf8523_read_reg(PCF8523_CTRL3_REG);
  ctrl3 &= ~0xE0;
}

bool pcf8523_hasLostPower(void)
{
  return (pcf8523_read_reg(PCF8523_STATUS_REG) >> 7);
}

uint8_t pcf8523_read_reg(uint8_t reg)
{
  uint8_t result[1];
  STATUS s;
  s = i2c_write(I2C_SENSOR_ID, PCF8523_I2C_ADDR, &reg, 1);
  s |= i2c_read(I2C_SENSOR_ID, PCF8523_I2C_ADDR, result, 1);

  if (s != STATUS_OK)
  {
    log_info_print("rtc error s = %d\n", s);
  }

  return result[0];
}

STATUS pcff8523_write_reg(uint8_t reg, uint8_t value)
{
  uint8_t buffer[2];
  STATUS s;
  buffer[0] = reg;
  buffer[1] = value;
  s = i2c_write(I2C_SENSOR_ID, PCF8523_I2C_ADDR, buffer, 2);
  return s;
}

STATUS pcf8523_set_date(struct tm* pDateTime)
{
  STATUS s;
  uint8_t buffer[8];

  buffer[0] = PCF8523_SECOND_REG;
  buffer[1] = bin2bcd(pDateTime->tm_sec);
  buffer[2] = bin2bcd(pDateTime->tm_min);
  buffer[3] = bin2bcd(pDateTime->tm_hour);
  buffer[4] = bin2bcd(pDateTime->tm_mday);
  buffer[5] = bin2bcd(pDateTime->tm_wday);
  buffer[6] = bin2bcd(pDateTime->tm_mon);
  buffer[7] = bin2bcd(pDateTime->tm_year - 100);

  s = i2c_write(I2C_SENSOR_ID, PCF8523_I2C_ADDR, buffer, 8);
  pcf8523_set_switchOverInStandardMode();
  return s;
}

STATUS pcf8523_get_date(struct tm* pDateTime)
{
  STATUS s;
  uint8_t buffer[7];
  uint8_t second_reg = PCF8523_SECOND_REG;
  s = i2c_write(I2C_SENSOR_ID, PCF8523_I2C_ADDR, &second_reg, 1);
  s |= i2c_read(I2C_SENSOR_ID, PCF8523_I2C_ADDR, buffer, 7);
  if (s == STATUS_OK)
  {
    pDateTime->tm_sec = bcd2bin(buffer[0] & 0x7F);
    pDateTime->tm_min = bcd2bin(buffer[1]);
    pDateTime->tm_hour = bcd2bin(buffer[2]);
    pDateTime->tm_mday = bcd2bin(buffer[3]);
    pDateTime->tm_wday = bcd2bin(buffer[4]);
    pDateTime->tm_mon = bcd2bin(buffer[5]);
    pDateTime->tm_year = bcd2bin(buffer[6]) + 100;
  }

  return s;
}

bool pcf8523_isBattLow(void)
{
  uint8_t v;
  v = pcf8523_read_reg(PCF8523_CTRL3_REG);
  return (v & 0x4) >> 2;
}


static uint8_t bcd2bin(uint8_t bcdValue)
{
  uint8_t binValue;
  binValue = ((bcdValue & 0xF0) >> 4) * 10 + (bcdValue & 0x0F);
  return binValue;
}

static uint8_t bin2bcd(uint8_t binValue)
{
  uint8_t bcdValue;
  bcdValue = ((binValue / 10) << 4) + (binValue % 10);
  return bcdValue;
}
