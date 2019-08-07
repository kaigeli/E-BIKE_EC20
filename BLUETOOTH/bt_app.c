#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "bt_app.h"
#include "Control_app.h"
#include "protocol.h"
#include "exti.h"
#include "time.h"
#include "adc.h"
#include "gpio.h"
#include "voice.h"
#include "usart.h"
#include "IoT_Hub.h"
#include "Control_app.h"
//#include "flash.h"

extern uint32_t diff_rotate,diff_mileage,diff_shake;
extern battery_info_struct curr_bat;
extern gps_info_struct gps_info;
extern uint8_t connect_times;

extern RTC_HandleTypeDef hrtc;

void bt_send(uint8_t*data, uint16_t len)
{
	uart3_send(data, len);
}

uint32_t GetTimeStamp(void)
{
	struct tm info;
	uint32_t sec;
	RTC_DateTypeDef sdatestructure;
	RTC_TimeTypeDef stimestructure;

	HAL_RTC_GetTime(&hrtc, &stimestructure, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &sdatestructure, RTC_FORMAT_BIN);
		
	info.tm_year = 100 + sdatestructure.Year;
	info.tm_mon = sdatestructure.Month - 1;
	info.tm_mday = sdatestructure.Date;
	info.tm_hour = stimestructure.Hours - 8;  //北京时间-8为UTC时间
	info.tm_min = stimestructure.Minutes;
	info.tm_sec = stimestructure.Seconds;
	sec = mktime(&info);   //sec就是秒数

	return sec;
}

/*蓝牙发送接口*/
void bt_send_data(uint8_t* data,uint16_t len)
{
	uint8_t tmp[128]={0};
	tmp[0]='F';
	tmp[1]='1';
	memcpy(tmp+2,data,len);
	bt_send(tmp,len+2);
}

void bt_prepare_send_data(uint8_t operate, uint8_t param_len, uint8_t* param)
{
	uint8_t *buffer=(uint8_t*)malloc(32);
	uint32_t ts = GetTimeStamp();
	uint16_t crc;

	if(!buffer)
		return;
	else
		memset(buffer,0,32);
	
	buffer[0] = 0x3a;
	buffer[1] = 0x02;
	buffer[2] = operate;
	buffer[3] = param_len;
	if(param&&param_len)
		memcpy(buffer+4, param, param_len);

	buffer[4+param_len]=ts&0xff;
	buffer[4+param_len+1]=(ts>>8)&0xff;
	buffer[4+param_len+2]=(ts>>16)&0xff;
	buffer[4+param_len+3]=(ts>>24)&0xff;
	crc = get_crc16(buffer+1, 3+param_len+4);
	buffer[4+param_len+4]=crc&0xff;
	buffer[4+param_len+5]=(crc>>8)&0xff;
	
	buffer[4+param_len+6]=0x0d;	
	buffer[4+param_len+7]=0x0a;	

	bt_send_data(buffer,12+param_len);

	free(buffer);
}

void bt_prepare_send_data_ext(uint8_t operate, uint8_t param_len, uint8_t* param)
{
	uint8_t *buffer=(uint8_t*)malloc(32);
	uint32_t ts = GetTimeStamp();
	uint16_t crc;

	if(!buffer)
		return;
	else
		memset(buffer,0,32);
	
	buffer[0] = 0x3a;
	buffer[1] = 0x02;
	buffer[2] = operate;
	if(param&&param_len)
		memcpy(buffer+3, param, param_len);

	buffer[3+param_len]=ts&0xff;
	buffer[3+param_len+1]=(ts>>8)&0xff;
	buffer[3+param_len+2]=(ts>>16)&0xff;
	buffer[3+param_len+3]=(ts>>24)&0xff;
	crc = get_crc16(buffer+1, 2+param_len+4);
	buffer[3+param_len+4]=crc&0xff;
	buffer[3+param_len+5]=(crc>>8)&0xff;
	
	buffer[3+param_len+6]=0x0d;	
	buffer[3+param_len+7]=0x0a;	
	
	bt_send_data(buffer,11+param_len);

	free(buffer);
}

void read_data(uint8_t operate)
{
	read_data_struct data;

	if(curr_bat.voltage>0 && curr_bat.voltage!=0xffff)	
		data.volt  = curr_bat.voltage/10;
	else
		data.volt  = get_bat_vol();

	if(g_flash.motor==1)
		data.hall = rotate_count/8;
	else
		data.hall = mileage_count/8;
	
	if(get_electric_gate_status())
		data.lock = 0;
	else
		data.lock = 1;

	Logln(D_INFO, "vol=%d,hall=%d,lock=%d\r\n",data.volt,data.hall,data.lock);

	bt_prepare_send_data(operate, 0x07, (uint8_t*)&data);
}

void send_ok_cmd(uint8_t operate)
{
	//char param[6]={0}; 
  	uint8_t param[1]={0};
	param[0] = 0;
	bt_prepare_send_data(operate, 1, param);
}
void send_error_cmd(uint8_t operate,uint8_t type)
{
	uint8_t param[6]={0};
	//char param[6]={0};

	param[0] = type;
	bt_prepare_send_data(operate, 1, param);
}
void bt_unlock(void)
{	
	flag_tangze_unlock = 1;
	g_flash.acc |= BT_OPEN;
	write_flash(CONFIG_ADDR, (uint8_t*)&g_flash,(uint16_t)sizeof(flash_struct));
	open_electric_door();
}

void bt_giveback_package(uint8_t operate)
{
	bt_giveback_struct bt_giveback_data;
	gps_data_struct gps_tracker_gps;
	uint8_t len;

/*增加还车时判断主电源如没插上就提示失败*/
	if(get_bat_connect_status())	
	{
		if(g_flash.acc)
		{
			bt_giveback_data.lock_state = 0;
		}
		else
		{
			bt_giveback_data.lock_state = 1;
		}
	}
	else
	{
		bt_giveback_data.lock_state = 0;
	}
	
	convert_gps_data_for_protocol(&gps_info,&gps_tracker_gps);
	
	bt_giveback_data.gps.latitude = gps_tracker_gps.latitude;
	bt_giveback_data.gps.longitude = gps_tracker_gps.longitude;
		
	len = sizeof(bt_giveback_struct);

	bt_prepare_send_data_ext(operate, len, (uint8_t*)&bt_giveback_data);
}

void bt_parse_proc(uint8_t* buf, uint16_t len)
{
//	uint8_t out[32]={0};
	uint16_t crc1,crc2;
	uint8_t cmd = buf[2];
	uint32_t timestamp1 = GetTimeStamp();
	uint32_t timestamp2 = buf[len-8]+buf[len-7]*0x100+buf[len-6]*0x10000+buf[len-5]*0x1000000;

//	 hex_convert_str(buf,len,out);
	 	
	crc1 = get_crc16(buf+1,len-5);
	crc2 = buf[len-4]+buf[len-3]*0x100;
	if(crc1 != crc2)
	{
		send_error_cmd(cmd,2);
		Logln(D_INFO, "bt checksum error,crc1=%x,crc2=%x",crc1,crc2);
		return;
	}
	if(0)//abs(timestamp1-timestamp2)>300)
	{
		send_error_cmd(cmd,3);
		Logln(D_INFO, "bt timestamp error");
		return;
	}
	Logln(D_INFO, "BT cmd=%d",cmd);
	switch(cmd)
	{
		case BT_LOCK:
		{
			if(g_flash.acc & KEY_OPEN)
			{
				send_error_cmd(cmd,1);
			}
			else if(g_flash.acc &(GPRS_OPEN|BT_OPEN))
			{
				if(lock_bike())
				{
					voice_play(VOICE_LOCK,1);
					send_ok_cmd(cmd);	
		  		}
		  		else
		  		{
					send_error_cmd(cmd,1);
		  		}
			}
			else
			{
				send_error_cmd(cmd,1);
			}
			break;
		}
		case BT_UNLOCK:
		{
			if(!g_flash.acc)
			{
				bt_unlock();
				voice_play(VOICE_UNLOCK,1);
				send_ok_cmd(cmd);
			}
			else
			{
				send_error_cmd(cmd,1);
			}
			break;
		}
		case BT_SEARCH:
		{
			voice_play(VOICE_SEARCH,g_flash.search_times);
			send_ok_cmd(cmd);
			break;
		}
		case BT_READ_DATA:
		{
			read_data(cmd);
			break;
		}
		case BT_DIANCHI:
		{
			open_dianchi_lock();
			send_ok_cmd(cmd);
			break;
		}
		case BT_GIVEBACK:
		{
			if(lock_bike())
			{
				voice_play(VOICE_LOCK,1);
			}
			bt_giveback_package(cmd);
			break;
		}
		case BT_GIVEBACK_SUCCESS:
		{
			//gps power off
			send_ok_cmd(cmd);
			break;
		}
		case BT_RESET:
		{
			//reset system
			send_ok_cmd(cmd);
			reset_system();
			break;
		}
		case BT_SIGNAL:
		{
			char param[4]={0}; 
			param[0] = convert_csq(dev_info.csq);
			param[1] = gps_info.sat_view;
			param[2] = gps_info.sat_uesd;
			param[3] = connect_times;

			Logln(D_INFO, "sig=%d,view=%d,used=%d,connect_times=%d",param[0],param[1],param[2],connect_times);
			bt_prepare_send_data(cmd, 4, param);
			break;
		}
		default:
			send_error_cmd(cmd,1);
			break;
	}
	
}

uint8_t get_int_number_two(char * s)
{
	uint8_t result=0;
	char tmp[16]={0};
	char* stopstr;
	
	memcpy(tmp,s,2);

	result = strtol(tmp,&stopstr,16);

	return result;
}

bool parse_bt_cmd(int8_t* buf, uint16_t len)
{
	uint8_t i,*head=NULL,*tail=NULL;
	uint8_t req[32]={0};
	uint8_t head_first = 1;
	bool flag = false;

	for(i=0; i<len; i++)
	{
		if(buf[i]==0x3a && head_first)
		{
			head = buf+i;
			head_first = 0;
		}
		else if(buf[i]==0x0d&&buf[i+1]==0x0a&& (i+2==len))
		{
			tail = buf+i+2;
			if(head)
			{
				memcpy(req, head, tail-head);
				bt_parse_proc(req,tail-head);
				flag = true;
			}
		}
	}

	if(!flag)
	{
		char *tmp=strstr(buf,"5b");
		char mac[6], name[10];
		if(tmp && strstr(buf,"CC"))
		{
			memcpy(dev_info.addr, tmp, 12);
			memcpy(dev_info.name,tmp+12, 10);
			flag = true;
		}
		else if(strstr(buf,"writeOK"))
		{
			Logln(D_INFO,"write mac&name ok");
			HAL_Delay(50);
			reset_system();
		}
		else if(strstr(buf,"writefail"))
		{
			Logln(D_INFO,"write mac&name fail");
		}
	}
	return flag;
}

