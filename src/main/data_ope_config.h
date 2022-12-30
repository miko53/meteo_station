#ifndef __DATA_OPE_CONFIG_H__
#define __DATA_OPE_CONFIG_H__

#include "common.h"
#include "data_defs.h"
#include "data_ope.h"
#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

extern data_operation_t* date_ope_config_get(void);
extern uint32_t date_ope_config_nbItems(void);
extern data_type_t date_ope_config_get_data_type(uint32_t indexSensor);


#ifdef __cplusplus
}
#endif

#endif /* __DATA_OPE_CONFIG_H__ */


