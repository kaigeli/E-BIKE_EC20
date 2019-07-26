#include "ota_download.h"

VER_STRUCT g_ver;
download_struct g_download;


uint32_t calc_start_timer_by_timestamp(uint32_t timestamp)
{
	applib_time_struct local_time,time;
	applib_time_struct utc_time;
	uint32_t sec_now,sec_aim;
	float home_tz = GetTimeZone(PhnsetGetHomeCity());
	uint32_t result;
 
	applib_dt_utc_sec_2_mytime(timestamp, &utc_time, KAL_TRUE);
    	applib_dt_utc_to_rtc(home_tz, &utc_time, &local_time);
	sec_aim = local_time.nHour*3600+local_time.nMin*60+local_time.nSec;
	applib_dt_get_rtc_time(&time);
	sec_now = time.nHour*3600+time.nMin*60+time.nSec;

	zt_trace(TOTA,"zone=%f,utc_timer %04d/%02d/%02d_%02d:%02d:%02d,local_timer %04d/%02d/%02d_%02d:%02d:%02d",home_tz,utc_time.nYear,utc_time.nMonth,utc_time.nDay,
		utc_time.nHour,utc_time.nMin,utc_time.nSec,local_time.nYear,local_time.nMonth,local_time.nDay,local_time.nHour,local_time.nMin,local_time.nSec);
	zt_trace(TOTA,"time %04d/%02d/%02d_%02d:%02d:%02d",time.nYear,time.nMonth,time.nDay,time.nHour,time.nMin,time.nSec);
	if(sec_aim>=sec_now)
		result = sec_aim-sec_now;
	else
		result = abs(86400-sec_now+sec_aim);
	zt_trace(TOTA,"%s,sec_aim=%d,sec_now=%d,result=%d",__func__, sec_aim,sec_now,result);
	return result*1000;
}


void parse_ver_package(uint8_t* buf,uint8_t len)
{
	uint8_t* pos1,*pos2;
	uint8_t tmp[16]={0};

	//flag
	pos1 = strstr(buf,"flag=")+strlen("flag=");
	pos2 = strstr(pos1,";");
	strncpy(tmp,pos1,pos2-pos1);
	g_ver.flag = atoi(tmp);

	//url
	pos1 = strstr(buf,"url=")+strlen("url=");
	pos2 = strstr(pos1,";");
	memset(g_ver.url,0,sizeof(g_ver.url));
	strncpy(g_ver.url,pos1,pos2-pos1);

	//size
	pos1 = strstr(buf,"size=")+strlen("size=");
	pos2 = strstr(pos1,";");
	memset(tmp,0,sizeof(tmp));
	strncpy(tmp,pos1,pos2-pos1);
	g_ver.size = atoi(tmp);

	//timestamp
	pos1 = strstr(buf,"time=")+strlen("time=");
	pos2 = strstr(pos1,";");
	memset(tmp,0,sizeof(tmp));
	strncpy(tmp,pos1,pos2-pos1);
	g_ver.timestamp = atoi(tmp);

	Logln(D_INFO,"flag=%d,size=%d,timestamp=%d",g_ver.flag,g_ver.size,g_ver.timestamp);
	Logln(D_INFO,"url=%s",g_ver.url);

	if(g_ver.flag==1)
	{
		pos1 = strstr(g_ver.url,"http://")+strlen("http://");
		pos2 = strstr(pos1,".");
		memset(tmp,0,sizeof(tmp));
		strncpy(tmp,pos1,pos2-pos1);
		g_download.ip[0]=atoi(tmp);
		
		pos1 = strstr(pos2+1,".");
		memset(tmp,0,sizeof(tmp));
		strncpy(tmp,pos2+1,pos1-(pos2+1));
		g_download.ip[1]=atoi(tmp);
		
		pos2 = strstr(pos1+1,".");
		memset(tmp,0,sizeof(tmp));
		strncpy(tmp,pos1+1,pos2-(pos1+1));
		g_download.ip[2]=atoi(tmp);
		
		pos1 = strstr(pos2+1,":");
		memset(tmp,0,sizeof(tmp));
		strncpy(tmp,pos2+1,pos1-(pos2+1));
		g_download.ip[3]=atoi(tmp);

		pos2 = strstr(pos1+1,"/");
		memset(tmp,0,sizeof(tmp));
		strncpy(tmp,pos1+1,pos2-(pos1+1));
		g_download.port=atoi(tmp);

		memset(g_download.path,0,sizeof(g_download.path));
		strcpy(g_download.path,pos2);

		Logln(D_INFO,"ip=%d.%d.%d.%d:%d,path=%s",g_download.ip[0],g_download.ip[1],g_download.ip[2],g_download.ip[3],g_download.port,g_download.path);
		
		g_download.total_size = g_ver.size;
		g_download.complete = 0;
		g_download.num = 0;
		if(g_download.total_size%ONE_PACKAGE)
		{
			g_download.sum = g_download.total_size/ONE_PACKAGE+1;
		}
		else
		{
			g_download.sum = g_download.total_size/ONE_PACKAGE;
		}
		zt_trace(TOTA,"total=%d,sum=%d",g_download.total_size,g_download.sum);
		
		if(!IsMyTimerExist(ZT_OTA_START_UPDATE_TIMER))
		{
			StartTimer(ZT_OTA_START_UPDATE_TIMER,calc_start_timer_by_timestamp(g_ver.timestamp),ota_download_pre_start);
		}
	}
}


uint16_t Http_package_construct(char* pBuff,char* path,char* hostname,int rangestart,int rangeend)
{
	uint16_t len=0;
		
	sprintf(pBuff, "GET %s HTTP/1.1\r\nHost:%s\r\nRange:bytes=%d-%d\r\nUser-Agent:curl/7.49.1\r\nAccept:*/*\r\n\r\n", path, hostname,rangestart,rangeend);
	len+=strlen(pBuff);
	
	return len;
}
void Http_package_send(void)
{
	uint16_t size;
	uint8_t buf[256]={0};
	uint8_t hostname[32]={0};

	sprintf(hostname,"%d.%d.%d.%d:%d",g_download.ip[0],g_download.ip[1],g_download.ip[2],g_download.ip[3],g_download.port);
	zt_trace(TOTA,"%s,%d,%d,hostname=%s",__func__,g_download.target_range1,g_download.target_range2,hostname);

	size = Http_package_construct(buf, g_download.path, hostname,g_download.target_range1, g_download.target_range2);
	zt_trace(TOTA,"%s",buf);
	zt_socket_send(ota_soc_app_id, buf, size);
}
/* HTTP/1.1 206 Partial Content
 Server: nginx/1.11.3
 Date: Sat, 06 Aug 2016 09:33:38 GMT
 Content-Type: application/zip
 Content-Length: 11
 Last-Modified: Sat, 06 Aug 2016 09:32:15 GMT
 Connection: keep-alive
 ETag: "57a5ae9f-21c"
 Content-Range: bytes 0-10/540
*/
void Http_package_parse_head_item(uint8_t* pBuff,uint16_t len)
{
	uint8_t* phead,*ptail;
	uint8_t tmp[5]={0};
	uint8_t param_name[32]={0};
	phead = pBuff;

	ptail = strchr(phead,':');
	strncpy(param_name, phead, ptail-phead);

	if(strncmp(param_name,"Server",strlen("Server")) == 0)
	{
	}
	else if(strncmp(param_name,"Date",strlen("Date")) == 0)
	{
	}
	else if(strncmp(param_name,"Content-Type",strlen("Content-Type")) == 0)
	{
	}
	else if(strncmp(param_name,"Content-Length",strlen("Content-Length")) == 0)
	{
	}
	else if(strncmp(param_name,"Last-Modified",strlen("Last-Modified")) == 0)
	{
	}
	else if(strncmp(param_name,"Connection",strlen("Connection")) == 0)
	{
	}
	else if(strncmp(param_name,"ETag",strlen("ETag")) == 0)
	{
	}
	else if(strncmp(param_name,"Content-Range",strlen("Content-Range")) == 0)
	{
		phead = strstr(ptail,"bytes ")+strlen("bytes ");
		ptail = strchr(phead,'-');
		strncpy(tmp, phead, ptail-phead);
		g_download.actual_range1 = atoi(tmp);

		ptail += 1;
		phead = strchr(ptail,'/');
		memset(tmp,0,sizeof(tmp));
		strncpy(tmp,ptail, phead-ptail);
		g_download.actual_range2 = atoi(tmp);

		phead += 1;
		ptail = pBuff+len;
		memset(tmp,0,sizeof(tmp));
		strncpy(tmp,phead, ptail-phead);
		g_download.total_size = atoi(tmp);
		zt_trace(TOTA,"range1=%d,range2=%d,total=%d",g_download.actual_range1,g_download.actual_range2,g_download.total_size);
	}
}

void Http_package_parse_head(uint8_t* pBuff,uint16_t len)
{
	uint8_t* phead,*ptail;
	uint8_t headitem[64];
	uint8_t tmp[5]={0};
	uint16_t ret;

	zt_trace(TOTA,"%s,len=%d",__func__,len);

	phead = pBuff+strlen("HTTP/1.1 ");
	ptail = strstr(phead," ");
	memcpy(tmp,phead,ptail-phead);
	ret = atoi(tmp);
	if(ret==200 ||ret==206)
	{
		ptail = strstr(phead,"\r\n");
		phead = ptail+2;
		
		do{
			memset(headitem, 0, sizeof(headitem));
			ptail = strstr(phead,"\r\n");
			if(ptail)
			{
				memcpy(headitem,phead,ptail-phead);
				Http_package_parse_head_item(headitem,ptail-phead);
				if(strncmp(ptail,"\r\n\r\n",4)==0)
				{
					break;
				}
				else
				{
					phead = ptail+2;
				}
			}
			else
			{
				break;
			}
			
		}while(1);
	}

}

void Http_package_parse_body(uint8_t* pBuff,uint16_t len)
{
	uint8_t write_result;

	if(len>0)
	{
		write_result = ota_write_fs(pBuff, len);
		zt_trace(TOTA, "write_result=%d",write_result);

		if(write_result)
		{
			g_download.item_size += len;

			zt_trace(TOTA,"len=%d,item_size=%d,r2-r1=%d",len,g_download.item_size,g_download.actual_range2-g_download.actual_range1);
			if(g_download.item_size == (g_download.actual_range2-g_download.actual_range1+1))
			{
				g_download.num++;
				if(g_download.num < g_download.sum)
				{
					zt_socket_free(ota_soc_app_id);
					ota_download_start();
				}
				else
				{
					g_download.complete = 1;
					zt_trace(TOTA,"download complete,after 5 second restart system");
					FS_Delete(OTA_FILE_FULL_PATH);
					FS_Rename(OTA_FILE_FULL_PATH_TEMP,OTA_FILE_FULL_PATH);
					StartTimer(ZT_OTA_RESET_SYSTEM_TIMER,5000,zt_reset_system);
				}
			}
		}
	}
}

void Http_package_parse(kal_int8 socket_id,RcvDataPtr GetRcvData)
{
	#define MAX_READ_COUNT 1024*50
	uint8_t* head_start,*head_end,*pBuff;
	uint16_t head_len,len;

	pBuff = (uint8_t*)zt_Malloc(MAX_READ_COUNT);
	memset(pBuff, 0, MAX_READ_COUNT);
	len = GetRcvData(socket_id,pBuff,MAX_READ_COUNT);

	head_start = strstr(pBuff,"HTTP/1.1");
	head_end = strstr(pBuff,"\r\n\r\n");

	if(head_start&&head_end)
	{
		head_len = (head_end+4)-head_start;
		Http_package_parse_head(head_start,head_len);
		if(len>head_len)
		{
			Http_package_parse_body(head_start+head_len, len-head_len);
		}
	}
	else
	{
		Http_package_parse_body(pBuff, len);
	}

	zt_Free(pBuff);
}
void ota_download_pre_start(void)
{
	int ret;
	
	if(zt_adc_get_value()>700 && g_download.complete==0)	//外电24V
	{
		ret = FS_Open(OTA_FILE_FULL_PATH_TEMP,FS_READ_ONLY);
		if(ret>=FS_NO_ERROR)
		{
			FS_Close(ret);
			FS_Delete(OTA_FILE_FULL_PATH_TEMP);
		}

		ota_download_start();
	}
}
void ota_download_start(void)
{	
	zt_trace(TOTA,"%s,adc=%d,complete=%d,num=%d,sum=%d",__func__,zt_adc_get_value(),g_download.complete,g_download.num,g_download.sum);
	if(zt_adc_get_value()>700 && g_download.complete==0)	//外电24V
	{	
		if(g_download.num<g_download.sum)
		{
			g_download.target_range1 = g_download.num*ONE_PACKAGE;
			g_download.target_range2 = (g_download.num+1)*ONE_PACKAGE-1;
			g_download.item_size = 0;
			ota_launch();
		}
	}
}

void ota_launch(void)
{
	zt_trace(TOTA, "%s",__func__);
	ota_soc_app_id = zt_socket_get_app_id();
	ota_networ_para.network_info.ip[0] = g_download.ip[0];
	ota_networ_para.network_info.ip[1] = g_download.ip[1];
	ota_networ_para.network_info.ip[2] = g_download.ip[2];
	ota_networ_para.network_info.ip[3] = g_download.ip[3];
	ota_networ_para.network_info.port = g_download.port;
	zt_socket_launch(ota_soc_app_id, &ota_networ_para);
}

