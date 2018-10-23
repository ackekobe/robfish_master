/**************************************************

文件名：altimeter.c
作  者：刘晓东
日  期：2018.7.20
版  本：V1.00
说  明：高度计初始化文件
修改记录：

**************************************************/

#include "altimeter.h"


u8 alti_DMA_Rx_Buf[REV_LEN];							//DMA接收缓存
u8 altitude[REV_LEN];
u16 RS485_Rx_Len = 0;								//数据接收长度
u16 USART2_RX_STA = 0;								//串口2接收状态
u8	altimer_rev_flag = ALTIMETER_Rev_Undo;							//高度计接收数据标志位

float fish_altitude = 0.0;							//鱼所在高度


/**************************************************

函数名：altit_RS485_Init
作  者：刘晓东
日  期：2018.7.20
版  本：V1.00
说  明：与高度计相连的RS485初始化
		接收高度数据使用DMA
参  数：bound 波特率
修改记录：

**************************************************/

static void altit_RS485_Init(u32 bound)
{  	 
	
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;
	NVIC_InitTypeDef	NVIC_InitStructure;
	
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); 							//使能GPIOA时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,ENABLE); 							//使能GPIOC时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);							//使能USART2时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
	
	
	
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource2,GPIO_AF_USART2); 						//GPIOA2复用为USART2
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource3,GPIO_AF_USART2); 						//GPIOA3复用为USART2
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3; 							//GPIOA2与GPIOA3
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;									//复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;								//速度2MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 									//推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 									//上拉
	GPIO_Init(GPIOA,&GPIO_InitStructure); 											//初始化PA2，PA3
	
	/************PC5推挽输出，485模式控制**********/ 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5; 										//GPIOC5
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;									//输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;								//速度2MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 									//推挽输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 									//上拉
	GPIO_Init(GPIOC,&GPIO_InitStructure); 											//初始化PC5
	
	/*************高度计电源控制管脚***************/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_Init(GPIOC,&GPIO_InitStructure);
	


	USART_InitStructure.USART_BaudRate = bound;										//波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;							//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;								//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					//收发模式
	USART_Init(USART2, &USART_InitStructure); 										//初始化串口2	
	
	/*****使能USART2中断*****/
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;								//USART2串口中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;						//抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;								//响应优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);									//使能IDLE中断	
	
	USART_Cmd(USART2, ENABLE);  													//使能串口 2	
	USART_ClearFlag(USART2, USART_FLAG_TC);
	
	
	
	DMA_DeInit(DMA1_Stream5);														//接收数据流初始化
	while(DMA_GetCmdStatus(DMA1_Stream5) != DISABLE);
	
	DMA_InitStructure.DMA_Channel             = DMA_Channel_4;      		// DMA通道       
    DMA_InitStructure.DMA_PeripheralBaseAddr  = (u32)(&(USART2->DR));   	//源地址
    DMA_InitStructure.DMA_Memory0BaseAddr     = (u32)alti_DMA_Rx_Buf;    		//目的地址         
    DMA_InitStructure.DMA_DIR                 = DMA_DIR_PeripheralToMemory; //传输方向   
    DMA_InitStructure.DMA_BufferSize          = REV_LEN;                    //数据长度        
    DMA_InitStructure.DMA_PeripheralInc       = DMA_PeripheralInc_Disable;  //外设地址不增长  
    DMA_InitStructure.DMA_MemoryInc           = DMA_MemoryInc_Enable;       //存储器地址不增长  
    DMA_InitStructure.DMA_PeripheralDataSize  = DMA_PeripheralDataSize_Byte;//外设数据宽度   
    DMA_InitStructure.DMA_MemoryDataSize      = DMA_MemoryDataSize_Byte;  	//存储器数据宽度    
    DMA_InitStructure.DMA_Mode                = DMA_Mode_Normal;        	//单次/循环传输     
    DMA_InitStructure.DMA_Priority            = DMA_Priority_VeryHigh;  	//优先级           
    DMA_InitStructure.DMA_FIFOMode            = DMA_FIFOMode_Disable;		//FIFO/直接模式          
    DMA_InitStructure.DMA_FIFOThreshold       = DMA_FIFOThreshold_HalfFull; //FIFO大小
    DMA_InitStructure.DMA_MemoryBurst         = DMA_MemoryBurst_Single;     //单次传输  
    DMA_InitStructure.DMA_PeripheralBurst     = DMA_PeripheralBurst_Single; //单次传输
	
	DMA_Init(DMA1_Stream5, &DMA_InitStructure);
	USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);							//使能串口DMA接收接口
	DMA_Cmd(DMA1_Stream5, ENABLE);

			
	ALTIMETER_TX_EN = 0;													//默认为接收模式
	ALTIMETER_POWER	= POWER_OFF;													//高度计默认为关闭状态
}

/**************************************************

函数名：altimeter_send
作  者：刘晓东
日  期：2018.7.20
版  本：V1.00
说  明：高度计发送命令与数据
参  数：buf 发送区首地址
		len 发送数据长度
修改记录：

**************************************************/
void altimeter_send(u8 *buf, u8 len)
{
	u8 t;
	
	USART_Cmd(USART2, ENABLE);
	ALTIMETER_TX_EN = 1;											//设置为发送模式
//	delay_ms(5);
	
	for(t=0; t<len; ++t)
	{
		while(USART_GetFlagStatus(USART2,USART_FLAG_TC)==RESET); 	//等待发送结束		
		USART_SendData(USART2,buf[t]); 								//发送数据
	}
	while(USART_GetFlagStatus(USART2,USART_FLAG_TC)==RESET); 		//等待发送结束

//	delay_ms(5);
	ALTIMETER_TX_EN = 0;											//设置为接收模式
}

/**************************************************

函数名：USART2_IRQHandler
作  者：刘晓东
日  期：2018.7.20
版  本：V1.00
说  明：USART2中断处理函数
修改记录：

**************************************************/
void USART2_IRQHandler(void)
{
	u16 len = 0;
	rt_interrupt_enter();
	if(USART_GetITStatus(USART2, USART_IT_IDLE) != RESET)
	{
		USART2->SR;
		USART2->DR;
		DMA_Cmd(DMA1_Stream5, DISABLE);
		DMA_ClearFlag(DMA1_Stream5, DMA_FLAG_TCIF5);
		len = REV_LEN - DMA_GetCurrDataCounter(DMA1_Stream5);
		rt_memcpy(altitude, alti_DMA_Rx_Buf, len);
		
		DMA_SetCurrDataCounter(DMA1_Stream5, REV_LEN);
		DMA_Cmd(DMA1_Stream5, ENABLE);
		if(len >= 0x0A)
		{
			ALTIMETER_POWER = POWER_OFF;				//在此处关闭高度计
			altimer_rev_flag = ALTIMETER_Rev_Done;		//高度计接收到正确数据
		}
	}

	rt_interrupt_leave();
}

/**************************************************

函数名：altimeter_Init(u32 bound)
作  者：刘晓东
日  期：2018.7.20
版  本：V1.00
说  明：高度计初始化接口函数
参  数：bound 波特率
修改记录：

**************************************************/
void altimeter_Init(u32 bound)
{
	altit_RS485_Init(bound);
}

/**************************************************

函数名：altitude_analy
作  者：刘晓东
日  期：2018.9.6
版  本：V1.00
说  明：高度计数据计算
参  数：
修改记录：

**************************************************/
void altitude_analy()
{
	u8 temp;						//缓存高度数据的每一位
	fish_altitude = 0.0;
//	temp = altitude[0] - 0x30;
//	fish_altitude += temp * 100;
	temp = altitude[1] - 0x30;
	fish_altitude += temp * 10;
	temp = altitude[2] - 0x30;
	fish_altitude += temp;
	temp = altitude[4] - 0x30;
	fish_altitude += temp * (float)0.1;
	temp = altitude[5] - 0x30;
	fish_altitude += temp * (float)0.01;
	temp = altitude[6] - 0x30;
	fish_altitude += temp * (float)0.001;
}



