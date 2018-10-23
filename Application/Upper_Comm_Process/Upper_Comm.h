#ifndef __UPPER_COMM_H
#define __UPPER_COMM_H

#include "stm32f4xx_conf.h"
#include "sys.h"
#include "rtthread.h"
#include "delay.h"

#include "struct.h"
#include "MS_Com_Process.h"
#include "RF.h"
#include "BD_RDSS.h"



#define BACK_STAT_LEN		(12)		//系统状态帧的长度
#define TRACE_INFO_LEN		(55)		//上送的航迹信息数据帧长度

#define RF_Type					(0)			//RF
#define	BD_Type					(1)			//北斗

#define Passive_Upload			(0)			//应上位机要求上送
#define Positive_Upload			(1)			//主动上送

#define ROUTE_NUM				(6)			//路径规划中路径点数

typedef enum _sys_master_stat			//从机主状态
{
	reday = 0x01,
	running,
	fatal_error,
	fault_run,
} fish_master_stat;


typedef enum _sys_slave_stat
{
	check_normal = 0x01,
	check_fault,
	stat_upload,
	stat_done,
	sys_stop,
	time_done,
	rising,
	rise_done,
	diving,
	dive_done,
	turn_lefting,
	turn_left_done,
	turn_righting,
	turn_right_done,
	backing,
	back_done,
	glidering,
	dlider_done,
	path_planing,
	path_plan_done,
	fish_forwarding,
	fish_forward_done,
	fish_zeroing,
	fish_zero_done,
	fish_triming,
	fish_trim_done,
#ifdef __DEBUG
	l_pectoral_moving,
	l_pectoral_move_done,
	r_pectoral_moving,
	r_pectoral_move_done,
	gravity_motor_moving,
	gravity_motor_move_done,
	cylinder_mot0r_moving,
	cylinder_motor_move_done,
	waist_motor_moving,
	waist_motor_move_done,
	tail_motor_moving,
	tail_motor_move_dong,
#endif
} fish_slave_stat;

typedef struct 						//系统状态结构体
{
	u8 sys_major_stat;				//系统主状态
	u16 sys_sub_stat;				//系统次状态
	float cabin_temp;					//舱体温度
	float elec_quantity;				//电量
} _sys_stat;


typedef struct						//鱼上浮数据
{
	u8 		up_mode;
	float 	up_dist;
} _fish_up;

typedef struct						//鱼后退数据
{
	u8 	back_mode;
	float back_dist;
} _fish_back;

typedef struct						//鱼下潜数据
{
	u8 	down_mode;
	float down_dist;
} _fish_down;

typedef struct						//鱼左转数据
{
	u8 	left_mode;
	float left_angle;
} _fish_left;

typedef struct						//鱼右转数据
{
	u8 	right_mode;
	float right_angle;
} _fish_right;

typedef struct						//鱼滑翔数据
{
	float max_depth;
	float glide_dist;
	u8 	glide_mode;
	float pectoral_angle;
} _fish_glide;

typedef struct						//鱼快速推进数据
{
	float ahead_speed;				//鱼快速推进时速度
} _fish_fast_ahead;

typedef struct						//鱼航路规划
{
	
	u8 longitude_type;				//0东经 1西经
	u8 latitude_type;				//0北纬 1南纬
	float longitude;				//经度			 
	float latitude;					//纬度		     	
	float depth;					//深度
} _fish_route;


typedef struct						//上位机下发的指定位置
{
	u8 new_loc;
	_fish_route loc_info;
	_time assign_timestamp;
} _upper_assign_loc;


typedef struct						//RF下发的的鱼体运行姿态
{
	u8 cmd_type;					//该状态量为鱼的实时状态，当状态改变时，需要实时更新该状态
	_fish_up fish_up;
	_fish_down fish_down;
	_fish_back fish_back;
	_fish_left fish_left;
	_fish_right fish_right;
	_fish_glide fish_glide;
	_fish_fast_ahead fish_fast_ahead;
	u16 out_water_inter;					//冒水时间间隔	
	_fish_route fish_route[ROUTE_NUM];		//6个航行点
#ifdef __DEBUG
	_move_motor left_pectoral_move;
	_move_motor right_pectoral_move;
	_oil_pumb_motor cylinder_move;
	_gravity_motor  gravity_move;
	_move_motor waist_move;
	_move_motor tail_move;
#endif
} _Upper_Fish_CMD;

//typedef struct						//鱼自身的航迹信息
//{
//	u8 longitude_type;				//0东经 1西经
//	u8 latitude_type;				//0北纬 1南纬
//	float longitude;				//经度			 
//	float latitude;					//纬度		     	
//	float depth;	
//} _Track_Information;

extern _fish_route Track_Info[ROUTE_NUM];
extern u8  RF_Send_Buf[UPPER_MAX_LEN];
extern _upper_assign_loc upper_assign_loc;


u8 stat_back_frame(u8 *buf, u8 up_type);
u8 track_frame(u8 *buf);
u8 data_upload_upper(u8 type, u8 *dat, u8 len);
u8 Upper_Message_Analy(u8 *buf,  u8 Upper_type);
void track_info_upper(u8 type);

#endif


