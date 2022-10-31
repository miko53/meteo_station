#ifndef __DATA_DEFS_H__
#define __DATA_DEFS_H__

typedef enum
{
  TEMPERATURE,
  HUMIDITY,
  RAIN,
  WIND_SPEED,
  WIND_DIR
} data_type_t;

typedef enum
{
  FLOAT,
  INTEGER_32
} data_container;

typedef union
{
  float f;
  uint32_t i;
} data_v_t;

typedef struct
{
  data_type_t type;
  data_container container;
  data_v_t value;
} data_msg_t;


#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif /* __DATA_DEFS_H__ */
