#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "stm32f0xx_hal.h"

#define  USART2_BUFFER_SIZE  120
#define  USART3_BUFFER_SIZE  120

/* Private variables ---------------------------------------------------------*/
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart1;

//void trace(char* buf, int len);
void uart2_send(uint8_t * pData,uint16_t Size);
void uart1_send(uint8_t * pData,uint16_t Size);
void uart2_process(void);

void uart3_send(uint8_t * pData,uint16_t Size);
void uart3_process(void);


extern char usart2_recv_buffer[USART2_BUFFER_SIZE];
extern short usart2_recv_buffer_index;
#endif


