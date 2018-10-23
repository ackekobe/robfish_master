#ifndef __UPPER_COMM_H
#define __UPPER_COMM_H

#include "stm32f4xx_conf.h"
#include "sys.h"
#include "rtthread.h"
#include "delay.h"

#include "struct.h"
#include "MS_Com_Process.h"
#include "RF.h"
#include "BD_RDSS.h"



#define BACK_STAT_LEN		(12)		//ϵͳ״̬֡�ĳ���
#define TRACE_INFO_LEN		(55)		//���͵ĺ�����Ϣ����֡����

#define RF_Type					(0)			//RF
#define	BD_Type					(1)			//����

#define Passive_Upload			(0)			//Ӧ��λ��Ҫ������
#define Positive_Upload			(1)			//��������

#define ROUTE_NUM				(6)			//·���滮��·������

typedef enum _sys_master_stat			//�ӻ���״̬
{
	reday = 0x01,
	running,
	fatal_error,
	fault_run,
} fish_master_stat;


typedef enum _sys_slave_stat
{
	check_normal = 0x01,
	check_fault,
	stat_upload,
	stat_done,
	sys_stop,
	time_done,
	rising,
	rise_done,
	diving,
	dive_done,
	turn_lefting,
	turn_left_done,
	turn_righting,
	turn_right_done,
	backing,
	back_done,
	glidering,
	dlider_done,
	path_planing,
	path_plan_done,
	fish_forwarding,
	fish_forward_done,
	fish_zeroing,
	fish_zero_done,
	fish_triming,
	fish_trim_done,
#ifdef __DEBUG
	l_pectoral_moving,
	l_pectoral_move_done,
	r_pectoral_moving,
	r_pectoral_move_done,
	gravity_motor_moving,
	gravity_motor_move_done,
	cylinder_mot0r_moving,
	cylinder_motor_move_done,
	waist_motor_moving,
	waist_motor_move_done,
	tail_motor_moving,
	tail_motor_move_dong,
#endif
} fish_slave_stat;

typedef struct 						//ϵͳ״̬�ṹ��
{
	u8 sys_major_stat;				//ϵͳ��״̬
	u16 sys_sub_stat;				//ϵͳ��״̬
	float cabin_temp;					//�����¶�
	float elec_quantity;				//����
} _sys_stat;


typedef struct						//���ϸ�����
{
	u8 		up_mode;
	float 	up_dist;
} _fish_up;

typedef struct						//���������
{
	u8 	back_mode;
	float back_dist;
} _fish_back;

typedef struct						//����Ǳ����
{
	u8 	down_mode;
	float down_dist;
} _fish_down;

typedef struct						//����ת����
{
	u8 	left_mode;
	float left_angle;
} _fish_left;

typedef struct						//����ת����
{
	u8 	right_mode;
	float right_angle;
} _fish_right;

typedef struct						//�㻬������
{
	float max_depth;
	float glide_dist;
	u8 	glide_mode;
	float pectoral_angle;
} _fish_glide;

typedef struct						//������ƽ�����
{
	float ahead_speed;				//������ƽ�ʱ�ٶ�
} _fish_fast_ahead;

typedef struct						//�㺽·�滮
{
	
	u8 longitude_type;				//0���� 1����
	u8 latitude_type;				//0��γ 1��γ
	float longitude;				//����			 
	float latitude;					//γ��		     	
	float depth;					//���
} _fish_route;


typedef struct						//��λ���·���ָ��λ��
{
	u8 new_loc;
	_fish_route loc_info;
	_time assign_timestamp;
} _upper_assign_loc;


typedef struct						//RF�·��ĵ�����������̬
{
	u8 cmd_type;					//��״̬��Ϊ���ʵʱ״̬����״̬�ı�ʱ����Ҫʵʱ���¸�״̬
	_fish_up fish_up;
	_fish_down fish_down;
	_fish_back fish_back;
	_fish_left fish_left;
	_fish_right fish_right;
	_fish_glide fish_glide;
	_fish_fast_ahead fish_fast_ahead;
	u16 out_water_inter;					//ðˮʱ����	
	_fish_route fish_route[ROUTE_NUM];		//6�����е�
#ifdef __DEBUG
	_move_motor left_pectoral_move;
	_move_motor right_pectoral_move;
	_oil_pumb_motor cylinder_move;
	_gravity_motor  gravity_move;
	_move_motor waist_move;
	_move_motor tail_move;
#endif
} _Upper_Fish_CMD;

//typedef struct						//������ĺ�����Ϣ
//{
//	u8 longitude_type;				//0���� 1����
//	u8 latitude_type;				//0��γ 1��γ
//	float longitude;				//����			 
//	float latitude;					//γ��		     	
//	float depth;	
//} _Track_Information;

extern _fish_route Track_Info[ROUTE_NUM];
extern u8  RF_Send_Buf[UPPER_MAX_LEN];
extern _upper_assign_loc upper_assign_loc;


u8 stat_back_frame(u8 *buf, u8 up_type);
u8 track_frame(u8 *buf);
u8 data_upload_upper(u8 type, u8 *dat, u8 len);
u8 Upper_Message_Analy(u8 *buf,  u8 Upper_type);
void track_info_upper(u8 type);

#endif


