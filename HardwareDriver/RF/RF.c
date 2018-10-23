/**************************************************

�ļ�����RF.c
��  �ߣ�������
��  �ڣ�2018.8.28
��  ����V1.00
˵  ����RFģ���ʼ���ļ�
�޸ļ�¼��

**************************************************/
#include "RF.h"

u8 RF_Rev_Buf[RF_Max_Len];		//RF����BUF


/**************************************************

��������RF_Init
��  �ߣ�������
��  �ڣ�2018.8.28
��  ����V1.00
˵  ��������RF���ڲ���
��  ����
�޸ļ�¼��

**************************************************/

void RF_Init(void)
{
	uartTydef uartParaStructure;
	u8 GIER_reg;
	
	
	uartParaStructure.BaudRate = BAUDRATE_9600;			//RNSS������9600
	uartParaStructure.Parity = WK2124_PAMN;				//��У��
	uartParaStructure.StopBits = WK2124_STPL_1;			//1��ֹͣλ
	uartParaStructure.WordLength = WK2124_WORDLENGTH_8;	//8λ�ֳ�
	uartParaStructure.rev_contact_val = CONTACT_VAL;	//�����ż�ֵ
	uartParaStructure.send_contact_val = CONTACT_VAL;
	uartParaStructure.interrupt_enable = WK2124_RXOUT_IEN;   //���մ����ж� ���ճ�ʱ�ж�
	WK2124_UART_Config(RF_UART1, &uartParaStructure);
	
	
	WK2124_read_reg(GIER, &GIER_reg);
	WK2124_write_reg(GIER, (u8)(1<<(RF_UART1>>4)) | GIER_reg);					//�򿪴����ж�

}

/**************************************************

��������RF_send
��  �ߣ�������
��  �ڣ�2018.8.29
��  ����V1.00
˵  ����RF��������
��  ����dat  ����������
		len  ���ݳ���
����ֵ�����͵��ֽ���
�޸ļ�¼��

**************************************************/
u8 RF_send(u8 *dat, u8 len)
{
	u8 longth = 0;										//�������ݳ���
	
	longth = WK2124_write_FIFO(RF_UART1, dat, len);		//��������
	delay_us(1000);										//spi�ٶȿ죬�����ٶ�������Ҫ��ʱ���ȴ����ڷ������
	delay_us(1000);
	return longth;
}



