/**************************************************

文件名：Upper_Comm.c
作  者：刘晓东
日  期：2018.8.28
版  本：V1.00
说  明：上位机与主机通信部分
		包括北斗与RF模块两部分
修改记录：

**************************************************/

#include "Upper_Comm.h"

extern u8 Feed_Flag;
extern _time Master_Time;



_time Upper_Time;					//上位机下发的时间

_Upper_Fish_CMD  Upper_Fish_CMD;
_fish_route Track_Info[ROUTE_NUM];	//鱼游动的航迹信息
_upper_assign_loc upper_assign_loc;	//上位机下发的地点信息


_sys_stat sys_stat;						//系统状态
u8  RF_Send_Buf[UPPER_MAX_LEN];			//RF发送缓存区


/**************************************************

函数名：Sys_Stat_Analy
作  者：刘晓东
日  期：2018.8.29
版  本：V1.00
说  明：系统状态分析
参  数：
修改记录：

**************************************************/
void Sys_Stat_Analy()
{
	
}

/**************************************************

函数名：Upper2Slave
作  者：刘晓东
日  期：2018.8.29
版  本：V1.00
说  明：上位机下发的数据转换为发送给从机的数据
参  数：
修改记录：

**************************************************/
void Upper2Slave()
{
	switch(Upper_Fish_CMD.cmd_type)
	{
		
	}
}

/**************************************************

函数名：Upper_Data_Analy
作  者：刘晓东
日  期：2018.8.29
版  本：V1.00
说  明：RF和北斗下发的数据参数转换
参  数：upper_cmd		最近一次发送的命令码
		buf				RF或北斗下发的数据
修改记录：

--------------------------------------------------------------
															  |
  分析结果全部是标准的数据值，使用时不需要再进行额外的转换    |
															  |
--------------------------------------------------------------

**************************************************/
void Upper_Data_Analy(u8 upper_cmd, u8 *buf)
{
	u8 point_index = 0;	
	u8 byte_num = 0;
	switch(upper_cmd)
	{		
		case XTJC:
		case ZTSB:
			Sys_Stat_Analy();			//分析系统目前所处的状态
			break;
		case YTSF:				//鱼体上浮
			Upper_Fish_CMD.fish_up.up_mode = buf[7] & 0x03;
			Upper_Fish_CMD.fish_up.up_dist = (float)(((buf[7]>>2 | (0x03 & buf[8])<<6) + ((buf[8]>>2)<<8)) * 200.0 / 0x3FFF);
			Upper2Slave();
			break;
		case YTXQ:				//鱼体下潜
			Upper_Fish_CMD.fish_down.down_mode = buf[7] & 0x03;
			Upper_Fish_CMD.fish_down.down_dist = (float)(((buf[7]>>2 | (0x03 & buf[8])<<6) + ((buf[8]>>2)<<8)) * 200.0 / 0x3FFF);
			Upper2Slave();
			break;
		case YTZZ:				//鱼体左转
			Upper_Fish_CMD.fish_left.left_mode = buf[7] & 0x03;
			Upper_Fish_CMD.fish_left.left_angle = (float)(((buf[7]>>2 | (0x03 & buf[8])<<6) + ((buf[8]>>2)<<8)) * 180.0 / 0x3FFF);
			Upper2Slave();
			break;
		case YTYZ:				//鱼体右转
			Upper_Fish_CMD.fish_right.right_mode = buf[7] & 0x03;
			Upper_Fish_CMD.fish_right.right_angle = (float)(((buf[7]>>2 | (0x03 & buf[8])<<6) + ((buf[8]>>2)<<8)) * 180.0 / 0x3FFF);
			Upper2Slave();
			break;
		case YTHT:				//鱼体后退
			Upper_Fish_CMD.fish_back.back_mode = buf[7] & 0x03;
			Upper_Fish_CMD.fish_back.back_dist = (float)(((buf[7]>>2 | (0x03 & buf[8])<<6) + ((buf[8]>>2)<<8)) * 200.0 / 0x3FFF);
			Upper2Slave();
			break;
		case YTHX:				//鱼体滑翔
			Upper_Fish_CMD.fish_glide.max_depth = (float)(((buf[7]) + (buf[8]<<8)) * 1000.0 / 0xFFFF);
			Upper_Fish_CMD.fish_glide.glide_dist = (float)(((buf[9]) + (buf[10]<<8)) / (float)0xFFFF * 100000.0 );
			Upper_Fish_CMD.fish_glide.glide_mode = buf[11] & 0x03;
			Upper_Fish_CMD.fish_glide.pectoral_angle = (float)(((buf[11]>>2 | (0x03 & buf[12])<<6) + ((buf[12]>>2)<<8)) * 90.0 / 0x3FFF);
			Upper2Slave();
			break;
		case YTKT:				//鱼体快速推进
			Upper_Fish_CMD.fish_fast_ahead.ahead_speed = (float)((buf[7] + ((buf[8]<<8))) *20.0  / 0xFFFF);
			Upper2Slave();
			break;
		case HLCR:				//航迹点插入
			rt_memcpy(&upper_assign_loc.assign_timestamp, &Upper_Time, sizeof(_time));					//指令下发的时间
			upper_assign_loc.loc_info.longitude_type = buf[13] >> 7;
			upper_assign_loc.loc_info.longitude = (float)((((u32)buf[7]<<16) + (buf[8]<<8) + buf[9]) * 180.0 / 0xFFFFFF);
			upper_assign_loc.loc_info.latitude_type = (buf[13] & 0x40) >> 6;
			upper_assign_loc.loc_info.latitude = (float)((((u32)buf[10]<<16) + (buf[11]<<8) + buf[12]) * 90.0 / 0xFFFFFF);
			upper_assign_loc.loc_info.depth = ((((u32)buf[13] & 0x3F)<<8) + buf[14])*1000.0 / 0x3FFFF;
			upper_assign_loc.new_loc = 1;																//指定了新的地点经纬度
			break;
		case HLGH:				//路径规划
			Upper_Fish_CMD.out_water_inter = buf[7] * 10;
			for(point_index = 0; point_index < ROUTE_NUM; ++point_index)
			{
				byte_num = 8*point_index + 8;
				Upper_Fish_CMD.fish_route[point_index].longitude_type = buf[byte_num+6] >> 7;
				Upper_Fish_CMD.fish_route[point_index].longitude = (float)((((u32)buf[byte_num]<<16) + (buf[byte_num+1]<<8) + buf[byte_num+2]) * 180.0 / 0xFFFFFF);
				Upper_Fish_CMD.fish_route[point_index].latitude_type = (buf[byte_num+6] & 0x40) >> 6;
				Upper_Fish_CMD.fish_route[point_index].latitude = (float)((((u32)buf[byte_num+3]<<16) + (buf[byte_num+4]<<8) + buf[byte_num+5]) * 90.0 / 0xFFFFFF);
				Upper_Fish_CMD.fish_route[point_index].depth = ((((u32)buf[byte_num+6] & 0x3F)<<8) + buf[byte_num+7])*1000.0 / 0x3FFFF;
			}
			break;
#ifdef __DEBUG
		case DJXQL:
			Upper_Fish_CMD.left_pectoral_move.run_mode = buf[13] & 0x3F;
			Upper_Fish_CMD.left_pectoral_move.flap_amplitude = (buf[13] & 0x80) ? (-1)*(((buf[10]<<8)+buf[9])* 90.0 / 0xFFFF) : (((buf[10]<<8)+buf[9])* 90.0 / 0xFFFF);
			Upper_Fish_CMD.left_pectoral_move.flap_fre_integer = (u8)(((buf[12]<<8)+buf[11])* 10.0 / 0xFFFF);
			Upper_Fish_CMD.left_pectoral_move.flap_fre_decimal = (u32)((((buf[12]<<8)+buf[11])* 10.0 / 0xFFFF) * 100) % 100;
			Upper_Fish_CMD.left_pectoral_move.target_location = (buf[13] & 0x40)? (-1)*(((buf[8]<<8) + buf[7]) * 180.0 / 0xFFFF) : (((buf[8]<<8) + buf[7]) * 180.0 / 0xFFFF);
			Upper2Slave();
			break;
		case DJXQR:
			Upper_Fish_CMD.right_pectoral_move.run_mode = buf[13] & 0x3F;
			Upper_Fish_CMD.right_pectoral_move.flap_amplitude = (buf[13] & 0x80) ? (-1)*(((buf[10]<<8)+buf[9])* 90.0 / 0xFFFF) : (((buf[10]<<8)+buf[9])* 90.0 / 0xFFFF);
			Upper_Fish_CMD.right_pectoral_move.flap_fre_integer = (u8)(((buf[12]<<8)+buf[11])* 10.0 / 0xFFFF);
			Upper_Fish_CMD.right_pectoral_move.flap_fre_decimal = (u32)(((((u32)buf[12]<<8)+buf[11])* 10.0 / 0xFFFF) * 100) % 100;
			Upper_Fish_CMD.right_pectoral_move.target_location = (buf[13] & 0x40)? (-1)*(((buf[8]<<8) + buf[7]) * 180.0 / 0xFFFF) : (((buf[8]<<8) + buf[7]) * 180.0 / 0xFFFF);
			Upper2Slave();
			break;
		case DJYG:
			Upper_Fish_CMD.cylinder_move.run_mode = buf[11] & 0x3F;
			Upper_Fish_CMD.cylinder_move.target_location = (buf[11] & 0x40)? (-1)*(((buf[8]<<8) + buf[7]) * 800.0 / 0xFFFF) : (((buf[8]<<8) + buf[7]) * 800.0 / 0xFFFF);
			Upper_Fish_CMD.cylinder_move.target_speed = (buf[11] & 0x80)? (-1)*(((buf[10]<<8) + buf[9]) * 800.0 / 0xFFFF) : (((buf[10]<<8) + buf[9]) * 800.0 / 0xFFFF);
			Upper2Slave();
			break;
		case DJZX:
			Upper_Fish_CMD.gravity_move.run_mode = buf[11] & 0x3F;
			Upper_Fish_CMD.gravity_move.target_location = (buf[11] & 0x40)? (-1)*(((buf[8]<<8) + buf[7]) * 1000.0 / 0xFFFF) : (((buf[8]<<8) + buf[7]) * 1000.0 / 0xFFFF);
			Upper_Fish_CMD.gravity_move.target_speed = (buf[11] & 0x80)? (-1)*(((buf[10]<<8) + buf[9]) * 1000.0 / 0xFFFF) : (((buf[10]<<8) + buf[9]) * 1000.0 / 0xFFFF);
			Upper2Slave();
			break;
		case DJYB:
			Upper_Fish_CMD.waist_move.run_mode = buf[13] & 0x3F;
			Upper_Fish_CMD.waist_move.flap_amplitude = (buf[13] & 0x80) ? (-1)*(((buf[10]<<8)+buf[9])* 90.0 / 0xFFFF) : (((buf[10]<<8)+buf[9])* 90.0 / 0xFFFF);
			Upper_Fish_CMD.waist_move.flap_fre_integer = (u8)(((buf[12]<<8)+buf[11])* 10.0 / 0xFFFF);
			Upper_Fish_CMD.waist_move.flap_fre_decimal = (u32)(((((u32)buf[12]<<8)+buf[11])* 10.0 / 0xFFFF) * 100) % 100;
			Upper_Fish_CMD.waist_move.target_location = (buf[13] & 0x40)? (-1)*(((buf[8]<<8) + buf[7]) * 90.0 / 0xFFFF) : (((buf[8]<<8) + buf[7]) * 90.0 / 0xFFFF);
			Upper2Slave();
			break;
		case DJWB:
			Upper_Fish_CMD.tail_move.run_mode = buf[13] & 0x3F;
			Upper_Fish_CMD.tail_move.flap_amplitude = (buf[13] & 0x80) ? (-1)*(((buf[10]<<8)+buf[9])* 90.0 / 0xFFFF) : (((buf[10]<<8)+buf[9])* 90.0 / 0xFFFF);
			Upper_Fish_CMD.tail_move.flap_fre_integer = (u8)(((buf[12]<<8)+buf[11])* 10.0 / 0xFFFF);
			Upper_Fish_CMD.tail_move.flap_fre_decimal = (u32)(((((u32)buf[12]<<8)+buf[11])* 10.0 / 0xFFFF) * 100) % 100;
			Upper_Fish_CMD.tail_move.target_location = (buf[13] & 0x40)? (-1)*(((buf[8]<<8) + buf[7]) * 90.0 / 0xFFFF) : (((buf[8]<<8) + buf[7]) * 90.0 / 0xFFFF);
			Upper2Slave();
			break;
#endif
		default:
			break;
	}

}
	
/**************************************************

函数名：stat_back_frame
作  者：刘晓东
日  期：2018.8.29
版  本：V1.00
说  明：状态上送组帧
参  数：buf 	数据发送缓存区
返回值：cnt		发送数据长度
修改记录：

**************************************************/
u8 stat_back_frame(u8 *buf, u8 up_type)
{
	sys_stat.sys_major_stat = 2;
	sys_stat.sys_sub_stat = 1;
	sys_stat.cabin_temp = 70;
	sys_stat.elec_quantity = 10;
	
	u8 cnt = 0;
	u16 temperature = (u16)(sys_stat.cabin_temp / 100.0 * 0x3FF);		//温度转换
	u16 quantity = (u16)(sys_stat.elec_quantity / 1000.0 * 0x3FFF);		//电量转换
	
	buf[cnt++] = '$';
	buf[cnt++] = up_type ? XTZT:ZTSB;
	buf[cnt++] = BACK_STAT_LEN | (Master_Time.month << 7);
	buf[cnt++] = (Master_Time.month >> 1) | (Master_Time.day << 3);
	buf[cnt++] = Master_Time.hour |	(Master_Time.min << 5);
	buf[cnt++] = (Master_Time.min >> 3) | (Master_Time.sec << 3);
	buf[cnt++] = (Master_Time.sec >> 5) |	((Master_Time.msec/10) << 1);
	buf[cnt++] = (sys_stat.sys_sub_stat << 3) | sys_stat.sys_major_stat;
	buf[cnt++] = sys_stat.sys_sub_stat >> 5;
	buf[cnt++] = temperature;
	buf[cnt++] = (temperature >> 8) | (quantity<<2);
	buf[cnt++] = (quantity >> 6);
	return cnt;
}

/**************************************************

函数名：track_frame
作  者：刘晓东
日  期：2018.8.30
版  本：V1.00
说  明：航迹信息组帧函数
参  数：buf 帧数据区
返回值：cnt	帧长度
修改记录：

**************************************************/
u8 track_frame(u8 *buf)
{
	u8 cnt = 0;
	u8 index = 0;
	u32 longitude_int, latitude_int;			////转化为整数的经度、纬度
	u16 depth_int;								//深度
	buf[cnt++] = '$';
	buf[cnt++] = HJXX;
	buf[cnt++] = TRACE_INFO_LEN | (Master_Time.month << 7);
	buf[cnt++] = (Master_Time.month >> 1) | (Master_Time.day << 3);
	buf[cnt++] = Master_Time.hour |	(Master_Time.min << 5);
	buf[cnt++] = (Master_Time.min >> 3) | (Master_Time.sec << 3);
	buf[cnt++] = (Master_Time.sec >> 5) |	((Master_Time.msec/10) << 1);
	for(index = 0; index < ROUTE_NUM; ++index)
	{
//		longitude_int = 1000;
//		latitude_int = 1000;
//		depth_int = 1000;
		longitude_int = (u32)(((float)0xFFFFFF) * Track_Info[index].longitude/(float)180.0);
		latitude_int = (u32)(((float)0xFFFFFF) * Track_Info[index].latitude/(float)90.0);
		depth_int = (u16)(((float)0x3FFF) * Track_Info[index].depth/(float)1000.0);
		buf[cnt++] = longitude_int;
		buf[cnt++] = longitude_int>>8;		
		buf[cnt++] = longitude_int>>16;
		buf[cnt++] = latitude_int;
		buf[cnt++] = latitude_int>>8;
		buf[cnt++] = latitude_int>>16;
		buf[cnt++] = (Track_Info[index].longitude_type & 0x01) | ((Track_Info[index].latitude_type & 0x01) << 1) | ((depth_int & 0x3F) << 2);
		buf[cnt++] = depth_int >> 6;
	}
	
	return cnt;
}

/**************************************************

函数名：Upper_Message_Analy
作  者：刘晓东
日  期：2018.8.28
版  本：V1.00
说  明：上位机下发数据解析(包括RF和北斗)

参  数：buf  上位机通过RF和北斗下发的数据
修改记录：
注  意：上位机下发数据格式如下,以位为分配存储单位：
-----------------------------------------------------------------
  8bit   |  8bit  | 7bit |4bit|5bit|5bit|6bit|6bit|7bit| ...... |
报文头($)|报文标志|帧长度| 月 | 日 | 时 | 分 | 秒 |毫秒|报文内容|
-----------------------------------------------------------------
**************************************************/
u8 Upper_Message_Analy(u8 *buf, u8 Upper_type)
{
	u8 len = 0;
	if(buf[0] != '$')
		return RT_ERROR;
	Upper_Fish_CMD.cmd_type = buf[1];							//分离出RF下发的CMD命令码
	Upper_Time.year = Master_Time.year;							//上位机下发的指令不带年份，用主机时间补充
	Upper_Time.month = (0x0F) & (buf[2] >> 7 | buf[3] << 1); 
	Upper_Time.day = buf[3] >> 3;
	Upper_Time.hour = (0x1F) & buf[4];
	Upper_Time.min = (0x3F) & (buf[4] >> 5 | buf[5] << 3);
	Upper_Time.sec = (0x3F) & (buf[5] >> 3 | buf[6] << 5);
	Upper_Time.msec = (buf[6] >> 1) * 10;					//分隔位10ms位单位，故*10
	
	switch(buf[1])
	{
		case XTJC:
			Master_Slave_Comm(ZTSB);
		case ZTSB:
			Upper_Data_Analy(buf[1], NULL);
			if(Upper_type == RF_Type)
			{
				len = stat_back_frame(RF_Send_Buf, Passive_Upload);							//系统状态帧组帧
				data_upload_upper(Upper_type, RF_Send_Buf, len);							//给上位机返回系统状态数据
			}
			else if(Upper_type == BD_Type)
			{
				len = stat_back_frame((u8*)RDSS_TX_buf, Passive_Upload);							//系统状态帧组帧
				data_upload_upper(Upper_type, (u8*)RDSS_TX_buf, len);							//给上位机返回系统状态数据
			}
			break;
		case XTTZ:												//系统停止		
			Master_Slave_Comm(XTTZ);
			break;
		case XTJS:
			Master_Slave_Comm(XTJS);					//系统校时，用北斗时间校时
			break;
		case XTFW:
			Master_Slave_Comm(XTFW);					//增加系统复位
			Feed_Flag = WDG_DISABLE;
			break;
		case YTSF:									//鱼体上浮
		case YTXQ:									//鱼体下潜
		case YTZZ:									//鱼体左转
		case YTYZ:									//鱼体右转
		case YTHT:									//鱼体后退
		case YTHX:									//鱼体滑翔
		case YTKT:									//鱼体快速推进
		case HLGH:									//鱼体航路规划
#ifdef __DEBUG
		case DJXQL:
		case DJXQR:
		case DJZX:
		case DJYB:
		case DJWB:
		case DJYG:
		case HLCR:									//鱼游动到制定位置
#endif
			Upper_Data_Analy(buf[1], buf);
		case YTHL:
		case YTZP:
			Master_Slave_Comm(buf[1]);
			break;
		default:
			break;
	}
	return RT_EOK;
}


/**************************************************

函数名：data_upload_upper
作  者：刘晓东
日  期：2018.8.30
版  本：V1.00
说  明：主机给上位机发送数据
		可以通过北斗和RF发送
参  数：type	上送方式 0:RF  1:北斗
		message	上送信息类型
		dat		待发送的数据
返回值：length	发送数据的字节数
修改记录：

**************************************************/
u8 data_upload_upper(u8 type, u8 *dat, u8 len)
{
	u8 length = 0;
	switch(type)
	{
		case RF_Type:
			length = RF_send(dat, len);
			break;
		case BD_Type:
			length = BD_RDSS_Send(dat, len);
			break;
		default:
			break;
	}
	return length;
}

/**************************************************

函数名：track_info_upper
作  者：刘晓东
日  期：2018.9.18
版  本：V1.00
说  明：鱼航迹信息上送
参  数：type	上送方式 0:RF  1:北斗
返回值：
修改记录：

**************************************************/
void track_info_upper(u8 type)
{
	u8 buf[64] = {0};
	u8 len = 0;
	len = track_frame(buf);
	data_upload_upper(type, buf, len);
}



