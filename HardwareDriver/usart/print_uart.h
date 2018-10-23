#ifndef __USART_H
#define __USART_H

#include "stm32f4xx_conf.h"
#include "sys.h" 
#include <rthw.h>
#include "rtthread.h"					 

#define FISH_PARAM_LEN		(16)

void uart4_init(u32 bound);
#endif


