/**************************************************

�ļ�����WK2124.c
��  �ߣ�������
��  �ڣ�2018.7.25
��  ����V1.00
˵  ����SPIתUSART�ĳ�ʼ���ļ�
�޸ļ�¼��

**************************************************/

#include "WK2124.h"

u8 uart_RST_Flag[4] = {0, 0, 0, 0};
u8 WK2124_Rev_Flag[4] = {WK2124_Rev_Undo, WK2124_Rev_Undo, WK2124_Rev_Undo, WK2124_Rev_Undo};		//WK2124�ĸ����ڽ��ձ�־λ
u16 RNSS_rev_cnt = 0;
u8 RNSS_FIFO_cnt = 0;

extern u8 RDSS_REV_buf[UPPER_MAX_LEN];		//RDSS���ջ���
extern u8 RF_Rev_Buf[RF_Max_Len];			//RF���ջ���
extern char BD_raw_data[LOCA_INFO_LEN];		//RNSS��λ��Ϣ���ջ���

extern struct rt_semaphore RNSS_rev_sem;	//RNSS�����ź���


const _baud_rate_def baud_array[BAUD_NUM] = {{307200,0x0002}, {153600,0x0005}, {76800, 0x000b}, {38400, 0x0017},
										     {19200, 0x002F}, {9600,  0x005f}, {4800,  0x00bf}, {2400,  0x017f},
										     {921600,0x0000}, {460800,0x0001}, {230400,0x0003}, {115200,0x0007},
										     {57600, 0x000f}, {28800, 0x001f}, {14400, 0x003f}, {7200,  0x007f}};

/**************************************************

��������WK2124_Init
��  �ߣ�������
��  �ڣ�2018.7.25
��  ����V1.00
˵  ������ʼ��SPI2�ӿ�
�޸ļ�¼��

**************************************************/

void WK2124_Init(void)
{
	GPIO_InitTypeDef	GPIO_InitStructure;
	SPI_InitTypeDef		SPI_InitStructure;
	EXTI_InitTypeDef	EXTI_InitStructure;
	NVIC_InitTypeDef	NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);					//ʹ��GPIOBʱ��
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);					//ʹ��GPIOEʱ�ӣ��ж��븴λ�ź�����PE��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);					//ʹ��SPI2ʱ��
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;		//PB12~15���ù������	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;							//���ù���
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;							//�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;						//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;							//����
	GPIO_Init(GPIOB, &GPIO_InitStructure);									//��ʼ��
	
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource13,GPIO_AF_SPI2); 					//PB13����Ϊ SPI2
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource14,GPIO_AF_SPI2); 					//PB14����Ϊ SPI2
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource15,GPIO_AF_SPI2); 					//PB15����Ϊ SPI2
	
	
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_SPI2,ENABLE);						//��λSPI2
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_SPI2,DISABLE);					//ֹͣ��λSPI2
	
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  	//����SPI�������˫�������ģʽ:SPI����Ϊ˫��˫��ȫ˫��
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;							//����SPI����ģʽ:����Ϊ��SPI
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;						//����SPI�����ݴ�С:SPI���ͽ���8λ֡�ṹ
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;								//����ͬ��ʱ�ӵĿ���״̬Ϊ�͵�ƽ
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;							//����ͬ��ʱ�ӵĵ�1�������أ��������½������ݱ�����
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;								//NSS�ź������ʹ��SSIλ������
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;		//���岨����Ԥ��Ƶ��ֵ:������Ԥ��ƵֵΪ64
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;						//ָ�����ݴ����MSBλ����LSBλ��ʼ:���ݴ����MSBλ��ʼ
	SPI_InitStructure.SPI_CRCPolynomial = 7;								//CRCֵ����Ķ���ʽ
	SPI_Init(SPI2, &SPI_InitStructure);  									//����SPI_InitStruct��ָ���Ĳ�����ʼ������SPIx�Ĵ���
	
//	SPI_SSOutputCmd(SPI2, ENABLE);											//NSS���ͣ�ʹ�ܴ�����
	
	SPI_Cmd(SPI2, ENABLE); 													//ʹ��SPI����
	WK2124_ReadWriteByte(0xff);												//��������,Ŀ��Ϊ�˱���MOSIΪ�ߵ�ƽ
	
	
	/************PG2��ʼ���ⲿ�ж�����*************************/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;								//PG2��Ϊ�ж�����	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;							//��ͨ����ģʽ
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;						//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;							//����
	GPIO_Init(GPIOG, &GPIO_InitStructure);									//��ʼ��
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);					//ʹ��SYSCFGʱ��
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOG, EXTI_PinSource2);			//PG2������2
	
	/*******************����EXTI_Line2***********************/
	EXTI_InitStructure.EXTI_Line = EXTI_Line2;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;					//�½��ش���
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;					//�ⲿ�ж���2
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;			//��ռ�ж����ȼ�
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;					
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	/************PG3���������WK2124��λ�ܽ�**********/ 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3; 										//GPIOG3
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;									//���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;								//�ٶ�2MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 									//�������
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 									//����
	GPIO_Init(GPIOG,&GPIO_InitStructure); 											//��ʼ��PG3
	
	/************PB12���������WK2124Ƭѡ�ܽ�**********/ 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12; 										//GPIOB12
	GPIO_Init(GPIOB,&GPIO_InitStructure); 
	
	WK_CS = 1;
	
	WK2124_RST();
	
}

/**************************************************

��������WK2124_SPI2_SetSpeed
��  �ߣ�������
��  �ڣ�2018.7.25
��  ����V1.00
˵  ��������SPI1���ٶ�
		SPI�ٶ� = fAPB1/��Ƶϵ��
		APB1�ٶ�һ��Ϊ42MHz
��	����SPI_BaudRatePrescaler ��Ƶϵ��
�޸ļ�¼��

**************************************************/
void WK2124_SPI2_SetSpeed(u8 SPI_BaudRatePrescaler)
{
	assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_BaudRatePrescaler));			//�ж���Ч��
	SPI2->CR1&=0XFFC7;														//λ3-5���㣬�������ò�����
	SPI2->CR1|=SPI_BaudRatePrescaler;										//����SPI2�ٶ� 
	SPI_Cmd(SPI2,ENABLE); 													//ʹ��SPI2
} 

/**************************************************

��������WK2124_ReadWriteByte
��  �ߣ�������
��  �ڣ�2018.7.25
��  ����V1.00
˵  ��������/��������
��	����TxData �����͵�����
�޸ļ�¼��

**************************************************/
u8 WK2124_ReadWriteByte(u8 TxData)
{		 			 
 
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);			//�ȴ���������  
	
	SPI_I2S_SendData(SPI2, TxData); 										//ͨ������SPI2����һ��byte����
		
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);		//�ȴ�������һ��byte  
 
	return SPI_I2S_ReceiveData(SPI2); 										//����ͨ��SPI2������յ�����	
 		    
}


/**************************************************

��������WK2124_read_reg
��  �ߣ�������
��  �ڣ�2018.7.25
��  ����V1.00
˵  ������WK2124�ļĴ���
��  ����reg 	�Ĵ�����ַ
		data 	���ջ���
�޸ļ�¼��

**************************************************/
u8 WK2124_read_reg(u8 reg, u8 *data)
{
	u8 buf_wr_data[2] = {0x00, 0xFF};							//���ͻ���
	u8 buf_rd_data[2] = {0x00, 0x00};							//���ջ���
	buf_wr_data[0] = reg | READ_REG_HEAD;
	
	WK_CS = 0;
	buf_rd_data[0] = WK2124_ReadWriteByte(buf_wr_data[0]);		//д��ַ
	buf_rd_data[1] = WK2124_ReadWriteByte(buf_wr_data[1]);		//���ռĴ�������
	*data = buf_rd_data[1];
	WK_CS = 1;
	delay_us(2);
	return RT_EOK;
}

/**************************************************

��������WK2124_write_reg
��  �ߣ�������
��  �ڣ�2018.7.25
��  ����V1.00
˵  ����дWK2124�ļĴ���
��  ����reg 	�Ĵ�����ַ
		data 	���͵ļĴ�������
�޸ļ�¼��

**************************************************/
u8 WK2124_write_reg(u8 reg, u8 data)
{
	u8 buf_wr_data[2] = {0, 0};			//���ͻ���
	buf_wr_data[0] = reg | WRITE_REG_HEAD;
	buf_wr_data[1] = data;
	WK_CS = 0;
	WK2124_ReadWriteByte(buf_wr_data[0]);		//д��ַ
	WK2124_ReadWriteByte(buf_wr_data[1]);		//д����
	delay_us(2);
	WK_CS = 1;
	delay_us(2);
	return 0;
}

/**************************************************

��������WK2124_FIFO_RST
��  �ߣ�������
��  �ڣ�2018.7.29
��  ����V1.00
˵  ������λWK2124��FIFO
��  ����reg 	�Ĵ�����ַ
		data 	���͵ļĴ�������
�޸ļ�¼��

**************************************************/
u8 WK2124_FIFO_RST(u8 uart_addr, u8 rev_send)
{
	u8 FCR_val = 0;
	u8 reg = 0;
	reg  = FCR | uart_addr;						//�Ĵ�����ַ
	WK2124_read_reg(reg, &FCR_val);	
	WK2124_write_reg(reg, FCR_val | (1<<rev_send));	
	return RT_EOK;
}

/**************************************************

��������WK2124_write_FIFO
��  �ߣ�������
��  �ڣ�2018.7.29
��  ����V1.00
˵  ����дWK2124�ķ���FIFO
��  ����uart_addt 	�Ӵ��ڵ�ַ
		data 		���͵�����
		length		�������ݵĳ���
����ֵ��-1	����ʧ��
		0  	���ͳɹ�
�޸ļ�¼��

**************************************************/
u8 WK2124_write_FIFO(u8 uart_addr, const u8 *data, u16 length)
{
	u8 reg = 0;
	u16 out_loop = 0, inner_loop = 0;
	u16 send_length = 0;				//�ѷ������ݳ���
	reg = uart_addr | WRITE_FIFO_HEAD;
	
	WK2124_FIFO_RST(uart_addr, SEND_BIT);					//��λ����FIFO
	WK_CS = 0;
	WK2124_ReadWriteByte(reg);
	for(out_loop = 0; out_loop <= length / FIFO_SIZE; ++out_loop)	
	{
		
		
		for(inner_loop = 0; inner_loop < FIFO_SIZE; ++inner_loop)//��һ��FIFO��СΪд����
		{
			WK2124_ReadWriteByte(data[out_loop*FIFO_SIZE + inner_loop]);
			if(++send_length >= length) 						//�ж��Ƿ������
				break;
		}
		
	}
	WK_CS = 1;
	if(send_length > length)
		return RT_ERROR;
	return RT_EOK;
}

/**************************************************

��������WK2124_read_FIFO
��  �ߣ�������
��  �ڣ�2018.7.29
��  ����V1.00
˵  ������WK2124�ķ���FIFO
��  ����uart_addt 	�Ӵ��ڵ�ַ
		data 		�������ݴ�Ż�����
����ֵ�����յ������ݳ���
�޸ļ�¼��

**************************************************/
u8 WK2124_read_FIFO(u8 uart_addr, u8 *data)
{
	u8 reg = 0, rev_cnt;
	u8 rev_length = 0;					//���յ����ݳ���
	reg = uart_addr | RFCNT;
	WK2124_read_reg(reg, &rev_length);	//����FIFO�����ݸ���
	
	reg = uart_addr | READ_FIFO_HEAD;
	WK_CS = 0;
	WK2124_ReadWriteByte(reg);
	for(rev_cnt = 0; rev_cnt < rev_length; ++rev_cnt)	//��ȡ����FIFO�е�����
	{
		data[rev_cnt] = WK2124_ReadWriteByte(0xFF);
	}
	WK_CS = 1;
	WK2124_FIFO_RST(uart_addr, REV_BIT);				//��λ����FIFO
	return rev_length;
}

/**************************************************

��������WK2124_UART_Config
��  �ߣ�������
��  �ڣ�2018.7.25
��  ����V1.00
˵  �������ô��ڲ���
��  ����uartParam	���ڲ���
�޸ļ�¼��

**************************************************/
void WK2124_UART_Config(u8 uart_addr, uartTydef *uartParam)
{
//	u8  FCR_reg, SIER_reg, rev_val, send_val, band1, band2;		//������
	u8 SCR_reg, LCR_reg = 0, GENA_reg = 0, i = 0;
	WK2124_read_reg(GENA, &GENA_reg);
	WK2124_write_reg(GENA, ((u8)(1<<(uart_addr>>4))) | GENA_reg);						//ʹ�ܴ���
	WK2124_write_reg(GRST,(u8)(1<<(uart_addr>>4)));										//��λ����		
	
	
	LCR_reg |= uartParam->StopBits | uartParam->WordLength;
	if(uartParam->Parity != WK2124_PAMN)
		LCR_reg |= uartParam->Parity;
	WK2124_write_reg(uart_addr | LCR, LCR_reg);
	
	WK2124_write_reg(uart_addr | SCR, WK2124_RXEN | WK2124_TXEN);  					//���������ʹ��
	WK2124_write_reg(uart_addr | FCR, WK2124_TFEN | WK2124_RFEN);					//ʹ�ܷ��������FIFO
	WK2124_write_reg(uart_addr | SIER, uartParam->interrupt_enable);							//�ж�ʹ��
	
	
	for(i = 0; i < BAUD_NUM; ++i)						//�����ض������ʶ�Ӧ�ļĴ�����ֵ
	{
		if(baud_array[i].baud_rate == uartParam->BaudRate)
			break;
	}
	
	WK2124_write_reg(uart_addr | SPAGE, WK2124_SPAGE1);	//дҳ1
	WK2124_write_reg(uart_addr | BAUD0, (u8)baud_array[i].baud_reg_val);		//���ò�����
	WK2124_write_reg(uart_addr | BAUD1, (u8)(baud_array[i].baud_reg_val>>8));	
	WK2124_write_reg(uart_addr | RFTL, uartParam->rev_contact_val);			//����FIFO����ֵ
	WK2124_write_reg(uart_addr | TFTL, uartParam->send_contact_val);			//����FIFO����ֵ
	WK2124_write_reg(uart_addr | SPAGE, WK2124_SPAGE0);	//дҳ0
	
	WK2124_read_reg(uart_addr | SCR, &SCR_reg);
	
	WK2124_write_reg(uart_addr | SCR, SCR_reg | WK2124_SLEEPEN);  //����ʹ��������������Ӵ��ڼĴ����޷�����
}


/**************************************************

��������WK2124_RST
��  �ߣ�������
��  �ڣ�2018.7.25
��  ����V1.00
˵  ����WK2124��λ
�޸ļ�¼��

**************************************************/
void WK2124_RST (void)
{
	u8 i = 0;
	WK_RST = 0;										//�ܽ����͸�λ
	for(i = 0; i<10; i++)
		delay_us(1000);
	WK_RST = 1;
	for(i = 0; i<10; i++)
		delay_us(1000);
}

/**************************************************

��������WK2124_USART_Ctr
��  �ߣ�������
��  �ڣ�2018.9.6
��  ����V1.00
˵  ����WK2124���ڿ���
��	����num 	�������
		stat	����״̬(1:�򿪣�0:�ر�)
����ֵ��RT_EOK		��ȷ
		RT_ERROR	������������
�޸ļ�¼��

**************************************************/
u8 WK2124_USART_Ctr(u8 num, u8 stat)
{
	u8 GENA_reg;
	WK2124_read_reg(GENA, &GENA_reg);
	if(stat == 0)
	{
		WK2124_write_reg(GENA, (~(1<<num)) & GENA_reg);						//	�رմ���
		return RT_EOK;
	}
	else if(stat == 1)
	{
		WK2124_write_reg(GENA, ((1<<num)) | GENA_reg);						//	�򿪴���
		return RT_EOK;
	}
	return RT_ERROR;
}

/**************************************************

��������EXTI2_IRQHandler
��  �ߣ�������
��  �ڣ�2018.7.25
��  ����V1.00
˵  �����ⲿ�ж���2���жϴ�����
�޸ļ�¼��

**************************************************/
void EXTI2_IRQHandler(void)
{
	u8 GIFR_reg = 0;
	u8 SIFR_reg = 0;
	rt_interrupt_enter();
	WK2124_read_reg(GIFR, &GIFR_reg);			//��ȡȫ���жϱ�־�Ĵ���
	if(GIFR_reg & 0x01)											//UART1�����ж�
	{
		WK2124_write_reg(RF_UART1 | SPAGE, WK2124_SPAGE0);		//����ҳ0
		WK2124_read_reg(SIFR | RF_UART1, &SIFR_reg);			//��ȡ�Ӵ����жϱ�־�Ĵ���
		if(SIFR_reg & (WK2124_FERR_INT | WK2124_TFTRIG_INT))
		{
			WK2124_write_reg(GRST,(u8)(1<<(RF_UART1>>4)));		//��λ����
			uart_RST_Flag[RF_UART] = 1;												//��λ���ڸ�λ��־
		}
		if(SIFR_reg & (WK2124_RXOVT_INT | WK2124_RFTRIG_INT))
		{
			WK2124_Rev_Flag[RF_UART] = WK2124_Rev_Done;
			WK2124_read_FIFO(RF_UART1, RF_Rev_Buf);
//			WK2124_USART_Ctr(RF_UART, 0);							//	�رմ���
			//��λ�ź��������ⲿ���н���
		}
	}
	
	if(GIFR_reg & 0x02)												//UART2 RNSS�����ж�
	{
		u8 last_data_len = 0;
		SIFR_reg = 0;
		WK2124_read_reg(SIFR | BDRN_UART2, &SIFR_reg);				//��ȡ�Ӵ����жϱ�־�Ĵ���
		if(SIFR_reg & (WK2124_FERR_INT | WK2124_TFTRIG_INT))
		{
			WK2124_write_reg(GRST,(u8)(1<<(BDRN_UART2>>4)));		//��λ����
			uart_RST_Flag[RNSS_UART] = 1;									//��λ���ڸ�λ��־
		}
		if(SIFR_reg & (WK2124_RFTRIG_INT))							//RNSS���ڽ��մ����ж�
		{
			WK_CS = 0;
			WK2124_ReadWriteByte(READ_FIFO_HEAD | BDRN_UART2);
			for(RNSS_FIFO_cnt = 0; RNSS_FIFO_cnt < CONTACT_VAL; ++RNSS_FIFO_cnt)	//��FIFO��ȡ��CONTACT_VAL������
			{
				BD_raw_data[RNSS_rev_cnt++] = WK2124_ReadWriteByte(0xFF);	
			}	
			WK_CS = 1;			
		}
		if(SIFR_reg & (WK2124_RXOVT_INT))							//RNSS���ڽ��ճ�ʱ�ж�
		{
			last_data_len = WK2124_read_FIFO(BDRN_UART2, (u8*)BD_raw_data + RNSS_rev_cnt);				//ȡ��ʣ�������
			BD_raw_data[RNSS_rev_cnt + last_data_len] = '\0';
			if(BD_raw_data[0] == '$')					//���յ������Ƿ���ȷ
			{
				WK2124_USART_Ctr(RNSS_UART, 0);							//	�رմ���
				WK2124_Rev_Flag[RNSS_UART] = WK2124_Rev_Done;
			}
			RNSS_rev_cnt = 0;																		//���ݼ�������				
		}
	}
	
	if(GIFR_reg & 0x04)											//UART3 RDSS�����ж�
	{
		SIFR_reg = 0;
		WK2124_read_reg(SIFR | BDRD_UART3, &SIFR_reg);			//��ȡ�Ӵ����жϱ�־�Ĵ���
		if(SIFR_reg & (WK2124_FERR_INT | WK2124_TFTRIG_INT))
		{
			WK2124_write_reg(GRST,(u8)(1<<(BDRD_UART3>>4)));		//��λ����
			uart_RST_Flag[RDSS_UART] = 1;												//��λ���ڸ�λ��־
		}
		if(SIFR_reg & WK2124_RXOVT_INT)
		{
			WK2124_Rev_Flag[RDSS_UART] = WK2124_Rev_Done;							//��λ���ڽ��ձ�־λ
			WK2124_read_FIFO(BDRD_UART3, RDSS_REV_buf);						//��FIFO�ж�ȡ����
			WK2124_USART_Ctr(RDSS_UART, 0);							//	�رմ���
		}
	}
	EXTI_ClearITPendingBit(EXTI_Line2);
	rt_interrupt_leave();
	if(WK2124_Rev_Flag[RNSS_UART] == WK2124_Rev_Done)
		rt_sem_release(&RNSS_rev_sem);
}




