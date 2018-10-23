#ifndef  __MS_COM_PROCESS_H
#define  __MS_COM_PROCESS_H


#include "stm32f4xx_conf.h"
#include "sys.h"
#include "rtthread.h"
#include "delay.h"
#include "struct.h"
#include "MS_Comm.h"
#include "CRC8.h"


#define  slave_stat_bytes		(28)

typedef enum _cmd				//���ӻ�ͨ��������
{
	XTJC = 0x01,				//ϵͳ�Լ�
	ZTSB = 0x02,				//״̬�ϱ�
	XTTZ = 0x03,				//ϵͳֹͣ
	XTJS = 0x04,				//ϵͳУʱ
	XTFW = 0x05,				//ϵͳ��λ   //����ϵͳ��λ
	
	YTSF = 0x10,				//�����ϸ�
	YTXQ = 0x11,				//������Ǳ
	YTZZ = 0x12,				//������ת
	YTYZ = 0x13,				//������ת
	YTHT = 0x14,				//�������
	YTHX = 0x15,				//���廬��
	YTKT = 0x16,				//��������ƽ�
	YTHL = 0x17,				//�������
	YTZP = 0x18,				//�����Զ���ƽ
	
	HLGH = 0x20,				//��·�滮
	HLCR = 0x21,				//���˶��õ�ָ��λ��
#ifdef  __DEBUG
	DJXQL  = 0x30,				//���������
	DJXQR  = 0x31,				//���������
	DJZX   = 0x32,				//���ĵ��
	DJYB   = 0x33,				//�������
	DJWB   = 0x34,				//β�����
	DJYG   = 0x35,				//�͸׵��
#endif

	XTZT   = 0x80,				//����ϵͳ״̬
	HJXX   = 0x90,				//���ͺ�����Ϣ	
} cmd;



typedef struct
{
	u8  left_pertoral;
	u8  right_pertoral;
	u8  oil_motor;
	u8  gravity_motor;
	u8  waist_motor;
	u8  tail_motor;
	u8  power_vol;
	u8  power_cur;
	u8  power_temp;
	u8  cabin1_temp;
	u8  cabin2_temp;
	u8  cabin3_temp;
	u8  cabin4_temp;
	u8  sys_master_stat;
	u8  sys_slave_stat;
	u32 sys_fault;
	float power_quanlity;
	float power_fault;
} _slave_stat;



extern u8 slave_stat_buf[28];
extern _move_motor  left_pectoral;	//����������
extern _move_motor	 right_pectoral;	//������
extern _oil_pumb_motor  oil_pumb_motor;		//�ͱõ������
extern _gravity_motor 	 gravity_motor;		//���ĵ������
extern _move_motor	 waist_motor;		//�����������	
extern _move_motor		 tail_motor;		//β���������


u8 Master_Slave_Comm(u8 CMD_ID);



#endif

