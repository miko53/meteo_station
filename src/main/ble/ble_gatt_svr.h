#ifndef __BLE_GATT_SVR_H__
#define __BLE_GATT_SVR_H__

#include "nimble/ble.h"
#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

extern uint16_t hrs_hrm_handle;
extern uint16_t handle_true_wind_speed;
extern uint16_t handle_true_wind_dir;
extern uint16_t handle_rainfall;
extern uint16_t handle_temperature;
extern uint16_t handle_humidity;
extern uint16_t handle_pressure;
extern uint16_t handle_current_time;

extern STATUS ble_gatt_srv_initialize(void);

#ifdef __cplusplus
}
#endif

#endif /* __BLE_GATT_SVR_H__ */

