#ifndef __DRIVER_SPI_FLASH_H_
#define __DRIVER_SPI_FLASH_H_


#include "stdint.h"
#include "stdio.h"



#define W25Q80 	0XEF13 	
#define W25Q16 	0XEF14
#define W25Q32 	0XEF15
#define W25Q64 	0XEF16
#define W25Q128	0XEF17
#define W25Q256 0XEF18

	   
 
////////////////////////////////////////////////////////////////////////////////// 
//指令表
#define W25X_WriteEnable		0x06 
#define W25X_WriteDisable		0x04 
#define W25X_ReadStatusReg1		0x05 
#define W25X_ReadStatusReg2		0x35 
#define W25X_ReadStatusReg3		0x15 
#define W25X_WriteStatusReg1    0x01 
#define W25X_WriteStatusReg2    0x31 
#define W25X_WriteStatusReg3    0x11 
#define W25X_ReadData			0x03 
#define W25X_FastReadData		0x0B 
#define W25X_FastReadDual		0x3B 
#define W25X_PageProgram		0x02 
#define W25X_BlockErase			0xD8 
#define W25X_SectorErase		0x20 
#define W25X_ChipErase			0xC7 
#define W25X_PowerDown			0xB9 
#define W25X_ReleasePowerDown	0xAB 
#define W25X_DeviceID			0xAB 
#define W25X_ManufactDeviceID	0x90 
#define W25X_JedecDeviceID		0x9F 
#define W25X_Enable4ByteAddr    0xB7
#define W25X_Exit4ByteAddr      0xE9
#define W25X_SetReadParam		0xC0 
#define W25X_EnterQPIMode       0x38
#define W25X_ExitQPIMode        0xFF

#define QUAD_INOUT_FAST_READ_CMD_4BYTE       0xEC

//  w25q64和w25q128使用24bits,  256使用32bits
#define QSPI_USE_ADRESS_BITS   QSPI_ADDRESS_24_BITS
//#define QSPI_USE_ADRESS_BITS   QSPI_ADDRESS_32_BITS

void QSPI_FLASH_Init(void);					//初始化W25QXX
uint16_t  QSPI_FLASH_ReadID(void);  	    		//读取FLASH ID
void QSPI_FLASH_Write_NoCheck(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite);//写flash,不校验
void QSPI_FLASH_Read(uint8_t* pBuffer,uint32_t ReadAddr,uint16_t NumByteToRead);   //读取flash
void QSPI_FLASH_Write(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite);//写入flash
void QSPI_FLASH_Erase_Chip(void);    	  	//整片擦除
void QSPI_FLASH_Erase_Sector(uint32_t Dst_Addr);	//扇区擦除


#endif
