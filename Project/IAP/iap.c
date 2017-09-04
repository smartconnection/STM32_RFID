#include "iap.h"
#include "card_config.h"
#include "spi_flash.h"
#include "crc.h"
#include "string.h"
#include "stm32f10x.h"
#include "stm32f10x_flash.h"

#define FLASH_APP1_ADDR		0x08005000  	//��һ��Ӧ�ó�����ʼ��ַ(�����FLASH)
											//�����Ŀռ�ΪIAPʹ��,Ԥ��20KB�ռ��BOOTLOADERʹ��

volatile uint8_t iap_buf[1024] = {0}; //���ڻ������ݵ�����
u16 receiveDataCur = 0;	//��ǰiapbuffer���Ѿ��������ݳ���,һ���������֮��д��flash������
u32 addrCur = FLASH_APP1_ADDR;			//��ǰϵͳд���ַ,ÿ��д��֮���ַ����2048

typedef  void (*iapfun)(void);				//����һ���������͵Ĳ���.
iapfun jump2app;

//����ջ����ַ
//addr:ջ����ַ
__asm void MSR_MSP(u32 addr) 
{
    MSR MSP, r0 			//set Main Stack value
    BX r14
}

//��ת��Ӧ�ó����
//appxaddr:�û�������ʼ��ַ.
void iap_load_app(u32 appxaddr)
{
	if(((*(vu32*)appxaddr)&0x2FFE0000)==0x20000000)	//���ջ����ַ�Ƿ�Ϸ�.0x20000000��sram����ʼ��ַ,Ҳ�ǳ����ջ����ַ
	{ 
		delay_ms(10);
		jump2app=(iapfun)*(vu32*)(appxaddr+4);		//�û��������ڶ�����Ϊ����ʼ��ַ(��λ��ַ)		
		MSR_MSP(*(vu32*)appxaddr);					//��ʼ��APP��ջָ��(�û��������ĵ�һ�������ڴ��ջ����ַ)
		jump2app();									//��ת��APP.
	}
	else
	{
		printf("program in flash is error\r\n");
	}
}

u16 STMFLASH_ReadHalfWord(u32 faddr)
{
    return *(vu16*)faddr; 
}

//��ָ����ַ��ʼ����ָ�����ȵ�����
//ReadAddr:��ʼ��ַ
//pBuffer:����ָ��
//NumToWrite:����(16λ)��
void STMFLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead)   	
{
    u16 i;
    for(i=0;i<NumToRead;i++)
    {
        pBuffer[i]=STMFLASH_ReadHalfWord(ReadAddr);//��ȡ2���ֽ�.
        ReadAddr+=2;//ƫ��2���ֽ�.	
    }
}

void JumpToApp(void)
{
	if(((*(vu32*)(FLASH_APP1_ADDR+4))&0xFF000000)==0x08000000)//�ж��Ƿ�Ϊ0X08XXXXXX.
	{	 
		iap_load_app(FLASH_APP1_ADDR);//��ת��app�ĸ�λ������ַ
	}
	else
	{
		printf("��FLASHӦ�ó���,�޷�ִ��!\r\n");
	}
}

uint8_t CheckProgFlag(void)
{
	uint16_t i;
	uint16_t crc_result;
	MCUUpgradeInfo mcu1_data,mcu2_data;
	SPI_Flash_Read(UPDATEADDR1 * 4096,sizeof(MCUUpgradeInfo),(uint8_t *)&mcu1_data);
	SPI_Flash_Read(UPDATEADDR2 * 4096,sizeof(MCUUpgradeInfo),(uint8_t *)&mcu2_data);
	crc_result = make_crc((unsigned char *)&mcu1_data,sizeof(MCUUpgradeInfo)-2);
	if((crc_result != mcu1_data.crc) || (crc_result == 0))
	{
		return 1;
	}
		
	if(0 == memcmp(&mcu1_data,&mcu2_data,sizeof(MCUUpgradeInfo) ))
	{
		return 0;
	}
		
	if ( mcu1_data.jump_times > 0 )
	{
		mcu1_data.jump_times--;
		mcu1_data.crc = make_crc((unsigned char *)&mcu1_data,sizeof(MCUUpgradeInfo)-2);
		SPI_FLASH_Erase(UPDATEADDR1,UPDATEADDR1);
		memcpy(buff,(uint8_t *)&mcu1_data,sizeof(MCUUpgradeInfo));
		SPI_Flash_Write(UPDATEADDR1*4096,256,buff);
		return 0;
	}
	return 1;
}



uint8_t EraseMcuFlash(uint8_t bgn,uint8_t end)
{
	uint8_t i;
	FLASH_Status flash_status = FLASH_COMPLETE;
	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR);
	for(i=bgn;i<=end;i++)
	{
		flash_status = FLASH_ErasePage(i*FLASH_PAGE_SIZE + 0x08000000);
		if(flash_status != FLASH_COMPLETE)
		{
			return 1;
		}
	}
	return 0;
}



//WriteAddr:��ʼ��ַ
//pBuffer:����ָ��
//NumToWrite:����(16λ)��   
uint8_t STMFLASH_Write_AndCheck(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)   
{ 			 		 
    u16 i;
		u32 tmp_addr;
    FLASH_Status flash_status = FLASH_COMPLETE;
		tmp_addr = WriteAddr;
    for(i=0;i<NumToWrite;i++)
    {
        flash_status = FLASH_ProgramHalfWord(tmp_addr,pBuffer[i]);
        if(flash_status != FLASH_COMPLETE)
        {
					return 1;
        }
        tmp_addr+=2;//��ַ����2.
    }
		tmp_addr = WriteAddr;
		for(i=0;i<NumToWrite;i++)
    {
				if(pBuffer[i] !=STMFLASH_ReadHalfWord(tmp_addr))//��ȡ2���ֽ�.
				{
					return 1;
				}
        tmp_addr+=2;//ƫ��2���ֽ�.	
    }
    return 0;
} 


uint8_t WriteMcuFlash(uint8_t ben,uint8_t end,uint32_t des)
{
  if(des<STM32_FLASH_BASE||(des>=(STM32_FLASH_BASE+1024*STM32_FLASH_SIZE)))
	{
		return 2;//�Ƿ���ַ
	}
    FLASH_Unlock();						//����
    if(STMFLASH_Write_AndCheck(des,(u16*)iap_buf,STM_SECTOR_SIZE/2))//д�Ѿ������˵�,ֱ��д������ʣ������.
    {
			FLASH_Lock();//����
			return 1;
    }
    FLASH_Lock();//����
    return 0;
}

