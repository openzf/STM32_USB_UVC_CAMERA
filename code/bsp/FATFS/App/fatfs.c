/**
  ******************************************************************************
  * @file   fatfs.c
  * @brief  Code for fatfs applications
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

#include "fatfs.h"

uint8_t retSD;    /* Return value for SD */
char SDPath[4];   /* SD logical drive path */
FATFS SDFatFS;    /* File system object for SD logical drive */
FIL SDFile;       /* File object for SD */
uint8_t retUSER;    /* Return value for USER */
char USERPath[4];   /* USER logical drive path */
FATFS USERFatFS;    /* File system object for USER logical drive */
FIL USERFile;       /* File object for USER */

/* USER CODE BEGIN Variables */
#define WORK_BUFFER_SIZE  4096
static char work_buffer[WORK_BUFFER_SIZE];
/* USER CODE END Variables */

void MX_FATFS_Init(void)
{
  /*## FatFS: Link the SD driver ###########################*/
  retSD = FATFS_LinkDriver(&SD_Driver, SDPath);
  /*## FatFS: Link the USER driver ###########################*/
  retUSER = FATFS_LinkDriver(&USER_Driver, USERPath);

  /* USER CODE BEGIN Init */
  /* additional user code for init */
  /* USER CODE END Init */
}

/**
  * @brief  Gets Time from RTC
  * @param  None
  * @retval Time in DWORD
  */
DWORD get_fattime(void)
{
  /* USER CODE BEGIN get_fattime */
  return 0;
  /* USER CODE END get_fattime */
}

/* USER CODE BEGIN Application */

void MX_FATFS_USER_Init()
{
	    // 挂载spi flash
    retSD = f_mount(&USERFatFS, USERPath, 0);
    // 如果挂载不成�?
    if(retSD)
    {
        // 如果没文件系�?
        if(retSD == FR_NO_FILESYSTEM)
        {
            printf("spi-flash f_mount no file system  begin mkfs spi-flash\r\n");
            retSD = f_mkfs(USERPath,FM_FAT,0,work_buffer,WORK_BUFFER_SIZE);
            if(retSD != FR_OK)
            {
                printf("f_mkfs spi-flash error,err = %d\r\n", retSD);
                while(1);
            }
            else
            {
                printf("f_mount spi-flash ok \r\n");
                retSD = f_mount(&USERFatFS, USERPath, 0);
                if(retSD != FR_OK)
                {
                    printf("f_mount spi-flash error,err = %d\r\n", retSD);
                }
                else {
                    printf("f_mount spi-flash ok\r\n");
                }
            }
        }
        else
        {
            printf("f_mount spi-flash,err = %d\r\n", retSD);
            while(1);
        }
    }
    else {
        printf("spi-flash init fs ok!\r\n");
    }
}

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
