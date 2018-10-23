/**************************************************

�ļ�����AHRS.c
��  �ߣ�������
��  �ڣ�2018.7.18
��  ����V1.00
˵  ������ʼ��AHRS
�޸ļ�¼��

**************************************************/


#include "AHRS.h"
#include <string.h>
#include <stdio.h>

u8 AHRS_RxBuf[USART1_BUFSIZE];
u8 AHRS_TxBuf[USART1_BUFSIZE];


//static u8 DMA_RxBuf[AHRS_DMA_LEN];
u16 USART1_RX_STA = 0;

u8 ahrs_rev_flag = AHRS_Rev_Undo;

u8 USART1_Tx_Flag = 0;				//USART1���ͱ�־λ��1��ʾ���ڷ������ݣ�0��ʾ�������

u8 attitude_data[AHRS_DATA_LEN];	//�ᷭ������
u8 angleRate_data[AHRS_DATA_LEN];	//���ٶ�����
u8 accRate_data[AHRS_DATA_LEN];		//���ٶ�����
u8 velocity_data[AHRS_DATA_LEN];	//�ٶ�����

const float pi = 3.1415926;
const float g = 9.80665;			//�������ٶȳ���

const u8 close_stream[11] = {0x75,0x65,0x0C,0x05,0x05,0x11,0x01,0x01,0x00,0x03,0x19};
const u8 polling_data[10] = {0x75,0x65,0x0C,0x04,0x04,0x01,0x00,0x00,0xEF,0xDA};

//�ߵ����ݲ���
const float vel_x_static_compensation = 0.058;			//ÿһ������x���ٶȵľ�̬����
const float vel_y_static_compensation = -0.0012;		//ÿһ������y���ٶȵľ�̬����
float vec_x_dynamic_compensation = 0.0;					//ÿһ������x���ٶȵĶ�̬����
float vec_y_dynamic_compensation = 0.0;					//ÿһ������y���ٶȵĶ�̬����

const float roll_compensation = 0.37;			//�����ǶȲ���
const float pitch_compensation = 5.1;			//��б�ǶȲ���

const float acc_x_compensation = 0.88;			//ˮƽ״̬��X����ٶȲ���
const float acc_y_compensation = -0.02;			//ˮƽ״̬��Y����ٶȲ���

union								//�������ݣ�����ת��������
{  
	u8 temp[4];
    float real;
} conver;

_attitude attitude;
_angRate angRate;
_accRate accRate;
_velocity delta_velocity;
_velocity velocity;


/**************************************************

��������uart1_init(uint32_t bound)
��  �ߣ�������
��  �ڣ�2018.7.18
��  ����V1.00
˵  ������ʼ������1
�޸ļ�¼��

**************************************************/

static void USART1_Init(u32 bound)
{
	GPIO_InitTypeDef	GPIO_InitStructure;
	USART_InitTypeDef	USART_InitStructure;
//	DMA_InitTypeDef		DMA_InitStructure;
	NVIC_InitTypeDef	NVIC_InitStructure;
	
	
	/****���ڻ�������ʼ��*****/
	rt_memset(AHRS_RxBuf, 0x00, sizeof(AHRS_RxBuf));
	rt_memset(AHRS_TxBuf, 0x00, sizeof(AHRS_TxBuf));
	USART1_RX_STA = 0;
	
	USART_DeInit(USART1);  											//��λ����1
	
	/********ʱ������*********/
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	
	/**********����IO����***********/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_USART1);		//�ܽŹ��ܸ���
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_USART1);
	
#if 0
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4; 										//GPIOC4
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;									//���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;								//�ٶ�2MHz  �㹻���㲨����
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 									//�������
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 									//����
	GPIO_Init(GPIOC,&GPIO_InitStructure); 											//��ʼ��PC4
#endif
	
	//*******���ڲ�������***********/
	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_Init(USART1, &USART_InitStructure);
	
	
	/*****ʹ��USART1�ж�*****/
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;				//USART1�����ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;		//��ռ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;				//��Ӧ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);					//ʹ�ܴ���DMA���ͽӿ�
	

	
	/*****����USART1�ж�*****/
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);					//ʹ���ֽڽ����ж�	
	
	USART_Cmd(USART1, ENABLE);
	
}

/**************************************************

��������AHRS_DMA_Tx_init()
��  �ߣ�������
��  �ڣ�2018.7.18
��  ����V1.00
˵  ������ʼ��DMA����
�޸ļ�¼��

**************************************************/
static void AHRS_DMA_Tx_Init()
{
	DMA_InitTypeDef DMA_InitStructure;
	NVIC_InitTypeDef	NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
	
	DMA_DeInit(DMA2_Stream7);												//������������ʼ��
	while(DMA_GetCmdStatus(DMA2_Stream7) != DISABLE);
	
	DMA_InitStructure.DMA_Channel             = DMA_Channel_4;      		// DMAͨ��       
    DMA_InitStructure.DMA_PeripheralBaseAddr  = (u32)(&(USART1->DR));   	//Ŀ�ĵ�ַ
    DMA_InitStructure.DMA_Memory0BaseAddr     = (u32)AHRS_TxBuf;    		//Դ��ַ         
    DMA_InitStructure.DMA_DIR                 = DMA_DIR_MemoryToPeripheral; //���䷽��   
    DMA_InitStructure.DMA_BufferSize          = AHRS_DMA_LEN;                    //���ݳ���        
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
	DMA_Init(DMA2_Stream7, &DMA_InitStructure);
	
	DMA_ITConfig(DMA2_Stream7, DMA_IT_TC, ENABLE);							//ʹ��DMA�ж�
	
	
	/***********����DMA�ж����ȼ�************/
	NVIC_InitStructure.NVIC_IRQChannel                   = DMA2_Stream7_IRQn;           
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;          
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 2; 
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
	
	DMA_Cmd(DMA2_Stream7, DISABLE);
	
}

#if 0
/**************************************************

��������AHRS_DMA_Rx_init()
��  �ߣ�������
��  �ڣ�2018.7.18
��  ����V1.00
˵  ������ʼ��DMA����
�޸ļ�¼��

**************************************************/

static void AHRS_DMA_Rx_Init()
{
	DMA_InitTypeDef DMA_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
	
	DMA_DeInit(DMA2_Stream5);												//������������ʼ��
	while(DMA_GetCmdStatus(DMA2_Stream5) != DISABLE);
	
	DMA_InitStructure.DMA_Channel             = DMA_Channel_4;      		// DMAͨ��       
    DMA_InitStructure.DMA_PeripheralBaseAddr  = (u32)(&(USART1->DR));   	//Ŀ�ĵ�ַ
    DMA_InitStructure.DMA_Memory0BaseAddr     = (u32)DMA_RxBuf;    			//Դ��ַ         
    DMA_InitStructure.DMA_DIR                 = DMA_DIR_PeripheralToMemory; //���䷽��   
    DMA_InitStructure.DMA_BufferSize          = AHRS_DMA_LEN;                    //���ݳ���        
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
	
	DMA_Init(DMA2_Stream5, &DMA_InitStructure);
	
	USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);							//ʹ�ܴ���DMA���սӿ�
	
	
	DMA_Cmd(DMA2_Stream5, ENABLE);
}
#endif
/**************************************************

��������AHRS_Tx_Enable()
��  �ߣ�������
��  �ڣ�2018.7.18
��  ����V1.00
˵  ����DMA����ʹ��
�޸ļ�¼��

**************************************************/
void AHRS_Tx_Enable(const u8 *data, u16 ndtr)
{
//	AHRS_TX_EN = 1;					//AHRS����ʹ��
	
	while(USART1_Tx_Flag);			//ȷ���ϴη����ѽ���
	USART1_Tx_Flag = 1;
	
	rt_memcpy(AHRS_TxBuf, data, ndtr);	
	DMA_SetCurrDataCounter(DMA2_Stream7, ndtr);
	DMA_Cmd(DMA2_Stream7, ENABLE);
	
//	AHRS_TX_EN = 0;					//AHRS�ָ�����״̬
}


/**************************************************

��������deal_irq_rx_end
��  �ߣ�������
��  �ڣ�2018.7.18
��  ����V1.00
˵  �������ڽ�����ɴ�����
��  ����
�޸ļ�¼���ɿ��н����޸�Ϊ�ֽڽ���  lxd  2018.8.3

**************************************************/
static void deal_irq_rx_end()
{
	u8 res;
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{
		res = USART1->DR;
		if((USART1_RX_STA & 0x8000) == 0)
		{
			if(USART1_RX_STA < USART1_BUFSIZE)		//�ж��ǹ����յ��㹻���ֽ���
				AHRS_RxBuf[USART1_RX_STA++] = res;
			else
			{
				USART1_RX_STA |= 1<<15;				//�������λ��λ
				ahrs_rev_flag = AHRS_Rev_Done;
			}
		}
	}
}


/**************************************************

��������deal_irq_tx_end
��  �ߣ�������
��  �ڣ�2018.7.18
��  ����V1.00
˵  �������ڽ�����ɴ�����
�޸ļ�¼��

**************************************************/

static void deal_irq_tx_end(void)
{
	if(USART_GetFlagStatus(USART1, USART_IT_TC) == RESET)
	{
		USART_ITConfig(USART1, USART_IT_TC, DISABLE);
		
		USART1_Tx_Flag = 0;
	}
}

/**************************************************

��������USART1_IRQHandler
��  �ߣ�������
��  �ڣ�2018.7.18
��  ����V1.00
˵  ����USART1�����жϴ�����
�޸ļ�¼��

**************************************************/
void USART1_IRQHandler(void)
{
	rt_interrupt_enter();
	
	deal_irq_tx_end();
	deal_irq_rx_end();
	
	rt_interrupt_leave();
}


/**************************************************

��������DMA2_Stream7_IRQHandler
��  �ߣ�������
��  �ڣ�2018.7.18
��  ����V1.00
˵  ����DMA�����жϴ�����
�޸ļ�¼��

**************************************************/
void DMA2_Stream7_IRQHandler(void)
{
	if(DMA_GetITStatus(DMA2_Stream7, DMA_IT_TCIF7) != RESET)
	{
		DMA_ClearFlag(DMA2_Stream7, DMA_IT_TCIF7);						//�巢����ɱ�־
		
		DMA_Cmd(DMA2_Stream7, DISABLE);									//�ر�DMA����
		
		USART_ITConfig(USART1, USART_IT_TC, ENABLE);					//�򿪷�������ж�
	}
}


/**************************************************

��������AHRS_Init
��  �ߣ�������
��  �ڣ�2018.7.18
��  ����V1.00
˵  �����ߵ���ʼ������
�޸ļ�¼��

**************************************************/
void AHRS_Init(u32 bound)
{
	USART1_Init(bound);
	AHRS_DMA_Tx_Init();
//	delay_us(1000);
	AHRS_Tx_Enable(close_stream, sizeof(close_stream));
#if 0
	AHRS_DMA_Rx_Init();
#endif
}


/**************************************************

��������AHRS_Require
��  �ߣ�������
��  �ڣ�2018.7.18
��  ����V1.00
˵  �����ߵ���������
�޸ļ�¼��

**************************************************/
void AHRS_Require(void)
{
	rt_memset(AHRS_RxBuf,0,sizeof(AHRS_RxBuf));
	USART1_RX_STA = 0;
	AHRS_Tx_Enable(polling_data, sizeof(polling_data));
}

/**************************************************

��������AHRS_data_analys
��  �ߣ�������
��  �ڣ�2018.8.3
��  ����V1.00
˵  ����AHRS�������ݽ���
��  ����
�޸ļ�¼��

**************************************************/

void AHRS_data_analys()
{
	u8 cnt = 0;
	u8 temp_buf[60];
#if 1
	static u8 temp_cnt = 0;
	static float temp_vel_x = 0.0;
	static float temp_vel_y = 0.0;
	static float temp_vel_z = 0.0;
	u8 buf[46];
#endif
	rt_memcpy(temp_buf, &AHRS_RxBuf[10], sizeof(temp_buf));
	if(temp_buf[0] == 0x75 && temp_buf[1] == 0x65)		//����ͷƥ��
	{
		rt_memcpy(accRate_data, &temp_buf[4], AHRS_DATA_LEN);
		rt_memcpy(angleRate_data, &temp_buf[18], AHRS_DATA_LEN);
		rt_memcpy(velocity_data, &temp_buf[32], AHRS_DATA_LEN);
		rt_memcpy(attitude_data, &temp_buf[46], AHRS_DATA_LEN);
		
		for(cnt = 0; cnt < 4; ++cnt)				//x����ٶ�
			conver.temp[cnt] = accRate_data[5-cnt];
		accRate.acc_x = conver.real * g + acc_x_compensation;
		
		for(cnt = 0; cnt < 4; ++cnt)				//y����ٶ�
			conver.temp[cnt] = accRate_data[9-cnt];
		accRate.acc_y = conver.real * g + acc_y_compensation;
		
		for(cnt = 0; cnt < 4; ++cnt)				//z����ٶ�
			conver.temp[cnt] = accRate_data[13-cnt];
		accRate.acc_z = conver.real * g;
		
#if 1		
		if(temp_cnt++ < 50)
		{
			temp_vel_x += accRate.acc_x;			
			temp_vel_y += accRate.acc_y;				
			temp_vel_z += accRate.acc_z;					
		}
		else
		{
			temp_cnt = 0;
			temp_vel_x /= 50.0;
			temp_vel_y /= 50.0;
			temp_vel_z /= 50.0;
			
			sprintf((char*)buf, "pos_0:%8.5f,pos_1:%8.5f,pos_02:%8.5f\r\n",temp_vel_x,temp_vel_y,temp_vel_z);
			rt_kprintf((char*)buf);
			
			temp_vel_x = 0.0;
			temp_vel_y = 0.0;
			temp_vel_z = 0.0;
			
		}
#endif		

		
		for(cnt = 0; cnt < 4; ++cnt)				//ת�����ٶ�
			conver.temp[cnt] = angleRate_data[5-cnt];
		angRate.angRate_x = conver.real * 180 / pi;
		
		for(cnt = 0; cnt < 4; ++cnt)				
			conver.temp[cnt] = angleRate_data[9-cnt];
		angRate.angRate_y = conver.real * 180 / pi;
		
		for(cnt = 0; cnt < 4; ++cnt)				
			conver.temp[cnt] = angleRate_data[13-cnt];
		angRate.angRate_z = conver.real * 180 / pi;
		
#if 0		
		sprintf((char*)buf, "pos_0:%8.5f,pos_1:%8.5f,pos_02:%8.5f\r\n",velocity.vel_x,velocity.vel_y,velocity.vel_z);
		rt_kprintf((char*)buf);

		if(temp_cnt++ < 50)
		{
			temp_vel_x += delta_velocity.vel_x;
			temp_vel_y += delta_velocity.vel_y;
			temp_vel_z += delta_velocity.vel_z;
		}
		else
		{
			temp_cnt = 0;
			temp_vel_x /= 50.0;
			temp_vel_y /= 50.0;
			temp_vel_z /= 50.0;
			
			temp_vel_x = 0.0;
			temp_vel_y = 0.0;
			temp_vel_z = 0.0;
		}
#endif
		
		for(cnt = 0; cnt < 4; ++cnt)				//ת����ƫ��
			conver.temp[cnt] = attitude_data[5-cnt];
		attitude.roll = conver.real * 180 / pi + roll_compensation;
		
		for(cnt = 0; cnt < 4; ++cnt)				
			conver.temp[cnt] = attitude_data[9-cnt];
		attitude.pitch =  conver.real * 180 / pi + pitch_compensation;
		
		for(cnt = 0; cnt < 4; ++cnt)				
			conver.temp[cnt] = attitude_data[13-cnt];
		attitude.yaw =  conver.real * 180 / pi;
		
#if 0		
		if(temp_cnt++ < 50)
		{
			temp_vel_x += attitude.roll;				//���������
			temp_vel_y += attitude.pitch;				//��б ������
			temp_vel_z += attitude.yaw;					//ƫ  ƽ���ƫת
		}
		else
		{
			temp_cnt = 0;
			temp_vel_x /= 50.0;
			temp_vel_y /= 50.0;
			temp_vel_z /= 50.0;
			
			sprintf((char*)buf, "pos_0:%8.5f,pos_1:%8.5f,pos_02:%8.5f\r\n",temp_vel_x,temp_vel_y,temp_vel_z);
			rt_kprintf((char*)buf);
			
			temp_vel_x = 0.0;
			temp_vel_y = 0.0;
			temp_vel_z = 0.0;
			
		}
#endif
		
//		for(cnt = 0; cnt < 4; ++cnt)				//ת���ٶ�
//			conver.temp[cnt] = velocity_data[5-cnt];
////		delta_velocity.vel_x = conver.real * g + vel_x_compensation;
//		velocity.vel_x += delta_velocity.vel_x;
//		
//		for(cnt = 0; cnt < 4; ++cnt)				
//			conver.temp[cnt] = velocity_data[9-cnt];
//		delta_velocity.vel_y =  conver.real * g + vel_y_compensation;
//		velocity.vel_y += delta_velocity.vel_y;
//		
//		for(cnt = 0; cnt < 4; ++cnt)				
//			conver.temp[cnt] = velocity_data[13-cnt];
//		delta_velocity.vel_z =  conver.real * g + vel_z_compensation;
//		velocity.vel_z += delta_velocity.vel_z;
	}
	
}




