#ifndef __STRUCT_H
#define __STRUCT_H

#include "stm32f4xx_conf.h"
#include "sys.h"


#define UPPER_MAX_LEN			(78)	//RDSS收发最大字节数,RF收发最大字节数为62
#define  RF_Max_Len				(62)	//RF收发数据的最大长度
#define  LOCA_INFO_LEN			(512)	//北斗RNSS定位信息数据长度

#define	CONTACT_VAL				(200)	//WK2124中RNSS接收FIFO的触点门槛值
typedef enum 						//WK2124的串口序号
{
	RF_UART = 0,
	RNSS_UART,
	RDSS_UART,
	Reserve_UART,
} wk2124_uart;


#define  WDG_ENABLE				(0)		//看门狗使能
#define  WDG_DISABLE			(0x55)	//看门狗不使能

typedef enum 
{
	motor_mode_wz = 1,				//位置模式
	motor_mode_sd,					//速度模式
	motor_mode_dd,					//断电模式
	motor_mode_tz,					//停止不断电模式
	motor_mode_hl,					//回零模式(每次冒水时电机寻零)
} _motor_mode;



typedef struct				//胸鳍电机参数
{
	u8 		run_mode;			//运行模式
	s8 		flap_amplitude;		//拍动幅度
	u8 		flap_fre_integer;	//拍动频率整数部分
	u8 		flap_fre_decimal;	//拍动频率小数部分，小数部分扩大100倍进行传输
	s16 	target_location;	//目标位置
	
} _move_motor;

typedef struct				//油泵电机参数
{
	u8 	run_mode;			//运行模式
	float target_location;	//目标位置
	float target_speed;		//目标速度
} _oil_pumb_motor;

typedef struct				//重心电机参数
{
	u8 	run_mode;			//运行模式
	s16 target_speed;		//目标速度
	float target_location;	//目标位置
} _gravity_motor;

#pragma pack(2)
typedef struct				//北斗上送日期时间
{
	u16 year;
	u8 month;
	u8 day;
	u8 hour;
	u8 min;
	u8 sec;
	u16 msec;
} _time;
#pragma pack()

typedef struct				//北斗上送位置
{
	u8 		N_S;			//南纬北纬
	float	latitude;		//纬度
	u8		E_W;			//东经西经
	float	longitude;		//经度
	float	altitude;		//海拔高度
}_location;

	
typedef struct 
{
	u8			data_state;		//定位数据有效位  1：有效  0：无效
	float 		speed;			//对地速度
	float		azimuth_angle;	//方位角
	_time 		time;				
	_location	location;		//位置信息
	
} _BD_RNSS_data;





#endif


