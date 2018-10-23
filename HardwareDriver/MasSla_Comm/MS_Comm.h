#ifndef __MS_COMM_
#define __MS_COMM_

#include "stm32f4xx_conf.h"
#include "sys.h"
#include "rtthread.h"
#include "delay.h"

#define  NSS_CS					(PAout(4))
#define  CMD_Timing				(0x01)
#define  CMD_Motor_Running		(0x02)
#define  CMD_Slave_Stat			(0x03)


typedef enum
{
	OVR_ERROR = 1,			//SPI…œ“Á¥ÌŒÛ
	MODF_ERROR,				//ƒ£ Ωπ ’œ¥ÌŒÛ
	CRC_ERROR,				//CRC¥ÌŒÛ
} _SPI_ERROR_STAT;

extern u8 slave_send_flag;

void MS_Comm_Init(void);
void MS_Comm_SetSpeed(u8 SPI_BaudRatePrescaler);
s16 MS_Comm_WriteByte(u8 TxData);
s16 Master_ReadByte(void);
u8 Master_Send_Data(u8* dat, u8 len);
u8 Master_Rev_Data(u8* rev_buf, u8 len);

#endif





