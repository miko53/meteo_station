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
    uint32_t period_sec;
    fixed_period_t f_period;
  };
} period_t;

typedef struct
{
  data_type_t sensor;
  uint32_t refresh_period_sec;
  period_t calcul_period;
  data_calcul operation;
  uint32_t history_depth;
  bool bStoreInSD;
} data_operation_t;

typedef union
{
  float f32;
  uint32_t i32;
} variant;

#define LAST_VALUE  (0)

#ifdef __cplusplus
extern "C" {
#endif

extern STATUS data_ope_init(data_operation_t pDataOpeList[], uint32_t nbItemsInList);
extern void data_ope_add(data_msg_t* pData);

extern STATUS histogram_get(uint32_t histoIndex, uint32_t index, variant* v);
extern int32_t histogram_nbItems(uint32_t histoIndex);

#ifdef __cplusplus
}
#endif

#endif /* __DATA_OPE_H__ */

