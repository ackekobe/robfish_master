/**************************************************

文件名：RTC.c
作  者：刘晓东
日  期：2018.8.13
版  本：V1.00
说  明：RTC实时时钟初始化文件
修改记录：

**************************************************/

#include "RTC.h"


/**************************************************

函数名：RTC_Set_Init
作  者：刘晓东
日  期：2018.8.13
版  本：V1.00
说  明：RTC设置时间
参  数：hour 	小时
		min		分钟
		sec		秒
		ampm	上午/下午
返回值：SUCCESS	设置成功
		ERROR	设置失败
修改记录：

**************************************************/
u8 RTC_Set_Time(u8 hour,u8 min,u8 sec,u8 ampm)
{
	RTC_TimeTypeDef RTC_TimeTypeInitStructure;
	
	RTC_TimeTypeInitStructure.RTC_Hours=hour;
	RTC_TimeTypeInitStructure.RTC_Minutes=min;
	RTC_TimeTypeInitStructure.RTC_Seconds=sec;
	RTC_TimeTypeInitStructure.RTC_H12=ampm;
	
	return RTC_SetTime(RTC_Format_BIN,&RTC_TimeTypeInitStructure);
	
}

/**************************************************

函数名：RTC_Set_date
作  者：刘晓东
日  期：2018.8.13
版  本：V1.00
说  明：RTC设置日期
参  数：year	年
		month	月
		date	日
		week	星期
返回值：RT_EOK	设置成功
		RT_ERROR	设置失败
修改记录：

**************************************************/
u8 RTC_Set_Date(u8 year,u8 month,u8 date,u8 week)
{
	
	RTC_DateTypeDef RTC_DateTypeInitStructure;
	RTC_DateTypeInitStructure.RTC_Date=date;
	RTC_DateTypeInitStructure.RTC_Month=month;
	RTC_DateTypeInitStructure.RTC_WeekDay=week;
	RTC_DateTypeInitStructure.RTC_Year=year;
	return RTC_SetDate(RTC_Format_BIN,&RTC_DateTypeInitStructure);
}


/**************************************************

函数名：MY_RTC_Init
作  者：刘晓东
日  期：2018.8.13
版  本：V1.00
说  明：RTC实时时钟初始化函数
参  数：
返回值：RT_ERROR	初始化失败
		RT_EOK		初始化成功
修改记录：

**************************************************/
u8 MY_RTC_Init(void)
{
	RTC_InitTypeDef RTC_InitStructure;
	u16 retry = 0x1FFF;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);		//使能PWR电源
	PWR_BackupAccessCmd(ENABLE);							//使能后备寄存器访问
	
	if(RTC_ReadBackupRegister(RTC_BKP_DR0) != 0x5050)
	{
		RCC_LSEConfig(RCC_LSE_ON);					//开启LSE
		while((RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET) && (retry != 0))
		{
			++retry;
			delay_us(10000);
		}
		if(retry == 0)
			return RT_ERROR;			//LSE开启失败
		
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);		//设置RTC时钟(RTCCLK),选择LSE作为RTC时钟    
		RCC_RTCCLKCmd(ENABLE);						//使能RTC时钟 
		
		RTC_InitStructure.RTC_AsynchPrediv = 0x7F;					//RTC异步分频系数(1~0X7F)
		RTC_InitStructure.RTC_SynchPrediv  = 0xFF;					//RTC同步分频系数(0~7FFF)
		RTC_InitStructure.RTC_HourFormat   = RTC_HourFormat_24;		//RTC设置为,24小时格式
		RTC_Init(&RTC_InitStructure);
		
		RTC_Set_Time(0, 0, 0, RTC_H12_AM);
		RTC_Set_Date(18, 1, 1, 1);
		
		RTC_WriteBackupRegister(RTC_BKP_DR0,0x5050);	//标记已经初始化过了
	}
	return RT_EOK;
}


