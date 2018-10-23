/**************************************************

文件名：Info_Record.c
作  者：刘晓东
日  期：2018.9.10
版  本：V1.00
说  明：信息记录程序
修改记录：

**************************************************/
#include "Info_Record.h"

_record_buf record_buf;			//信息记录缓存

struct rt_event info_record_event;		//信息记录事件event

u8 *error_dir = (u8*)"0:/error.txt";
u8 *sys_stat_dir = (u8*)"0:/sys_stat.txt";
u8 *location_dir = (u8*)"0:/location.txt";

/**************************************************

函数名：write_info
作  者：刘晓东
日  期：2018.9.10
版  本：V1.00
说  明：信息写入特定文件
参  数：dir			信息存储路径
		buf 		信息缓存
		len			信息内容长度
返回值：RT_EOK		信息写入成功
		RT_ERROR	信息写入失败
修改记录：

**************************************************/
u8 write_info(u8 *dir, u8 *buf, u8 len)
{

	u32 file_size = 0;
	if(mf_open(dir, FA_OPEN_ALWAYS | FA_WRITE, SD_dev) != FR_OK)
		return RT_ERROR;
	file_size = mf_size(SD_file);
	mf_lseek(file_size, SD_file);
	if(mf_write(buf, len, SD_file) != FR_OK)
		return RT_ERROR;
	mf_close(SD_file);
	delay_ms(10);
	return RT_EOK;
}


/**************************************************

函数名：info_record
作  者：刘晓东
日  期：2018.9.10
版  本：V1.00
说  明：记录信息
参  数：type		信息类型
		buf 		信息缓存
		len			信息内容长度
返回值：RT_EOK		信息记录成功
		RT_ERROR	信息记录失败
修改记录：

**************************************************/
u8 info_record(u8 type, u8 *buf, u8 len)
{
	u8 res = 0;
	switch(type)
	{
		case sys_stat_info:						//系统状态信息
			res = write_info(sys_stat_dir, buf, len);
			break;
		case location_info:						//定位信息
			res = write_info(location_dir, buf, len);
			break;
		case error_info:						//错误信息
			res = write_info(error_dir, buf, len);
			break;
		default:
			break;
	}
	return res;
}


/**************************************************

函数名：info_record_thread_entry
作  者：刘晓东
日  期：2018.9.11
版  本：V1.00
说  明：信息记录线程执行
参  数：parameter	线程参数
返回值：
修改记录：

**************************************************/
void info_record_thread_entry(void* parameter)
{
	u32 rev = 0;
	while(1)
	{
		if(rt_event_recv(&info_record_event, ((1<<0) | (1<<1) | (1<<2)), RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,	//有事件发生
			RT_WAITING_FOREVER, (rt_uint32_t*)&rev) == RT_EOK)
		{
			if(rev & sys_stat_bit)
				info_record(sys_stat_info, record_buf.sys_stat_buf, INFO_LEN);
			if(rev & location_bit)
				info_record(location_info, record_buf.location_buf, INFO_LEN);
			if(rev & error_info)
				info_record(error_info, record_buf.error_buf, INFO_LEN);
		}
	}
}



