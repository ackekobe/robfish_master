/**************************************************

文件名：WK2124.c
作  者：刘晓东
日  期：2018.7.25
版  本：V1.00
说  明：SPI转USART的初始化文件
修改记录：

**************************************************/

#include "WK2124.h"

u8 uart_RST_Flag[4] = {0, 0, 0, 0};
u8 WK2124_Rev_Flag[4] = {WK2124_Rev_Undo, WK2124_Rev_Undo, WK2124_Rev_Undo, WK2124_Rev_Undo};		//WK2124四个串口接收标志位
u16 RNSS_rev_cnt = 0;
u8 RNSS_FIFO_cnt = 0;

extern u8 RDSS_REV_buf[UPPER_MAX_LEN];		//RDSS接收缓存
extern u8 RF_Rev_Buf[RF_Max_Len];			//RF接收缓存
extern char BD_raw_data[LOCA_INFO_LEN];		//RNSS定位信息接收缓存

extern struct rt_semaphore RNSS_rev_sem;	//RNSS接收信号量


const _baud_rate_def baud_array[BAUD_NUM] = {{307200,0x0002}, {153600,0x0005}, {76800, 0x000b}, {38400, 0x0017},
										     {19200, 0x002F}, {9600,  0x005f}, {4800,  0x00bf}, {2400,  0x017f},
										     {921600,0x0000}, {460800,0x0001}, {230400,0x0003}, {115200,0x0007},
										     {57600, 0x000f}, {28800, 0x001f}, {14400, 0x003f}, {7200,  0x007f}};

/**************************************************

函数名：WK2124_Init
作  者：刘晓东
日  期：2018.7.25
版  本：V1.00
说  明：初始化SPI2接口
修改记录：

**************************************************/

void WK2124_Init(void)
{
	GPIO_InitTypeDef	GPIO_InitStructure;
	SPI_InitTypeDef		SPI_InitStructure;
	EXTI_InitTypeDef	EXTI_InitStructure;
	NVIC_InitTypeDef	NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);					//使能GPIOB时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);					//使能GPIOE时钟，中断与复位信号线在PE上
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);					//使能SPI2时钟
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;		//PB12~15复用功能输出	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;							//复用功能
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;							//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;						//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;							//上拉
	GPIO_Init(GPIOB, &GPIO_InitStructure);									//初始化
	
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource13,GPIO_AF_SPI2); 					//PB13复用为 SPI2
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource14,GPIO_AF_SPI2); 					//PB14复用为 SPI2
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource15,GPIO_AF_SPI2); 					//PB15复用为 SPI2
	
	
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_SPI2,ENABLE);						//复位SPI2
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_SPI2,DISABLE);					//停止复位SPI2
	
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  	//设置SPI单向或者双向的数据模式:SPI设置为双线双向全双工
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;							//设置SPI工作模式:设置为主SPI
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;						//设置SPI的数据大小:SPI发送接收8位帧结构
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;								//串行同步时钟的空闲状态为低电平
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;							//串行同步时钟的第1个跳变沿（上升或下降）数据被采样
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;								//NSS信号软件（使用SSI位）管理
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;		//定义波特率预分频的值:波特率预分频值为64
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;						//指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
	SPI_InitStructure.SPI_CRCPolynomial = 7;								//CRC值计算的多项式
	SPI_Init(SPI2, &SPI_InitStructure);  									//根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器
	
//	SPI_SSOutputCmd(SPI2, ENABLE);											//NSS拉低，使能从器件
	
	SPI_Cmd(SPI2, ENABLE); 													//使能SPI外设
	WK2124_ReadWriteByte(0xff);												//启动传输,目的为了保持MOSI为高电平
	
	
	/************PG2初始化外部中断输入*************************/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;								//PG2作为中断输入	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;							//普通输入模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;						//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;							//上拉
	GPIO_Init(GPIOG, &GPIO_InitStructure);									//初始化
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);					//使能SYSCFG时钟
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOG, EXTI_PinSource2);			//PG2连接线2
	
	/*******************配置EXTI_Line2***********************/
	EXTI_InitStructure.EXTI_Line = EXTI_Line2;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;					//下降沿触发
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;					//外部中断线2
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;			//抢占中断优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;					
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	/************PG3推挽输出，WK2124复位管脚**********/ 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3; 										//GPIOG3
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;									//输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;								//速度2MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 									//推挽输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 									//上拉
	GPIO_Init(GPIOG,&GPIO_InitStructure); 											//初始化PG3
	
	/************PB12推挽输出，WK2124片选管脚**********/ 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12; 										//GPIOB12
	GPIO_Init(GPIOB,&GPIO_InitStructure); 
	
	WK_CS = 1;
	
	WK2124_RST();
	
}

/**************************************************

函数名：WK2124_SPI2_SetSpeed
作  者：刘晓东
日  期：2018.7.25
版  本：V1.00
说  明：配置SPI1的速度
		SPI速度 = fAPB1/分频系数
		APB1速度一般为42MHz
参	数：SPI_BaudRatePrescaler 分频系数
修改记录：

**************************************************/
void WK2124_SPI2_SetSpeed(u8 SPI_BaudRatePrescaler)
{
	assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_BaudRatePrescaler));			//判断有效性
	SPI2->CR1&=0XFFC7;														//位3-5清零，用来设置波特率
	SPI2->CR1|=SPI_BaudRatePrescaler;										//设置SPI2速度 
	SPI_Cmd(SPI2,ENABLE); 													//使能SPI2
} 

/**************************************************

函数名：WK2124_ReadWriteByte
作  者：刘晓东
日  期：2018.7.25
版  本：V1.00
说  明：发送/接收数据
参	数：TxData 待发送的数据
修改记录：

**************************************************/
u8 WK2124_ReadWriteByte(u8 TxData)
{		 			 
 
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);			//等待发送区空  
	
	SPI_I2S_SendData(SPI2, TxData); 										//通过外设SPI2发送一个byte数据
		
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);		//等待接收完一个byte  
 
	return SPI_I2S_ReceiveData(SPI2); 										//返回通过SPI2最近接收的数据	
 		    
}


/**************************************************

函数名：WK2124_read_reg
作  者：刘晓东
日  期：2018.7.25
版  本：V1.00
说  明：读WK2124的寄存器
参  数：reg 	寄存器地址
		data 	接收缓存
修改记录：

**************************************************/
u8 WK2124_read_reg(u8 reg, u8 *data)
{
	u8 buf_wr_data[2] = {0x00, 0xFF};							//发送缓存
	u8 buf_rd_data[2] = {0x00, 0x00};							//接收缓存
	buf_wr_data[0] = reg | READ_REG_HEAD;
	
	WK_CS = 0;
	buf_rd_data[0] = WK2124_ReadWriteByte(buf_wr_data[0]);		//写地址
	buf_rd_data[1] = WK2124_ReadWriteByte(buf_wr_data[1]);		//接收寄存器数据
	*data = buf_rd_data[1];
	WK_CS = 1;
	delay_us(2);
	return RT_EOK;
}

/**************************************************

函数名：WK2124_write_reg
作  者：刘晓东
日  期：2018.7.25
版  本：V1.00
说  明：写WK2124的寄存器
参  数：reg 	寄存器地址
		data 	发送的寄存器数据
修改记录：

**************************************************/
u8 WK2124_write_reg(u8 reg, u8 data)
{
	u8 buf_wr_data[2] = {0, 0};			//发送缓存
	buf_wr_data[0] = reg | WRITE_REG_HEAD;
	buf_wr_data[1] = data;
	WK_CS = 0;
	WK2124_ReadWriteByte(buf_wr_data[0]);		//写地址
	WK2124_ReadWriteByte(buf_wr_data[1]);		//写数据
	delay_us(2);
	WK_CS = 1;
	delay_us(2);
	return 0;
}

/**************************************************

函数名：WK2124_FIFO_RST
作  者：刘晓东
日  期：2018.7.29
版  本：V1.00
说  明：复位WK2124的FIFO
参  数：reg 	寄存器地址
		data 	发送的寄存器数据
修改记录：

**************************************************/
u8 WK2124_FIFO_RST(u8 uart_addr, u8 rev_send)
{
	u8 FCR_val = 0;
	u8 reg = 0;
	reg  = FCR | uart_addr;						//寄存器地址
	WK2124_read_reg(reg, &FCR_val);	
	WK2124_write_reg(reg, FCR_val | (1<<rev_send));	
	return RT_EOK;
}

/**************************************************

函数名：WK2124_write_FIFO
作  者：刘晓东
日  期：2018.7.29
版  本：V1.00
说  明：写WK2124的发送FIFO
参  数：uart_addt 	子串口地址
		data 		发送的数据
		length		发送数据的长度
返回值：-1	发送失败
		0  	发送成功
修改记录：

**************************************************/
u8 WK2124_write_FIFO(u8 uart_addr, const u8 *data, u16 length)
{
	u8 reg = 0;
	u16 out_loop = 0, inner_loop = 0;
	u16 send_length = 0;				//已发送数据长度
	reg = uart_addr | WRITE_FIFO_HEAD;
	
	WK2124_FIFO_RST(uart_addr, SEND_BIT);					//复位发送FIFO
	WK_CS = 0;
	WK2124_ReadWriteByte(reg);
	for(out_loop = 0; out_loop <= length / FIFO_SIZE; ++out_loop)	
	{
		
		
		for(inner_loop = 0; inner_loop < FIFO_SIZE; ++inner_loop)//以一个FIFO大小为写周期
		{
			WK2124_ReadWriteByte(data[out_loop*FIFO_SIZE + inner_loop]);
			if(++send_length >= length) 						//判断是否发送完成
				break;
		}
		
	}
	WK_CS = 1;
	if(send_length > length)
		return RT_ERROR;
	return RT_EOK;
}

/**************************************************

函数名：WK2124_read_FIFO
作  者：刘晓东
日  期：2018.7.29
版  本：V1.00
说  明：读WK2124的发送FIFO
参  数：uart_addt 	子串口地址
		data 		接收数据存放缓存区
返回值：接收到的数据长度
修改记录：

**************************************************/
u8 WK2124_read_FIFO(u8 uart_addr, u8 *data)
{
	u8 reg = 0, rev_cnt;
	u8 rev_length = 0;					//接收的数据长度
	reg = uart_addr | RFCNT;
	WK2124_read_reg(reg, &rev_length);	//接收FIFO中数据个数
	
	reg = uart_addr | READ_FIFO_HEAD;
	WK_CS = 0;
	WK2124_ReadWriteByte(reg);
	for(rev_cnt = 0; rev_cnt < rev_length; ++rev_cnt)	//读取接收FIFO中的数据
	{
		data[rev_cnt] = WK2124_ReadWriteByte(0xFF);
	}
	WK_CS = 1;
	WK2124_FIFO_RST(uart_addr, REV_BIT);				//复位接收FIFO
	return rev_length;
}

/**************************************************

函数名：WK2124_UART_Config
作  者：刘晓东
日  期：2018.7.25
版  本：V1.00
说  明：配置串口参数
参  数：uartParam	串口参数
修改记录：

**************************************************/
void WK2124_UART_Config(u8 uart_addr, uartTydef *uartParam)
{
//	u8  FCR_reg, SIER_reg, rev_val, send_val, band1, band2;		//测试用
	u8 SCR_reg, LCR_reg = 0, GENA_reg = 0, i = 0;
	WK2124_read_reg(GENA, &GENA_reg);
	WK2124_write_reg(GENA, ((u8)(1<<(uart_addr>>4))) | GENA_reg);						//使能串口
	WK2124_write_reg(GRST,(u8)(1<<(uart_addr>>4)));										//复位串口		
	
	
	LCR_reg |= uartParam->StopBits | uartParam->WordLength;
	if(uartParam->Parity != WK2124_PAMN)
		LCR_reg |= uartParam->Parity;
	WK2124_write_reg(uart_addr | LCR, LCR_reg);
	
	WK2124_write_reg(uart_addr | SCR, WK2124_RXEN | WK2124_TXEN);  					//发送与接收使能
	WK2124_write_reg(uart_addr | FCR, WK2124_TFEN | WK2124_RFEN);					//使能发送与接收FIFO
	WK2124_write_reg(uart_addr | SIER, uartParam->interrupt_enable);							//中断使能
	
	
	for(i = 0; i < BAUD_NUM; ++i)						//查找特定波特率对应的寄存器的值
	{
		if(baud_array[i].baud_rate == uartParam->BaudRate)
			break;
	}
	
	WK2124_write_reg(uart_addr | SPAGE, WK2124_SPAGE1);	//写页1
	WK2124_write_reg(uart_addr | BAUD0, (u8)baud_array[i].baud_reg_val);		//设置波特率
	WK2124_write_reg(uart_addr | BAUD1, (u8)(baud_array[i].baud_reg_val>>8));	
	WK2124_write_reg(uart_addr | RFTL, uartParam->rev_contact_val);			//接收FIFO触点值
	WK2124_write_reg(uart_addr | TFTL, uartParam->send_contact_val);			//发送FIFO触点值
	WK2124_write_reg(uart_addr | SPAGE, WK2124_SPAGE0);	//写页0
	
	WK2124_read_reg(uart_addr | SCR, &SCR_reg);
	
	WK2124_write_reg(uart_addr | SCR, SCR_reg | WK2124_SLEEPEN);  //休眠使能最后开启，否则子串口寄存器无法配置
}


/**************************************************

函数名：WK2124_RST
作  者：刘晓东
日  期：2018.7.25
版  本：V1.00
说  明：WK2124复位
修改记录：

**************************************************/
void WK2124_RST (void)
{
	u8 i = 0;
	WK_RST = 0;										//管脚拉低复位
	for(i = 0; i<10; i++)
		delay_us(1000);
	WK_RST = 1;
	for(i = 0; i<10; i++)
		delay_us(1000);
}

/**************************************************

函数名：WK2124_USART_Ctr
作  者：刘晓东
日  期：2018.9.6
版  本：V1.00
说  明：WK2124串口开关
参	数：num 	串口序号
		stat	开关状态(1:打开；0:关闭)
返回值：RT_EOK		正确
		RT_ERROR	函数参数错误
修改记录：

**************************************************/
u8 WK2124_USART_Ctr(u8 num, u8 stat)
{
	u8 GENA_reg;
	WK2124_read_reg(GENA, &GENA_reg);
	if(stat == 0)
	{
		WK2124_write_reg(GENA, (~(1<<num)) & GENA_reg);						//	关闭串口
		return RT_EOK;
	}
	else if(stat == 1)
	{
		WK2124_write_reg(GENA, ((1<<num)) | GENA_reg);						//	打开串口
		return RT_EOK;
	}
	return RT_ERROR;
}

/**************************************************

函数名：EXTI2_IRQHandler
作  者：刘晓东
日  期：2018.7.25
版  本：V1.00
说  明：外部中断线2的中断处理函数
修改记录：

**************************************************/
void EXTI2_IRQHandler(void)
{
	u8 GIFR_reg = 0;
	u8 SIFR_reg = 0;
	rt_interrupt_enter();
	WK2124_read_reg(GIFR, &GIFR_reg);			//读取全局中断标志寄存器
	if(GIFR_reg & 0x01)											//UART1产生中断
	{
		WK2124_write_reg(RF_UART1 | SPAGE, WK2124_SPAGE0);		//进入页0
		WK2124_read_reg(SIFR | RF_UART1, &SIFR_reg);			//读取子串口中断标志寄存器
		if(SIFR_reg & (WK2124_FERR_INT | WK2124_TFTRIG_INT))
		{
			WK2124_write_reg(GRST,(u8)(1<<(RF_UART1>>4)));		//复位串口
			uart_RST_Flag[RF_UART] = 1;												//置位串口复位标志
		}
		if(SIFR_reg & (WK2124_RXOVT_INT | WK2124_RFTRIG_INT))
		{
			WK2124_Rev_Flag[RF_UART] = WK2124_Rev_Done;
			WK2124_read_FIFO(RF_UART1, RF_Rev_Buf);
//			WK2124_USART_Ctr(RF_UART, 0);							//	关闭串口
			//置位信号量，在外部进行接收
		}
	}
	
	if(GIFR_reg & 0x02)												//UART2 RNSS产生中断
	{
		u8 last_data_len = 0;
		SIFR_reg = 0;
		WK2124_read_reg(SIFR | BDRN_UART2, &SIFR_reg);				//读取子串口中断标志寄存器
		if(SIFR_reg & (WK2124_FERR_INT | WK2124_TFTRIG_INT))
		{
			WK2124_write_reg(GRST,(u8)(1<<(BDRN_UART2>>4)));		//复位串口
			uart_RST_Flag[RNSS_UART] = 1;									//置位串口复位标志
		}
		if(SIFR_reg & (WK2124_RFTRIG_INT))							//RNSS串口接收触点中断
		{
			WK_CS = 0;
			WK2124_ReadWriteByte(READ_FIFO_HEAD | BDRN_UART2);
			for(RNSS_FIFO_cnt = 0; RNSS_FIFO_cnt < CONTACT_VAL; ++RNSS_FIFO_cnt)	//从FIFO中取出CONTACT_VAL个数据
			{
				BD_raw_data[RNSS_rev_cnt++] = WK2124_ReadWriteByte(0xFF);	
			}	
			WK_CS = 1;			
		}
		if(SIFR_reg & (WK2124_RXOVT_INT))							//RNSS串口接收超时中断
		{
			last_data_len = WK2124_read_FIFO(BDRN_UART2, (u8*)BD_raw_data + RNSS_rev_cnt);				//取出剩余的数据
			BD_raw_data[RNSS_rev_cnt + last_data_len] = '\0';
			if(BD_raw_data[0] == '$')					//接收的数据是否正确
			{
				WK2124_USART_Ctr(RNSS_UART, 0);							//	关闭串口
				WK2124_Rev_Flag[RNSS_UART] = WK2124_Rev_Done;
			}
			RNSS_rev_cnt = 0;																		//数据计数清零				
		}
	}
	
	if(GIFR_reg & 0x04)											//UART3 RDSS产生中断
	{
		SIFR_reg = 0;
		WK2124_read_reg(SIFR | BDRD_UART3, &SIFR_reg);			//读取子串口中断标志寄存器
		if(SIFR_reg & (WK2124_FERR_INT | WK2124_TFTRIG_INT))
		{
			WK2124_write_reg(GRST,(u8)(1<<(BDRD_UART3>>4)));		//复位串口
			uart_RST_Flag[RDSS_UART] = 1;												//置位串口复位标志
		}
		if(SIFR_reg & WK2124_RXOVT_INT)
		{
			WK2124_Rev_Flag[RDSS_UART] = WK2124_Rev_Done;							//置位串口接收标志位
			WK2124_read_FIFO(BDRD_UART3, RDSS_REV_buf);						//从FIFO中读取数据
			WK2124_USART_Ctr(RDSS_UART, 0);							//	关闭串口
		}
	}
	EXTI_ClearITPendingBit(EXTI_Line2);
	rt_interrupt_leave();
	if(WK2124_Rev_Flag[RNSS_UART] == WK2124_Rev_Done)
		rt_sem_release(&RNSS_rev_sem);
}




