#ifndef __AHRS_H
#define __AHRS_H

#include "stm32f4xx_conf.h"
#include "sys.h"
#include "rtthread.h"
#include "delay.h"

#define USART1_BUFSIZE 	(70)		//AHRS�Ľ������ݻ�������С
#define AHRS_DATA_LEN	(14)		//AHRS����������ÿһ�����ݵĳ���
#define AHRS_DMA_LEN	(100)

#define AHRS_Rev_Done	(1)			//AHRS�����������
#define AHRS_Rev_Undo	(0)			//AHRSδ��������

//#define AHRS_TX_EN		(PCout(4))

extern u8 USART1_RxBuf[USART1_BUFSIZE];
extern u8 USART1_TxBuf[USART1_BUFSIZE];

extern u8 attitude_data[AHRS_DATA_LEN];	//��̬����
extern u8 angleRate_data[AHRS_DATA_LEN];	//���ٶ�����
extern u8 accRate_data[AHRS_DATA_LEN];		//���ٶ�����



typedef struct
{
	float roll;
	float pitch;		//��б��
	float yaw;			//ƫ����
}  _attitude;			//��̬���ݽṹ��

typedef struct
{
	float angRate_x;		
	float angRate_y;		
	float angRate_z;			
}  _angRate;			//���ٶȽṹ��

typedef struct
{
	float acc_x;		
	float acc_y;		
	float acc_z;			
}  _accRate;			//���ٶȽṹ��

typedef struct
{
	float vel_x;		
	float vel_y;		
	float vel_z;			
}  _velocity;			//���ٶȽṹ��

extern u16 USART1_RX_STA;
extern _attitude attitude;
extern _angRate angRate;
extern _accRate accRate;
extern _velocity velocity;
extern const float pi;

extern const u8 close_stream[11];
extern const u8 polling_data[10];

void AHRS_Require(void);
void AHRS_Tx_Enable(const u8 *data, u16 ndtr);
void AHRS_Init(u32 bound);
void AHRS_data_analys(void);

#endif





