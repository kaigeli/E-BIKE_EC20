#include "stm32f0xx_hal.h"
#include "protocol.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "usart.h"
#include "IoT_Hub.h"
#include "control_app.h"
#include "queen.h"

extern RTC_HandleTypeDef hrtc;
extern gps_info_struct gps_info;
uint16_t kfd_sn;
config_struct g_config;
uint8_t g_hb_send_times = 0;
work_state net_work_state = EN_INIT_STATE;
extern uint8_t flag_delay1s;


static uint16_t CRC16_TABLE[] = {
	0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
	0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
	0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
	0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
	0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
	0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
	0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
	0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
	0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
	0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
	0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
	0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
	0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
	0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
	0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
	0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
	0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
	0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
	0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
	0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
	0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
	0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
	0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
	0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
	0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
	0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
	0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
	0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
	0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
	0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
	0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
	0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
};	

/*****************************************************************************
 * FUNCTION
 *	GetCRC
 * DESCRIPTION
 *	获取字节数组的 CRC
 * PARAMETERS
 *	void
 * RETURNS
 *	void
 *****************************************************************************/
uint16_t get_crc16(uint8_t* bytes, uint16_t len)
{	
	uint16_t value = 0xffff;
	int i;

	if (bytes == NULL)
		return 0;

	
	for (i = 0; i < len; i++) {
		value = (value >> 8) ^ CRC16_TABLE[(value ^ bytes[i]) & 0xff];			
	}
	
	return ~value;
}

void hex_convert_str(uint8_t *in,uint8_t len, uint8_t *out)
{
	uint16_t i;
	uint8_t high,low;

	for(i=0; i<len; i++)
	{
		high = *(in+i)/16;
		low = *(in+i)%16;
		
		if(high>=0 && high<=9)
			*(out+2*i) = high+48;
		else if(high >=10 && high<=15)
			*(out+2*i) = high+55;

		if(low>=0 && low<=9)
			*(out+2*i+1) = low+48;
		else if(low >=10 && low<=15)
			*(out+2*i+1) = low+55;
	}
}
int htoi(char s[])  
{  
    int i;  
    int n = 0;  
    if (s[0] == '0' && (s[1]=='x' || s[1]=='X'))  
    {  
        i = 2;  
    }  
    else  
    {  
        i = 0;  
    }  
    for (; (s[i] >= '0' && s[i] <= '9') || (s[i] >= 'a' && s[i] <= 'z') || (s[i] >='A' && s[i] <= 'Z');++i)  
    {  
        if (tolower(s[i]) > '9')  
        {  
            n = 16 * n + (10 + tolower(s[i]) - 'a');  
        }  
        else  
        {  
            n = 16 * n + (tolower(s[i]) - '0');  
        }  
    }  
    return n;  
}
void str_convert_hex(char* in, int len,unsigned char* out)
{
	int i;
	char tmp[3];

	for(i=0; i<len/2; i++)
	{
		memset(tmp, 0, sizeof(tmp));
		memcpy(tmp, in+2*i, 2);
		*(out+i) = (int)htoi(tmp);
	}
}
/*****************************************************************************
 * FUNCTION
 *	hex_str_2_bytes
 * DESCRIPTION
 *	将16进制的串转换成字节数组
 * PARAMETERS
 *	void
 * RETURNS
 *	void
 *****************************************************************************/
uint32_t hex_str_2_bytes(char* str, uint32_t str_len, uint8_t* bytes, uint32_t bytes_len)
{
	uint32_t i;
	uint8_t hex = 0;
	uint8_t offset = 0;

	if (str == NULL || bytes == NULL)
		return false;
	
	if((str_len +1) /2 > bytes_len)
	{
		return false;
	}
	
	if (str_len%2 != 0)
	{
		offset = 1;
	}

	for(i =0; i < str_len; i++)
	{
		//判断hex 字符串的有效性
		if(str[i] >= '0' && str[i] <= '9')
		{
			hex = str[i] - '0';
		}
		else if( str[i] >= 'a' && str[i] <= 'z')
		{
			hex = str[i] - 'a';
		}
		else if(str[i] >= 'A' && str[i] <= 'Z')
		{
			hex = str[i] - 'A';
		}
		else
		{
			return false;
		}

		bytes[(i + offset)/2] += hex << 4*((i + offset + 1)%2);
	}

	return true;
}

/*****************************************************************************
 * FUNCTION
 * kfd_get_sn
 * DESCRIPTION
 *获取sn序列号
 * PARAMETERS
 *	void
 *	
 * RETURNS
 *	void
 *****************************************************************************/
uint16_t kfd_get_sn(void)
{
	kfd_sn++;
	return kfd_sn;
}

/*****************************************************************************
 * FUNCTION
 *  kfd_format_cb_to_buffer
 * DESCRIPTION
 *  组包
 起始符2，校验码2，内容长度1，协议类型1，序列号2，	日期时间6，
 内容context_len,	结束符2
 * PARAMETERS
 *  void  
 *  
 * RETURNS
 *  uint8_t
 *****************************************************************************/
uint8_t send_package(GT_PROT_TYPE_EN prot_type, uint8_t *context,uint8_t context_len)
{
	char* buf;
//	char outbuf2[128]={0};
	uint16_t* crc = NULL;
	uint8_t crc_len,sum_len = 0;
	pkg_head_struct head;
	pkg_tail_struct tail;
	RTC_DateTypeDef sdatestructure;
	RTC_TimeTypeDef stimestructure;

	buf = (char*)malloc(256);
	if(!buf)
		return 0;
	else
		memset(buf,0,256);
	
	head.start = 0xffff;
	head.pack_len = context_len;
	head.prot_type = prot_type;
	head.sn = kfd_get_sn();
	
	HAL_RTC_GetTime(&hrtc, &stimestructure, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &sdatestructure, RTC_FORMAT_BIN);
	head.datetime[0] = sdatestructure.Year;//18;
	head.datetime[1] = sdatestructure.Month;//4
	head.datetime[2] = sdatestructure.Date;//17;
	head.datetime[3] = stimestructure.Hours;//16;
	head.datetime[4] = stimestructure.Minutes;//6;
	head.datetime[5] = stimestructure.Seconds;//3


	Logln(D_INFO,"%d_%d_%d %d:%d:%d", sdatestructure.Year, sdatestructure.Month, sdatestructure.Date,stimestructure.Hours,stimestructure.Minutes,stimestructure.Seconds);
	tail.stop = 0x0a0d;
	//包头
	memcpy(buf,&head,sizeof(pkg_head_struct));
	//包内容
	if(context && context_len>0)
		memcpy(buf+sizeof(pkg_head_struct),context,context_len);
	//包尾
	memcpy(buf+sizeof(pkg_head_struct)+context_len,&tail,sizeof(pkg_tail_struct));
	
	crc = (uint16_t*)(buf + 2);
	crc_len = 	sizeof(pkg_head_struct)+context_len-4;	
	*crc = get_crc16(buf+4, crc_len);
	
	sum_len = sizeof(pkg_head_struct)+context_len+sizeof(pkg_tail_struct);		

//        hex_convert_str(buf,sum_len,outbuf2);
//	Logln(D_INFO,"send=%d,%s\r\n",sum_len,outbuf2);
	
	send_data(buf,sum_len);

	free(buf);
	return sum_len;
}

bool get_work_state(void)
{
	if(net_work_state==EN_CONNECTED_STATE)
		return true;
	else
		return false;
}

void upload_login_package(void)
{
	char* imei = get_imei();
	login_pkg_struct login_package={0};

	login_package.dev_type = 0x10;
	login_package.auth_code = 0;
	
	Logln(D_INFO, "upload_login_package,imei=%s",imei);

	hex_str_2_bytes(imei, strlen(imei), login_package.dev_id, 8);
    
	send_package(EN_GT_PT_LOGIN,(char*)&login_package,sizeof(login_pkg_struct));
}

void upload_hb_package(void)
{
	Logln(D_INFO,"upload_hb_package,times=%d",g_hb_send_times);
	if(g_hb_send_times >= 2)
	{//reconnect
		g_hb_send_times = 0;
		Logln(D_INFO,"hb reconnect");
		net_work_state = EN_CONNECT_STATE;
	}
	else
	{
		send_package(EN_GT_PT_HB,NULL,0);
		g_hb_send_times++;
	}
}

void upload_alarm_package(void)
{
  	Logln(D_INFO,"upload_alarm_package");
	alarm_pkg_struct alarm_pkg;
	uint16_t curr_speed = (uint16_t)(gps_info.speed*1.852);
	uint8_t ind;
	uint8_t i = 0;
	uint8_t is_update = 0;
	uint8_t package_len = 0;
	alarm_struct alarm_info = {0};

	alarm_info.speed_ind = 1;
	alarm_info.speed = 50;
	alarm_info.vibr_ind = 1;
	alarm_info.vibr_value = 10;
	alarm_info.pwr_off_ind = 1;
	
	ind = alarm_info.pwr_low_ind << EN_GT_AT_PWR_LOW | (alarm_info.pwr_off_ind << EN_GT_AT_PWR_OFF) 
		|(alarm_info.vibr_ind << EN_GT_AT_VIBR)|(alarm_info.oil_pwr_ind << EN_GT_AT_OIL_PWR)
		|(alarm_info.speed_ind << EN_GT_AT_SPEED);
	
	for(i = 0; i < EN_GT_AT_END; i++)
	{	
		if(ind & (0x01<<i))
		{	
			if(i == EN_GT_AT_VIBR)
			{
				if(g_config.alarm_switch.vibr == EN_GT_SWT_ON)
				{					
					alarm_pkg.type = EN_GT_AT_VIBR;
					alarm_pkg.value_len = sizeof(alarm_info.vibr_value);
					alarm_pkg.value[0] = alarm_info.vibr_value;
					is_update = true;
				}

				//本次数据已经处理完毕，清除数据标志
				alarm_info.vibr_ind = 0;
				alarm_info.vibr_value = 0;
			}	
			else if(i == EN_GT_AT_SPEED)
			{
				if(g_config.alarm_switch.speed == EN_GT_SWT_ON)	
				{					
					alarm_pkg.type = EN_GT_AT_SPEED;
					alarm_pkg.value_len = sizeof(alarm_info.speed);
					alarm_pkg.value[0] = (alarm_info.speed>>8)&0xff;
					alarm_pkg.value[1] = alarm_info.speed&0xff;
					is_update = true;
				}

				//本次数据已经处理完毕，清除数据标志
				alarm_info.speed_ind = 0;
				alarm_info.speed = 0;
			}	
			else if(i == EN_GT_AT_PWR_LOW)
			{
				if(g_config.alarm_switch.pwr_low == EN_GT_SWT_ON)
				{					
					alarm_pkg.type = EN_GT_AT_PWR_LOW;
					alarm_pkg.value_len = sizeof(alarm_info.pwr_level);
					alarm_pkg.value[0] = alarm_info.pwr_level;
					is_update = true;
				}

				//本次数据已经处理完毕，清除数据标志
				alarm_info.pwr_low_ind = 0;
				alarm_info.pwr_level = 0;
			}
			else if(i == EN_GT_AT_PWR_OFF)
			{
				if(g_config.alarm_switch.pwr_off== EN_GT_SWT_ON)
				{					
					alarm_pkg.type = EN_GT_AT_PWR_OFF;
					alarm_pkg.value_len = 0;
					is_update = true;
				}

				//本次数据已经处理完毕，清除数据标志
				alarm_info.pwr_off_ind= 0;
			}
			else if(i == EN_GT_AT_OIL_PWR)
			{
				if(g_config.alarm_switch.oil_pwr == EN_GT_SWT_ON)
				{					
					alarm_pkg.type = EN_GT_AT_OIL_PWR;
					alarm_pkg.value_len = 0;
					is_update = true;
				}

				//本次数据已经处理完毕，清除数据标志
				alarm_info.oil_pwr_ind= 0;
			}
			else
			{
				is_update = false;
			}
			
			if(is_update)
			{
				is_update = false;
				package_len = alarm_pkg.value_len+2;
				send_package(EN_GT_PT_ALARM,(char*)&alarm_pkg,package_len);
			}	
		}		
	}	
}

uint32_t kfd_lat_long_convert(double latorlong)
{
	uint32_t sum;
	sum = latorlong*1800000;
	
	return (uint32_t)sum;
}
void convert_gps_data_for_protocol(gps_info_struct* gps_data, gps_data_struct* kfd_gps_data)
{

	kfd_gps_data->datetime[0] = (uint8_t)(gps_data->dt.nYear%100);
	kfd_gps_data->datetime[1] = gps_data->dt.nMonth;
	kfd_gps_data->datetime[2] = gps_data->dt.nDay;
	kfd_gps_data->datetime[3] = gps_data->dt.nHour;
	kfd_gps_data->datetime[4] = gps_data->dt.nMin;
	kfd_gps_data->datetime[5] = gps_data->dt.nSec;

	if(gps_data->state == 'A')
		kfd_gps_data->state =EN_GT_GS_A;
	else if(gps_data->state == 'V')
		kfd_gps_data->state = EN_GT_GS_V;
	else
		kfd_gps_data->state = EN_GT_GS_INV;

	kfd_gps_data->latitude = kfd_lat_long_convert(gps_data->latitude);
	if(gps_data->NS== 'N' )
		kfd_gps_data->lat_ind= EN_GT_NORTH;
	else if(gps_data->NS == 'S')
		kfd_gps_data->lat_ind = EN_GT_SOUTH;
	else
		kfd_gps_data->lat_ind = EN_GT_INV;
	
	kfd_gps_data->longitude = kfd_lat_long_convert(gps_data->longitude);
	if(gps_data->EW == 'E' )
		kfd_gps_data->long_ind= EN_GT_EAST;
	else if(gps_data->EW == 'W')
		kfd_gps_data->long_ind = EN_GT_WEST;
	else
		kfd_gps_data->long_ind = EN_GT_INV;

	kfd_gps_data->speed = gps_data->speed*1.852*100;	//节转换为公里/小时再乘以100
	kfd_gps_data->course = (uint16_t)(gps_data->angle*100);
	kfd_gps_data->magnetic_value = (uint16_t)(gps_data->magnetic_value*100);

	if(gps_data->magnetic_ind == 'E' )
		kfd_gps_data->magnetic_ind= EN_GT_EAST;
	else if(gps_data->magnetic_ind == 'W')
		kfd_gps_data->magnetic_ind = EN_GT_WEST;
	else
		kfd_gps_data->magnetic_ind = EN_GT_INV;

	if(gps_data->mode == 'A' )
		kfd_gps_data->mode= EN_GT_GM_A;
	else if(gps_data->mode == 'D')
		kfd_gps_data->mode = EN_GT_GM_D;
	else if(gps_data->mode == 'E')
		kfd_gps_data->mode = EN_GT_GM_E;
	else if(gps_data->mode == 'N')
		kfd_gps_data->mode = EN_GT_GM_N;
	else
		kfd_gps_data->mode = EN_GT_GM_INV;

	kfd_gps_data->sat_uesed = gps_data->sat_uesd;
	kfd_gps_data->msl_altitude = gps_data->altitude*10;	//分米
	kfd_gps_data->hdop = gps_data->hdop*100;

}

void upload_gps_package(void)
{
	static lat_lon_struct lat_lon;
	gps_pkg_struct gps_pkg;
	gps_info_struct* p_gps = &gps_info;
	gps_data_struct gps_pkg_gps = {0};

	Logln(D_INFO,"upload_gps_package,lat=%f,lon=%f",p_gps->latitude,p_gps->longitude);
	
	if(check_zhendong() || get_electric_gate_status())
	{
		if(p_gps->latitude!=lat_lon.lat && p_gps->longitude!=lat_lon.lon)
		{
			lat_lon.lat = p_gps->latitude;
			lat_lon.lon = p_gps->longitude;
			convert_gps_data_for_protocol(p_gps,&gps_pkg_gps);
					
			//定位类型GPS
			gps_pkg.loc_type = EN_GT_LT_GPS;
			gps_pkg.latitude = gps_pkg_gps.latitude;
			gps_pkg.longitude = gps_pkg_gps.longitude;
			gps_pkg.speed = gps_pkg_gps.speed;					
			
			//航向
			gps_pkg.course= gps_pkg_gps.course;
			//可用卫星
			gps_pkg.reserv_satnum = gps_pkg_gps.sat_uesed;

			if(gps_pkg_gps.lat_ind == EN_GT_SOUTH)
			{
				gps_pkg.property.lat_ind = 0;	
			}
			else if(gps_pkg_gps.lat_ind == EN_GT_NORTH)
			{
				gps_pkg.property.lat_ind = 1;
			}

			if(gps_pkg_gps.long_ind == EN_GT_WEST)
			{
				gps_pkg.property.long_ind = 0;	
			}
			else if(gps_pkg_gps.long_ind == EN_GT_EAST)
			{
				gps_pkg.property.long_ind = 1;
			}

			if(gps_pkg_gps.mode == EN_GT_GM_A)
			{
				gps_pkg.property.mode = 0;	
			}
			else if(gps_pkg_gps.mode == EN_GT_GM_D)
			{
				gps_pkg.property.mode = 1;
			}
			else if(gps_pkg_gps.mode == EN_GT_GM_E)
			{
				gps_pkg.property.mode = 2;
			}
			else if(gps_pkg_gps.mode == EN_GT_GM_N)
			{
				gps_pkg.property.mode = 3;
			}

			gps_pkg.bid = 0;
			
			send_package(EN_GT_PT_GPS,(char*)&gps_pkg,sizeof(gps_pkg_struct));
		}
	}
}

void upload_lbs_package(void)
{
	lbs_pkg_struct lbs_pkg;

	send_package(EN_GT_PT_LBS,(char*)&lbs_pkg,sizeof(lbs_pkg_struct));
}

void upload_version_package(void)
{
	data_pkg_struct data_pkg;

	data_pkg.type = EN_GT_DT_VER;
	data_pkg.value_len = strlen(DEV_VER); 
	strcpy(data_pkg.value,DEV_VER);

	Logln(D_INFO,"upload_version_package");
	send_package(EN_GT_PT_DEV_DATA, (char*)&data_pkg, data_pkg.value_len+2);

}

void upload_imsi_package(void)
{
	data_pkg_struct data_pkg;

	data_pkg.type = EN_GT_DT_IMSI;
	data_pkg.value_len = strlen(get_imsi()); 
	strcpy(data_pkg.value,get_imsi());

	Logln(D_INFO,"upload_imsi_package,imsi=%s",get_imsi());

	send_package(EN_GT_PT_DEV_DATA, (char*)&data_pkg, data_pkg.value_len+2);
}
void upload_bt_addr_package(void)
{
	data_pkg_struct data_pkg;

	data_pkg.type = EN_GT_DT_BT_ADDR;
	data_pkg.value_len = strlen(dev_info.addr); 
	strcpy(data_pkg.value,dev_info.addr);

	Logln(D_INFO,"upload_bt_addr_package");

	send_package(EN_GT_PT_DEV_DATA, (char*)&data_pkg, data_pkg.value_len+2);
}
void upload_ebike_data_package(void)
{
  	Logln(D_INFO,"upload_ebike_data_package");
	ebike_pkg_struct ebike_pkg;
	uint8_t package_len;

	get_ebike_data(&ebike_pkg);
	package_len = ebike_pkg.value_len + 2;

	send_package(EN_GT_PT_CONTROL, (char*)&ebike_pkg, package_len);
}

void upload_give_back_package(uint8_t gate)
{
	uint8_t num=0;
	give_back_pkg_struct give_back_pkg;
	gps_data_struct gps_data;
	uint8_t len;

/*增加还车时判断主电源如没插上就提示失败*/
	if(get_bat_connect_status())	
	{
		if(gate)
		{
			give_back_pkg.lock_state = 0;
		}
		else
		{
			give_back_pkg.lock_state = 1;
		}
	}
	else
	{
		give_back_pkg.lock_state = 0;
	}
	
	Logln(D_INFO,"lock_state=%d,gate=%d,gps=%c",give_back_pkg.lock_state,gate,gps_info.state);

	if(gps_info.state=='A')
	{
		memset(&gps_data,0,sizeof(gps_data_struct));
		convert_gps_data_for_protocol(&gps_info,&gps_data);
		
		give_back_pkg.gps_array[num].latitude = gps_data.latitude;
		give_back_pkg.gps_array[num].longitude = gps_data.longitude;
		give_back_pkg.gps_array[num].speed = gps_data.speed;					
		give_back_pkg.gps_array[num].course= gps_data.course;
		give_back_pkg.gps_array[num].reserv_satnum = gps_data.sat_uesed;
		if(gps_data.lat_ind == EN_GT_SOUTH)
		{
			give_back_pkg.gps_array[num].property.lat_ind = 0;	
		}
		else if(gps_data.lat_ind == EN_GT_NORTH)
		{
			give_back_pkg.gps_array[num].property.lat_ind = 1;
		}

		if(gps_data.long_ind == EN_GT_WEST)
		{
			give_back_pkg.gps_array[num].property.long_ind = 0;	
		}
		else if(gps_data.long_ind == EN_GT_EAST)
		{
			give_back_pkg.gps_array[num].property.long_ind = 1;
		}

		if(gps_data.mode == EN_GT_GM_A)
		{
			give_back_pkg.gps_array[num].property.mode = 0;	
		}
		else if(gps_data.mode == EN_GT_GM_D)
		{
			give_back_pkg.gps_array[num].property.mode = 1;
		}
		else if(gps_data.mode == EN_GT_GM_E)
		{
			give_back_pkg.gps_array[num].property.mode = 2;
		}
		else if(gps_data.mode == EN_GT_GM_N)
		{
			give_back_pkg.gps_array[num].property.mode = 3;
		}
		num = 1;
	}
	
	give_back_pkg.gps_data_num = num;
	len = 2+sizeof(giveback_cell_struct)*num;
	
	send_package(EN_GT_PT_GIVE_BACK,(uint8_t*)&give_back_pkg, len);
}

void push_interval_package_process(void)
{
	static uint8_t delay_index=0;
    
	if(!get_work_state())
		return;

	if(flag_delay1s)
	{
		RxMsgTypeDef msgType;
		
		flag_delay1s = 0;

		if ((delay_index)%15 == 0)	//15秒
		{
			msgType.Data[0] = AT_UP_GPS;
			PushElement(&at_send_Queue, msgType, 1);
		}
		else 	if((delay_index+1)%30==0)
		{
			msgType.Data[0] = AT_UP_ALARM;
			PushElement(&at_send_Queue, msgType, 1);
		} 
		else if((delay_index+2)%30==0)
		{
			msgType.Data[0] = AT_UP_CSQ;
			PushElement(&at_send_Queue, msgType, 1);
			msgType.Data[0] = AT_UP_EBIKE;
			PushElement(&at_send_Queue, msgType, 1);
		}
		else if((delay_index+3)%60==0)
		{
			msgType.Data[0] = AT_UP_HB;
			PushElement(&at_send_Queue, msgType, 1);
		}
		else if((delay_index+4)%5==0)
		{
			msgType.Data[0] = AT_UP_QGPSLOC;
			PushElement(&at_send_Queue, msgType, 1);
		//	msgType.Data[0] = AT_UP_GGA;
		//	PushElement(&at_send_Queue, msgType, 1);
		}

		
		{//电门开关上报状态
			static uint8_t last_acc=0;
			if(last_acc != get_electric_gate_status())	
			{
				last_acc = get_electric_gate_status();
				msgType.Data[0] = AT_UP_EBIKE;
				PushElement(&at_send_Queue, msgType, 1);
			}
		}
		
		{//电池插拔上报状态
			static bool flag=false;
			if(get_bat_connect_status() && flag)
			{
				Logln(D_INFO, "BAT CONNECT");
				msgType.Data[0] = AT_UP_EBIKE;
				PushElement(&at_send_Queue, msgType, 1);
				flag = false;
			}
			else if(!get_bat_connect_status() && !flag)
			{
				Logln(D_INFO,"BAT DISCONNECT");
				msgType.Data[0] = AT_UP_EBIKE;
				PushElement(&at_send_Queue, msgType, 1);
				flag = true;
			}
		}
		
		delay_index++;

		if(delay_index>59)
			delay_index = 0;
	}
}

void PopATcmd(void)
{
	RxMsgTypeDef at_send_type;

	if(PopElement(&at_send_Queue,&at_send_type))
	{
		switch(at_send_type.Data[0])
		{
			case AT_UP_QGPSLOC:
				send_gps_QGPSLOC_cmd();
				break;
		/*	case AT_UP_GGA:
				send_gps_gga_cmd();
				break;*/
			case AT_UP_LOGIN:
				upload_login_package();
				break;
			case AT_UP_VER:
				upload_version_package();
				break;
			case AT_UP_IMSI:
				upload_imsi_package();
				break;
			case AT_UP_BT_ADDR:
				upload_bt_addr_package();
				break;
			case AT_UP_GPS:
				upload_gps_package();
				break;
			case AT_UP_ALARM:
				upload_alarm_package();
				break;
			case AT_UP_EBIKE:
				upload_ebike_data_package();
				break;
			case AT_UP_HB:
				upload_hb_package();
				break;
			case AT_UP_CSQ:
				Send_AT_Command_ext(AT_CSQ, 1);
				break;
			case AT_UP_GIVEBACK:
				upload_give_back_package(g_flash.acc);
				break;
			default:
				break;
		}
	}

}

void calibration_time(uint8_t* buf)
{
	uint8_t* date_time;
	RTC_DateTypeDef sdatestructure;
	RTC_TimeTypeDef stimestructure;


	date_time = ((uint8_t*)buf + sizeof(pkg_head_struct) - 6);
	sdatestructure.Year = date_time[0];
	sdatestructure.Month = date_time[1];
	sdatestructure.Date = date_time[2];
	sdatestructure.WeekDay = 0x01;
	stimestructure.Hours = date_time[3];
	stimestructure.Minutes = date_time[4];
	stimestructure.Seconds = date_time[5];
	stimestructure.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	stimestructure.StoreOperation = RTC_STOREOPERATION_RESET;

Logln(D_INFO, "cali %d-%d-%d %d:%d:%d",sdatestructure.Year,sdatestructure.Month,sdatestructure.Date,stimestructure.Hours,stimestructure.Minutes,stimestructure.Seconds);
  	HAL_RTC_SetTime(&hrtc, &stimestructure, RTC_FORMAT_BIN);//设置时分秒
	HAL_RTC_SetDate(&hrtc, &sdatestructure, RTC_FORMAT_BIN);//设置年月日

	HAL_RTC_GetTime(&hrtc, &stimestructure, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &sdatestructure, RTC_FORMAT_BIN);
Logln(D_INFO, " %d-%d-%d %d:%d:%d",sdatestructure.Year,sdatestructure.Month,sdatestructure.Date,stimestructure.Hours,stimestructure.Minutes,stimestructure.Seconds);

}

void parse_dev_data(data_pkg_struct* data)
{
	if (data == NULL)
		return;
	
	switch(data->type)
	{					
		case EN_GT_DT_VER:
//			parse_ver_package(data->value,data->value_len);
			break;
		case EN_GT_DT_IMSI:
			parse_imsi_package(data->value,data->value_len);
			break;
		default:
			break;
	}
}

bool protocol_proc(unsigned char* buf ,int len)
{	
	pkg_head_struct* head;
//	char out[256]={0};
	int crc1,crc2;

	if (buf == NULL)
		return true;

	head = (pkg_head_struct*)buf;

	crc2 = buf[2]*0x100+buf[3];
	crc1 = get_crc16(buf+4, len-6);
//	hex_convert_str(buf,len, out);

	if(crc1 != crc2)
	{
		Logln(D_INFO,"check sum error");
		return false;
	}

	switch(head->prot_type)
	{
		case EN_GT_PT_LOGIN:						
		{	
			RxMsgTypeDef msgType;
			
			Logln(D_INFO,"login rsp sn ok");
			calibration_time(buf);
			msgType.Data[0] = AT_UP_VER;
			PushElement(&at_send_Queue, msgType, 1);
			msgType.Data[0] = AT_UP_IMSI;
			PushElement(&at_send_Queue, msgType, 1);
			msgType.Data[0] = AT_UP_BT_ADDR;
			PushElement(&at_send_Queue, msgType, 1);
			net_work_state = EN_CONNECTED_STATE;
			break;																	
		}		
		case EN_GT_PT_GPS: 			
		{	
			Logln(D_INFO,"EN_GT_PT_GPS");
			break;
		}
		case EN_GT_PT_STATUS: 			
		{	
			Logln(D_INFO,"EN_GT_PT_STATUS");
			break;
		}
		case EN_GT_PT_HB: 
		{	
			g_hb_send_times = 0;
			Logln(D_INFO,"HB rsp sn ok################");
			break; 
		}
		case EN_GT_PT_ALARM: 					
		{	
			Logln(D_INFO,"EN_GT_PT_ALARM");
			break;
		}		
		case EN_GT_PT_DEV_DATA:			
		{	
			data_pkg_struct* data;

			Logln(D_INFO,"EN_GT_PT_DEV_DATA");
			data = (data_pkg_struct*)((uint8_t*)head + sizeof(pkg_head_struct));	
			parse_dev_data(data);
			break;						
		}			
		case EN_GT_PT_CONTROL: 					
		{	
			ebike_cmd_struct* control_data;
			Logln(D_INFO,"EN_GT_PT_CONTROL");
			control_data = (ebike_cmd_struct*)((uint8_t*)head + sizeof(pkg_head_struct));
			parse_network_cmd(control_data);
			break;
		}
		case EN_GT_PT_LBS:
		{
			Logln(D_INFO,"EN_GT_PT_LBS");
			break;
		}
		case EN_GT_PT_SRV_DATA:
		{
			Logln(D_INFO,"EN_GT_PT_SRV_DATA");
			break;	
		}		
		default:
			Logln(D_INFO,"EN_GT_PT_DEFAULT");
			break;			
	}					

	return true;
}

uint8_t protocol_parse(unsigned char *pBuf, int len)
{
	unsigned char req[512]={0};
	int i;
	char* head,*tail,head_first=1;
	uint8_t ret=0;

	if(len<256)
	{
		hex_convert_str(pBuf,len, req);
		Logln(D_INFO,"len = %d,rec=%s",len,req);
	}
	
	for(i = 0; i<len-1; i++)
	{
		//找包头 0xffff
		if(pBuf[i]==0xff && pBuf[i+1]==0xff && head_first)
		{
			head = pBuf+i;
			head_first = 0;
		}
		else if((pBuf[i]==0x0d && pBuf[i+1]==0x0a))
		{
			tail = pBuf+i+2;//结尾

			//验证长度合法性
			if(tail - head == head[4] + PACKET_FRAME_LEN)
			{
				memset(req, 0, sizeof(req));
				memcpy(req, head, tail-head);
				protocol_proc(req,tail-head);
                		head_first = 1;
				//找到合法结尾
				head = tail;
               			ret = 1;
			}
			//继续找结尾
			i +=1;	//代码加1，for循环自加1 ，等效于向后偏移2个字节（0d0a）			
		}
	}
    return ret;
}
