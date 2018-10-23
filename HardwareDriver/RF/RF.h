#ifndef  __RF_H
#define  __RF_H

#include "stm32f4xx_conf.h"
#include "sys.h"
#include "rtthread.h"
#include "delay.h"
#include "WK2124.h"
#include "struct.h"



extern u8 RF_Rev_Buf[RF_Max_Len];


void RF_Init(void);
u8 RF_send(u8 *dat, u8 len);

#endif



