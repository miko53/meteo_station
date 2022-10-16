#ifndef __SD_CARD_H__
#define __SD_CARD_H__

#include "common.h"

extern STATUS sd_card_init(char* mount_point);
extern void sd_card_umount(void);

#endif /* __SD_CARD_H__ */
