#ifndef  __INFO_RECORD_H
#define  __INFO_RECORD_H

#include "SD.h"
#include "ff.h"
#include "exfuns.h"
#include <rthw.h>
#include "fattester.h"
#include "delay.h"
#include "print_uart.h"
#include "rtthread.h"
#include "rtdef.h"

#define INFO_LEN		(32)	//信息记录的长度
#define sys_stat_bit	(0x01)	//系统状态信息位
#define location_bit	(0x02)	//定位信息位
#define error_bit		(0x04)	//错误信息位


typedef enum 					//信息记录的类型
{
	sys_stat_info = 0x01,
	location_info,
	error_info,
} _info_record;


typedef struct					//存储记录信息的buf
{
	u8 sys_stat_buf[INFO_LEN];
	u8 location_buf[INFO_LEN];
	u8 error_buf[INFO_LEN];
} _record_buf;


u8 info_record(u8 type, u8 *buf, u8 len);
void info_record_thread_entry(void* parameter);

#endif



