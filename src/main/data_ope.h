#ifndef __DATA_OPE_H__
#define __DATA_OPE_H__

#include "common.h"
#include "data_defs.h"
#include "histogram.h"

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

typedef struct
{
  data_operation_t* pDataOpeList;
  uint32_t nbItemsInList;
  void (*on_new_calculated_data)(uint32_t operationIndex, variant_t* data);
} data_ope_cnf;

#ifdef __cplusplus
extern "C" {
#endif

extern STATUS data_ope_init(data_ope_cnf* pConfig);
extern void data_ope_add_sample(data_type_t dataType, variant_t* pSample);

extern uint32_t data_ope_nb_operation(void);
extern data_operation_t* date_ope_get_operation(uint32_t indexOperation);

extern histogram_t* data_ope_get_histo(uint32_t indexOperation);

extern void data_ope_activate_all(void);

#ifdef __cplusplus
}
#endif

#endif /* __DATA_OPE_H__ */

