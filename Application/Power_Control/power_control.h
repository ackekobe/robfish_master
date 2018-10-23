#ifndef __POWER_CONTROL_h
#define __POWER_CONTROL_h

#include "stm32f4xx_conf.h"
#include "sys.h"
#include "rtthread.h"
#include "delay.h"

#define  POWER_24V		(PFout(6))
#define  POWER_48V		(PFout(8))

#define  POWER_ON		(1)
#define  POWER_OFF		(0)

void power_control_init(void);

#endif

