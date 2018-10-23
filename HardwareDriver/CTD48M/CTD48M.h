#ifndef __CTD48M_H
#define __CTD48M_H

#include "stm32f4xx_conf.h"
#include "sys.h"
#include "rtthread.h"
#include "delay.h"


#define CTD_BUF_LEN		(32)		//CTD���ջ��泤��
#define	DEPTH_ADDR		(0x10)		//ѹ��(���)���ݵ�ַ
#define	TEMP_ADDR		(0x18)		//�¶����ݵ�ַ
#define	COND_ADDR		(0x20)		//�ζ����ݵ�ַ

#define CTD_Power		(PFout(9))

#define CTM48_Rev_Done	(1)			//����������������
#define CTM48_Rev_Undo	(0)			//������δ��������

#define CTM48_ON	(1)			//�����
#define CTM48_OFF	(0)			//�������

typedef struct					//������ֵ
{
	float depth_cal_val;		
	float temp_cal_val;		
	float cond_cal_val;
} _Cal_Val;

typedef struct					//����rawֵ
{
	u16 press_sam_val;
	u16 temp_sam_val;
	u16 cond_sam_val;
} _sample_val;


extern u8 CTD_RxBuf[CTD_BUF_LEN];
extern u16 USART3_RX_STA;

extern _Cal_Val CTD_Final_Val;

void CTD_Init(u32 bound);
u8 CTD_Data_Trans(u8* data);
void CTD_send(u8 *buf, u8 len);
void CTD_Rx_Enable(void);

#endif

