#ifndef __CONTROL_INTERFACE_H__
#define __CONTROL_INTERFACE_H__
#include "stm32f0xx_hal.h"
#include <stdlib.h>
#include <stdbool.h>


typedef enum
{
	HIGH_SPEED = 1,
	LOW_SPEED,
}SPEED;

typedef enum
{
	HIGH = 1,
	LOW,
}QIANYA;
typedef enum
{
	DIANDONG =1,
	ZHULI=2,
	ZHULI2=3,
	RENLI=4,
	HUNHE=5,
}DONGLI;
typedef enum
{
	VOT36V=1,
	VOT48V=2,	
}DIANYUAN;
typedef enum
{
	XF_INVALID=1,
	XF_OK=2,	
}XIUFU;
typedef struct
{
	uint8_t fault;
	uint32_t hall;
	SPEED tiaosu;
	QIANYA qianya;
	DONGLI zhuli;
	DIANYUAN dy;
	XIUFU xf;
}controller_struct;

typedef struct
{
	controller_struct require;
	controller_struct actual;
}control_struct;

typedef enum
{
	CMD_CONTROL=1,
	CMD_READ,
}cmd_enum;

typedef enum
{
	ADDR_BAT=0x16,
	ADDR_CONTROL=0x1d,	
}addr_enum;

typedef enum
{
	bat_temp=0x08,
	bat_vol=0x09,
	bat_curr=0x0a,
	bat_cap=0x0f,
	bat_total_cap=0x10,
	bat_cycle=0x17,
	bat_interval=0x47,
	bat_max_interval=0x48,	
}bat_cmd_enum;

#pragma pack (1)
typedef struct
{
	uint16_t temp;
	uint16_t voltage;
	uint16_t current;
	uint16_t residual_cap;
	uint16_t total_cap;
	uint16_t cycle_count;
	uint16_t interval;
	uint16_t max_interval;
}battery_info_struct;
#pragma pack ()

extern control_struct controller;
extern void zt_controller_send(uint8_t addr,cmd_enum cmd, uint8_t data1,uint8_t data2);
bool parse_control_cmd(uint8_t* buf, uint16_t len);
#endif
