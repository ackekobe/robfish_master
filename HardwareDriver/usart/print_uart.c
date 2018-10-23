/**************************************************

文件名：print_uart.c
作  者：刘晓东
日  期：2018.7.18
版  本：V1.00
说  明：初始化打印串口uart4
修改记录：

**************************************************/

#include "print_uart.h"	
#include "stdio.h"



u16 UART4_RX_STA = 0;
u8 fish_param[17];
u8 fish_param_flag = 0;				//接收到鱼运行参数标志


//#if 1
//#pragma import(__use_no_semihosting)             
////标准库需要的支持函数                 
//struct __FILE 
//{ 
//	int handle; 
//}; 

//FILE __stdout;       
////定义_sys_exit()以避免使用半主机模式    
//void _sys_exit(int x) 
//{ 
//	x = x; 
//} 
////重定义fputc函数 
//int fputc(int ch, FILE *f)
//{ 	
//	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
//	USART1->DR = (u8) ch;      
//	return ch;
//}
//#endif
// 

/**************************************************

函数名：uart4_init()
作  者：刘晓东
日  期：2018.7.18
版  本：V1.00
说  明：初始化IO口与串口4
修改记录：

**************************************************/
void uart4_init(u32 bound)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); 						//使能GPIOA时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4,ENABLE);						//使能USART4时钟
	
	NVIC_InitTypeDef	NVIC_InitStructure;										//中断配置
	
	
	
	//GPIO端口配置
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;								//输出功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;							//速度2MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 								//推挽输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 								//上拉	 
	GPIO_Init(GPIOA,&GPIO_InitStructure); 
	
	//串口4对应引脚复用映射
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource0,GPIO_AF_UART4); 					//GPIOA0复用为USART4
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource1,GPIO_AF_UART4); 					//GPIOA1复用为USART4
	

   //UART4 初始化设置
	USART_InitStructure.USART_BaudRate = bound;									//波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;					//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;						//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;							//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;				//收发模式
	USART_Init(UART4, &USART_InitStructure); 									//初始化串口4
	USART_Cmd(UART4, ENABLE);  													//使能串口4
	
	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;							//串口4中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;						//抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;								//响应优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);									//USART4接收中断开启
	
}

/**************************************************

函数名：rt_hw_console_output(const char *str)
作  者：刘晓东
日  期：2018.7.18
版  本：V1.00
说  明：重写打印串口的输出函数
修改记录：

**************************************************/
void rt_hw_console_output(const char *str)
{
    /* empty console output */	
	rt_base_t level;
	level = rt_hw_interrupt_disable();

	while(*str!='\0')
	{
		if(*str=='\n')
		{
			USART_SendData(UART4, '\r'); 
			while(USART_GetFlagStatus(UART4, USART_FLAG_TXE) == RESET);
		}
		USART_SendData(UART4, *str++); 
		while(USART_GetFlagStatus(UART4, USART_FLAG_TXE) == RESET);	
	}
	rt_hw_interrupt_enable(level);
}

/**************************************************

函数名：UART4_IRQHandler
作  者：刘晓东
日  期：2018.7.24
版  本：V1.00
说  明：UART4传输完成中断处理函数
修改记录：

**************************************************/
void UART4_IRQHandler(void)
{
	u8 res;
	rt_interrupt_enter();
	if(USART_GetITStatus(UART4, USART_IT_RXNE) != RESET)
	{
		res = UART4->DR;
		if((UART4_RX_STA & 0x8000) == 0)
		{
			if(UART4_RX_STA < FISH_PARAM_LEN)		//判断是够接收到足够的字节数
				fish_param[UART4_RX_STA++] = res;
			else
			{
				fish_param[UART4_RX_STA] = res;
				UART4_RX_STA |= 1<<15;				//接收完成位置位
				UART4->CR1 &= ~USART_Mode_Rx;
				fish_param_flag = 1;
				UART4_RX_STA = 0;
			}
		}
	}
	rt_interrupt_leave();
}

 



