#include "can.h"

//u32 CANERR_Flag = 0;                   // CAN出错
//u8 CAN_ErrorCode;
//u32 test6;
extern CAN_HandleTypeDef hcan1;

CanTxMsgTypeDef   TxMessage;
CanRxMsgTypeDef   RxMessage;
//CAN初始化
//mode:CAN_Mode_Normal,普通模式;CAN_Mode_LoopBack,回环模式;
//Fpclk1的时钟在初始化的时候设置为36M,如果设置CAN_Mode_Init(CAN_SJW_1tq,CAN_BS2_8tq,CAN_BS1_9tq,4,CAN_Mode_LoopBack);
//则波特率为:36M/((8+9+1)*4)=500Kbps 
/*******************************************************************************
 * 名称: 配置CAN通信波特率
 * 功能: 
 * 形参: 无
 * 返回: 无
 * 说明: 
 0-----5k
 1-----10k
 2-----20k
 3-----50k
 4-----100k
 5-----125k
 6-----250k
 7-----500k
 8-----800k
 9-----1M
 ******************************************************************************/
void CAN_InitConf(uint8_t baudrate)
{ 
	hcan1.Instance = CAN1;                //CAN1通道
	hcan1.pTxMsg = &TxMessage;
  hcan1.pRxMsg = &RxMessage;
	hcan1.Init.Mode = CAN_MODE_NORMAL;    //正常模式
  hcan1.Init.TTCM = DISABLE;            //非时间触发通信模式  
  hcan1.Init.ABOM = ENABLE;							//软件自动离线管理	
  hcan1.Init.AWUM = DISABLE;						//睡眠模式通过软件唤醒(清除CAN->MCR的SLEEP位)
  hcan1.Init.NART = ENABLE;							//禁止报文自动传送 
  hcan1.Init.RFLM = DISABLE; 						//报文不锁定,新的覆盖旧的
  hcan1.Init.TXFP = ENABLE;							//优先级由报文标识符决定 
	//设置波特率
	hcan1.Init.SJW = CAN_SJW_1TQ;	        //重新同步跳跃宽度(Tsjw)为tsjw+1个时间单位  CAN_SJW_1tq	 CAN_SJW_2tq CAN_SJW_3tq CAN_SJW_4tq//是为了兼容不同波特率的总线

  //tsjw:重新同步跳跃时间单元.范围:CAN_SJW_1tq~ CAN_SJW_4tq
  //tbs2:时间段2的时间单元.   范围:CAN_BS2_1tq~CAN_BS2_8tq;
  //tbs1:时间段1的时间单元.   范围:CAN_BS1_1tq ~CAN_BS1_16tq
  //brp :波特率分频器.范围:1~1024;  tq=(brp)*tpclk1
  //波特率=Fpclk1/((Tbs1+Tbs2+1)*brp);    
	switch(baudrate)
	{
		case BITRATE_5K: 
        hcan1.Init.Prescaler= 420;                
				hcan1.Init.BS1   = CAN_BS1_16TQ;             
				hcan1.Init.BS2   = CAN_BS2_3TQ;  
				break;
		case BITRATE_10K:
				hcan1.Init.Prescaler= 210;                
				hcan1.Init.BS1   = CAN_BS1_16TQ;             
				hcan1.Init.BS2   = CAN_BS2_3TQ;             
				break;
		case BITRATE_20K:
				hcan1.Init.Prescaler= 100;                
				hcan1.Init.BS1   = CAN_BS1_14TQ;             
				hcan1.Init.BS2   = CAN_BS2_6TQ;             
				break;
		case BITRATE_50K:
				hcan1.Init.Prescaler= 42;                
				hcan1.Init.BS1   = CAN_BS1_16TQ;             
				hcan1.Init.BS2   = CAN_BS2_3TQ;             
				break;
		case BITRATE_100K:
				hcan1.Init.Prescaler= 21;                
				hcan1.Init.BS1   = CAN_BS1_16TQ;             
				hcan1.Init.BS2   = CAN_BS2_3TQ;             
				break;
		case BITRATE_125K:
				hcan1.Init.Prescaler= 16;                
				hcan1.Init.BS1   = CAN_BS1_13TQ;             
				hcan1.Init.BS2   = CAN_BS2_7TQ;             
				break;
		case BITRATE_250K:
				hcan1.Init.Prescaler= 8;                
				hcan1.Init.BS1   = CAN_BS1_13TQ;             
				hcan1.Init.BS2   = CAN_BS2_7TQ;             
				break;
		case BITRATE_500K:
				hcan1.Init.Prescaler= 6;                
				hcan1.Init.BS1   = CAN_BS1_8TQ;             
				hcan1.Init.BS2   = CAN_BS2_5TQ;             
        break;
		/*case BITRATE_800K:
				hcan1.Init.Prescaler= 6;                
				hcan1.Init.BS1   = CAN_BS1_6TQ;             
				hcan1.Init.BS2   = CAN_BS2_1TQ;             
				break;*/
		case BITRATE_1M:
				hcan1.Init.Prescaler= 6;                
				hcan1.Init.BS1   = CAN_BS1_5TQ;             
				hcan1.Init.BS2   = CAN_BS2_1TQ;             
				break;
		default:
				hcan1.Init.Prescaler= 3;                
				hcan1.Init.BS1   = CAN_BS1_8TQ;             
				hcan1.Init.BS2   = CAN_BS2_3TQ;             
				break;
	}
	if(HAL_CAN_Init(&hcan1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }
	
  CAN_RxFilerconfig(0,CANRX32IDMASK);                        //接收过滤模式设置

	InitCircleQueue(&RxCAN_Queue);	                           //初始化队列*/
}   
/*******************************************************************************
 * 名称: 
 * 功能: 
 * 形参: 无
 * 返回: 无
 * 说明: 
 ******************************************************************************/
void CAN_RxFilerconfig(uint8_t FilterNum,uint8_t FilterMode)
{
  CAN_FilterConfTypeDef  sFilterConfig; 
   
   sFilterConfig.FilterNumber=FilterNum;	//过滤器号0~13可选
   if(FilterMode==CANRX32IDMASK)
   {
     sFilterConfig.FilterMode=CAN_FILTERMODE_IDMASK; 	//标识符屏蔽模式
     sFilterConfig.FilterScale=CAN_FILTERSCALE_32BIT; 	//32位宽 
   }
   else if(FilterMode==CANRX32IDLIST)
   {
     sFilterConfig.FilterMode=CAN_FILTERMODE_IDLIST; 	//标识符列表模式
     sFilterConfig.FilterScale=CAN_FILTERSCALE_32BIT; 	//32位宽 
   }
   else if(FilterMode==CANRX16IDMASK)
   {
     sFilterConfig.FilterMode=CAN_FILTERMODE_IDMASK; 	//标识符屏蔽模式
     sFilterConfig.FilterScale=CAN_FILTERSCALE_16BIT; 	//16位宽 
   }
   else if(FilterMode==CANRX16IDLIST)
   {
     sFilterConfig.FilterMode=CAN_FILTERMODE_IDLIST; 	//标识符列表模式
     sFilterConfig.FilterScale=CAN_FILTERSCALE_16BIT; 	//16位宽
   }
   //标识符寄存器FxR1
   sFilterConfig.FilterIdHigh=0x0000;//0x000<<5;                  //32位ID，高16位
   sFilterConfig.FilterIdLow=0x0000;                     //低16位
   //屏蔽寄存器FxR2
   sFilterConfig.FilterMaskIdHigh=0x0000;//0xfe1f;                //32位MASK，高16位
   sFilterConfig.FilterMaskIdLow=0x0000;//0xffff;                 //低16位
	
   sFilterConfig.FilterFIFOAssignment=0;//过滤器0关联到FIFO0
   
   sFilterConfig.FilterActivation=ENABLE;//激活过滤器0

	 sFilterConfig.BankNumber=14;      //0-13 filter BANK分配给CAN1,14-27分配给 filter BANK分配给CAN2              
	 
   if(HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig) != HAL_OK)
   {
    /* Filter configuration Error */
    Error_Handler();
   }
	 
} 
/**
  * @brief  Configures the CAN, transmit and receive by polling
  * @param  None
  * @retval PASSED if the reception is well done, FAILED in other case
  */
HAL_StatusTypeDef CAN_Polling(void)
{
  /*##-3- Start the Transmission process #####################################*/
  hcan1.pTxMsg->StdId = 0x11;
  hcan1.pTxMsg->RTR = CAN_RTR_DATA;
  hcan1.pTxMsg->IDE = CAN_ID_STD;
  hcan1.pTxMsg->DLC = 2;
  hcan1.pTxMsg->Data[0] = 0xCA;
  hcan1.pTxMsg->Data[1] = 0xFE;
  
  if(HAL_CAN_Transmit(&hcan1, 10) != HAL_OK)
  {
    /* Transmission Error */
    Error_Handler();
  }
  
  if(HAL_CAN_GetState(&hcan1) != HAL_CAN_STATE_READY)
  {
    return HAL_ERROR;
  }
  
  /*##-4- Start the Reception process ########################################*/
  if(HAL_CAN_Receive(&hcan1, CAN_FIFO0,10) != HAL_OK)
  {
    /* Reception Error */
    Error_Handler();
  }
  
  if(HAL_CAN_GetState(&hcan1) != HAL_CAN_STATE_READY)
  {
    return HAL_ERROR;
  }
  
  if (hcan1.pRxMsg->StdId != 0x11)
  {
    return HAL_ERROR;  
  }

  if (hcan1.pRxMsg->IDE != CAN_ID_STD)
  {
    return HAL_ERROR;
  }

  if (hcan1.pRxMsg->DLC != 2)
  {
    return HAL_ERROR;  
  }

  if ((hcan1.pRxMsg->Data[0]<<8|RxMessage.Data[1]) != 0xCAFE)
  {
    return HAL_ERROR;
  }
  
  return HAL_OK; /* Test Passed */
}

/*******************************************************************************
 * 名称: CAN_Dataprocess
 * 功能: 主机发送状态命令请求来控制GPIO和读取GPIO的状态
 * 形参: 无
 * 返回: 无
 * 说明: 
CAN通讯数据协议=（11位）标准帧+数据（1个 8位）
标准帧：0~6位用于CANID 后面7~10为请求状态
目前只制定两个状态：
0x01 ---读取GPIO的状态  ---0x80+CANID
0x02 ---写入GPIO        ---0x100+CANID

 ******************************************************************************/
void CAN_Dataprocess(void)
{
	CanRxMsgTypeDef RxData;
	uint8_t i;
  uint8_t Canstatus,canbuf[8],CanDeviceID,CANID,TxStdId;
	CanDeviceID=Get_CanID();
	//printf("CANID=%d \r\n",CANID);
	//    CanTxMsg TxMessage;
	  /*CANID=0x01;
    for(i=0;i<8;i++)
    {
	     canbuf[i]=i;//填充发送缓冲区
    }
    if(CANID>0)
	  {
      status=CAN_Send_Msg(canbuf,8,CANID);
	    printf("Tx status=%d \r\n",status);
	    HAL_Delay(100);
		}*/
		if(PopElement(&RxCAN_Queue,&RxData)==TRUE)      //判断队列中有数据才去把数据压出队列
		{	
			 printf("RxData.StdId=%d \r\n",RxData.StdId);
			 CAN_Send_Msg(RxData.Data,RxData.DLC,RxData.StdId);
			    /*CANID=0x07f&RxData.StdId;       //获取CANID
			    Canstatus=RxData.StdId>>7;      //
			    printf("CANID=%d Canstatus=%d \r\n",CANID,Canstatus);
			    if(CANID==CanDeviceID&&Canstatus==0x01)//读取状态
					{
			       TxStdId=CanDeviceID+0x80;
						 canbuf[0]=Get_SwitchValue();
						 CAN_Send_Msg(canbuf,1,TxStdId);
      		   //printf("RxData.StdId=%d \r\n",RxData.StdId);
          }
					else if(CANID==CanDeviceID&&Canstatus==0x02)//写入状态
					{
					   Write_OutputGpio(RxData.Data[0]);         //控制IO
					}*/
		}
}
//can发送一组数据(固定格式:ID为0X12,标准帧,数据帧)	
//len:数据长度(最大为8)				     
//msg:数据指针,最大为8个字节.
//返回值:0,成功;
//		 其他,失败;
HAL_StatusTypeDef CAN_Send_Msg(uint8_t* msg,uint8_t len,uint8_t id)
{	
  uint8_t i;
  hcan1.pTxMsg->StdId=id;         //|0x10;
	hcan1.pTxMsg->ExtId=0x00;
	hcan1.pTxMsg->IDE=CAN_ID_STD; 	// 标准帧
	hcan1.pTxMsg->RTR=CAN_RTR_DATA;		// 数据帧
	hcan1.pTxMsg->DLC=len;				// 要发送的数据长度
	
	for(i=0;i<len;i++)
	hcan1.pTxMsg->Data[i]=msg[i];	

  HAL_CAN_Transmit(&hcan1, 10);	
  //if(HAL_CAN_Transmit(&hcan1, 100) != HAL_OK)
  //{
  //  printf("Send data fail !! \r\n");
		/* Transmission Error */
  //  Error_Handler();
 // }
	//if(HAL_CAN_GetState(&hcan1) != HAL_CAN_STATE_READY)
 // {
	//	printf("Send data fail1 !! \r\n");
  //  return HAL_ERROR;
  //}		

	return HAL_OK; /* Test Passed */
}
/**
  * @brief  Transmission  complete callback in non blocking mode 
  * @param  CanHandle: pointer to a CAN_HandleTypeDef structure that contains
  *         the configuration information for the specified CAN.
  * @retval None
  */
void HAL_CAN_RxCpltCallback(CAN_HandleTypeDef* hcan1)
{
  //if ((CanHandle->pRxMsg->StdId == 0x321)&&(CanHandle->pRxMsg->IDE == CAN_ID_STD) && (CanHandle->pRxMsg->DLC == 2))
  //{
  //  LED_Display(CanHandle->pRxMsg->Data[0]);
  //  ubKeyNumber = CanHandle->pRxMsg->Data[0];
  //}
  /* Receive */
  if(HAL_CAN_Receive_IT(hcan1, CAN_FIFO0) != HAL_OK)
  {
    /* Reception Error */
    Error_Handler();
  }
	 PushElement(&RxCAN_Queue,hcan1->pRxMsg,1);//将接收到的数据压入队列中
	 //printf("hcan1->pRxMsg->StdId=%d\r\n",RxCAN_Queue.CAN_RxMsg->StdId);
	 //printf("received success !! \r\n");
	 
}
/**
  * @brief  CAN1错误处理
  * @param  None
  * @retval None
  */
/*void CAN_ERR_Handle(void)
{	
	static uint8_t bus_off_cnt = 0;
	
    if( (CANERR_Flag & CAN_IT_LEC) == CAN_IT_LEC)// 上次错误号中断
    {    
        CANERR_Flag &= ~CAN_IT_LEC;
		switch(CAN_ErrorCode)
        {
			case CAN_ErrorCode_NoErr:
				printf("No Error\n\r");
				break;
			case CAN_ErrorCode_StuffErr:
				printf("Stuff Error\n\r");
				break;
			case CAN_ErrorCode_FormErr:
				printf("Form Error\n\r");
				break;
			case CAN_ErrorCode_ACKErr:
				printf("Accknowlege Error\n\r");
				break;
			case CAN_ErrorCode_BitRecessiveErr:
				printf("Bit Recessive Error\n\r");
				break;
			case CAN_ErrorCode_BitDominantErr:
				printf("Bit Dominant Error\n\r");
				break;
			case CAN_ErrorCode_CRCErr:
				printf("CRC Error\n\r");
				break;
			case CAN_ErrorCode_SoftwareSetErr:
				printf("Software Set Error\n\r");
				break;
			default:
				break;
		}
    }
    if( (CANERR_Flag & CAN_IT_EPV) == CAN_IT_EPV ) // 错误被动
    {    
        CANERR_Flag &= ~CAN_IT_EPV;
		printf("error Passive\n\r");
    }	
    if( (CANERR_Flag & CAN_IT_EWG) == CAN_IT_EWG) // 错误警告
    {   
        CANERR_Flag &= ~CAN_IT_EWG;
		printf("error active\n\r");	
    }
    if( (CANERR_Flag & CAN_IT_ERR) == CAN_IT_ERR)// 有错误
    {    
        CANERR_Flag &= ~CAN_IT_ERR;
		printf("error occured\n\r");		
    }
    if( (CANERR_Flag & CAN_IT_BOF) == CAN_IT_BOF)  // 总线离线
    {   
        CANERR_Flag &= ~CAN_IT_BOF;
		bus_off_cnt++;
		printf("BUS Off times %d \n\r", bus_off_cnt);		
    }
}

//中断服务函数			    
void USB_LP_CAN1_RX0_IRQHandler(void)
{
  	CanRxMsg RxMessage;
    //u8  i=0;
    
    if(CAN_GetITStatus(CAN1,CAN_IT_FOV0)!=RESET)     //接收邮箱溢出中断
    {
       CAN_ClearITPendingBit(CAN1,CAN_IT_FOV0);     //清中断标志位
       //接收邮箱溢出怎么办，是不是要覆盖掉新的
	}
	else if(CAN_GetITStatus(CAN1,CAN_IT_FF0)!=RESET) //接收邮箱满中断
	{
       CAN_ClearITPendingBit(CAN1,CAN_IT_FF0);
	}
	else                                            //接收邮箱正常中断
	{	
       CAN_Receive(CAN1,0,&RxMessage);
       PushElement(&RxCAN_Queue,RxMessage,1);
	   printf("ID:%d \r\n",RxMessage.StdId);
    }
}
///////////////CAN 中断发送//////////////////////////////
void USB_HP_CAN1_TX_IRQHandler(void)
{
	if(CAN_msg_num[0])
    {
       if(CAN_GetITStatus(CAN1,CAN_IT_RQCP0)!=RESET)
	   {
          CAN_ClearITPendingBit(CAN1,CAN_IT_RQCP0);
		  CAN_ITConfig(CAN1,CAN_IT_TME,DISABLE);				//发送中断允许
          CAN_msg_num[0]=0;
	   }
	}
    if(CAN_msg_num[1])
    {
       if(CAN_GetITStatus(CAN1,CAN_IT_RQCP1)!=RESET)
	   {
          CAN_ClearITPendingBit(CAN1,CAN_IT_RQCP1);
		  CAN_ITConfig(CAN1,CAN_IT_TME,DISABLE);				//发送中断允许
          CAN_msg_num[1]=0;
	   }
	}
	if(CAN_msg_num[2])
	{
      if(CAN_GetITStatus(CAN1,CAN_IT_RQCP2)!=RESET)
	   {
          CAN_ClearITPendingBit(CAN1,CAN_IT_RQCP2);
		  CAN_ITConfig(CAN1,CAN_IT_TME,DISABLE);				//发送中断允许
          CAN_msg_num[2]=0;
	   }
	} 
}
*/

















