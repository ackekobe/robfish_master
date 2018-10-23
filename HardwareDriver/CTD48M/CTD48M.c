/**************************************************

�ļ�����CTD48M.c
��  �ߣ�������
��  �ڣ�2018.7.24
��  ����V1.00
˵  �����ṩ������CTD48M�ĳ�ʼ����ӿں���
�޸ļ�¼��

**************************************************/

#include "CTD48M.h"
#include "math.h"					//�õ�����ѧ��ʽ���Ӽ���


#ifdef USART3_DMA_RX	
static u8 CTD_DMA_RxBuf[CTD_BUF_LEN];
#endif


u8 CTD48M_rev_flag = CTM48_Rev_Undo;

u8 CTD_RxBuf[CTD_BUF_LEN];
u16 USART3_RX_STA = 0;		//���������״̬

const float ctd_temp_coeffi[2] = {-2.58156E+0, 5.90399E-4};								//�¶ȼ���ϵ��
const float ctd_depth_coeffi[4] = {-2.51174E+2, 3.52897E-2, 2.00434E-10,-1.11898E-14};	//���/ѹ������ϵ��
const float ctd_cond_coeffi[4] = {-2.75897E-1, 1.22162E-3, 2.52963E-10, -7.70466E-16}; //�ζȼ���ϵ��





_Cal_Val CTD_Final_Val;			//���������������ս��

_sample_val sample_data;


/**************************************************

��������USART3_Init
��  �ߣ�������
��  �ڣ�2018.7.24
��  ����V1.00
˵  ������������������USART3��ʼ��
		DMA1��ʼ��������DMA1�����ж�
��  ����bound ������
�޸ļ�¼��

**************************************************/
static void USART3_Init(u32 bound)
{  	 
	
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
#ifdef USART3_DMA_RX	
	DMA_InitTypeDef DMA_InitStructure;
#endif
	NVIC_InitTypeDef	NVIC_InitStructure;
	
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE); 							//ʹ��GPIOBʱ��
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF,ENABLE); 							//ʹ��GPIOFʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);							//ʹ��USART3ʱ��
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
	
	
	
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource10,GPIO_AF_USART3); 						//GPIOA10����ΪUSART3
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource11,GPIO_AF_USART3); 						//GPIOA11����ΪUSART3
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11; 						
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;									//���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;								//�ٶ�100MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 									//���츴�����
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 									//����
	GPIO_Init(GPIOB,&GPIO_InitStructure); 											//��ʼ��PA10��PA11

	/*****************������عܽ�*************************************/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; 										//GPIOF9
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;									//���
	GPIO_Init(GPIOF,&GPIO_InitStructure); 
	
	


	USART_InitStructure.USART_BaudRate = bound;										//����������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;							//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_Odd;							//��У��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					//�շ�ģʽ
	USART_Init(USART3, &USART_InitStructure); 										//��ʼ������3	
		
	
	USART_Cmd(USART3, ENABLE);  													//ʹ�ܴ���3	
	USART_ClearFlag(USART3, USART_FLAG_TC);
	
	
#ifdef USART3_DMA_RX	
	DMA_DeInit(DMA1_Stream1);														//������������ʼ��
	while(DMA_GetCmdStatus(DMA1_Stream1) != DISABLE);
	
	DMA_InitStructure.DMA_Channel             = DMA_Channel_4;      		// DMAͨ��       
    DMA_InitStructure.DMA_PeripheralBaseAddr  = (u32)(&(USART3->DR));   	//Ŀ�ĵ�ַ
    DMA_InitStructure.DMA_Memory0BaseAddr     = (u32)CTD_DMA_RxBuf;    		//Դ��ַ         
    DMA_InitStructure.DMA_DIR                 = DMA_DIR_PeripheralToMemory; //���䷽��   
    DMA_InitStructure.DMA_BufferSize          = CTD_BUF_LEN;                //���ݳ���        
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

	DMA_Init(DMA1_Stream1, &DMA_InitStructure);
	USART_DMACmd(USART3, USART_DMAReq_Rx, ENABLE);							//ʹ�ܴ���DMA���սӿ�
	DMA_Cmd(DMA1_Stream1, ENABLE);
#endif
	
	/*****ʹ��DMA1_Stream1�ж�*****/
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;							//����3�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;						//��ռ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;								//��Ӧ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);									//USART3�����жϿ���
	
	CTD_Power = CTM48_OFF;
}


/**************************************************

��������CTD_send
��  �ߣ�������
��  �ڣ�2018.7.24
��  ����V1.00
˵  ���������������������
��  ����buf �������׵�ַ
		len �������ݳ���
�޸ļ�¼��

**************************************************/
void CTD_send(u8 *buf, u8 len)
{
	u8 t;
	
	for(t=0; t<len; ++t)
	{
		while(USART_GetFlagStatus(USART3,USART_FLAG_TC)==RESET); 	//�ȴ����ͽ���		
		USART_SendData(USART3,buf[t]); 								//��������
	}
	while(USART_GetFlagStatus(USART3,USART_FLAG_TC)==RESET); 		//�ȴ����ͽ���
	
}


/**************************************************

��������DMA1_Stream1_IRQHandler
��  �ߣ�������
��  �ڣ�2018.7.24
��  ����V1.00
˵  ����DMA1��������жϴ�����
�޸ļ�¼��

**************************************************/
void USART3_IRQHandler(void)
{
	u8 res;
	rt_interrupt_enter();
	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
	{
		res = USART3->DR;
		if((USART3_RX_STA & 0x8000) == 0)
		{
			if(USART3_RX_STA < CTD_BUF_LEN)		//�ж��ǹ����յ��㹻���ֽ���
				CTD_RxBuf[USART3_RX_STA++] = res;
			else
			{
				USART3_RX_STA |= 1<<15;				//�������λ��λ
				CTD_Power = CTM48_OFF;
				CTD48M_rev_flag = CTM48_Rev_Done;
			}
		}
	}
	rt_interrupt_leave();
}

/**************************************************

��������CTD_Init(u32 bound)
��  �ߣ�������
��  �ڣ�2018.7.24
��  ����V1.00
˵  �����߶ȼƳ�ʼ���ӿں���
��  ����bound ������
�޸ļ�¼��

**************************************************/
void CTD_Init(u32 bound)
{
	USART3_Init(bound);
}

/**************************************************

��������CTD_Rx_Enable
��  �ߣ�������
��  �ڣ�2018.8.9
��  ����V1.00
˵  �������������ʹ��
��  ����
�޸ļ�¼��

**************************************************/
void CTD_Rx_Enable(void)
{
	delay_ms(1000);
	CTD_Power = CTM48_ON;
}


/**************************************************

��������CTD_Val_Swap
��  �ߣ�������
��  �ڣ�2018.7.24
��  ����V1.00
˵  ����������������ȡ����Ч����
		����CTD�û��ֲ��е����ݸ�ʽ����ת��
��  ����data ������Ĳ�������
����ֵ��temp_val ת�����ֵ
�޸ļ�¼��

**************************************************/
static u16 CTD_Val_Swap(u8 *data)
{
	u16 byte0, byte1, byte2;
	u16 temp_val = 0;
	
	byte0 = data[0] & 0xFE;  			//LSBת��ΪMSB,ת��Ϊ���һλΪ��Чλ��ȡ����
	byte1 = data[1] & 0xFE;
	byte2 = data[2] & 0x06;
	
	temp_val = (byte2 << 13) | (byte1 << 6) | (byte0 >> 1); 
	return temp_val;
}


/**************************************************

��������CTD_Data_Cal
��  �ߣ�������
��  �ڣ�2018.7.24
��  ����V1.00
˵  ������16���Ƶ�����ת��Ϊdouble����
		�������ݵļ��㹫ʽ�ɳ����û��ֲ����
��  ����data ���������ת���������
		addr ���ݵ�ַ
����ֵ��
�޸ļ�¼��

**************************************************/

static void CTD_Data_Cal(_sample_val *data, u8 addr)
{
	int i = 0;
	switch(addr)
	{
		case DEPTH_ADDR:
			CTD_Final_Val.depth_cal_val = 0.0;
			for(i = 0; i < 4; ++i)				//CTD�û��ֲ��и��Ĺ�ʽ����			
			{
				CTD_Final_Val.depth_cal_val += (ctd_depth_coeffi[i] * pow(data->press_sam_val, i));
			}
			break;
		case TEMP_ADDR:
			CTD_Final_Val.temp_cal_val = 0.0;
			for(i = 0; i < 2; ++i)
			{
				CTD_Final_Val.temp_cal_val += (ctd_temp_coeffi[i] * pow(data->temp_sam_val, i));
			}
			break;
		case COND_ADDR:
			CTD_Final_Val.cond_cal_val = 0.0;
			for(i = 0; i < 4; ++i)
			{
				CTD_Final_Val.cond_cal_val += (ctd_cond_coeffi[i] * pow(data->cond_sam_val, i));
			}
			break;
		default:
			break;
	}
}



/**************************************************

��������CTD_Data_Trans
��  �ߣ�������
��  �ڣ�2018.7.24
��  ����V1.00
˵  ������������ת��Ϊ��ʵ����
��  ����data ������Ĳ�������
����ֵ��-1 ��������
	    0  ���ݼ�����ȷ
�޸ļ�¼��

**************************************************/

u8 CTD_Data_Trans(u8* data)
{
	u8 i = 0;
	u8 addr = 0;
	u8 press_cnt = 0, temp_cnt = 0, cond_cnt = 0;			//ת����������
	while(!((data[i] == 0x45) && (data[i+1] == 0x43) && (data[i+2] == 0x45) && (data[i+3] == 0x36)))  //�ĸ��ֽ�ΪCTD���������е��ֽ�
	{
		if(++i == (CTD_BUF_LEN - 27))  						// CTD�������ģ�15��+һ�����ݱ��ģ�12�� = 27
		{
			rt_kprintf("CTD Data Error!\n");
			return RT_ERROR;
		}
	}	
	
	for(i += 15; i < CTD_BUF_LEN; i += 3)					// i+15 :����CTD����������
	{
		if(press_cnt && temp_cnt  && cond_cnt)				//�ɼ������ݣ��˳�ѭ��
			break;
		addr = data[i+2] & 0xF8;
		switch(addr)
		{
			case DEPTH_ADDR:
				if(press_cnt++ == 0)	
					sample_data.press_sam_val = CTD_Val_Swap(data+i);				
				break;
			case TEMP_ADDR:
				if(temp_cnt++ == 0)
					sample_data.temp_sam_val = CTD_Val_Swap(data+i);				
				break;
			case COND_ADDR:
				if(cond_cnt++ == 0)
					sample_data.cond_sam_val = CTD_Val_Swap(data+i);				
				break;
			default:
				break;
		}
	}
	
	CTD_Data_Cal(&sample_data, DEPTH_ADDR);					//��������ת��Ϊ��������
	CTD_Data_Cal(&sample_data, TEMP_ADDR);
	CTD_Data_Cal(&sample_data, COND_ADDR);
	
	USART3_RX_STA = 0;										//״̬������      
	return RT_EOK;
}





