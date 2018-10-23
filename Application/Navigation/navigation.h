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


#define SEA_CHART_POS_NUM		(50)		//����ĺ�ͼλ����Ϣ��Ŀ



#define  BEIJING

#define  BJ		(0)
#define  TJ		(1)

typedef struct								//�Ӻ�ͼ��ȡ����Ϣ���ݽṹ
{
	u8 sea_chart_index;
	float sea_chart_latitude;
	float sea_chart_longtitude;
}_sea_chart;

typedef struct								//���Ŀ�����
{
	u8 index;								//��ͼ�ص����к�
	float distance;							//����	
} _target_loc;

typedef enum
{
	NaviPointTracking_NaviPointChoose = 0,							//��ʼ״̬����״̬����Ҫѡȡ������
	NaviPointTracking_NaviPointTracking,							//���պ�ͼ�ϵĵ����׷��
	NaviPointTracking_NaviPointSwi,									//�л���ͼ�ϵĵ�
	NaviPointTracking_NewNaviPointTracking,							//��λ���·����º�����
	NaviPointTracking_NaviEndPoint,									//�����յ�״̬
}_navi_stat;

void calc_location(void);
void Navigation_entry(void* parameter);
float time_trans(RTC_DateTypeDef *date, RTC_TimeTypeDef *time, _time *master_time_type);

#endif

