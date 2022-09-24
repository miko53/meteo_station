#include "common.h"
#include "log.h"
#include <stdio.h>
#include "drivers/i2c.h"

void app_main(void)
{
  STATUS s;
  s = i2c_init();
  log_info_print("status s=%d\n", s);
  fprintf(stdout, "Hello World\n");
}
