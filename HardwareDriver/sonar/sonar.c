/**************************************************

�ļ�����sonar.c
��  �ߣ�������
��  �ڣ�2018.7.27
��  ����V1.00
˵  �����������ų�ʼ���ļ�
�޸ļ�¼��

**************************************************/
#include "sonar.h"
#include <string.h>


static u8 Sonar_DMA_RxBuf[SONAR_BUFSIZE];

u8 Sonar_RxBuf[SONAR_BUFSIZE];
u8 Sonar_TxBuf[SONAR_BUFSIZE];

u8 	Sonar_Tx_Flag = 0;				//���ŷ��ͱ�־λ��1��ʾ���ڷ������ݣ�0��ʾ�������
u16 Sonar_Rx_Len = 0;				//���ݽ��ճ���
u16 Sonar_DMA_index = 0;

struct rt_semaphore senor_rev_sem;				//�������ݽ����ź���

const char sonar_start1[13] = {0x40,0x30,0x30,0x30,0x38,0x08,0x00,0xFF,0x02,0x03,0x17,0x80,0x02};
const char sonar_start2[13] = {0x40,0x30,0x30,0x30,0x38,0x08,0x00,0xFF,0x02,0x03,0x18,0x80,0x02};

//�����������ò���
const char sonar_set1[81] = {0x40,0x30,0x30,0x34,0x43,0x4C,0x00,0xFF,0x02,0x47,0x13,0x80,0x02,0x1D,0x05,0x23,0x11,0x99,0x99,0x99,
						  0x05,0xE1,0x7A,0x14,0x00,0x99,0x99,0x99,0x05,0xEB,0x51,0xB8,0x03,0x32,0x00,0x64,0x00,0xF0,0x0A,0x10,
						  0x0E,0x4F,0x61,0x3F,0x64,0x64,0x00,0x0A,0x00,0x19,0x40,0x55,0x00,0xF9,0x00,0xE8,0x03,0xF4,0x01,0x40,
						  0x06,0x01,0x00,0x00,0x00,0x4F,0x32,0x61,0x00,0x3F,0x64,0x00,0x00,0x64,0x00,0x0A,0x00,0x00,0x00,0x00,
						  0x00};
const char sonar_set2[14] = {0x40,0x30,0x30,0x30,0x39,0x09,0x00,0xFF,0x02,0x04,0x13,0x80,0x02,0x1C};

const char sonar_samp_cmd[17] = {0x40,0x30,0x30,0x30,0x43,0x0C,0x00,0xFF,0x02,0x07,0x19,0x80,0x02,0x40,0x40,0x40,0x01};
	
u8 sonar_samp_data[Scan_Num][300];					//�ɼ����ݴ洢��
	
block_data_struct scan_result[Scan_Num];			//�洢һ��ɨ��Ľ��

/**************************************************

��������uart6_init(uint32_t bound)
��  �ߣ�������
��  �ڣ�2018.7.27
��  ����V1.00
˵  ������ʼ������6
��  ����bound	������
�޸ļ�¼��

**************************************************/

static void USART6_Init(u32 bound)
{
	GPIO_InitTypeDef	GPIO_InitStructure;
	USART_InitTypeDef	USART_InitStructure;
	NVIC_InitTypeDef	NVIC_InitStructure;
	
	
	/****���ڻ�������ʼ��*****/
	rt_memset(Sonar_RxBuf, 0x00, sizeof(Sonar_RxBuf));
	rt_memset(Sonar_TxBuf, 0x00, sizeof(Sonar_RxBuf));
	
	USART_DeInit(USART6);  											//��λ����6
	
	/********ʱ������*********/
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, ENABLE);
	
	/**********����IO����***********/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_USART6);		//�ܽŹ��ܸ���
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_USART6);
	
	/************PB0���������485ģʽ����**********/ 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0; 										//GPIOB0
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;									//���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;								//�ٶ�2MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 									//�������
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 									//����
	GPIO_Init(GPIOB,&GPIO_InitStructure); 											//��ʼ��PB0
	
	/*************�������ŵ�Դ���ƹܽ�***************/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_Init(GPIOC,&GPIO_InitStructure);
	
	//*******���ڲ�������***********/
	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_Init(USART6, &USART_InitStructure);
	
	
	/*****ʹ��USART6�ж�*****/
	NVIC_InitStructure.NVIC_IRQChannel = USART6_IRQn;				//USART6�����ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;		//��ռ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;				//��Ӧ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	USART_DMACmd(USART6, USART_DMAReq_Tx, ENABLE);					//ʹ�ܴ���DMA���ͽӿ�
	
	/*****����USART6�ж�*****/
	USART_ITConfig(USART6, USART_IT_IDLE, ENABLE);					//ʹ��IDLE�ж�	
	
	USART_Cmd(USART6, ENABLE);
	
	SONAR_TX_EN = 0;												//��������Ĭ��Ϊ����״̬
	SONAR_POWER = 0;												//��������Ĭ��Ϊ�ر�״̬
	
	
	if(rt_sem_init(&senor_rev_sem, "senor_sem", 0, RT_IPC_FLAG_FIFO) != RT_EOK)	//��ʼ���ź���
		rt_kprintf("senor_sem init ERROR!\n");
}

/**************************************************

��������Sonar_DMA_Tx_init()
��  �ߣ�������
��  �ڣ�2018.7.27
��  ����V1.00
˵  ������ʼ��DMA����
�޸ļ�¼��

**************************************************/
static void Sonar_DMA_Tx_Init()
{
	DMA_InitTypeDef DMA_InitStructure;
	NVIC_InitTypeDef	NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
	
	DMA_DeInit(DMA2_Stream6);												//������������ʼ��
	while(DMA_GetCmdStatus(DMA2_Stream6) != DISABLE);
	
	DMA_InitStructure.DMA_Channel             = DMA_Channel_5;      		// DMAͨ��       
    DMA_InitStructure.DMA_PeripheralBaseAddr  = (u32)(&(USART6->DR));   	//Ŀ�ĵ�ַ
    DMA_InitStructure.DMA_Memory0BaseAddr     = (u32)Sonar_TxBuf;    	    //Դ��ַ         
    DMA_InitStructure.DMA_DIR                 = DMA_DIR_MemoryToPeripheral; //���䷽��   
    DMA_InitStructure.DMA_BufferSize          = SONAR_BUFSIZE;              //���ݳ���        
    DMA_InitStructure.DMA_PeripheralInc       = DMA_PeripheralInc_Disable;  //�����ַ������  
    DMA_InitStructure.DMA_MemoryInc           = DMA_MemoryInc_Enable;       //�洢����ַ����  
    DMA_InitStructure.DMA_PeripheralDataSize  = DMA_PeripheralDataSize_Byte;//�������ݿ��   
    DMA_InitStructure.DMA_MemoryDataSize      = DMA_MemoryDataSize_Byte;  	//�洢�����ݿ��    
    DMA_InitStructure.DMA_Mode                = DMA_Mode_Normal;        	//����/ѭ������     
    DMA_InitStructure.DMA_Priority            = DMA_Priority_High;  		//���ȼ�           
    DMA_InitStructure.DMA_FIFOMode            = DMA_FIFOMode_Disable;		//FIFO/ֱ��ģʽ          
    DMA_InitStructure.DMA_FIFOThreshold       = DMA_FIFOThreshold_HalfFull; //FIFO��С
    DMA_InitStructure.DMA_MemoryBurst         = DMA_MemoryBurst_Single;     //���δ���  
    DMA_InitStructure.DMA_PeripheralBurst     = DMA_PeripheralBurst_Single; //���δ���
	DMA_Init(DMA2_Stream6, &DMA_InitStructure);
	
	DMA_ITConfig(DMA2_Stream6, DMA_IT_TC, ENABLE);							//ʹ��DMA�ж�
	
	
	/***********����DMA�ж����ȼ�************/
	NVIC_InitStructure.NVIC_IRQChannel                   = DMA2_Stream6_IRQn;           
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;          
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0; 
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
	
	DMA_Cmd(DMA2_Stream6, DISABLE);
	
	SONAR_TX_EN = 0;													//Ĭ��Ϊ����ģʽ
}

/**************************************************

��������Sonar_DMA_Rx_init()
��  �ߣ�������
��  �ڣ�2018.7.27
��  ����V1.00
˵  ������ʼ��DMA����
�޸ļ�¼��

**************************************************/
static void Sonar_DMA_Rx_Init()
{
	DMA_InitTypeDef DMA_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
	
	DMA_DeInit(DMA2_Stream1);												//������������ʼ��
	while(DMA_GetCmdStatus(DMA2_Stream1) != DISABLE);
	
	DMA_InitStructure.DMA_Channel             = DMA_Channel_5;      		// DMAͨ��       
    DMA_InitStructure.DMA_PeripheralBaseAddr  = (u32)(&(USART6->DR));   	//Դ��ַ
    DMA_InitStructure.DMA_Memory0BaseAddr     = (u32)Sonar_DMA_RxBuf;    	//Ŀ�ĵ�ַ         
    DMA_InitStructure.DMA_DIR                 = DMA_DIR_PeripheralToMemory; //���䷽��   
    DMA_InitStructure.DMA_BufferSize          = SONAR_BUFSIZE;              //���ݳ���        
    DMA_InitStructure.DMA_PeripheralInc       = DMA_PeripheralInc_Disable;  //�����ַ������  
    DMA_InitStructure.DMA_MemoryInc           = DMA_MemoryInc_Enable;       //�洢����ַ����  
    DMA_InitStructure.DMA_PeripheralDataSize  = DMA_PeripheralDataSize_Byte;//�������ݿ��   
    DMA_InitStructure.DMA_MemoryDataSize      = DMA_MemoryDataSize_Byte;  	//�洢�����ݿ��    
    DMA_InitStructure.DMA_Mode                = DMA_Mode_Normal;        	//����/ѭ������     
    DMA_InitStructure.DMA_Priority            = DMA_Priority_VeryHigh;  	//���ȼ�           
    DMA_InitStructure.DMA_FIFOMode            = DMA_FIFOMode_Disable;		//FIFO/ֱ��ģʽ          
    DMA_InitStructure.DMA_FIFOThreshold       = DMA_FIFOThreshold_HalfFull; //FIFO��С
    DMA_InitStructure.DMA_MemoryBurst         = DMA_MemoryBurst_Single;     //���δ���  
    DMA_InitStructure.DMA_PeripheralBurst     = DMA_PeripheralBurst_Single; //���δ���
	
	DMA_Init(DMA2_Stream1, &DMA_InitStructure);
	
	USART_DMACmd(USART6, USART_DMAReq_Rx, ENABLE);							//ʹ�ܴ���DMA���սӿ�
	
	
	DMA_Cmd(DMA2_Stream1, ENABLE);
}


/**************************************************

��������Sonar_Tx_Enable()
��  �ߣ�������
��  �ڣ�2018.7.27
��  ����V1.00
˵  ����DMA����ʹ��
�޸ļ�¼��

**************************************************/
void Sonar_Tx_Enable(const char  *data, u16 ndtr)
{
	SONAR_TX_EN = 1;					//Sonar����ʹ��
	
	while(Sonar_Tx_Flag);			//ȷ���ϴη����ѽ���
	Sonar_Tx_Flag = 1;
	
	rt_memcpy(Sonar_TxBuf, data, ndtr);	
	DMA_SetCurrDataCounter(DMA2_Stream6, ndtr);
	DMA_Cmd(DMA2_Stream6, ENABLE);
	
	while(Sonar_Tx_Flag);
	
	SONAR_TX_EN = 0;					//Sonar�ָ�����״̬
}

/**************************************************

��������senor_irq_rx_end
��  �ߣ�������
��  �ڣ�2018.7.27
��  ����V1.00
˵  �������ڽ�����ɴ�����
��  ����buf �������ݵĻ�����
�޸ļ�¼��

**************************************************/
static u16 Senor_irq_rx_end(u8 *buf)
{
	u16 len = 0;
	if(USART_GetITStatus(USART6, USART_IT_IDLE) != RESET)
	{
		USART6->SR;
		USART6->DR;
		DMA_Cmd(DMA2_Stream1, DISABLE);
//		USART6->CR1 &= ~(USART_Mode_Rx);						//�رմ��ڽ���
		DMA_ClearFlag(DMA2_Stream1, DMA_FLAG_TCIF1);
		len = SONAR_BUFSIZE - DMA_GetCurrDataCounter(DMA2_Stream1);
		
		
		rt_memcpy(buf, Sonar_DMA_RxBuf, len);
		
		
		
		rt_sem_release(&senor_rev_sem);				//�ͷ�һ���ź���
		
		Sonar_DMA_Rx_Init();
		
		return len;
	}
	return 0;
}

/**************************************************

��������Senor_irq_tx_end
��  �ߣ�������
��  �ڣ�2018.7.27
��  ����V1.00
˵  �������ڽ�����ɴ�����
�޸ļ�¼��

**************************************************/

static void Senor_irq_tx_end(void)
{
	if(USART_GetFlagStatus(USART6, USART_IT_TC) == RESET)
	{
		USART_ITConfig(USART6, USART_IT_TC, DISABLE);
		
		Sonar_Tx_Flag = 0;
	}
}

/**************************************************

��������USART6_IRQHandler
��  �ߣ�������
��  �ڣ�2018.7.27
��  ����V1.00
˵  ����USART6�����жϴ�����
�޸ļ�¼��

**************************************************/
void USART6_IRQHandler(void)
{
	rt_interrupt_enter();
	
	Senor_irq_tx_end();
	Sonar_Rx_Len = Senor_irq_rx_end(Sonar_RxBuf);
	
	rt_interrupt_leave();
}

/**************************************************

��������DMA2_Stream6_IRQHandler
��  �ߣ�������
��  �ڣ�2018.7.27
��  ����V1.00
˵  ����DMA�����жϴ�����
�޸ļ�¼��

**************************************************/
void DMA2_Stream6_IRQHandler(void)
{
	rt_interrupt_enter();
	
	if(DMA_GetITStatus(DMA2_Stream6, DMA_IT_TCIF6) != RESET)
	{
		DMA_ClearFlag(DMA2_Stream6, DMA_IT_TCIF6);						//�巢����ɱ�־
		
		DMA_Cmd(DMA2_Stream6, DISABLE);									//�ر�DMA����
		
		USART_ITConfig(USART6, USART_IT_TC, ENABLE);					//�򿪷�������ж�
	}
	
	rt_interrupt_leave();
}


/**************************************************

��������Sonar_Init
��  �ߣ�������
��  �ڣ�2018.7.27
��  ����V1.00
˵  �����������ų�ʼ������
�޸ļ�¼��

**************************************************/
void Sonar_Init(u32 bound)
{
	USART6_Init(bound);
	Sonar_DMA_Tx_Init();
	Sonar_DMA_Rx_Init();
}


/**************************************************

��������Sonar_run()
��  �ߣ�������
��  �ڣ�2018.8.6
��  ����V1.00
˵  ���������������к��������й���ɨ��
�޸ļ�¼��

**************************************************/
void Sonar_run()
{
	u8 sonar_stat = 0;			//��������״̬
	u8 falut_cnt = 0;			//���մ��������־λ
	u8 samp_cnt = 0;			//��������
	
	SONAR_POWER = Sonar_ON;		//�����ϵ�
//	rt_sem_take(&senor_rev_sem, RT_WAITING_FOREVER);
//	if(Sonar_RxBuf[11] == 0x04)
//		rt_memset(Sonar_RxBuf, 0x00, sizeof(Sonar_RxBuf));
	USART6->CR1 |= USART_Mode_Rx;			//����ʹ��
	while(1)
	{
		switch(sonar_stat)
		{
			case 0:
				samp_cnt = 0;
				falut_cnt = 0;
				Sonar_Tx_Enable(sonar_start1, sizeof(sonar_start1));				
				++sonar_stat;
				break;
			case 1:
				rt_sem_take(&senor_rev_sem, RT_WAITING_FOREVER);
				if(Sonar_RxBuf[10] == 0x01)
				{
					Sonar_Tx_Enable(sonar_start2, sizeof(sonar_start2));
					++sonar_stat;
				}
				else if(++falut_cnt == FAULT_NUM)					//���ճ������Ϊ3�Σ����³�ʼ��
				{
 					sonar_stat = 0;
					falut_cnt = 0;
				}
				break;
			case 2:
				rt_sem_take(&senor_rev_sem, RT_WAITING_FOREVER);
				if(Sonar_Rx_Len == 372 && Sonar_RxBuf[10] == 0x06)
				{
					Sonar_Tx_Enable(sonar_set1, sizeof(sonar_set1));		//�������Ų���
					delay_us(1000);
					Sonar_Tx_Enable(sonar_set2, sizeof(sonar_set2));
					++sonar_stat;
				}
				else if(++falut_cnt == FAULT_NUM)									//���ճ������Ϊ3�Σ����³�ʼ��
				{
					sonar_stat = 0;
					falut_cnt = 0;
					
				}
				break;
			case 3:
				rt_sem_take(&senor_rev_sem, RT_WAITING_FOREVER);
				if(Sonar_Rx_Len == 22 && Sonar_RxBuf[10] == 0x04)
				{
					Sonar_Tx_Enable(sonar_samp_cmd, sizeof(sonar_samp_cmd));		//��ʼ����
					samp_cnt = 0;
					++sonar_stat;	
				}
				else if(++falut_cnt == FAULT_NUM)									//���ճ������Ϊ3�Σ����³�ʼ��
				{
					sonar_stat = 0;
					falut_cnt = 0;
				}
				break;
			case 4:
				rt_sem_take(&senor_rev_sem, RT_WAITING_FOREVER);
				if(Sonar_RxBuf[10] == 0x02)
				{
					rt_memcpy(&sonar_samp_data[samp_cnt++],Sonar_RxBuf, Sonar_Rx_Len);
				}
				Sonar_Tx_Enable(sonar_samp_cmd, sizeof(sonar_samp_cmd));
				break;
			default:
				break;
		}
		if(samp_cnt == 15)
		{
			sonar_stat = 0;
			samp_cnt = 0;				//�����ﵽ15�Σ��˳�����
			SONAR_POWER = Sonar_OFF;
			break;
		}
		delay_ms(10);
	}
}

/**************************************************

��������Sonar_analyst()
��  �ߣ�������
��  �ڣ�2018.8.24
��  ����V1.00
˵  �������ϲ������ݷ���
��  ����senor_data  ����ɨ��ɼ���������
�޸ļ�¼��

**************************************************/
void Sonar_analyze()
{
	u8 cnt = 0;
	u16 bin_cnt = 0;
	u16 scan_dir = 0;
	u16 data_len = 0;
	for(cnt = 0; cnt < Scan_Num; ++cnt)
	{
		scan_dir = ((u16)sonar_samp_data[cnt][41]<<8) + sonar_samp_data[cnt][40];		//��ȡ��ɨ��ķ���
		scan_result[cnt].angle = (float)scan_dir/16 * (float)0.9;
		data_len = ((u16)sonar_samp_data[cnt][43]<<8) + sonar_samp_data[cnt][42];		//���ݵĳ���
		for(bin_cnt = 8; bin_cnt < data_len-2; ++bin_cnt)					//8����Ϊ���ŵ�ǰ0.3m̽�ⲻ���������Թ�
		{																	//datalen-2����Ϊ̽�⵽��������bin�����ϰ������̽�⵽�ϰ���
			if(sonar_samp_data[cnt][44+bin_cnt] > 128 && sonar_samp_data[cnt][45+bin_cnt]>128 && sonar_samp_data[cnt][46+bin_cnt]>128)
			{
				scan_result[cnt].block = 1;
				scan_result[cnt].block_dist = (float)bin_cnt *100 / data_len;
				break;
			}
		}
		if(bin_cnt >= data_len-2)
		{
			scan_result[cnt].block = 0;
			scan_result[cnt].block_dist = 0;
		}
	}
}









