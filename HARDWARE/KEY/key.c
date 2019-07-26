#include "key.h"
#define KeyDebug  //按键程序调试打印
/*******************************************************************************
* 名称:KEY_Init
* 功能:拔码按键初始化函数
* 形参:无
* 返回:无
* 说明:对PD8,9,10、PE9,14,15进行初始化，并配置成上拉输入
******************************************************************************/
void Key_Init(void) //IO初始化
{ 
 	GPIO_InitTypeDef GPIO_InitStructure;
 
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOE|RCC_APB2Periph_GPIOD,ENABLE);//使能PORTD,PORTE时钟

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10;   //
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
 	GPIO_Init(GPIOD, &GPIO_InitStructure);//初始化GPIOD.8,9,10
 	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_9|GPIO_Pin_14|GPIO_Pin_15;  //
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
 	GPIO_Init(GPIOE, &GPIO_InitStructure);//初始化GPIOE.9,14,15
 	
}
/*******************************************************************************
* 名称:scan_key
* 功能:扫描拔码开关值
* 形参:无
* 返回:拔了开关值
* 说明: 无
******************************************************************************/
u8 scan_key(void)
{
     u8 keyvalue;
	 
	 if(KEY1==0 && KEY2==1 && KEY3==1 && KEY4==1 && KEY5==1 && KEY6==1)
     {
        keyvalue=1;
	 }
	 else if(KEY1==1 && KEY2==0 && KEY3==1 && KEY4==1 && KEY5==1 && KEY6==1) 
	 {
        keyvalue=2;
	 }
     else if(KEY1==1 && KEY2==1 && KEY3==0 && KEY4==1 && KEY5==1 && KEY6==1) 
	 {
        keyvalue=3;
	 }
	 else if(KEY1==1 && KEY2==1 && KEY3==1 && KEY4==0 && KEY5==1 && KEY6==1) 
	 {
        keyvalue=4;
	 }
	 else if(KEY1==1 && KEY2==1 && KEY3==1 && KEY4==1 && KEY5==0 && KEY6==1) 
	 {
        keyvalue=5;
	 }
	 else if(KEY1==1 && KEY2==1 && KEY3==1 && KEY4==1 && KEY5==1 && KEY6==0) 
	 {
        keyvalue=6;
	 }
	 else 
	 {
        keyvalue=7;
	 }
	 
	 return keyvalue;     
}
/*******************************************************************************
* 名称:scan_key
* 功能:扫描拔码开关值
* 形参:无
* 返回:拔了开关值
* 说明: 无
******************************************************************************/
void Key_Process(void)
{
    u8 KEYS;
    KEYS=scan_key();
	switch(KEYS)
	{
       case 1: 
              status=1;
	          break;
       case 2: 
              status=2;
			  break;
	   case 3: 
              status=3;
		      break;
	   case 4: 
              status=4;
	          break;
	   case 5: 
              status=5;
		      break;
	   case 6: 
              status=6;
		      break;
	   default:
	   	      status=0;
		      break;
	}
	   #ifdef KeyDebug
	   printf("status=%d\r\n",status);
	   #endif
}

/*u8 scan_key(void)    //按键扫描
{
	//u8 key_status;
	
	if((longkey == OFF) && (keybuf != _KEYNULL))
	{
		// 在keybuf被标记为长按或短按后，若是按键已经松开，
		// 则在主循环跑完一次后，及时将按键状态标记为无按键按下。
		keybuf = _KEYNULL;
	}
	if(key_handle ==ON) 
	{ //5ms进入一次
		key_handle = OFF;	  // 复位5ms标志位 
		//key_status = WK_UP;
		//if(key_status == 0) 
		if(WK_UP==OFF)      //WK_UP键被按下
		{	
			longkey++;      //按下按键累计
		}
		else if(USER1==OFF) //USER1键被按下
		{
      longkey1++;     //按下按键累计
    }	
		else if(USER2==OFF) //USER2键被按下
		{
      longkey2++;     //按下按键累计
    }	
		else if(TAMPER==OFF)//TAMPER键被按下
		{
      longkey3++;     //按下按键累计
    }	
		else              //松开按键
		{	
			if((longkey >= 3)&&(longkey <= 100))  // 按下15ms - 1s
			{	
				//keybuf = _SHORTKEY;
				longkey = 0;                   
				longkey1 = 0;
				longkey2 = 0;
				longkey3 = 0;
				return WKUP_PRES;              //WKUP键被按下
			} 
			else if((longkey1 >= 3)&&(longkey1 <= 100))  // 按下15ms - 1s
			{
				longkey = 0;                   //按键按下计数1清0
				longkey1 = 0;                  //按键按下计数2清0
				longkey2 = 0;                  //按键按下计数3清0
				longkey3 = 0;                  //按键按下计数4清0
				return KEY2_PRES;              //KEY2键被按下
      }
			else if((longkey2 >= 3)&&(longkey2 <= 100))  // 按下15ms - 1s
			{
        longkey = 0;                   //按键按下计数1清0
				longkey1 = 0;                  //按键按下计数2清0
				longkey2 = 0;                  //按键按下计数3清0
				longkey3 = 0;                  //按键按下计数4清0
				return KEY1_PRES;              //KEY1键被按下
      }
			else if((longkey3 >= 3)&&(longkey3 <= 100))  // 按下15ms - 1s
			{
        longkey = 0;                   //按键按下计数1清0
				longkey1 = 0;                  //按键按下计数2清0
				longkey2 = 0;                  //按键按下计数3清0
				longkey3 = 0;                  //按键按下计数4清0
				return KEY0_PRES;              //KEY0键被按下
      }
			//else if(longkey >= 200) 
			//{ // 2s
			//	//keybuf = _LONGKEY;
			//	longkey = 0;
			//	return _LONGKEY;
			//}
			else 
			{
				//keybuf = _KEYNULL;// 若为扰动，按键状态也该为无按键按下
				longkey = 0;                  //按键按下计数1清0
				longkey1 = 0;                 //按键按下计数2清0
				longkey2 = 0;                 //按键按下计数3清0
				longkey3 = 0;                 //按键按下计数4清0
			  return _KEYNULL;              //无键被按下
      }
		}
	}
	return _KEYNULL;                    //无键被按下
}
void key_process(void) //按键处理
{
    keynum=scan_key();               //按键值读取
		if(keynum==KEY0_PRES)
		{
			LED2=!LED2;                   //D2灯取反，进来之后原来是亮变暗，原来是暗变亮
		  //printf("\r\nAD:");       //串口打印数据
			adcx=1234;
			USART_SendData(USART1,adcx/1000);        
			USART_SendData(USART1,adcx%1000/100);
			USART_SendData(USART1,adcx%1000/100/10);
			USART_SendData(USART1,adcx%1000/100%10);
		}
		else if(keynum==KEY1_PRES)
		{
      LED0=!LED0;                   //D0灯取反，进来之后原来是亮变暗，原来是暗变亮
		  printf("\r\nKEY3\r\n");       //串口打印数据
    }
    else if(keynum==KEY2_PRES)	
    {
      LED1=!LED1;                   //D1灯取反，进来之后原来是亮变暗，原来是暗变亮
		  printf("\r\nKEY4\r\n");       //串口打印数据
    }
    else if(keynum==WKUP_PRES)
    {
      LED3=!LED3;                   //D3灯取反，进来之后原来是亮变暗，原来是暗变亮
		  printf("\r\nKEY2\r\n");       //串口打印数据
    }		
}*/	
