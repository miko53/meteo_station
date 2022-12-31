#include "ble_date_srv.h"
#include "libs.h"
#include <string.h>

void ble_date_build_frame(uint8_t frame[BLE_DATE_FRAME_SIZE])
{
  struct tm localTime;

  date_get_localtime(&localTime);
  localTime.tm_year += 1900;
  localTime.tm_mon += 1;

  frame[0] = localTime.tm_year & 0xFF;
  frame[1] = (localTime.tm_year & 0xFF00) >> 8;
  frame[2] = (localTime.tm_mon);
  frame[3] = (localTime.tm_mday);
  frame[4] = (localTime.tm_hour);
  frame[5] = (localTime.tm_min);
  frame[6] = (localTime.tm_sec);
  frame[7] = 0;
  frame[8] = 0;
  frame[9] = 0;
}

STATUS ble_date_decode(struct tm* pDate, uint8_t frame[BLE_DATE_FRAME_SIZE])
{
  memset(pDate, 0, sizeof(struct tm));
  pDate->tm_year = (frame[1] << 8) | frame[0];
  pDate->tm_mon = frame[2];
  pDate->tm_mday = frame[3];
  pDate->tm_hour = frame[4];
  pDate->tm_min = frame[5];
  pDate->tm_sec = frame[6];

  pDate->tm_mon -=  1;
  pDate->tm_year -= 1900;
  return STATUS_OK;
}
