#include "usart.h"	  
#include "stm32f0xx_hal.h"
#include <stdio.h> 
#include "queen.h"
#include <string.h>
#include <stdlib.h>
#include "Control_interface.h"
#include "IoT_Hub.h"

#ifdef __GNUC__  
  /* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf 
     set to 'Yes') calls __io_putchar() */  
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)  
#else  
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)  
#endif /* __GNUC__ */  
/** 
  * @brief  Retargets the C library printf function to the USART. 
  * @param  None 
  * @retval None 
  */  
PUTCHAR_PROTOTYPE  
{  
  /* Place your implementation of fputc here */
  /* Loop until the end of transmission */  
   while(__HAL_UART_GET_FLAG(&huart2, UART_FLAG_TC) == RESET){}   
  /* e.g. write a character to the USART */  
    huart2.Instance->TDR = (uint8_t) ch;  

  return ch;  
}  

char usart2_recv_buffer[USART2_BUFFER_SIZE] = {0};
short usart2_recv_buffer_index = 0;

char usart3_recv_buffer[USART3_BUFFER_SIZE] = {0};
short usart3_recv_buffer_index = 0;

/*******************************************************************************
* 函数名  : UART2_Data
* 描述    : USART2发送一个字节
* 输入    : byte 一个字节
* 输出    : 无
* 返回    : 无 
* 说明    : 无
*******************************************************************************/

void uart1_send(uint8_t* pData, uint16_t Size)
{
	Logln(D_INFO,"uart send %s", pData);
	HAL_UART_Transmit(&huart1, pData, Size, 1000);
	while(__HAL_UART_GET_FLAG(&huart1, UART_FLAG_TC)!=SET);
}

void uart2_send(uint8_t* pData, uint16_t Size)
{
	HAL_UART_Transmit(&huart2, pData, Size, 1000);
	while(__HAL_UART_GET_FLAG(&huart2, UART_FLAG_TC)!=SET);
}

void uart3_send(uint8_t* pData, uint16_t Size)
{
	HAL_UART_Transmit(&huart3, pData, Size, 1000);
	while(__HAL_UART_GET_FLAG(&huart3, UART_FLAG_TC)!=SET);
}

void uart2_process(void)
{
	  RxMsgTypeDef Uart2_Rxbuf;
	  if (PopElement(&RxUart2_Queue,&Uart2_Rxbuf) == TRUE)  //
  	 {
		usart2_recv_buffer[usart2_recv_buffer_index] = Uart2_Rxbuf.Data[0]; //将接收到的字符串存到缓存中
		usart2_recv_buffer_index++;
		if(usart2_recv_buffer_index >= USART2_BUFFER_SIZE)       														//如果缓存满,将缓存指针指向缓存的首地址
		{
			usart2_recv_buffer_index = 0;
		}
	}
	else
	{   
		 if (usart2_recv_buffer_index > 0)
		{
			if(parse_control_cmd(usart2_recv_buffer,usart2_recv_buffer_index))
			{
				usart2_recv_buffer_index = 0;
				memset(usart2_recv_buffer, 0, USART2_BUFFER_SIZE);
			}
		}
	}
}

void uart3_process(void)
{
	  RxMsgTypeDef Uart3_Rxbuf;
	  if (PopElement(&RxUart3_Queue,&Uart3_Rxbuf) == TRUE)  //
  	 {
		usart3_recv_buffer[usart3_recv_buffer_index] = Uart3_Rxbuf.Data[0]; //将接收到的字符串存到缓存中
		usart3_recv_buffer_index++;
		if(usart3_recv_buffer_index >= USART3_BUFFER_SIZE)       														//如果缓存满,将缓存指针指向缓存的首地址
		{
			usart3_recv_buffer_index = 0;
		}
	}
	else
	{   
		 if (usart3_recv_buffer_index > 0)
		{
			if(parse_bt_cmd(usart3_recv_buffer,usart3_recv_buffer_index))
			{
				usart3_recv_buffer_index = 0;
				memset(usart3_recv_buffer, 0, USART3_BUFFER_SIZE);
			}
		}
	}
}

