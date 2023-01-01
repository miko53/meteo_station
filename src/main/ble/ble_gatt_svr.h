#ifndef __BLE_GATT_SVR_H__
#define __BLE_GATT_SVR_H__

#include "nimble/ble.h"
#include "common.h"

typedef enum
{
  NOTIFY_HANDLE_TRUE_WIND_SPEED,
  NOTIFY_HANDLE_TRUE_WIND_DIR,
  NOTIFY_HANDLE_RAINFALL,
  NOTIFY_HANDLE_TEMPERATURE,
  NOTIFY_HANDLE_HUMIDITY,
  NOTIFY_HANDLE_PRESSURE,
  NOTIFY_HANDLE_CURRENT_TIME,
  NB_BLE_NOTIFY_HANDLE
} ble_notify_handle_t;

#ifdef __cplusplus
extern "C" {
#endif

extern STATUS ble_gatt_srv_initialize(void);
extern uint16_t ble_gatt_svr_get_notify_handle(ble_notify_handle_t handle);

#ifdef __cplusplus
}
#endif

#endif /* __BLE_GATT_SVR_H__ */

