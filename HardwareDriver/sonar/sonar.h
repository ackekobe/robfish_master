#ifndef __SONAR_H
#define __SONAR_H

#include "stm32f4xx_conf.h"
#include "sys.h"
#include "rtthread.h"
#include "delay.h"


#define SONAR_BUFSIZE 	(512)
#define SONAR_TX_EN		(PBout(0))			//声呐发送使能端口
#define SONAR_POWER		(PCout(1))			//声呐电源管脚
#define FAULT_NUM		(5)					//接收错误最大次数

#define Sonar_ON		(1)
#define Sonar_OFF		(0)

#define Scan_Num		(15)				//每次扫描的次数

#define  Left_Limit		(0x0AF0)			//扫描范围左边界
#define  Right_Limit	(0x0E10)			//扫描范围右边界

typedef	struct								//扫描结果结构体
{
	float angle; 		//角度方位
	float block_dist;	//故障距离
	u8	  block;		//障碍物
} block_data_struct;

extern u8 sonar_samp_data[Scan_Num][300];
extern block_data_struct scan_result[Scan_Num];

void Sonar_Tx_Enable(const char *data, u16 ndtr);
void Sonar_Init(u32 bound);
void Sonar_run(void);
void Sonar_analyze(void);


#endif

