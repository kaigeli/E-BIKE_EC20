#ifndef __BT_APP_H__
#define __BT_APP_H__
#include "stm32f0xx_hal.h"

#pragma pack (1)

typedef struct
{
	uint16_t volt;
	uint32_t hall;
	uint8_t lock;
}read_data_struct;

typedef struct
{
	uint32_t latitude;
	uint32_t longitude;
}location_struct;
typedef struct
{
	uint8_t lock_state;
	location_struct gps;
}bt_giveback_struct;

#pragma pack ()

extern void bt_init(void);
#endif
