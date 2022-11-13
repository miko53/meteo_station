
#include <check.h>
#include <stdlib.h>
#include <stdio.h>
#include "common.h"
#include "config.h"
#include "data_ope.h"

static data_operation_t data_ope_config_list[] =
{
  {
    .sensor = RAIN, .refresh_period_sec = 10,
    .calcul_period = { .type = SLIDING_PERIOD, .period_sec = 10 },
    .operation = OPE_CUMUL, .history_depth = 24, .bStoreInSD = false
  },
};

data_operation_t* get_tu_operation_list(void)
{
  return data_ope_config_list;
}

uint32_t get_tu_operation_nb_items(void)
{
  return sizeof(data_ope_config_list) / sizeof(data_operation_t);
}

#define LAST_VALUE  (0)

///calcul de cumul avec 1 echantillon à chaque et test de l'historique sur une periode de 24
START_TEST(test_data_ope_01)
{
  STATUS s;
  variant r;
  int32_t nbItemsHisto;

  s = data_ope_init(get_tu_operation_list(), get_tu_operation_nb_items());
  ck_assert(s == STATUS_OK);

  //histogram must be empty here
  s = histogram_get(0, LAST_VALUE, &r);
  ck_assert(s == STATUS_ERROR);

  nbItemsHisto = histogram_nbItems(0);
  ck_assert(nbItemsHisto == 0);

  for (uint32_t nbSample = 0; nbSample < 24; nbSample++)
  {
    data_msg_t data;
    data.type = RAIN;
    data.container = FLOAT;
    data.value.f = 1.0;
    data_ope_add(&data);

    nbItemsHisto = histogram_nbItems(0);
    fprintf(stdout, "nbItemsHisto = %d\n", nbItemsHisto);
    ck_assert(nbItemsHisto == (nbSample + 1));

    s = histogram_get(0, LAST_VALUE, &r);
    ck_assert(s == STATUS_OK);
    fprintf(stdout, "r = %f\n", r.f32);
  }
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
