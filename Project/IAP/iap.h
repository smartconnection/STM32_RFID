#ifndef __IAP_H_
#define __IAP_H_

#include "usart.h"
#include "sys.h"
#include "delay.h"


//0x08000000 flash��ʼ��ַ
//0x2000 iap���볤��
//0x1ffc-0x1fff��Ϊ�̻����ô���

extern volatile uint8_t iap_buf[]; //���ڻ������ݵ�����


#define STM32_FLASH_SIZE 128 	 		//��ѡSTM32��FLASH������С(��λΪK)
#define FLASH_PAGE_SIZE (1024)
#if STM32_FLASH_SIZE<256
#define STM_SECTOR_SIZE 1024 //�ֽ�
#else 
#define STM_SECTOR_SIZE	2048
#endif

//FLASH��ʼ��ַ
#define STM32_FLASH_BASE 0x08000000 	//STM32 FLASH����ʼ��ַ


uint8_t CheckProgFlag(void);
void JumpToApp(void);
uint8_t EraseMcuFlash(uint8_t bgn,uint8_t end);
uint8_t WriteMcuFlash(uint8_t ben,uint8_t end,uint32_t des);
#endif


















