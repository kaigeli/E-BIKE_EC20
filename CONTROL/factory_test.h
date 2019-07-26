#ifndef __ZT_FACTORY_TEST_H__
#define __ZT_FACTORY_TEST_H__
#include "stm32f0xx_hal.h"

typedef void (*testFun)(void);
typedef enum
{
	TEST_INIT=0,
	TEST_CPIN,
	TEST_HALL,		
	TEST_DIANMEN,
	TEST_BATLOCK,
	TEST_DIANJILOCK,	
	TEST_LUNDONG,
	TEST_VOICE,	
	TEST_A,
	TEST_B,
	TEST_GSEN,
	TEST_ADC,
	TEST_KEY,
	TEST_BT,
	TEST_GSM,
	TEST_GPS,
	
	TEST_MAX,
	
}TEST_ITEM;

typedef struct
{
	TEST_ITEM item;
	testFun ts_on;
	testFun ts_off;
	char name[20];
	uint8_t result;
}zt_factory_test_struct;

 void test_process(void);
#endif
