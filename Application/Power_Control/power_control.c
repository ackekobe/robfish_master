/**************************************************

�ļ�����power_control.c
��  �ߣ�������
��  �ڣ�2018.9.20
��  ����V1.00
˵  ������Դ���Ƴ���
�޸ļ�¼��

**************************************************/

#include "power_control.h"

void power_control_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF,ENABLE); 							//ʹ��GPIOCʱ��
	
	/************PC1,2�����������Դ���ؿ���**********/ 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_8; 							//GPIOC1��2
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;									//���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;								//�ٶ�2MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 									//�������
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN; 									//����
	GPIO_Init(GPIOF,&GPIO_InitStructure); 											//��ʼ��PC
	
	POWER_24V = POWER_ON;
	POWER_48V = POWER_ON;
}


