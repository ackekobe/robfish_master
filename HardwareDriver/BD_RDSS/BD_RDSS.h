#ifndef __BD_RDSS_H
#define __BD_RDSS_H

#include "stm32f4xx_conf.h"
#include "sys.h"
#include "rtthread.h"
#include "delay.h"
#include "WK2124.h"



#define  BD_RDSS_Tx		(PDout(8))

extern char RDSS_TX_buf[UPPER_MAX_LEN];
extern u8	 RDSS_REV_buf[UPPER_MAX_LEN];
extern const u8 ICJC[12];
extern const u8 XTZJ[13];



void BD_RDSS_Init(void);
u8 BD_RDSS_Send(const u8 *data, u8 len);
void BD_RDSS_TXSQ(u32 local_addr, u32 rev_addr, u8 ack, char *data, u8 len);

#endif


