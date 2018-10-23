/**************************************************

文件名：MS_Com_Process.c
作  者：刘晓东
日  期：2018.8.27
版  本：V1.00
说  明：主从机通讯流程
修改记录：

**************************************************/
#include "MS_Com_Process.h"



_time	Master_Time;			//主机时间，该时间同步于北斗卫星时间
_move_motor  left_pectoral;				//左胸鳍参数
_move_motor	 right_pectoral;			//右胸鳍
_oil_pumb_motor  oil_pumb_motor;		//油泵电机参数
_gravity_motor 	 gravity_motor;			//重心电机参数
_move_motor	 waist_motor;				//腰部电机参数	
_move_motor		 tail_motor;			//尾部电机参数

_slave_stat  slave_stat;

u8 Master_send_buf[64];					//主机发送缓存区
u8 slave_stat_buf[28];						//接收到的从机状态缓存区

/**************************************************

函数名：sys_check_framing
作  者：刘晓东
日  期：2018.8.27
版  本：V1.00
说  明：系统自检组帧
参  数：buf 	发送缓存
返回值：len		发送数据长度
修改记录：

**************************************************/
u8 sys_check_framing(u8 *buf)
{
	u8 cnt = 0;
	buf[cnt++] = XTJC;
	buf[cnt++] = 13;
	buf[cnt++] = Master_Time.year>>8;
	buf[cnt++] = Master_Time.year;
	buf[cnt++] = Master_Time.month;
	buf[cnt++] = Master_Time.day;
	buf[cnt++] = Master_Time.hour;
	buf[cnt++] = Master_Time.min;
	buf[cnt++] = Master_Time.sec;
	buf[cnt++] = Master_Time.msec>>8;
	buf[cnt++] = Master_Time.msec;
	buf[cnt] = CRC8_Table(buf, cnt);
	buf[++cnt] = 0xEE;
	return cnt+1;
}

/**************************************************

函数名：read_stat_framing
作  者：刘晓东
日  期：2018.8.27
版  本：V1.00
说  明：读取从机状态
参  数：buf 	发送缓存
返回值：len		发送数据长度
修改记录：

**************************************************/
u8 read_stat_framing(u8 *buf)
{
	u8 cnt = 0;
	buf[cnt++] = ZTSB;
	for(; cnt < 29; ++cnt)
		buf[cnt] = 0x00;				//读取从机数据，主机下发数据无意义，全部下发0，此处不可更改为下发其他数据
	return cnt;
}


/**************************************************

函数名：sys_timing_framing
作  者：刘晓东
日  期：2018.8.27
版  本：V1.00
说  明：系统自检组帧(低字节在前，高字节在后)
参  数：buf 	发送缓存
返回值：len		发送数据长度
修改记录：

**************************************************/
u8 sys_timing_framing(u8 *buf)
{
	u8 cnt = 0;
	buf[cnt++] = XTJS;
	buf[cnt++] = 13;
	buf[cnt++] = Master_Time.year>>8;
	buf[cnt++] = Master_Time.year;
	buf[cnt++] = Master_Time.month;
	buf[cnt++] = Master_Time.day;
	buf[cnt++] = Master_Time.hour;
	buf[cnt++] = Master_Time.min;
	buf[cnt++] = Master_Time.sec;
	buf[cnt++] = Master_Time.msec>>8;
	buf[cnt++] = Master_Time.msec;
	buf[cnt] = CRC8_Table(buf, cnt);
	buf[++cnt] = 0xEE;
	return cnt+1;
}


/**************************************************

函数名：fish_run_framing
作  者：刘晓东
日  期：2018.8.27
版  本：V1.00
说  明：鱼运行状态组帧
参  数：buf 	发送缓存
		CMD_ID	鱼的运行状态
返回值：len		发送数据长度
修改记录：

**************************************************/
u8 fish_run_framing(u8 *buf, u8 CMD_ID)
{
	u8 cnt = 0;
	buf[cnt++] = CMD_ID;
	buf[cnt++] = 38;
	buf[cnt++] = left_pectoral.run_mode;
	buf[cnt++] = left_pectoral.flap_amplitude;
	buf[cnt++] = left_pectoral.flap_fre_integer;
	buf[cnt++] = left_pectoral.flap_fre_decimal;			//小数部分*100变为整数
	buf[cnt++] = left_pectoral.target_location>>8;
	buf[cnt++] = left_pectoral.target_location;
	buf[cnt++] = right_pectoral.run_mode;
	buf[cnt++] = right_pectoral.flap_amplitude;
	buf[cnt++] = right_pectoral.flap_fre_integer;
	buf[cnt++] = right_pectoral.flap_fre_decimal;
	buf[cnt++] = right_pectoral.target_location>>8;
	buf[cnt++] = right_pectoral.target_location;
	buf[cnt++] = oil_pumb_motor.run_mode;
	buf[cnt++] = (u16)oil_pumb_motor.target_location>>8;
	buf[cnt++] = (u16)oil_pumb_motor.target_location;
	buf[cnt++] = (u16)oil_pumb_motor.target_speed>>8;
	buf[cnt++] = (u16)oil_pumb_motor.target_speed;
	buf[cnt++] = gravity_motor.run_mode;
	buf[cnt++] = ((u32)gravity_motor.target_location * 1000)>> 24;
	buf[cnt++] = ((u32)gravity_motor.target_location * 1000) >>16;
	buf[cnt++] = ((u32)gravity_motor.target_location * 1000) >>8;
	buf[cnt++] = ((u32)gravity_motor.target_location * 1000) ;
	buf[cnt++] = gravity_motor.target_speed>>8;
	buf[cnt++] = gravity_motor.target_speed;
	buf[cnt++] = waist_motor.run_mode;
	buf[cnt++] = waist_motor.flap_amplitude;
	buf[cnt++] = waist_motor.flap_fre_integer;
	buf[cnt++] = waist_motor.flap_fre_decimal;		//小数部分*100变为整数
	buf[cnt++] = (s8)waist_motor.target_location;
	buf[cnt++] = tail_motor.run_mode;
	buf[cnt++] = tail_motor.flap_amplitude;
	buf[cnt++] = tail_motor.flap_fre_integer;
	buf[cnt++] = tail_motor.flap_fre_decimal;		//小数部分*100变为整数
	buf[cnt++] = (s8)tail_motor.target_location;
	buf[cnt] = CRC8_Table(buf, cnt);
	buf[++cnt] = 0xEE;
	return cnt+1;
}



/**************************************************

函数名：Master_Slave_Comm
作  者：刘晓东
日  期：2018.8.27
版  本：V1.00
说  明：主从机通信函数
参  数：CMD_ID 通信命令码
返回值：RT_EOK		成功
		RT_ERROR	失败
修改记录：

**************************************************/
u8 Master_Slave_Comm(u8 CMD_ID)
{
	u8 send_len = 0;
	switch(CMD_ID)
	{
		case XTJC:												//系统自检
			send_len = sys_check_framing(Master_send_buf);
			if(Master_Send_Data(Master_send_buf, send_len))
				return RT_ERROR;
			break;
		case ZTSB:												//从机状态上送
#ifdef  __DEBUG
			rt_kprintf("\n\n\n\n\ntest start!\n");
#endif
			if(MS_Comm_WriteByte(ZTSB))
				return RT_ERROR;
			if(Master_ReadByte())
				return RT_ERROR;
			if(MS_Comm_WriteByte(0x00))
				return RT_ERROR;
			if(Master_ReadByte())
				return RT_ERROR;									//接收字节
			if(Master_Rev_Data(slave_stat_buf, slave_stat_bytes))
				return RT_ERROR;									//从机状态字节数为 28
			break;
		case XTTZ:												//从机电机停止运行
			if(MS_Comm_WriteByte(XTTZ))
				return RT_ERROR;
			break;
		case XTJS:
			send_len = sys_timing_framing(Master_send_buf);
			if(Master_Send_Data(Master_send_buf, send_len))
				return RT_ERROR;
			break;
		case XTFW:												//系统复位
			if(MS_Comm_WriteByte(XTFW))
				return RT_ERROR;
			break;
		case YTHL:
			if(MS_Comm_WriteByte(YTHL))							//鱼体回零
				return RT_ERROR;
			break;
		case YTZP:
			if(MS_Comm_WriteByte(YTZP))							//鱼体回零
				return RT_ERROR;
			break;
		case YTSF:												//鱼体运行姿态
		case YTXQ:
		case YTZZ:
		case YTYZ:
		case YTHT:
		case YTHX:
		case YTKT:												//鱼体快速推进
#ifdef __DEBUG
		case DJXQL:
		case DJXQR:
		case DJZX:
		case DJYB:
		case DJWB:
		case DJYG:
#endif
			send_len = fish_run_framing(Master_send_buf, CMD_ID);
			if(Master_Send_Data(Master_send_buf, send_len))
				return RT_ERROR;
			break;
		default:
			break;
	}
	return RT_EOK;
}


/**************************************************

函数名：Slave_Stat_Anay
作  者：刘晓东
日  期：2018.8.28
版  本：V1.00
说  明：从机状态解析函数
参  数：buf 从机状态缓存区
返回值：RT_EOK		成功
		RT_ERROR	失败
修改记录：

**************************************************/
u8 Slave_Stat_Anay(u8* buf)
{
	if(buf[0] != ZTSB || buf[27] != 0xEE)
		return RT_ERROR;
	if(CRC8_Table(buf, 26) != buf[27])
		return RT_ERROR;
	slave_stat.sys_master_stat = buf[2] & 0x0F;
	slave_stat.sys_slave_stat  = (buf[3] << 4) + (buf[2] >> 4);
	slave_stat.sys_fault = (buf[4]<<24) + (buf[5]<<16) + (buf[6]<<8) + buf[7];
	slave_stat.left_pertoral = buf[8]>>4;
	slave_stat.right_pertoral = buf[8] & 0x0F;
	slave_stat.oil_motor = buf[9]>>4;
	slave_stat.gravity_motor = buf[9] & 0x0F;
	slave_stat.waist_motor = buf[10]>>4;
	slave_stat.tail_motor = buf[10] & 0x0F;
	slave_stat.power_vol = buf[11];
	slave_stat.power_cur = buf[12];
	slave_stat.power_temp = (float)(((buf[13]<<24) + (buf[14]<<16) + (buf[15]<<8) +buf[16]) / 1000.0);
	slave_stat.power_temp = buf[17];
	slave_stat.power_fault = (float)(((buf[18]<<24) + (buf[19]<<16) + (buf[20]<<8) +buf[21]) / 1000.0);
	slave_stat.cabin1_temp = buf[22];	
	slave_stat.cabin2_temp = buf[23];	
	slave_stat.cabin3_temp = buf[24];	
	slave_stat.cabin4_temp = buf[25];	
	return RT_EOK;
}







