/**************************************************

文件名：CTD48M.c
作  者：刘晓东
日  期：2018.7.24
版  本：V1.00
说  明：提供温盐深CTD48M的初始化与接口函数
修改记录：

**************************************************/

#include "CTD48M.h"
#include "math.h"					//用到了数学公式复杂计算


#ifdef USART3_DMA_RX	
static u8 CTD_DMA_RxBuf[CTD_BUF_LEN];
#endif


u8 CTD48M_rev_flag = CTM48_Rev_Undo;

u8 CTD_RxBuf[CTD_BUF_LEN];
u16 USART3_RX_STA = 0;		//温盐深接收状态

const float ctd_temp_coeffi[2] = {-2.58156E+0, 5.90399E-4};								//温度计算系数
const float ctd_depth_coeffi[4] = {-2.51174E+2, 3.52897E-2, 2.00434E-10,-1.11898E-14};	//深度/压力计算系数
const float ctd_cond_coeffi[4] = {-2.75897E-1, 1.22162E-3, 2.52963E-10, -7.70466E-16}; //盐度计算系数





_Cal_Val CTD_Final_Val;			//温盐深计算出的最终结果

_sample_val sample_data;


/**************************************************

函数名：USART3_Init
作  者：刘晓东
日  期：2018.7.24
版  本：V1.00
说  明：与温盐深相连的USART3初始化
		DMA1初始化，开启DMA1接收中断
参  数：bound 波特率
修改记录：

**************************************************/
static void USART3_Init(u32 bound)
{  	 
	
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
#ifdef USART3_DMA_RX	
	DMA_InitTypeDef DMA_InitStructure;
#endif
	NVIC_InitTypeDef	NVIC_InitStructure;
	
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE); 							//使能GPIOB时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF,ENABLE); 							//使能GPIOF时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);							//使能USART3时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
	
	
	
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource10,GPIO_AF_USART3); 						//GPIOA10复用为USART3
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource11,GPIO_AF_USART3); 						//GPIOA11复用为USART3
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11; 						
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;									//复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;								//速度100MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 									//推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 									//上拉
	GPIO_Init(GPIOB,&GPIO_InitStructure); 											//初始化PA10，PA11

	/*****************温盐深开关管脚*************************************/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; 										//GPIOF9
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;									//输出
	GPIO_Init(GPIOF,&GPIO_InitStructure); 
	
	


	USART_InitStructure.USART_BaudRate = bound;										//波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;							//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_Odd;							//奇校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					//收发模式
	USART_Init(USART3, &USART_InitStructure); 										//初始化串口3	
		
	
	USART_Cmd(USART3, ENABLE);  													//使能串口3	
	USART_ClearFlag(USART3, USART_FLAG_TC);
	
	
#ifdef USART3_DMA_RX	
	DMA_DeInit(DMA1_Stream1);														//接收数据流初始化
	while(DMA_GetCmdStatus(DMA1_Stream1) != DISABLE);
	
	DMA_InitStructure.DMA_Channel             = DMA_Channel_4;      		// DMA通道       
    DMA_InitStructure.DMA_PeripheralBaseAddr  = (u32)(&(USART3->DR));   	//目的地址
    DMA_InitStructure.DMA_Memory0BaseAddr     = (u32)CTD_DMA_RxBuf;    		//源地址         
    DMA_InitStructure.DMA_DIR                 = DMA_DIR_PeripheralToMemory; //传输方向   
    DMA_InitStructure.DMA_BufferSize          = CTD_BUF_LEN;                //数据长度        
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

	DMA_Init(DMA1_Stream1, &DMA_InitStructure);
	USART_DMACmd(USART3, USART_DMAReq_Rx, ENABLE);							//使能串口DMA接收接口
	DMA_Cmd(DMA1_Stream1, ENABLE);
#endif
	
	/*****使能DMA1_Stream1中断*****/
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;							//串口3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;						//抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;								//响应优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);									//USART3接收中断开启
	
	CTD_Power = CTM48_OFF;
}


/**************************************************

函数名：CTD_send
作  者：刘晓东
日  期：2018.7.24
版  本：V1.00
说  明：问盐深发送命令与数据
参  数：buf 发送区首地址
		len 发送数据长度
修改记录：

**************************************************/
void CTD_send(u8 *buf, u8 len)
{
	u8 t;
	
	for(t=0; t<len; ++t)
	{
		while(USART_GetFlagStatus(USART3,USART_FLAG_TC)==RESET); 	//等待发送结束		
		USART_SendData(USART3,buf[t]); 								//发送数据
	}
	while(USART_GetFlagStatus(USART3,USART_FLAG_TC)==RESET); 		//等待发送结束
	
}


/**************************************************

函数名：DMA1_Stream1_IRQHandler
作  者：刘晓东
日  期：2018.7.24
版  本：V1.00
说  明：DMA1传输完成中断处理函数
修改记录：

**************************************************/
void USART3_IRQHandler(void)
{
	u8 res;
	rt_interrupt_enter();
	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
	{
		res = USART3->DR;
		if((USART3_RX_STA & 0x8000) == 0)
		{
			if(USART3_RX_STA < CTD_BUF_LEN)		//判断是够接收到足够的字节数
				CTD_RxBuf[USART3_RX_STA++] = res;
			else
			{
				USART3_RX_STA |= 1<<15;				//接收完成位置位
				CTD_Power = CTM48_OFF;
				CTD48M_rev_flag = CTM48_Rev_Done;
			}
		}
	}
	rt_interrupt_leave();
}

/**************************************************

函数名：CTD_Init(u32 bound)
作  者：刘晓东
日  期：2018.7.24
版  本：V1.00
说  明：高度计初始化接口函数
参  数：bound 波特率
修改记录：

**************************************************/
void CTD_Init(u32 bound)
{
	USART3_Init(bound);
}

/**************************************************

函数名：CTD_Rx_Enable
作  者：刘晓东
日  期：2018.8.9
版  本：V1.00
说  明：温盐深接收使能
参  数：
修改记录：

**************************************************/
void CTD_Rx_Enable(void)
{
	delay_ms(1000);
	CTD_Power = CTM48_ON;
}


/**************************************************

函数名：CTD_Val_Swap
作  者：刘晓东
日  期：2018.7.24
版  本：V1.00
说  明：采样数据中提取出有效数据
		根据CTD用户手册中的数据格式进行转换
参  数：data 温盐深的采样数据
返回值：temp_val 转换后的值
修改记录：

**************************************************/
static u16 CTD_Val_Swap(u8 *data)
{
	u16 byte0, byte1, byte2;
	u16 temp_val = 0;
	
	byte0 = data[0] & 0xFE;  			//LSB转换为MSB,转化为最后一位为无效位，取消掉
	byte1 = data[1] & 0xFE;
	byte2 = data[2] & 0x06;
	
	temp_val = (byte2 << 13) | (byte1 << 6) | (byte0 >> 1); 
	return temp_val;
}


/**************************************************

函数名：CTD_Data_Cal
作  者：刘晓东
日  期：2018.7.24
版  本：V1.00
说  明：由16进制的数据转换为double数据
		三个数据的计算公式由厂家用户手册给出
参  数：data 温盐深采样转换后的数据
		addr 数据地址
返回值：
修改记录：

**************************************************/

static void CTD_Data_Cal(_sample_val *data, u8 addr)
{
	int i = 0;
	switch(addr)
	{
		case DEPTH_ADDR:
			CTD_Final_Val.depth_cal_val = 0.0;
			for(i = 0; i < 4; ++i)				//CTD用户手册中给的公式计算			
			{
				CTD_Final_Val.depth_cal_val += (ctd_depth_coeffi[i] * pow(data->press_sam_val, i));
			}
			break;
		case TEMP_ADDR:
			CTD_Final_Val.temp_cal_val = 0.0;
			for(i = 0; i < 2; ++i)
			{
				CTD_Final_Val.temp_cal_val += (ctd_temp_coeffi[i] * pow(data->temp_sam_val, i));
			}
			break;
		case COND_ADDR:
			CTD_Final_Val.cond_cal_val = 0.0;
			for(i = 0; i < 4; ++i)
			{
				CTD_Final_Val.cond_cal_val += (ctd_cond_coeffi[i] * pow(data->cond_sam_val, i));
			}
			break;
		default:
			break;
	}
}



/**************************************************

函数名：CTD_Data_Trans
作  者：刘晓东
日  期：2018.7.24
版  本：V1.00
说  明：采样数据转化为真实数据
参  数：data 温盐深的采样数据
返回值：-1 数据有误
	    0  数据计算正确
修改记录：

**************************************************/

u8 CTD_Data_Trans(u8* data)
{
	u8 i = 0;
	u8 addr = 0;
	u8 press_cnt = 0, temp_cnt = 0, cond_cnt = 0;			//转换次数计数
	while(!((data[i] == 0x45) && (data[i+1] == 0x43) && (data[i+2] == 0x45) && (data[i+3] == 0x36)))  //四个字节为CTD启动报文中的字节
	{
		if(++i == (CTD_BUF_LEN - 27))  						// CTD启动报文（15）+一次数据报文（12） = 27
		{
			rt_kprintf("CTD Data Error!\n");
			return RT_ERROR;
		}
	}	
	
	for(i += 15; i < CTD_BUF_LEN; i += 3)					// i+15 :跳过CTD的启动报文
	{
		if(press_cnt && temp_cnt  && cond_cnt)				//采集到数据，退出循环
			break;
		addr = data[i+2] & 0xF8;
		switch(addr)
		{
			case DEPTH_ADDR:
				if(press_cnt++ == 0)	
					sample_data.press_sam_val = CTD_Val_Swap(data+i);				
				break;
			case TEMP_ADDR:
				if(temp_cnt++ == 0)
					sample_data.temp_sam_val = CTD_Val_Swap(data+i);				
				break;
			case COND_ADDR:
				if(cond_cnt++ == 0)
					sample_data.cond_sam_val = CTD_Val_Swap(data+i);				
				break;
			default:
				break;
		}
	}
	
	CTD_Data_Cal(&sample_data, DEPTH_ADDR);					//采样数据转化为最终数据
	CTD_Data_Cal(&sample_data, TEMP_ADDR);
	CTD_Data_Cal(&sample_data, COND_ADDR);
	
	USART3_RX_STA = 0;										//状态量清零      
	return RT_EOK;
}





