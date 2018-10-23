#ifndef __WK2124_H
#define __WK2124_H

#include "stm32f4xx_conf.h"
#include "sys.h"
#include "rtthread.h"
#include "delay.h"
#include "struct.h"

#define WK_RST		(PGout(3))			//WK2124��λ�ܽ�
#define WK_CS		(PBout(12))			//WK2124Ƭѡ�ܽ�
#define BAUD_NUM	(16)

typedef struct 							//���ڲ����ṹ��
{
	u32 BaudRate;
	u16 WordLength;
	u8 	StopBits;
	u8 	Parity;
	u8  interrupt_enable;
	u8 	rev_contact_val;
	u8 	send_contact_val;
} uartTydef;



typedef struct							//��������Ĵ���ֵ��Ӧ��
{
	u32 baud_rate;
	u16 baud_reg_val;
} _baud_rate_def;

/*******************�����ʶ����*********************/

#define BAUDRATE_9600		(9600)
#define BAUDRATE_115200		(115200)



/*********�Ӵ��ڵ�ַ�����************/

#define  RF_UART1		((u8)0x00)		//RF����
#define  BDRN_UART2		((u8)0x10)		//����RNSS����
#define  BDRD_UART3		((u8)0x20)		//����RDSS����
#define  BK_UART4		((u8)0x30)		//UART4 ���ô���

											 
/*************��������ͷ*****************/
											 
#define  WRITE_REG_HEAD			((u8)0x00)
#define  READ_REG_HEAD			((u8)0x40)
#define  WRITE_FIFO_HEAD		((u8)0x80)
#define  READ_FIFO_HEAD			((u8)0xCF)

#define REV_BIT					((u8)0x00)			//FIFO����λ
#define SEND_BIT				((u8)0x01)			//FIFO����λ

#define FIFO_SIZE				(256)
											 

/***************WK2124�Ĵ����б�*******************/

//ȫ�ּĴ����б�
#define  GENA		((u8)0x00)			//ȫ�ֿ��ƼĴ���
#define  GRST		((u8)0x01)			//ȫ���Ӵ��ڸ�λ���ƼĴ�ȥ
#define  GIER		((u8)0x10)			//ȫ���жϼĴ���
#define  GIFR		((u8)0x11)			//ȫ���жϱ�־�Ĵ���

//�Ӵ��ڿ��ƼĴ���

//SPAGE0
#define  SPAGE		((u8)0x03)			//�Ӵ���ҳ���ƼĴ���
#define  SCR		((u8)0x04)			//�Ӵ���ʹ�ܼĴ���
#define	 LCR		((u8)0x05)			//�Ӵ������üĴ���
#define  FCR		((u8)0x06)			//�Ӵ���FIFO���ƼĴ���
#define  SIER		((u8)0x07)			//�Ӵ����ж�ʹ�ܼĴ���
#define  SIFR		((u8)0x08)			//�Ӵ����жϱ�־�Ĵ���
#define  TFCNT		((u8)0x09)			//�Ӵ��ڷ���FIFO�����Ĵ���
#define  RFCNT		((u8)0x0A)			//�Ӵ��ڽ���FIFO�����Ĵ���
#define  FSR		((u8)0x0B)			//�Ӵ���FIFO״̬�Ĵ���
#define  LSR		((u8)0x0C)			//�Ӵ��ڽ���״̬�Ĵ���
#define  FDAT		((u8)0x0D)			//�Ӵ���FIFO���ݼĴ���

//SPAGE1
#define  BAUD1		((u8)0x04)			//�Ӵ��ڲ��������üĴ������ֽ�
#define  BAUD0		((u8)0x05)			//�Ӵ��ڲ��������üĴ������ֽ�
#define  PRES		((u8)0x06)			//�Ӵ��ڲ��������üĴ���С������
#define  RFTL		((u8)0x07)			//�Ӵ��ڽ���FIFO�жϴ��������üĴ���
#define  TFTL		((u8)0x08)			//�Ӵ��ڷ���FIFO�жϴ��������üĴ���

/*******************�Ӵ���ʹ��*************************/

#define 	WK2124_UT4EN	0x08
#define 	WK2124_UT3EN	0x04
#define 	WK2124_UT2EN	0x02
#define 	WK2124_UT1EN	0x01


/*******************�Ӵ��������븴λ*************************/

#define 	WK2124_UT4SLEEP	0x80
#define 	WK2124_UT3SLEEP	0x40
#define 	WK2124_UT2SLEEP	0x20
#define 	WK2124_UT1SLEEP	0x10
#define 	WK2124_UT4RST	0x08
#define 	WK2124_UT3RST	0x04
#define 	WK2124_UT2RST	0x02
#define 	WK2124_UT1RST	0x01

/*******************ȫ���ж�*************************/

#define 	WK2124_UT4IE	0x08
#define 	WK2124_UT3IE	0x04
#define 	WK2124_UT2IE	0x02
#define 	WK2124_UT1IE	0x01


/*******************ȫ���жϱ�־*********************/

#define 	WK2124_UT4INT	0x08
#define 	WK2124_UT3INT	0x04
#define 	WK2124_UT2INT	0x02
#define 	WK2124_UT1INT	0x01

/*******************ҳ*********************/

#define 	WK2124_SPAGE0	0x00
#define 	WK2124_SPAGE1   0x01

/*******************�Ӵ��ڿ��ƼĴ���(SCR)*****************/

#define 	WK2124_SLEEPEN	0x04
#define 	WK2124_TXEN     0x02
#define 	WK2124_RXEN     0x01

/*****************�Ӵ������üĴ���(LCR)*****************/

#define 	WK2124_BREAK		0x20
#define 	WK2124_NORMAL_OUT	0x00
#define 	WK2124_IREN     	0x10
#define 	WK2124_NORMAL     	0x10
#define 	WK2124_WORDLENGTH_9 0x08
#define 	WK2124_WORDLENGTH_8 0x00
#define 	WK2124_PAME     	0x04		//żУ��
#define 	WK2124_PAMO     	0x02		//��У��
#define 	WK2124_PAMN     	0x00		//��У��
#define 	WK2124_STPL_2   	0x01
#define 	WK2124_STPL_1   	0x00

/*****************�Ӵ���FIFO���ƼĴ���(FCR)*****************/

#define		WK2124_TFEN			0x08
#define		WK2124_RFEN			0x04

/************�Ӵ����ж�ʹ�ܼĴ���(SIER)*****************/

#define 	WK2124_FERR_IEN      0x80
#define 	WK2124_CTS_IEN       0x40
#define 	WK2124_RTS_IEN       0x20
#define 	WK2124_XOFF_IEN      0x10
#define 	WK2124_TFEMPTY_IEN   0x08
#define 	WK2124_TFTRIG_IEN    0x04
#define 	WK2124_RXOUT_IEN     0x02
#define 	WK2124_RFTRIG_IEN    0x01

/************�Ӵ����жϱ�־�Ĵ���(SIFR)*****************/

#define 	WK2124_FERR_INT      0x80
#define 	WK2124_CTS_INT       0x40
#define 	WK2124_RTS_INT       0x20
#define 	WK2124_XOFF_INT      0x10
#define 	WK2124_TFEMPTY_INT   0x08
#define 	WK2124_TFTRIG_INT    0x04
#define 	WK2124_RXOVT_INT     0x02
#define 	WK2124_RFTRIG_INT    0x01


/************�Ӵ���FIFO״̬�Ĵ���(FSR)*****************/

#define 	WK2124_RFOE     0x80
#define 	WK2124_RFBI     0x40
#define 	WK2124_RFFE     0x20
#define 	WK2124_RFPE     0x10
#define 	WK2124_RDAT     0x08
#define 	WK2124_TDAT     0x04
#define 	WK2124_TFULL    0x02
#define 	WK2124_TBUSY    0x01

/************�Ӵ��ڽ���״̬�Ĵ���(LSR)*****************/

#define 	WK2124_OE       0x08
#define 	WK2124_BI       0x04
#define 	WK2124_FE       0x02
#define 	WK2124_PE       0x01

#define 	WK2124_Rev_Done		(1)		//wk2124���յ�����
#define		WK2124_Rev_Undo		(0)		//WK2124δ���յ�����			

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


