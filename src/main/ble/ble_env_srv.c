#include "ble_env_srv.h"


void build_frame(uint8_t frame[ES_MEASUREMENT_DESC_LEN], gatt_es_measurement_desc* esDesc)
{
  frame[0] = 0;
  frame[1] = 0;
  frame[2] = esDesc->sampling_fun;
  frame[3] = (esDesc->measurement_period & 0xFF);
  frame[4] = (esDesc->measurement_period & 0xFF00) >> 8;
  frame[5] = (esDesc->measurement_period & 0xFF0000) >> 16;
  frame[6] = (esDesc->update_interval & 0xFF);
  frame[7] = (esDesc->update_interval & 0xFF00) >> 8;
  frame[8] = (esDesc->update_interval & 0xFF0000) >> 16;
  frame[9] = (esDesc->application);
  frame[10] = (esDesc->measurement_incertainty);
}

es_sampling_fun_t gatt_desc_get_sampling_func(data_calcul dataCalOpe)
{
  es_sampling_fun_t r;

  switch (dataCalOpe)
  {
    case OPE_AVERAGE:
      r = SAMPLING_FUN_ARITHMETIC_MEAN;
      break;
    case OPE_CUMUL:
      r = SAMPLING_FUN_ACCUMULATED;
      break;
    case OPE_MAX:
      r = SAMPLING_FUN_MAXIMUM;
      break;
    case OPE_MIN:
      r = SAMPLING_FUN_MINIMUM;
      break;
    default:
      r = -1;
      break;
  }
  return r;
}
