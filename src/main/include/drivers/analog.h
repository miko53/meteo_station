#ifndef __ANALOG_H__
#define __ANALOG_H__

#include "common.h"
#include "driver/adc.h"

#ifdef __cplusplus
extern "C" {
#endif

extern STATUS analog_configure(uint32_t gpio_id, adc_bits_width_t width, adc_atten_t attenuation, void** handle);
extern void analog_display_efuse_config(void);
extern uint32_t analog_do_conversion(void* handle);

#ifdef __cplusplus
}
#endif

#endif /* __ANALOG_H__ */

