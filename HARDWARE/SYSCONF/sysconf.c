#include "sysconf.h"



void NVIC_INIT(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    // 中断优先级分组
	// 4级主优先级， 4级次优先级
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //设置NVIC中断分组2:2位抢占优先级，2位响应优先级

    //Usart2 中断优先级配置
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;        //串口2中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;  //抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =0;		 //子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
    //CAN 发送中断优先级配置
	NVIC_InitStructure.NVIC_IRQChannel = USB_HP_CAN1_TX_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;   // 主优先级为1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;          // 次优先级为0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
    //CAN FIFO0接收中断优先级配置
	NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;     // 主优先级为1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;            // 次优先级为0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
    //CAN 错误中断优先级配置
	NVIC_InitStructure.NVIC_IRQChannel = CAN1_SCE_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;     // 主优先级为1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;            // 次优先级为0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
}
void Hardware_Init(void)
{
	 LED_Init();			                         //LED端口初始化
	 Key_Init();                                     //按键IO初始化
     Relay_Init();                                   //继电器初始化
     uart_init(115200);	                             //串口初始化为115200
     Can_Init(BITRATE_500K);                         //CAN初始化
}
void Variable_Init(void)                             //全局变量初始化
{  
     ;
}

 
