#ifndef __DELAY_H
#define __DELAY_H 			   

#include "stm32f10x.h"



extern RCC_ClocksTypeDef  rcc_clocks;  	 //��tzsk.c�ж����оƬʱ��Ƶ�ʽṹ����

void delay_init(u8 SYSCLK);
void delay_us(u32 nus);
void delay_ms(u16 nms);
#endif



