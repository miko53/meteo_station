#ifndef __SD_CARD_H__
#define __SD_CARD_H__

#include "common.h"

typedef struct
{
  char* name;
  char* type;
  uint64_t capacity;
  bool mounted;
} sd_cart_infos_t;


extern STATUS sd_card_init(char* mount_point);
extern void sd_card_umount(void);
extern STATUS sd_card_get_infos(sd_cart_infos_t* pSdCardInfos);

#endif /* __SD_CARD_H__ */
