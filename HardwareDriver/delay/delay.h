#ifndef __DELAY_H
#define __DELAY_H

#include "stm32f4xx_conf.h"
#include "sys.h"
#include "stddef.h"										//Ϊ��ʹ��NULL����������ļ�
#include <sys.h>

#if SYSTEM_SUPPORT_OS
#include "rtthread.h"									//֧��OSʱ��ʹ��
#endif
	  
//////////////////////////////////////////////////////////////////////////////////  
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F407������
//ʹ��SysTick����ͨ����ģʽ���ӳٽ��й���(֧��ucosii)
//����delay_us,delay_ms
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2014/5/2
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved
//********************************************************************************
//�޸�˵��
//��
////////////////////////////////////////////////////////////////////////////////// 	 
void delay_init(void);
void delay_ms(u32 nms);
void delay_us(u32 nus);

#endif





























