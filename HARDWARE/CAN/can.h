#ifndef __CAN_H
#define __CAN_H	 

#include "stm32f4xx_hal.h"
//#include "string.h"
#include "usart.h"
#include "queen.h"
#include "gpio.h"

#define CANRX32IDMASK   1       //32位标识符屏蔽模式
#define CANRX32IDLIST   2       //32位标识符列表模式
#define CANRX16IDMASK   3       //16位标识符屏蔽模式
#define CANRX16IDLIST   4       //16位标识符列表模式
typedef enum {
        BITRATE_5K=0,
        BITRATE_10K,
        BITRATE_20K,
        //BITRATE_40K,
        BITRATE_50K,
        BITRATE_100K,
        BITRATE_125K,
        BITRATE_250K,
        //BITRATE_400K,
        BITRATE_500K,
        //BITRATE_800K,
        BITRATE_1M  
} BITRATE;

void CAN_InitConf(uint8_t baudrate);
HAL_StatusTypeDef CAN_Send_Msg(uint8_t* msg,uint8_t len,uint8_t id);

//u8 Can_Receive_Msg(u8 *buf);							//接收数据

void CAN_Dataprocess(void);

void CAN_RxFilerconfig(uint8_t FilterNum,uint8_t FilterMode);    //接收过滤模式设置 

HAL_StatusTypeDef CAN_Polling(void);

//void CAN_ERR_Handle(void);


#endif

















