#ifndef __BLE_GATT_SVR_H__
#define __BLE_GATT_SVR_H__

#include "nimble/ble.h"
#include "modlog/modlog.h"
#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Heart-rate configuration */
#define GATT_HRS_UUID                           0x180D
#define GATT_HRS_MEASUREMENT_UUID               0x2A37
#define GATT_HRS_BODY_SENSOR_LOC_UUID           0x2A38
#define GATT_DEVICE_INFO_UUID                   0x180A

extern uint16_t hrs_hrm_handle;
extern STATUS ble_gatt_srv_initialize(void);

#ifdef __cplusplus
}
#endif

#endif /* __BLE_GATT_SVR_H__ */

