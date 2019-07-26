#include "timer.h"
#include "label.h"
#include "can.h"
  	 
/*******************************************************************************
 * 名称: TIM3_Time_Init
 * 功能: TIM3定时中断初始化
 * 形参: arr:自动重装值，psc:时钟预分频数
 * 返回: 无
 * 说明: 主要用于定时中断
 * 这里时钟选择为APB1的2倍，却APB1的时钟为36M
 * 定时时间T=(arr+1)*(psc+1)/72M 
 ******************************************************************************/
void TIM3_Time_Init(u16 arr,u16 psc)
{
    //GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
    //TIM_OCInitTypeDef  TIM_OCInitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);       //TIM3时钟使能
	
	//定时器TIM3初始化
	TIM_TimeBaseStructure.TIM_Period = arr;                    //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
	TIM_TimeBaseStructure.TIM_Prescaler =psc;                  //设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;    //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;//TIM向上计数模式
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);            //根据指定的参数初始化TIMx的时间基数单位

	//中断优先级NVIC设置
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;            //TIM3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;  //先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;         //从优先级3级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;            //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);                            //初始化NVIC寄存器
    
	TIM_ClearFlag(TIM3,TIM_IT_Update);                         //清更新中断标志位
    TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);                   //使能指定的TIM3中断,允许更新中断
	TIM_Cmd(TIM3, ENABLE);                                     //使能TIM3					 
}
/*******************************************************************************
 * 名称: TIM5_PWMIn_Init
 * 功能: TIM5PWM输入捕获初始化
 * 形参: arr:设定计数器自动重装值 psc:预分频器值
 * 返回: 无
 * 说明: 主要用于测量外部脉冲的频率及占空比
 * 输入捕获有两种方式:普通输入捕获和PWM输入捕获
 * PWM输入捕获可以计算脉冲频率及占空比
 ******************************************************************************/
void TIM5_PWMIn_Init(u16 arr,u16 psc)
{	 
	TIM_ICInitTypeDef  TIM5_ICInitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
   	NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5,ENABLE);	             //使能TIM5时钟
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);             //使能GPIOA时钟

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0;                   
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;            //PAO 输入   
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	GPIO_Init(GPIOA, &GPIO_InitStructure);
		
	//初始化定时器TIM5	 
	TIM_TimeBaseStructure.TIM_Period = arr;                          //设定计数器自动重装值 
	TIM_TimeBaseStructure.TIM_Prescaler =psc; 	                     //预分频器   
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;          //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;      //TIM向上计数模式
	TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);              
  
	//初始化TIM5输入捕获参数
	TIM5_ICInitStructure.TIM_Channel =TIM_Channel_1;                 //CC1S=01 	选择输入端 IC1映射到TI1上
  	TIM5_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	 //上升沿捕获
  	TIM5_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; 
  	TIM5_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	         //配置输入分频,不分频 
  	TIM5_ICInitStructure.TIM_ICFilter = 0x00;                        //IC1F=0000 配置输入滤波器 不滤波
  	//TIM_ICInit(TIM5, &TIM5_ICInitStructure);                       //输入捕获模式
    TIM_PWMIConfig(TIM5, &TIM5_ICInitStructure);                     //PWM输入捕获模式

	TIM_SelectInputTrigger(TIM5,TIM_TS_TI1FP1);                      //选择IC1为始终触发源

    TIM_SelectSlaveMode(TIM5,TIM_SlaveMode_Reset);                   //TIM从模式：触发信号的上升沿重新初始化计数器和触发寄存器的更新事件
 
    TIM_SelectMasterSlaveMode(TIM5,TIM_MasterSlaveMode_Enable);      //启动定时器的被动触发
	
	//中断分组初始化
	NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;                  //TIM5中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority =0;         //先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =2;                //从优先级2级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                  //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);                         
     
	TIM_ClearFlag(TIM5,TIM_IT_CC1);                                  //清除中断标志位
    TIM_Cmd(TIM5,ENABLE); 	                                         //使能定时器5
	TIM_ITConfig(TIM5,TIM_IT_CC1,ENABLE);                            //允许CC1IE捕获中断	
}
/**************************************************************** 
 * 函数名：TIM8_PWMOut_Init  
 * 描述:  TIM8 PWM输出初始化 
 *      CH1:输出 T=2.5ms(f=1/2.5ms=400Hz)  D=0.6的PWM波(高电平在前，低电平在后) 
 *       
 *      步骤一：通过T和TIMxCLK的时钟源确定TIM_Period和TIM_Prescaler  
 *          T=(TIM_Period+1)*(TIM_Prescaler+1)/TIMxCLK=2.5ms  
 *          因为 TIM_Period<65535，所以 TIM_Prescaler>1,即 TIM_Prescaler=2 
 *          所以 TIM_Period=59999=0xEA5F 
 *      步骤二：根据TIM_Period的值，高低电平的先后D，确定CCR和TIM_OCPolarity 
 *          CH1：因为D=0.6，先高后低； 
 *               所以CCR1=(TIM_Period+1)* D=36000;TIM_OCPolarity=TIM_OCPolarity_High 
 * 形参:  arr:自动重装值(0--65535)，psc:时钟预分频数(0--65535), duty:设定占空比值
 * 返回值:无
 * 说明：主要用于PWM输出
 * PWM的频率=72M/((1+arr)*(1+psc))
 *    占空比=duty/(1+arr)
 ***************************************************************/  
void TIM8_PWMOut_Init(u16 arr,u16 psc,u16 duty)  
{  
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;  
    TIM_OCInitTypeDef  TIM_OCInitStructure;  
	GPIO_InitTypeDef GPIO_InitStructure; 
   
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_TIM8, ENABLE); //使能GPIOC时钟、定时器TIM8时钟
 
	   
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_9; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;                           // 复用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	GPIO_Init(GPIOC, &GPIO_InitStructure); 
	  
    /* Time base configuration */                                            
    TIM_TimeBaseStructure.TIM_Period = arr;  
    TIM_TimeBaseStructure.TIM_Prescaler = psc;                                //设置预分频 
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;                              //设置时钟分频系数：不分频  
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;               //向上计数溢出模式  
    TIM_TimeBaseInit(TIM8, &TIM_TimeBaseStructure);  
    /* PWM1 Mode configuration: Channel4 */  
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;                         //配置为PWM模式1  
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;                
    TIM_OCInitStructure.TIM_Pulse = duty;                                     //设置跳变值，当计数器计数到这个值时，电平发生跳变  
    TIM_OCInitStructure.TIM_OCPolarity =TIM_OCPolarity_High;                  //当定时器计数值小于CCR1时为高电平  
    TIM_OC4Init(TIM8, &TIM_OCInitStructure);                                       
    TIM_OC4PreloadConfig(TIM8, TIM_OCPreload_Enable);  
   
    TIM_CtrlPWMOutputs(TIM8,ENABLE);                                          //MOE 主输出使能//只有高级定时器TIM1和TIM8需要进行主输出使能
	TIM_ARRPreloadConfig(TIM8, ENABLE);                                       //使能TIM8重载寄存器ARR  
    /* TIM8 enable counter */  
    TIM_Cmd(TIM8, ENABLE);                                                    //使能TIM8
} 
/*******************************************************************************
 * 名称: ControlPWMOut1()
 * 功能: 控制PWM输出的频率及占空比
 * 形参: TIMx:通道选择 arr:设定计数器自动重装值 psc:预分频器值 duty:占空比
 * 返回: 无
 * 说明: 主要控制PWM的输出，包括控制PWM的频率及占空比
 * 频率f=72M/((arr+1)*(psc+1)), arr:0~65535，psc:0~65535
 * 占空比dutyvalue=duty/(arr+1)   例如:占空比为0.6，则duty=0.6*(arr+1)
 * 定时器8通道4输出PWM
 * 调节的频率范围:1HZ~24MHZ
 * 调节的占空比的范围:0~100%
 * 频率误差:
 ******************************************************************************/
void ControlPWMOut1(TIM_TypeDef* TIMx,u16 arr,u16 psc,u16 duty)
{
   TIM_SetAutoreload(TIMx,arr);                                   //arr,psc决定输出的频率
   TIM_PrescalerConfig(TIMx,psc,TIM_PSCReloadMode_Immediate);
   TIM_SetCompare4(TIMx,duty);                                    //占空比
}
/*******************************************************************************
 * 名称: TIM8_In_Init
 * 功能: TIM8 普通输入捕获初始化
 * 形参: arr:设定计数器自动重装值 psc:预分频器值
 * 返回: 无
 * 说明: 主要用于测量脉冲的频率
 * 输入捕获有两种方式:普通输入捕获和PWM输入捕获
 * 输入捕获可以计算脉冲频率
 ******************************************************************************/
void TIM8_In_Init(u16 arr,u16 psc)
{	 
	TIM_ICInitTypeDef  TIM8_ICInitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
   	NVIC_InitTypeDef NVIC_InitStructure;

 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_TIM8, ENABLE); //使能GPIOC时钟、定时器TIM8时钟
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_9;                   
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;                    //PC9 输入   
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOC,GPIO_Pin_9);			                     //PC9 下拉
	
	//初始化定时器8 TIM8	 
	TIM_TimeBaseStructure.TIM_Period = arr;                          //设定计数器自动重装值 
	TIM_TimeBaseStructure.TIM_Prescaler =psc; 	                     //预分频器   
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;          //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;      //TIM向上计数模式
	TIM_TimeBaseInit(TIM8, &TIM_TimeBaseStructure); 
  
	//初始化TIM8输入捕获参数
	TIM8_ICInitStructure.TIM_Channel =TIM_Channel_4;                 //CC4S=01 	选择输入端 IC4映射到TI4上
  	TIM8_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	 //上升沿捕获
  	TIM8_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; //映射到TI4上
  	TIM8_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	         //配置输入分频,不分频 
  	TIM8_ICInitStructure.TIM_ICFilter = 0x00;                        //IC1F=0000 配置输入滤波器 不滤波
  	TIM_ICInit(TIM8, &TIM8_ICInitStructure);                         //输入捕获模式
    
	//中断分组初始化
	NVIC_InitStructure.NVIC_IRQChannel = TIM8_CC_IRQn;               //TIM8中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority =0;         //先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;                //从优先级3级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                  //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);                          
  
    TIM_Cmd(TIM8,ENABLE); 	                                         //使能定时器8
	TIM_ITConfig(TIM8,TIM_IT_CC4,ENABLE);                            //允许CC4IE捕获中断	
}
/*******************************************************************************
 * 名称: TIM2_Extcount_Init
 * 功能: 定时器2外部计数模式初始化
 * 形参: 无
 * 返回: 无
 * 说明: 
   定时器外部计数，利用TIM_ETR引脚进行外部计数,用于计数脉冲
 ******************************************************************************/
void TIM2_ExtCount_Init(void)
{	 
   
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);	   //使能TIM2时钟
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);      //使能GPIOA时钟
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0;                  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;              //PA0 输入  
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOA,GPIO_Pin_0);						   //PA0 下拉
   
	//初始化定时器2 TIM2	 
	TIM_TimeBaseStructure.TIM_Period = 0xFFFF;                 //设定计数器自动重装值 
	TIM_TimeBaseStructure.TIM_Prescaler =0; 	               //预分频器   
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;    //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;//TIM向上计数模式
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);            //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
	 
		
	//TIM_ITRxExternalClockConfig(TIM2,TIM_TS_ETRF);             //配置TIMx内部触发为外部时钟模式
	TIM_ETRClockMode2Config(TIM2, TIM_ExtTRGPSC_OFF, TIM_ExtTRGPolarity_NonInverted, 0);//配置TIMx为外部时钟模式2
    TIM_SetCounter(TIM2,0); 
	//中断优先级NVIC设置
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;            //TIM2中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =0;          //从优先级0级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;            //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);                            //初始化NVIC寄存器
	TIM_ClearFlag(TIM2,TIM_FLAG_Update);                       //清除更新中断标志,因为只要开启定时器就立即进入一次中断
    TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);                   //使能指定的TIM2中断,允许更新中断

  	TIM_Cmd(TIM2,ENABLE); 	                                   //使能定时器2
}
/*******************************************************************************
 * 名称: TIM3_IRQHandler
 * 功能: TIM3定时中断服务函数
 * 形参: 无
 * 返回: 无
 * 说明: 1S定时中断一次
 ******************************************************************************/
void TIM3_IRQHandler(void)
{
    u16 TIM2CH1_CAPTURE_VAL;
	if(TIM_GetITStatus(TIM3,TIM_IT_Update)!= RESET)            //检查TIM3更新中断发生与否
	{
		  TIM_ClearITPendingBit(TIM3,TIM_IT_Update);           //清除TIM3更新中断标志 
		  if(msecnum<180)
		  {
		     msecnum++;
             //msecnum=CLEAR;
			 if(msecnum==180)                                  //上电3分钟后即:5ms*35000=175s 就可以进行MQ2气体传感器的校准
			 flagfirstmq2=ON;                                  //进行校准标志
		  }
        //res=Can_Send_Msg(canbuf,8);//发送8个字节 
        //printf("res=%d \r\n",res);
		TIM2CH1_CAPTURE_VAL=TIM_GetCounter(TIM2);
        if(countover>0)                                        //计数器计数溢出情况                            
	    {
		   frq=TIM2CH1_CAPTURE_VAL+countover*65536;
		   countover=0;
        }
		else
	    {
           frq=TIM2CH1_CAPTURE_VAL;
	    }
		TIM_SetCounter(TIM2,0);
	}
}
/*******************************************************************************
 * 名称: TIM2_IRQHandler
 * 功能: TIM2 计数溢出中断服务函数
 * 形参: 无
 * 返回: 无
 * 说明: 计数器溢出中断
 ******************************************************************************/
void TIM2_IRQHandler(void)     
{
     if(TIM_GetITStatus(TIM2,TIM_IT_Update)!= RESET)           //检查TIM2更新中断发生与否
	{
		  TIM_ClearITPendingBit(TIM2,TIM_IT_Update);           //清除TIM2更新中断标志 
		  if(countover<65535)                         
		  countover++;                                         //计数器溢出计数
    }
}
/*******************************************************************************
 * 名称: TIM5_IRQHandler
 * 功能: 通用定时器5中断服务函数
 * 形参: 无
 * 返回: 无
 * 说明: PWM输入捕获频率及占空比计算
 ******************************************************************************/
void TIM5_IRQHandler(void)
{
   u16 IC2Value;
   if(TIM_GetITStatus(TIM5,TIM_IT_CC1)!= RESET)                //检查TIM5边沿中断发生与否
  {
	TIM_ClearITPendingBit(TIM5, TIM_IT_CC1);                   //清除中断标志位
	IC2Value = TIM_GetCapture1(TIM5);                          //读取IC1捕获寄存器的值，即为PWM周期的计数值
	  if(IC2Value != 0)
	  {
	   
	     DutyCycle =(TIM_GetCapture2(TIM5) * 100) / IC2Value;  //读取IC2捕获寄存器的值，并计算占空比
	     frq=72000000/IC2Value;                                //计算PWM频率。
	  }
	  else
	  {
	     DutyCycle = 0;
	     frq= 0;
	  }
  }
}
/*******************************************************************************
 * 名称: TIM8_CC_IRQHandler
 * 功能: TIM8 边沿触发中断服务函数
 * 形参: 无
 * 返回: 无
 * 说明: 
 ******************************************************************************/
void TIM8_CC_IRQHandler(void)
{
   static u8 risecount=0;
   static u16 timecount=0,timecount1=0;
   u32 countsection;    //计数区间
   if(TIM_GetITStatus(TIM8,TIM_IT_CC4)!=RESET)
   {
	   TIM_ClearITPendingBit(TIM8,TIM_IT_CC4); //清除中断标志位
	   if(risecount==0)
       {
           risecount=1;
		   timecount=TIM_GetCapture4(TIM8);    //读取第一个上升沿的CNT数
	   }
	   else if(risecount==1)
	   {
           risecount=0;
		   timecount1=TIM_GetCapture4(TIM8);   //读取第二个上升沿的CNT数
           if(timecount<timecount1)
           {
               countsection=timecount1-timecount;
		   }
		   else if(timecount>timecount1)
		   {
               countsection=(0xffff-timecount)+timecount1;
		   }
		   else
		   	   countsection=0;

		   frq=1000000/(countsection);        //计算频率
	   }
   }
}















