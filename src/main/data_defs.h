#ifndef __DATA_DEFS_H__
#define __DATA_DEFS_H__

#include "variant.h"

typedef enum
{
  RAIN,
  WIND_SPEED,
  WIND_DIR,
  TEMPERATURE,
  HUMIDITY,
  PRESSURE,
  NB_DATA_TYPE
} data_type_t;

extern const char* sensor_name[NB_DATA_TYPE]; 
extern const char* sensor_unit[NB_DATA_TYPE];


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
