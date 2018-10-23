/**************************************************

�ļ�����delay.c
��  �ߣ�������
��  �ڣ�2018.7.18
��  ����V1.00
˵  �����ṩ׼ȷ��ms/us����ʱ����
�޸ļ�¼��

**************************************************/




#include "delay.h"




static u32  fac_us=0;							//us��ʱ������			   
static u32  fac_ms=0;							//ms��ʱ������,��os��,����ÿ�����ĵ�ms��
	
#if SYSTEM_SUPPORT_OS									//���SYSTEM_SUPPORT_OS������,˵��Ҫ֧��OS��


uint8_t OSRunning = 0;									//OS���б�־λ����ϵͳ��ʼ����������λ  lxd 18.7.17
extern volatile rt_uint8_t rt_interrupt_nest;
								
#define delay_osrunning		OSRunning					//OS�Ƿ����б��,0,������;1,������
#define delay_ostickspersec	RT_TICK_PER_SECOND			//OSʱ�ӽ���,��ÿ����ȴ���
#define delay_osintnesting 	rt_interrupt_nest			//�ж�Ƕ�׼���,���ж�Ƕ�״���


/**************************************************

��������delay_osschedlock
��  �ߣ�������
��  �ڣ�2018.7.18
��  ����V1.00
˵  ����us����ʱʱ,�ر��������(��ֹ���us���ӳ�)
�޸ļ�¼��

**************************************************/

void delay_osschedlock(void)
{
	rt_enter_critical();
}


/**************************************************

��������delay_osschedlock
��  �ߣ�������
��  �ڣ�2018.7.18
��  ����V1.00
˵  ����us����ʱʱ,�ָ��������
�޸ļ�¼��

**************************************************/

void delay_osschedunlock(void)
{	
	rt_exit_critical();
} 

#endif
			   
/**************************************************

��������delay_init()
��  �ߣ�������
��  �ڣ�2018.7.18
��  ����V1.00
˵  ������ʼ��fac_ms��fac_us
�޸ļ�¼��

**************************************************/
void delay_init()
{

	fac_us= SystemCoreClock/1000000;						//�����Ƿ�ʹ��OS,fac_us����Ҫʹ��
#if SYSTEM_SUPPORT_OS 										//�����Ҫ֧��OS.	
	fac_ms=1000/delay_ostickspersec;						//����OS������ʱ�����ٵ�λ,��һ��tick��ʱ��ms��      
#else
	fac_ms= fac_us*1000;								//��OS��,����ÿ��ms��Ҫ��systickʱ����   
#endif
}								    

#if SYSTEM_SUPPORT_OS 										//�����Ҫ֧��OS.


/**************************************************

��������delay_us
��  �ߣ�������
��  �ڣ�2018.7.18
��  ����V1.00
˵  ����OS��us����ʱ����
���������nus ��ʱ��us������ֵ��ΧΪһ��tick�ڣ���0-2000������ֵ�ϴ�ʱ
		  �ֿ���������������ʱ
�޸ļ�¼��

**************************************************/
void delay_us(u32 nus)
{		
	u32 ticks;
	u32 told,tnow,tcnt=0;
	u32 reload=SysTick->LOAD;				//LOAD��ֵ	    	 
	ticks=nus*fac_us; 						//��Ҫ�Ľ����� 
	delay_osschedlock();					//��ֹOS���ȣ���ֹ���us��ʱ
	told=SysTick->VAL;        				//�ս���ʱ�ļ�����ֵ
	while(1)
	{
		tnow=SysTick->VAL;	
		if(tnow!=told)
		{	    
			if(tnow<told)tcnt+=told-tnow;	//����ע��һ��SYSTICK��һ���ݼ��ļ������Ϳ�����.
			else tcnt+=reload-tnow+told;	    
			told=tnow;
			if(tcnt>=ticks)break;			//ʱ�䳬��/����Ҫ�ӳٵ�ʱ��,���˳�.
		}  
	};
	delay_osschedunlock();					//�ָ�OS����											    
}  


/**************************************************

��������delay_ms
��  �ߣ�������
��  �ڣ�2018.7.18
��  ����V1.00
˵  ����OS��ms����ʱ����
���������nms ��ʱ��ms��
�޸ļ�¼��

**************************************************/
void delay_ms(u32 nms)
{	
	u32 offset = SysTick->LOAD - SysTick->VAL;				//��ʱ����ֵ
	u16 tick_num = 0;										//��ʱtick��
	u32 us_num = 0;											//��ʱus��
	
	if(delay_osrunning)		//���OS�Ѿ�������,���Ҳ������ж�����(�ж����治���������)	    
	{
		if(nms >= fac_ms)
		{
			tick_num = nms/fac_ms;							//��Ҫ��ʱ��tick��
			nms %= fac_ms;
			us_num = (u32)nms*1000 + offset/fac_us;
			if(us_num > (u32)fac_ms * 1000)
			{
				++tick_num;									//����tick��							
				us_num -= ((u32)fac_ms * 1000);
			}
		}
		else
		{
			tick_num = 0;
			us_num = nms * 1000;
		}
	}
	tick_num ? rt_thread_delay(tick_num) : NULL;			//��ʱtick
	delay_us(us_num);										//��ͨ��ʽ��ʱ
}


#else  														//��ʹ��OSʱ

/**************************************************

��������delay_us
��  �ߣ�������
��  �ڣ�2018.7.18
��  ����V1.00
˵  ������OS��us����ʱ������nusֵ��Ҫ����798915us(���ֵ2^24/fac_us)
���������nus ��ʱ��us��
�޸ļ�¼��

**************************************************/
void delay_us(u32 nus)
{		
	u32 temp;	    	 
	SysTick->LOAD=nus*fac_us; 				//ʱ�����	  		 
	SysTick->VAL=0x00;        				//��ռ�����
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ; //��ʼ���� 	 
	do
	{
		temp=SysTick->CTRL;
	}while((temp&0x01)&&!(temp&(1<<16)));	//�ȴ�ʱ�䵽��   
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk; //�رռ�����
	SysTick->VAL =0X00;       				//��ռ����� 
}


/**************************************************

��������delay_xms
��  �ߣ�������
��  �ڣ�2018.7.18
��  ����V1.00
˵  ������OS��ms����ʱ������nmsֵ��Ҫ����798ms(Ϊһ��SysTick������ʱ�����ֵ)
���������xms ��ʱ��ms��
�޸ļ�¼��

**************************************************/
void delay_xms(u16 nms)
{	 		  	  
	u32 temp;		   
	SysTick->LOAD=(u32)nms*fac_ms;						//ʱ�����(SysTick->LOADΪ24bit)
	SysTick->VAL =0x00;           						//��ռ�����
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;          	//��ʼ���� 
	do
	{
		temp=SysTick->CTRL;
	}while((temp&0x01)&&!(temp&(1<<16)));				//�ȴ�ʱ�䵽��   
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;       		//�رռ�����
	SysTick->VAL =0X00;     		  					//��ռ�����	  	    
} 

/**************************************************

��������delay_ms
��  �ߣ�������
��  �ڣ�2018.7.18
��  ����V1.00
˵  ������OS��ms����ʱ����
���������nus ��ʱ��ms��
�޸ļ�¼��

**************************************************/
void delay_ms(u16 nms)
{	 	 
	u8 repeat=nms/540;								//������540,�ǿ��ǵ�ĳЩ�ͻ����ܳ�Ƶʹ��,
													//���糬Ƶ��248M��ʱ��,delay_xms���ֻ����ʱ541ms������
	u16 remain=nms%540;
	while(repeat)
	{
		delay_xms(540);
		repeat--;
	}
	if(remain)delay_xms(remain);
} 
#endif
			 



































