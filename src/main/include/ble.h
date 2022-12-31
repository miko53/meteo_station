#ifndef __BLE_H__
#define __BLE_H__

#include "common.h"
#include "data_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

extern STATUS ble_init(void);
//extern STATUS ble_deinit(void);
extern STATUS ble_notify_new_data(data_type_t indexSensor, variant_t* pData);

#ifdef __cplusplus
}
#endif
#endif /* __BLE_H__ */
