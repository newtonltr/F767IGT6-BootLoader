#ifndef _EMB_FLASH_H
#define _EMB_FLASH_H

#include "stm32f7xx.h"

//FLASH起始地址
#define FLASH_WAITETIME 	50000          //FLASH等待超时时间

//sector first address
#define ADDR_SECTOR_0	0x8000000
#define ADDR_SECTOR_1	(ADDR_SECTOR_0 + 32*1024)
#define ADDR_SECTOR_2	(ADDR_SECTOR_1 + 32*1024)
#define ADDR_SECTOR_3	(ADDR_SECTOR_2 + 32*1024)
#define ADDR_SECTOR_4	(ADDR_SECTOR_3 + 32*1024)
#define ADDR_SECTOR_5	(ADDR_SECTOR_4 + 128*1024)
#define ADDR_SECTOR_6	(ADDR_SECTOR_5 + 256*1024)
#define ADDR_SECTOR_7	(ADDR_SECTOR_6 + 256*1024)

uint32_t FLASH_ReadWord(uint32_t faddr);
uint8_t FLASH_GetFlashSector(uint32_t addr);
void FLASH_Write(uint32_t WriteAddr,uint32_t *pBuffer,uint32_t NumToWrite);
void FLASH_Read(uint32_t ReadAddr,uint32_t *pBuffer,uint32_t NumToRead);







#endif





