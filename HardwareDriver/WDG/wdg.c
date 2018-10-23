/**************************************************

文件名：wdg.c
作  者：刘晓东
日  期：2018.8.9
版  本：V1.00
说  明：看门狗初始化函数
修改记录：

**************************************************/

#include "wdg.h"

u8 Feed_Flag = WDG_ENABLE; 		//喂狗标志，0 喂狗  0x55 不喂狗


/**************************************************

函数名：WDG_Init
作  者：刘晓东
日  期：2018.8.6
版  本：V1.00
说  明：看门狗初始化函数	
参  数：
修改记录：

**************************************************/
void WDG_Init()
{
	
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);							//使能GPIOD时钟
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11; 						
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;									//
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;								//速度2MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 									//推挽输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 									//上拉
	GPIO_Init(GPIOD,&GPIO_InitStructure); 											//初始化PD11

	WDI =0;																	
}


/**************************************************

函数名：WDG_Feed
作  者：刘晓东
日  期：2018.8.12
版  本：V1.00
说  明：喂狗函数	
参  数：
修改记录：

**************************************************/
void WDG_Feed()
{
	if(Feed_Flag == WDG_ENABLE)
		WDI = !WDI;
	delay_us(800);
}


