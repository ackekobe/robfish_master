#ifndef  __RTC_H
#define  __RTC_H

#include "stm32f4xx_conf.h"
#include "sys.h"
#include "rtthread.h"
#include "delay.h"

u8 MY_RTC_Init(void);
u8 RTC_Set_Time(u8 hour,u8 min,u8 sec,u8 ampm);
u8 RTC_Set_Date(u8 year,u8 month,u8 date,u8 week);


#endif

