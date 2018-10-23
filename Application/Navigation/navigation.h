#ifndef __NAVIGATION_H
#define __NAVIGATION_H

#include "stm32f4xx_conf.h"
#include "sys.h"
#include "rtthread.h"
#include "delay.h"
#include "BD_RNSS.h"
#include "AHRS.h"
#include "Info_Record.h"
#include "Upper_Comm.h"


#define SEA_CHART_POS_NUM		(50)		//保存的海图位置信息数目



#define  BEIJING

#define  BJ		(0)
#define  TJ		(1)

typedef struct								//从海图读取的信息数据结构
{
	u8 sea_chart_index;
	float sea_chart_latitude;
	float sea_chart_longtitude;
}_sea_chart;

typedef struct								//最近目标距离
{
	u8 index;								//海图地点序列号
	float distance;							//距离	
} _target_loc;

typedef enum
{
	NaviPointTracking_NaviPointChoose = 0,							//初始状态，该状态下需要选取航迹点
	NaviPointTracking_NaviPointTracking,							//按照海图上的点进行追踪
	NaviPointTracking_NaviPointSwi,									//切换海图上的点
	NaviPointTracking_NewNaviPointTracking,							//上位机下发的新航迹点
	NaviPointTracking_NaviEndPoint,									//航迹终点状态
}_navi_stat;

void calc_location(void);
void Navigation_entry(void* parameter);
float time_trans(RTC_DateTypeDef *date, RTC_TimeTypeDef *time, _time *master_time_type);

#endif

