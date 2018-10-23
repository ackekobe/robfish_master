#ifndef __WK2124_H
#define __WK2124_H

#include "stm32f4xx_conf.h"
#include "sys.h"
#include "rtthread.h"
#include "delay.h"
#include "struct.h"

#define WK_RST		(PGout(3))			//WK2124复位管脚
#define WK_CS		(PBout(12))			//WK2124片选管脚
#define BAUD_NUM	(16)

typedef struct 							//串口参数结构体
{
	u32 BaudRate;
	u16 WordLength;
	u8 	StopBits;
	u8 	Parity;
	u8  interrupt_enable;
	u8 	rev_contact_val;
	u8 	send_contact_val;
} uartTydef;



typedef struct							//波特率与寄存器值对应表
{
	u32 baud_rate;
	u16 baud_reg_val;
} _baud_rate_def;

/*******************波特率定义表*********************/

#define BAUDRATE_9600		(9600)
#define BAUDRATE_115200		(115200)



/*********子串口地址分配表************/

#define  RF_UART1		((u8)0x00)		//RF串口
#define  BDRN_UART2		((u8)0x10)		//北斗RNSS串口
#define  BDRD_UART3		((u8)0x20)		//北斗RDSS串口
#define  BK_UART4		((u8)0x30)		//UART4 备用串口

											 
/*************控制命令头*****************/
											 
#define  WRITE_REG_HEAD			((u8)0x00)
#define  READ_REG_HEAD			((u8)0x40)
#define  WRITE_FIFO_HEAD		((u8)0x80)
#define  READ_FIFO_HEAD			((u8)0xCF)

#define REV_BIT					((u8)0x00)			//FIFO接收位
#define SEND_BIT				((u8)0x01)			//FIFO发送位

#define FIFO_SIZE				(256)
											 

/***************WK2124寄存器列表*******************/

//全局寄存器列表
#define  GENA		((u8)0x00)			//全局控制寄存器
#define  GRST		((u8)0x01)			//全局子串口复位控制寄存去
#define  GIER		((u8)0x10)			//全局中断寄存器
#define  GIFR		((u8)0x11)			//全局中断标志寄存器

//子串口控制寄存器

//SPAGE0
#define  SPAGE		((u8)0x03)			//子串口页控制寄存器
#define  SCR		((u8)0x04)			//子串口使能寄存器
#define	 LCR		((u8)0x05)			//子串口配置寄存器
#define  FCR		((u8)0x06)			//子串口FIFO控制寄存器
#define  SIER		((u8)0x07)			//子串口中断使能寄存器
#define  SIFR		((u8)0x08)			//子串口中断标志寄存器
#define  TFCNT		((u8)0x09)			//子串口发送FIFO计数寄存器
#define  RFCNT		((u8)0x0A)			//子串口接收FIFO计数寄存器
#define  FSR		((u8)0x0B)			//子串口FIFO状态寄存器
#define  LSR		((u8)0x0C)			//子串口接收状态寄存器
#define  FDAT		((u8)0x0D)			//子串口FIFO数据寄存器

//SPAGE1
#define  BAUD1		((u8)0x04)			//子串口波特率配置寄存器高字节
#define  BAUD0		((u8)0x05)			//子串口波特率配置寄存器低字节
#define  PRES		((u8)0x06)			//子串口波特率配置寄存器小数部分
#define  RFTL		((u8)0x07)			//子串口接收FIFO中断触发点配置寄存器
#define  TFTL		((u8)0x08)			//子串口发送FIFO中断触发点配置寄存器

/*******************子串口使能*************************/

#define 	WK2124_UT4EN	0x08
#define 	WK2124_UT3EN	0x04
#define 	WK2124_UT2EN	0x02
#define 	WK2124_UT1EN	0x01


/*******************子串口休眠与复位*************************/

#define 	WK2124_UT4SLEEP	0x80
#define 	WK2124_UT3SLEEP	0x40
#define 	WK2124_UT2SLEEP	0x20
#define 	WK2124_UT1SLEEP	0x10
#define 	WK2124_UT4RST	0x08
#define 	WK2124_UT3RST	0x04
#define 	WK2124_UT2RST	0x02
#define 	WK2124_UT1RST	0x01

/*******************全局中断*************************/

#define 	WK2124_UT4IE	0x08
#define 	WK2124_UT3IE	0x04
#define 	WK2124_UT2IE	0x02
#define 	WK2124_UT1IE	0x01


/*******************全局中断标志*********************/

#define 	WK2124_UT4INT	0x08
#define 	WK2124_UT3INT	0x04
#define 	WK2124_UT2INT	0x02
#define 	WK2124_UT1INT	0x01

/*******************页*********************/

#define 	WK2124_SPAGE0	0x00
#define 	WK2124_SPAGE1   0x01

/*******************子串口控制寄存器(SCR)*****************/

#define 	WK2124_SLEEPEN	0x04
#define 	WK2124_TXEN     0x02
#define 	WK2124_RXEN     0x01

/*****************子串口配置寄存器(LCR)*****************/

#define 	WK2124_BREAK		0x20
#define 	WK2124_NORMAL_OUT	0x00
#define 	WK2124_IREN     	0x10
#define 	WK2124_NORMAL     	0x10
#define 	WK2124_WORDLENGTH_9 0x08
#define 	WK2124_WORDLENGTH_8 0x00
#define 	WK2124_PAME     	0x04		//偶校验
#define 	WK2124_PAMO     	0x02		//奇校验
#define 	WK2124_PAMN     	0x00		//无校验
#define 	WK2124_STPL_2   	0x01
#define 	WK2124_STPL_1   	0x00

/*****************子串口FIFO控制寄存器(FCR)*****************/

#define		WK2124_TFEN			0x08
#define		WK2124_RFEN			0x04

/************子串口中断使能寄存器(SIER)*****************/

#define 	WK2124_FERR_IEN      0x80
#define 	WK2124_CTS_IEN       0x40
#define 	WK2124_RTS_IEN       0x20
#define 	WK2124_XOFF_IEN      0x10
#define 	WK2124_TFEMPTY_IEN   0x08
#define 	WK2124_TFTRIG_IEN    0x04
#define 	WK2124_RXOUT_IEN     0x02
#define 	WK2124_RFTRIG_IEN    0x01

/************子串口中断标志寄存器(SIFR)*****************/

#define 	WK2124_FERR_INT      0x80
#define 	WK2124_CTS_INT       0x40
#define 	WK2124_RTS_INT       0x20
#define 	WK2124_XOFF_INT      0x10
#define 	WK2124_TFEMPTY_INT   0x08
#define 	WK2124_TFTRIG_INT    0x04
#define 	WK2124_RXOVT_INT     0x02
#define 	WK2124_RFTRIG_INT    0x01


/************子串口FIFO状态寄存器(FSR)*****************/

#define 	WK2124_RFOE     0x80
#define 	WK2124_RFBI     0x40
#define 	WK2124_RFFE     0x20
#define 	WK2124_RFPE     0x10
#define 	WK2124_RDAT     0x08
#define 	WK2124_TDAT     0x04
#define 	WK2124_TFULL    0x02
#define 	WK2124_TBUSY    0x01

/************子串口接收状态寄存器(LSR)*****************/

#define 	WK2124_OE       0x08
#define 	WK2124_BI       0x04
#define 	WK2124_FE       0x02
#define 	WK2124_PE       0x01

#define 	WK2124_Rev_Done		(1)		//wk2124接收到数据
#define		WK2124_Rev_Undo		(0)		//WK2124未接收到数据			

extern u8 WK2124_Rev_Flag[4];


void WK2124_RST (void);
void WK2124_SPI2_SetSpeed(u8 SPI_BaudRatePrescaler);
u8 WK2124_ReadWriteByte(u8 TxData);
void WK2124_Init(void);
u8 WK2124_write_reg(u8 reg, u8 data);
u8 WK2124_read_reg(u8 reg, u8 *data);
u8 WK2124_FIFO_RST(u8 uart_addr, u8 rev_send);
u8 WK2124_write_FIFO(u8 uart_addr, const u8 *data, u16 length);
u8 WK2124_read_FIFO(u8 uart_addr, u8 *data);
void WK2124_UART_Config(u8 uart_addr, uartTydef *uartParam);
u8 WK2124_USART_Ctr(u8 num, u8 stat);

#endif


