/**************************************************

�ļ�����navigation.c
��  �ߣ�������
��  �ڣ�2018.10.8
��  ����V1.00
˵  ���������ļ�
�޸ļ�¼��

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


//�Ƕȡ�����ת������
const float angle2rad = 0.0174533;				//�Ƕȵ����ȵ�ת��
const float rad2angle = 57.29577951308233;		//���ȵ��ǶȵĻ���

//��ʱ����س���
const u32 day_s = 86400;						//һ��֮�ڵ�����
const u16 prediv_s = 255;						//ͬ��Ԥ��Ƶϵ�������ڼ���������


//������س���
const float ROUNDING_ERROR = 0.000001;			//���㸡�����Ƿ���ȵľ���ֵ
const float earth_radius = 6378.137;			//�������뾶����λ��km��


//����״̬�л���س���
const float disired_navi_point_distance = 10;			//�����ľ���(��λ������)
const float navi_point_swi_distance = 5;				//�л��µĺ�����ʱ�ľ����ż�ֵ(��λ������)
const float max_new_navi_point_time = 18000;			//5Сʱ(��λ��s)
const float max_tracking_time = 18000;					//5Сʱ
const float navi_timeout_ticks = max_tracking_time *500;//������ʱ��tick��



//��ƫ�ǳ���  
//��ƫΪ��������ƫΪ����
const float declination[2] = {-6.87,-6.91};


//λ�ø������ڳ�������λs��
const float PosUpdatePeriod = 60.0; 			// ���ﵽ���¼�¼һ�ε�ǰ��γ��λ�ü�ʱ���������Ϊ60s


static u8 navi_stat = NaviPointTracking_NaviPointChoose;	//����״̬
static u8 navi_timeout_flag = 0;							//������ʱ״̬��־λ 0������ 1����ʱ
u8 navi_terminal_arrival_flag = 0;							//�����յ㵽���־λ 0��δ���� 1������

//��ƫ����ر���
float roll_b, pitch_b, yaw_b;					//���������µ���̬�� ��������ϵ ͷ-�Ҳ�-��
float roll_b_pre, pitch_b_pre, yaw_b_pre;		//ǰһ�������������µ���̬��


//�ٶ���ر���
float vel_b[3];									//���������µ������ٶ�
float vel_n[3]; 								//��������ϵ�µ������ٶ�m/s	 // ��������ϵ������-��-�أ�
float vel_n_xy_dir; 							//�ٶȷ���
float vel_n_pre[3]; 							//ǰһ���ڵ�������ϵ�µ������ٶ�


//ʱ����ر���
RTC_TimeTypeDef time_navi;							//ʱ��
RTC_DateTypeDef date_navi;							//����

float CurrentSysTime = 0; 						//ϵͳ��ǰʱ��
float CurrentSysTime_Pre = 0; 					//ϵͳǰһʱ��ʱ��
float TimeInterval = 0;   						//��ȡ����ִ��Navigation�߳�ʱ����
float CtrlTimeElapsed = 0;  					//��λ�����ۼ�ʱ��
float PosPeriod;								//����һ��λ�ø��¾�����ʱ�䣬��λs

float CurrLatitude;  							//�����㵱ǰγ��ֵ d.d
float CurrLongitude; 							//�����㵱ǰ����ֵ d.d
u32 CurrTiming;    								//�����㵱ǰ��γ��ʱ���

float pos_n[3];									//��������ϵ�µ�λ��

float LastBeiDouLatitude = 39.97911;  			//��һ�λ��׼ȷ�����㱱��λ��γ��ֵ d.d
float LastBeiDouLongitude = 116.332478; 		//��һ�λ��׼ȷ�����㱱��λ�þ���ֵ d.d
u32 LastBeiDouTiming;    						//��һ�λ��׼ȷ�����㱱��λ��ʱ���


u8 bBD_Avail = 0;								//�Ƿ��յ���������
u8 bPosReconingStarted; 						//�Ƿ�����λ������
float move_dist = 0.0;							//һ����λ���������ڣ�������о���
float devi_angle = 0.0;							//һ����λ���������ڣ����˶������ص��ƫ���


u8 position_info[62];							//����λ����Ϣ,���ڱ���


_sea_chart sea_chart_info[SEA_CHART_POS_NUM];			//��ͼλ����Ϣ
_target_loc target_loc;									//Ŀ��ص�
float target_latitude;									//Ŀ��ص㾭γ��
float target_longtitude;
float disire_deviation = 0.0;							//������ƫ���
float curr_loc2target_dist = 0.0;						//��ǰλ�þ���Ŀ���ľ���



_BD_RNSS_data sysBD_position_g;					//��������


/**************************************************

��������equals
��  �ߣ�������
��  �ڣ�2018.10.8
��  ����V1.00
˵  �����ж������������Ƿ����
��  ����
�޸ļ�¼��

**************************************************/
static u8 equals(float a, float b)
{
	return (a + ROUNDING_ERROR > b) && (a - ROUNDING_ERROR < b);
}

/**************************************************

��������navi_timeout
��  �ߣ�������
��  �ڣ�2018.10.12
��  ����V1.00
˵  ����������ʱ����
��  ����
�޸ļ�¼��

**************************************************/
void navi_timeout(void * parameter)
{
	navi_timeout_flag = 1;
}

/**************************************************

��������timer_reset
��  �ߣ�������
��  �ڣ�2018.10.12
��  ����V1.00
˵  ������ʱ����λ����
��  ����
�޸ļ�¼��

**************************************************/
void timer_reset(rt_timer_t  timer)
{
	rt_timer_stop(timer);
	delay_ms(1);
	rt_timer_start(timer);
}

/**************************************************

��������calc_position
��  �ߣ�������
��  �ڣ�2018.10.8
��  ����V1.00
˵  ����������㺯��
��  ����u1 γ��
		w1 ����
		C  ƫ���
		S  ����
		u2 Ŀ�ĵ�γ��
		w2 Ŀ�ĵؾ���
�޸ļ�¼��

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

��������calc_trace
��  �ߣ�������
��  �ڣ�2018.10.8
��  ����V1.00
˵  ������ط��㺯��
��  ����u1 ��ʼγ��
		w1 ��ʼ����
		u2 Ŀ�ĵ�γ��
		w2 Ŀ�ĵؾ���
		C  ƫ���
		S  ����
�޸ļ�¼��

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

��������time_trans
��  �ߣ�������
��  �ڣ�2018.10.8
��  ����V1.00
˵  ����ʱ�����2018.1.1 0:0:0��ʱ��(��sΪ��λ)
��  ����
�޸ļ�¼��

**************************************************/
float time_trans(RTC_DateTypeDef *date, RTC_TimeTypeDef *time, _time *master_time_type)
{
	u16 _year = 0;
	u8 _month = 0, _day = 0, _hour = 0, _min = 0, _sec = 0;
	
	u16 cnt = 0;
	u16 day_num = 0;										//����һ��ʱ������
	u32 total_sec = 0;										//�������
	u8 leap_year = 0;										//���������
	u32 ssr_reg = 0;										//����Ĵ�����ֵ
	float ss_val = 0.0;										//������ֵ
	u8 inter_year = 0, inter_mon = 0, inter_day = 0;
	
	if(date != NULL && time != NULL)
	{
		_year = date->RTC_Year + 2000;
		_month = date->RTC_Month;
		_day = date->RTC_Date;
		_hour = time->RTC_Hours;
		_min = time->RTC_Minutes;
		_sec = time->RTC_Seconds;
		ssr_reg = RTC_GetSubSecond();									//����Ĵ�����ֵ
		ss_val = (float)(prediv_s - ssr_reg) /((prediv_s + 1) * 1.0);	//������
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
	
	for(cnt = 2018;cnt <= _year; ++ cnt)	//�ж�������
		if(isLeap(cnt))
			++leap_year;
		
	if(_month <= 2)								//�ж��Ƿ����2��
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

��������angle_trans
��  �ߣ�������
��  �ڣ�2018.10.8
��  ����V1.00
˵  �����ѿ�������ϵ��ת��Ϊ�شű�-��-������ϵ
��  ����
�޸ļ�¼��

**************************************************/
float angle_trans(float angle)
{
	float magn_decl;				//��ƫ��
	float trans_angle;				//ת����ĽǶ�
#ifdef  BEIJING																	//����ƫ���
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
	
	else if((angle > (float)0.0) && angle < (float)90.0)					//�ڵ�һ����
		trans_angle = 90 - angle;
	else if((angle > (float)90.0 ) && angle < (float)180.0)
		trans_angle = 540-angle;	
	else if((angle > (float)-180.0) && angle < (float)-90.0)
		trans_angle = 90 - angle;
	else if((angle > (float)-90.0) && angle < (float)0.0)
		trans_angle = 90 - angle;
	
	trans_angle += magn_decl;
	
	if(trans_angle < (float)0.0 && !equals(trans_angle, 0.0))					//ƫ��Ƿ��ű仯֮���ת��
		trans_angle += (float)360.0;
	
	return trans_angle;
}

/**************************************************

��������calc_diatance
��  �ߣ�������
��  �ڣ�2018.10.11
��  ����V1.00
˵  ����google�ṩ���ɾ�γ�ȼ�������֮��ľ���
��  ����u1 ��ʼγ��
		w1 ��ʼ����
		u2 Ŀ�ĵ�γ��
		w2 Ŀ�ĵؾ���
����ֵ��s:����֮��ľ���(��λ������)
�޸ļ�¼��

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

��������search_closet_site
��  �ߣ�������
��  �ڣ�2018.10.11
��  ����V1.00
˵  �������Ҿ��뵱ǰ������ĺ�ͼ�ص�
��  ����start:��ʼ���ҵ�����
����ֵ��
�޸ļ�¼��

**************************************************/
void search_closet_site(u16 start)
{
	u16 cnt = 0;
	float temp_dist = 0.0;											//����λ��֮��ľ���
	float dist_diff_pre = earth_radius / 1.852;						//��һ������ֵ
	float dist_diff_cur = 0.0;										//��ǰ������ֵ
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

��������calc_location
��  �ߣ�������
��  �ڣ�2018.10.8
��  ����V1.00
˵  �����㵱ǰλ�ü���
��  ����
�޸ļ�¼��

**************************************************/
void calc_location(void)
{	
	static u16 cnt = 0;
	u8 buf[46];
	RTC_GetTime(RTC_Format_BIN, &time_navi);		//�õ�ʱ��
	RTC_GetDate(RTC_Format_BIN, &date_navi);		//�õ�����
	
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
		LastBeiDouLatitude = sysBD_position_g.location.latitude;		//������γ����Ϣ
		LastBeiDouLongitude = sysBD_position_g.location.longitude;		//�����ľ�����Ϣ
		date_navi.RTC_Year = sysBD_position_g.time.year;					//����ʱ��ת��Ϊ�ɼ����RTC��ʽʱ��
		date_navi.RTC_Month = sysBD_position_g.time.month;
		date_navi.RTC_Date = sysBD_position_g.time.day;
		time_navi.RTC_Hours = sysBD_position_g.time.hour;
		time_navi.RTC_Minutes = sysBD_position_g.time.min;
		time_navi.RTC_Seconds = sysBD_position_g.time.sec;
		LastBeiDouTiming = time_trans(&date_navi, &time_navi, NULL);				//���㱱��ʱ���2018.1.1 0:0:0��ʱ��(��λΪs)
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
	
	vel_n_xy_dir = atan2(vel_n[1], vel_n[0]) * rad2angle;						//������ٶȷ���
	devi_angle = angle_trans(vel_n_xy_dir);									//����ϵת�� ����ƫ���

	
	if(bBD_Avail)
		bPosReconingStarted = 0; 												//��������λ����
	else
		bPosReconingStarted = 1;												//������λ����
	
	if(!bPosReconingStarted)
	{
		CurrLatitude = LastBeiDouLatitude; 										//��ǰλ�þ�γ��Ϊ����������λ�þ�γ��
		CurrLongitude = LastBeiDouLongitude;
		CurrTiming = LastBeiDouTiming;
		
		pos_n[0] = 0.0;  										// ��ʼʱ�̵�������ϵ�µ�λ�� 
		pos_n[1] = 0.0;
		pos_n[2] = 0.0;
		
		roll_b_pre = roll_b;  									//������һ�κ��ƫ����
		pitch_b_pre = pitch_b;
		yaw_b_pre = yaw_b;

		vel_n_pre[0] = vel_n[0];								//���浼������ϵ�µ��ٶ�
		vel_n_pre[1] = vel_n[1];
		vel_n_pre[2] = vel_n[2];

		PosPeriod = 0.0; 										//λ�ø�������
		CtrlTimeElapsed = 0.0;									//��λ�����ۼ�ʱ��
	}
	else if(bPosReconingStarted == 1)
	{
		CtrlTimeElapsed += TimeInterval;						//��λ�����ۼ�ʱ��
		PosPeriod += TimeInterval;

		roll_b_pre = roll_b;
		pitch_b_pre = pitch_b;
		yaw_b_pre = yaw_b;
		
		
		//�����ٶȸ��µ�ǰ�������ڳ�ʼ��������ϵ�µ�λ��
//		if(cnt++ < 200)
//		{
			pos_n[0] += (vel_n[0] + vel_n_pre[0])*TimeInterval/2;  //���ݵ�ǰʱ���ٶȺ���һʱ���ٶȸ���λ��
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
//			vel_n_xy_dir = atan2(vel_n[1], vel_n[0]) * rad2angle;						//������ٶȷ���
//			devi_angle = angle_trans(vel_n_xy_dir);									//����ϵת�� ����ƫ���
			
			sqrtf(pos_n[0] * pos_n[0] + pos_n[1] * pos_n[1],&move_dist);
			calc_position(LastBeiDouLatitude, LastBeiDouLongitude, devi_angle, move_dist,&CurrLatitude, &CurrLongitude);
			CurrTiming = CurrentSysTime;											//��ǰʱ��Ϊϵͳʱ��
			
			sprintf((char*)position_info, "20%02d-%02d-%02d, %02d:%02d:%02d",date_navi.RTC_Year, date_navi.RTC_Month, \
				date_navi.RTC_Date, time_navi.RTC_Hours, time_navi.RTC_Minutes, time_navi.RTC_Seconds);
			sprintf((char*)&position_info[20], "  latitude:%08.5f, longitude:%09.5f\r\n",CurrLatitude,CurrLongitude);
			info_record(location_info,position_info,sizeof(position_info));			//��¼λ����Ϣ
		}
		else
		{
		}
	}
}

/**************************************************

��������calc_target_site
��  �ߣ�������
��  �ڣ�2018.10.8
��  ����V1.00
˵  ����������Ŀ��λ��
��  ����
�޸ļ�¼��

**************************************************/
void calc_target_site(void)
{
	float cur2target_dist;									//��ǰλ����Ŀ��λ�õľ���(��λ������)
	switch(navi_stat)
	{
		case NaviPointTracking_NaviPointChoose:
			if(navi_terminal_arrival_flag == 0)								//��δ�����յ�
			{
				search_closet_site(0);										//Ѱ������ĵ�
				if(target_loc.index + 1 == SEA_CHART_POS_NUM)				//���һ�������㣬�л����յ�״̬
					navi_stat = NaviPointTracking_NaviEndPoint;
				else
					navi_stat = NaviPointTracking_NaviPointTracking;
				rt_timer_start(&navi_timeout_timer);						//����������ʱ��
			}
			break;
		
		case NaviPointTracking_NaviPointTracking:
			/*��λ���Ƿ��·�ָ��λ��*/
			if(upper_assign_loc.new_loc == 1 && (CurrentSysTime - time_trans(NULL, NULL, &upper_assign_loc.assign_timestamp)) < max_tracking_time)
			{
				rt_timer_stop(&navi_timeout_timer);						//ֹͣ������ʱ��
				target_latitude = upper_assign_loc.loc_info.latitude;
				target_longtitude = upper_assign_loc.loc_info.longitude;
				navi_timeout_flag = 0;
				navi_stat = NaviPointTracking_NewNaviPointTracking;
				rt_timer_start(&navi_timeout_timer);					//����������ʱ��
				break;
			}
			else														//��λ��û���·�λ�� ���� �·�ʱ�䳬ʱ
				upper_assign_loc.new_loc = 0;
			/*�����Ƿ�ʱ*/
			if(navi_timeout_flag == 1)
			{
				navi_stat = NaviPointTracking_NaviPointChoose;								//������ʱ״̬
				navi_timeout_flag = 0;
				break;
			}
			//��Ŀ�������Ƿ���5����֮��
			cur2target_dist = calc_diatance(CurrLatitude, CurrLongitude, target_latitude,\
									target_longtitude );
			if(cur2target_dist <= navi_point_swi_distance)
			{
				navi_stat = NaviPointTracking_NaviPointSwi;
				navi_timeout_flag = 0;
				rt_timer_stop(&navi_timeout_timer);						//ֹͣ������ʱ��
			}
			break;
			
		case NaviPointTracking_NaviPointSwi:
			++target_loc.index;
			target_latitude = sea_chart_info[target_loc.index].sea_chart_latitude;
			target_longtitude = sea_chart_info[target_loc.index].sea_chart_longtitude;
			if(target_loc.index + 1 == SEA_CHART_POS_NUM)				//���һ�������㣬�л����յ�״̬
				navi_stat = NaviPointTracking_NaviEndPoint;
			else
				navi_stat = NaviPointTracking_NaviPointTracking;
			rt_timer_start(&navi_timeout_timer);						//����������ʱ��
			break;
		
		case NaviPointTracking_NewNaviPointTracking:
			if(upper_assign_loc.new_loc == 1 && (CurrentSysTime - time_trans(NULL, NULL, &upper_assign_loc.assign_timestamp)) < max_tracking_time)
			{
				rt_timer_stop(&navi_timeout_timer);						//ֹͣ������ʱ��
				target_latitude = upper_assign_loc.loc_info.latitude;
				target_longtitude = upper_assign_loc.loc_info.longitude;
				navi_timeout_flag = 0;
				rt_timer_start(&navi_timeout_timer);					//����������ʱ��
				break;
			}
			else														//��λ��û���·�λ�� ���� �·�ʱ�䳬ʱ
				upper_assign_loc.new_loc = 0;
			
			//�����Ƿ�ʱ
			if(navi_timeout_flag == 1)
			{
				navi_stat = NaviPointTracking_NaviPointChoose;								//������ʱ״̬
				navi_timeout_flag = 0;
				break;
			}
			
			//��Ŀ�������Ƿ���5����֮��
			cur2target_dist = calc_diatance(CurrLatitude, CurrLongitude, target_latitude,\
									target_longtitude );
			if(cur2target_dist <= navi_point_swi_distance)
			{
				navi_stat = NaviPointTracking_NaviPointChoose;
				navi_timeout_flag = 0;
				rt_timer_stop(&navi_timeout_timer);						//ֹͣ������ʱ��
			}
			break;
			
		case NaviPointTracking_NaviEndPoint:
			if(navi_timeout_flag == 1)
			{
				navi_stat = NaviPointTracking_NaviPointChoose;								//������ʱ״̬
				navi_timeout_flag = 0;
				break;
			}
			cur2target_dist = calc_diatance(CurrLatitude, CurrLongitude, target_latitude,\
									target_longtitude );
			if(cur2target_dist <= navi_point_swi_distance)
			{
				navi_terminal_arrival_flag = 1;												//�㵽���յ�
				navi_stat = NaviPointTracking_NaviPointChoose;								//�л�����ʼ��״̬
			}
			break;
		default:
			break;
	}
}

/**************************************************

��������Navigation_entry
��  �ߣ�������
��  �ڣ�2018.10.8
��  ����V1.00
˵  ���������㷨����
��  ����
�޸ļ�¼��

**************************************************/
void Navigation_entry(void* parameter)
{	
	
	rt_timer_init(&navi_timeout_timer, "navi_timeout_timer", navi_timeout, RT_NULL, navi_timeout_ticks, RT_TIMER_FLAG_SOFT_TIMER | RT_TIMER_FLAG_ONE_SHOT);
	while(1)
	{
		calc_location();
		calc_target_site();
		calc_trace(CurrLatitude, CurrLongitude,target_latitude, target_longtitude, &disire_deviation, &curr_loc2target_dist);
		//���� ƫ��ǡ��ٶȷ��򡢺�����ȡ���ǰλ�þ�γ��
	}
}


