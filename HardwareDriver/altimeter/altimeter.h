#ifndef __ALTIMETER_H
#define __ALTIMETER_H

#include "stm32f4xx_conf.h"
#include "sys.h"
#include "rtthread.h"

#define ALTIMETER_TX_EN		(PCout(5))
#define ALTIMETER_POWER		(PCout(0))	//高度计电源开关
#define REV_LEN				(16)

#define ALTIMETER_Rev_Done	(1)			//高度计接收数据完成
#define ALTIMETER_Rev_Undo	(0)			//高度计未接收数据

#define POWER_ON	(1)			//高度计开
#define POWER_OFF	(0)			//高度计关

extern u16 USART2_RX_STA;
extern u8 altitude[REV_LEN];
extern float fish_altitude;

void altimeter_send(u8 *buf, u8 len);
void altimeter_Init(u32 bound);
void altitude_analy(void);

#endif
