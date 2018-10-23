/**************************************************

�ļ�����BD_RDSS.c
��  �ߣ�������
��  �ڣ�2018.7.30
��  ����V1.00
˵  ��������ģ��RDSS�ĳ�ʼ���ļ�
�޸ļ�¼��

**************************************************/

#include "BD_RDSS.h"
#include <string.h>

char RDSS_TX_buf[UPPER_MAX_LEN];
u8	 RDSS_REV_buf[UPPER_MAX_LEN];
u32  local_addr = 0;

const u8 ICJC[12] = {0x24,0x49,0x43,0x4A,0x43,0x00,0x0C,0x00,0x00,0x00,0x00,0x2B};			//IC���ʱ���͵ı���
const u8 XTZJ[13] = {0x24,0x58,0x54,0x5A,0x4A,0x00,0x0D,0x00,0x00,0x00,0x00,0x00,0x35};		//�Լ���ʱ���͵ı���



/**************************************************

��������BD_RDSS_Init
��  �ߣ�������
��  �ڣ�2018.7.30
��  ����V1.00
˵  ��������RDSS���ڲ���
		
��  ����
�޸ļ�¼��

**************************************************/

void BD_RDSS_Init(void)
{
	GPIO_InitTypeDef	GPIO_InitStructure;
	uartTydef uartParaStructure;
	u8 GIER_reg, FCR_reg;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);					//ʹ��GPIOBʱ��
	
	uartParaStructure.BaudRate = BAUDRATE_115200;			//RNSS������115200
	uartParaStructure.Parity = WK2124_PAMN;					//��У��
	uartParaStructure.StopBits = WK2124_STPL_1;				//1��ֹͣλ
	uartParaStructure.WordLength = WK2124_WORDLENGTH_8;		//8λ�ֳ�
	uartParaStructure.rev_contact_val = 0xFF;				//�����ż�ֵ
	uartParaStructure.send_contact_val = 0xFF;
	uartParaStructure.interrupt_enable = WK2124_RXOUT_IEN;  //���ճ�ʱ�ж�
	WK2124_UART_Config(BDRD_UART3, &uartParaStructure);
	
	WK2124_read_reg(GIER, &GIER_reg);
	WK2124_write_reg(GIER, (u8)(1<<(BDRD_UART3>>4)) | GIER_reg);					//�򿪴����ж�
	
	WK2124_read_reg(FCR | BDRD_UART3, &FCR_reg);
	WK2124_write_reg(FCR | BDRD_UART3, 0x03 | FCR_reg);					//��λFIFO
	
	
	
	/************PD8���������RDSS���ڷ���ʹ�ܹܽ�**********/ 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8; 										//GPIOD8
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;									//���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;								//�ٶ�2MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 									//�������
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 									//����
	GPIO_Init(GPIOD,&GPIO_InitStructure); 											//��ʼ��PD8
	
	BD_RDSS_Tx = 0;																	//Ĭ��Ϊ����
}

/**************************************************

��������BD_RDSS_Send
��  �ߣ�������
��  �ڣ�2018.7.30
��  ����V1.00
˵  ����RDSS���ڷ��ͺ���		
��  ����data ���͵�����
		len	���͵����ݳ���
����ֵ�����������ֽ���
�޸ļ�¼��

**************************************************/

u8 BD_RDSS_Send(const u8 *data, u8 len)
{
	u8 longth = 0;										//�������ݳ���
	BD_RDSS_Tx = 1;										//ʹ��RDSS����
	
	longth = WK2124_write_FIFO(BDRD_UART3, data, len);	//��������
	delay_us(1000);										//spi�ٶȿ죬�����ٶ�������Ҫ��ʱ���ȴ����ڷ������
	delay_us(1000);
	BD_RDSS_Tx = 0;
	return longth;
}


/**************************************************

��������BD_RDSS_TXSQ
��  �ߣ�������
��  �ڣ�2018.7.30
��  ����V1.00
˵  ����RDSSͨ��������֡		
��  ����local_addr 	���ص�ַ
		rev_addr   	���͵�ַ
		ack			�Ƿ�Ӧ��
		data		��������
		len			���ݳ���
����ֵ��
�޸ļ�¼��

**************************************************/

void BD_RDSS_TXSQ(u32 local_addr, u32 rev_addr, u8 ack, char *data, u8 len)
{
	u8 cnt = 0;
	u8 xor_val = '$';
	rt_memset(RDSS_TX_buf, 0x00, UPPER_MAX_LEN);
	strcpy(RDSS_TX_buf, "$TXSQ");		//����ָ��ͷ
	
	RDSS_TX_buf[5] = (len + 18) >>8;   	//����֡���ȸ��ֽ�
	RDSS_TX_buf[6] = len + 18;   		//���ֽ�
	
	RDSS_TX_buf[7] = (local_addr) >> 16;   	//���ص�ַ���ֽ�
	RDSS_TX_buf[8] = (local_addr) >> 8;   		
	RDSS_TX_buf[9] = local_addr;
	RDSS_TX_buf[10] = 0x04;					//��Ϣ���
	
	RDSS_TX_buf[11] = (rev_addr) >> 16;   	//���շ���ַ���ֽ�
	RDSS_TX_buf[12] = (rev_addr) >> 8;   		
	RDSS_TX_buf[13] = rev_addr;
	
	RDSS_TX_buf[14] = (len*8)>>8;			//���ĳ����Ա���λ��λ
	RDSS_TX_buf[15] = len*8;
	
	RDSS_TX_buf[16] = 0x01;
	
	strcpy(RDSS_TX_buf + 17, data);			//���ݷŽ����ͻ�����
	
	for(cnt = 1; cnt < len + 17; ++cnt)		//�ֽ����
	{
		xor_val ^= RDSS_TX_buf[cnt];
	}
	RDSS_TX_buf[17+len] = xor_val;
}






