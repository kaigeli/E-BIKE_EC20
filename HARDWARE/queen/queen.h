#ifndef __QUEEN_h
#define __QUEEN_h

#include <stdio.h>
#include "stm32f0xx_hal.h"

#define TRUE		1
#define FALSE		0
#define _FRONT		0
#define _REAR		1

#define ON  1		/*高电平*/
#define OFF 0		/*低电平*/

#define QUEUESIZE     200    //定义队列的大小
#define BUFFER_SIZE   50  //定义缓存数组的大小 

typedef int DataType;
//typedef char bool;
typedef struct
{
	//uint8_t Len;              //数据长度
	uint8_t Data[1];        //数组
}RxMsgTypeDef;
typedef struct  
{      
	RxMsgTypeDef circle_buffer[QUEUESIZE]; //
	unsigned char atom;	// 锁    
	int front; //指向队头的索引，这个所指的空间不存放元素      
	int rear; //指向队尾的索引，存放最后一个元素
	uint16_t free;   //统计缓存数组的余量  
}CircleQueue; 
typedef struct
{
   uint8_t circle_buffer[BUFFER_SIZE]; //
   uint8_t atom;                // 锁
   uint8_t front;               //指向队头的索引，这个所指的空间不存放元素 
   uint8_t rear;                //指向队尾的索引，存放最后一个元素 
}Circle_buffer;

// extern CircleQueue		 TxCAN_Queue;
extern CircleQueue		 RxUart2_Queue;		/* 串口2接收缓冲区*/
extern CircleQueue		 RxUart3_Queue;		/* 串口2接收缓冲区*/
extern CircleQueue at_send_Queue;
extern Circle_buffer buffer;
extern RxMsgTypeDef Uart1_buf, Uart2_buf,Uart3_buf;

_Bool InitCircleQueue(CircleQueue *pCQ);
_Bool IsQueueEmpty(CircleQueue *pCQ);
_Bool IsQueueFull(CircleQueue *pCQ);
_Bool PushElement(CircleQueue *pCQ, RxMsgTypeDef dMsgData, unsigned char dir);
_Bool PopElement(CircleQueue *pCQ, RxMsgTypeDef *pMsgData);
//bool GetHeadElement(CircleQueue *pCQ, CanRxMsgTypeDef *pMsgData);
void* mymemcpy(void* dest, void* source, int count);
uint8_t bufferPop(uint8_t* _buf);
void bufferPush(const uint8_t _buf);
void bufferInit(void);


#endif
