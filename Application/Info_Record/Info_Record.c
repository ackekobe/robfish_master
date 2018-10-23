/**************************************************

�ļ�����Info_Record.c
��  �ߣ�������
��  �ڣ�2018.9.10
��  ����V1.00
˵  ������Ϣ��¼����
�޸ļ�¼��

**************************************************/
#include "Info_Record.h"

_record_buf record_buf;			//��Ϣ��¼����

struct rt_event info_record_event;		//��Ϣ��¼�¼�event

u8 *error_dir = (u8*)"0:/error.txt";
u8 *sys_stat_dir = (u8*)"0:/sys_stat.txt";
u8 *location_dir = (u8*)"0:/location.txt";

/**************************************************

��������write_info
��  �ߣ�������
��  �ڣ�2018.9.10
��  ����V1.00
˵  ������Ϣд���ض��ļ�
��  ����dir			��Ϣ�洢·��
		buf 		��Ϣ����
		len			��Ϣ���ݳ���
����ֵ��RT_EOK		��Ϣд��ɹ�
		RT_ERROR	��Ϣд��ʧ��
�޸ļ�¼��

**************************************************/
u8 write_info(u8 *dir, u8 *buf, u8 len)
{

	u32 file_size = 0;
	if(mf_open(dir, FA_OPEN_ALWAYS | FA_WRITE, SD_dev) != FR_OK)
		return RT_ERROR;
	file_size = mf_size(SD_file);
	mf_lseek(file_size, SD_file);
	if(mf_write(buf, len, SD_file) != FR_OK)
		return RT_ERROR;
	mf_close(SD_file);
	delay_ms(10);
	return RT_EOK;
}


/**************************************************

��������info_record
��  �ߣ�������
��  �ڣ�2018.9.10
��  ����V1.00
˵  ������¼��Ϣ
��  ����type		��Ϣ����
		buf 		��Ϣ����
		len			��Ϣ���ݳ���
����ֵ��RT_EOK		��Ϣ��¼�ɹ�
		RT_ERROR	��Ϣ��¼ʧ��
�޸ļ�¼��

**************************************************/
u8 info_record(u8 type, u8 *buf, u8 len)
{
	u8 res = 0;
	switch(type)
	{
		case sys_stat_info:						//ϵͳ״̬��Ϣ
			res = write_info(sys_stat_dir, buf, len);
			break;
		case location_info:						//��λ��Ϣ
			res = write_info(location_dir, buf, len);
			break;
		case error_info:						//������Ϣ
			res = write_info(error_dir, buf, len);
			break;
		default:
			break;
	}
	return res;
}


/**************************************************

��������info_record_thread_entry
��  �ߣ�������
��  �ڣ�2018.9.11
��  ����V1.00
˵  ������Ϣ��¼�߳�ִ��
��  ����parameter	�̲߳���
����ֵ��
�޸ļ�¼��

**************************************************/
void info_record_thread_entry(void* parameter)
{
	u32 rev = 0;
	while(1)
	{
		if(rt_event_recv(&info_record_event, ((1<<0) | (1<<1) | (1<<2)), RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,	//���¼�����
			RT_WAITING_FOREVER, (rt_uint32_t*)&rev) == RT_EOK)
		{
			if(rev & sys_stat_bit)
				info_record(sys_stat_info, record_buf.sys_stat_buf, INFO_LEN);
			if(rev & location_bit)
				info_record(location_info, record_buf.location_buf, INFO_LEN);
			if(rev & error_info)
				info_record(error_info, record_buf.error_buf, INFO_LEN);
		}
	}
}



