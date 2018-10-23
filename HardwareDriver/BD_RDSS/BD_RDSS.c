/**************************************************

文件名：BD_RDSS.c
作  者：刘晓东
日  期：2018.7.30
版  本：V1.00
说  明：北斗模块RDSS的初始化文件
修改记录：

**************************************************/

#include "BD_RDSS.h"
#include <string.h>

char RDSS_TX_buf[UPPER_MAX_LEN];
u8	 RDSS_REV_buf[UPPER_MAX_LEN];
u32  local_addr = 0;

const u8 ICJC[12] = {0x24,0x49,0x43,0x4A,0x43,0x00,0x0C,0x00,0x00,0x00,0x00,0x2B};			//IC检测时发送的报文
const u8 XTZJ[13] = {0x24,0x58,0x54,0x5A,0x4A,0x00,0x0D,0x00,0x00,0x00,0x00,0x00,0x35};		//自检检测时发送的报文



/**************************************************

函数名：BD_RDSS_Init
作  者：刘晓东
日  期：2018.7.30
版  本：V1.00
说  明：配置RDSS串口参数
		
参  数：
修改记录：

**************************************************/

void BD_RDSS_Init(void)
{
	GPIO_InitTypeDef	GPIO_InitStructure;
	uartTydef uartParaStructure;
	u8 GIER_reg, FCR_reg;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);					//使能GPIOB时钟
	
	uartParaStructure.BaudRate = BAUDRATE_115200;			//RNSS波特率115200
	uartParaStructure.Parity = WK2124_PAMN;					//无校验
	uartParaStructure.StopBits = WK2124_STPL_1;				//1个停止位
	uartParaStructure.WordLength = WK2124_WORDLENGTH_8;		//8位字长
	uartParaStructure.rev_contact_val = 0xFF;				//触点门槛值
	uartParaStructure.send_contact_val = 0xFF;
	uartParaStructure.interrupt_enable = WK2124_RXOUT_IEN;  //接收超时中断
	WK2124_UART_Config(BDRD_UART3, &uartParaStructure);
	
	WK2124_read_reg(GIER, &GIER_reg);
	WK2124_write_reg(GIER, (u8)(1<<(BDRD_UART3>>4)) | GIER_reg);					//打开串口中断
	
	WK2124_read_reg(FCR | BDRD_UART3, &FCR_reg);
	WK2124_write_reg(FCR | BDRD_UART3, 0x03 | FCR_reg);					//复位FIFO
	
	
	
	/************PD8推挽输出，RDSS串口发送使能管脚**********/ 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8; 										//GPIOD8
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;									//输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;								//速度2MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 									//推挽输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 									//上拉
	GPIO_Init(GPIOD,&GPIO_InitStructure); 											//初始化PD8
	
	BD_RDSS_Tx = 0;																	//默认为接收
}

/**************************************************

函数名：BD_RDSS_Send
作  者：刘晓东
日  期：2018.7.30
版  本：V1.00
说  明：RDSS串口发送函数		
参  数：data 发送的数据
		len	发送的数据长度
返回值：发送数据字节数
修改记录：

**************************************************/

u8 BD_RDSS_Send(const u8 *data, u8 len)
{
	u8 longth = 0;										//发送数据长度
	BD_RDSS_Tx = 1;										//使能RDSS发送
	
	longth = WK2124_write_FIFO(BDRD_UART3, data, len);	//发送数据
	delay_us(1000);										//spi速度快，串口速度慢，需要延时，等待串口发送完毕
	delay_us(1000);
	BD_RDSS_Tx = 0;
	return longth;
}


/**************************************************

函数名：BD_RDSS_TXSQ
作  者：刘晓东
日  期：2018.7.30
版  本：V1.00
说  明：RDSS通信申请组帧		
参  数：local_addr 	本地地址
		rev_addr   	发送地址
		ack			是否应答
		data		发送数据
		len			数据长度
返回值：
修改记录：

**************************************************/

void BD_RDSS_TXSQ(u32 local_addr, u32 rev_addr, u8 ack, char *data, u8 len)
{
	u8 cnt = 0;
	u8 xor_val = '$';
	rt_memset(RDSS_TX_buf, 0x00, UPPER_MAX_LEN);
	strcpy(RDSS_TX_buf, "$TXSQ");		//发送指令头
	
	RDSS_TX_buf[5] = (len + 18) >>8;   	//报文帧长度高字节
	RDSS_TX_buf[6] = len + 18;   		//低字节
	
	RDSS_TX_buf[7] = (local_addr) >> 16;   	//本地地址高字节
	RDSS_TX_buf[8] = (local_addr) >> 8;   		
	RDSS_TX_buf[9] = local_addr;
	RDSS_TX_buf[10] = 0x04;					//信息类别
	
	RDSS_TX_buf[11] = (rev_addr) >> 16;   	//接收方地址高字节
	RDSS_TX_buf[12] = (rev_addr) >> 8;   		
	RDSS_TX_buf[13] = rev_addr;
	
	RDSS_TX_buf[14] = (len*8)>>8;			//电文长度以比特位单位
	RDSS_TX_buf[15] = len*8;
	
	RDSS_TX_buf[16] = 0x01;
	
	strcpy(RDSS_TX_buf + 17, data);			//数据放进发送缓冲区
	
	for(cnt = 1; cnt < len + 17; ++cnt)		//字节异或
	{
		xor_val ^= RDSS_TX_buf[cnt];
	}
	RDSS_TX_buf[17+len] = xor_val;
}






