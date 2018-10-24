#include "sys.h"
#include "delay.h"
#include "print_uart.h"
#include "arm_math.h"
#include "led.h"
#include "rtthread.h"
#include "struct.h"
#include "SD.h"
#include "ff.h"
#include "exfuns.h"
#include <rthw.h>
#include "fattester.h"
#include "usbh_usr.h"
#include "test.h"
#include "RF.h"
#include "Upper_Comm.h"
#include "AHRS.h"
#include "Info_Record.h"





static struct rt_thread led0_thread;//线程控制块
static struct rt_thread led1_thread;//线程控制块
ALIGN(RT_ALIGN_SIZE)
static rt_uint8_t rt_led0_thread_stack[1024];//线程栈 
static rt_uint8_t rt_led1_thread_stack[1024];//线程栈 


u8 wr_data[30];
u16 sd_cnt = 0;

u8 check_sum_byte1 = 0, check_sum_byte2 = 0;
u16 check_sum = 0;
u8 buf[20] = {0x75,0x65,0x0C,0x10,0x10,0x08,0x01,0x04,0x04,0x00,0x32,0x05,0x00,0x32,0x08,0x00,0x32,0x0C,0x00,0x32};

USBH_HOST  USB_Host;
USB_OTG_CORE_HANDLE  USB_OTG_Core;


//u8 USH_User_App(void)
//{ 
//	u32 total,free;
//	u8 res=0;	 
//	res=exf_getfree("1:",&total,&free);
//	if(res==0)
//	{
//			
//	} 
// 
//	while(HCD_IsDeviceConnected(&USB_OTG_Core))//设备连接成功
//	{	
//		LED1=!LED1;
//		delay_us(1000);
//	}
//	
//	return res;
//} 

//线程LED0
static void led0_thread_entry(void* parameter)
{
	
while(1)
{

		

	LED2 = 0;
	LED1 = 1;
	delay_ms(1000);
	LED2 = 1;
	LED1 = 0;
	delay_ms(1000);
	
}


}

//线程LED1
static void led1_thread_entry(void* parameter)
{
//	u16 reg_val = 0, cnt = 0;

//	f_mount(fs[0], "0:", 1);
//	f_mount(fs[1], "1:", 1);
//	u32 total, free;
//	USBH_Init(&USB_OTG_Core,USB_OTG_FS_CORE_ID,&USB_Host,&USBH_MSC_cb,&USR_Callbacks);
//	BD_RDSS_Send(XTZJ, sizeof(XTZJ));
//	
//	delay_ms(100);
//	AHRS_Init(115200);

while(1)
{
	u8 res = 0;
	u16 cnt = 0;
//	RF_test();
//	Test_Sonar_USART6();
//	AHRS_calc_CRC();
//	for(cnt = 0; cnt < sizeof(buf); ++cnt)
//	{
//		check_sum_byte1 += buf[cnt];
//		check_sum_byte2 += check_sum_byte1;
//	}
//	check_sum = ((u16)check_sum_byte1 << 8) + (u16)check_sum_byte2;
//	cnt = sizeof(long long);
//	WDG_Feed();
//	Test_altimeter_USART2();
	delay_ms(3000);
	Master_Slave_Test();
//	Test_SPI_Comm_Master();
//	Upper_Master_Test();
	
//	RF_test();

//	RTC_test();
//	sprintf((char*)wr_data, "1234567890qazws这是测试:%04d\r\n", sd_cnt);
//	rt_kprintf("  %d\n",sd_cnt++);
//	Test_AHRS_USART1();
//	info_record(location_info, wr_data, 30);
//	delay_ms(10);
//	delay_ms(1000);
//	delay_ms(1000);
	
//	Test_AHRS_USART1();
//	Master_Slave_Test();
	
//	navi_location_test();
//	RF_send(buf, 12);
//	Test_AHRS_USART1();
//	Test_altimeter_USART2();
//	Sonar_run();
//	u8 res;
//	u8 buf[10] = "1234567890";
//	USBH_Process(&USB_OTG_Core, &USB_Host);
//	delay_us(500);
//	LED1 = 0;
//	delay_us(500);
//	LED1 = 1;
//	if(U_link == 1)
//	{
//		res=exf_getfree("1:",&total,&free);
//		if(res != FR_OK)
//			delay_us(1000);
//		mf_scan_files((u8*)"1:");
//		delay_us(1000);
//		if(++cnt == 1)
//		{
//		res = mf_open((u8*)"1:/test1.txt",FA_READ | FA_WRITE, USB_dev);
//		if(res != FR_OK)
//			delay_us(1000);
//		res = mf_write(buf, 10, USB_file);
//		if(res != FR_OK)
//			delay_us(1000);
//		mf_close(USB_file);
//		}
//		cnt = 10;
//		delay_us(1000);
//	}
}
}


int main(void)
{
	u16 i = 0;;
//	u8 buf[5] = "12345";
//	RTC_TimeTypeDef RTC_TimeStruct;
//	RTC_DateTypeDef RTC_DateStruct;
	
	
	rt_base_t level;
	level = rt_hw_interrupt_disable();
	LED_Init();
	
//	while(SD_Init())
//	{
//		if(++i >= 200)
//			break;
//		LED2 = 0;
//	}
	
//	i = 0;
//	while(f_mount(fs[0],"0:",1) != FR_OK)
//	{
//		if(++i >= 200)
//			break;
//		LED2 = 0;
//	}
	MS_Comm_TIMode_Init();
//	WDG_Init();
//	MY_RTC_Init();
//	WK2124_Init();
//	BD_RNSS_Init();
//	RNSS_start();
//	RF_Init();
//	Sonar_Init(115200);

	
	LED1 = 0;
	while(i++ <1000)
	delay_us(1000);	
	LED1 = 1;
	rt_kprintf("reset!\n");
	
//	AHRS_Init(115200);
	
	

//	exfuns_init();
rt_hw_interrupt_enable(level);
//	while(1)
//	{
//		delay_ms(1);
//		Test_AHRS_USART1();
//	}
    
    // 创建静态线程
    rt_thread_init(&led0_thread,                 	//线程控制块
                   "led0",                       	//线程名字，在shell里面可以看到
                   led0_thread_entry,            	//线程入口函数
                   RT_NULL,                     	//线程入口函数参数
                   &rt_led0_thread_stack[0],     	//线程栈起始地址
                   sizeof(rt_led0_thread_stack),  	//线程栈大小
                   5,    //线程的优先级
                   20);                         	//线程时间片
    rt_thread_startup(&led0_thread);             	//启动线程led0_thread  
				   
	rt_thread_init(&led1_thread,                 	//线程控制块
                   "led1",                      	//线程名字，在shell里面可以看到
                   led1_thread_entry,            	//线程入口函数
                   RT_NULL,                     	//线程入口函数参数
                   &rt_led1_thread_stack[0],		//线程栈起始地址
                   sizeof(rt_led1_thread_stack), 	//线程栈大小
                   4,    //线程的优先级
                   20);         				   

	rt_thread_startup(&led1_thread);             //启动线程led1_thread 
    
 
}



 



