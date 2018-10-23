/**************************************************

文件名：navigation.c
作  者：刘晓东
日  期：2018.10.8
版  本：V1.00
说  明：导航文件
修改记录：

**************************************************/

#include "navigation.h"
#include "stdio.h"
#include "arm_math.h"
#include "math.h"


#define sqrtf(X,Y) 		arm_sqrt_f32((X),(Y))
#define sinf(X) 		arm_sin_f32(X)
#define cosf(X) 		arm_cos_f32(X)
#define tanf(X) 		tan(X)
#define atanf(X) 		atan(X)
#define log10f(X) 		log10(X)
#define ceilf(X) 		ceil(X)
#define floorf(X) 		floor(X)
#define powf(X,Y) 		pow((X),(Y))
#define fmodf(X,Y) 		fmod((X),(Y))


static struct rt_timer navi_timeout_timer;


//角度、弧度转换常量
const float angle2rad = 0.0174533;				//角度到弧度的转换
const float rad2angle = 57.29577951308233;		//弧度到角度的换算

//与时间相关常量
const u32 day_s = 86400;						//一天之内的秒数
const u16 prediv_s = 255;						//同步预分频系数，用于计算亚秒数


//距离相关常量
const float ROUNDING_ERROR = 0.000001;			//计算浮点数是否相等的精度值
const float earth_radius = 6378.137;			//地球赤道半径（单位：km）


//导航状态切换相关常量
const float disired_navi_point_distance = 10;			//期望的距离(单位：海里)
const float navi_point_swi_distance = 5;				//切换新的航迹点时的距离门槛值(单位：海里)
const float max_new_navi_point_time = 18000;			//5小时(单位：s)
const float max_tracking_time = 18000;					//5小时
const float navi_timeout_ticks = max_tracking_time *500;//导航超时的tick数



//磁偏角常量  
//西偏为负数，东偏为正数
const float declination[2] = {-6.87,-6.91};


//位置更新周期常量，单位s，
const float PosUpdatePeriod = 60.0; 			// 即达到更新记录一次当前经纬度位置及时间戳，暂设为60s


static u8 navi_stat = NaviPointTracking_NaviPointChoose;	//导航状态
static u8 navi_timeout_flag = 0;							//导航超时状态标志位 0：正常 1：超时
u8 navi_terminal_arrival_flag = 0;							//导航终点到达标志位 0：未到达 1：到达

//横偏滚相关变量
float roll_b, pitch_b, yaw_b;					//鱼体坐标下的姿态角 机体坐标系 头-右侧-下
float roll_b_pre, pitch_b_pre, yaw_b_pre;		//前一周期鱼体坐标下的姿态角


//速度相关变量
float vel_b[3];									//鱼体坐标下的三轴速度
float vel_n[3]; 								//导航坐标系下的三轴速度m/s	 // 导航坐标系：（北-东-地）
float vel_n_xy_dir; 							//速度方向
float vel_n_pre[3]; 							//前一周期导航坐标系下的三轴速度


//时间相关变量
RTC_TimeTypeDef time_navi;							//时间
RTC_DateTypeDef date_navi;							//日期

float CurrentSysTime = 0; 						//系统当前时钟
float CurrentSysTime_Pre = 0; 					//系统前一时刻时钟
float TimeInterval = 0;   						//获取两次执行Navigation线程时间间隔
float CtrlTimeElapsed = 0;  					//航位推演累计时间
float PosPeriod;								//离上一次位置更新经过的时间，单位s

float CurrLatitude;  							//机器鱼当前纬度值 d.d
float CurrLongitude; 							//机器鱼当前经度值 d.d
u32 CurrTiming;    								//机器鱼当前经纬度时间戳

float pos_n[3];									//导航坐标系下的位置

float LastBeiDouLatitude = 39.97911;  			//上一次获得准确机器鱼北斗位置纬度值 d.d
float LastBeiDouLongitude = 116.332478; 		//上一次获得准确机器鱼北斗位置经度值 d.d
u32 LastBeiDouTiming;    						//上一次获得准确机器鱼北斗位置时间戳


u8 bBD_Avail = 0;								//是否收到北斗数据
u8 bPosReconingStarted; 						//是否启动位置推演
float move_dist = 0.0;							//一个航位推演周期内，鱼的运行距离
float devi_angle = 0.0;							//一个航位推演周期内，鱼运动所到地点的偏向角


u8 position_info[62];							//保存位置信息,用于保存


_sea_chart sea_chart_info[SEA_CHART_POS_NUM];			//海图位置信息
_target_loc target_loc;									//目标地点
float target_latitude;									//目标地点经纬度
float target_longtitude;
float disire_deviation = 0.0;							//期望的偏向角
float curr_loc2target_dist = 0.0;						//当前位置距离目标点的距离



_BD_RNSS_data sysBD_position_g;					//北斗数据


/**************************************************

函数名：equals
作  者：刘晓东
日  期：2018.10.8
版  本：V1.00
说  明：判断两个浮点数是否相等
参  数：
修改记录：

**************************************************/
static u8 equals(float a, float b)
{
	return (a + ROUNDING_ERROR > b) && (a - ROUNDING_ERROR < b);
}

/**************************************************

函数名：navi_timeout
作  者：刘晓东
日  期：2018.10.12
版  本：V1.00
说  明：导航超时函数
参  数：
修改记录：

**************************************************/
void navi_timeout(void * parameter)
{
	navi_timeout_flag = 1;
}

/**************************************************

函数名：timer_reset
作  者：刘晓东
日  期：2018.10.12
版  本：V1.00
说  明：定时器复位函数
参  数：
修改记录：

**************************************************/
void timer_reset(rt_timer_t  timer)
{
	rt_timer_stop(timer);
	delay_ms(1);
	rt_timer_start(timer);
}

/**************************************************

函数名：calc_position
作  者：刘晓东
日  期：2018.10.8
版  本：V1.00
说  明：大地正算函数
参  数：u1 纬度
		w1 精度
		C  偏向角
		S  距离
		u2 目的地纬度
		w2 目的地经度
修改记录：

**************************************************/
void calc_position(float u1, float w1, float C, float S, float *u2, float *w2)
{
	float deltaU = 0.0;
	float deltaW = 0.0;

	while(u1>(float)90.0) { u1 -= (float)180.0; }
	while(u1<-(float)90.0) { u1 += (float)180.0; }
	while(w1>(float)180.0) { w1 -= (float)2.0*(float)180.0; }
	while(w1<-(float)180.0) { w1 += (float)2.0*(float)180.0; }
	while(C<(float)0.0) { C += (float)360.0; }
	while(C>(float)360.0) { C -= (float)360.0; }

	C *= angle2rad;
	deltaU = S*cosf(C);
	deltaU /= (float)60.0;
	float secU = cosf((u1+deltaU/(float)2.0)*angle2rad);
	if(equals(secU, (float)0.0))
	{
		deltaW = (float)0.0;
	}
	else
	{
		deltaW = S*sinf(C)/(float)60.0/secU;
	}
	u1 += deltaU;
	w1 += deltaW;

	while(u1>(float)90.0) { u1 -= (float)180.0; }
	while(u1<-(float)90.0) { u1 += (float)180.0; }
	while(w1>(float)180.0) { w1 -= (float)2.0*(float)180.0; }
	while(w1<-(float)180.0) { w1 += (float)2.0*(float)180.0; }

	*u2 = u1;
	*w2 = w1;

}


/**************************************************

函数名：calc_trace
作  者：刘晓东
日  期：2018.10.8
版  本：V1.00
说  明：大地反算函数
参  数：u1 初始纬度
		w1 初始经度
		u2 目的地纬度
		w2 目的地经度
		C  偏向角
		S  距离
修改记录：

**************************************************/
void calc_trace(float u1, float w1, float u2, float w2, float *C, float *S)
{
	while(u1>(float)90.0) { u1 -= (float)180.0; }
	while(u1<-(float)90.0) { u1 += (float)180.0; }
	while(w1>(float)180.0) { w1 -= (float)2.0*(float)180.0; }
	while(w1<-(float)180.0) { w1 += (float)2.0*(float)180.0; }
	while(u2>(float)90.0) { u2 -= (float)180.0; }
	while(u2<-(float)90.0) { u2 += (float)180.0; }
	while(w2>(float)180.0) { w2 -= (float)2.0*(float)180.0; }
	while(w2<-(float)180.0) { w2 += (float)2.0*(float)180.0; }

	float deltaU = 0.0;
	float deltaW = 0.0;
	float deltaD = 0.0;
	float D1 = 0.0;
	float D2 = 0.0;
	float fC = 0.0;
	float fS = 0.0;

	D1 = (float)7915.70447*log10f(tanf(((float)45.0+u1/(float)2.0)*angle2rad));
	D2 = (float)7915.70447*log10f(tanf(((float)45.0+u2/(float)2.0)*angle2rad));

	deltaU = (u2 - u1)*(float)60.0;
	deltaW = (w2 - w1)*(float)60.0;
	deltaD = D2 - D1;
	if(equals(deltaD, (float)0.0))
	{
		if(deltaW>(float)0.0) fC = (float)90.0;
		else fC = (float)270.0;
	}
	else if(equals(deltaW, (float)0.0))
	{
		if(deltaD>(float)0.0) fC = (float)0.0;
		else fC = (float)180.0;
	}
	else
	{
		fC = atanf(deltaW/deltaD)*rad2angle;
		if(deltaW>(float)0.0&&deltaD>(float)0.0) fC = fC;
		if(deltaW>(float)0.0&&deltaD<(float)0.0) fC += (float)180.0;
		if(deltaW<(float)0.0&&deltaD<(float)0.0) fC += (float)180.0;
		if(deltaW<(float)0.0&&deltaD>(float)0.0) fC += (float)360.0;
	}
	while(fC<(float)0.0) { fC += (float)360.0; }
	while(fC>(float)360.0) { fC -= (float)360.0; }

	if((fC>(float)45.0&&fC<(float)135.0)||(fC>(float)225.0&&fC<(float)315.0))
	{
		if(equals(deltaD, (float)0.0))
		{
			fS = deltaW*cosf((u1+u2)/(float)2.0*angle2rad)/sinf(fC*angle2rad);
		}
		else
		{
			fS = deltaW*(deltaU/deltaD)/sinf(fC*angle2rad);
		}
	}
	else
	{
		fS = deltaU/cosf(fC*angle2rad);	
	}
	*C = fC;
	*S = fS;
}



/**************************************************

函数名：time_trans
作  者：刘晓东
日  期：2018.10.8
版  本：V1.00
说  明：时间距离2018.1.1 0:0:0的时间(以s为单位)
参  数：
修改记录：

**************************************************/
float time_trans(RTC_DateTypeDef *date, RTC_TimeTypeDef *time, _time *master_time_type)
{
	u16 _year = 0;
	u8 _month = 0, _day = 0, _hour = 0, _min = 0, _sec = 0;
	
	u16 cnt = 0;
	u16 day_num = 0;										//不足一年时的天数
	u32 total_sec = 0;										//间隔秒数
	u8 leap_year = 0;										//闰年的年数
	u32 ssr_reg = 0;										//亚秒寄存器数值
	float ss_val = 0.0;										//亚秒数值
	u8 inter_year = 0, inter_mon = 0, inter_day = 0;
	
	if(date != NULL && time != NULL)
	{
		_year = date->RTC_Year + 2000;
		_month = date->RTC_Month;
		_day = date->RTC_Date;
		_hour = time->RTC_Hours;
		_min = time->RTC_Minutes;
		_sec = time->RTC_Seconds;
		ssr_reg = RTC_GetSubSecond();									//亚秒寄存器数值
		ss_val = (float)(prediv_s - ssr_reg) /((prediv_s + 1) * 1.0);	//亚秒数
	}
	else if(master_time_type != NULL)
	{
		_year = master_time_type->year;
		_month = master_time_type->month;
		_day = master_time_type->day;
		_hour = master_time_type->hour;
		_min = master_time_type->min;
		_sec = master_time_type->sec;
		ss_val = (float)master_time_type->msec / 1000;
	}
	else
		return 0.0;
	
	if(_year < 18)
		return 0;
	
	for(cnt = 2018;cnt <= _year; ++ cnt)	//判断闰年数
		if(isLeap(cnt))
			++leap_year;
		
	if(_month <= 2)								//判断是否过了2月
		--leap_year;
	inter_year = _year - 2018;
	inter_mon = _month - 1;
	inter_day = _day - 1;
	for(cnt = 0; cnt < inter_mon; ++cnt)
	{
		day_num = day_per_month[cnt];
	}
	day_num += inter_day;
	total_sec = (inter_year * 365 + day_num + leap_year) * day_s + _hour * 3600 + _min * 60 + _sec;
	
	
	return (float)total_sec + ss_val;								
}

/**************************************************

函数名：angle_trans
作  者：刘晓东
日  期：2018.10.8
版  本：V1.00
说  明：笛卡尔坐标系下转换为地磁北-东-地坐标系
参  数：
修改记录：

**************************************************/
float angle_trans(float angle)
{
	float magn_decl;				//磁偏角
	float trans_angle;				//转换后的角度
#ifdef  BEIJING																	//补偿偏向角
	magn_decl = declination[BJ];
#endif	
#ifdef  TIANJIN
	magn_decl = declination[TJ];
#endif
	
	if(equals(angle, (float)0.0))
		trans_angle = 90;
	else if(equals(angle, (float)90.0))
		trans_angle = 0;
	else if(equals(angle, (float)180.0))
		trans_angle = 270;
	else if(equals(angle, (float)-90.0))
		trans_angle = 180;
	
	else if((angle > (float)0.0) && angle < (float)90.0)					//在第一象限
		trans_angle = 90 - angle;
	else if((angle > (float)90.0 ) && angle < (float)180.0)
		trans_angle = 540-angle;	
	else if((angle > (float)-180.0) && angle < (float)-90.0)
		trans_angle = 90 - angle;
	else if((angle > (float)-90.0) && angle < (float)0.0)
		trans_angle = 90 - angle;
	
	trans_angle += magn_decl;
	
	if(trans_angle < (float)0.0 && !equals(trans_angle, 0.0))					//偏向角符号变化之后的转换
		trans_angle += (float)360.0;
	
	return trans_angle;
}

/**************************************************

函数名：calc_diatance
作  者：刘晓东
日  期：2018.10.11
版  本：V1.00
说  明：google提供，由经纬度计算两点之间的距离
参  数：u1 初始纬度
		w1 初始经度
		u2 目的地纬度
		w2 目的地经度
返回值：s:两点之间的距离(单位：海里)
修改记录：

**************************************************/
float calc_diatance(float u1, float w1, float u2, float w2)
{
	float s = 0.0;
	float temp_val =  0.0;
	float rad_u1 = u1 * angle2rad;
	float rad_u2 = u2 * angle2rad;
	float u_change = rad_u1 - rad_u2;
	float w_change = w1 * angle2rad - w2 * angle2rad;
	
	sqrtf(powf(sinf(u_change/2), 2) + cosf(rad_u1) * cosf(rad_u2) + powf(sinf(w_change/2), 2), &temp_val);
	s = 2 * asin(temp_val) * earth_radius;
	s = (float)lroundf(s * 10000) / 10000;
	s /= 1.852; 
	return s;
}

/**************************************************

函数名：search_closet_site
作  者：刘晓东
日  期：2018.10.11
版  本：V1.00
说  明：查找距离当前点最近的海图地点
参  数：start:开始查找的索引
返回值：
修改记录：

**************************************************/
void search_closet_site(u16 start)
{
	u16 cnt = 0;
	float temp_dist = 0.0;											//两个位置之间的距离
	float dist_diff_pre = earth_radius / 1.852;						//上一点距离差值
	float dist_diff_cur = 0.0;										//当前点距离差值
	for(cnt = start; cnt < SEA_CHART_POS_NUM; ++cnt)
	{
		temp_dist = calc_diatance(CurrLatitude, CurrLongitude, sea_chart_info[cnt].sea_chart_latitude,sea_chart_info[cnt].sea_chart_longtitude);
		dist_diff_cur = (float)fabs(temp_dist - disired_navi_point_distance);
		if(dist_diff_cur < dist_diff_pre)
		{
			target_loc.index = cnt;
			target_loc.distance = temp_dist;
			dist_diff_pre = dist_diff_cur;
		}
	}
	target_latitude = sea_chart_info[target_loc.index].sea_chart_latitude;
	target_longtitude = sea_chart_info[target_loc.index].sea_chart_longtitude;
}
/**************************************************

函数名：calc_location
作  者：刘晓东
日  期：2018.10.8
版  本：V1.00
说  明：鱼当前位置计算
参  数：
修改记录：

**************************************************/
void calc_location(void)
{	
	static u16 cnt = 0;
	u8 buf[46];
	RTC_GetTime(RTC_Format_BIN, &time_navi);		//得到时间
	RTC_GetDate(RTC_Format_BIN, &date_navi);		//得到日期
	
	CurrentSysTime = time_trans(&date_navi, &time_navi,NULL);
	if(CurrentSysTime_Pre < 0.01)
		TimeInterval = 0.0;
	else
		TimeInterval = CurrentSysTime - CurrentSysTime_Pre;
	CurrentSysTime_Pre = CurrentSysTime;
	
	roll_b = attitude.roll;
	pitch_b = attitude.pitch;
	yaw_b = attitude.yaw;
	vel_b[0] = velocity.vel_x;
	vel_b[1] = velocity.vel_y;
	vel_b[2] = velocity.vel_z;
	
	if((bBD_Avail = sysBD_position_g.data_state) == 1)
	{
		LastBeiDouLatitude = sysBD_position_g.location.latitude;		//北斗的纬度信息
		LastBeiDouLongitude = sysBD_position_g.location.longitude;		//北斗的经度信息
		date_navi.RTC_Year = sysBD_position_g.time.year;					//北斗时间转换为可计算的RTC格式时间
		date_navi.RTC_Month = sysBD_position_g.time.month;
		date_navi.RTC_Date = sysBD_position_g.time.day;
		time_navi.RTC_Hours = sysBD_position_g.time.hour;
		time_navi.RTC_Minutes = sysBD_position_g.time.min;
		time_navi.RTC_Seconds = sysBD_position_g.time.sec;
		LastBeiDouTiming = time_trans(&date_navi, &time_navi, NULL);				//计算北斗时间距2018.1.1 0:0:0的时长(单位为s)
	}
	vel_n[0] = arm_cos_f32(yaw_b * angle2rad)*arm_cos_f32(pitch_b * angle2rad)*vel_b[0]
	+ (arm_cos_f32(yaw_b * angle2rad)*arm_sin_f32(pitch_b * angle2rad)*arm_sin_f32(roll_b * angle2rad) - arm_sin_f32(yaw_b * angle2rad)*arm_cos_f32(roll_b * angle2rad))*vel_b[1]
	+ (arm_cos_f32(yaw_b * angle2rad)*arm_sin_f32(pitch_b * angle2rad)*arm_cos_f32(roll_b * angle2rad) + arm_sin_f32(yaw_b * angle2rad)*arm_sin_f32(roll_b * angle2rad))*vel_b[2];
	
	vel_n[1] = arm_sin_f32(yaw_b * angle2rad)*arm_cos_f32(pitch_b * angle2rad)*vel_b[0]
	+ (arm_sin_f32(yaw_b * angle2rad)*arm_sin_f32(pitch_b * angle2rad)*arm_sin_f32(roll_b * angle2rad) + arm_cos_f32(yaw_b * angle2rad)*arm_cos_f32(roll_b * angle2rad))*vel_b[1]
	+ (arm_sin_f32(yaw_b * angle2rad)*arm_sin_f32(pitch_b * angle2rad)*arm_cos_f32(roll_b * angle2rad) - arm_cos_f32(yaw_b * angle2rad)*arm_sin_f32(roll_b * angle2rad))*vel_b[2];

	vel_n[2] = -arm_sin_f32(pitch_b * angle2rad)*vel_b[0]
	+ arm_cos_f32(pitch_b * angle2rad)*arm_sin_f32(roll_b * angle2rad)*vel_b[1]
	+ arm_cos_f32(pitch_b * angle2rad)*arm_cos_f32(roll_b * angle2rad)*vel_b[2];
	
	vel_n_xy_dir = atan2(vel_n[1], vel_n[0]) * rad2angle;						//计算出速度方向
	devi_angle = angle_trans(vel_n_xy_dir);									//坐标系转换 补偿偏向角

	
	if(bBD_Avail)
		bPosReconingStarted = 0; 												//不启动航位推算
	else
		bPosReconingStarted = 1;												//启动航位推算
	
	if(!bPosReconingStarted)
	{
		CurrLatitude = LastBeiDouLatitude; 										//当前位置经纬度为北斗给出的位置经纬度
		CurrLongitude = LastBeiDouLongitude;
		CurrTiming = LastBeiDouTiming;
		
		pos_n[0] = 0.0;  										// 起始时刻导航坐标系下的位置 
		pos_n[1] = 0.0;
		pos_n[2] = 0.0;
		
		roll_b_pre = roll_b;  									//保存上一次横滚偏数据
		pitch_b_pre = pitch_b;
		yaw_b_pre = yaw_b;

		vel_n_pre[0] = vel_n[0];								//保存导航坐标系下的速度
		vel_n_pre[1] = vel_n[1];
		vel_n_pre[2] = vel_n[2];

		PosPeriod = 0.0; 										//位置更新周期
		CtrlTimeElapsed = 0.0;									//航位推演累计时间
	}
	else if(bPosReconingStarted == 1)
	{
		CtrlTimeElapsed += TimeInterval;						//航位推演累计时间
		PosPeriod += TimeInterval;

		roll_b_pre = roll_b;
		pitch_b_pre = pitch_b;
		yaw_b_pre = yaw_b;
		
		
		//根据速度更新当前航行器在初始导航坐标系下的位置
//		if(cnt++ < 200)
//		{
			pos_n[0] += (vel_n[0] + vel_n_pre[0])*TimeInterval/2;  //根据当前时刻速度和上一时刻速度更新位置
			pos_n[1] += (vel_n[1] + vel_n_pre[1])*TimeInterval/2;
			pos_n[2] += (vel_n[2] + vel_n_pre[2])*TimeInterval/2;
//		}
//		else
//		{	cnt = 0;
			sprintf((char*)buf, "pos_0:%8.5f,pos_1:%8.5f,pos_02:%8.5f\r\n",pos_n[0],pos_n[1],pos_n[2]);
			rt_kprintf((char*)buf);
//			pos_n[0] = 0;
//			pos_n[1] = 0;
//			pos_n[2] = 0;
//		}
		
		vel_n_pre[0] = vel_n[0];
		vel_n_pre[1] = vel_n[1];
		vel_n_pre[2] = vel_n[2];
		if(PosPeriod > PosUpdatePeriod && equals(PosPeriod, PosUpdatePeriod))
		{
//			vel_n_xy_dir = atan2(vel_n[1], vel_n[0]) * rad2angle;						//计算出速度方向
//			devi_angle = angle_trans(vel_n_xy_dir);									//坐标系转换 补偿偏向角
			
			sqrtf(pos_n[0] * pos_n[0] + pos_n[1] * pos_n[1],&move_dist);
			calc_position(LastBeiDouLatitude, LastBeiDouLongitude, devi_angle, move_dist,&CurrLatitude, &CurrLongitude);
			CurrTiming = CurrentSysTime;											//当前时间为系统时间
			
			sprintf((char*)position_info, "20%02d-%02d-%02d, %02d:%02d:%02d",date_navi.RTC_Year, date_navi.RTC_Month, \
				date_navi.RTC_Date, time_navi.RTC_Hours, time_navi.RTC_Minutes, time_navi.RTC_Seconds);
			sprintf((char*)&position_info[20], "  latitude:%08.5f, longitude:%09.5f\r\n",CurrLatitude,CurrLongitude);
			info_record(location_info,position_info,sizeof(position_info));			//记录位置信息
		}
		else
		{
		}
	}
}

/**************************************************

函数名：calc_target_site
作  者：刘晓东
日  期：2018.10.8
版  本：V1.00
说  明：计算鱼目标位置
参  数：
修改记录：

**************************************************/
void calc_target_site(void)
{
	float cur2target_dist;									//当前位置与目标位置的距离(单位：海里)
	switch(navi_stat)
	{
		case NaviPointTracking_NaviPointChoose:
			if(navi_terminal_arrival_flag == 0)								//鱼未到达终点
			{
				search_closet_site(0);										//寻找最近的点
				if(target_loc.index + 1 == SEA_CHART_POS_NUM)				//最后一个航迹点，切换到终点状态
					navi_stat = NaviPointTracking_NaviEndPoint;
				else
					navi_stat = NaviPointTracking_NaviPointTracking;
				rt_timer_start(&navi_timeout_timer);						//启动导航定时器
			}
			break;
		
		case NaviPointTracking_NaviPointTracking:
			/*上位机是否下发指定位置*/
			if(upper_assign_loc.new_loc == 1 && (CurrentSysTime - time_trans(NULL, NULL, &upper_assign_loc.assign_timestamp)) < max_tracking_time)
			{
				rt_timer_stop(&navi_timeout_timer);						//停止导航定时器
				target_latitude = upper_assign_loc.loc_info.latitude;
				target_longtitude = upper_assign_loc.loc_info.longitude;
				navi_timeout_flag = 0;
				navi_stat = NaviPointTracking_NewNaviPointTracking;
				rt_timer_start(&navi_timeout_timer);					//启动导航定时器
				break;
			}
			else														//上位机没有下发位置 或者 下发时间超时
				upper_assign_loc.new_loc = 0;
			/*导航是否超时*/
			if(navi_timeout_flag == 1)
			{
				navi_stat = NaviPointTracking_NaviPointChoose;								//导航超时状态
				navi_timeout_flag = 0;
				break;
			}
			//与目标点距离是否在5海里之内
			cur2target_dist = calc_diatance(CurrLatitude, CurrLongitude, target_latitude,\
									target_longtitude );
			if(cur2target_dist <= navi_point_swi_distance)
			{
				navi_stat = NaviPointTracking_NaviPointSwi;
				navi_timeout_flag = 0;
				rt_timer_stop(&navi_timeout_timer);						//停止导航定时器
			}
			break;
			
		case NaviPointTracking_NaviPointSwi:
			++target_loc.index;
			target_latitude = sea_chart_info[target_loc.index].sea_chart_latitude;
			target_longtitude = sea_chart_info[target_loc.index].sea_chart_longtitude;
			if(target_loc.index + 1 == SEA_CHART_POS_NUM)				//最后一个航迹点，切换到终点状态
				navi_stat = NaviPointTracking_NaviEndPoint;
			else
				navi_stat = NaviPointTracking_NaviPointTracking;
			rt_timer_start(&navi_timeout_timer);						//启动导航定时器
			break;
		
		case NaviPointTracking_NewNaviPointTracking:
			if(upper_assign_loc.new_loc == 1 && (CurrentSysTime - time_trans(NULL, NULL, &upper_assign_loc.assign_timestamp)) < max_tracking_time)
			{
				rt_timer_stop(&navi_timeout_timer);						//停止导航定时器
				target_latitude = upper_assign_loc.loc_info.latitude;
				target_longtitude = upper_assign_loc.loc_info.longitude;
				navi_timeout_flag = 0;
				rt_timer_start(&navi_timeout_timer);					//启动导航定时器
				break;
			}
			else														//上位机没有下发位置 或者 下发时间超时
				upper_assign_loc.new_loc = 0;
			
			//导航是否超时
			if(navi_timeout_flag == 1)
			{
				navi_stat = NaviPointTracking_NaviPointChoose;								//导航超时状态
				navi_timeout_flag = 0;
				break;
			}
			
			//与目标点距离是否在5海里之内
			cur2target_dist = calc_diatance(CurrLatitude, CurrLongitude, target_latitude,\
									target_longtitude );
			if(cur2target_dist <= navi_point_swi_distance)
			{
				navi_stat = NaviPointTracking_NaviPointChoose;
				navi_timeout_flag = 0;
				rt_timer_stop(&navi_timeout_timer);						//停止导航定时器
			}
			break;
			
		case NaviPointTracking_NaviEndPoint:
			if(navi_timeout_flag == 1)
			{
				navi_stat = NaviPointTracking_NaviPointChoose;								//导航超时状态
				navi_timeout_flag = 0;
				break;
			}
			cur2target_dist = calc_diatance(CurrLatitude, CurrLongitude, target_latitude,\
									target_longtitude );
			if(cur2target_dist <= navi_point_swi_distance)
			{
				navi_terminal_arrival_flag = 1;												//鱼到达终点
				navi_stat = NaviPointTracking_NaviPointChoose;								//切换到初始化状态
			}
			break;
		default:
			break;
	}
}

/**************************************************

函数名：Navigation_entry
作  者：刘晓东
日  期：2018.10.8
版  本：V1.00
说  明：导航算法函数
参  数：
修改记录：

**************************************************/
void Navigation_entry(void* parameter)
{	
	
	rt_timer_init(&navi_timeout_timer, "navi_timeout_timer", navi_timeout, RT_NULL, navi_timeout_ticks, RT_TIMER_FLAG_SOFT_TIMER | RT_TIMER_FLAG_ONE_SHOT);
	while(1)
	{
		calc_location();
		calc_target_site();
		calc_trace(CurrLatitude, CurrLongitude,target_latitude, target_longtitude, &disire_deviation, &curr_loc2target_dist);
		//更新 偏向角、速度方向、海洋深度、当前位置经纬度
	}
}


