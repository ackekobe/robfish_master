/**************************************************

�ļ�����MS_Comm.c
��  �ߣ�������
��  �ڣ�2018.7.19
��  ����V1.00
˵  �������ӻ�SPIͨ�ų�ʼ���ļ�
�޸ļ�¼��

**************************************************/

#include "MS_Comm.h"


u8 slave_send_flag = 0;		//�ӻ��������ݱ�־��0 �ӻ�û����������  1 �ӻ�������Ҫ����
u8 SPI_Error_stat = 0;		//SPI�Ĵ���״̬


/**************************************************

��������MS_Comm_Init
��  �ߣ�������
��  �ڣ�2018.7.19
��  ����V1.00
˵  ������ʼ��SPI1�ӿ�
�޸ļ�¼��

**************************************************/

void MS_Comm_Init(void)
{
	GPIO_InitTypeDef	GPIO_InitStructure;
	SPI_InitTypeDef		SPI_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);					//ʹ��GPIOAʱ��
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);					//ʹ��GPIOFʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);					//ʹ��SPI1ʱ��
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;			//PB5~7���ù������	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;							//���ù���
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;							//�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;						//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;							//����
	GPIO_Init(GPIOA, &GPIO_InitStructure);									//��ʼ��
	
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
//	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource4,GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource5,GPIO_AF_SPI1); 					//PB5����Ϊ SPI1
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource6,GPIO_AF_SPI1); 					//PB6����Ϊ SPI1
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource7,GPIO_AF_SPI1); 					//PB7����Ϊ SPI1
	
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI1,ENABLE);						//��λSPI1
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI1,DISABLE);					//ֹͣ��λSPI1
	
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  	//����SPI�������˫�������ģʽ:SPI����Ϊ˫��˫��ȫ˫��
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;							//����SPI����ģʽ:����Ϊ��SPI
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;						//����SPI�����ݴ�С:SPI���ͽ���8λ֡�ṹ
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;								//����ͬ��ʱ�ӵĿ���״̬Ϊ�ߵ�ƽ
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;							//����ͬ��ʱ�ӵĵڶ��������أ��������½������ݱ�����
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;								//NSS�ź���Ӳ����NSS�ܽţ����������ʹ��SSIλ������:�ڲ�NSS�ź���SSIλ����
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;	//���岨����Ԥ��Ƶ��ֵ:������Ԥ��ƵֵΪ256
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;						//ָ�����ݴ����MSBλ����LSBλ��ʼ:���ݴ����MSBλ��ʼ
	SPI_InitStructure.SPI_CRCPolynomial = 7;								//CRCֵ����Ķ���ʽ
	SPI_Init(SPI1, &SPI_InitStructure);  									//����SPI_InitStruct��ָ���Ĳ�����ʼ������SPIx�Ĵ���

	
	SPI_Cmd(SPI1, ENABLE); 													//ʹ��SPI����
	

	
}

/**************************************************

��������MS_Comm_TIMode_Init
��  �ߣ�������
��  �ڣ�2018.7.19
��  ����V1.00
˵  ������ʼ��SPI1�ӿ�
�޸ļ�¼��

**************************************************/

void MS_Comm_TIMode_Init(void)
{
	GPIO_InitTypeDef	GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);					//ʹ��GPIOAʱ��
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);					//ʹ��GPIOFʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);					//ʹ��SPI1ʱ��
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;			//PB5~7���ù������	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;							//���ù���
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;							//�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;						//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;							//����
	GPIO_Init(GPIOA, &GPIO_InitStructure);									//��ʼ��
	
	
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource4,GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource5,GPIO_AF_SPI1); 					//PB5����Ϊ SPI1
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource6,GPIO_AF_SPI1); 					//PB6����Ϊ SPI1
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource7,GPIO_AF_SPI1); 					//PB7����Ϊ SPI1
	
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI1,ENABLE);						//��λSPI1
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI1,DISABLE);					//ֹͣ��λSPI1
	
	
	SPI1->CR1 |= SPI_BaudRatePrescaler_256;
	SPI1->CR1 |= SPI_DataSize_8b;
	SPI_TIModeCmd(SPI1, ENABLE);
	SPI1->CR1 |= 0x0044;	
}

/**************************************************

��������MS_Comm_SetSpeed
��  �ߣ�������
��  �ڣ�2018.7.19
��  ����V1.00
˵  ��������SPI1���ٶ�
		SPI�ٶ� = fAPB2/��Ƶϵ��
		APB2�ٶ�һ��Ϊ84MHz
��	����SPI_BaudRatePrescaler ��Ƶϵ��
�޸ļ�¼��

**************************************************/
void MS_Comm_SetSpeed(u8 SPI_BaudRatePrescaler)
{
	assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_BaudRatePrescaler));			//�ж���Ч��
	SPI1->CR1&=0XFFC7;														//λ3-5���㣬�������ò�����
	SPI1->CR1|=SPI_BaudRatePrescaler;										//����SPI1�ٶ� 
	SPI_Cmd(SPI1,ENABLE); 													//ʹ��SPI1
} 

/**************************************************

��������MS_Comm_WriteByte
��  �ߣ�������
��  �ڣ�2018.7.19
��  ����V1.00
˵  ��������/��������
��	����TxData �����͵�����
�޸ļ�¼��

**************************************************/
s16 MS_Comm_WriteByte(u8 TxData)
{	
	u8 retry = 0;
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET)			//�ȴ��������� 	
	{
		if(++retry > 200)
			return RT_FAULT;
	}		
	SPI_I2S_SendData(SPI1, TxData); 										//ͨ������SPI1����һ��byte����	
	return RT_EOK;
}

/**************************************************

��������Master_ReadByte
��  �ߣ�������
��  �ڣ�2018.8.21
��  ����V1.00
˵  ������������
��	����
�޸ļ�¼��

**************************************************/
s16 Master_ReadByte(void)
{  
	u8 retry = 0;
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET)		//�ȴ�������һ��byte			
	{
		if(++retry > 200)
			return RT_FAULT;
	}	
	return SPI_I2S_ReceiveData(SPI1); 										//����ͨ��SPI1������յ�����      
}

/**************************************************

��������Master_Send_Data
��  �ߣ�������
��  �ڣ�2018.8.21
��  ����V1.00
˵  ������������
��	����dat	 	Ҫ�������ݵĻ�����
		len		Ҫ�������ݵĳ���
�޸ļ�¼��

**************************************************/
u8 Master_Send_Data(u8* dat, u8 len)
{
	u8 cnt = 0;
//	NSS_CS = 1;
	delay_us(5);
	for(cnt = 0; cnt < len; ++cnt)
	{
//		NSS_CS = 0;
		if(MS_Comm_WriteByte(*(dat+cnt)) == RT_FAULT)
		{
//			NSS_CS = 0;
			return RT_ERROR;
		}
#ifdef __DEBUG
//		rt_kprintf("%d\n", dat[cnt]);
#endif
		if(Master_ReadByte() == RT_FAULT)
		{
//			NSS_CS = 0;
			return RT_ERROR;
		}
//		NSS_CS = 1;
		delay_us(5);
	}
//	NSS_CS = 1;
	return RT_EOK;
}

/**************************************************

��������Master_Send_Data
��  �ߣ�������
��  �ڣ�2018.8.22
��  ����V1.00
˵  ������������
��	����rev_buf	 	���ݽ��ջ���
		len			���յ����ݳ���
�޸ļ�¼��

**************************************************/
u8 Master_Rev_Data(u8* rev_buf, u8 len)
{
	u8 cnt = 0;
	s16 temp_val = 0;
//	NSS_CS = 0;
	for(cnt = 0; cnt < len; ++cnt)
	{
		if(MS_Comm_WriteByte(0x00) == RT_FAULT)
		{
//			NSS_CS = 1;
			return RT_ERROR;
		}
		temp_val = Master_ReadByte();
		if(temp_val == RT_FAULT)
		{
//			NSS_CS = 1;
			return RT_ERROR;
		}
		rev_buf[cnt] = temp_val;
		
#ifdef  __DEBUG
		rt_kprintf("%d\n",rev_buf[cnt]);
#endif
	}
//	NSS_CS = 1;
	return RT_EOK;
}




