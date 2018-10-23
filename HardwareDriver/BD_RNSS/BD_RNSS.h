#ifndef __BD_RNSS_H
#define __BD_RNSS_H

#include "stm32f4xx_conf.h"
#include "sys.h"
#include "rtthread.h"
#include "delay.h"
#include "struct.h"
#include "WK2124.h"




#define BD_RNSS_Tx			(PGout(1))
#define BD_Power_En			(PCout(2))



extern struct rt_semaphore RNSS_rev_sem;
extern char BD_raw_data[LOCA_INFO_LEN];
extern u8 day_per_month[12];


u8 isLeap(u16 year);
void BD_RNSS_Init(void);
void RNSS_start(void);
void BD_data_trans(char *BD_raw_data, _BD_RNSS_data *trans_result);



#endif



