/**************************************************

文件名：led.c
作  者：刘晓东
日  期：2018.8.3
版  本：V1.00
说  明：五个LED灯的初始化
修改记录：

**************************************************/

#include "led.h" 

/**************************************************

函数名：LED_Init
作  者：刘晓东
日  期：2018.8.6
版  本：V1.00
说  明：LED初始化函数	
参  数：
修改记录：

**************************************************/
void LED_Init(void)
{    	 
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);//使能GPIOG时钟

	//GPIOG初始化设置
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;//LED0和LED1对应IO口
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;		//普通输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;	//25MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;		//上拉
	GPIO_Init(GPIOG, &GPIO_InitStructure);//初始化GPIO
	
	GPIO_SetBits(GPIOG,GPIO_Pin_9 | GPIO_Pin_10);//灯灭

}






