/**************************************************

文件名：sonar.c
作  者：刘晓东
日  期：2018.7.27
版  本：V1.00
说  明：避障声呐初始化文件
修改记录：

**************************************************/
#include "sonar.h"
#include <string.h>


static u8 Sonar_DMA_RxBuf[SONAR_BUFSIZE];

u8 Sonar_RxBuf[SONAR_BUFSIZE];
u8 Sonar_TxBuf[SONAR_BUFSIZE];

u8 	Sonar_Tx_Flag = 0;				//声呐发送标志位，1表示正在发送数据，0表示发送完成
u16 Sonar_Rx_Len = 0;				//数据接收长度
u16 Sonar_DMA_index = 0;

struct rt_semaphore senor_rev_sem;				//声呐数据接收信号量

const char sonar_start1[13] = {0x40,0x30,0x30,0x30,0x38,0x08,0x00,0xFF,0x02,0x03,0x17,0x80,0x02};
const char sonar_start2[13] = {0x40,0x30,0x30,0x30,0x38,0x08,0x00,0xFF,0x02,0x03,0x18,0x80,0x02};

//避障声呐设置参数
const char sonar_set1[81] = {0x40,0x30,0x30,0x34,0x43,0x4C,0x00,0xFF,0x02,0x47,0x13,0x80,0x02,0x1D,0x05,0x23,0x11,0x99,0x99,0x99,
						  0x05,0xE1,0x7A,0x14,0x00,0x99,0x99,0x99,0x05,0xEB,0x51,0xB8,0x03,0x32,0x00,0x64,0x00,0xF0,0x0A,0x10,
						  0x0E,0x4F,0x61,0x3F,0x64,0x64,0x00,0x0A,0x00,0x19,0x40,0x55,0x00,0xF9,0x00,0xE8,0x03,0xF4,0x01,0x40,
						  0x06,0x01,0x00,0x00,0x00,0x4F,0x32,0x61,0x00,0x3F,0x64,0x00,0x00,0x64,0x00,0x0A,0x00,0x00,0x00,0x00,
						  0x00};
const char sonar_set2[14] = {0x40,0x30,0x30,0x30,0x39,0x09,0x00,0xFF,0x02,0x04,0x13,0x80,0x02,0x1C};

const char sonar_samp_cmd[17] = {0x40,0x30,0x30,0x30,0x43,0x0C,0x00,0xFF,0x02,0x07,0x19,0x80,0x02,0x40,0x40,0x40,0x01};
	
u8 sonar_samp_data[Scan_Num][300];					//采集数据存储区
	
block_data_struct scan_result[Scan_Num];			//存储一次扫描的结果

/**************************************************

函数名：uart6_init(uint32_t bound)
作  者：刘晓东
日  期：2018.7.27
版  本：V1.00
说  明：初始化串口6
参  数：bound	波特率
修改记录：

**************************************************/

static void USART6_Init(u32 bound)
{
	GPIO_InitTypeDef	GPIO_InitStructure;
	USART_InitTypeDef	USART_InitStructure;
	NVIC_InitTypeDef	NVIC_InitStructure;
	
	
	/****串口缓冲区初始化*****/
	rt_memset(Sonar_RxBuf, 0x00, sizeof(Sonar_RxBuf));
	rt_memset(Sonar_TxBuf, 0x00, sizeof(Sonar_RxBuf));
	
	USART_DeInit(USART6);  											//复位串口6
	
	/********时钟配置*********/
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, ENABLE);
	
	/**********串口IO配置***********/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_USART6);		//管脚功能复用
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_USART6);
	
	/************PB0推挽输出，485模式控制**********/ 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0; 										//GPIOB0
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;									//输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;								//速度2MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 									//推挽输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 									//上拉
	GPIO_Init(GPIOB,&GPIO_InitStructure); 											//初始化PB0
	
	/*************避障声呐电源控制管脚***************/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_Init(GPIOC,&GPIO_InitStructure);
	
	//*******串口参数设置***********/
	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_Init(USART6, &USART_InitStructure);
	
	
	/*****使能USART6中断*****/
	NVIC_InitStructure.NVIC_IRQChannel = USART6_IRQn;				//USART6串口中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;		//抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;				//响应优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	USART_DMACmd(USART6, USART_DMAReq_Tx, ENABLE);					//使能串口DMA发送接口
	
	/*****配置USART6中断*****/
	USART_ITConfig(USART6, USART_IT_IDLE, ENABLE);					//使能IDLE中断	
	
	USART_Cmd(USART6, ENABLE);
	
	SONAR_TX_EN = 0;												//避障声呐默认为接收状态
	SONAR_POWER = 0;												//避障声呐默认为关闭状态
	
	
	if(rt_sem_init(&senor_rev_sem, "senor_sem", 0, RT_IPC_FLAG_FIFO) != RT_EOK)	//初始化信号量
		rt_kprintf("senor_sem init ERROR!\n");
}

/**************************************************

函数名：Sonar_DMA_Tx_init()
作  者：刘晓东
日  期：2018.7.27
版  本：V1.00
说  明：初始化DMA发送
修改记录：

**************************************************/
static void Sonar_DMA_Tx_Init()
{
	DMA_InitTypeDef DMA_InitStructure;
	NVIC_InitTypeDef	NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
	
	DMA_DeInit(DMA2_Stream6);												//发送数据流初始化
	while(DMA_GetCmdStatus(DMA2_Stream6) != DISABLE);
	
	DMA_InitStructure.DMA_Channel             = DMA_Channel_5;      		// DMA通道       
    DMA_InitStructure.DMA_PeripheralBaseAddr  = (u32)(&(USART6->DR));   	//目的地址
    DMA_InitStructure.DMA_Memory0BaseAddr     = (u32)Sonar_TxBuf;    	    //源地址         
    DMA_InitStructure.DMA_DIR                 = DMA_DIR_MemoryToPeripheral; //传输方向   
    DMA_InitStructure.DMA_BufferSize          = SONAR_BUFSIZE;              //数据长度        
    DMA_InitStructure.DMA_PeripheralInc       = DMA_PeripheralInc_Disable;  //外设地址不增长  
    DMA_InitStructure.DMA_MemoryInc           = DMA_MemoryInc_Enable;       //存储器地址增长  
    DMA_InitStructure.DMA_PeripheralDataSize  = DMA_PeripheralDataSize_Byte;//外设数据宽度   
    DMA_InitStructure.DMA_MemoryDataSize      = DMA_MemoryDataSize_Byte;  	//存储器数据宽度    
    DMA_InitStructure.DMA_Mode                = DMA_Mode_Normal;        	//单次/循环传输     
    DMA_InitStructure.DMA_Priority            = DMA_Priority_High;  		//优先级           
    DMA_InitStructure.DMA_FIFOMode            = DMA_FIFOMode_Disable;		//FIFO/直接模式          
    DMA_InitStructure.DMA_FIFOThreshold       = DMA_FIFOThreshold_HalfFull; //FIFO大小
    DMA_InitStructure.DMA_MemoryBurst         = DMA_MemoryBurst_Single;     //单次传输  
    DMA_InitStructure.DMA_PeripheralBurst     = DMA_PeripheralBurst_Single; //单次传输
	DMA_Init(DMA2_Stream6, &DMA_InitStructure);
	
	DMA_ITConfig(DMA2_Stream6, DMA_IT_TC, ENABLE);							//使能DMA中断
	
	
	/***********配置DMA中断优先级************/
	NVIC_InitStructure.NVIC_IRQChannel                   = DMA2_Stream6_IRQn;           
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;          
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0; 
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
	
	DMA_Cmd(DMA2_Stream6, DISABLE);
	
	SONAR_TX_EN = 0;													//默认为接收模式
}

/**************************************************

函数名：Sonar_DMA_Rx_init()
作  者：刘晓东
日  期：2018.7.27
版  本：V1.00
说  明：初始化DMA接收
修改记录：

**************************************************/
static void Sonar_DMA_Rx_Init()
{
	DMA_InitTypeDef DMA_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
	
	DMA_DeInit(DMA2_Stream1);												//接收数据流初始化
	while(DMA_GetCmdStatus(DMA2_Stream1) != DISABLE);
	
	DMA_InitStructure.DMA_Channel             = DMA_Channel_5;      		// DMA通道       
    DMA_InitStructure.DMA_PeripheralBaseAddr  = (u32)(&(USART6->DR));   	//源地址
    DMA_InitStructure.DMA_Memory0BaseAddr     = (u32)Sonar_DMA_RxBuf;    	//目的地址         
    DMA_InitStructure.DMA_DIR                 = DMA_DIR_PeripheralToMemory; //传输方向   
    DMA_InitStructure.DMA_BufferSize          = SONAR_BUFSIZE;              //数据长度        
    DMA_InitStructure.DMA_PeripheralInc       = DMA_PeripheralInc_Disable;  //外设地址不增长  
    DMA_InitStructure.DMA_MemoryInc           = DMA_MemoryInc_Enable;       //存储器地址增长  
    DMA_InitStructure.DMA_PeripheralDataSize  = DMA_PeripheralDataSize_Byte;//外设数据宽度   
    DMA_InitStructure.DMA_MemoryDataSize      = DMA_MemoryDataSize_Byte;  	//存储器数据宽度    
    DMA_InitStructure.DMA_Mode                = DMA_Mode_Normal;        	//单次/循环传输     
    DMA_InitStructure.DMA_Priority            = DMA_Priority_VeryHigh;  	//优先级           
    DMA_InitStructure.DMA_FIFOMode            = DMA_FIFOMode_Disable;		//FIFO/直接模式          
    DMA_InitStructure.DMA_FIFOThreshold       = DMA_FIFOThreshold_HalfFull; //FIFO大小
    DMA_InitStructure.DMA_MemoryBurst         = DMA_MemoryBurst_Single;     //单次传输  
    DMA_InitStructure.DMA_PeripheralBurst     = DMA_PeripheralBurst_Single; //单次传输
	
	DMA_Init(DMA2_Stream1, &DMA_InitStructure);
	
	USART_DMACmd(USART6, USART_DMAReq_Rx, ENABLE);							//使能串口DMA接收接口
	
	
	DMA_Cmd(DMA2_Stream1, ENABLE);
}


/**************************************************

函数名：Sonar_Tx_Enable()
作  者：刘晓东
日  期：2018.7.27
版  本：V1.00
说  明：DMA发送使能
修改记录：

**************************************************/
void Sonar_Tx_Enable(const char  *data, u16 ndtr)
{
	SONAR_TX_EN = 1;					//Sonar发送使能
	
	while(Sonar_Tx_Flag);			//确认上次发送已结束
	Sonar_Tx_Flag = 1;
	
	rt_memcpy(Sonar_TxBuf, data, ndtr);	
	DMA_SetCurrDataCounter(DMA2_Stream6, ndtr);
	DMA_Cmd(DMA2_Stream6, ENABLE);
	
	while(Sonar_Tx_Flag);
	
	SONAR_TX_EN = 0;					//Sonar恢复接收状态
}

/**************************************************

函数名：senor_irq_rx_end
作  者：刘晓东
日  期：2018.7.27
版  本：V1.00
说  明：串口接收完成处理函数
参  数：buf 接收数据的缓冲区
修改记录：

**************************************************/
static u16 Senor_irq_rx_end(u8 *buf)
{
	u16 len = 0;
	if(USART_GetITStatus(USART6, USART_IT_IDLE) != RESET)
	{
		USART6->SR;
		USART6->DR;
		DMA_Cmd(DMA2_Stream1, DISABLE);
//		USART6->CR1 &= ~(USART_Mode_Rx);						//关闭串口接收
		DMA_ClearFlag(DMA2_Stream1, DMA_FLAG_TCIF1);
		len = SONAR_BUFSIZE - DMA_GetCurrDataCounter(DMA2_Stream1);
		
		
		rt_memcpy(buf, Sonar_DMA_RxBuf, len);
		
		
		
		rt_sem_release(&senor_rev_sem);				//释放一个信号量
		
		Sonar_DMA_Rx_Init();
		
		return len;
	}
	return 0;
}

/**************************************************

函数名：Senor_irq_tx_end
作  者：刘晓东
日  期：2018.7.27
版  本：V1.00
说  明：串口接收完成处理函数
修改记录：

**************************************************/

static void Senor_irq_tx_end(void)
{
	if(USART_GetFlagStatus(USART6, USART_IT_TC) == RESET)
	{
		USART_ITConfig(USART6, USART_IT_TC, DISABLE);
		
		Sonar_Tx_Flag = 0;
	}
}

/**************************************************

函数名：USART6_IRQHandler
作  者：刘晓东
日  期：2018.7.27
版  本：V1.00
说  明：USART6接收中断处理函数
修改记录：

**************************************************/
void USART6_IRQHandler(void)
{
	rt_interrupt_enter();
	
	Senor_irq_tx_end();
	Sonar_Rx_Len = Senor_irq_rx_end(Sonar_RxBuf);
	
	rt_interrupt_leave();
}

/**************************************************

函数名：DMA2_Stream6_IRQHandler
作  者：刘晓东
日  期：2018.7.27
版  本：V1.00
说  明：DMA发送中断处理函数
修改记录：

**************************************************/
void DMA2_Stream6_IRQHandler(void)
{
	rt_interrupt_enter();
	
	if(DMA_GetITStatus(DMA2_Stream6, DMA_IT_TCIF6) != RESET)
	{
		DMA_ClearFlag(DMA2_Stream6, DMA_IT_TCIF6);						//清发送完成标志
		
		DMA_Cmd(DMA2_Stream6, DISABLE);									//关闭DMA发送
		
		USART_ITConfig(USART6, USART_IT_TC, ENABLE);					//打开发送完成中断
	}
	
	rt_interrupt_leave();
}


/**************************************************

函数名：Sonar_Init
作  者：刘晓东
日  期：2018.7.27
版  本：V1.00
说  明：避障声呐初始化函数
修改记录：

**************************************************/
void Sonar_Init(u32 bound)
{
	USART6_Init(bound);
	Sonar_DMA_Tx_Init();
	Sonar_DMA_Rx_Init();
}


/**************************************************

函数名：Sonar_run()
作  者：刘晓东
日  期：2018.8.6
版  本：V1.00
说  明：避障声呐运行函数，进行故障扫描
修改记录：

**************************************************/
void Sonar_run()
{
	u8 sonar_stat = 0;			//声呐运行状态
	u8 falut_cnt = 0;			//接收错误次数标志位
	u8 samp_cnt = 0;			//采样次数
	
	SONAR_POWER = Sonar_ON;		//声呐上电
//	rt_sem_take(&senor_rev_sem, RT_WAITING_FOREVER);
//	if(Sonar_RxBuf[11] == 0x04)
//		rt_memset(Sonar_RxBuf, 0x00, sizeof(Sonar_RxBuf));
	USART6->CR1 |= USART_Mode_Rx;			//接收使能
	while(1)
	{
		switch(sonar_stat)
		{
			case 0:
				samp_cnt = 0;
				falut_cnt = 0;
				Sonar_Tx_Enable(sonar_start1, sizeof(sonar_start1));				
				++sonar_stat;
				break;
			case 1:
				rt_sem_take(&senor_rev_sem, RT_WAITING_FOREVER);
				if(Sonar_RxBuf[10] == 0x01)
				{
					Sonar_Tx_Enable(sonar_start2, sizeof(sonar_start2));
					++sonar_stat;
				}
				else if(++falut_cnt == FAULT_NUM)					//接收出错次数为3次，重新初始化
				{
 					sonar_stat = 0;
					falut_cnt = 0;
				}
				break;
			case 2:
				rt_sem_take(&senor_rev_sem, RT_WAITING_FOREVER);
				if(Sonar_Rx_Len == 372 && Sonar_RxBuf[10] == 0x06)
				{
					Sonar_Tx_Enable(sonar_set1, sizeof(sonar_set1));		//设置声呐参数
					delay_us(1000);
					Sonar_Tx_Enable(sonar_set2, sizeof(sonar_set2));
					++sonar_stat;
				}
				else if(++falut_cnt == FAULT_NUM)									//接收出错次数为3次，重新初始化
				{
					sonar_stat = 0;
					falut_cnt = 0;
					
				}
				break;
			case 3:
				rt_sem_take(&senor_rev_sem, RT_WAITING_FOREVER);
				if(Sonar_Rx_Len == 22 && Sonar_RxBuf[10] == 0x04)
				{
					Sonar_Tx_Enable(sonar_samp_cmd, sizeof(sonar_samp_cmd));		//开始采样
					samp_cnt = 0;
					++sonar_stat;	
				}
				else if(++falut_cnt == FAULT_NUM)									//接收出错次数为3次，重新初始化
				{
					sonar_stat = 0;
					falut_cnt = 0;
				}
				break;
			case 4:
				rt_sem_take(&senor_rev_sem, RT_WAITING_FOREVER);
				if(Sonar_RxBuf[10] == 0x02)
				{
					rt_memcpy(&sonar_samp_data[samp_cnt++],Sonar_RxBuf, Sonar_Rx_Len);
				}
				Sonar_Tx_Enable(sonar_samp_cmd, sizeof(sonar_samp_cmd));
				break;
			default:
				break;
		}
		if(samp_cnt == 15)
		{
			sonar_stat = 0;
			samp_cnt = 0;				//采样达到15次，退出采样
			SONAR_POWER = Sonar_OFF;
			break;
		}
		delay_ms(10);
	}
}

/**************************************************

函数名：Sonar_analyst()
作  者：刘晓东
日  期：2018.8.24
版  本：V1.00
说  明：避障采样数据分析
参  数：senor_data  声呐扫描采集到的数据
修改记录：

**************************************************/
void Sonar_analyze()
{
	u8 cnt = 0;
	u16 bin_cnt = 0;
	u16 scan_dir = 0;
	u16 data_len = 0;
	for(cnt = 0; cnt < Scan_Num; ++cnt)
	{
		scan_dir = ((u16)sonar_samp_data[cnt][41]<<8) + sonar_samp_data[cnt][40];		//读取出扫描的方向
		scan_result[cnt].angle = (float)scan_dir/16 * (float)0.9;
		data_len = ((u16)sonar_samp_data[cnt][43]<<8) + sonar_samp_data[cnt][42];		//数据的长度
		for(bin_cnt = 8; bin_cnt < data_len-2; ++bin_cnt)					//8是因为声呐的前0.3m探测不到，所以略过
		{																	//datalen-2是因为探测到连续三个bin都有障碍物才算探测到障碍物
			if(sonar_samp_data[cnt][44+bin_cnt] > 128 && sonar_samp_data[cnt][45+bin_cnt]>128 && sonar_samp_data[cnt][46+bin_cnt]>128)
			{
				scan_result[cnt].block = 1;
				scan_result[cnt].block_dist = (float)bin_cnt *100 / data_len;
				break;
			}
		}
		if(bin_cnt >= data_len-2)
		{
			scan_result[cnt].block = 0;
			scan_result[cnt].block_dist = 0;
		}
	}
}









