/**************************************************

文件名：BD_RNSS.c
作  者：刘晓东
日  期：2018.7.30
版  本：V1.00
说  明：北斗RNSS部分初始化文件
修改记录：

**************************************************/

#include "BD_RNSS.h"
#include <stdlib.h>				//字符串与整数、浮点数转换函数
#include <ctype.h>				//包含判断字符是否是字母、数字函数
#include <string.h>

char BD_raw_data[LOCA_INFO_LEN];

/****北斗模块上送的数据头*****/
const char* string_head[6] = {"GNRMC", "GNGGA", "GNGLL", "GNGSA", "GPGSV", "BDGSV"};
u8 day_per_month[12] = {31,28,31,30,31,30,31,31,30,31,30,31};

struct rt_semaphore RNSS_rev_sem;	//RNSS数据接收信号量





/**************************************************

函数名：BD_RNSS_Init
作  者：刘晓东
日  期：2018.7.30
版  本：V1.00
说  明：配置RNSS串口参数
		包括北斗模块的电源开关管脚初始化
参  数：
修改记录：

**************************************************/

void BD_RNSS_Init(void)
{
	GPIO_InitTypeDef	GPIO_InitStructure;
	uartTydef uartParaStructure;
	u8 GIER_reg, FCR_reg;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);					//使能GPIOB时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);					//使能GPIOB时钟
	
	uartParaStructure.BaudRate = BAUDRATE_9600;			//RNSS波特率9600
	uartParaStructure.Parity = WK2124_PAMN;				//无校验
	uartParaStructure.StopBits = WK2124_STPL_1;			//1个停止位
	uartParaStructure.WordLength = WK2124_WORDLENGTH_8;	//8位字长
	uartParaStructure.rev_contact_val = CONTACT_VAL;	//触点门槛值
	uartParaStructure.send_contact_val = CONTACT_VAL;
	uartParaStructure.interrupt_enable = WK2124_RFTRIG_IEN | WK2124_RXOUT_IEN;   //接收触点中断 接收超时中断
	WK2124_UART_Config(BDRN_UART2, &uartParaStructure);
	
	
	WK2124_read_reg(GIER, &GIER_reg);
	WK2124_write_reg(GIER, (u8)(1<<(BDRN_UART2>>4)) | GIER_reg);					//打开串口中断
	
	WK2124_read_reg(FCR | BDRN_UART2, &FCR_reg);
	WK2124_write_reg(FCR | BDRN_UART2, 0x03 | FCR_reg);					//复位FIFO
	
	
	
	
	/************PC2推挽输出，北斗开关管脚**********/ 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; 										//GPIOC2
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;									//输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;								//速度2MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 									//推挽输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 									//上拉
	GPIO_Init(GPIOC,&GPIO_InitStructure); 											//初始化PC2
	
	/************PG1推挽输出，RNSS串口发送使能**********/ 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1; 										//GPIOG1
	GPIO_Init(GPIOG,&GPIO_InitStructure); 
	
	if(rt_sem_init(&RNSS_rev_sem, "RNSS_sem", 0, RT_IPC_FLAG_FIFO) != RT_EOK)	//初始化信号量
		rt_kprintf("RNSS_sem init ERROR!\n");
	
	BD_RNSS_Tx = 0;																	//默认为接收
	BD_Power_En = 0;																//初始化时北斗不上电
}

/**************************************************

函数名：RNSS_start
作  者：刘晓东
日  期：2018.8.13
版  本：V1.00
说  明：
参  数：
返回值：
修改记录：

**************************************************/
void RNSS_start(void)
{
	u8 GENA_reg = 0;
	u8 FCR_reg = 0;
	BD_Power_En = 1;		//北斗模块上电
	
	WK2124_read_reg(GENA, &GENA_reg);
	WK2124_write_reg(GENA, 0x02 | GENA_reg);	//使能串口
	
//	WK2124_read_reg(FCR | BDRN_UART2, &FCR_reg);
//	WK2124_write_reg(FCR | BDRN_UART2, 0x03 | FCR_reg);					//复位FIFO
}


/**************************************************

函数名：BD_seek_pos
作  者：刘晓东
日  期：2018.7.30
版  本：V1.00
说  明：寻找字符串中第num个逗号的位置
		北斗上送的数据以逗号分隔
参  数：str				寻找的字符串
		num				寻找的逗号
返回值：逗号所在的位置
修改记录：

**************************************************/
char* BD_seek_pos(char* str, u8 num)
{
	while(num)
	{
		if(*str == '*' || (!isalnum(*str) && *str != ',' && *str != '.' && *str != '-'))  //字符非逗号、小数点、负号、字母、数字或者到达字符串结尾,报错
			return NULL;
		if(*str++ == ',')
			--num;
	}
	return str;
}

/**************************************************

函数名：isLeap
作  者：刘晓东
日  期：2018.8.2
版  本：V1.00
说  明：判断年份是否为闰年
参  数：year 年份
返回值：1 是闰年
		0 不是闰年
修改记录：

**************************************************/
u8 isLeap(u16 year)
{
	if(year % 100 == 0)
	{
		return year % 400 ? 1 : 0;
	}
	return year%4 ? 1 : 0;
}

/**************************************************

函数名：Time_UTC2BJ
作  者：刘晓东
日  期：2018.8.2
版  本：V1.00
说  明：UTC时间转换为北京时间
参  数：trans_result	北斗上送的UTC时间所在的结构体
返回值：
修改记录：

**************************************************/

void Time_UTC2BJ(_BD_RNSS_data *trans_result)
{
	
	if(trans_result->time.hour < 16)	//补上时差是否大于24
	{
		trans_result->time.hour += 8;
		return;
	}
	trans_result->time.hour -= 16;
	
	if(isLeap(trans_result->time.year))	//是否是闰年
		day_per_month[1] = 29;
	else
		day_per_month[1] = 28;
	
	if(!(trans_result->time.day < day_per_month[trans_result->time.month - 1]))	//是否是一月最后一天
	{
		trans_result->time.day += 1;
		return;
	}
	trans_result->time.day = 1;
	if(trans_result->time.month < 12)											//是否是12月
	{
		trans_result->time.month += 1;
		return;
	}
	trans_result->time.month = 1;
	trans_result->time.year += 1;
	return;
}

/**************************************************

函数名：BD_GNRMC_trans
作  者：刘晓东
日  期：2018.7.30
版  本：V1.00
说  明：GNRMC中的数据转换
参  数：str				要转换的子串
		trans_result	转换结果
修改记录：

**************************************************/
u8 BD_GNRMC_trans(char *str, _BD_RNSS_data *trans_result)
{
	char *pos = NULL;
	char *sub_str = NULL;
	u32 date = 0;
	
/******************解析时间*********************/
	pos = BD_seek_pos(str, 1);			//寻找时间数据在字符串中的位置
	if(pos == NULL)
		return RT_ERROR;
	else
	{
		trans_result->time.hour = (*pos - 0x30)*10 + (*(pos+1)-0x30); 		//得到小时
		trans_result->time.min = (*(pos+2) - 0x30)*10 + (*(pos+3)-0x30);		//得到分钟
		trans_result->time.sec = (*(pos+4) - 0x30)*10 + (*(pos+5)-0x30);		//得到秒数
		trans_result->time.msec = ((u16)*(pos+7) - 0x30)*100 + (*(pos+8) - 0x30)*10 + (*(pos+9)-0x30); //	得到毫秒数
	}
	
/**********解析数据有效位**************/
	pos = BD_seek_pos(str, 2);			//寻找数据状态在字符串中的位置
	if(pos == NULL)
		return RT_ERROR;
	else
	{
		if(*pos == 'A')						//数据有效
			trans_result->data_state = 1;
		else								//数据无效
		{
			trans_result->data_state = 0;
			return RT_ERROR;				//状态位异常
		}		
	}
/**********解析纬度**************/
	pos = BD_seek_pos(str, 3);			
	if(pos == NULL)
		return RT_ERROR;
	else
	{
		sub_str = rt_strncpy(sub_str, pos, 9);
		trans_result->location.latitude = atof(sub_str);		//字符串转换为float类型
		trans_result->location.N_S = *(pos+10);				//南纬还是北纬
	}
/**********解析经度**************/
	pos = BD_seek_pos(str, 5);			
	if(pos == NULL)
		return RT_ERROR;
	else
	{
		sub_str = rt_strncpy(sub_str, pos, 10);
		trans_result->location.longitude = atof(sub_str);		//字符串转换为float类型
		trans_result->location.E_W = *(pos+11);					//东经、西经
	}
/**********解析日期**************/
	pos = BD_seek_pos(str, 9);			
	if(pos == NULL)
		return RT_ERROR;
	else
	{
		sub_str = rt_strncpy(str, pos, 6);
		date = atol(sub_str);
		trans_result->time.day = date/10000;					//日
		trans_result->time.year = 2000 + date%100;				//年
		trans_result->time.month = (date/100)%100;				//月
	}
	
	Time_UTC2BJ(trans_result);									//UTC时间转换为北京时间
	return RT_EOK;
}


/**************************************************

函数名：BD_GNGGA_trans
作  者：刘晓东
日  期：2018.7.30
版  本：V1.00
说  明：GNGGA中的数据转换
参  数：str				要转换的子串
		trans_result	转换结果
修改记录：

**************************************************/
u8 BD_GNGGA_trans(char *str, _BD_RNSS_data *trans_result)
{
	u8 data_len = 0;
	s8 symbol = 1;
	char *pos1 = NULL;
	char *pos2 = NULL;
	char *sub_str = NULL;				//存储数据子串
	pos1 = BD_seek_pos(str, 9);			//寻找海拔高度在字符串中的位置
	if(pos1 == NULL)
		return RT_ERROR;
	pos2 = BD_seek_pos(str, 10);			
	if(pos2 == NULL)
		return RT_ERROR;
	if(*pos1 == '-')
	{
		++pos1;
		symbol = -1;
	}
	data_len = pos2 - pos1-1;			//高度字节占得字节数
	sub_str = rt_strncpy(sub_str, pos1, data_len);
	trans_result->location.altitude = symbol * atof(sub_str);	//高度字符串转换为float类型
	return RT_EOK;
}

/**************************************************

函数名：BD_single_trans
作  者：刘晓东
日  期：2018.7.30
版  本：V1.00
说  明：截取字符串中的单个子串进行转换
参  数：str				要转换的字符串		
		head_str		要转换的子串串头
		trans_result	转换结果
修改记录：

**************************************************/
s8 BD_single_trans(char *str ,const char* head_str, _BD_RNSS_data *trans_result)
{
	u8 num;
	for(num = 0; num < 6; ++num)			//寻找匹配的字符串头	
	{
		if(head_str == string_head[num])
			break;
	}
	switch(num)
	{
		case 0:
			BD_GNRMC_trans(str, trans_result);		//转换GNRMC数据
			break;
		case 1:
			BD_GNGGA_trans(str, trans_result);		//转换GNGGA数据
			break;
		case 2:
		case 3:
		case 4:
		case 5:
		default:
			break;
	}
	return RT_EOK;
}


/**************************************************

函数名：BD_data_trans
作  者：刘晓东
日  期：2018.7.30
版  本：V1.00
说  明：从北斗上送的字符串数据中解析出需要的数据
参  数：BD_raw_data		北斗上送的原始数据
		trans_result	数据转换结果
修改记录：

**************************************************/

void BD_data_trans(char *BD_raw_data, _BD_RNSS_data *trans_result)
{
	u8 BD_data_head = 0;
	for(BD_data_head = 0; BD_data_head < 2; ++BD_data_head)
	{
		char *pos = NULL;										//在字符串中出现的位置
		if((pos = rt_strstr(BD_raw_data, string_head[BD_data_head]) )!= RT_NULL)
		{
			BD_single_trans(pos, string_head[BD_data_head], trans_result);			//单个字符串进行转换
		}
	}
}



