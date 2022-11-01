#ifndef __DATA_OPE_H__
#define __DATA_OPE_H__

#include "common.h"
#include "data_defs.h"

typedef enum
{
  OPE_CUMUL,
  OPE_AVERAGE,
  OPE_MIN,
  OPE_MAX
} data_calcul;

typedef enum
{
  SLIDING_PERIOD,
  FIXED_PERIOD
} period_def;

typedef enum
{
  BY_HOUR,
  BY_DAY,
  BY_MONTH
  //BY_YEAR
} fixed_period_unit;

typedef struct
{
  uint32_t period;
  fixed_period_unit unit;
} fixed_period_t;

typedef struct
{
  period_def type;
  union
  {
    uint32_t period;
    fixed_period_t f_period;
  };
} period_t;

typedef struct
{
  data_type_t sensor;
  uint32_t refresh_period;
  period_t calcul_period;
  data_calcul operation;
  uint32_t history_depth;
  bool bStoreInSD;
} data_operation_t;


#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif /* __DATA_OPE_H__ */

