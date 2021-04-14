#ifndef _SIIC_H
#define _SIIC_H


#include "stdio.h"
#include "stdint.h"


void SIIC_Init(void);
void SIIC_Start(void);
void SIIC_Stop(void);
uint8_t SIIC_Wait_Ack(void);
void SIIC_Ack(void);
void SIIC_NAck(void);
void SIIC_Send_Byte(uint8_t txd);
uint8_t SIIC_Read_Byte(unsigned char ack);
#endif
