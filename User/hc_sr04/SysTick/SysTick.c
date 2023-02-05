#include "SysTick.h"
#include "bsp_usart.h"
			
u32 DELAY_USX=1000;

///////////////////////////////////////////////////
//
//     函数：   SysTick_Init()
//     输入：   无
//     输出：   无
//     功能：   初始化系统时钟，用于定时发送超声波
//              采集距离
//     说明：   系统时钟每1ms中断一次，每中断一千次便
//              输出一次距离，即每隔一秒输出一次距离
//
/////////////////////////////////////////////////
void SysTick_Init(void)
{
  if(SysTick_Config(MOST/1000)==1)
		while(1);
	SysTick->CTRL|=0<<0;
	SysTick->CTRL|=1<<0;          //使能系统时钟
}

/////////////////////////////////////////////////
//
//     函数：   SysTick_IRQHander()
//     输入：   无
//     输出：   无
//     功能：   间接充当系统时钟的中断函数
//     说明：   该函数被系统时钟的中断函数调用，
//              每1ms进入该函数，每次DELAY_USX减一，
//              当DELAY_USX减到0时采集距离
//
/////////////////////////////////////////////////
void SysTick_IRQHandler1(void)
{
  if(DELAY_USX>0)
	{
		DELAY_USX--;                 //时间标志自减1
	  if(DELAY_USX==0)
	  {
	    GetDistance();            //调用函数采集距离
	  	DELAY_USX=1000;           //时间标志重新赋值
  	}
	}
}






