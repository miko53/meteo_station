#include <check.h>
#include <stdlib.h>
#include <stdio.h>
#include "common.h"
#include "config.h"
#include "data_ope.h"
#include "libs.h"

struct tm date_get_localtime_r;

void localtime_add_min(struct tm* pDate, uint32_t min);
void localtime_add_hour(struct tm* pDate, uint32_t hour);
void localtime_add_day(struct tm* pDate, uint32_t day);
void localtime_add_month(struct tm* pDate, uint32_t month);
void localtime_add_year(struct tm* pDate, uint32_t year);
void localtime_display(struct tm* pDate);

void date_get_localtime(struct tm* pDate)
{
  UNUSED(pDate);
  //not used for these test cases
  *pDate = date_get_localtime_r;
}

void tu_set_localtime(struct tm* pDate)
{
  date_get_localtime_r = *pDate;
}

void localtime_add_min(struct tm* pDate, uint32_t min)
{
  uint32_t r;
  pDate->tm_min += min;
  r = pDate->tm_min / 60;
  if (r != 0)
  {
    localtime_add_hour(pDate, 1);
    pDate->tm_min = pDate->tm_min % 60;
  }
}

void localtime_add_hour(struct tm* pDate, uint32_t hour)
{
  uint32_t r;
  pDate->tm_hour += hour;
  r = pDate->tm_hour / 24;
  if (r != 0)
  {
    localtime_add_day(pDate, 1);
    pDate->tm_hour = pDate->tm_hour % 24;
  }
}

uint32_t month_conversion[] =
{
  31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

void localtime_add_day(struct tm* pDate, uint32_t day)
{
  uint32_t r;
  uint32_t d;
  pDate->tm_mday += day;

  if ((pDate->tm_mon == 1) && ((pDate->tm_year % 4) == 0))
  {
    d = month_conversion[pDate->tm_mon] + 1;
  }
  else
    d = month_conversion[pDate->tm_mon];

  r = (pDate->tm_mday + 1) / d;
  if (r != 0)
  {
    localtime_add_month(pDate, 1);
    pDate->tm_mday = (pDate->tm_mday % d);
    if (pDate->tm_mday == 0)
      pDate->tm_mday = 1;
  }
}

void localtime_add_month(struct tm* pDate, uint32_t month)
{
  uint32_t r;
  pDate->tm_mon += month;

  r = pDate->tm_mon / 12;
  if (r != 0)
  {
    localtime_add_year(pDate, 1);
    pDate->tm_mon = pDate->tm_mon % 12;
  }
}

void localtime_add_year(struct tm* pDate, uint32_t year)
{
  pDate->tm_year += year;
}

void localtime_display(struct tm* pDate)
{
  fprintf(stdout, "%.2d:%.2d:%.2d %.2d/%.2d/%.4d\n",
          pDate->tm_hour, pDate->tm_min, pDate->tm_sec,
          pDate->tm_mday, pDate->tm_mon + 1, pDate->tm_year + 1900);
}



START_TEST(test_localdate_update)
{
  struct tm currentDate;
  currentDate.tm_sec = 15;
  currentDate.tm_min = 32;
  currentDate.tm_hour = 9;
  currentDate.tm_mday = 10;
  currentDate.tm_mon = 10;
  currentDate.tm_year = 122;

  localtime_add_min(&currentDate, 1);

  ck_assert(currentDate.tm_sec == 15);
  ck_assert(currentDate.tm_min == 33);
  ck_assert(currentDate.tm_hour == 9);
  ck_assert(currentDate.tm_mday == 10);
  ck_assert(currentDate.tm_mon == 10);
  ck_assert(currentDate.tm_year == 122);

  localtime_add_min(&currentDate, 27);

  ck_assert(currentDate.tm_sec == 15);
  ck_assert(currentDate.tm_min == 00);
  ck_assert(currentDate.tm_hour == 10);
  ck_assert(currentDate.tm_mday == 10);
  ck_assert(currentDate.tm_mon == 10);
  ck_assert(currentDate.tm_year == 122);

  localtime_add_hour(&currentDate, 3);

  ck_assert(currentDate.tm_sec == 15);
  ck_assert(currentDate.tm_min == 00);
  ck_assert(currentDate.tm_hour == 13);
  ck_assert(currentDate.tm_mday == 10);
  ck_assert(currentDate.tm_mon == 10);
  ck_assert(currentDate.tm_year == 122);

  localtime_add_hour(&currentDate, 12);
  ck_assert(currentDate.tm_sec == 15);
  ck_assert(currentDate.tm_min == 00);
  ck_assert(currentDate.tm_hour == 01);
  ck_assert(currentDate.tm_mday == 11);
  ck_assert(currentDate.tm_mon == 10);
  ck_assert(currentDate.tm_year == 122);

  localtime_add_day(&currentDate, 1);
  ck_assert(currentDate.tm_sec == 15);
  ck_assert(currentDate.tm_min == 00);
  ck_assert(currentDate.tm_hour == 01);
  ck_assert(currentDate.tm_mday == 12);
  ck_assert(currentDate.tm_mon == 10);
  ck_assert(currentDate.tm_year == 122);

  localtime_add_day(&currentDate, 20);

  ck_assert(currentDate.tm_sec == 15);
  ck_assert(currentDate.tm_min == 00);
  ck_assert(currentDate.tm_hour == 01);
  ck_assert(currentDate.tm_mday == 2);
  ck_assert(currentDate.tm_mon == 11);
  ck_assert(currentDate.tm_year == 122);

  localtime_add_month(&currentDate, 1);

  ck_assert(currentDate.tm_sec == 15);
  ck_assert(currentDate.tm_min == 00);
  ck_assert(currentDate.tm_hour == 01);
  ck_assert(currentDate.tm_mday == 2);
  ck_assert(currentDate.tm_mon == 0);
  ck_assert(currentDate.tm_year == 123);

  localtime_add_month(&currentDate, 1);

  ck_assert(currentDate.tm_sec == 15);
  ck_assert(currentDate.tm_min == 00);
  ck_assert(currentDate.tm_hour == 01);
  ck_assert(currentDate.tm_mday == 2);
  ck_assert(currentDate.tm_mon == 1);
  ck_assert(currentDate.tm_year == 123);

  localtime_add_year(&currentDate, 1);
  ck_assert(currentDate.tm_sec == 15);
  ck_assert(currentDate.tm_min == 00);
  ck_assert(currentDate.tm_hour == 01);
  ck_assert(currentDate.tm_mday == 2);
  ck_assert(currentDate.tm_mon == 1);
  ck_assert(currentDate.tm_year == 124);

  localtime_add_day(&currentDate, 29);
  ck_assert(currentDate.tm_sec == 15);
  ck_assert(currentDate.tm_min == 00);
  ck_assert(currentDate.tm_hour == 01);
  ck_assert(currentDate.tm_mday == 2);
  ck_assert(currentDate.tm_mon == 2);
  ck_assert(currentDate.tm_year == 124);

  currentDate.tm_sec = 15;
  currentDate.tm_min = 00;
  currentDate.tm_hour = 1;
  currentDate.tm_mday = 2;
  currentDate.tm_mon = 1;
  currentDate.tm_year = 122;
  localtime_add_day(&currentDate, 29);

  ck_assert(currentDate.tm_sec == 15);
  ck_assert(currentDate.tm_min == 00);
  ck_assert(currentDate.tm_hour == 01);
  ck_assert(currentDate.tm_mday == 3);
  ck_assert(currentDate.tm_mon == 2);
  ck_assert(currentDate.tm_year == 122);

  localtime_display(&currentDate);
  localtime_add_day(&currentDate, 29);
  localtime_display(&currentDate);
  ck_assert(currentDate.tm_sec == 15);
  ck_assert(currentDate.tm_min == 00);
  ck_assert(currentDate.tm_hour == 01);
  ck_assert(currentDate.tm_mday == 1);
  ck_assert(currentDate.tm_mon == 3);
  ck_assert(currentDate.tm_year == 122);

  currentDate.tm_sec = 15;
  currentDate.tm_min = 00;
  currentDate.tm_hour = 1;
  currentDate.tm_mday = 3;
  currentDate.tm_mon = 2;
  currentDate.tm_year = 122;

  localtime_display(&currentDate);
  localtime_add_day(&currentDate, 28);
  localtime_display(&currentDate);
  ck_assert(currentDate.tm_sec == 15);
  ck_assert(currentDate.tm_min == 00);
  ck_assert(currentDate.tm_hour == 01);
  ck_assert(currentDate.tm_mday == 1);
  ck_assert(currentDate.tm_mon == 3);
  ck_assert(currentDate.tm_year == 122);

  currentDate.tm_sec = 15;
  currentDate.tm_min = 00;
  currentDate.tm_hour = 1;
  currentDate.tm_mday = 3;
  currentDate.tm_mon = 2;
  currentDate.tm_year = 122;
  localtime_display(&currentDate);
  localtime_add_day(&currentDate, 27);
  localtime_display(&currentDate);
  ck_assert(currentDate.tm_sec == 15);
  ck_assert(currentDate.tm_min == 00);
  ck_assert(currentDate.tm_hour == 01);
  ck_assert(currentDate.tm_mday == 30);
  ck_assert(currentDate.tm_mon == 3);
  ck_assert(currentDate.tm_year == 122);

}
END_TEST

static data_operation_t data_ope_config_list_test1[] =
{
  {
    .sensor = RAIN, .refresh_period_sec = 10,
    .calcul_period = { .type = FIXED_PERIOD, .f_period = {.period = 1, .unit = BY_HOUR }},
    .operation = OPE_CUMUL, .history_depth = 24, .bStoreInSD = false
  },
};


START_TEST(test_date_fp_one_hour)
{
  STATUS s;
  s = data_ope_init(data_ope_config_list_test1, 1);
  ck_assert(s == STATUS_OK);

  struct tm currentDate;
  currentDate.tm_sec = 15;
  currentDate.tm_min = 32;
  currentDate.tm_hour = 9;
  currentDate.tm_mday = 10;
  currentDate.tm_mon = 10;
  currentDate.tm_year = 122;

  tu_set_localtime(&currentDate);

  data_msg_t data;
  data.type = RAIN;
  data.container = FLOAT;
  data.value.f = 10.0;
  data_ope_add(&data);


}
END_TEST


Suite* test_suite(void)
{
  Suite* s;
  TCase* tc_core;
  tc_core = tcase_create("data_operation_fix_period");
  s = suite_create("operation on data");
  suite_add_tcase(s, tc_core);
  tcase_add_test(tc_core, test_localdate_update);
  tcase_add_test(tc_core, test_date_fp_one_hour);
  //   tcase_add_test(tc_core, test_data_ope_cumul_02);
  //   tcase_add_test(tc_core, test_data_ope_avg_01);
  //   tcase_add_test(tc_core, test_data_ope_min_01);
  //   tcase_add_test(tc_core, test_data_ope_max_01);
  //   tcase_add_test(tc_core, test_data_ope_robustness_1);
  return s;
}

int main(int argc, char* argv[])
{
  UNUSED(argc);
  UNUSED(argv);
  int number_test_failed;
  Suite* s;
  SRunner* sr;

  s = test_suite();
  sr = srunner_create(s);
  srunner_set_xml(sr, "test_fix_period.xml");
  srunner_run_all(sr, CK_VERBOSE);
  number_test_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  return (number_test_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

