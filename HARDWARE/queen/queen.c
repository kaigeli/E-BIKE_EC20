/*******************************************************************************
 * File name:           queen.c
 * Descriptions:        队列相关函数
 * Created date:        2016-11-22
 * Last modified Date:  2016-11-22
********************************************************************************/
#include <stdio.h>
#include "queen.h"
#include "string.h"
#include <stdlib.h>

// CircleQueue		 TxCAN_Queue;	/* CAN 发送缓冲区*/
CircleQueue		 RxUart2_Queue;		/* 串口2接收缓冲区*/
CircleQueue		 RxUart3_Queue;		/* 串口3接收缓冲区*/
CircleQueue at_send_Queue;
Circle_buffer    buffer;
RxMsgTypeDef   Uart1_buf, Uart2_buf,Uart3_buf;
/*********************************************************************************************************
** Functoin name:       InitCircleQueue
** Descriptions:        初始化队列，使队列头指向队列尾
** input paraments:     传入队列    
** Returned values:     TRUE初始化成功，否则失败
*********************************************************************************************************/
_Bool InitCircleQueue(CircleQueue *pCQ)  
{       
     if (pCQ == NULL)          
	 	return FALSE;      
	 else      
	 	{         
	 	  pCQ->front = 0;          
		  pCQ->rear = 0;
		  pCQ->free=QUEUESIZE;
		  pCQ->atom = FALSE;   
		}  
	 return TRUE;  
}  
void ClearQueue(CircleQueue *pCQ)  //清队列
{
    pCQ->front = 0;          
		pCQ->rear = 0;
	  pCQ->free=QUEUESIZE;
}
/*********************************************************************************************************
** Functoin name:       IsQueueEmpty
** Descriptions:        检查队列是否为空
** input paraments:     队列指针 CircleQueue *pCQ
** Returned values:     TRUE为空
*********************************************************************************************************/
_Bool IsQueueEmpty(CircleQueue *pCQ) 
{      
      if (pCQ->front == pCQ->rear)         
				return TRUE;     
			else          
				return FALSE;  
}    
/*********************************************************************************************************
** Functoin name:       IsQueueFull
** Descriptions:        检查队列是否已满
** input paraments:     队列指针 CircleQueue *pCQ
** Returned values:     TRUE为满
*********************************************************************************************************/
_Bool IsQueueFull(CircleQueue *pCQ) 
{     
   if ((pCQ->rear + 1) % QUEUESIZE == pCQ->front)         
	 	return TRUE;     
	 else          
	 	return FALSE; 
}   
/*********************************************************************************************************
** Functoin name:       PushElement
** Descriptions:        将元素压入队列
** input paraments:     队列指针 CircleQueue *pCQ， 要压入的值CanRxMsg dMsgData
** Returned values:     TRUE压入队列成功，否则失败
*********************************************************************************************************/
_Bool PushElement(CircleQueue *pCQ, RxMsgTypeDef dMsgData, unsigned char dir)
{  	
   if (IsQueueFull(pCQ)) 
	{
		return FALSE;
    //ClearQueue(pCQ);  //清队列	
	}
	pCQ->atom = TRUE;

	if(dir == _REAR) 
    {
		pCQ->rear = (pCQ->rear + 1) % QUEUESIZE;  
		mymemcpy(&(pCQ->circle_buffer[pCQ->rear]), &dMsgData, sizeof(RxMsgTypeDef));
		pCQ->free--;
	} 
	else if(dir == _FRONT) 
	{
		mymemcpy(&(pCQ->circle_buffer[pCQ->front]), &dMsgData, sizeof(RxMsgTypeDef));
		if(pCQ->front == 0) 
		{
			pCQ->front = (QUEUESIZE - 1);
		} 
		else 
		{
			pCQ->front = (pCQ->front - 1);
		}
		pCQ->free--;
	} 
	else 
	{
		return FALSE;
	}
	
	pCQ->atom = FALSE;
	return TRUE;  
} 
/*********************************************************************************************************
** Functoin name:       PopElement
** Descriptions:        将队列的元素取出
** input paraments:     队列指针CircleQueue *pCQ，接收压出队列的结构体指针CanRxMsg *pMsgData
** Returned values:     TRUE为队列压出成功，否则失败
*********************************************************************************************************/
_Bool PopElement(CircleQueue *pCQ, RxMsgTypeDef *pMsgData)
{      
   if( (IsQueueEmpty(pCQ)) || (pCQ->atom == TRUE) )        
	 	return FALSE;       
	 pCQ->front = (pCQ->front + 1) % QUEUESIZE;  	
	 mymemcpy(pMsgData, &(pCQ->circle_buffer[pCQ->front]), sizeof(RxMsgTypeDef)); 
	 pCQ->free++;
     
	 return TRUE;  
} 
/*********************************************************************************************************
** Functoin name:       mymemcpy
** Descriptions:        将数据从源地址拷贝到目标地址
** input paraments:     dest 目的地址 source 源地址 count 拷贝个数
** Returned values:     无
*********************************************************************************************************/
void* mymemcpy(void* dest, void* source, int count)
{
	char *ret = (char *)dest;
	char *dest_t = ret;
	char *source_t = (char *)source;
	
	while (count--)
		*dest_t++ = *source_t++; 
	
	return ret;
}
/*********************************************************************************************************
** Functoin name:       
** Descriptions:       
** input paraments:     
** Returned values: 
*********************************************************************************************************/
void bufferInit(void)
{
     buffer.front=OFF;
     buffer.atom=OFF;
     buffer.rear=OFF;
}
uint8_t bufferPop(uint8_t* _buf)
{
    if(buffer.front==buffer.rear||buffer.atom==ON)                    //如果头尾接触表示缓冲区为空
          return FALSE;
    else 
    {
        if(buffer.atom==OFF)
       {
          *_buf=buffer.circle_buffer[buffer.front];    //如果缓冲区非空则取头节点值并偏移头节点
          if(++buffer.front>=BUFFER_SIZE)
            buffer.front=0;
       }
    }
    return TRUE;
}

void bufferPush(const uint8_t _buf)
{   
    buffer.atom=ON;
    buffer.circle_buffer[buffer.rear]=_buf;          //从尾部追加
    if(++buffer.rear>=BUFFER_SIZE)                   //尾节点偏移
        buffer.rear=0;                               //大于数组最大长度 制零 形成环形队列
        if(buffer.rear==buffer.front)                //如果尾部节点追到头部节点 则修改头节点偏移位置丢弃早期数据
        if(++buffer.front>=BUFFER_SIZE)
            buffer.front=0;
    buffer.atom=OFF;
}

