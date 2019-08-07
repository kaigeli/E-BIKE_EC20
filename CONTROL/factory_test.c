#include "factory_test.h"
#include "gpio.h"
#include "voice.h"
#include "exti.h"
#include "Control_app.h"
#include "IoT_Hub.h"
#include "adc.h"

#define DIANMEN_TEST (44|0x80)
#define KEY_DETECT_TEST (8|0x80)
#define LOCK_A_TEST (28|0x80)
#define LOCK_B_TEST (27|0x80)
#define CONTROL_TEST (54|0x80)

extern gps_info_struct gps_info;

zt_factory_test_struct g_test_table[];
uint8_t g_test_index = 0;
uint32_t g_hall,g_lundong,g_shake;
uint8_t g_key_state;
extern void MODULE_RST(void);

void test_init(void)
{
	MODULE_RST();

	gsm_led_on;
	g_key_state = read_key_det;
	HAL_Delay(2000);
	Send_AT_Command_Timeout(AT_ATE0, 5); 
	Send_AT_Command_Timeout(AT_QGPS_ON, 1);

	g_test_table[TEST_INIT].result=1;
}

void test_cpin(void)
{
	if(Send_AT_Command_Timeout(AT_CPIN,2))
		g_test_table[TEST_CPIN].result = 1;
	else
		g_test_table[TEST_CPIN].result = 0;
}

void test_dianmen(void)
{
	voice_play(VOICE_UNLOCK,1);
	ACC_on;
	g_test_table[TEST_DIANMEN].result=1;
	g_lundong = rotate_count;
	g_shake = shake_count;	
}
void test_off_dianmen(void)
{
	voice_play(VOICE_LOCK,1);
	ACC_off;
}
void test_bat_led(void)
{
	battery_B_on;
	g_test_table[TEST_BATLOCK].result=1;
}
void test_off_bat_led(void)
{
	battery_B_off;
}
void test_dianji_led(void)
{
	motor_A_on;
	g_test_table[TEST_DIANJILOCK].result=1;
}
void test_off_dianji_led(void)
{
	motor_A_off;
}
void test_a_led(void)
{
	tangze_A_on;
	g_test_table[TEST_A].result=1;	
}
void test_off_a_led(void)
{
	tangze_A_off;
}
void test_b_led(void)
{
	tangze_B_on;
	g_test_table[TEST_B].result=1;	
}
void test_off_b_led(void)
{
	tangze_B_off;
}
void test_voice(void)
{
	g_test_table[TEST_VOICE].result=1;
	voice_play(VOICE_ALARM,1);
}
void test_off_voice(void)
{
}

void test_off_gsen(void)
{
	uint8_t x=0,y=0,z=0;
	uint8_t buf[50]={0};

	uint32_t deta_shake = shake_count-g_shake;
	sprintf(buf,"end zhendong=%d\r\n",deta_shake);
	uart2_send(buf,strlen(buf));

	if(deta_shake>10)
		g_test_table[TEST_GSEN].result=1;
	else
		g_test_table[TEST_GSEN].result=0;
}
void test_adc(void)
{
	float ave_vol;

	Logln(D_INFO, "Vol=%d,adc_param=%f", get_bat_vol(),adc_param);

	if(g_flash.adc_vol > 0)
	{
		ave_vol = (get_bat_vol()*4095)/(adc_param*100);
		g_flash.adc_param = g_flash.adc_vol*4095/ave_vol;
		adc_param = g_flash.adc_param;
		Logln(D_INFO, "Vol=%d, ave_vol=%f, param=%f, cali vol=%d", g_flash.adc_vol, ave_vol, adc_param,get_bat_vol());	
	}
}
void test_adc_off(void)
{
	uint8_t buf[50]={0};
	uint16_t adc = get_bat_vol();

	sprintf(buf, "adc=%d\r\n",adc);
	uart2_send(buf,strlen(buf));
	if(adc>3000 && adc<9000)	//30v-90v
		g_test_table[TEST_ADC].result=1;
	else
		g_test_table[TEST_ADC].result=0;
}

void test_key(void)
{	
	if(g_key_state && g_key_state!=read_key_det)
		g_test_table[TEST_KEY].result=1;
	else
		g_test_table[TEST_KEY].result=0;
}

void test_hall(void)
{
	g_hall = mileage_count;
}

void test_off_hall(void)
{
	uint8_t buf[50]={0};
	uint32_t deta_hall = mileage_count-g_hall;

	sprintf(buf, "test hall =%d\r\n", deta_hall);
	uart2_send(buf,strlen(buf));

	if(deta_hall >140 && deta_hall<180)
		g_test_table[TEST_HALL].result=1;
	else
		g_test_table[TEST_HALL].result=0;
}

void test_off_lundong(void)
{
	uint8_t buf[50]={0};

	uint32_t deta_lundong = rotate_count-g_lundong;
	sprintf(buf, "test lundong=%d\r\n", deta_lundong);

	uart2_send(buf, strlen(buf));
	if(deta_lundong>3)
		g_test_table[TEST_LUNDONG].result=1;
	else
		g_test_table[TEST_LUNDONG].result=0;
}

void test_bt(void)
{
	bt_send("F2", 2);
}
void test_bt_off(void)
{
	uint8_t buf[128]={0};

	sprintf(buf,"name=%s,mac=%c%c:%c%c:%c%c:%c%c:%c%c:%c%c\r\n",dev_info.name,dev_info.addr[0],dev_info.addr[1],dev_info.addr[2],dev_info.addr[3],
		dev_info.addr[4],dev_info.addr[5],dev_info.addr[6],dev_info.addr[7],dev_info.addr[8],dev_info.addr[9],dev_info.addr[10],dev_info.addr[11]);

	if(strlen(dev_info.name))
		g_test_table[TEST_BT].result=1;

	uart2_send(buf,strlen(buf));
}
void test_gps(void)
{
	Send_AT_Command_Timeout(AT_QGPSLOC, 1);
}
void test_again_gps(void)
{
	uint8_t buf[128]={0};
	
	sprintf(buf, "sat_uesd=%d,sat_view=%d\r\n",gps_info.sat_uesd,gps_info.sat_view);

	uart2_send(buf,strlen(buf));

	if(g_test_table[TEST_GPS].result ==0)
	{
		if(gps_info.sat_view>2)
		{
			g_test_table[TEST_GPS].result=1;
		}
		else
		{
			g_test_table[TEST_GPS].result=0;
		}
	}
}
void test_gsm(void)
{
	bool ret;
	
	ret = Send_AT_Command_Timeout(AT_CREG, 10);
	Send_AT_Command_Timeout(AT_CSQ,1);

	if(convert_csq(dev_info.csq)>=40 && ret)
		g_test_table[TEST_GSM].result=1;
	else
		g_test_table[TEST_GSM].result=0;
}



zt_factory_test_struct g_test_table[]=
{
	{TEST_INIT, test_init, NULL, "Init", 0},
	{TEST_CPIN, test_cpin, NULL,"SIM CARD",0},		
	{TEST_HALL, test_hall, test_off_hall,"licheng",0},		
	{TEST_DIANMEN, test_dianmen, test_off_dianmen,"dianmen",0},
	{TEST_BATLOCK, test_bat_led, test_off_bat_led,"dianchi led",0},
	{TEST_DIANJILOCK, test_dianji_led, test_off_dianji_led,"dianji led",0},
	{TEST_LUNDONG, NULL, test_off_lundong,"lundong",0},	
	{TEST_VOICE, test_voice, NULL,"laba",0},	
	{TEST_A, test_a_led, test_off_a_led,"A led",0},
	{TEST_B, test_b_led, test_off_b_led,"B led",0},
	{TEST_GSEN, NULL, test_off_gsen,"zhengdong",0},
	{TEST_ADC, test_adc, test_adc_off,"dianyuan",0},
	{TEST_KEY, test_key, NULL,"KEY",0},
	{TEST_BT, test_bt, test_bt_off,"lanya",0},
	{TEST_GSM,test_gsm, NULL,"GSM",0},
	{TEST_GPS, test_gps, test_again_gps,"GPS",0},
	
	{TEST_MAX, NULL, NULL,"",0}
};

void zt_test_result(void)
{
	uint8_t i;
	uint8_t buf[50]={0};
	uint8_t tmp[5];
	uint8_t ret=1;

	uart2_send("-----------------TEST OVER-----------------\r\n",strlen("-----------------TEST OVER-----------------\r\n"));

	sprintf(buf,"BT name:%s\r\nIMEI:%s\r\nVER:%s\r\n",dev_info.name,g_flash.imei,DEV_VER);

	uart2_send(buf,strlen(buf));

	memset(buf,0,sizeof(buf));
	strcpy(buf,"result:");
	for(i=0; i<TEST_MAX; i++)
	{
		if(g_test_table[i].result==0)
		{
			sprintf(tmp,"%d,",i);
			strcat(buf,tmp);
			ret = 0;
		}
	}
	if(ret==1)
		strcat(buf,"PASS\r\n");
	else
		strcat(buf,"(NG)\r\n");
	
	uart2_send(buf,strlen(buf));
	uart2_send("-------------------------------------------\r\n",strlen("-------------------------------------------\r\n"));

}

void test_process(void)
{
	uint8_t buf[50]={0};

	if(g_test_index>0 && g_test_table[g_test_index-1].ts_off)
		g_test_table[g_test_index-1].ts_off();

	sprintf(buf,"%d.%s\r\n",g_test_index,g_test_table[g_test_index].name);
	uart2_send(buf,strlen(buf));
	
	if(g_test_table[g_test_index].ts_on)
		g_test_table[g_test_index].ts_on();

	g_test_index++;

	if(g_test_index >= TEST_MAX)
	{
		if(g_test_table[g_test_index-1].ts_off)
			g_test_table[g_test_index-1].ts_off();
	
		zt_test_result();
		g_flash.lundong = 0;
		g_flash.hall = 0;
		g_flash.mode = 0;
		write_flash(CONFIG_ADDR, (uint8_t*)&g_flash,(uint16_t)sizeof(flash_struct));
		uart2_send("reset_system\r\n",strlen("reset_system\r\n"));
		reset_system();
	}
	else
	{
		HAL_Delay(1000);
	}
}

