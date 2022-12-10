#ifndef __IO_H__
#define __IO_H__

#include "driver/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

extern STATUS io_init(void);
extern STATUS io_configure_output(uint32_t gpio_id, bool initial_level);
extern STATUS io_configure_inputs(gpio_int_type_t int_type, uint64_t gpio_mask);
extern STATUS io_configure_input_isr(uint32_t gpio_id, gpio_int_type_t int_type, gpio_isr_t isr_handler, void* args);

extern STATUS io_set_level(uint32_t gpio_id, bool level);

#ifdef __cplusplus
}
#endif

#endif /* __IO_H__ */
