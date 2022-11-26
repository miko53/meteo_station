#ifndef __DATA_DEFS_H__
#define __DATA_DEFS_H__

#include "variant.h"

typedef enum
{
  TEMPERATURE,
  HUMIDITY,
  RAIN,
  WIND_SPEED,
  WIND_DIR
} data_type_t;

typedef struct
{
  data_type_t sensor;
  variant_t value;
} data_msg_t;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif /* __DATA_DEFS_H__ */
