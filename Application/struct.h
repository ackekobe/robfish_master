#ifndef __STRUCT_H
#define __STRUCT_H

#include "stm32f4xx_conf.h"
#include "sys.h"


#define UPPER_MAX_LEN			(78)	//RDSS�շ�����ֽ���,RF�շ�����ֽ���Ϊ62
#define  RF_Max_Len				(62)	//RF�շ����ݵ���󳤶�
#define  LOCA_INFO_LEN			(512)	//����RNSS��λ��Ϣ���ݳ���

#define	CONTACT_VAL				(200)	//WK2124��RNSS����FIFO�Ĵ����ż�ֵ
typedef enum 						//WK2124�Ĵ������
{
	RF_UART = 0,
	RNSS_UART,
	RDSS_UART,
	Reserve_UART,
} wk2124_uart;


#define  WDG_ENABLE				(0)		//���Ź�ʹ��
#define  WDG_DISABLE			(0x55)	//���Ź���ʹ��

typedef enum 
{
	motor_mode_wz = 1,				//λ��ģʽ
	motor_mode_sd,					//�ٶ�ģʽ
	motor_mode_dd,					//�ϵ�ģʽ
	motor_mode_tz,					//ֹͣ���ϵ�ģʽ
	motor_mode_hl,					//����ģʽ(ÿ��ðˮʱ���Ѱ��)
} _motor_mode;



typedef struct				//�����������
{
	u8 		run_mode;			//����ģʽ
	s8 		flap_amplitude;		//�Ķ�����
	u8 		flap_fre_integer;	//�Ķ�Ƶ����������
	u8 		flap_fre_decimal;	//�Ķ�Ƶ��С�����֣�С����������100�����д���
	s16 	target_location;	//Ŀ��λ��
	
} _move_motor;

typedef struct				//�ͱõ������
{
	u8 	run_mode;			//����ģʽ
	float target_location;	//Ŀ��λ��
	float target_speed;		//Ŀ���ٶ�
} _oil_pumb_motor;

typedef struct				//���ĵ������
{
	u8 	run_mode;			//����ģʽ
	s16 target_speed;		//Ŀ���ٶ�
	float target_location;	//Ŀ��λ��
} _gravity_motor;

#pragma pack(2)
typedef struct				//������������ʱ��
{
	u16 year;
	u8 month;
	u8 day;
	u8 hour;
	u8 min;
	u8 sec;
	u16 msec;
} _time;
#pragma pack()

typedef struct				//��������λ��
{
	u8 		N_S;			//��γ��γ
	float	latitude;		//γ��
	u8		E_W;			//��������
	float	longitude;		//����
	float	altitude;		//���θ߶�
}_location;

	
typedef struct 
{
	u8			data_state;		//��λ������Чλ  1����Ч  0����Ч
	float 		speed;			//�Ե��ٶ�
	float		azimuth_angle;	//��λ��
	_time 		time;				
	_location	location;		//λ����Ϣ
	
} _BD_RNSS_data;





#endif


