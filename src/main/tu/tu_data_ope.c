
#include <check.h>
#include <stdlib.h>
#include "common.h"
#include "data_ope.h"

START_TEST(test_data_ope_01)
{
  STATUS s;
  s = data_ope_init();

  data_msg_t data;
  data.type = RAIN;
  data.container = FLOAT;
  data.value.f = 5.2;
  data_ope_add(&data);

  ck_assert(s == STATUS_OK);
}
END_TEST


Suite* test_suite(void)
{
  Suite* s;
  TCase* tc_core;
  tc_core = tcase_create("data_operation");
  s = suite_create("operation on data");
  suite_add_tcase(s, tc_core);
  tcase_add_test(tc_core, test_data_ope_01);
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
  srunner_set_xml(sr, "test.xml");
  srunner_run_all(sr, CK_VERBOSE);
  number_test_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  return (number_test_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
