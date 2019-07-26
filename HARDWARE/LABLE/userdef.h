#ifndef __USERDEF_H__
#define __USERDEF_H__
//#ifndef typedef_h
//#define typedef_h
#include "typedef.h"
/*date type defination*/
typedef signed 	 char  		int8;
typedef signed 	 short 	  int16;
typedef signed 	 long  		int32;
typedef unsigned char  		uint8;
typedef unsigned short 		uint16;
typedef unsigned long  	  uint32;
typedef unsigned int      UINT16;

struct FLAG
{
   /*uint8 bit7:1;
   uint8 bit6:1;
   uint8 bit5:1;
   uint8 bit4:1;
   uint8 bit3:1;
   uint8 bit2:1;
   uint8 bit1:1;
   uint8 bit0:1;*/
    uint8 bit0:1;
    uint8 bit1:1;
    uint8 bit2:1;
    uint8 bit3:1;
    uint8 bit4:1;
    uint8 bit5:1;
    uint8 bit6:1;
    uint8 bit7:1;      
};
union FLAGS
{
    uint8 byte;
    struct FLAG bit;
};
/*
typedef struct
{
 // UINT16 fover:1;
 // UINT16 frequest:1;
 // UINT16 count:14;
  UINT16 count:14;         //低14位
  UINT16 frequest:1;       //第15位
  UINT16 fover:1;          //第16位
}TIME16;*/
typedef struct
{
   UINT16 DegreeC;
   uint8  flagerror;
   uint8  flagBelow0;
   uint8  flagmorethan60;
}Temp;        //定义温度参数
typedef struct
{
   UINT16 strmq2vol;   //动态电压值AD值
   UINT16 stamq2vol;   //静态电压值AD值 
   UINT16 mq2section;  //静态电压值与动态电压值之间的差值
   uint8  error;       //故障标志
   uint8  Normal;      //运行标志
   uint8  question;    //静态电压大于动态电压标志
}Mq2;
/*typedef union
{
   UINT16 word;
   TIME16 time;
}UTIME16;*/

/*struct MQDATA
{
   //uint32 strvol:13;       //静态电压值
   //uint32 stavol:13;       //动态电压值
   uint8 fault:1;         //故障标志
   uint8 Normal:1;        //运行状态标志
};
union MQ2
{
    uint8 byte8;
	struct MQDATA flag;
};*/

#endif
