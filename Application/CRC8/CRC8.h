#ifndef __CRC8_H
#define __CRC8_H

#include "stm32f4xx_conf.h"
#include "sys.h"
#include "rtthread.h"
#include "delay.h"

u8 CRC8_Table(u8 *p, u8 len);

#endif


