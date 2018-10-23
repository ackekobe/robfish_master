#ifndef __ALTIMETER_H
#define __ALTIMETER_H

#include "stm32f4xx_conf.h"
#include "sys.h"
#include "rtthread.h"

#define ALTIMETER_TX_EN		(PCout(5))
#define ALTIMETER_POWER		(PCout(0))	//�߶ȼƵ�Դ����
#define REV_LEN				(16)

#define ALTIMETER_Rev_Done	(1)			//�߶ȼƽ����������
#define ALTIMETER_Rev_Undo	(0)			//�߶ȼ�δ��������

#define POWER_ON	(1)			//�߶ȼƿ�
#define POWER_OFF	(0)			//�߶ȼƹ�

extern u16 USART2_RX_STA;
extern u8 altitude[REV_LEN];
extern float fish_altitude;

void altimeter_send(u8 *buf, u8 len);
void altimeter_Init(u32 bound);
void altitude_analy(void);

#endif
