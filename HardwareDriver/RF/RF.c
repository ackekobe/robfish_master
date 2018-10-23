/**************************************************

文件名：RF.c
作  者：刘晓东
日  期：2018.8.28
版  本：V1.00
说  明：RF模块初始化文件
修改记录：

**************************************************/
#include "RF.h"

u8 RF_Rev_Buf[RF_Max_Len];		//RF接收BUF


/**************************************************

函数名：RF_Init
作  者：刘晓东
日  期：2018.8.28
版  本：V1.00
说  明：配置RF串口参数
参  数：
修改记录：

**************************************************/

void RF_Init(void)
{
	uartTydef uartParaStructure;
	u8 GIER_reg;
	
	
	uartParaStructure.BaudRate = BAUDRATE_9600;			//RNSS波特率9600
	uartParaStructure.Parity = WK2124_PAMN;				//无校验
	uartParaStructure.StopBits = WK2124_STPL_1;			//1个停止位
	uartParaStructure.WordLength = WK2124_WORDLENGTH_8;	//8位字长
	uartParaStructure.rev_contact_val = CONTACT_VAL;	//触点门槛值
	uartParaStructure.send_contact_val = CONTACT_VAL;
	uartParaStructure.interrupt_enable = WK2124_RXOUT_IEN;   //接收触点中断 接收超时中断
	WK2124_UART_Config(RF_UART1, &uartParaStructure);
	
	
	WK2124_read_reg(GIER, &GIER_reg);
	WK2124_write_reg(GIER, (u8)(1<<(RF_UART1>>4)) | GIER_reg);					//打开串口中断

}

/**************************************************

函数名：RF_send
作  者：刘晓东
日  期：2018.8.29
版  本：V1.00
说  明：RF发送数据
参  数：dat  待发送数据
		len  数据长度
返回值：发送的字节数
修改记录：

**************************************************/
u8 RF_send(u8 *dat, u8 len)
{
	u8 longth = 0;										//发送数据长度
	
	longth = WK2124_write_FIFO(RF_UART1, dat, len);		//发送数据
	delay_us(1000);										//spi速度快，串口速度慢，需要延时，等待串口发送完毕
	delay_us(1000);
	return longth;
}



