#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "usart.h"
#include "gpio.h"
#include "IoT_Hub.h"
#include "protocol.h"
#include "queen.h"
#include "Control_app.h"

extern UART_HandleTypeDef huart1;

char module_recv_buffer[MODULE_BUFFER_SIZE] = {0};
unsigned short module_recv_write_index = 0;
unsigned short recv_read_start_index = 0;
unsigned short recv_read_end_index = 0;

char send_buffer[SEND_BUF_SIZE];
unsigned short send_len;

char gnss_buf[512];
char gprs_buf[512];
unsigned short gprs_size=0;
char another_buf[512];

gps_info_struct gps_info;
dev_struct dev_info;
uint8_t login_protect_timeout=0;
extern uint8_t flag_delay1s;
extern work_state net_work_state;
uint16_t connect_times;
extern uint8_t first_pwr;
cell_location_struct cell_loc;


RxMsgTypeDefExt at_curr_node={0};

void parse_imei_cmd(char* buf, int len);
void parse_csq_cmd(char* buf, int len);
void parse_imsi_cmd(char* buf, int len);
void parse_gnss_cmd(char* buf, int len);
void parse_location_cmd(char* buf, int len);
void parse_cell_location_cmd(char* buf, int len);

AT_STRUCT at_pack[]={
	{AT_ATI,"ATI","OK",300,NULL},
	{AT_ATE0,"ATE0","OK",300,NULL},
	{AT_IPR,"AT+IPR=115200","OK",300,NULL},
	{AT_W,"AT&W","OK",300,NULL},
	{AT_ATS0,"ATS0=1",300,NULL},
	{AT_CPIN,"AT+CPIN?","OK",300,NULL},
	{AT_CREG,"AT+CREG?","0,1",500,NULL},
	{AT_CSQ,"AT+CSQ","OK",300,parse_csq_cmd},
	{AT_GSN,"AT+GSN","OK",300,parse_imei_cmd},
	{AT_CIMI,"AT+CIMI","OK",300,parse_imsi_cmd},
	{AT_QIMODE,"AT+QIMODE=0","OK",300,NULL},
	{AT_QICSGP,"AT+QICSGP=1,1,\"CMNET\",\"\",\"\"","OK",300,NULL},
	{AT_QIREGAPP,"AT+QIREGAPP","OK",300,NULL},
	{AT_QIACT,"AT+QIACT=1","OK",3000,NULL},
	{AT_COPS,"AT+COPS?","OK",300,NULL},
	{AT_QCELLLOC,"AT+QCELLLOC=1","OK",1000,parse_cell_location_cmd},
	{AT_QIMUX,"AT+QIMUX=0","OK",300,NULL},
	{AT_QIDNSIP,"AT+QIDNSIP=1","OK",300,NULL},
	{AT_QIOPEN,"","OK",1000,NULL},
	{AT_QISEND,"",">",2000,NULL},
	{AT_QRECV,"+QIURC: \"recv\"","OK",300,NULL},
	{AT_QICLOSE,"AT+QICLOSE=0","OK",300,NULL},
	{AT_QPING,"AT+QPING=1,\"zcwebx.liabar.cn\",4,1","OK",300,NULL},
	
	{AT_QGPS_ON,"AT+QGPS=1","OK",1000,NULL},
	{AT_QGPS_OFF,"AT+QGPSEND","OK",300,NULL},
	{AT_QGPS_RMC,"AT+QGNSSRD=\"NMEA/RMC\"","OK",500,NULL},
	{AT_QGPS_GGA,"AT+QGNSSRD=\"NMEA/GGA\"","OK",500,NULL},	
	{AT_QGPS_GSV,"AT+QGNSSRD=\"NMEA/GSV\"","OK",500,NULL},	
	{AT_QGPSLOC,"AT+QGPSLOC=1","+QGPSLOC:",500,NULL},	
	{AT_QIFGCNT1,"AT+QIFGCNT=1","OK",300,NULL},
	{AT_QIFGCNT2,"AT+QIFGCNT=2","OK",300,NULL},	
	{AT_QGNSSTS,"AT+QGNSSTS?","OK",500,NULL},	
	{AT_QGNSSEPO,"AT+QGNSSEPO=1","OK",500,NULL},	
	{AT_QGREFLOC,"","OK",500,NULL},	
	{AT_QGEPOAID,"AT+QGEPOAID","OK",500,NULL},	
	{AT_QGEPOF,"", "OK", 500,NULL},
	
	{ATA,"ATA","OK",300,NULL},
	{AT_MAX,"","",0,NULL}
};

void MODULE_RST(void)
{
	MODULE_RST_H;
	HAL_Delay(200);
	MODULE_RST_L;
	HAL_Delay(100);

	Logln(D_INFO,"RESET");
}
void MODULE_PWRON(void)
{
	MODULE_PWRKEY_H;
	HAL_Delay(600);
	MODULE_PWRKEY_L;
	HAL_Delay(100);

	Logln(D_INFO,"POWER ON");
}
void MODULE_PWROFF(void)
{
	MODULE_PWRKEY_H;
	HAL_Delay(650);
	MODULE_PWRKEY_L;
	HAL_Delay(100);
}
void conventdata0(char* data, int len)
{
	int i;

	for(i=0; i<len; i++)
	{
		if(*(data+i)==0)
		{
			*(data+i)='#';
		}
	}
}

void conventdatahash(char* data, int len)
{
	int i;

	for(i=0; i<len; i++)
	{
		if(*(data+i)=='#')
		{
			*(data+i)=0;
		}
	}
}

void pure_uart1_buf(void)
{
	Logln(D_INFO, "pure_uart1_buf");		
	memset(module_recv_buffer, 0, MODULE_BUFFER_SIZE);
	module_recv_write_index = 0;
}

int GetComma(int num,char *str)
{
	int i,j=0;
	int len = strlen(str);
	for(i=0;i<len;i++)
	{
	    if(str[i]==',')
	    {
	         j++;
	    }

	    if(j==num)
	        return i+1;
	}
	return 0;
}
double get_double_number(char *s)
{
	char buf[16];
	int i;
	double rev;
	i=GetComma(1,s);
	strncpy(buf,s,i);
	buf[i-1]=0;
	rev=(double)atof(buf);

	return rev;
}
double get_locate(double temp)
{
	int m;
	double  n;
	m=(int)temp/100;
	n=(temp-m*100)/60;
	n=n+m;
	return n;
}
int Get_Char_Pos(char *str,char find,int num)
{
	int i,j=0;
	int len = strlen(str);
	for(i=0;i<len;i++)
	{
	    if(str[i]==find)
	    {
	         j++;
	    }

	    if(j==num)
	        return i+1;
	}
	return 0;
}
double get_used_sat(char *s)
{
	char buf[8]={0};
	double rev;
	
	strcpy(buf, s);
	rev=(double)atof(buf);

	return rev;
}
// example 21*47
char GetLastSnr(char* buf)
{
	char tmp[6]={0},*sr;
	sr = strstr(buf,"*");
	if(sr)
	{
		memcpy(tmp,buf,sr-buf);
	}

	return atoi(tmp);
}
void parse_location_cmd(char* buf, int len)
{
	char* tmp;
	if(tmp=strstr(buf, "+QGREFLOC:"))
	{
		Logln(D_INFO,"%s",tmp);
	}
}

void parse_cell_location_cmd(char* buf, int len)
{
	char* tmp,*tmp1,*tmp2;
	if(tmp=strstr(buf, "+QCELLLOC: "))
	{
		tmp+=strlen("+QCELLLOC: ");
		memset(&cell_loc, 0, sizeof(cell_location_struct));
		tmp1 = strstr(tmp, ",");
		strncpy(cell_loc.lon, tmp, tmp1-tmp);
		tmp2 = strstr(tmp1, "\r\n");
		strncpy(cell_loc.lat, tmp1+1, tmp2-(tmp1+1));
	}
}
void parse_gnss_cmd(char* buf, int len)
{
	char* pnmea = buf;
	unsigned long tmp;
	char *tmp1,*tmp2;

	if((tmp1=strstr(pnmea+1,"GPRMC"))||(tmp2=strstr(pnmea+1,"GNRMC")))
	{
		if(tmp1)
			pnmea = tmp1;
		else if(tmp2)
			pnmea = tmp2;
		//$GPRMC,085118.00,A,2235.87223,N,11359.99731,E,0.349,,270416,,,A*7D
		tmp = (unsigned long)get_double_number(&pnmea[GetComma(1,pnmea)]);
		gps_info.dt.nHour = tmp/10000;
		gps_info.dt.nMin = (tmp%10000)/100;
		gps_info.dt.nSec = tmp%100;
		tmp = (unsigned long)get_double_number(&pnmea[GetComma(9,pnmea)]);
		gps_info.dt.nDay = tmp/10000;
		gps_info.dt.nMonth  = (tmp%10000)/100;
		gps_info.dt.nYear   = tmp%100+2000;

		gps_info.state = pnmea[GetComma(2,pnmea)];
		gps_info.latitude = get_locate(get_double_number(&pnmea[GetComma(3,pnmea)]));
		gps_info.NS = pnmea[GetComma(4,pnmea)];
		gps_info.longitude = get_locate(get_double_number(&pnmea[GetComma(5,pnmea)]));
		gps_info.EW = pnmea[GetComma(6,pnmea)];
		gps_info.speed = get_double_number(&pnmea[GetComma(7,pnmea)]);
		gps_info.angle = get_double_number(&pnmea[GetComma(8,pnmea)]);
		gps_info.magnetic_value = get_double_number(&pnmea[GetComma(10,pnmea)]);
		gps_info.magnetic_ind = pnmea[GetComma(11,pnmea)];
		gps_info.mode = pnmea[GetComma(12,pnmea)];

		Logln(D_INFO,"lat=%f,lon=%f",gps_info.latitude,gps_info.longitude);
	}
	else if((tmp1=strstr(pnmea+1,"GPGGA"))||(tmp2=strstr(pnmea+1,"GNGGA")))
	{
		//$GPGGA,085118.00,2235.87223,N,11359.99731,E,1,03,4.72,138.3,M,-2.6,M,,*4B
		if(tmp1)
			pnmea = tmp1;
		else if(tmp2)
			pnmea = tmp2;
	    	gps_info.sat_uesd = (char)get_double_number(&pnmea[GetComma(7,pnmea)]);
		gps_info.hdop = get_double_number(&pnmea[GetComma(8,pnmea)]);;
		gps_info.altitude = get_double_number(&pnmea[GetComma(9,pnmea)]);
	}
	else if(tmp1 = strstr(pnmea+1,"GPGSV"))
	{
		char n,isat, isi, nsat;
		//$GPGSV,5,1,17,01,45,168,19,03,00,177,,07,53,321,08,08,53,014,*7C

		if(tmp1)
			pnmea = tmp1;
		gps_info.sat_view = (char)get_double_number(&pnmea[GetComma(3,pnmea)]);
		n = (char)get_double_number(&pnmea[GetComma(2,pnmea)]);
		nsat = (n-1)*4;
		nsat = (nsat+4>gps_info.sat_view)?gps_info.sat_view-nsat:4;
		
		for(isat = 0; isat < nsat; ++isat)
		{
			isi = (n - 1) * 4 + isat; 
			gps_info.gsv.sat_num[isi] = (char)get_double_number(&pnmea[GetComma(4*(isat+1),pnmea)]);
			
			if(GetComma(3+4*(isat+1)+1,pnmea)==0)
				gps_info.gsv.sat_db[isi] = GetLastSnr(&pnmea[GetComma(3+4*(isat+1),pnmea)]);
			else
				gps_info.gsv.sat_db[isi] = (char)get_double_number(&pnmea[GetComma(3+4*(isat+1),pnmea)]); 	
		}
		Logln(D_INFO,"gsv=%d",nsat);

	}
	else if(strstr(pnmea+1,"GPGLL"))
	{
		//$GPGLL,4250.5589,S,14718.5084,E,092204.999,A*2D
	}
	else if((tmp1=strstr(pnmea+1,"GPGSA"))||(tmp2=strstr(pnmea+1,"GNGSA")))
	{
		//$GPGSA,A,3,01,20,19,13,,,,,,,,,40.4,24.4,32.2*0A
		if(tmp1)
			pnmea = tmp1;
		else if(tmp2)
			pnmea = tmp2;
		
		gps_info.type = pnmea[GetComma(2,pnmea)]-'0';
		if(gps_info.type==2)
		{
			gps_info.state = 'V';
		}
	}
	else if(strstr(pnmea+1,"GPVTG"))
	{
		//$GPVTG,89.68,T,,M,0.00,N,0.0,K*5F
	}
}
void parse_gps_data(char* buf, int len)
{
	uint32_t tmp;	  
	char pnmea[128]={0};
	
//	strcpy(pnmea,"QGPSLOC: 102551.0,2237.315292,N,11402.528687,E,1.2,113.0,2,0.00,0.0,0.0,300318,09\r\n");
	strcpy(pnmea,buf);
	tmp = (uint32_t)get_double_number(&pnmea[Get_Char_Pos(pnmea,':',1)]);
	gps_info.dt.nHour = tmp/10000;
	gps_info.dt.nMin = (tmp%10000)/100;
	gps_info.dt.nSec = tmp%100;
	
	gps_info.latitude = get_locate(get_double_number(&pnmea[GetComma(1,pnmea)]));
	gps_info.NS = pnmea[GetComma(2,pnmea)];
	gps_info.longitude = get_locate(get_double_number(&pnmea[GetComma(3,pnmea)]));
	gps_info.EW = pnmea[GetComma(4,pnmea)];
	gps_info.hdop = get_double_number(&pnmea[GetComma(5,pnmea)]);
	gps_info.altitude = get_double_number(&pnmea[GetComma(6,pnmea)]);
	gps_info.type = (uint8_t)get_double_number(&pnmea[GetComma(7,pnmea)]);
	gps_info.angle = get_double_number(&pnmea[GetComma(8,pnmea)]);
	gps_info.speed = get_double_number(&pnmea[GetComma(10,pnmea)]);
	tmp = (uint32_t)get_double_number(&pnmea[GetComma(11,pnmea)]);
	gps_info.dt.nYear = tmp%100;
	gps_info.dt.nMonth = (tmp%10000)/100;
	gps_info.dt.nDay = tmp/10000;
	gps_info.sat_uesd = (uint8_t)get_used_sat(&pnmea[GetComma(12,pnmea)]);

	if(gps_info.latitude !=0 && gps_info.longitude !=0)
	{
		gps_info.state = 'A';
		gps_led_flag = 1;
	}
	else
	{
		gps_info.state = 'V';
		gps_led_flag = 0;
	}
	
	Logln(D_INFO,"lat:%f,long:%f,used:%d",gps_info.latitude,gps_info.longitude,gps_info.sat_uesd);
}

int8_t GetATIndex(AT_CMD cmd)
{
	int8_t i=0;
	
	while(cmd != at_pack[i].cmd)
	{
		if(at_pack[i].cmd==AT_MAX)
		{
			i=-1;
			break;
		}
		i++;
	}
	return i;
}

uint8_t Send_AT_Command(AT_CMD cmd)
{
	int len;
	uint8_t end = 0x1a;	
	char buf[512]={0};
	
	int8_t i=GetATIndex(cmd), ret=0;

	if(i==-1)
	{
		Logln(D_INFO, "error cmd");
		return ret;
	}
	uart1_send(at_pack[i].cmd_txt, strlen(at_pack[i].cmd_txt));
	uart1_send("\r\n", strlen("\r\n"));
	
	HAL_Delay(at_pack[i].timeout);
	
	len = get_uart_data(buf, 512);
	Logln(D_INFO, "rcv %d,%s", len,buf);

	if(strstr(buf,at_pack[i].cmd_ret))
	{
		if(at_pack[i].fun)
		{
			at_pack[i].fun(buf,len);
		}
		ret = 1;
	}
	return ret;
}

bool Send_AT_Command_Timeout(AT_CMD cmd, uint8_t timeout)
{
	while(1)
	{
		if(Send_AT_Command(cmd))
		{
			return true;
		}
		else
		{
			timeout--;
			
			if(timeout==0)
				return false;
		}
	}
}

void Send_AT_Command_ext(AT_CMD cmd)
{
	uint16_t ret,timeout=0;
	int8_t i=GetATIndex(cmd);

	if(i==-1)
	{
		Logln(D_INFO, "error cmd");
		return;
	}

	uart1_send(at_pack[i].cmd_txt, strlen(at_pack[i].cmd_txt));
	uart1_send("\r\n", strlen("\r\n"));

	HAL_Delay(at_pack[i].timeout);
}

int get_connect_id(char *s,char find,int num)
{
	char buf[16]={0};
	int n1,n2;
	int rev;
	
	n1=Get_Char_Pos(s,find,num);
	n2=Get_Char_Pos(s,',',1);

	strncpy(buf,s+n2,n1-n2-1);
	rev=(int)atoi(buf);

	return rev;
}
int get_recv_len(char *s,char find,int num)
{
	char buf[16]={0};
	int n1,n2;
	int rev;
	
	n1=Get_Char_Pos(s,find,num);
	n2=Get_Char_Pos(s,',',2);

	strncpy(buf,s+n2,n1-n2-1);
	rev=(int)atoi(buf);

	return rev;
}

int get_uart_data_ext(char*buf, int count)
{
	uint16_t ulen = module_recv_write_index;

	if (ulen > 0)
	{
	//	Logln(D_INFO,"get_uart_data_ext rcv=%d,%d",ulen,count);
		if (count >= ulen)
		{
			memcpy(buf, module_recv_buffer, ulen);
			buf[ulen] = 0;
		//	Logln(D_INFO,"rcv=%d,%s",ulen,buf);
			return ulen;
		}
		else
		{
			char* pDst = module_recv_buffer;
			char* pSrc = module_recv_buffer + count;
			char* pDstEnd = pDst + (ulen - count);

			memcpy(buf, module_recv_buffer, count);
			buf[count] = 0;
			Logln(D_INFO,"2--%s",buf);

			for (; pDst < pDstEnd; ++pDst, ++pSrc)
			{
				*pDst = *pSrc;
			}
			
			module_recv_write_index = ulen - count;
			return count;
		}
	}
	else
	{
		return 0;
	}	
}

int get_uart_data(char*buf, int count)
{
	uint16_t ulen = module_recv_write_index;

	if (ulen > 0)
	{
		if (count >= ulen)
		{
			memcpy(buf, module_recv_buffer, ulen);
			buf[ulen] = 0;
			module_recv_write_index = 0;
			return ulen;
		}
		else
			return 0;
	}
	else
	{
		return 0;
	}	
}

void parse_imei_cmd(char* buf, int len)
{
	char* tmp1 = NULL,*tmp2= NULL;
	
	memset(dev_info.imei,0,sizeof(dev_info.imei));
	tmp1 = strstr(buf,"\r\n");	
	tmp2 = strstr(tmp1+2,"\r\n");
	memcpy(dev_info.imei,tmp1+strlen("\r\n"),tmp2-(tmp1+strlen("\r\n")));
	Logln(D_INFO,"imei=%s",dev_info.imei);
}

void parse_csq_cmd(char* buf, int len)
{
	char* tmp1 = NULL,*tmp2= NULL;
	char tmp[3]={0};
	
	memset(dev_info.imei,0,sizeof(dev_info.imei));
	tmp1 = strstr(buf,"CSQ: ");	
	tmp2 = strstr(tmp1+2,",");
	memcpy(tmp,tmp1+strlen("CSQ: "),tmp2-(tmp1+strlen("CSQ: ")));
	dev_info.csq = atoi(tmp);
	Logln(D_INFO,"csq=%d",dev_info.csq);
}

char* get_imei(void)
{
	if(strlen(g_flash.imei)>0)
	{
		return g_flash.imei;
	}
	else
	{
	strcpy(dev_info.imei,"515092400103763");
	return dev_info.imei;
	}
}
void parse_imsi_cmd(char* buf, int len)
{
	char* tmp1 = NULL,*tmp2= NULL;
	
	memset(dev_info.imsi,0,sizeof(dev_info.imsi));
	tmp1 = strstr(buf,"\r\n");	
	tmp2 = strstr(tmp1+2,"\r\n");
	memcpy(dev_info.imsi,tmp1+strlen("\r\n"),tmp2-(tmp1+strlen("\r\n")));
	Logln(D_INFO,"imsi=%s",dev_info.imsi);
}
char* get_imsi(void)
{
//	strcpy(dev_info.imsi,"515092400103763");
	return dev_info.imsi;
}
void send_data(char* buf, int len)
{    	
	int i = GetATIndex(AT_QISEND);	
	
	memset(at_pack[i].cmd_txt, 0, sizeof(at_pack[i].cmd_txt));
	sprintf(at_pack[i].cmd_txt,"AT+QISEND=0,%d\r\n",len);
	Logln(D_INFO, "send data %d byte", len);
	
	uart1_send(at_pack[i].cmd_txt, strlen(at_pack[i].cmd_txt));
	
	HAL_Delay(at_pack[i].timeout);

	memset(send_buffer, 0, SEND_BUF_SIZE);
	memcpy(send_buffer, buf, len);
	send_len = len;
}

/*查询文件大小*/
void QGEPOF1(void)
{
	int8_t i = GetATIndex(AT_QGEPOF);

	strcpy(at_pack[i].cmd_txt, "AT+QGEPOF=0,255");
	Send_AT_Command_Timeout(AT_QGEPOF,2);
}

/*查询有效期*/
void QGEPOF2(void)
{
	int8_t i = GetATIndex(AT_QGEPOF);
	
	strcpy(at_pack[i].cmd_txt, "AT+QGEPOF=2");
	Send_AT_Command_Timeout(AT_QGEPOF,2);
}

void parse_gnss_pkg(char* buf, int len)
{
	char* phead;
	char* ptail=NULL;
	char* pseek=NULL;
	char pkg[128]={0};

	Logln(D_INFO,"parse_gnss_pkg=%s",buf);
	do{
		pseek = strstr(phead,"+QGPSLOC:");

		if(pseek)
		{
			ptail = strstr(pseek,"\r\n");
			if(ptail)
			{
				memset(pkg, 0, sizeof(pkg));
				memcpy(pkg, pseek, ptail-pseek);
				parse_gps_data(pkg, ptail-pseek);
				phead = ptail+2;
			}
			else
			{
				break;
			}
		}
		else
		{
			break;
		}
		
	}while(1);
	
}

void parse_another_cmd(char* buf, int len)
{
	char* tmp1=NULL,*tmp2 = NULL;

	Logln(D_INFO,"parse_another_cmd,%s", buf);	

	if(tmp1 = strstr(buf,"CMS ERROR:"))
	{
		char data[6]={0};
		int err;
		tmp2 = strstr(tmp1,"\r\n");
		memcpy(data,tmp1+strlen("CMS ERROR:"),tmp2-(tmp1+strlen("CMS ERROR:")));
		err = atoi(data);
		Logln(D_INFO,"ERROR code = %d",err);
	}
	else if(tmp1 = strstr(buf,"CME ERROR:"))
	{
		char data[6]={0};
		int err;
		tmp2 = strstr(tmp1,"\r\n");
		memcpy(data,tmp1+strlen("CME ERROR:"),tmp2-(tmp1+strlen("CME ERROR:")));
		err = atoi(data);
		Logln(D_INFO,"CME ERROR code = %d",err);
		if(err == 7103)
			net_work_state=EN_INIT_STATE;
		else if(err == 516)	//gps未定位
			gps_led_flag = 0;
	}
	else if(strstr(buf,"QIOPEN:"))//CONNECT OK
	{
		if(net_work_state!=EN_CONNECTED_STATE)
		{
			RxMsgTypeDef msgType;

			net_work_state = EN_LOGING_STATE;
			msgType.Data[0] = AT_UP_LOGIN;
			PushElement(&at_send_Queue, msgType, 1);
		}
	}
	else if(strstr(buf,"ALREADY CONNECT"))
	{
		net_work_state=EN_CONNECTED_STATE;
	}
	else if(strstr(buf,"RDY"))
	{
		module_init();	
	}
	else if(strstr(buf,"RING"))
	{
	}
	else if(strstr(buf,"PDP DEACT") /*|| strstr(buf,"CLOSED")*/)
	{//NETWORD disconnect
		Logln(D_INFO,"CLOSED ---%s",buf);
		net_work_state=EN_CONNECT_STATE;
	}
	else if(tmp1 = strstr(buf,"CSQ:"))
	{
		parse_csq_cmd(tmp1,strlen(tmp1));
	}
	
	if(send_len>0)
	{
		uint8_t esc_val = 0;	

		if(strstr(buf,">"))
		{
			uart1_send(send_buffer, send_len);
			esc_val = 0x1a;
			uart1_send(&esc_val, 1);
			Logln(D_INFO, "send data1");
		}
		else
		{
			esc_val = 0x1B;		
			uart1_send(&esc_val, 1);
			Logln(D_INFO, "send data2");	
		}
		send_len = 0;
	}
}

bool cmpdata(char* data1, char* data2, unsigned short len)
{
	while(len--)
	{
		if(*(data1+len) != *(data2+len))
			return false;
	}

	return true;
}

/*返回最后一个包的结尾索引值*/
unsigned short split_diff_type(char* buf, unsigned short size)
{
	char* head,*tail;
	char gns[]={'+','Q','G','P','S','L','O','C',':'};
	char *find="OK\r\n";
	unsigned short i,gnss_index=0,gprs_index=0;
	unsigned short last_gns=0, last_gprs=0,last_end=0,ret=0,j=0;

	memset(gnss_buf, 0, sizeof(gnss_buf));
	memset(gprs_buf, 0, sizeof(gprs_buf));
	memset(another_buf, 0, sizeof(another_buf));

	for(i=0; i<size; i++)
	{
		if(*(buf+i) == '+')
		{
			head = buf+i;
			if(size-i>=sizeof(gns) && cmpdata(head, gns, sizeof(gns)))
			{
				tail = strstr(head, find);
				if(tail)
				{
					memcpy(gnss_buf+gnss_index, head, tail+strlen(find)-head);
					gnss_index += tail+strlen(find)-head;
					last_gns = tail+strlen(find)-head+i;
					i = last_gns-1;
				}
			}
		}
		else if(*(unsigned char*)(buf+i) == 0xff)
		{
			if(size-i>=1 && *(unsigned char*)(buf+i+1)==0xff)
			{
				unsigned char len;
				unsigned short j=0;
				
				head = buf+i;
				len=head[4];

				do
				{
					if(size-(i+j)>=1 && *(head+j)==0x0d && *(head+j+1)==0x0a)
					{
						if((len+PACKET_FRAME_LEN)==(j+2))
						{
							memcpy(gprs_buf+gprs_index, head, j+2);
							gprs_index += 2+j;
							last_gprs = 2+j+i;
							i = last_gprs-1;
							gprs_size = gprs_index;
							break;
						}
					}
					j++;
				}while(i+j<size);
				
			}
		}
		else
		{
			another_buf[j] = *(buf+i);
			j++;
			if(size-i>=1 && *(buf+i)=='\r'&& *(buf+i+1)=='\n')
				last_end = i+2;
		}
	}

//取4个数的最大值,最后一个回车的位置
	ret = last_gns>last_gprs?last_gns:last_gprs;
	ret = ret>last_end?ret:last_end;

//	Logln(D_INFO,"size%d,gns%d,gps%d,ret%d",size,last_gns,last_gprs,ret);

//	Logln(D_INFO,"return%d,split_diff_type=%s",ret,buf);
	return size-ret;
}

void parse_package_type(void)
{
	unsigned short size=0,offset=0;
	char tmp[512];
	char* buf = module_recv_buffer;
	unsigned short write_index = module_recv_write_index;	//用个局部变量保存，否则中断会改变这个值

	memset(tmp, 0, 512);
	recv_read_start_index = recv_read_end_index;
	recv_read_end_index = write_index;
//	Logln(D_INFO,"start=%d,end=%d",recv_read_start_index,recv_read_end_index);

	if(recv_read_end_index > recv_read_start_index)
	{
		size = recv_read_end_index-recv_read_start_index;
		memcpy(tmp, buf+recv_read_start_index, size);
	}
	else if(recv_read_end_index < recv_read_start_index)
	{
		size = MODULE_BUFFER_SIZE-recv_read_start_index;
		memcpy(tmp, buf+recv_read_start_index, size);
		memcpy(tmp+size, buf, recv_read_end_index);
		size += recv_read_end_index;
	}

//	conventdata0(tmp, size);
	offset = split_diff_type(tmp, size);
	if(write_index>=offset)
		recv_read_end_index = write_index-offset;
	else
		recv_read_end_index = MODULE_BUFFER_SIZE+write_index-offset;
	
//	Logln(D_INFO,"write=%d,module_write=%d,end=%d",write_index,module_recv_write_index,recv_read_end_index);
	
}

bool at_parse_recv(void)
{
	parse_package_type();

	if(strlen(gnss_buf)>0)
		parse_gnss_pkg(gnss_buf, strlen(gnss_buf));

	if(strlen(another_buf)>0)
		parse_another_cmd(another_buf, strlen(another_buf));

	if(gprs_size>0)
	{
		Logln(D_INFO,"gprs size %d",gprs_size);
		protocol_parse(gprs_buf, gprs_size);
		gprs_size = 0;
	}
}

void AT_QGREFLOC_FUN(void)
{
	int8_t i = GetATIndex(AT_QGREFLOC);
	
	sprintf(at_pack[i].cmd_txt,"AT+QGREFLOC=%s,%s",cell_loc.lat,cell_loc.lon);
	if(strlen(cell_loc.lat)==0 || strlen(cell_loc.lon)==0)
		return;
	Send_AT_Command_Timeout(AT_QGREFLOC, 10);
}
void miaoding(void)
{
	Send_AT_Command_Timeout(AT_QIFGCNT1, 2);     	
	Send_AT_Command_Timeout(AT_QCELLLOC, 10);	//获取基站经纬度

	Send_AT_Command_Timeout(AT_QIFGCNT2, 2);  
	AT_QGREFLOC_FUN();		//设置参考位置

	Send_AT_Command_Timeout(AT_QGPS_ON, 2);   //打开GPS	
	Send_AT_Command_Timeout(AT_QGNSSTS, 2); 	//GPS模块时间同步状态，对于EPO非常有用

	Send_AT_Command_Timeout(AT_QGNSSEPO, 2);   //打开EPO功能
	Send_AT_Command_Timeout(AT_QGEPOAID, 2);   //触发EPO功能

	QGEPOF1();	//查询文件大小
	QGEPOF2();	//查询有效期
}
void gnss_init(void)
{
	Send_AT_Command_Timeout(AT_QGPS_ON, 2);   	
//	Send_AT_Command_Timeout(AT_QGNSSTS, 2); 	
}
void module_init(void)
{
	Logln(D_INFO, "IOT_module Start Init");

	pure_uart1_buf();
	connect_times = 0;
	
	Send_AT_Command_Timeout(AT_ATE0, 20); 
	if(g_flash.flag!= 1)
	{
		Send_AT_Command_Timeout(AT_IPR, 1);
		Send_AT_Command_Timeout(AT_W, 1);
	}
	Send_AT_Command_Timeout(AT_ATS0, 1);
	Send_AT_Command_Timeout(AT_ATI, 1);	
	Send_AT_Command_Timeout(AT_CPIN, 5);
	Send_AT_Command_Timeout(AT_GSN, 2);
	Send_AT_Command_Timeout(AT_CIMI, 2);
	Send_AT_Command_Timeout(AT_CREG, 20);
	gnss_init();	
//	Send_AT_Command_Timeout(AT_QIMODE, 1);
	Send_AT_Command_Timeout(AT_QICSGP, 2);     
//	Send_AT_Command_Timeout(AT_QIREGAPP, 2);	
	Send_AT_Command_Timeout(AT_QIACT, 5);		
	Send_AT_Command_Timeout(AT_COPS, 2);
	
//	Send_AT_Command_Timeout(AT_QIFGCNT1, 2);     
//	Send_AT_Command_Timeout(AT_QIDNSIP, 2);

	Logln(D_INFO,"Init Complete");

}
void at_connect_service(void)
{
	int8_t i = GetATIndex(AT_QIOPEN);

	Logln(D_INFO,"%s,%d",g_flash.net.domain,g_flash.net.port);
	sprintf(at_pack[i].cmd_txt,"AT+QIOPEN=1,0,\"TCP\",\"%s\",%d,0,1",g_flash.net.domain,g_flash.net.port);
	Send_AT_Command_ext(AT_QIOPEN);
}

void at_close_service(void)
{
	Send_AT_Command_ext(AT_QICLOSE);
}

void send_gps_rmc_cmd(void)
{	
	Send_AT_Command_ext(AT_QGPS_RMC);
}

void send_gps_gga_cmd(void)
{	
	Send_AT_Command_ext(AT_QGPS_GGA);
}
void send_gps_QGPSLOC_cmd(void)
{	
	Send_AT_Command_ext(AT_QGPSLOC);
}

void at_process(void)
{	
	static uint16_t i=0;
//	Logln(D_INFO,"state@=%d",net_work_state);

	if(net_work_state==EN_INIT_STATE)
	{
		gsm_led_flag = 0;
		MODULE_RST();
		MODULE_PWRON();
		HAL_Delay(8000);
		gsm_led_flag = 1;
		module_init();
		net_work_state = EN_CONNECT_STATE;
	}
	else if(net_work_state==EN_CONNECT_STATE)
	{
		gsm_led_flag = 1;
		if(connect_times%40==0)
		{
			at_close_service();
			at_connect_service();
		}
		connect_times++;
		if(first_pwr && connect_times>200)
		{
			first_pwr = 0;
			Logln(D_INFO, "First reconnect %d times, restart ME",connect_times/40);
			net_work_state=EN_INIT_STATE;
		}
		else if(connect_times > 2000)
		{
			Logln(D_INFO, "reconnect %d times, restart ME",connect_times/40);
			net_work_state=EN_INIT_STATE;
		}
		i=0;
	}
	else if(net_work_state==EN_LOGING_STATE)
	{
		i++;
		if(i>200)
		{
			i=0;
			net_work_state = EN_CONNECT_STATE;
			Logln(D_INFO, "Loging Timeout, retry connect");
		}
	}
	else if(net_work_state==EN_CONNECTED_STATE)
	{
		gsm_led_flag = 2;
		connect_times = 0;
		push_interval_package_process();
	}

	PopATcmd();

	at_parse_recv();	
	
	HAL_Delay(50);
}
   
