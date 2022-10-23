
#include "libs.h"
#include <stdio.h>

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

