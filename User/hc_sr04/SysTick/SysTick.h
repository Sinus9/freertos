#ifndef __SysTick_H
#define __SysTick_H




#define MOST 72000000          //ϵͳ����Ƶ��

#include "stm32f10x.h"
#include "ultrasonic.h"



void SysTick_Init(void);
void SysTick_IRQHandler1(void);



#endif

