#ifndef  __MS_COM_PROCESS_H
#define  __MS_COM_PROCESS_H


#include "stm32f4xx_conf.h"
#include "sys.h"
#include "rtthread.h"
#include "delay.h"
#include "struct.h"
#include "MS_Comm.h"
#include "CRC8.h"


#define  slave_stat_bytes		(28)

typedef enum _cmd				//主从机通信命令码
{
	XTJC = 0x01,				//系统自检
	ZTSB = 0x02,				//状态上报
	XTTZ = 0x03,				//系统停止
	XTJS = 0x04,				//系统校时
	XTFW = 0x05,				//系统复位   //增加系统复位
	
	YTSF = 0x10,				//鱼体上浮
	YTXQ = 0x11,				//鱼体下潜
	YTZZ = 0x12,				//鱼体左转
	YTYZ = 0x13,				//鱼体右转
	YTHT = 0x14,				//鱼体后退
	YTHX = 0x15,				//鱼体滑翔
	YTKT = 0x16,				//鱼体快速推进
	YTHL = 0x17,				//鱼体回零
	YTZP = 0x18,				//鱼体自动配平
	
	HLGH = 0x20,				//航路规划
	HLCR = 0x21,				//鱼运动得到指定位置
#ifdef  __DEBUG
	DJXQL  = 0x30,				//左胸鳍电机
	DJXQR  = 0x31,				//右胸鳍电机
	DJZX   = 0x32,				//重心电机
	DJYB   = 0x33,				//腰部电机
	DJWB   = 0x34,				//尾部电机
	DJYG   = 0x35,				//油缸电机
#endif

	XTZT   = 0x80,				//上送系统状态
	HJXX   = 0x90,				//上送航迹信息	
} cmd;



typedef struct
{
	u8  left_pertoral;
	u8  right_pertoral;
	u8  oil_motor;
	u8  gravity_motor;
	u8  waist_motor;
	u8  tail_motor;
	u8  power_vol;
	u8  power_cur;
	u8  power_temp;
	u8  cabin1_temp;
	u8  cabin2_temp;
	u8  cabin3_temp;
	u8  cabin4_temp;
	u8  sys_master_stat;
	u8  sys_slave_stat;
	u32 sys_fault;
	float power_quanlity;
	float power_fault;
} _slave_stat;



extern u8 slave_stat_buf[28];
extern _move_motor  left_pectoral;	//左胸鳍参数
extern _move_motor	 right_pectoral;	//右胸鳍
extern _oil_pumb_motor  oil_pumb_motor;		//油泵电机参数
extern _gravity_motor 	 gravity_motor;		//重心电机参数
extern _move_motor	 waist_motor;		//腰部电机参数	
extern _move_motor		 tail_motor;		//尾部电机参数


u8 Master_Slave_Comm(u8 CMD_ID);



#endif

