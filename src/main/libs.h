#ifndef __LIBS_H__
#define __LIBS_H__

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void dump_date(struct tm* pDate);
extern void date_get_localtime(struct tm* pDate);
extern void date_set_localtime(struct tm* pDate);

#ifdef __cplusplus
}
#endif

#endif /* __LIBS_H__ */
