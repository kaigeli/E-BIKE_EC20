#ifndef __OTA_DOWNLOAD_H__
#define __OTA_DOWNLOAD_H__

#include "stm32f0xx_hal.h"


typedef struct
{
	uint8_t flag;
	uint8_t url[256];
	uint16_t size;
	uint32_t timestamp;
}VER_STRUCT;

typedef struct
{
	uint16_t target_range1;
	uint16_t target_range2;
	uint16_t actual_range1;
	uint16_t actual_range2;
	uint32_t total_size;
	uint16_t item_size;
	uint8_t num;
	uint8_t sum;
	uint8_t complete;
	uint8_t ip[4];
	uint16_t port;
	uint8_t path[100];
	
}download_struct;

void Http_package_send(void);
extern void parse_ver_package(uint8_t* buf,uint8_t len);


#endif
