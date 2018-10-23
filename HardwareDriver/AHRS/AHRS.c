/**************************************************

文件名：AHRS.c
作  者：刘晓东
日  期：2018.7.18
版  本：V1.00
说  明：初始化AHRS
修改记录：

**************************************************/


#include "AHRS.h"
#include <string.h>
#include <stdio.h>

u8 AHRS_RxBuf[USART1_BUFSIZE];
u8 AHRS_TxBuf[USART1_BUFSIZE];


//static u8 DMA_RxBuf[AHRS_DMA_LEN];
u16 USART1_RX_STA = 0;

u8 ahrs_rev_flag = AHRS_Rev_Undo;

u8 USART1_Tx_Flag = 0;				//USART1发送标志位，1表示正在发送数据，0表示发送完成

u8 attitude_data[AHRS_DATA_LEN];	//横翻滚数据
u8 angleRate_data[AHRS_DATA_LEN];	//角速度数据
u8 accRate_data[AHRS_DATA_LEN];		//加速度数据
u8 velocity_data[AHRS_DATA_LEN];	//速度数据

const float pi = 3.1415926;
const float g = 9.80665;			//重力加速度常量

const u8 close_stream[11] = {0x75,0x65,0x0C,0x05,0x05,0x11,0x01,0x01,0x00,0x03,0x19};
const u8 polling_data[10] = {0x75,0x65,0x0C,0x04,0x04,0x01,0x00,0x00,0xEF,0xDA};

//惯导数据补偿
const float vel_x_static_compensation = 0.058;			//每一个周期x轴速度的静态补偿
const float vel_y_static_compensation = -0.0012;		//每一个周期y轴速度的静态补偿
float vec_x_dynamic_compensation = 0.0;					//每一个周期x轴速度的动态补偿
float vec_y_dynamic_compensation = 0.0;					//每一个周期y轴速度的动态补偿

const float roll_compensation = 0.37;			//翻滚角度补偿
const float pitch_compensation = 5.1;			//倾斜角度补偿

const float acc_x_compensation = 0.88;			//水平状态下X轴加速度补偿
const float acc_y_compensation = -0.02;			//水平状态下Y轴加速度补偿

union								//联合数据，用于转换浮点数
{  
	u8 temp[4];
    float real;
} conver;

_attitude attitude;
_angRate angRate;
_accRate accRate;
_velocity delta_velocity;
_velocity velocity;


/**************************************************

函数名：uart1_init(uint32_t bound)
作  者：刘晓东
日  期：2018.7.18
版  本：V1.00
说  明：初始化串口1
修改记录：

**************************************************/

static void USART1_Init(u32 bound)
{
	GPIO_InitTypeDef	GPIO_InitStructure;
	USART_InitTypeDef	USART_InitStructure;
//	DMA_InitTypeDef		DMA_InitStructure;
	NVIC_InitTypeDef	NVIC_InitStructure;
	
	
	/****串口缓冲区初始化*****/
	rt_memset(AHRS_RxBuf, 0x00, sizeof(AHRS_RxBuf));
	rt_memset(AHRS_TxBuf, 0x00, sizeof(AHRS_TxBuf));
	USART1_RX_STA = 0;
	
	USART_DeInit(USART1);  											//复位串口1
	
	/********时钟配置*********/
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	
	/**********串口IO配置***********/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_USART1);		//管脚功能复用
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_USART1);
	
#if 0
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4; 										//GPIOC4
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;									//输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;								//速度2MHz  足够满足波特率
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 									//推挽输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 									//上拉
	GPIO_Init(GPIOC,&GPIO_InitStructure); 											//初始化PC4
#endif
	
	//*******串口参数设置***********/
	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_Init(USART1, &USART_InitStructure);
	
	
	/*****使能USART1中断*****/
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;				//USART1串口中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;		//抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;				//响应优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);					//使能串口DMA发送接口
	

	
	/*****配置USART1中断*****/
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);					//使能字节接收中断	
	
	USART_Cmd(USART1, ENABLE);
	
}

/**************************************************

函数名：AHRS_DMA_Tx_init()
作  者：刘晓东
日  期：2018.7.18
版  本：V1.00
说  明：初始化DMA发送
修改记录：

**************************************************/
static void AHRS_DMA_Tx_Init()
{
	DMA_InitTypeDef DMA_InitStructure;
	NVIC_InitTypeDef	NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
	
	DMA_DeInit(DMA2_Stream7);												//发送数据流初始化
	while(DMA_GetCmdStatus(DMA2_Stream7) != DISABLE);
	
	DMA_InitStructure.DMA_Channel             = DMA_Channel_4;      		// DMA通道       
    DMA_InitStructure.DMA_PeripheralBaseAddr  = (u32)(&(USART1->DR));   	//目的地址
    DMA_InitStructure.DMA_Memory0BaseAddr     = (u32)AHRS_TxBuf;    		//源地址         
    DMA_InitStructure.DMA_DIR                 = DMA_DIR_MemoryToPeripheral; //传输方向   
    DMA_InitStructure.DMA_BufferSize          = AHRS_DMA_LEN;                    //数据长度        
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
	DMA_Init(DMA2_Stream7, &DMA_InitStructure);
	
	DMA_ITConfig(DMA2_Stream7, DMA_IT_TC, ENABLE);							//使能DMA中断
	
	
	/***********配置DMA中断优先级************/
	NVIC_InitStructure.NVIC_IRQChannel                   = DMA2_Stream7_IRQn;           
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;          
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 2; 
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
	
	DMA_Cmd(DMA2_Stream7, DISABLE);
	
}

#if 0
/**************************************************

函数名：AHRS_DMA_Rx_init()
作  者：刘晓东
日  期：2018.7.18
版  本：V1.00
说  明：初始化DMA接收
修改记录：

**************************************************/

static void AHRS_DMA_Rx_Init()
{
	DMA_InitTypeDef DMA_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
	
	DMA_DeInit(DMA2_Stream5);												//接收数据流初始化
	while(DMA_GetCmdStatus(DMA2_Stream5) != DISABLE);
	
	DMA_InitStructure.DMA_Channel             = DMA_Channel_4;      		// DMA通道       
    DMA_InitStructure.DMA_PeripheralBaseAddr  = (u32)(&(USART1->DR));   	//目的地址
    DMA_InitStructure.DMA_Memory0BaseAddr     = (u32)DMA_RxBuf;    			//源地址         
    DMA_InitStructure.DMA_DIR                 = DMA_DIR_PeripheralToMemory; //传输方向   
    DMA_InitStructure.DMA_BufferSize          = AHRS_DMA_LEN;                    //数据长度        
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
	
	DMA_Init(DMA2_Stream5, &DMA_InitStructure);
	
	USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);							//使能串口DMA接收接口
	
	
	DMA_Cmd(DMA2_Stream5, ENABLE);
}
#endif
/**************************************************

函数名：AHRS_Tx_Enable()
作  者：刘晓东
日  期：2018.7.18
版  本：V1.00
说  明：DMA发送使能
修改记录：

**************************************************/
void AHRS_Tx_Enable(const u8 *data, u16 ndtr)
{
//	AHRS_TX_EN = 1;					//AHRS发送使能
	
	while(USART1_Tx_Flag);			//确认上次发送已结束
	USART1_Tx_Flag = 1;
	
	rt_memcpy(AHRS_TxBuf, data, ndtr);	
	DMA_SetCurrDataCounter(DMA2_Stream7, ndtr);
	DMA_Cmd(DMA2_Stream7, ENABLE);
	
//	AHRS_TX_EN = 0;					//AHRS恢复接收状态
}


/**************************************************

函数名：deal_irq_rx_end
作  者：刘晓东
日  期：2018.7.18
版  本：V1.00
说  明：串口接收完成处理函数
参  数：
修改记录：由空闲接收修改为字节接收  lxd  2018.8.3

**************************************************/
static void deal_irq_rx_end()
{
	u8 res;
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{
		res = USART1->DR;
		if((USART1_RX_STA & 0x8000) == 0)
		{
			if(USART1_RX_STA < USART1_BUFSIZE)		//判断是够接收到足够的字节数
				AHRS_RxBuf[USART1_RX_STA++] = res;
			else
			{
				USART1_RX_STA |= 1<<15;				//接收完成位置位
				ahrs_rev_flag = AHRS_Rev_Done;
			}
		}
	}
}


/**************************************************

函数名：deal_irq_tx_end
作  者：刘晓东
日  期：2018.7.18
版  本：V1.00
说  明：串口接收完成处理函数
修改记录：

**************************************************/

static void deal_irq_tx_end(void)
{
	if(USART_GetFlagStatus(USART1, USART_IT_TC) == RESET)
	{
		USART_ITConfig(USART1, USART_IT_TC, DISABLE);
		
		USART1_Tx_Flag = 0;
	}
}

/**************************************************

函数名：USART1_IRQHandler
作  者：刘晓东
日  期：2018.7.18
版  本：V1.00
说  明：USART1接收中断处理函数
修改记录：

**************************************************/
void USART1_IRQHandler(void)
{
	rt_interrupt_enter();
	
	deal_irq_tx_end();
	deal_irq_rx_end();
	
	rt_interrupt_leave();
}


/**************************************************

函数名：DMA2_Stream7_IRQHandler
作  者：刘晓东
日  期：2018.7.18
版  本：V1.00
说  明：DMA发送中断处理函数
修改记录：

**************************************************/
void DMA2_Stream7_IRQHandler(void)
{
	if(DMA_GetITStatus(DMA2_Stream7, DMA_IT_TCIF7) != RESET)
	{
		DMA_ClearFlag(DMA2_Stream7, DMA_IT_TCIF7);						//清发送完成标志
		
		DMA_Cmd(DMA2_Stream7, DISABLE);									//关闭DMA发送
		
		USART_ITConfig(USART1, USART_IT_TC, ENABLE);					//打开发送完成中断
	}
}


/**************************************************

函数名：AHRS_Init
作  者：刘晓东
日  期：2018.7.18
版  本：V1.00
说  明：惯导初始化函数
修改记录：

**************************************************/
void AHRS_Init(u32 bound)
{
	USART1_Init(bound);
	AHRS_DMA_Tx_Init();
//	delay_us(1000);
	AHRS_Tx_Enable(close_stream, sizeof(close_stream));
#if 0
	AHRS_DMA_Rx_Init();
#endif
}


/**************************************************

函数名：AHRS_Require
作  者：刘晓东
日  期：2018.7.18
版  本：V1.00
说  明：惯导请求数据
修改记录：

**************************************************/
void AHRS_Require(void)
{
	rt_memset(AHRS_RxBuf,0,sizeof(AHRS_RxBuf));
	USART1_RX_STA = 0;
	AHRS_Tx_Enable(polling_data, sizeof(polling_data));
}

/**************************************************

函数名：AHRS_data_analys
作  者：刘晓东
日  期：2018.8.3
版  本：V1.00
说  明：AHRS接收数据解析
参  数：
修改记录：

**************************************************/

void AHRS_data_analys()
{
	u8 cnt = 0;
	u8 temp_buf[60];
#if 1
	static u8 temp_cnt = 0;
	static float temp_vel_x = 0.0;
	static float temp_vel_y = 0.0;
	static float temp_vel_z = 0.0;
	u8 buf[46];
#endif
	rt_memcpy(temp_buf, &AHRS_RxBuf[10], sizeof(temp_buf));
	if(temp_buf[0] == 0x75 && temp_buf[1] == 0x65)		//报文头匹配
	{
		rt_memcpy(accRate_data, &temp_buf[4], AHRS_DATA_LEN);
		rt_memcpy(angleRate_data, &temp_buf[18], AHRS_DATA_LEN);
		rt_memcpy(velocity_data, &temp_buf[32], AHRS_DATA_LEN);
		rt_memcpy(attitude_data, &temp_buf[46], AHRS_DATA_LEN);
		
		for(cnt = 0; cnt < 4; ++cnt)				//x轴加速度
			conver.temp[cnt] = accRate_data[5-cnt];
		accRate.acc_x = conver.real * g + acc_x_compensation;
		
		for(cnt = 0; cnt < 4; ++cnt)				//y轴加速度
			conver.temp[cnt] = accRate_data[9-cnt];
		accRate.acc_y = conver.real * g + acc_y_compensation;
		
		for(cnt = 0; cnt < 4; ++cnt)				//z轴加速度
			conver.temp[cnt] = accRate_data[13-cnt];
		accRate.acc_z = conver.real * g;
		
#if 1		
		if(temp_cnt++ < 50)
		{
			temp_vel_x += accRate.acc_x;			
			temp_vel_y += accRate.acc_y;				
			temp_vel_z += accRate.acc_z;					
		}
		else
		{
			temp_cnt = 0;
			temp_vel_x /= 50.0;
			temp_vel_y /= 50.0;
			temp_vel_z /= 50.0;
			
			sprintf((char*)buf, "pos_0:%8.5f,pos_1:%8.5f,pos_02:%8.5f\r\n",temp_vel_x,temp_vel_y,temp_vel_z);
			rt_kprintf((char*)buf);
			
			temp_vel_x = 0.0;
			temp_vel_y = 0.0;
			temp_vel_z = 0.0;
			
		}
#endif		

		
		for(cnt = 0; cnt < 4; ++cnt)				//转换角速度
			conver.temp[cnt] = angleRate_data[5-cnt];
		angRate.angRate_x = conver.real * 180 / pi;
		
		for(cnt = 0; cnt < 4; ++cnt)				
			conver.temp[cnt] = angleRate_data[9-cnt];
		angRate.angRate_y = conver.real * 180 / pi;
		
		for(cnt = 0; cnt < 4; ++cnt)				
			conver.temp[cnt] = angleRate_data[13-cnt];
		angRate.angRate_z = conver.real * 180 / pi;
		
#if 0		
		sprintf((char*)buf, "pos_0:%8.5f,pos_1:%8.5f,pos_02:%8.5f\r\n",velocity.vel_x,velocity.vel_y,velocity.vel_z);
		rt_kprintf((char*)buf);

		if(temp_cnt++ < 50)
		{
			temp_vel_x += delta_velocity.vel_x;
			temp_vel_y += delta_velocity.vel_y;
			temp_vel_z += delta_velocity.vel_z;
		}
		else
		{
			temp_cnt = 0;
			temp_vel_x /= 50.0;
			temp_vel_y /= 50.0;
			temp_vel_z /= 50.0;
			
			temp_vel_x = 0.0;
			temp_vel_y = 0.0;
			temp_vel_z = 0.0;
		}
#endif
		
		for(cnt = 0; cnt < 4; ++cnt)				//转换横偏滚
			conver.temp[cnt] = attitude_data[5-cnt];
		attitude.roll = conver.real * 180 / pi + roll_compensation;
		
		for(cnt = 0; cnt < 4; ++cnt)				
			conver.temp[cnt] = attitude_data[9-cnt];
		attitude.pitch =  conver.real * 180 / pi + pitch_compensation;
		
		for(cnt = 0; cnt < 4; ++cnt)				
			conver.temp[cnt] = attitude_data[13-cnt];
		attitude.yaw =  conver.real * 180 / pi;
		
#if 0		
		if(temp_cnt++ < 50)
		{
			temp_vel_x += attitude.roll;				//滚，横滚角
			temp_vel_y += attitude.pitch;				//横斜 俯仰角
			temp_vel_z += attitude.yaw;					//偏  平面的偏转
		}
		else
		{
			temp_cnt = 0;
			temp_vel_x /= 50.0;
			temp_vel_y /= 50.0;
			temp_vel_z /= 50.0;
			
			sprintf((char*)buf, "pos_0:%8.5f,pos_1:%8.5f,pos_02:%8.5f\r\n",temp_vel_x,temp_vel_y,temp_vel_z);
			rt_kprintf((char*)buf);
			
			temp_vel_x = 0.0;
			temp_vel_y = 0.0;
			temp_vel_z = 0.0;
			
		}
#endif
		
//		for(cnt = 0; cnt < 4; ++cnt)				//转换速度
//			conver.temp[cnt] = velocity_data[5-cnt];
////		delta_velocity.vel_x = conver.real * g + vel_x_compensation;
//		velocity.vel_x += delta_velocity.vel_x;
//		
//		for(cnt = 0; cnt < 4; ++cnt)				
//			conver.temp[cnt] = velocity_data[9-cnt];
//		delta_velocity.vel_y =  conver.real * g + vel_y_compensation;
//		velocity.vel_y += delta_velocity.vel_y;
//		
//		for(cnt = 0; cnt < 4; ++cnt)				
//			conver.temp[cnt] = velocity_data[13-cnt];
//		delta_velocity.vel_z =  conver.real * g + vel_z_compensation;
//		velocity.vel_z += delta_velocity.vel_z;
	}
	
}




