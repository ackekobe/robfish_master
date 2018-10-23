/**************************************************

�ļ�����BD_RNSS.c
��  �ߣ�������
��  �ڣ�2018.7.30
��  ����V1.00
˵  ��������RNSS���ֳ�ʼ���ļ�
�޸ļ�¼��

**************************************************/

#include "BD_RNSS.h"
#include <stdlib.h>				//�ַ�����������������ת������
#include <ctype.h>				//�����ж��ַ��Ƿ�����ĸ�����ֺ���
#include <string.h>

char BD_raw_data[LOCA_INFO_LEN];

/****����ģ�����͵�����ͷ*****/
const char* string_head[6] = {"GNRMC", "GNGGA", "GNGLL", "GNGSA", "GPGSV", "BDGSV"};
u8 day_per_month[12] = {31,28,31,30,31,30,31,31,30,31,30,31};

struct rt_semaphore RNSS_rev_sem;	//RNSS���ݽ����ź���





/**************************************************

��������BD_RNSS_Init
��  �ߣ�������
��  �ڣ�2018.7.30
��  ����V1.00
˵  ��������RNSS���ڲ���
		��������ģ��ĵ�Դ���عܽų�ʼ��
��  ����
�޸ļ�¼��

**************************************************/

void BD_RNSS_Init(void)
{
	GPIO_InitTypeDef	GPIO_InitStructure;
	uartTydef uartParaStructure;
	u8 GIER_reg, FCR_reg;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);					//ʹ��GPIOBʱ��
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);					//ʹ��GPIOBʱ��
	
	uartParaStructure.BaudRate = BAUDRATE_9600;			//RNSS������9600
	uartParaStructure.Parity = WK2124_PAMN;				//��У��
	uartParaStructure.StopBits = WK2124_STPL_1;			//1��ֹͣλ
	uartParaStructure.WordLength = WK2124_WORDLENGTH_8;	//8λ�ֳ�
	uartParaStructure.rev_contact_val = CONTACT_VAL;	//�����ż�ֵ
	uartParaStructure.send_contact_val = CONTACT_VAL;
	uartParaStructure.interrupt_enable = WK2124_RFTRIG_IEN | WK2124_RXOUT_IEN;   //���մ����ж� ���ճ�ʱ�ж�
	WK2124_UART_Config(BDRN_UART2, &uartParaStructure);
	
	
	WK2124_read_reg(GIER, &GIER_reg);
	WK2124_write_reg(GIER, (u8)(1<<(BDRN_UART2>>4)) | GIER_reg);					//�򿪴����ж�
	
	WK2124_read_reg(FCR | BDRN_UART2, &FCR_reg);
	WK2124_write_reg(FCR | BDRN_UART2, 0x03 | FCR_reg);					//��λFIFO
	
	
	
	
	/************PC2����������������عܽ�**********/ 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; 										//GPIOC2
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;									//���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;								//�ٶ�2MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 									//�������
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 									//����
	GPIO_Init(GPIOC,&GPIO_InitStructure); 											//��ʼ��PC2
	
	/************PG1���������RNSS���ڷ���ʹ��**********/ 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1; 										//GPIOG1
	GPIO_Init(GPIOG,&GPIO_InitStructure); 
	
	if(rt_sem_init(&RNSS_rev_sem, "RNSS_sem", 0, RT_IPC_FLAG_FIFO) != RT_EOK)	//��ʼ���ź���
		rt_kprintf("RNSS_sem init ERROR!\n");
	
	BD_RNSS_Tx = 0;																	//Ĭ��Ϊ����
	BD_Power_En = 0;																//��ʼ��ʱ�������ϵ�
}

/**************************************************

��������RNSS_start
��  �ߣ�������
��  �ڣ�2018.8.13
��  ����V1.00
˵  ����
��  ����
����ֵ��
�޸ļ�¼��

**************************************************/
void RNSS_start(void)
{
	u8 GENA_reg = 0;
	u8 FCR_reg = 0;
	BD_Power_En = 1;		//����ģ���ϵ�
	
	WK2124_read_reg(GENA, &GENA_reg);
	WK2124_write_reg(GENA, 0x02 | GENA_reg);	//ʹ�ܴ���
	
//	WK2124_read_reg(FCR | BDRN_UART2, &FCR_reg);
//	WK2124_write_reg(FCR | BDRN_UART2, 0x03 | FCR_reg);					//��λFIFO
}


/**************************************************

��������BD_seek_pos
��  �ߣ�������
��  �ڣ�2018.7.30
��  ����V1.00
˵  ����Ѱ���ַ����е�num�����ŵ�λ��
		�������͵������Զ��ŷָ�
��  ����str				Ѱ�ҵ��ַ���
		num				Ѱ�ҵĶ���
����ֵ���������ڵ�λ��
�޸ļ�¼��

**************************************************/
char* BD_seek_pos(char* str, u8 num)
{
	while(num)
	{
		if(*str == '*' || (!isalnum(*str) && *str != ',' && *str != '.' && *str != '-'))  //�ַ��Ƕ��š�С���㡢���š���ĸ�����ֻ��ߵ����ַ�����β,����
			return NULL;
		if(*str++ == ',')
			--num;
	}
	return str;
}

/**************************************************

��������isLeap
��  �ߣ�������
��  �ڣ�2018.8.2
��  ����V1.00
˵  �����ж�����Ƿ�Ϊ����
��  ����year ���
����ֵ��1 ������
		0 ��������
�޸ļ�¼��

**************************************************/
u8 isLeap(u16 year)
{
	if(year % 100 == 0)
	{
		return year % 400 ? 1 : 0;
	}
	return year%4 ? 1 : 0;
}

/**************************************************

��������Time_UTC2BJ
��  �ߣ�������
��  �ڣ�2018.8.2
��  ����V1.00
˵  ����UTCʱ��ת��Ϊ����ʱ��
��  ����trans_result	�������͵�UTCʱ�����ڵĽṹ��
����ֵ��
�޸ļ�¼��

**************************************************/

void Time_UTC2BJ(_BD_RNSS_data *trans_result)
{
	
	if(trans_result->time.hour < 16)	//����ʱ���Ƿ����24
	{
		trans_result->time.hour += 8;
		return;
	}
	trans_result->time.hour -= 16;
	
	if(isLeap(trans_result->time.year))	//�Ƿ�������
		day_per_month[1] = 29;
	else
		day_per_month[1] = 28;
	
	if(!(trans_result->time.day < day_per_month[trans_result->time.month - 1]))	//�Ƿ���һ�����һ��
	{
		trans_result->time.day += 1;
		return;
	}
	trans_result->time.day = 1;
	if(trans_result->time.month < 12)											//�Ƿ���12��
	{
		trans_result->time.month += 1;
		return;
	}
	trans_result->time.month = 1;
	trans_result->time.year += 1;
	return;
}

/**************************************************

��������BD_GNRMC_trans
��  �ߣ�������
��  �ڣ�2018.7.30
��  ����V1.00
˵  ����GNRMC�е�����ת��
��  ����str				Ҫת�����Ӵ�
		trans_result	ת�����
�޸ļ�¼��

**************************************************/
u8 BD_GNRMC_trans(char *str, _BD_RNSS_data *trans_result)
{
	char *pos = NULL;
	char *sub_str = NULL;
	u32 date = 0;
	
/******************����ʱ��*********************/
	pos = BD_seek_pos(str, 1);			//Ѱ��ʱ���������ַ����е�λ��
	if(pos == NULL)
		return RT_ERROR;
	else
	{
		trans_result->time.hour = (*pos - 0x30)*10 + (*(pos+1)-0x30); 		//�õ�Сʱ
		trans_result->time.min = (*(pos+2) - 0x30)*10 + (*(pos+3)-0x30);		//�õ�����
		trans_result->time.sec = (*(pos+4) - 0x30)*10 + (*(pos+5)-0x30);		//�õ�����
		trans_result->time.msec = ((u16)*(pos+7) - 0x30)*100 + (*(pos+8) - 0x30)*10 + (*(pos+9)-0x30); //	�õ�������
	}
	
/**********����������Чλ**************/
	pos = BD_seek_pos(str, 2);			//Ѱ������״̬���ַ����е�λ��
	if(pos == NULL)
		return RT_ERROR;
	else
	{
		if(*pos == 'A')						//������Ч
			trans_result->data_state = 1;
		else								//������Ч
		{
			trans_result->data_state = 0;
			return RT_ERROR;				//״̬λ�쳣
		}		
	}
/**********����γ��**************/
	pos = BD_seek_pos(str, 3);			
	if(pos == NULL)
		return RT_ERROR;
	else
	{
		sub_str = rt_strncpy(sub_str, pos, 9);
		trans_result->location.latitude = atof(sub_str);		//�ַ���ת��Ϊfloat����
		trans_result->location.N_S = *(pos+10);				//��γ���Ǳ�γ
	}
/**********��������**************/
	pos = BD_seek_pos(str, 5);			
	if(pos == NULL)
		return RT_ERROR;
	else
	{
		sub_str = rt_strncpy(sub_str, pos, 10);
		trans_result->location.longitude = atof(sub_str);		//�ַ���ת��Ϊfloat����
		trans_result->location.E_W = *(pos+11);					//����������
	}
/**********��������**************/
	pos = BD_seek_pos(str, 9);			
	if(pos == NULL)
		return RT_ERROR;
	else
	{
		sub_str = rt_strncpy(str, pos, 6);
		date = atol(sub_str);
		trans_result->time.day = date/10000;					//��
		trans_result->time.year = 2000 + date%100;				//��
		trans_result->time.month = (date/100)%100;				//��
	}
	
	Time_UTC2BJ(trans_result);									//UTCʱ��ת��Ϊ����ʱ��
	return RT_EOK;
}


/**************************************************

��������BD_GNGGA_trans
��  �ߣ�������
��  �ڣ�2018.7.30
��  ����V1.00
˵  ����GNGGA�е�����ת��
��  ����str				Ҫת�����Ӵ�
		trans_result	ת�����
�޸ļ�¼��

**************************************************/
u8 BD_GNGGA_trans(char *str, _BD_RNSS_data *trans_result)
{
	u8 data_len = 0;
	s8 symbol = 1;
	char *pos1 = NULL;
	char *pos2 = NULL;
	char *sub_str = NULL;				//�洢�����Ӵ�
	pos1 = BD_seek_pos(str, 9);			//Ѱ�Һ��θ߶����ַ����е�λ��
	if(pos1 == NULL)
		return RT_ERROR;
	pos2 = BD_seek_pos(str, 10);			
	if(pos2 == NULL)
		return RT_ERROR;
	if(*pos1 == '-')
	{
		++pos1;
		symbol = -1;
	}
	data_len = pos2 - pos1-1;			//�߶��ֽ�ռ���ֽ���
	sub_str = rt_strncpy(sub_str, pos1, data_len);
	trans_result->location.altitude = symbol * atof(sub_str);	//�߶��ַ���ת��Ϊfloat����
	return RT_EOK;
}

/**************************************************

��������BD_single_trans
��  �ߣ�������
��  �ڣ�2018.7.30
��  ����V1.00
˵  ������ȡ�ַ����еĵ����Ӵ�����ת��
��  ����str				Ҫת�����ַ���		
		head_str		Ҫת�����Ӵ���ͷ
		trans_result	ת�����
�޸ļ�¼��

**************************************************/
s8 BD_single_trans(char *str ,const char* head_str, _BD_RNSS_data *trans_result)
{
	u8 num;
	for(num = 0; num < 6; ++num)			//Ѱ��ƥ����ַ���ͷ	
	{
		if(head_str == string_head[num])
			break;
	}
	switch(num)
	{
		case 0:
			BD_GNRMC_trans(str, trans_result);		//ת��GNRMC����
			break;
		case 1:
			BD_GNGGA_trans(str, trans_result);		//ת��GNGGA����
			break;
		case 2:
		case 3:
		case 4:
		case 5:
		default:
			break;
	}
	return RT_EOK;
}


/**************************************************

��������BD_data_trans
��  �ߣ�������
��  �ڣ�2018.7.30
��  ����V1.00
˵  �����ӱ������͵��ַ��������н�������Ҫ������
��  ����BD_raw_data		�������͵�ԭʼ����
		trans_result	����ת�����
�޸ļ�¼��

**************************************************/

void BD_data_trans(char *BD_raw_data, _BD_RNSS_data *trans_result)
{
	u8 BD_data_head = 0;
	for(BD_data_head = 0; BD_data_head < 2; ++BD_data_head)
	{
		char *pos = NULL;										//���ַ����г��ֵ�λ��
		if((pos = rt_strstr(BD_raw_data, string_head[BD_data_head]) )!= RT_NULL)
		{
			BD_single_trans(pos, string_head[BD_data_head], trans_result);			//�����ַ�������ת��
		}
	}
}



