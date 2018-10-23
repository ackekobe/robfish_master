/**************************************************

�ļ�����led.c
��  �ߣ�������
��  �ڣ�2018.8.3
��  ����V1.00
˵  �������LED�Ƶĳ�ʼ��
�޸ļ�¼��

**************************************************/

#include "led.h" 

/**************************************************

��������LED_Init
��  �ߣ�������
��  �ڣ�2018.8.6
��  ����V1.00
˵  ����LED��ʼ������	
��  ����
�޸ļ�¼��

**************************************************/
void LED_Init(void)
{    	 
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);//ʹ��GPIOGʱ��

	//GPIOG��ʼ������
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;//LED0��LED1��ӦIO��
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;		//��ͨ���ģʽ
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	//�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;	//25MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;		//����
	GPIO_Init(GPIOG, &GPIO_InitStructure);//��ʼ��GPIO
	
	GPIO_SetBits(GPIOG,GPIO_Pin_9 | GPIO_Pin_10);//����

}






