#ifndef __SONAR_H
#define __SONAR_H

#include "stm32f4xx_conf.h"
#include "sys.h"
#include "rtthread.h"
#include "delay.h"


#define SONAR_BUFSIZE 	(512)
#define SONAR_TX_EN		(PBout(0))			//���ŷ���ʹ�ܶ˿�
#define SONAR_POWER		(PCout(1))			//���ŵ�Դ�ܽ�
#define FAULT_NUM		(5)					//���մ���������

#define Sonar_ON		(1)
#define Sonar_OFF		(0)

#define Scan_Num		(15)				//ÿ��ɨ��Ĵ���

#define  Left_Limit		(0x0AF0)			//ɨ�跶Χ��߽�
#define  Right_Limit	(0x0E10)			//ɨ�跶Χ�ұ߽�

typedef	struct								//ɨ�����ṹ��
{
	float angle; 		//�Ƕȷ�λ
	float block_dist;	//���Ͼ���
	u8	  block;		//�ϰ���
} block_data_struct;

extern u8 sonar_samp_data[Scan_Num][300];
extern block_data_struct scan_result[Scan_Num];

void Sonar_Tx_Enable(const char *data, u16 ndtr);
void Sonar_Init(u32 bound);
void Sonar_run(void);
void Sonar_analyze(void);


#endif

