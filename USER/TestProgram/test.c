/**************************************************

文件名：test.c
作  者：刘晓东
日  期：2018.8.15
版  本：V1.00
说  明：各个外设测试代码
修改记录：

**************************************************/

#include "test.h"

#if TEST_FLAG
extern u8 ahrs_rev_flag;
extern u8 altimer_rev_flag;
extern u8 CTD48M_rev_flag;

extern u8 fish_param_flag;
extern u8 fish_param[14];
extern u16 UART4_RX_STA;

RTC_TimeTypeDef time;
RTC_DateTypeDef date;

extern USBH_HOST  USB_Host;
extern USB_OTG_CORE_HANDLE  USB_OTG_Core;

u8 motor_test[17];			//电机测试过程中指令发送缓存
/**************************************************

函数名：Test_AHRS_USART1()
作  者：刘晓东
日  期：2018.8.18
版  本：V1.00
说  明：惯导测试程序
修改记录：

**************************************************/
void Test_AHRS_USART1(void)
{
	u8 buf[4][64];
	u8 timestamp[26];
	if(ahrs_rev_flag == AHRS_Rev_Done)
	{
		AHRS_data_analys();
//		RTC_GetTime(RTC_Format_BIN, &time);
//		RTC_GetDate(RTC_Format_BIN, &date);
//		sprintf((char*)buf[0],"AHRS attutide:roll %8.4f,pitch %8.4f,yaw %8.4f\r\n",attitude.roll,attitude.pitch,attitude.yaw);
//		sprintf((char*)buf[1],"AHRS angRate:x %8.4f,y %8.4f,z %8.4f\r\n",angRate.angRate_x,angRate.angRate_y,angRate.angRate_z);
//		sprintf((char*)buf[2],"AHRS accRate:x %8.4f,y %8.4f,z %8.4f\r\n",accRate.acc_x,accRate.acc_y,accRate.acc_z);
		sprintf((char*)buf[3],"AHRS velocity:x %8.4f,y %8.4f,z %8.4f\r\n",velocity.vel_x,velocity.vel_y,velocity.vel_z);
//		sprintf((char*)timestamp, "time:20%02d-%02d-%02d %02d:%02d:%02d\r\n",date.RTC_Year,date.RTC_Month,date.RTC_Date,time.RTC_Hours,time.RTC_Minutes,time.RTC_Seconds);
//		RF_send(timestamp, 26);
//		delay_ms(1000);
//		RF_send(buf[0],57);
//		delay_ms(1000);
//		RF_send(buf[1],47);
//		delay_ms(1000);
//		RF_send(buf[2],47);
//		delay_ms(1000);
		rt_kprintf((const char*)buf[3]);
//		RF_send(buf[3],48);
		ahrs_rev_flag = AHRS_Rev_Undo;
	}
	AHRS_Require();
}



/**************************************************

函数名：Test_altimeter_USART2()
作  者：刘晓东
日  期：2018.8.18
版  本：V1.00
说  明：高度计测试程序
修改记录：

**************************************************/
void Test_altimeter_USART2(void)
{
	u8 buf[40] = {0};

	if(altimer_rev_flag == ALTIMETER_Rev_Done)
	{		
		altitude_analy();
		RTC_GetTime(RTC_Format_BIN, &time);
		RTC_GetDate(RTC_Format_BIN, &date);
		sprintf((char*)buf, "height:%6.3fm 20%02d-%02d-%02d %02d:%02d:%02d\r\n",fish_altitude,date.RTC_Year,date.RTC_Month,date.RTC_Date,time.RTC_Hours,time.RTC_Minutes,time.RTC_Seconds);
		RF_send(buf, 36);
		altimer_rev_flag = ALTIMETER_Rev_Undo;
		USART2_RX_STA = 0;
		delay_ms(50);
		ALTIMETER_POWER = POWER_ON;	//?ú′?′|′ò?a???è??
	}
}


/**************************************************

oˉêy??￡oTest_Sonar_USART6()
×÷  ??￡oá??t??
è?  ?ú￡o2018.9.17
°?  ±?￡oV1.00
?μ  ?÷￡o±ü??éù??2aê?3ìDò
DT??????￡o

**************************************************/
void Test_Sonar_USART6()
{
	u8 cnt = 0;
	u8 buf[64] = {0};
	Sonar_run();
	POWER_24V = POWER_OFF;
	Sonar_analyze();
	RTC_GetTime(RTC_Format_BIN, &time);
	RTC_GetDate(RTC_Format_BIN, &date);
	sprintf((char*)buf, "sonar:");
	for(cnt = 0; cnt <15; ++cnt)
		sprintf((char*)&buf[2*cnt+6], " %d",scan_result[cnt].block);
	sprintf((char*)&buf[2*cnt+6], " 20%02d-%02d-%02d %02d:%02d:%02d\r\n",date.RTC_Year,date.RTC_Month,date.RTC_Date,time.RTC_Hours,time.RTC_Minutes,time.RTC_Seconds);
	rt_enter_critical();
	RF_send(buf, 58);
	rt_exit_critical();
	
}

/**************************************************/


/**************************************************

函数名：Test_CTD48M_USART3()
作  者：刘晓东
日  期：2018.8.18
版  本：V1.00
说  明：温盐深测试程序
修改记录：

**************************************************/
void Test_CTD48M_USART3(void)
{
	u8 buf[62] = {0};
	if(CTD48M_rev_flag == CTM48_Rev_Done)
	{
		CTD_Data_Trans(CTD_RxBuf);
		RTC_GetTime(RTC_Format_BIN, &time);
		RTC_GetDate(RTC_Format_BIN, &date);
		sprintf((char*)buf,"temp:%7.4f depth:%7.4f cond:%7.4f 20%02d-%02d-%02d %02d:%02d:%02d\r\n",CTD_Final_Val.temp_cal_val,CTD_Final_Val.depth_cal_val,CTD_Final_Val.cond_cal_val,
					date.RTC_Year,date.RTC_Month,date.RTC_Date,time.RTC_Hours,time.RTC_Minutes,time.RTC_Seconds);
		RF_send(buf, 61);
		USART3_RX_STA = 0;
		CTD48M_rev_flag = CTM48_Rev_Undo;	
		CTD_Rx_Enable();	
	}
}

/**************************************************

函数名：RTC_test()
作  者：刘晓东
日  期：2018.8.18
版  本：V1.00
说  明：RTC测试程序
修改记录：

**************************************************/
void RTC_test()
{
	u8 buf[32];
	u8 cnt;
	RTC_TimeTypeDef time;
	RTC_DateTypeDef date;

	if(RTC_ReadBackupRegister(RTC_BKP_DR0) != 0x5050)
		MY_RTC_Init();
	
	RTC_GetTime(RTC_Format_BIN, &time);
	RTC_GetDate(RTC_Format_BIN, &date);
	sprintf((char*)buf, "20%02d-%02d-%02d, %02d:%02d:%02d\n",date.RTC_Year, date.RTC_Month, date.RTC_Date, time.RTC_Hours, time.RTC_Minutes, time.RTC_Seconds);

	while(buf[cnt] != '\n')
	{
		++cnt;
	}	
	data_upload_upper(RF_Type, buf, cnt+1);
}

/**************************************************

函数名：RNSS_test()
作  者：刘晓东
日  期：2018.9.6
版  本：V1.00
说  明：RNSS测试程序
修改记录：

**************************************************/
void RNSS_test()
{
	_BD_RNSS_data BD_position;
	
	if(WK2124_Rev_Flag[1] == WK2124_Rev_Done)
	{
		BD_data_trans(BD_raw_data, &BD_position);
		rt_kprintf("date  %4d-%02d-%02d %02d:%02d:%02d:%04d\n",BD_position.time.year,BD_position.time.month,BD_position.time.day,
			BD_position.time.hour,BD_position.time.min,BD_position.time.sec,BD_position.time.msec);
		rt_kprintf("location:latitude-%10.4f%c,longitute-%10.4f%c\n",BD_position.location.latitude,BD_position.location.N_S,
			BD_position.location.longitude,BD_position.location.E_W);
		WK2124_Rev_Flag[1] = WK2124_Rev_Undo;
		RNSS_start();
	}
}

/**************************************************

函数名：RDSS_test()
作  者：刘晓东
日  期：2018.9.6
版  本：V1.00
说  明：RDSS测试程序
修改记录：

**************************************************/
void RDSS_test()
{
	u8 buf[5] = {'1','2','3','4','5'};
	if(WK2124_Rev_Flag[2] == WK2124_Rev_Done)
	{
		Upper_Message_Analy(RDSS_REV_buf, BD_Type);
		BD_RDSS_TXSQ(0, 0, 0, (char*)buf, 5);
		data_upload_upper(BD_Type, (u8*)RDSS_TX_buf, sizeof(RDSS_TX_buf));
		WK2124_Rev_Flag[2] = WK2124_Rev_Undo;
	}
}


/**************************************************

函数名：RF_test()
作  者：刘晓东
日  期：2018.9.6
版  本：V1.00
说  明：RF测试程序
修改记录：

**************************************************/
void RF_test()
{
//	u8 cnt = 0;;
	u8 buf[BACK_STAT_LEN];
	stat_back_frame(buf,XTZT);
//	RF_send(buf, BACK_STAT_LEN);
	if(WK2124_Rev_Flag[0] == WK2124_Rev_Done)
	{
//		while(RF_Rev_Buf[cnt++] != '\n');
		
		Upper_Message_Analy(RF_Rev_Buf,RF_Type);
	
		WK2124_Rev_Flag[0] = WK2124_Rev_Undo;
	}
}
/**************************************************

函数名：SD_test()
作  者：刘晓东
日  期：2018.8.18
版  本：V1.00
说  明：SD卡测试程序
修改记录：

**************************************************/
void SD_test()
{
	u32 total = 0, free = 0;
	static u8 init_flag = 0;
	while(init_flag == 0 && SD_Init())
	{
		rt_kprintf("SD Init failed!");
		return ;
	}
	init_flag = 1;
	
	
	while(exf_getfree((u8*)"0", &total, &free))
	{
		delay_us(1000);
	}
	mf_scan_files((u8*)"0:");
}


/**************************************************

函数名：USB_test()
作  者：刘晓东
日  期：2018.10.11
版  本：V1.00
说  明：USB测试程序
修改记录：

**************************************************/
void USB_test()
{	
	u32 total, free;
	u8 buf[20] = {0x75,0x65,0x0C,0x10,0x10,0x08,0x01,0x04,0x04,0x00,0x32,0x05,0x00,0x32,0x08,0x00,0x32,0x0C,0x00,0x32};
	exfuns_init();
	f_mount(fs[0], "0:", 1);
	f_mount(fs[1], "1:", 1);
	USBH_Init(&USB_OTG_Core,USB_OTG_FS_CORE_ID,&USB_Host,&USBH_MSC_cb,&USR_Callbacks);
	while(1)
	{
		u8 res = 0;
		u16 cnt = 0;
		
		USBH_Process(&USB_OTG_Core, &USB_Host);
	delay_us(500);
//	LED1 = 0;
	delay_us(500);
//	LED1 = 1;
	if(U_link == 1)
	{
		res=exf_getfree((u8*)"1:",&total,&free);
		if(res != FR_OK)
			delay_us(1000);
		mf_scan_files((u8*)"1:");
		delay_us(1000);
		if(++cnt == 1)
		{
		res = mf_open((u8*)"1:/test1.txt",FA_READ | FA_WRITE | FA_CREATE_NEW, USB_dev);
		if(res != FR_OK)
			delay_us(1000);
		res = mf_write(buf, 10, USB_file);
		if(res != FR_OK)
			delay_us(1000);
		mf_close(USB_file);
		}
		cnt = 10;
		delay_us(1000);
	}
	}
}

/**************************************************

函数名：Test_SPI_Comm_Master()
作  者：刘晓东
日  期：2018.8.18
版  本：V1.00
说  明：主从机通信测试程序，从机上送状态
修改记录：

**************************************************/
u8 Test_SPI_Comm_Master()
{
	static u8 cnt = 0;
	s16 Master_Temp = 0;
	
	for(cnt = 0; cnt < 10; ++cnt)
	{
		if(MS_Comm_WriteByte(0x00) == RT_FAULT)
			return RT_ERROR;			//发一个字节
		Master_Temp = Master_ReadByte();
		if(Master_ReadByte() == RT_FAULT)
			return RT_ERROR;
		else
			rt_kprintf("%d\n", Master_Temp);
		delay_ms(500);	
	}
}

/**************************************************

函数名：motor_Test()
作  者：刘晓东
日  期：2018.9.25
版  本：V1.00
说  明：电机测试函数
修改记录：

**************************************************/
void motor_Test()
{
	static s8 turn_angle = -45;			//左右胸鳍转动角度
	static u8 oil_height = 184;			//油泵高度参数
	static u8 gravity_position = 4;		//重心调节滑块位置
	
	motor_test[0] = 0x36;
	motor_test[11] = 0x11;
	motor_test[12] = 0x05;
	motor_test[13] = 0x01;
	rt_kprintf((const char*)motor_test);
	rt_kprintf("\r\n");
	Master_Send_Data(motor_test, 17);		//开所有驱动器电源
	delay_ms(10);
	
	rt_memset(motor_test, 0, 17);			
	motor_test[0] = 0x36;
	motor_test[8] = 0x08;
	rt_kprintf((const char*)motor_test);
	rt_kprintf("\r\n");
	Master_Send_Data(motor_test, 17);		//油缸位置初始化
	delay_ms(100);
	
	rt_memset(motor_test, 0, 17);			
	motor_test[0] = 0x36;
	motor_test[5] = 0x05;
	rt_kprintf((const char*)motor_test);
	rt_kprintf("\r\n");
	Master_Send_Data(motor_test, 17);		//重心调节初始化指令
	delay_ms(100);
	
	while(1)
	{
		rt_memset(motor_test, 0, 17);			
		motor_test[0] = 0x36;
		motor_test[1] = 0x03;
		motor_test[2] = 0x03;
		motor_test[3] = ++turn_angle;
		rt_kprintf((const char*)motor_test);
		rt_kprintf("\r\n");
		Master_Send_Data(motor_test, 17);		//左右胸鳍同时转动一定角度
		delay_ms(100);
		if(turn_angle == 90)
				turn_angle = -90;
		
		rt_memset(motor_test, 0, 17);			
		motor_test[0] = 0x36;
		motor_test[1] = 0x03;
		motor_test[2] = 0x03;
		motor_test[3] = 0x00;
		rt_kprintf((const char*)motor_test);
		rt_kprintf("\r\n");
		Master_Send_Data(motor_test, 17);		//左右胸鳍同时转动到0度
		delay_ms(100);
		
		rt_memset(motor_test, 0, 17);			
		motor_test[0] = 0x36;
		motor_test[4] = 0x04;
		rt_kprintf((const char*)motor_test);
		rt_kprintf("\r\n");
		Master_Send_Data(motor_test, 17);		//胸鳍拍动
		delay_ms(100);
		
		rt_memset(motor_test, 0, 17);			
		motor_test[0] = 0x36;
		motor_test[1] = 0x03;
		motor_test[2] = 0x03;
		rt_kprintf((const char*)motor_test);
		rt_kprintf("\r\n");
		Master_Send_Data(motor_test, 17);		//胸鳍停止拍动
		delay_ms(100);
		
		rt_memset(motor_test, 0, 17);			
		motor_test[0] = 0x36;
		motor_test[9] = 0x09;
		motor_test[10] = oil_height++;
		rt_kprintf((const char*)motor_test);
		rt_kprintf("\r\n");
		Master_Send_Data(motor_test, 17);		//油缸高度值设定
		if(oil_height == 215)
				oil_height = 184;
		delay_ms(100);
		
		rt_memset(motor_test, 0, 17);			
		motor_test[0] = 0x36;
		motor_test[6] = 0x06;
		motor_test[7] = gravity_position++;
		rt_kprintf((const char*)motor_test);
		rt_kprintf("\r\n");
		Master_Send_Data(motor_test, 17);		//重心调节高度值设定
		if(gravity_position == 64)
				gravity_position = 4;
		delay_ms(100);
	}
}


/**************************************************

函数名：Master_Slave_Test()
作  者：刘晓东
日  期：2018.8.18
版  本：V1.00
说  明：SPI主从机通信测试,主机下发程序
修改记录：

**************************************************/
void Master_Slave_Test()
{
#ifndef __DEBUG
	if(fish_param_flag == 1)
	{
		fish_param[0] = 0x36;
		Master_Send_Data(fish_param, 17);
		UART4->CR1 |= USART_Mode_Rx;
		fish_param_flag = 0;
		UART4_RX_STA = 0;
	}
	else
	{
		motor_Test();
	}
#else
//	Master_Slave_Comm(TEST);
	left_pectoral.run_mode = 1;
	left_pectoral.flap_amplitude  = 38;
	left_pectoral.flap_fre_integer = 1;
	left_pectoral.flap_fre_decimal = 25;
	left_pectoral.target_location = 123;
	
	right_pectoral.run_mode = 1;
	right_pectoral.flap_amplitude  = 38;
	right_pectoral.flap_fre_integer = 1;
	right_pectoral.flap_fre_decimal = 25;
	right_pectoral.target_location = 123;
	
	waist_motor.run_mode = 1;
	waist_motor.flap_amplitude  = 38;
	waist_motor.flap_fre_integer = 1;
	waist_motor.flap_fre_decimal = 25;
	waist_motor.target_location = 66;
	
	tail_motor.run_mode = 1;
	tail_motor.flap_amplitude  = 38;
	tail_motor.flap_fre_integer = 1;
	tail_motor.flap_fre_decimal = 25;
	tail_motor.target_location = 66;
	
	oil_pumb_motor.run_mode = 1;
	oil_pumb_motor.target_location = 566;
	oil_pumb_motor.target_speed = 100;
	
	gravity_motor.run_mode = 1;
	gravity_motor.target_location = 100;
	gravity_motor.target_speed = 22;
	
	Master_Slave_Comm(YTSF);
#endif	
}

/**************************************************

函数名：Upper_Master_Test()
作  者：刘晓东
日  期：2018.8.31
版  本：V1.00
说  明：上位机与主机通信测试
修改记录：

**************************************************/

void Upper_Master_Test()
{
	
	if(WK2124_Rev_Flag[0] == 1)
	{
		Upper_Message_Analy(RF_Rev_Buf,  RF_Type);
		WK2124_Rev_Flag[0] = 0;
//		RF_send(RF_Rev_Buf, sizeof(RF_Rev_Buf));
//		data_upload_upper(RF_Type, XTZT, RF_Send_Buf);
	}
	
}

/**************************************************

函数名：AHRS_calc_CRC()
作  者：刘晓东
日  期：2018.8.31
版  本：V1.00
说  明：惯导计算校验码
修改记录：

**************************************************/
void AHRS_calc_CRC()
{
	u8 buf[7] = {0x75,0x65,0x0C,0x03,0x03,0x40,0x03};
	u8 cnt = 0, check_sum_byte1 = 0, check_sum_byte2 = 0;
	u16 check_sum = 0;
	for(cnt = 0; cnt < sizeof(buf); ++cnt)
	{
		check_sum_byte1 += buf[cnt];
		check_sum_byte2 += check_sum_byte1;
	}
	check_sum = ((u16)check_sum_byte1 << 8) + (u16)check_sum_byte2;
	delay_ms(1000);
}

/**************************************************

函数名：location_test()
作  者：刘晓东
日  期：2018.10.15
版  本：V1.00
说  明：导航中定位的计算鱼演算
修改记录：

**************************************************/
void navi_location_test()
{	
	if(USART1_RX_STA & 0x8000)
	{
		AHRS_data_analys();
	//	calc_location();
	}
	AHRS_Require();
	delay_ms(50);
}



#endif


