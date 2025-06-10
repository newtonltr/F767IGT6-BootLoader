#ifndef __COUNT_H
#define __COUNT_H

#include "main.h"

unsigned long HextoDec(const unsigned char *hex, int length);
int DectoHex(int dec, unsigned char *hex, int length);
unsigned long power(int base, int times);
unsigned long  BCDtoDec(const unsigned char *bcd, int length);
int DectoBCD(int temp);
void FloatToByte(float floatNum,unsigned char* byteArry);
void DoubleToByte(double floatNum,unsigned char* byteArry);
unsigned short CalcCRC(uint8_t *data, uint32_t size);
uint16_t  CRC_Check(uint8_t *m_Data,uint8_t m_Size);
uint16_t  SUM_Check(uint8_t *m_Data,uint8_t m_Size);
uint16_t CRC_Compute(uint8_t *puchMsg, uint16_t usDataLen);
uint16_t crc_compute_lsb(uint8_t *data, uint16_t len);
uint32_t  mpeg_crc32(const uint8_t *data, uint16_t len);
#endif
