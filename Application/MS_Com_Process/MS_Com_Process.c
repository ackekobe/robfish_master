/**************************************************

�ļ�����MS_Com_Process.c
��  �ߣ�������
��  �ڣ�2018.8.27
��  ����V1.00
˵  �������ӻ�ͨѶ����
�޸ļ�¼��

**************************************************/
#include "MS_Com_Process.h"



_time	Master_Time;			//����ʱ�䣬��ʱ��ͬ���ڱ�������ʱ��
_move_motor  left_pectoral;				//����������
_move_motor	 right_pectoral;			//������
_oil_pumb_motor  oil_pumb_motor;		//�ͱõ������
_gravity_motor 	 gravity_motor;			//���ĵ������
_move_motor	 waist_motor;				//�����������	
_move_motor		 tail_motor;			//β���������

_slave_stat  slave_stat;

u8 Master_send_buf[64];					//�������ͻ�����
u8 slave_stat_buf[28];						//���յ��Ĵӻ�״̬������

/**************************************************

��������sys_check_framing
��  �ߣ�������
��  �ڣ�2018.8.27
��  ����V1.00
˵  ����ϵͳ�Լ���֡
��  ����buf 	���ͻ���
����ֵ��len		�������ݳ���
�޸ļ�¼��

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

��������read_stat_framing
��  �ߣ�������
��  �ڣ�2018.8.27
��  ����V1.00
˵  ������ȡ�ӻ�״̬
��  ����buf 	���ͻ���
����ֵ��len		�������ݳ���
�޸ļ�¼��

**************************************************/
u8 read_stat_framing(u8 *buf)
{
	u8 cnt = 0;
	buf[cnt++] = ZTSB;
	for(; cnt < 29; ++cnt)
		buf[cnt] = 0x00;				//��ȡ�ӻ����ݣ������·����������壬ȫ���·�0���˴����ɸ���Ϊ�·���������
	return cnt;
}


/**************************************************

��������sys_timing_framing
��  �ߣ�������
��  �ڣ�2018.8.27
��  ����V1.00
˵  ����ϵͳ�Լ���֡(���ֽ���ǰ�����ֽ��ں�)
��  ����buf 	���ͻ���
����ֵ��len		�������ݳ���
�޸ļ�¼��

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

��������fish_run_framing
��  �ߣ�������
��  �ڣ�2018.8.27
��  ����V1.00
˵  ����������״̬��֡
��  ����buf 	���ͻ���
		CMD_ID	�������״̬
����ֵ��len		�������ݳ���
�޸ļ�¼��

**************************************************/
u8 fish_run_framing(u8 *buf, u8 CMD_ID)
{
	u8 cnt = 0;
	buf[cnt++] = CMD_ID;
	buf[cnt++] = 38;
	buf[cnt++] = left_pectoral.run_mode;
	buf[cnt++] = left_pectoral.flap_amplitude;
	buf[cnt++] = left_pectoral.flap_fre_integer;
	buf[cnt++] = left_pectoral.flap_fre_decimal;			//С������*100��Ϊ����
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
	buf[cnt++] = waist_motor.flap_fre_decimal;		//С������*100��Ϊ����
	buf[cnt++] = (s8)waist_motor.target_location;
	buf[cnt++] = tail_motor.run_mode;
	buf[cnt++] = tail_motor.flap_amplitude;
	buf[cnt++] = tail_motor.flap_fre_integer;
	buf[cnt++] = tail_motor.flap_fre_decimal;		//С������*100��Ϊ����
	buf[cnt++] = (s8)tail_motor.target_location;
	buf[cnt] = CRC8_Table(buf, cnt);
	buf[++cnt] = 0xEE;
	return cnt+1;
}



/**************************************************

��������Master_Slave_Comm
��  �ߣ�������
��  �ڣ�2018.8.27
��  ����V1.00
˵  �������ӻ�ͨ�ź���
��  ����CMD_ID ͨ��������
����ֵ��RT_EOK		�ɹ�
		RT_ERROR	ʧ��
�޸ļ�¼��

**************************************************/
u8 Master_Slave_Comm(u8 CMD_ID)
{
	u8 send_len = 0;
	switch(CMD_ID)
	{
		case XTJC:												//ϵͳ�Լ�
			send_len = sys_check_framing(Master_send_buf);
			if(Master_Send_Data(Master_send_buf, send_len))
				return RT_ERROR;
			break;
		case ZTSB:												//�ӻ�״̬����
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
				return RT_ERROR;									//�����ֽ�
			if(Master_Rev_Data(slave_stat_buf, slave_stat_bytes))
				return RT_ERROR;									//�ӻ�״̬�ֽ���Ϊ 28
			break;
		case XTTZ:												//�ӻ����ֹͣ����
			if(MS_Comm_WriteByte(XTTZ))
				return RT_ERROR;
			break;
		case XTJS:
			send_len = sys_timing_framing(Master_send_buf);
			if(Master_Send_Data(Master_send_buf, send_len))
				return RT_ERROR;
			break;
		case XTFW:												//ϵͳ��λ
			if(MS_Comm_WriteByte(XTFW))
				return RT_ERROR;
			break;
		case YTHL:
			if(MS_Comm_WriteByte(YTHL))							//�������
				return RT_ERROR;
			break;
		case YTZP:
			if(MS_Comm_WriteByte(YTZP))							//�������
				return RT_ERROR;
			break;
		case YTSF:												//����������̬
		case YTXQ:
		case YTZZ:
		case YTYZ:
		case YTHT:
		case YTHX:
		case YTKT:												//��������ƽ�
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

��������Slave_Stat_Anay
��  �ߣ�������
��  �ڣ�2018.8.28
��  ����V1.00
˵  �����ӻ�״̬��������
��  ����buf �ӻ�״̬������
����ֵ��RT_EOK		�ɹ�
		RT_ERROR	ʧ��
�޸ļ�¼��

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







