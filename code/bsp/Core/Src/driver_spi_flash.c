/*----------------------------------------------------------------------------------------------------------------
 * Copyright(c)
 * ---------------------------------------------------------------------------------------------------------------
 * File Name : kprotocol.c
 * Author    : kirito
 * Brief     : 
 * Date      :  2020.11.06
 * ---------------------------------------------------------------------------------------------------------------
 * Modifier                                    Data                                             Brief
 * -------------------------------------------------------------------------------------------------------------*/

#include "driver_spi_flash.h"
#include "gpio.h"
#include "spi.h"

static uint16_t SPI_FLASH_TYPE=W25Q128;	//默认是W25Q256

// cs -low
static void SPI_FLASH_CS_LOW() {
    HAL_GPIO_WritePin(FLASH_CS_GPIO_Port,FLASH_CS_Pin,0);
}

// cs- high
static void  SPI_FLASH_CS_HIGH() {
    HAL_GPIO_WritePin(FLASH_CS_GPIO_Port,FLASH_CS_Pin,1);
}

// 内部函数

static uint8_t SPI_FLASH_SendByte(uint8_t byte);


uint8_t SPI_FLASH_ReadSR(uint8_t regno);             //读取状态寄存器 
void SPI_FLASH_4ByteAddr_Enable(void);     //使能4字节地址模式
void SPI_FLASH_Write_SR(uint8_t regno,uint8_t sr);   //写状态寄存器
void SPI_FLASH_Write_Enable(void);  		//写使能 
void SPI_FLASH_Write_Disable(void);		//写保护
void SPI_FLASH_Wait_Busy(void);           	//等待空闲
void SPI_FLASH_PowerDown(void);        	//进入掉电模式
void SPI_FLASH_WAKEUP(void);				//唤醒

// test
#define  FLASH_WriteAddress     0x00000
#define  FLASH_ReadAddress      FLASH_WriteAddress
#define  FLASH_SectorToErase    FLASH_WriteAddress

#define BufferSize 4096
uint8_t Tx_Buffer[] = "hell world123123";
uint8_t Rx_Buffer[BufferSize];


/**
* @ Function Name : kprotocol_init
* @ Author        : kirito
* @ Brief         : service
* @ Date          : 2020.11.06
* @ Modify        : ...
**/
char SPI_FLASH_Init(void)
{

    // cs -high
    SPI_FLASH_CS_HIGH();
    // enable spi1
    __HAL_SPI_ENABLE(&hspi2);
		
    SPI_FLASH_WAKEUP();
		
	  printf("spi test begin\r\n");
    // read flash_id
    uint16_t id = SPI_FLASH_ReadID();

    if(id == SPI_FLASH_TYPE) {
        printf("spi flash init ok!!!\r\n");
				
//        SPI_FLASH_Write(Tx_Buffer, FLASH_WriteAddress, BufferSize);
//				
//        SPI_FLASH_Read(Rx_Buffer, FLASH_WriteAddress, BufferSize);
//        printf("spi data:%s", Rx_Buffer);
			
				return 0;
    }
		return -1;
}

// ---------------- API ---- 


//读取W25QXX的状态寄存器，W25QXX一共有3个状态寄存器
//状态寄存器1：
//BIT7  6   5   4   3   2   1   0
//SPR   RV  TB BP2 BP1 BP0 WEL BUSY
//SPR:默认0,状态寄存器保护位,配合WP使用
//TB,BP2,BP1,BP0:FLASH区域写保护设置
//WEL:写使能锁定
//BUSY:忙标记位(1,忙;0,空闲)
//默认:0x00
//状态寄存器2：
//BIT7  6   5   4   3   2   1   0
//SUS   CMP LB3 LB2 LB1 (R) QE  SRP1
//状态寄存器3：
//BIT7      6    5    4   3   2   1   0
//HOLD/RST  DRV1 DRV0 (R) (R) WPS ADP ADS
//regno:状态寄存器号，范:1~3
//返回值:状态寄存器值
uint8_t SPI_FLASH_ReadSR(uint8_t regno)   
{  
	uint8_t byte=0,command=0; 
    switch(regno)
    {
        case 1:
            command=W25X_ReadStatusReg1;    //读状态寄存器1指令
            break;
        case 2:
            command=W25X_ReadStatusReg2;    //读状态寄存器2指令
            break;
        case 3:
            command=W25X_ReadStatusReg3;    //读状态寄存器3指令
            break;
        default:
            command=W25X_ReadStatusReg1;    
            break;
    }    
	SPI_FLASH_CS_LOW();                       	//使能器件   
	SPI_FLASH_SendByte(command);            //发送读取状态寄存器命令    
	byte=SPI_FLASH_SendByte(0Xff);          //读取一个字节  
	SPI_FLASH_CS_HIGH();                         	//取消片选     
	return byte;   
} 
//写W25QXX状态寄存器
void SPI_FLASH_Write_SR(uint8_t regno,uint8_t sr)   
{   
    uint8_t command=0;
    switch(regno)
    {
        case 1:
            command=W25X_WriteStatusReg1;    //写状态寄存器1指令
            break;
        case 2:
            command=W25X_WriteStatusReg2;    //写状态寄存器2指令
            break;
        case 3:
            command=W25X_WriteStatusReg3;    //写状态寄存器3指令
            break;
        default:
            command=W25X_WriteStatusReg1;    
            break;
    }   
	SPI_FLASH_CS_LOW();                            //使能器件   
	SPI_FLASH_SendByte(command);            //发送写取状态寄存器命令    
	SPI_FLASH_SendByte(sr);                 //写入一个字节  
	SPI_FLASH_CS_HIGH();                            //取消片选     	      
}   
//W25QXX写使能	
//将WEL置位   
void SPI_FLASH_Write_Enable(void)   
{
	SPI_FLASH_CS_LOW();                            //使能器件   
  SPI_FLASH_SendByte(W25X_WriteEnable);   //发送写使能  
	SPI_FLASH_CS_HIGH();                            //取消片选     	      
} 
//W25QXX写禁止	
//将WEL清零  
void SPI_FLASH_Write_Disable(void)   
{  
	SPI_FLASH_CS_LOW();                            //使能器件   
  SPI_FLASH_SendByte(W25X_WriteDisable);  //发送写禁止指令    
	SPI_FLASH_CS_HIGH();                            //取消片选     	      
} 

//读取芯片ID
//返回值如下:				   
//0XEF13,表示芯片型号为W25Q80  
//0XEF14,表示芯片型号为W25Q16    
//0XEF15,表示芯片型号为W25Q32  
//0XEF16,表示芯片型号为W25Q64 
//0XEF17,表示芯片型号为W25Q128 	  
//0XEF18,表示芯片型号为W25Q256
uint16_t SPI_FLASH_ReadID(void)
{
	uint16_t Temp = 0;	  
	SPI_FLASH_CS_LOW();				    
	SPI_FLASH_SendByte(0x90);//发送读取ID命令	    
	SPI_FLASH_SendByte(0x00); 	    
	SPI_FLASH_SendByte(0x00); 	    
	SPI_FLASH_SendByte(0x00); 	 			   
	Temp|=SPI_FLASH_SendByte(0xFF)<<8;  
	Temp|=SPI_FLASH_SendByte(0xFF);	 
	SPI_FLASH_CS_HIGH();				    
	return Temp;
}   		    
//读取SPI FLASH  
//在指定地址开始读取指定长度的数据
//pBuffer:数据存储区
//ReadAddr:开始读取的地址(24bit)
//NumByteToRead:要读取的字节数(最大65535)
void SPI_FLASH_Read(uint8_t* pBuffer,uint32_t ReadAddr,uint16_t NumByteToRead)   
{ 
 	uint16_t i;   										    
	SPI_FLASH_CS_LOW();                            //使能器件   
    SPI_FLASH_SendByte(W25X_ReadData);      //发送读取命令  
    if(SPI_FLASH_TYPE==W25Q256)                //如果是W25Q256的话地址为4字节的，要发送最高8位
    {
        SPI_FLASH_SendByte((uint8_t)((ReadAddr)>>24));    
    }
    SPI_FLASH_SendByte((uint8_t)((ReadAddr)>>16));   //发送24bit地址    
    SPI_FLASH_SendByte((uint8_t)((ReadAddr)>>8));   
    SPI_FLASH_SendByte((uint8_t)ReadAddr);   
    for(i=0;i<NumByteToRead;i++)
	{ 
        pBuffer[i]=SPI_FLASH_SendByte(0XFF);    //循环读数  
    }
	SPI_FLASH_CS_HIGH();  				    	      
}  
//SPI在一页(0~65535)内写入少于256个字节的数据
//在指定地址开始写入最大256字节的数据
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(24bit)
//NumByteToWrite:要写入的字节数(最大256),该数不应该超过该页的剩余字节数!!!	 
void SPI_FLASH_Write_Page(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite)
{
 	uint16_t i;  
    SPI_FLASH_Write_Enable();                  //SET WEL 
	SPI_FLASH_CS_LOW();                            //使能器件   
    SPI_FLASH_SendByte(W25X_PageProgram);   //发送写页命令   
    if(SPI_FLASH_TYPE==W25Q256)                //如果是W25Q256的话地址为4字节的，要发送最高8位
    {
        SPI_FLASH_SendByte((uint8_t)((WriteAddr)>>24)); 
    }
    SPI_FLASH_SendByte((uint8_t)((WriteAddr)>>16)); //发送24bit地址    
    SPI_FLASH_SendByte((uint8_t)((WriteAddr)>>8));   
    SPI_FLASH_SendByte((uint8_t)WriteAddr);   
    for(i=0;i<NumByteToWrite;i++)SPI_FLASH_SendByte(pBuffer[i]);//循环写数  
	SPI_FLASH_CS_HIGH();                            //取消片选 
	SPI_FLASH_Wait_Busy();					   //等待写入结束
} 
//无检验写SPI FLASH 
//必须确保所写的地址范围内的数据全部为0XFF,否则在非0XFF处写入的数据将失败!
//具有自动换页功能 
//在指定地址开始写入指定长度的数据,但是要确保地址不越界!
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(24bit)
//NumByteToWrite:要写入的字节数(最大65535)
//CHECK OK
void SPI_FLASH_Write_NoCheck(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite)   
{ 			 		 
	uint16_t pageremain;	   
	pageremain=256-WriteAddr%256; //单页剩余的字节数		 	    
	if(NumByteToWrite<=pageremain)pageremain=NumByteToWrite;//不大于256个字节
	while(1)
	{	   
		SPI_FLASH_Write_Page(pBuffer,WriteAddr,pageremain);
		if(NumByteToWrite==pageremain)break;//写入结束了
	 	else //NumByteToWrite>pageremain
		{
			pBuffer+=pageremain;
			WriteAddr+=pageremain;	

			NumByteToWrite-=pageremain;			  //减去已经写入了的字节数
			if(NumByteToWrite>256)pageremain=256; //一次可以写入256个字节
			else pageremain=NumByteToWrite; 	  //不够256个字节了
		}
	};	    
} 
//写SPI FLASH  
//在指定地址开始写入指定长度的数据
//该函数带擦除操作!
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(24bit)						
//NumByteToWrite:要写入的字节数(最大65535)   
uint8_t SPI_FLASH_BUFFER[4096];		 
void SPI_FLASH_Write(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite)   
{ 
	uint32_t secpos;
	uint16_t secoff;
	uint16_t secremain;	   
 	uint16_t i;    
	uint8_t * SPI_FLASH_BUF;	  
   	SPI_FLASH_BUF=SPI_FLASH_BUFFER;	     
 	secpos=WriteAddr/4096;//扇区地址  
	secoff=WriteAddr%4096;//在扇区内的偏移
	secremain=4096-secoff;//扇区剩余空间大小   
 	//printf("ad:%X,nb:%X\r\n",WriteAddr,NumByteToWrite);//测试用
 	if(NumByteToWrite<=secremain)secremain=NumByteToWrite;//不大于4096个字节
	while(1) 
	{	
		SPI_FLASH_Read(SPI_FLASH_BUF,secpos*4096,4096);//读出整个扇区的内容
		for(i=0;i<secremain;i++)//校验数据
		{
			if(SPI_FLASH_BUF[secoff+i]!=0XFF)break;//需要擦除  	  
		}
		if(i<secremain)//需要擦除
		{
			SPI_FLASH_Erase_Sector(secpos);//擦除这个扇区
			for(i=0;i<secremain;i++)	   //复制
			{
				SPI_FLASH_BUF[i+secoff]=pBuffer[i];	  
			}
			SPI_FLASH_Write_NoCheck(SPI_FLASH_BUF,secpos*4096,4096);//写入整个扇区  

		}else SPI_FLASH_Write_NoCheck(pBuffer,WriteAddr,secremain);//写已经擦除了的,直接写入扇区剩余区间. 				   
		if(NumByteToWrite==secremain)break;//写入结束了
		else//写入未结束
		{
			secpos++;//扇区地址增1
			secoff=0;//偏移位置为0 	 

		   	pBuffer+=secremain;  //指针偏移
			WriteAddr+=secremain;//写地址偏移	   
		   	NumByteToWrite-=secremain;				//字节数递减
			if(NumByteToWrite>4096)secremain=4096;	//下一个扇区还是写不完
			else secremain=NumByteToWrite;			//下一个扇区可以写完了
		}	 
	};	 
}
//擦除整个芯片		  
//等待时间超长...
void SPI_FLASH_Erase_Chip(void)   
{                                   
    SPI_FLASH_Write_Enable();                  //SET WEL 
    SPI_FLASH_Wait_Busy();   
  	SPI_FLASH_CS_LOW();                            //使能器件   
    SPI_FLASH_SendByte(W25X_ChipErase);        //发送片擦除命令  
	SPI_FLASH_CS_HIGH();                            //取消片选     	      
	SPI_FLASH_Wait_Busy();   				   //等待芯片擦除结束
}   
//擦除一个扇区
//Dst_Addr:扇区地址 根据实际容量设置
//擦除一个扇区的最少时间:150ms
void SPI_FLASH_Erase_Sector(uint32_t Dst_Addr)   
{  
	//监视falsh擦除情况,测试用   
 	//printf("fe:%x\r\n",Dst_Addr);	  
 	  Dst_Addr*=4096;
    SPI_FLASH_Write_Enable();                  //SET WEL 	 
    SPI_FLASH_Wait_Busy();   
  	SPI_FLASH_CS_LOW();                            //使能器件   
    SPI_FLASH_SendByte(W25X_SectorErase);   //发送扇区擦除指令 
    if(SPI_FLASH_TYPE==W25Q256)                //如果是W25Q256的话地址为4字节的，要发送最高8位
    {
        SPI_FLASH_SendByte((uint8_t)((Dst_Addr)>>24)); 
    }
    SPI_FLASH_SendByte((uint8_t)((Dst_Addr)>>16));  //发送24bit地址    
    SPI_FLASH_SendByte((uint8_t)((Dst_Addr)>>8));   
    SPI_FLASH_SendByte((uint8_t)Dst_Addr);  
		SPI_FLASH_CS_HIGH();                            //取消片选     	      
    SPI_FLASH_Wait_Busy();   				    //等待擦除完成
}  
//等待空闲
void SPI_FLASH_Wait_Busy(void)   
{   
	while((SPI_FLASH_ReadSR(1)&0x01)==0x01);   // 等待BUSY位清空
}  
//进入掉电模式
void SPI_FLASH_PowerDown(void)   
{ 
  	SPI_FLASH_CS_LOW();                            //使能器件   
    SPI_FLASH_SendByte(W25X_PowerDown);     //发送掉电命令  
	  SPI_FLASH_CS_HIGH();                            //取消片选     	      
                              //等待TPD  
}   
//唤醒
void SPI_FLASH_WAKEUP(void)   
{  
  	SPI_FLASH_CS_LOW();                              	//使能器件   
    SPI_FLASH_SendByte(W25X_ReleasePowerDown);	//  send W25X_PowerDown command 0xAB    
	SPI_FLASH_CS_HIGH();                              	//取消片选     	      
                               //等待TRES1
}   

//------------------- API end --------------

// 读取数据

// 写入数据
static uint8_t SPI_FLASH_SendByte(uint8_t byte)
{
    uint8_t Rxdata;
    HAL_SPI_TransmitReceive(&hspi2,&byte,&Rxdata,1, 2000);
    return Rxdata;
}


