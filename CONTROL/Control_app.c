#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "stm32f0xx_hal.h"
#include "protocol.h"
#include "control_app.h"
#include "gpio.h"
#include "control_interface.h"
#include "exti.h"
#include "voice.h"
#include "adc.h"
#include "IoT_Hub.h"
#include "queen.h"
//#include "flash.h"

#define DOMAIN 	"devzuche.liabar.com"
#define PORT 9000

flash_struct g_flash;

uint32_t last_mileage;
extern uint32_t diff_rotate,diff_mileage,diff_shake;
extern battery_info_struct curr_bat;
extern gps_info_struct gps_info;
extern uint8_t flag_alarm;
extern uint8_t flag_delay3s;

/*PB9拉高，500ms之后拉低*/
void open_dianchi_lock(void)
{
	flag_battery_lock = 1;
}

uint8_t convert_csq(uint8_t csq)
{
	if(csq==99)
		return 0;
	else if(csq>=0 && csq<=30)
		return csq*100/30;
	else 
		return 100;
}

bool get_bat_connect_status(void)
{
	if(get_bat_vol()>1200)	//大于12V
		return true;
	else
		return false;
}

uint8_t get_electric_gate_status(void)
{
	return g_flash.acc;
}

bool zt_smart_check_hall_is_run()
{
	if(diff_mileage > 2)
		return true;
	else
		return false;
}

bool check_zhendong(void)
{
	if(diff_shake>100)
		return true;
	else
		return false;
}

bool check_sharp_zhendong(void)
{
	if(diff_shake > g_flash.zd_sen)
		return true;
	else
		return false;
}
bool zt_smart_check_lundong_is_run()
{
	if(diff_rotate > 2)
		return true;
	else
		return false;
}

bool lock_bike(void)
{
	bool result = false;
	
	if(!(g_flash.acc&KEY_OPEN)&&(g_flash.acc&(BT_OPEN|GPRS_OPEN)))
	{
		if((!zt_smart_check_hall_is_run()&&g_flash.motor==0)||(!zt_smart_check_lundong_is_run()&&g_flash.motor==1))
		{
			close_electric_door();
			//tangze_lock_bike();
			flag_tangze_lock = 1;
            		g_flash.acc = 0;
			write_flash(CONFIG_ADDR, (uint8_t*)&g_flash,(uint16_t)sizeof(flash_struct));
			result = true;
		}
	}

	return result;
}

void gprs_unlock(void)
{
	flag_tangze_unlock = 1;
    	g_flash.acc  |= GPRS_OPEN;
	write_flash(CONFIG_ADDR, (uint8_t*)&g_flash,(uint16_t)sizeof(flash_struct));
    	open_electric_door();
}

void parse_network_cmd(ebike_cmd_struct *cmd)
{
	if(cmd->addr == 0x1C)
	{
        	Logln(D_INFO, "NETWORK cmd->type = %d",cmd->type);
		switch(cmd->type)
		{
			case SEARCH_CMD:
				voice_play(VOICE_SEARCH,g_flash.search_times);
				break;
			case LOCK_CMD:
                		Logln(D_INFO, "cmd->para[0] = %d,g_flash.acc=%d",cmd->para[0],g_flash.acc);
				if(cmd->para[0]==1)	//锁车
				{
					if(lock_bike())
					{
						voice_play(VOICE_LOCK,1);
					}
				}
				else if(cmd->para[0]==0)	//解锁
				{
					if(!g_flash.acc)
					{
						gprs_unlock();
						voice_play(VOICE_UNLOCK,1);
					}
				}
				break;
			case MUTE_CMD:
				if(cmd->para[0]==1)
				{
					g_flash.zd_alarm = 1;
				}
				else if(cmd->para[0]==0)
				{
					g_flash.zd_alarm = 0;
				}
				else	//震动灵敏度
				{
					g_flash.zd_sen = cmd->para[0];
				}
				write_flash(CONFIG_ADDR, (uint8_t*)&g_flash,(uint16_t)sizeof(flash_struct));
				break;
			case DIANCHI_CMD:
				if(cmd->para[0]==1)
				{
          				open_dianchi_lock();
				}
				break;
			case RESET_CMD:
				reset_system();
				break;
			case BT_RESET_CMD:
				break;
			case GIVE_BACK_CMD:
				if(cmd->para[0]==1)	//还车指令
				{
					RxMsgTypeDef msgType;
					
					if(lock_bike())
					{
						voice_play(VOICE_LOCK,1);
					}
					msgType.Data[0] = AT_UP_GIVEBACK;
					PushElement(&at_send_Queue, msgType, 1);
				}
				else if(cmd->para[0]==2)	//服务器判断还车成功之后下发指令
				{
					//关闭GPS
				}
				break;
			case BORROW_CMD:
				if(cmd->para[0]==1)	//租车成功
				{
					//开启GPS
				}
				break;
			case ALARM_CMD:
				g_flash.ld_alarm = cmd->para[0];
				write_flash(CONFIG_ADDR, (uint8_t*)&g_flash,(uint16_t)sizeof(flash_struct));
				break;
			case TIAOSU_CMD:
				if(cmd->para[0]==0)
				{
					zt_controller_send(ADDR_CONTROL, CMD_CONTROL,1,HIGH_SPEED);
					controller.require.tiaosu = HIGH_SPEED;
				}
				else
				{
					zt_controller_send(ADDR_CONTROL, CMD_CONTROL,1,LOW_SPEED);
					controller.require.tiaosu = LOW_SPEED;
				}
				break;
			case QIANYA_CMD:
				if(cmd->para[0]==0)
				{
					zt_controller_send(ADDR_CONTROL,CMD_CONTROL,2,HIGH);
					controller.require.qianya = HIGH;
				}
				else
				{
					zt_controller_send(ADDR_CONTROL,CMD_CONTROL,2,LOW);
					controller.require.qianya = LOW;
				}
				break;
			case ZHULI_CMD:
				if(cmd->para[0]==0)
				{
					zt_controller_send(ADDR_CONTROL,CMD_CONTROL,3,DIANDONG);
					controller.require.zhuli = DIANDONG;
				}
				else if(cmd->para[0]==1)
				{
					zt_controller_send(ADDR_CONTROL,CMD_CONTROL,3,ZHULI);
					controller.require.zhuli = ZHULI;
				}
				else if(cmd->para[0]==2)
				{
					zt_controller_send(ADDR_CONTROL,CMD_CONTROL,3,ZHULI2);
					controller.require.zhuli = ZHULI2;
				}
				else if(cmd->para[0]==3)
				{
					zt_controller_send(ADDR_CONTROL,CMD_CONTROL,3,RENLI); 
					controller.require.zhuli = RENLI;
				}
				else
				{
					zt_controller_send(ADDR_CONTROL,CMD_CONTROL,3,HUNHE); 
					controller.require.zhuli = HUNHE;
				}
				break;
			case XIUFU_CMD:
				if(cmd->para[0]==1)
				{
					zt_controller_send(ADDR_CONTROL,CMD_CONTROL,4,XF_OK);
					controller.require.xf = XF_OK;
				}
				else if(cmd->para[0]==0)
				{
					zt_controller_send(ADDR_CONTROL,CMD_CONTROL,4,XF_INVALID);
					controller.require.xf = XF_INVALID;
				}
				break;
			case DIANYUAN_CMD:
				if(cmd->para[0]==0)//36V
				{
					zt_controller_send(ADDR_CONTROL,CMD_CONTROL,5,VOT36V);
					controller.require.dy = VOT36V;
				}
				else if(cmd->para[0]==1)//48V
				{
					zt_controller_send(ADDR_CONTROL,CMD_CONTROL,5, VOT48V);
					controller.require.dy = VOT48V;
				}
				break;
			case DIANJI_CMD:
				if(cmd->para[0]==0)	//普通电机
				{
					g_flash.motor = 0;
					write_flash(CONFIG_ADDR, (uint8_t*)&g_flash,(uint16_t)sizeof(flash_struct));
					Logln(D_INFO, "General Motor");
				}
				else if(cmd->para[0]==1)//高速电机
				{
					g_flash.motor = 1;
					write_flash(CONFIG_ADDR, (uint8_t*)&g_flash,(uint16_t)sizeof(flash_struct));
					Logln(D_INFO, "High Speed Motor");
				}
				break;
			case SEARCH_TIMES_CMD:
				{
					if(cmd->para[0]>0)
					{
						g_flash.search_times = cmd->para[0];
						write_flash(CONFIG_ADDR, (uint8_t*)&g_flash,(uint16_t)sizeof(flash_struct));
					}
					break;
				}
			case GB_CMD:
				{
					g_flash.gb_alarm = cmd->para[0];
					g_flash.gb_speed = cmd->para[1];
					write_flash(CONFIG_ADDR, (uint8_t*)&g_flash,(uint16_t)sizeof(flash_struct));
					Logln(D_INFO,"gb_alarm=%d,gb_speed=%d",g_flash.gb_alarm,g_flash.gb_speed);
					break;
				}
			default:
				break;
		}
	}
}

void get_ebike_data(ebike_pkg_struct* ebike_pkg)
{
	ebike_struct ebike;
	
	ebike.fault = controller.actual.fault;
	if(controller.actual.tiaosu==HIGH_SPEED)
		ebike.status.tiaosu = 0;
	else if(controller.actual.tiaosu==LOW_SPEED)
		ebike.status.tiaosu = 1;
	
	if(controller.actual.qianya==HIGH)
		ebike.status.qianya = 0;
	else if(controller.actual.qianya==LOW)
		ebike.status.qianya = 1;

	if(controller.actual.zhuli==DIANDONG)
		ebike.status.zhuli = 0;
	else if(controller.actual.zhuli==ZHULI)
		ebike.status.zhuli = 1;
	else if(controller.actual.zhuli==ZHULI2)
		ebike.status.zhuli = 2;
	else if(controller.actual.zhuli==RENLI)
		ebike.status.zhuli = 3;
	else if(controller.actual.zhuli==HUNHE)
		ebike.status.zhuli = 4;
	
	if(controller.actual.dy==VOT36V)
		ebike.status.dy = 0;
	else if(controller.actual.dy==VOT48V)
		ebike.status.dy = 1;
	
	if(controller.actual.xf==XF_INVALID)
		ebike.status.xf = 0;
	else if(controller.actual.xf==XF_OK)
		ebike.status.xf = 1;	
	
	if(g_flash.acc)
		ebike.status.lock = 0;
	else
		ebike.status.lock = 1;

	if(g_flash.ld_alarm== 1)
		ebike.status.alarm= 1;	
	else
		ebike.status.alarm = 0;

	if(g_flash.zd_alarm == 1)
		ebike.status.zd_alarm = 1;
	else
		ebike.status.zd_alarm = 0;
	
	if(g_flash.motor == 1)
		ebike.status.motor = 1;
	else
		ebike.status.motor = 0;
	

	if(g_flash.motor==1)
		ebike.hall = rotate_count/8;
	else
		ebike.hall = mileage_count/8;

	ebike.bat.temp = curr_bat.temp;
	if(curr_bat.voltage>0 && curr_bat.voltage!=0xffff)	
		ebike.bat.voltage = curr_bat.voltage/10;
	else
		ebike.bat.voltage = get_bat_vol();
		
	ebike.bat.current= curr_bat.current;
	ebike.bat.residual_cap= curr_bat.residual_cap;
	ebike.bat.total_cap= curr_bat.total_cap;
	ebike.bat.cycle_count= curr_bat.cycle_count;
	ebike.bat.interval= curr_bat.interval;
	ebike.bat.max_interval= curr_bat.max_interval;

	ebike.sig.gsm_signal = convert_csq(dev_info.csq);
	ebike.sig.gps_viewd = gps_info.sat_view;
	ebike.sig.gps_used = gps_info.sat_uesd;
    
    	memcpy(ebike_pkg->value,&ebike,sizeof(ebike_struct));
	ebike_pkg->addr = 0x1d;
	ebike_pkg->value_len = sizeof(ebike_struct);

	g_flash.hall = mileage_count;
	g_flash.lundong = rotate_count;

	Logln(D_INFO, "ebike.bat.voltage=%d,mile=%d,last mile=%d,ACC=%d,lock=%d,gsm_signal=%d,gps_used=%d",ebike.bat.voltage,mileage_count,last_mileage,g_flash.acc,ebike.status.lock,ebike.sig.gsm_signal,ebike.sig.gps_used);

	if(mileage_count-last_mileage>100)
	{
		last_mileage = mileage_count; 
		write_flash(CONFIG_ADDR, (uint8_t*)&g_flash,(uint16_t)sizeof(flash_struct));
	}
	
}

double getPerimeter(int n)
{
	switch(n){
		   case 14:
			   return 1.12;
		   case 10:
			   return 1.27;
		   case 12:
			   return 1.44;
		   case 16:
			   return 1.27;
		   case 18:
			   return 1.43;
		   case 20:
			   return 1.59;
		   case 22:
			   return 1.75;
		  default:
			   return 1.91;
		}
}
/*算出时速在1秒的霍尔数量*/
uint32_t get_hall_for_speed(uint32_t speed)
{
	return  (uint32_t)(speed*g_flash.cigang)/(getPerimeter(g_flash.lunjing)*2*3600);
}
bool zt_smart_check_gb_speed(uint32_t hall_1sec)
{
	if(hall_1sec > get_hall_for_speed(g_flash.gb_speed*1000))
		return true;
	else
		return false;
}

void gb_speed_process(void)
{
	if(zt_smart_check_gb_speed(diff_mileage) && g_flash.gb_alarm && flag_delay3s)
	{
		flag_delay3s = 0;
		voice_play(VOICE_SEARCH,1);
	}
}

void parse_imsi_package(uint8_t* data, uint8_t len)
{
	if(g_flash.lunjing != data[0] ||g_flash.cigang != data[1])
	{
		g_flash.lunjing = data[0];
		g_flash.cigang = data[1];
		write_flash(CONFIG_ADDR, (uint8_t*)&g_flash,(uint16_t)sizeof(flash_struct));
	}
	Logln(D_INFO,"lunjin=%d,cigang=%d",g_flash.lunjing,g_flash.cigang);
}

void motorlock_process(void)
{
	if (diff_rotate > 4 && f_motorlock == 0 && g_flash.acc == 0) 
	{
		f_motorlock = 1;
		if(g_flash.ld_alarm)
		{
			voice_play(VOICE_ALARM,1);
		}
	}

	if (f_motorlock) 
	{
		motor_lock_bike();
	}
}
void shake_process(void)
{
        if (check_sharp_zhendong() && g_flash.acc == 0 && flag_alarm==0)
        {
        	if(g_flash.zd_alarm)
        	{
        		Logln(D_INFO,"shake--%d",diff_shake);
              		voice_play(VOICE_ALARM,1);
			flag_alarm = 1;
			flag_delay8s = 1;
        	}
        }
}
void key_check_process(void)
{
	uint8_t value = read_key_det;
	uint8_t value2 = read_acc_det;
	static uint16_t key_detect_num = 0;

	if(!value &&(!(g_flash.acc&BT_OPEN) && !(g_flash.acc&KEY_OPEN) && !(g_flash.acc&GPRS_OPEN)))
	{
		Logln(D_INFO,"key_detect_num=%d",key_detect_num);
		key_detect_num++;
		if(key_detect_num>10)
		{
			key_detect_num = 0;
			g_flash.acc |= KEY_OPEN;
			voice_play(VOICE_UNLOCK,1);
		}
	}
	else if(value && g_flash.acc && value2)
	{
		key_detect_num++;
		if(key_detect_num>10)
		{
			key_detect_num = 0;
			g_flash.acc = 0;
			voice_play(VOICE_LOCK,1);
		}
	}
	else
	{
		key_detect_num = 0;
	}
}

void init_flash(void)
{
	read_flash(CONFIG_ADDR,(uint8_t*)&g_flash,(uint16_t)sizeof(flash_struct));
	HAL_Delay(1);
	Logln(D_INFO,"flag=%d,mode=%d,imei=%s,acc=%d,hall=%d,ld=%d,motot=%d,ld_a=%d,zd_a=%d,zd_se=%d,times=%d,gb_a=%d,gb_s=%d,lj=%d,cg=%d,%s:%d,vol=%d,size=%d",g_flash.flag,g_flash.mode,g_flash.imei,g_flash.acc,g_flash.hall,g_flash.lundong,
		g_flash.motor,g_flash.ld_alarm,g_flash.zd_alarm,g_flash.zd_sen,g_flash.search_times,g_flash.gb_alarm,g_flash.gb_speed,g_flash.lunjing,g_flash.cigang,g_flash.net.domain, g_flash.net.port, g_flash.adc_vol,sizeof(flash_struct));
		
	if(g_flash.flag !=1)
	{
		memset(&g_flash, 0, sizeof(flash_struct));
		write_flash(CONFIG_ADDR, (uint8_t*)&g_flash,(uint16_t)sizeof(flash_struct));
		HAL_Delay(1);

		g_flash.flag = 1;
		g_flash.acc = 0;
		g_flash.hall = 0;
		g_flash.lundong = 0;
		g_flash.motor = 0;
		g_flash.ld_alarm = 0;
		g_flash.zd_alarm = 0;
		g_flash.zd_sen = 80;
		g_flash.search_times = 3;
		g_flash.gb_alarm = 0;
		g_flash.gb_speed = 15;
		g_flash.cigang = 46;
		g_flash.lunjing = 14;
		g_flash.adc_param = 3.3*34;
		memset(g_flash.imei,0,sizeof(g_flash.imei));
		strcpy(g_flash.net.domain, DOMAIN);
		g_flash.net.port = PORT;
		g_flash.mode = 0;
		write_flash(CONFIG_ADDR, (uint8_t*)&g_flash,(uint16_t)sizeof(flash_struct));
		HAL_Delay(1);
	}

	if(g_flash.acc&(BT_OPEN|GPRS_OPEN|KEY_OPEN))
	{
		open_electric_door();
	}

	if(g_flash.adc_param != 0)
	{
		adc_param = g_flash.adc_param;
	}
	else
		adc_param = 3.3*34;

	Logln(D_INFO,"%f,%f", g_flash.adc_param,adc_param);
	
	mileage_count = g_flash.hall;
	rotate_count = g_flash.lundong;
}

