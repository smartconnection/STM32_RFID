#ifndef __LCD5110_H
#define __LCD5110_H

#include "sys.h"

/**************************ö�� D/Cģʽѡ�� **************************/
typedef enum
{
  DC_CMD  = 0,	//д����
	DC_DATA = 1		//д����	
} DCType;


#define LCD_DC		PBout(12)
#define LCD_RST		PBout(3)
#define LCD_CE		PAout(15)

#define EN_SPI2_DMA 0




void LCD_GPIO_Init(void);
void LCD5110_Init(void);
void LCD_Send(u8 data, DCType dc);
void LCD_Clear(void);
void LCD_Write_EnStr(u8 X, u8 Y, u8* s);
void LCD_Printf(u8 x,u8 y,const char *fmt,...);







#endif


