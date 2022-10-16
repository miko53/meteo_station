#ifndef __PCF_8523_H__
#define __PCF_8523_H__

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

// extern STATUS pcf_8523_init(void);
extern bool pcf8523_hasLostPower(void);
extern bool pcf8523_isBattLow(void);
extern STATUS pcf8523_set_date(struct tm* pDateTime);
extern STATUS pcf8523_get_date(struct tm* pDateTime);


#ifdef __cplusplus
}
#endif

#endif /* __PCF_8523_H__ */


