#include "libs.h"
#include <stdio.h>
#include <time.h>
#include <sys/time.h>

void dump_date(struct tm* pDate)
{
  fprintf(stdout, "%.2d/%.2d/%.4d %.2d:%.2d:%.2d (day = %d)\n",
          pDate->tm_mday,
          pDate->tm_mon + 1,
          pDate->tm_year + 1900,
          pDate->tm_hour,
          pDate->tm_min,
          pDate->tm_sec,
          pDate->tm_wday);
}

void date_get_localtime(struct tm* pDate)
{
  struct timeval currentTime;
  gettimeofday(&currentTime, NULL);
  localtime_r(&currentTime.tv_sec, pDate);
}

void date_set_localtime(struct tm* pDate)
{
  time_t t = mktime(pDate);
  struct timeval tv;
  tv.tv_sec = t;
  tv.tv_usec = 0;
  settimeofday(&tv, NULL);
}
