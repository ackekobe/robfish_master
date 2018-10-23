/**************************************************

�ļ�����wdg.c
��  �ߣ�������
��  �ڣ�2018.8.9
��  ����V1.00
˵  �������Ź���ʼ������
�޸ļ�¼��

**************************************************/

#include "wdg.h"

u8 Feed_Flag = WDG_ENABLE; 		//ι����־��0 ι��  0x55 ��ι��


/**************************************************

��������WDG_Init
��  �ߣ�������
��  �ڣ�2018.8.6
��  ����V1.00
˵  �������Ź���ʼ������	
��  ����
�޸ļ�¼��

**************************************************/
void WDG_Init()
{
	
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);							//ʹ��GPIODʱ��
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11; 						
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;									//
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;								//�ٶ�2MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 									//�������
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 									//����
	GPIO_Init(GPIOD,&GPIO_InitStructure); 											//��ʼ��PD11

	WDI =0;																	
}


/**************************************************

��������WDG_Feed
��  �ߣ�������
��  �ڣ�2018.8.12
��  ����V1.00
˵  ����ι������	
��  ����
�޸ļ�¼��

**************************************************/
void WDG_Feed()
{
	if(Feed_Flag == WDG_ENABLE)
		WDI = !WDI;
	delay_us(800);
}


