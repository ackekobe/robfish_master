/**************************************************

文件名：power_control.c
作  者：刘晓东
日  期：2018.9.20
版  本：V1.00
说  明：电源控制程序
修改记录：

**************************************************/

#include "power_control.h"

void power_control_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF,ENABLE); 							//使能GPIOC时钟
	
	/************PC1,2推挽输出，电源开关控制**********/ 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_8; 							//GPIOC1，2
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;									//输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;								//速度2MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 									//推挽输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN; 									//上拉
	GPIO_Init(GPIOF,&GPIO_InitStructure); 											//初始化PC
	
	POWER_24V = POWER_ON;
	POWER_48V = POWER_ON;
}


