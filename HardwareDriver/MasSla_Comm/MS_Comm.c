/**************************************************

文件名：MS_Comm.c
作  者：刘晓东
日  期：2018.7.19
版  本：V1.00
说  明：主从机SPI通信初始化文件
修改记录：

**************************************************/

#include "MS_Comm.h"


u8 slave_send_flag = 0;		//从机上送数据标志，0 从机没有数据上送  1 从机有数据要上送
u8 SPI_Error_stat = 0;		//SPI的错误状态


/**************************************************

函数名：MS_Comm_Init
作  者：刘晓东
日  期：2018.7.19
版  本：V1.00
说  明：初始化SPI1接口
修改记录：

**************************************************/

void MS_Comm_Init(void)
{
	GPIO_InitTypeDef	GPIO_InitStructure;
	SPI_InitTypeDef		SPI_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);					//使能GPIOA时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);					//使能GPIOF时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);					//使能SPI1时钟
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;			//PB5~7复用功能输出	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;							//复用功能
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;							//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;						//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;							//上拉
	GPIO_Init(GPIOA, &GPIO_InitStructure);									//初始化
	
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
//	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource4,GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource5,GPIO_AF_SPI1); 					//PB5复用为 SPI1
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource6,GPIO_AF_SPI1); 					//PB6复用为 SPI1
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource7,GPIO_AF_SPI1); 					//PB7复用为 SPI1
	
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI1,ENABLE);						//复位SPI1
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI1,DISABLE);					//停止复位SPI1
	
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  	//设置SPI单向或者双向的数据模式:SPI设置为双线双向全双工
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;							//设置SPI工作模式:设置为主SPI
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;						//设置SPI的数据大小:SPI发送接收8位帧结构
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;								//串行同步时钟的空闲状态为高电平
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;							//串行同步时钟的第二个跳变沿（上升或下降）数据被采样
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;								//NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号有SSI位控制
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;	//定义波特率预分频的值:波特率预分频值为256
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;						//指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
	SPI_InitStructure.SPI_CRCPolynomial = 7;								//CRC值计算的多项式
	SPI_Init(SPI1, &SPI_InitStructure);  									//根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器

	
	SPI_Cmd(SPI1, ENABLE); 													//使能SPI外设
	

	
}

/**************************************************

函数名：MS_Comm_TIMode_Init
作  者：刘晓东
日  期：2018.7.19
版  本：V1.00
说  明：初始化SPI1接口
修改记录：

**************************************************/

void MS_Comm_TIMode_Init(void)
{
	GPIO_InitTypeDef	GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);					//使能GPIOA时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);					//使能GPIOF时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);					//使能SPI1时钟
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;			//PB5~7复用功能输出	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;							//复用功能
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;							//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;						//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;							//上拉
	GPIO_Init(GPIOA, &GPIO_InitStructure);									//初始化
	
	
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource4,GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource5,GPIO_AF_SPI1); 					//PB5复用为 SPI1
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource6,GPIO_AF_SPI1); 					//PB6复用为 SPI1
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource7,GPIO_AF_SPI1); 					//PB7复用为 SPI1
	
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI1,ENABLE);						//复位SPI1
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI1,DISABLE);					//停止复位SPI1
	
	
	SPI1->CR1 |= SPI_BaudRatePrescaler_256;
	SPI1->CR1 |= SPI_DataSize_8b;
	SPI_TIModeCmd(SPI1, ENABLE);
	SPI1->CR1 |= 0x0044;	
}

/**************************************************

函数名：MS_Comm_SetSpeed
作  者：刘晓东
日  期：2018.7.19
版  本：V1.00
说  明：配置SPI1的速度
		SPI速度 = fAPB2/分频系数
		APB2速度一般为84MHz
参	数：SPI_BaudRatePrescaler 分频系数
修改记录：

**************************************************/
void MS_Comm_SetSpeed(u8 SPI_BaudRatePrescaler)
{
	assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_BaudRatePrescaler));			//判断有效性
	SPI1->CR1&=0XFFC7;														//位3-5清零，用来设置波特率
	SPI1->CR1|=SPI_BaudRatePrescaler;										//设置SPI1速度 
	SPI_Cmd(SPI1,ENABLE); 													//使能SPI1
} 

/**************************************************

函数名：MS_Comm_WriteByte
作  者：刘晓东
日  期：2018.7.19
版  本：V1.00
说  明：发送/接收数据
参	数：TxData 待发送的数据
修改记录：

**************************************************/
s16 MS_Comm_WriteByte(u8 TxData)
{	
	u8 retry = 0;
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET)			//等待发送区空 	
	{
		if(++retry > 200)
			return RT_FAULT;
	}		
	SPI_I2S_SendData(SPI1, TxData); 										//通过外设SPI1发送一个byte数据	
	return RT_EOK;
}

/**************************************************

函数名：Master_ReadByte
作  者：刘晓东
日  期：2018.8.21
版  本：V1.00
说  明：接收数据
参	数：
修改记录：

**************************************************/
s16 Master_ReadByte(void)
{  
	u8 retry = 0;
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET)		//等待接收完一个byte			
	{
		if(++retry > 200)
			return RT_FAULT;
	}	
	return SPI_I2S_ReceiveData(SPI1); 										//返回通过SPI1最近接收的数据      
}

/**************************************************

函数名：Master_Send_Data
作  者：刘晓东
日  期：2018.8.21
版  本：V1.00
说  明：发送数据
参	数：dat	 	要发送数据的缓存区
		len		要发送数据的长度
修改记录：

**************************************************/
u8 Master_Send_Data(u8* dat, u8 len)
{
	u8 cnt = 0;
//	NSS_CS = 1;
	delay_us(5);
	for(cnt = 0; cnt < len; ++cnt)
	{
//		NSS_CS = 0;
		if(MS_Comm_WriteByte(*(dat+cnt)) == RT_FAULT)
		{
//			NSS_CS = 0;
			return RT_ERROR;
		}
#ifdef __DEBUG
//		rt_kprintf("%d\n", dat[cnt]);
#endif
		if(Master_ReadByte() == RT_FAULT)
		{
//			NSS_CS = 0;
			return RT_ERROR;
		}
//		NSS_CS = 1;
		delay_us(5);
	}
//	NSS_CS = 1;
	return RT_EOK;
}

/**************************************************

函数名：Master_Send_Data
作  者：刘晓东
日  期：2018.8.22
版  本：V1.00
说  明：发送数据
参	数：rev_buf	 	数据接收缓存
		len			接收的数据长度
修改记录：

**************************************************/
u8 Master_Rev_Data(u8* rev_buf, u8 len)
{
	u8 cnt = 0;
	s16 temp_val = 0;
//	NSS_CS = 0;
	for(cnt = 0; cnt < len; ++cnt)
	{
		if(MS_Comm_WriteByte(0x00) == RT_FAULT)
		{
//			NSS_CS = 1;
			return RT_ERROR;
		}
		temp_val = Master_ReadByte();
		if(temp_val == RT_FAULT)
		{
//			NSS_CS = 1;
			return RT_ERROR;
		}
		rev_buf[cnt] = temp_val;
		
#ifdef  __DEBUG
		rt_kprintf("%d\n",rev_buf[cnt]);
#endif
	}
//	NSS_CS = 1;
	return RT_EOK;
}




