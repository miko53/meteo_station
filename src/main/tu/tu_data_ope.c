
#include <check.h>
#include <stdlib.h>
#include <stdio.h>
#include "common.h"
#include "config.h"
#include "data_ope.h"
#include "histogram.h"
#include "libs.h"

void date_get_localtime(struct tm* pDate)
{
  UNUSED(pDate);
  //not used for these test cases
}

static data_operation_t data_ope_config_list_test1[] =
{
  {
    .sensor = RAIN, .refresh_period_sec = 10,
    .calcul_period = { .type = SLIDING_PERIOD, .period_sec = 10 },
    .operation = OPE_CUMUL, .history_depth = 24, .bStoreInSD = false
  },
};

data_operation_t* get_tu_operation_list_test_1(void)
{
  return data_ope_config_list_test1;
}

uint32_t get_tu_operation_nb_items_test_1(void)
{
  return sizeof(data_ope_config_list_test1) / sizeof(data_operation_t);
}


///calcul de cumul avec 1 echantillon à chaque et test de l'historique sur une periode de 24
START_TEST(test_data_ope_cumul_01)
{
  STATUS s;
  variant r;
  int32_t nbItemsHisto;
  histogram_t* histo;

  s = data_ope_init(get_tu_operation_list_test_1(), get_tu_operation_nb_items_test_1());
  ck_assert(s == STATUS_OK);

  data_ope_activate_all();

  histo = data_ope_get_histo(0);
  ck_assert(histo != NULL);

  //histogram must be empty here
  s = histogram_get(histo, LAST_VALUE, &r);
  ck_assert(s == STATUS_ERROR);

  nbItemsHisto = histogram_nbItems(histo);
  ck_assert(nbItemsHisto == 0);

  //check ramp of data until histo are filled
  for (int32_t nbSample = 0; nbSample < 30; nbSample++)
  {
    data_msg_t data;
    data.type = RAIN;
    data.container = FLOAT;
    data.value.f = 1.0;
    data_ope_add_sample(&data);

    nbItemsHisto = histogram_nbItems(histo);
    fprintf(stdout, "nbItemsHisto = %d\n", nbItemsHisto);
    if (nbSample < 24)
      ck_assert(nbItemsHisto == (nbSample + 1));
    else
      ck_assert(nbItemsHisto == 24);

    s = histogram_get(histo, LAST_VALUE, &r);
    ck_assert(s == STATUS_OK);
    fprintf(stdout, "nbSample = %d, r = %f\n", nbSample + 1, r.f32);
    ck_assert_float_eq_tol(r.f32, 1.0, 0.1);
  }
}
END_TEST


static data_operation_t data_ope_config_list_test2[] =
{
  {
    .sensor = RAIN, .refresh_period_sec = 10,
    .calcul_period = { .type = SLIDING_PERIOD, .period_sec = 3600 },
    .operation = OPE_CUMUL, .history_depth = 24, .bStoreInSD = false
  },
};

data_operation_t* get_tu_operation_list_test_2(void)
{
  return data_ope_config_list_test2;
}

uint32_t get_tu_operation_nb_items_test_2(void)
{
  return sizeof(data_ope_config_list_test2) / sizeof(data_operation_t);
}


///calcul de cumul avec x echantillons à chaque et test de l'historique sur une periode de 24
START_TEST(test_data_ope_cumul_02)
{
  STATUS s;
  variant r;
  int32_t nbItemsHisto;
  histogram_t* histo;

  s = data_ope_init(get_tu_operation_list_test_2(), get_tu_operation_nb_items_test_2());
  ck_assert(s == STATUS_OK);

  data_ope_activate_all();

  histo = data_ope_get_histo(0);
  ck_assert(histo != NULL);

  for (int32_t nbPeriod = 0; nbPeriod < 24; nbPeriod++)
  {
    //check ramp of data until histo are filled
    for (uint32_t nbSample = 0; nbSample < 360; nbSample++)
    {
      data_msg_t data;
      data.type = RAIN;
      data.container = FLOAT;
      data.value.f = 2.0;
      data_ope_add_sample(&data);
    }

    nbItemsHisto = histogram_nbItems(histo);
    fprintf(stdout, "nbItemsHisto = %d\n", nbItemsHisto);
    ck_assert(nbItemsHisto == (nbPeriod + 1));

    s = histogram_get(histo, LAST_VALUE, &r);
    ck_assert(s == STATUS_OK);
    fprintf(stdout, "nbItemsHisto = %d, r = %f\n", nbItemsHisto, r.f32);
    ck_assert_float_eq_tol(r.f32, 360 * 2.0, 0.1);
  }
}
END_TEST


static data_operation_t data_ope_config_list_avg_1[] =
{
  {
    .sensor = WIND_SPEED, .refresh_period_sec = 1,
    .calcul_period = { .type = SLIDING_PERIOD, .period_sec = 10 },
    .operation = OPE_AVERAGE, .history_depth = 60, .bStoreInSD = false
  },
};


//calcul de moyenne sur une period donnee un echantillon
START_TEST(test_data_ope_avg_01)
{
  STATUS s;
  variant r;
  int32_t nbItemsHisto;
  histogram_t* histo;

  s = data_ope_init(data_ope_config_list_avg_1, 1);
  ck_assert(s == STATUS_OK);

  data_ope_activate_all();

  histo = data_ope_get_histo(0);
  ck_assert(histo != NULL);

  //check ramp of data until histo are filled
  for (int32_t nbPeriod = 0; nbPeriod < 120; nbPeriod++)
  {
    for (int32_t nbSample = 0; nbSample < 10; nbSample++)
    {
      data_msg_t data;
      data.type = WIND_SPEED;
      data.container = FLOAT;
      data.value.f = 10.0 + nbPeriod + nbSample;
      data_ope_add_sample(&data);
    }
    nbItemsHisto = histogram_nbItems(histo);
    fprintf(stdout, "nbItemsHisto = %d\n", nbItemsHisto);
    if (nbPeriod < 60)
      ck_assert(nbItemsHisto == (nbPeriod + 1));
    else
      ck_assert(nbItemsHisto == 60);

    s = histogram_get(histo, LAST_VALUE, &r);
    ck_assert(s == STATUS_OK);
    fprintf(stdout, "nbPeriod = %d, r = %f\n", nbPeriod + 1, r.f32);
    ck_assert_float_eq_tol(r.f32, 14.5 + nbPeriod, 0.1);
  }
}
END_TEST

static data_operation_t data_ope_config_list_min_1[] =
{
  {
    .sensor = WIND_SPEED, .refresh_period_sec = 1,
    .calcul_period = { .type = SLIDING_PERIOD, .period_sec = 10 },
    .operation = OPE_MIN, .history_depth = 60, .bStoreInSD = false
  },
};


//calcul de minimum sur une period donnee un echantillon
START_TEST(test_data_ope_min_01)
{
  STATUS s;
  variant r;
  int32_t nbItemsHisto;
  histogram_t* histo;

  s = data_ope_init(data_ope_config_list_min_1, 1);
  ck_assert(s == STATUS_OK);

  data_ope_activate_all();

  histo = data_ope_get_histo(0);
  ck_assert(histo != NULL);

#define FIRST_PERIOD      (10)
  //check ramp of data until histo are filled min at begin
  for (int32_t nbPeriod = 0; nbPeriod < FIRST_PERIOD; nbPeriod++)
  {
    for (int32_t nbSample = 0; nbSample < 10; nbSample++)
    {
      data_msg_t data;
      data.type = WIND_SPEED;
      data.container = FLOAT;
      data.value.f = 10.0 + nbPeriod + nbSample;
      data_ope_add_sample(&data);
    }
    nbItemsHisto = histogram_nbItems(histo);
    fprintf(stdout, "nbItemsHisto = %d\n", nbItemsHisto);
    if (nbPeriod < 60)
      ck_assert(nbItemsHisto == (nbPeriod + 1));
    else
      ck_assert(nbItemsHisto == 60);

    s = histogram_get(histo, LAST_VALUE, &r);
    ck_assert(s == STATUS_OK);
    fprintf(stdout, "nbPeriod = %d, r = %f\n", nbPeriod + 1, r.f32);
    ck_assert_float_eq_tol(r.f32, 10.0 + nbPeriod, 0.1);
  }

  //check ramp of data until histo are filled min at end
  for (int32_t nbPeriod = 0; nbPeriod < 10; nbPeriod++)
  {
    for (int32_t nbSample = 0; nbSample < 10; nbSample++)
    {
      data_msg_t data;
      data.type = WIND_SPEED;
      data.container = FLOAT;
      data.value.f = 10.0 - nbPeriod - nbSample;
      data_ope_add_sample(&data);
    }
    nbItemsHisto = histogram_nbItems(histo);
    fprintf(stdout, "nbItemsHisto = %d\n", nbItemsHisto);
    if (nbPeriod < 60)
      ck_assert(nbItemsHisto == (nbPeriod + FIRST_PERIOD + 1));
    else
      ck_assert(nbItemsHisto == 60);

    s = histogram_get(histo, LAST_VALUE, &r);
    ck_assert(s == STATUS_OK);
    fprintf(stdout, "nbPeriod = %d, r = %f\n", nbPeriod + 1, r.f32);
    ck_assert_float_eq_tol(r.f32, 10.0 - nbPeriod - 9, 0.1);
  }
}
END_TEST

static data_operation_t data_ope_config_list_max_1[] =
{
  {
    .sensor = WIND_SPEED, .refresh_period_sec = 1,
    .calcul_period = { .type = SLIDING_PERIOD, .period_sec = 10 },
    .operation = OPE_MAX, .history_depth = 60, .bStoreInSD = false
  },
};

//calcul de minimum sur une period donnee un echantillon
START_TEST(test_data_ope_max_01)
{
  STATUS s;
  variant r;
  int32_t nbItemsHisto;
  histogram_t* histo;

  s = data_ope_init(data_ope_config_list_max_1, 1);
  ck_assert(s == STATUS_OK);

  data_ope_activate_all();

  histo = data_ope_get_histo(0);
  ck_assert(histo != NULL);

#define FIRST_PERIOD      (10)
  //check ramp of data until histo are filled max at end
  for (int32_t nbPeriod = 0; nbPeriod < FIRST_PERIOD; nbPeriod++)
  {
    for (int32_t nbSample = 0; nbSample < 10; nbSample++)
    {
      data_msg_t data;
      data.type = WIND_SPEED;
      data.container = FLOAT;
      data.value.f = 10.0 + nbPeriod + nbSample;
      data_ope_add_sample(&data);
    }
    nbItemsHisto = histogram_nbItems(histo);
    fprintf(stdout, "nbItemsHisto = %d\n", nbItemsHisto);
    if (nbPeriod < 60)
      ck_assert(nbItemsHisto == (nbPeriod + 1));
    else
      ck_assert(nbItemsHisto == 60);

    s = histogram_get(histo, LAST_VALUE, &r);
    ck_assert(s == STATUS_OK);
    fprintf(stdout, "nbPeriod = %d, r = %f\n", nbPeriod + 1, r.f32);
    ck_assert_float_eq_tol(r.f32, 10.0 + nbPeriod + 9.0, 0.1);
  }

  //check ramp of data until histo are filled max at begin
  for (int32_t nbPeriod = 0; nbPeriod < 10; nbPeriod++)
  {
    for (int32_t nbSample = 0; nbSample < 10; nbSample++)
    {
      data_msg_t data;
      data.type = WIND_SPEED;
      data.container = FLOAT;
      data.value.f = 10.0 - nbPeriod - nbSample;
      data_ope_add_sample(&data);
    }
    nbItemsHisto = histogram_nbItems(histo);
    fprintf(stdout, "nbItemsHisto = %d\n", nbItemsHisto);
    if (nbPeriod < 60)
      ck_assert(nbItemsHisto == (nbPeriod + FIRST_PERIOD + 1));
    else
      ck_assert(nbItemsHisto == 60);

    s = histogram_get(histo, LAST_VALUE, &r);
    ck_assert(s == STATUS_OK);
    fprintf(stdout, "nbPeriod = %d, r = %f\n", nbPeriod + 1, r.f32);
    ck_assert_float_eq_tol(r.f32, 10.0 - nbPeriod, 0.1);
  }
}
END_TEST

static data_operation_t data_ope_config_list_wrong_period[] =
{
  {
    .sensor = WIND_DIR, .refresh_period_sec = 10,
    .calcul_period = { .type = SLIDING_PERIOD, .period_sec = 15 },
    .operation = OPE_MAX, .history_depth = 60, .bStoreInSD = false
  },
};

static data_operation_t data_ope_config_list_avg_2[] =
{
  {
    .sensor = WIND_SPEED, .refresh_period_sec = 1,
    .calcul_period = { .type = SLIDING_PERIOD, .period_sec = 5 },
    .operation = OPE_AVERAGE, .history_depth = 15, .bStoreInSD = false
  },
};

static float data_ope_result_for_avg2_robustness_1[] =
{
  27.5,
  26.5,
  25.5,
  24.5,
  23.5,
  22.5,
  21.5,
  20.5,
  19.5,
  18.5,
  17.5,
  16.5,
  15.5,
  14.5,
  13.5
};

START_TEST(test_data_ope_robustness_1)
{
  variant r;
  int32_t nbItemsHisto;
  STATUS s, s2;
  histogram_t* histo;

  s = data_ope_init(data_ope_config_list_wrong_period, 1);
  ck_assert(s == STATUS_ERROR);

  s = data_ope_init(data_ope_config_list_avg_2, 1);
  ck_assert(s == STATUS_OK);

  data_ope_activate_all();

  histo = data_ope_get_histo(0);
  ck_assert(histo != NULL);

  //check ramp of data until histo are filled

  for (int32_t nbPeriod = 0; nbPeriod < 15; nbPeriod++)
  {
    for (int32_t nbSample = 0; nbSample < 5; nbSample++)
    {
      data_msg_t data;
      data.type = WIND_SPEED;
      data.container = FLOAT;
      data.value.f = 11.5 + nbPeriod + nbSample;
      data_ope_add_sample(&data);
    }

    nbItemsHisto = histogram_nbItems(histo);
    fprintf(stdout, "nbItemsHisto = %d\n", nbItemsHisto);

    for (int32_t i = 0; i < 15; i++)
    {
      if (i < nbItemsHisto)
      {
        s = STATUS_OK;
      }
      else
        s = STATUS_ERROR;
      s2 = histogram_get(histo, i, &r);
      ck_assert(s == s2);
    }
  }

  //check wrong index of data

  int32_t r2 = histogram_nbItems(NULL);
  ck_assert(r2 == -1);

  s2 = histogram_get(NULL, LAST_VALUE, &r);
  ck_assert(s2 == STATUS_ERROR);

  nbItemsHisto = histogram_nbItems(histo);
  fprintf(stdout, "nbItemsHisto = %d\n", nbItemsHisto);
  ck_assert(nbItemsHisto == 15);
  for (int32_t i = 0; i < nbItemsHisto; i++)
  {
    s2 = histogram_get(histo, i, &r);
    //fprintf(stdout, "i= %d, r = %f, data_ope_result_for_avg2_robustness_1[] = %f \n", -i, r.f32, data_ope_result_for_avg2_robustness_1[i]);
    ck_assert_float_eq_tol(r.f32, data_ope_result_for_avg2_robustness_1[i], 0.1);
  }

  histo = data_ope_get_histo(1);
  ck_assert(histo == NULL);
}
END_TEST

Suite* test_suite(void)
{
  Suite* s;
  TCase* tc_core;
  tc_core = tcase_create("data_operation");
  s = suite_create("operation on data");
  suite_add_tcase(s, tc_core);
  tcase_add_test(tc_core, test_data_ope_cumul_01);
  tcase_add_test(tc_core, test_data_ope_cumul_02);
  tcase_add_test(tc_core, test_data_ope_avg_01);
  tcase_add_test(tc_core, test_data_ope_min_01);
  tcase_add_test(tc_core, test_data_ope_max_01);
  tcase_add_test(tc_core, test_data_ope_robustness_1);
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
