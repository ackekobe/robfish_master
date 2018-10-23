/**************************************************

�ļ�����RTC.c
��  �ߣ�������
��  �ڣ�2018.8.13
��  ����V1.00
˵  ����RTCʵʱʱ�ӳ�ʼ���ļ�
�޸ļ�¼��

**************************************************/

#include "RTC.h"


/**************************************************

��������RTC_Set_Init
��  �ߣ�������
��  �ڣ�2018.8.13
��  ����V1.00
˵  ����RTC����ʱ��
��  ����hour 	Сʱ
		min		����
		sec		��
		ampm	����/����
����ֵ��SUCCESS	���óɹ�
		ERROR	����ʧ��
�޸ļ�¼��

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

��������RTC_Set_date
��  �ߣ�������
��  �ڣ�2018.8.13
��  ����V1.00
˵  ����RTC��������
��  ����year	��
		month	��
		date	��
		week	����
����ֵ��RT_EOK	���óɹ�
		RT_ERROR	����ʧ��
�޸ļ�¼��

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

��������MY_RTC_Init
��  �ߣ�������
��  �ڣ�2018.8.13
��  ����V1.00
˵  ����RTCʵʱʱ�ӳ�ʼ������
��  ����
����ֵ��RT_ERROR	��ʼ��ʧ��
		RT_EOK		��ʼ���ɹ�
�޸ļ�¼��

**************************************************/
u8 MY_RTC_Init(void)
{
	RTC_InitTypeDef RTC_InitStructure;
	u16 retry = 0x1FFF;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);		//ʹ��PWR��Դ
	PWR_BackupAccessCmd(ENABLE);							//ʹ�ܺ󱸼Ĵ�������
	
	if(RTC_ReadBackupRegister(RTC_BKP_DR0) != 0x5050)
	{
		RCC_LSEConfig(RCC_LSE_ON);					//����LSE
		while((RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET) && (retry != 0))
		{
			++retry;
			delay_us(10000);
		}
		if(retry == 0)
			return RT_ERROR;			//LSE����ʧ��
		
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);		//����RTCʱ��(RTCCLK),ѡ��LSE��ΪRTCʱ��    
		RCC_RTCCLKCmd(ENABLE);						//ʹ��RTCʱ�� 
		
		RTC_InitStructure.RTC_AsynchPrediv = 0x7F;					//RTC�첽��Ƶϵ��(1~0X7F)
		RTC_InitStructure.RTC_SynchPrediv  = 0xFF;					//RTCͬ����Ƶϵ��(0~7FFF)
		RTC_InitStructure.RTC_HourFormat   = RTC_HourFormat_24;		//RTC����Ϊ,24Сʱ��ʽ
		RTC_Init(&RTC_InitStructure);
		
		RTC_Set_Time(0, 0, 0, RTC_H12_AM);
		RTC_Set_Date(18, 1, 1, 1);
		
		RTC_WriteBackupRegister(RTC_BKP_DR0,0x5050);	//����Ѿ���ʼ������
	}
	return RT_EOK;
}


