#ifndef __FLASH_H
#define __FLASH_H

#include "stm32f0xx_hal.h"
//exterm uint32_t writeflashdata;
#define 	STM32_FLASH_ADDRESS		0x0800f000 
#define sEE_PAGESIZE           16
#define sEE_FLAG_TIMEOUT         ((uint32_t)0x1000)
#define sEE_LONG_TIMEOUT         ((uint32_t)(10 * sEE_FLAG_TIMEOUT))


static FLASH_EraseInitTypeDef EraseInitStruct;

//void writeFlash(uint32_t addr, uint32_t data);
//void readflash(uint32_t addr);
void write_flash(uint32_t WriteAddr, uint8_t *pBuffer, uint16_t NumByteToWrite);
void read_flash(uint16_t ReadAddr, uint8_t* pBuffer, uint16_t NumByteToRead);


#endif
