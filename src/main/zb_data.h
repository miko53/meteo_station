#ifndef __ZB_DATA_H__
#define __ZB_DATA_H__
#include "common.h"

#define ZB_DATA_MAX_SENSOR      (8)

typedef struct
{
  uint8_t type;
  uint8_t status;
  uint16_t data;
} __attribute__((packed)) sensorData;

typedef struct
{
  uint8_t sensorDataNumber;
  sensorData sensors[ZB_DATA_MAX_SENSOR];
} __attribute__((packed)) sensorFrameStruct;


typedef struct
{
  uint8_t v1;
  uint8_t v2;
  uint8_t v3;
} __attribute__((packed)) sensorDbgFrameStruct;

typedef enum
{
  SENSOR_PROTOCOL_DATA_TYPE = 0x00,
  SENSOR_PROTOCOL_DBG_TYPE  = 0x01
} zb_payload_type;

typedef struct
{
  uint8_t dataType;
  uint8_t counter;
  union
  {
    sensorFrameStruct frame;
    sensorDbgFrameStruct dbgFrame;
  };
} __attribute__((packed)) zb_payload_frame;

typedef enum
{
  SENSOR_HYT221_TEMP = 0x01,
  SENSOR_HYT221_HUM = 0x02,
  SENSOR_VOLTAGE = 0x03,
  SENSOR_WIND_SPEED = 0x04, //m.s-1 *10
  SENSOR_WIND_DIR = 0x05,  //deg * 10
  SENSOR_PRESSURE = 0x06, // hpa *10
  SENSOR_RAINFALL = 0x07, // mm
  ACT_HEATER = 0x81
} sensor_Type;

#ifdef __cplusplus
extern "C" {
#endif
#ifdef __cplusplus
}
#endif
#endif /* __ZB_DATA_H__ */
