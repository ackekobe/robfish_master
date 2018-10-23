/**************************************************

�ļ�����altimeter.c
��  �ߣ�������
��  �ڣ�2018.7.20
��  ����V1.00
˵  �����߶ȼƳ�ʼ���ļ�
�޸ļ�¼��

**************************************************/

#include "altimeter.h"


u8 alti_DMA_Rx_Buf[REV_LEN];							//DMA���ջ���
u8 altitude[REV_LEN];
u16 RS485_Rx_Len = 0;								//���ݽ��ճ���
u16 USART2_RX_STA = 0;								//����2����״̬
u8	altimer_rev_flag = ALTIMETER_Rev_Undo;							//�߶ȼƽ������ݱ�־λ

float fish_altitude = 0.0;							//�����ڸ߶�


/**************************************************

��������altit_RS485_Init
��  �ߣ�������
��  �ڣ�2018.7.20
��  ����V1.00
˵  ������߶ȼ�������RS485��ʼ��
		���ո߶�����ʹ��DMA
��  ����bound ������
�޸ļ�¼��

**************************************************/

static void altit_RS485_Init(u32 bound)
{  	 
	
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;
	NVIC_InitTypeDef	NVIC_InitStructure;
	
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); 							//ʹ��GPIOAʱ��
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,ENABLE); 							//ʹ��GPIOCʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);							//ʹ��USART2ʱ��
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
	
	
	
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource2,GPIO_AF_USART2); 						//GPIOA2����ΪUSART2
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource3,GPIO_AF_USART2); 						//GPIOA3����ΪUSART2
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3; 							//GPIOA2��GPIOA3
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;									//���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;								//�ٶ�2MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 									//���츴�����
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 									//����
	GPIO_Init(GPIOA,&GPIO_InitStructure); 											//��ʼ��PA2��PA3
	
	/************PC5���������485ģʽ����**********/ 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5; 										//GPIOC5
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;									//���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;								//�ٶ�2MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 									//�������
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 									//����
	GPIO_Init(GPIOC,&GPIO_InitStructure); 											//��ʼ��PC5
	
	/*************�߶ȼƵ�Դ���ƹܽ�***************/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_Init(GPIOC,&GPIO_InitStructure);
	


	USART_InitStructure.USART_BaudRate = bound;										//����������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;							//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;								//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					//�շ�ģʽ
	USART_Init(USART2, &USART_InitStructure); 										//��ʼ������2	
	
	/*****ʹ��USART2�ж�*****/
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;								//USART2�����ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;						//��ռ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;								//��Ӧ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);									//ʹ��IDLE�ж�	
	
	USART_Cmd(USART2, ENABLE);  													//ʹ�ܴ��� 2	
	USART_ClearFlag(USART2, USART_FLAG_TC);
	
	
	
	DMA_DeInit(DMA1_Stream5);														//������������ʼ��
	while(DMA_GetCmdStatus(DMA1_Stream5) != DISABLE);
	
	DMA_InitStructure.DMA_Channel             = DMA_Channel_4;      		// DMAͨ��       
    DMA_InitStructure.DMA_PeripheralBaseAddr  = (u32)(&(USART2->DR));   	//Դ��ַ
    DMA_InitStructure.DMA_Memory0BaseAddr     = (u32)alti_DMA_Rx_Buf;    		//Ŀ�ĵ�ַ         
    DMA_InitStructure.DMA_DIR                 = DMA_DIR_PeripheralToMemory; //���䷽��   
    DMA_InitStructure.DMA_BufferSize          = REV_LEN;                    //���ݳ���        
    DMA_InitStructure.DMA_PeripheralInc       = DMA_PeripheralInc_Disable;  //�����ַ������  
    DMA_InitStructure.DMA_MemoryInc           = DMA_MemoryInc_Enable;       //�洢����ַ������  
    DMA_InitStructure.DMA_PeripheralDataSize  = DMA_PeripheralDataSize_Byte;//�������ݿ��   
    DMA_InitStructure.DMA_MemoryDataSize      = DMA_MemoryDataSize_Byte;  	//�洢�����ݿ��    
    DMA_InitStructure.DMA_Mode                = DMA_Mode_Normal;        	//����/ѭ������     
    DMA_InitStructure.DMA_Priority            = DMA_Priority_VeryHigh;  	//���ȼ�           
    DMA_InitStructure.DMA_FIFOMode            = DMA_FIFOMode_Disable;		//FIFO/ֱ��ģʽ          
    DMA_InitStructure.DMA_FIFOThreshold       = DMA_FIFOThreshold_HalfFull; //FIFO��С
    DMA_InitStructure.DMA_MemoryBurst         = DMA_MemoryBurst_Single;     //���δ���  
    DMA_InitStructure.DMA_PeripheralBurst     = DMA_PeripheralBurst_Single; //���δ���
	
	DMA_Init(DMA1_Stream5, &DMA_InitStructure);
	USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);							//ʹ�ܴ���DMA���սӿ�
	DMA_Cmd(DMA1_Stream5, ENABLE);

			
	ALTIMETER_TX_EN = 0;													//Ĭ��Ϊ����ģʽ
	ALTIMETER_POWER	= POWER_OFF;													//�߶ȼ�Ĭ��Ϊ�ر�״̬
}

/**************************************************

��������altimeter_send
��  �ߣ�������
��  �ڣ�2018.7.20
��  ����V1.00
˵  �����߶ȼƷ�������������
��  ����buf �������׵�ַ
		len �������ݳ���
�޸ļ�¼��

**************************************************/
void altimeter_send(u8 *buf, u8 len)
{
	u8 t;
	
	USART_Cmd(USART2, ENABLE);
	ALTIMETER_TX_EN = 1;											//����Ϊ����ģʽ
//	delay_ms(5);
	
	for(t=0; t<len; ++t)
	{
		while(USART_GetFlagStatus(USART2,USART_FLAG_TC)==RESET); 	//�ȴ����ͽ���		
		USART_SendData(USART2,buf[t]); 								//��������
	}
	while(USART_GetFlagStatus(USART2,USART_FLAG_TC)==RESET); 		//�ȴ����ͽ���

//	delay_ms(5);
	ALTIMETER_TX_EN = 0;											//����Ϊ����ģʽ
}

/**************************************************

��������USART2_IRQHandler
��  �ߣ�������
��  �ڣ�2018.7.20
��  ����V1.00
˵  ����USART2�жϴ�����
�޸ļ�¼��

**************************************************/
void USART2_IRQHandler(void)
{
	u16 len = 0;
	rt_interrupt_enter();
	if(USART_GetITStatus(USART2, USART_IT_IDLE) != RESET)
	{
		USART2->SR;
		USART2->DR;
		DMA_Cmd(DMA1_Stream5, DISABLE);
		DMA_ClearFlag(DMA1_Stream5, DMA_FLAG_TCIF5);
		len = REV_LEN - DMA_GetCurrDataCounter(DMA1_Stream5);
		rt_memcpy(altitude, alti_DMA_Rx_Buf, len);
		
		DMA_SetCurrDataCounter(DMA1_Stream5, REV_LEN);
		DMA_Cmd(DMA1_Stream5, ENABLE);
		if(len >= 0x0A)
		{
			ALTIMETER_POWER = POWER_OFF;				//�ڴ˴��رո߶ȼ�
			altimer_rev_flag = ALTIMETER_Rev_Done;		//�߶ȼƽ��յ���ȷ����
		}
	}

	rt_interrupt_leave();
}

/**************************************************

��������altimeter_Init(u32 bound)
��  �ߣ�������
��  �ڣ�2018.7.20
��  ����V1.00
˵  �����߶ȼƳ�ʼ���ӿں���
��  ����bound ������
�޸ļ�¼��

**************************************************/
void altimeter_Init(u32 bound)
{
	altit_RS485_Init(bound);
}

/**************************************************

��������altitude_analy
��  �ߣ�������
��  �ڣ�2018.9.6
��  ����V1.00
˵  �����߶ȼ����ݼ���
��  ����
�޸ļ�¼��

**************************************************/
void altitude_analy()
{
	u8 temp;						//����߶����ݵ�ÿһλ
	fish_altitude = 0.0;
//	temp = altitude[0] - 0x30;
//	fish_altitude += temp * 100;
	temp = altitude[1] - 0x30;
	fish_altitude += temp * 10;
	temp = altitude[2] - 0x30;
	fish_altitude += temp;
	temp = altitude[4] - 0x30;
	fish_altitude += temp * (float)0.1;
	temp = altitude[5] - 0x30;
	fish_altitude += temp * (float)0.01;
	temp = altitude[6] - 0x30;
	fish_altitude += temp * (float)0.001;
}



