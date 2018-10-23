/**************************************************

文件名：delay.c
作  者：刘晓东
日  期：2018.7.18
版  本：V1.00
说  明：提供准确的ms/us级延时函数
修改记录：

**************************************************/




#include "delay.h"




static u32  fac_us=0;							//us延时倍乘数			   
static u32  fac_ms=0;							//ms延时倍乘数,在os下,代表每个节拍的ms数
	
#if SYSTEM_SUPPORT_OS									//如果SYSTEM_SUPPORT_OS定义了,说明要支持OS了


uint8_t OSRunning = 0;									//OS运行标志位，在系统初始化结束后置位  lxd 18.7.17
extern volatile rt_uint8_t rt_interrupt_nest;
								
#define delay_osrunning		OSRunning					//OS是否运行标记,0,不运行;1,在运行
#define delay_ostickspersec	RT_TICK_PER_SECOND			//OS时钟节拍,即每秒调度次数
#define delay_osintnesting 	rt_interrupt_nest			//中断嵌套级别,即中断嵌套次数


/**************************************************

函数名：delay_osschedlock
作  者：刘晓东
日  期：2018.7.18
版  本：V1.00
说  明：us级延时时,关闭任务调度(防止打断us级延迟)
修改记录：

**************************************************/

void delay_osschedlock(void)
{
	rt_enter_critical();
}


/**************************************************

函数名：delay_osschedlock
作  者：刘晓东
日  期：2018.7.18
版  本：V1.00
说  明：us级延时时,恢复任务调度
修改记录：

**************************************************/

void delay_osschedunlock(void)
{	
	rt_exit_critical();
} 

#endif
			   
/**************************************************

函数名：delay_init()
作  者：刘晓东
日  期：2018.7.18
版  本：V1.00
说  明：初始化fac_ms与fac_us
修改记录：

**************************************************/
void delay_init()
{

	fac_us= SystemCoreClock/1000000;						//不论是否使用OS,fac_us都需要使用
#if SYSTEM_SUPPORT_OS 										//如果需要支持OS.	
	fac_ms=1000/delay_ostickspersec;						//代表OS可以延时的最少单位,即一个tick延时的ms数      
#else
	fac_ms= fac_us*1000;								//非OS下,代表每个ms需要的systick时钟数   
#endif
}								    

#if SYSTEM_SUPPORT_OS 										//如果需要支持OS.


/**************************************************

函数名：delay_us
作  者：刘晓东
日  期：2018.7.18
版  本：V1.00
说  明：OS下us级延时函数
输入参数：nus 延时的us数，该值范围为一个tick内，即0-2000。但当值较大时
		  分开几个函数进行延时
修改记录：

**************************************************/
void delay_us(u32 nus)
{		
	u32 ticks;
	u32 told,tnow,tcnt=0;
	u32 reload=SysTick->LOAD;				//LOAD的值	    	 
	ticks=nus*fac_us; 						//需要的节拍数 
	delay_osschedlock();					//阻止OS调度，防止打断us延时
	told=SysTick->VAL;        				//刚进入时的计数器值
	while(1)
	{
		tnow=SysTick->VAL;	
		if(tnow!=told)
		{	    
			if(tnow<told)tcnt+=told-tnow;	//这里注意一下SYSTICK是一个递减的计数器就可以了.
			else tcnt+=reload-tnow+told;	    
			told=tnow;
			if(tcnt>=ticks)break;			//时间超过/等于要延迟的时间,则退出.
		}  
	};
	delay_osschedunlock();					//恢复OS调度											    
}  


/**************************************************

函数名：delay_ms
作  者：刘晓东
日  期：2018.7.18
版  本：V1.00
说  明：OS下ms级延时函数
输入参数：nms 延时的ms数
修改记录：

**************************************************/
void delay_ms(u32 nms)
{	
	u32 offset = SysTick->LOAD - SysTick->VAL;				//延时补偿值
	u16 tick_num = 0;										//延时tick数
	u32 us_num = 0;											//延时us数
	
	if(delay_osrunning)		//如果OS已经在跑了,并且不是在中断里面(中断里面不能任务调度)	    
	{
		if(nms >= fac_ms)
		{
			tick_num = nms/fac_ms;							//需要延时的tick数
			nms %= fac_ms;
			us_num = (u32)nms*1000 + offset/fac_us;
			if(us_num > (u32)fac_ms * 1000)
			{
				++tick_num;									//补偿tick数							
				us_num -= ((u32)fac_ms * 1000);
			}
		}
		else
		{
			tick_num = 0;
			us_num = nms * 1000;
		}
	}
	tick_num ? rt_thread_delay(tick_num) : NULL;			//延时tick
	delay_us(us_num);										//普通方式延时
}


#else  														//不使用OS时

/**************************************************

函数名：delay_us
作  者：刘晓东
日  期：2018.7.18
版  本：V1.00
说  明：非OS下us级延时函数，nus值不要大于798915us(最大值2^24/fac_us)
输入参数：nus 延时的us数
修改记录：

**************************************************/
void delay_us(u32 nus)
{		
	u32 temp;	    	 
	SysTick->LOAD=nus*fac_us; 				//时间加载	  		 
	SysTick->VAL=0x00;        				//清空计数器
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ; //开始倒数 	 
	do
	{
		temp=SysTick->CTRL;
	}while((temp&0x01)&&!(temp&(1<<16)));	//等待时间到达   
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk; //关闭计数器
	SysTick->VAL =0X00;       				//清空计数器 
}


/**************************************************

函数名：delay_xms
作  者：刘晓东
日  期：2018.7.18
版  本：V1.00
说  明：非OS下ms级延时函数，nms值不要大于798ms(为一次SysTick可以延时的最大值)
输入参数：xms 延时的ms数
修改记录：

**************************************************/
void delay_xms(u16 nms)
{	 		  	  
	u32 temp;		   
	SysTick->LOAD=(u32)nms*fac_ms;						//时间加载(SysTick->LOAD为24bit)
	SysTick->VAL =0x00;           						//清空计数器
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;          	//开始倒数 
	do
	{
		temp=SysTick->CTRL;
	}while((temp&0x01)&&!(temp&(1<<16)));				//等待时间到达   
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;       		//关闭计数器
	SysTick->VAL =0X00;     		  					//清空计数器	  	    
} 

/**************************************************

函数名：delay_ms
作  者：刘晓东
日  期：2018.7.18
版  本：V1.00
说  明：非OS下ms级延时函数
输入参数：nus 延时的ms数
修改记录：

**************************************************/
void delay_ms(u16 nms)
{	 	 
	u8 repeat=nms/540;								//这里用540,是考虑到某些客户可能超频使用,
													//比如超频到248M的时候,delay_xms最大只能延时541ms左右了
	u16 remain=nms%540;
	while(repeat)
	{
		delay_xms(540);
		repeat--;
	}
	if(remain)delay_xms(remain);
} 
#endif
			 



































