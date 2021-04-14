#include "driver_qspi_flash.h"
#include "quadspi.h"


uint16_t QSPI_FLASH_TYPE=W25Q128;	//默认是W25Q256
uint8_t QSPI_FLASH_QPI_MODE=0;		//QSPI模式标志:0,SPI模式;1,QPI模式.


static void QSPI_FLASH_Qspi_Enable(void);			//使能QSPI模式
static void QSPI_FLASH_Qspi_Disable(void);			//关闭QSPI模式
static uint8_t QSPI_FLASH_ReadSR(uint8_t regno);             //读取状态寄存器 
static void QSPI_FLASH_4ByteAddr_Enable(void);     //使能4字节地址模式
static void QSPI_FLASH_Write_SR(uint8_t regno,uint8_t sr);   //写状态寄存器
static void QSPI_FLASH_Write_Enable(void);  		//写使能 
static void QSPI_FLASH_Write_Disable(void);		//写保护
static void QSPI_FLASH_Wait_Busy(void);           	//等待空闲


static uint8_t QSPI_Receive(uint8_t* buf,uint32_t datalen);
static uint8_t QSPI_Transmit(uint8_t* buf,uint32_t datalen);
static void QSPI_Send_CMD(uint32_t instruction,uint32_t address,uint32_t dummyCycles,uint32_t instructionMode,uint32_t addressMode,uint32_t addressSize,uint32_t dataMode);

//4Kbytes为一个Sector
//16个扇区为1个Block
//W25Q256
//容量为32M字节,共有512个Block,8192个Sector

//初始化SPI FLASH的IO口
void QSPI_FLASH_Init(void)
{
    uint8_t temp;

    QSPI_FLASH_Qspi_Enable();			//使能QSPI模式
    QSPI_FLASH_TYPE=QSPI_FLASH_ReadID();	//读取FLASH ID.
    printf("ID:%x\r\n",QSPI_FLASH_TYPE);
    if(QSPI_FLASH_TYPE==W25Q128)        //SPI FLASH为W25Q256
    {
        temp=QSPI_FLASH_ReadSR(3);      //读取状态寄存器3，判断地址模式
        if((temp&0X01)==0)			//如果不是4字节地址模式,则进入4字节地址模式
        {
            QSPI_FLASH_Write_Enable();	//写使能
            QSPI_Send_CMD(W25X_Enable4ByteAddr,0,0,QSPI_INSTRUCTION_4_LINES,QSPI_ADDRESS_NONE,QSPI_ADDRESS_8_BITS,QSPI_DATA_NONE);//QPI,使能4字节地址指令,地址为0,无数据_8位地址_无地址_4线传输指令,无空周期,0个字节数据
        }
        QSPI_FLASH_Write_Enable();		//写使能
        QSPI_Send_CMD(W25X_SetReadParam,0,0,QSPI_INSTRUCTION_4_LINES,QSPI_ADDRESS_NONE,QSPI_ADDRESS_8_BITS,QSPI_DATA_4_LINES); 		//QPI,设置读参数指令,地址为0,4线传数据_8位地址_无地址_4线传输指令,无空周期,1个字节数据
        temp=3<<4;					//设置P4&P5=11,8个dummy clocks,104M
        QSPI_Transmit(&temp,1);		//发送1个字节
    }
}


//返回值如下:
//0XEF13,表示芯片型号为W25Q80
//0XEF14,表示芯片型号为W25Q16
//0XEF15,表示芯片型号为W25Q32
//0XEF16,表示芯片型号为W25Q64
//0XEF17,表示芯片型号为W25Q128
//0XEF18,表示芯片型号为W25Q256
uint16_t QSPI_FLASH_ReadID(void)
{
    uint8_t temp[2];
    uint16_t deviceid;
    if(QSPI_FLASH_QPI_MODE)
			QSPI_Send_CMD(W25X_ManufactDeviceID,0,0,QSPI_INSTRUCTION_4_LINES,QSPI_ADDRESS_4_LINES,QSPI_USE_ADRESS_BITS,QSPI_DATA_4_LINES);//QPI,读id,地址为0,4线传输数据_24位地址_4线传输地址_4线传输指令,无空周期,2个字节数据
    else 
			QSPI_Send_CMD(W25X_ManufactDeviceID,0,0,QSPI_INSTRUCTION_1_LINE,QSPI_ADDRESS_1_LINE,QSPI_USE_ADRESS_BITS,QSPI_DATA_1_LINE);			//SPI,读id,地址为0,单线传输数据_24位地址_单线传输地址_单线传输指令,无空周期,2个字节数据
    QSPI_Receive(temp,2);
    deviceid=(temp[0]<<8)|temp[1];
    return deviceid;
}

//读取SPI FLASH,仅支持QPI模式
//在指定地址开始读取指定长度的数据
//pBuffer:数据存储区
//ReadAddr:开始读取的地址(最大32bit)
//NumByteToRead:要读取的字节数(最大65535)
void QSPI_FLASH_Read(uint8_t* pBuffer,uint32_t ReadAddr,uint16_t NumByteToRead)
{
    QSPI_Send_CMD(W25X_FastReadData,ReadAddr,8,QSPI_INSTRUCTION_4_LINES,QSPI_ADDRESS_4_LINES,QSPI_USE_ADRESS_BITS,QSPI_DATA_4_LINES);	//QPI,快速读数据,地址为ReadAddr,4线传输数据_32位地址_4线传输地址_4线传输指令,8空周期,NumByteToRead个数据
    QSPI_Receive(pBuffer,NumByteToRead);
}

//SPI在一页(0~65535)内写入少于256个字节的数据
//在指定地址开始写入最大256字节的数据
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(最大32bit)
//NumByteToWrite:要写入的字节数(最大256),该数不应该超过该页的剩余字节数!!!
void QSPI_FLASH_Write_Page(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite)
{
    QSPI_FLASH_Write_Enable();					//写使能
    QSPI_Send_CMD(W25X_PageProgram,WriteAddr,0,QSPI_INSTRUCTION_4_LINES,QSPI_ADDRESS_4_LINES,QSPI_USE_ADRESS_BITS,QSPI_DATA_4_LINES);	//QPI,页写指令,地址为WriteAddr,4线传输数据_32位地址_4线传输地址_4线传输指令,无空周期,NumByteToWrite个数据
    QSPI_Transmit(pBuffer,NumByteToWrite);
    QSPI_FLASH_Wait_Busy();					   //等待写入结束
}

//无检验写SPI FLASH
//必须确保所写的地址范围内的数据全部为0XFF,否则在非0XFF处写入的数据将失败!
//具有自动换页功能
//在指定地址开始写入指定长度的数据,但是要确保地址不越界!
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(最大32bit)
//NumByteToWrite:要写入的字节数(最大65535)
//CHECK OK
void QSPI_FLASH_Write_NoCheck(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite)
{
    uint16_t pageremain;
    pageremain=256-WriteAddr%256; //单页剩余的字节数
    if(NumByteToWrite<=pageremain)pageremain=NumByteToWrite;//不大于256个字节
    while(1)
    {
        QSPI_FLASH_Write_Page(pBuffer,WriteAddr,pageremain);
        if(NumByteToWrite==pageremain)break;//写入结束了
        else //NumByteToWrite>pageremain
        {
            pBuffer+=pageremain;
            WriteAddr+=pageremain;

            NumByteToWrite-=pageremain;			  //减去已经写入了的字节数
            if(NumByteToWrite>256)pageremain=256; //一次可以写入256个字节
            else pageremain=NumByteToWrite; 	  //不够256个字节了
        }
    }
}

//写SPI FLASH
//在指定地址开始写入指定长度的数据
//该函数带擦除操作!
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(最大32bit)
//NumByteToWrite:要写入的字节数(最大65535)
uint8_t QSPI_FLASH_BUFFER[4096];
void QSPI_FLASH_Write(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite)
{
    uint32_t secpos;
    uint16_t secoff;
    uint16_t secremain;
    uint16_t i;
    uint8_t * QSPI_FLASH_BUF;
    QSPI_FLASH_BUF=QSPI_FLASH_BUFFER;
    secpos=WriteAddr/4096;//扇区地址
    secoff=WriteAddr%4096;//在扇区内的偏移
    secremain=4096-secoff;//扇区剩余空间大小
    //printf("ad:%X,nb:%X\r\n",WriteAddr,NumByteToWrite);//测试用
    if(NumByteToWrite<=secremain)secremain=NumByteToWrite;//不大于4096个字节
    while(1)
    {
        QSPI_FLASH_Read(QSPI_FLASH_BUF,secpos*4096,4096);//读出整个扇区的内容
        for(i=0; i<secremain; i++) //校验数据
        {
            if(QSPI_FLASH_BUF[secoff+i]!=0XFF)break;//需要擦除
        }
        if(i<secremain)//需要擦除
        {
            QSPI_FLASH_Erase_Sector(secpos);//擦除这个扇区
            for(i=0; i<secremain; i++)	 //复制
            {
                QSPI_FLASH_BUF[i+secoff]=pBuffer[i];
            }
            QSPI_FLASH_Write_NoCheck(QSPI_FLASH_BUF,secpos*4096,4096);//写入整个扇区

        } else QSPI_FLASH_Write_NoCheck(pBuffer,WriteAddr,secremain);//写已经擦除了的,直接写入扇区剩余区间.
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
void QSPI_FLASH_Erase_Chip(void)
{
    QSPI_FLASH_Write_Enable();					//SET WEL
    QSPI_FLASH_Wait_Busy();
    QSPI_Send_CMD(W25X_ChipErase,0,0,QSPI_INSTRUCTION_4_LINES,QSPI_ADDRESS_NONE,QSPI_ADDRESS_8_BITS,QSPI_DATA_NONE);//QPI,写全片擦除指令,地址为0,无数据_8位地址_无地址_4线传输指令,无空周期,0个字节数据
    QSPI_FLASH_Wait_Busy();						//等待芯片擦除结束
}

//擦除一个扇区
//Dst_Addr:扇区地址 根据实际容量设置
//擦除一个扇区的最少时间:150ms
void QSPI_FLASH_Erase_Sector(uint32_t Dst_Addr)
{

    Dst_Addr*=4096;
    QSPI_FLASH_Write_Enable();                  //SET WEL
    QSPI_FLASH_Wait_Busy();
    QSPI_Send_CMD(W25X_SectorErase,Dst_Addr,0,QSPI_INSTRUCTION_4_LINES,QSPI_ADDRESS_4_LINES,QSPI_USE_ADRESS_BITS,QSPI_DATA_NONE);//QPI,写扇区擦除指令,地址为0,无数据_32位地址_4线传输地址_4线传输指令,无空周期,0个字节数据
    QSPI_FLASH_Wait_Busy();   				    //等待擦除完成
}


/**
  * @brief  配置QSPI为内存映射模式
  * @retval QSPI内存状态
  */
uint32_t QSPI_EnableMemoryMappedMode()
{
 QSPI_CommandTypeDef      s_command;
  QSPI_MemoryMappedTypeDef s_mem_mapped_cfg;
 
  /* Configure the command for the read instruction */
	s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
  s_command.Instruction       = 0xeb;
  s_command.AddressMode       = QSPI_ADDRESS_4_LINES;
  s_command.AddressSize       = QSPI_USE_ADRESS_BITS;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_4_LINES;
	s_command.AlternateBytesSize = 0;
  s_command.DataMode          = QSPI_DATA_4_LINES;
  s_command.DummyCycles       = 6;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_HALF_CLK_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;


 
  /* Configure the memory mapped mode */
  s_mem_mapped_cfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;
  s_mem_mapped_cfg.TimeOutPeriod     = 0;
 


  if (HAL_QSPI_MemoryMapped(&hqspi, &s_command, &s_mem_mapped_cfg) != HAL_OK)
  {
    return 0;
  }
	
  return 1;
}



/// ------------API   END--------

//W25QXX进入QSPI模式
static void QSPI_FLASH_Qspi_Enable(void)
{
    uint8_t stareg2;
    stareg2=QSPI_FLASH_ReadSR(2);		//先读出状态寄存器2的原始值
    if((stareg2&0X02)==0)			//QE位未使能
    {
        QSPI_FLASH_Write_Enable();		//写使能
        stareg2|=1<<1;				//使能QE位
        QSPI_FLASH_Write_SR(2,stareg2);	//写状态寄存器2
    }
    QSPI_Send_CMD(W25X_EnterQPIMode,0,0,QSPI_INSTRUCTION_1_LINE,QSPI_ADDRESS_NONE,QSPI_ADDRESS_8_BITS,QSPI_DATA_NONE);//写command指令,地址为0,无数据_8位地址_无地址_单线传输指令,无空周期,0个字节数据
    QSPI_FLASH_QPI_MODE=1;				//标记QSPI模式
}

//W25QXX退出QSPI模式
static void QSPI_FLASH_Qspi_Disable(void)
{
    QSPI_Send_CMD(W25X_ExitQPIMode,0,0,QSPI_INSTRUCTION_4_LINES,QSPI_ADDRESS_NONE,QSPI_ADDRESS_8_BITS,QSPI_DATA_NONE);//写command指令,地址为0,无数据_8位地址_无地址_4线传输指令,无空周期,0个字节数据
    QSPI_FLASH_QPI_MODE=0;				//标记SPI模式
}

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
static uint8_t QSPI_FLASH_ReadSR(uint8_t regno)
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
    if(QSPI_FLASH_QPI_MODE)
			QSPI_Send_CMD(command,0,0,QSPI_INSTRUCTION_4_LINES,QSPI_ADDRESS_NONE,QSPI_ADDRESS_8_BITS,QSPI_DATA_4_LINES);	//QPI,写command指令,地址为0,4线传数据_8位地址_无地址_4线传输指令,无空周期,1个字节数据
    else 
			QSPI_Send_CMD(command,0,0,QSPI_INSTRUCTION_1_LINE,QSPI_ADDRESS_NONE,QSPI_ADDRESS_8_BITS,QSPI_DATA_1_LINE);				//SPI,写command指令,地址为0,单线传数据_8位地址_无地址_单线传输指令,无空周期,1个字节数据
    QSPI_Receive(&byte,1);
    return byte;
}

//写W25QXX状态寄存器
static void QSPI_FLASH_Write_SR(uint8_t regno,uint8_t sr)
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
    if(QSPI_FLASH_QPI_MODE)
			QSPI_Send_CMD(command,0,0,QSPI_INSTRUCTION_4_LINES,QSPI_ADDRESS_NONE,QSPI_ADDRESS_8_BITS,QSPI_DATA_4_LINES);	//QPI,写command指令,地址为0,4线传数据_8位地址_无地址_4线传输指令,无空周期,1个字节数据
    else 
			QSPI_Send_CMD(command,0,0, QSPI_INSTRUCTION_1_LINE,QSPI_ADDRESS_NONE,QSPI_ADDRESS_8_BITS,QSPI_DATA_1_LINE);				//SPI,写command指令,地址为0,单线传数据_8位地址_无地址_单线传输指令,无空周期,1个字节数据
    QSPI_Transmit(&sr,1);
}

//W25QXX写使能
//将S1寄存器的WEL置位
static void QSPI_FLASH_Write_Enable(void)
{
    if(QSPI_FLASH_QPI_MODE)
			QSPI_Send_CMD(W25X_WriteEnable,0,0,QSPI_INSTRUCTION_4_LINES,QSPI_ADDRESS_NONE,QSPI_ADDRESS_8_BITS,QSPI_DATA_NONE);	//QPI,写使能指令,地址为0,无数据_8位地址_无地址_4线传输指令,无空周期,0个字节数据
    else 
			QSPI_Send_CMD(W25X_WriteEnable,0,0,QSPI_INSTRUCTION_1_LINE,QSPI_ADDRESS_NONE,QSPI_ADDRESS_8_BITS,QSPI_DATA_NONE);				//SPI,写使能指令,地址为0,无数据_8位地址_无地址_单线传输指令,无空周期,0个字节数据
}

//W25QXX写禁止
//将WEL清零
static void QSPI_FLASH_Write_Disable(void)
{
    if(QSPI_FLASH_QPI_MODE)
			QSPI_Send_CMD(W25X_WriteDisable,0,0,QSPI_INSTRUCTION_4_LINES,QSPI_ADDRESS_NONE,QSPI_ADDRESS_8_BITS,QSPI_DATA_NONE);//QPI,写禁止指令,地址为0,无数据_8位地址_无地址_4线传输指令,无空周期,0个字节数据
    else 
			QSPI_Send_CMD(W25X_WriteDisable,0,0,QSPI_INSTRUCTION_1_LINE,QSPI_ADDRESS_NONE,QSPI_ADDRESS_8_BITS,QSPI_DATA_NONE);				//SPI,写禁止指令,地址为0,无数据_8位地址_无地址_单线传输指令,无空周期,0个字节数据
}

//等待空闲
static void QSPI_FLASH_Wait_Busy(void)
{
    while((QSPI_FLASH_ReadSR(1)&0x01)==0x01);   // 等待BUSY位清空
}



//QSPI发送命令
//instruction:要发送的指令
//address:发送到的目的地址
//dummyCycles:空指令周期数
//	instructionMode:指令模式;QSPI_INSTRUCTION_NONE,QSPI_INSTRUCTION_1_LINE,QSPI_INSTRUCTION_2_LINE,QSPI_INSTRUCTION_4_LINE
//	addressMode:地址模式; QSPI_ADDRESS_NONE,QSPI_ADDRESS_1_LINE,QSPI_ADDRESS_2_LINE,QSPI_ADDRESS_4_LINE
//	addressSize:地址长度;QSPI_ADDRESS_8_BITS,QSPI_ADDRESS_16_BITS,QSPI_USE_ADRESS_BITS,QSPI_USE_ADRESS_BITS
//	dataMode:数据模式; QSPI_DATA_NONE,QSPI_DATA_1_LINE,QSPI_DATA_2_LINE,QSPI_DATA_4_LINE

static void QSPI_Send_CMD(uint32_t instruction,uint32_t address,uint32_t dummyCycles,uint32_t instructionMode,uint32_t addressMode,uint32_t addressSize,uint32_t dataMode)
{
    QSPI_CommandTypeDef Cmdhandler;

    Cmdhandler.Instruction=instruction;                 	//指令
    Cmdhandler.Address=address;                            	//地址
    Cmdhandler.DummyCycles=dummyCycles;                     //设置空指令周期数
    Cmdhandler.InstructionMode=instructionMode;				//指令模式
    Cmdhandler.AddressMode=addressMode;   					//地址模式
    Cmdhandler.AddressSize=addressSize;   					//地址长度
    Cmdhandler.DataMode=dataMode;             				//数据模式
    Cmdhandler.SIOOMode=QSPI_SIOO_INST_EVERY_CMD;       	//每次都发送指令
    Cmdhandler.AlternateByteMode=QSPI_ALTERNATE_BYTES_NONE; //无交替字节
    Cmdhandler.DdrMode=QSPI_DDR_MODE_DISABLE;           	//关闭DDR模式
    Cmdhandler.DdrHoldHalfCycle=QSPI_DDR_HHC_ANALOG_DELAY;

    HAL_QSPI_Command(&hqspi,&Cmdhandler,5000);
}

//QSPI接收指定长度的数据
//buf:接收数据缓冲区首地址
//datalen:要传输的数据长度
//返回值:0,正常
//    其他,错误代码
static uint8_t QSPI_Receive(uint8_t* buf,uint32_t datalen)
{
    hqspi.Instance->DLR=datalen-1;                           //配置数据长度
    if(HAL_QSPI_Receive(&hqspi,buf,5000)==HAL_OK) return 0;  //接收数据
    else return 1;
}

//QSPI发送指定长度的数据
//buf:发送数据缓冲区首地址
//datalen:要传输的数据长度
//返回值:0,正常
//    其他,错误代码
static uint8_t QSPI_Transmit(uint8_t* buf,uint32_t datalen)
{
    hqspi.Instance->DLR=datalen-1;                            //配置数据长度
    if(HAL_QSPI_Transmit(&hqspi,buf,5000)==HAL_OK) return 0;  //发送数据
    else return 1;
}

