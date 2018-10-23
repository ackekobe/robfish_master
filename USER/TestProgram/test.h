#ifndef __TEST_H
#define __TEST_H


#include "stm32f4xx_conf.h"
#include "sys.h"
#include "AHRS.h"
#include "altimeter.h"
#include "delay.h"
#include "struct.h"
#include "CTD48M.h"
#include "RTC.h"
#include <stdio.h>
#include "ff.h"
#include "SD.h"
#include "exfuns.h"
#include "fattester.h"
#include "BD_RNSS.h"
#include "BD_RDSS.h"
#include "sonar.h"

#include "MS_Com_Process.h"
#include "Upper_Comm.h"
#include "power_control.h"
#include "navigation.h"

#define TEST_FLAG		(1)			//ÊÇ·ñ²âÊÔ£¬1£º²âÊÔ	0£º²»²âÊÔ
#define loop(x,y,z)	(((x) >= (z)) ? ((x)=(y) : x++))


#if TEST_FLAG

void Test_AHRS_USART1(void);
void Test_altimeter_USART2(void);
void RTC_test(void);
void SD_test(void);
u8 Test_SPI_Comm_Master(void);
void Master_Slave_Test(void);
void Upper_Master_Test(void);
void Test_CTD48M_USART3(void);
void Test_Sonar_USART6(void);

void RNSS_test(void);
void RDSS_test(void);
void RF_test(void);
void AHRS_calc_CRC(void);
void navi_location_test(void);

#endif
#endif


