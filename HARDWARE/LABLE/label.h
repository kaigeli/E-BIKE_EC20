
//变量定义区域

#ifndef label_h
#define label_h
#include "typedef.h"
#include "userdef.h"
#ifdef LABEL
#define extern
#endif

extern uint16 test;
extern uint16 test1;
extern uint8 test2;
extern uint8 test5;
//extern uint8 dir;           //
extern uint16 led0pwmval;
extern uint16 adcx;       //AD采集缓存变量
extern float temp;        //AD采集处理变量

extern uint16 ADbuf1[64];        //ad1缓存数组a
extern uint16 ADave1;    //采集ad1的平均值
extern uint16 ADave2;    //采集ad2的平均值 
extern uint16 msecnum;     //秒计数

extern uint16 mq2adjust;  //MQ2烟雾传感器校准值
extern uint16 mq2adjust1;

extern uint16 countover;   //计数溢出计数
extern uint16 speed;
     
extern uint32 frq;           //脉冲频率值            
extern uint16 DutyCycle;

extern uint16 res;
extern uint8 canbuf[8];
extern uint8 canbuf1[8];
extern uint8 status;
extern uint8 sendcount;
extern uint8 CAN_msg_num[3];    //发送邮箱标记
//extern uint8 KeyStatus;
/*typedef struct
{
  UTIME16 testone;
}_tm_4ms16;
extern _tm_4ms16 tm_4ms16;*/
extern Temp TempData;      //包含温度的参数
extern Mq2  Mq2Datas;       //包含气体的参数
 
extern union FLAGS flag0; 

#define flagtest            flag0.byte
#define flagfirstmq2        flag0.bit.bit0     //MQ2烟雾传感器上电预热标志
//#define flagmq2             flag0.bit.bit1     //MQ2烟雾传感器动态采集标志
//#define flagcalibration     flag0.bit.bit2     //校准是否正常标志
//#define flagmq2Normal       flag0.bit.bit2   //MQ2烟雾传感器检测是否正常标志
//#define flagmq2fault        flag0.bit.bit3   //MQ2烟雾传感器故障标志
//#define flagtempfault       flag0.bit.bit4   //温度传感器运行状态标志
//#define flagtempBelow0      flag0.bit.bit5   //0度以下标志
//#define flagtempmorethan60  flag0.bit.bit6   //超过60度以上标志

//extern union TEMP TempData;  //包含温度的参数
//extern union MQ2  Mq2Datas;   //包含气体的参数




#endif

