#include "siic.h"
#include "delay.h"
#include "touch_conf.h"


// 平台文件
#include "stm32h7xx_hal.h"

#define SIIC_SDA_PORT      GPIOI
#define SIIC_SDA_PIN  	   GPIO_PIN_3

#define SIIC_SCL_PORT      GPIOH
#define SIIC_SCL_PIN  	   GPIO_PIN_6

#define SIIC_SDA_H  HAL_GPIO_WritePin(SIIC_SDA_PORT,SIIC_SDA_PIN,GPIO_PIN_SET)
#define SIIC_SDA_L  HAL_GPIO_WritePin(SIIC_SDA_PORT,SIIC_SDA_PIN,GPIO_PIN_RESET)

#define SIIC_SCL_H  HAL_GPIO_WritePin(SIIC_SCL_PORT,SIIC_SCL_PIN,GPIO_PIN_SET)
#define SIIC_SCL_L  HAL_GPIO_WritePin(SIIC_SCL_PORT,SIIC_SCL_PIN,GPIO_PIN_RESET)

#define SIIC_SPEED_DELAY()        delay_us(2)
#define SIIC_SPEED_START_DELAY()  delay_us(30)


void SIIC_SDA_OUT()
{
    GPIO_InitTypeDef GPIO_Initure;
    //SDA
    GPIO_Initure.Pin=SIIC_SDA_PIN;
    GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //推挽输出
    GPIO_Initure.Pull=GPIO_PULLUP;          //上拉
    GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //高速
    HAL_GPIO_Init(SIIC_SDA_PORT,&GPIO_Initure);     //初始化
}
void SIIC_SIIC_SDA_IN()
{
    GPIO_InitTypeDef GPIO_Initure;
    //SDA
    GPIO_Initure.Pin=SIIC_SDA_PIN;
    GPIO_Initure.Mode=GPIO_MODE_INPUT;  //推挽输出
    GPIO_Initure.Pull=GPIO_PULLUP;          //上拉
    GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //高速
    HAL_GPIO_Init(SIIC_SDA_PORT,&GPIO_Initure);     //初始化
}

uint8_t SIIC_SIIC_SDA_Read(void)
{

    return HAL_GPIO_ReadPin(SIIC_SDA_PORT,SIIC_SDA_PIN);
}

void SIIC_SIIC_SDA_Write(uint8_t state)
{
    HAL_GPIO_WritePin(SIIC_SDA_PORT,SIIC_SDA_PIN,state);
}



//SIIC初始化
void SIIC_Init(void)
{
    GPIO_InitTypeDef GPIO_Initure;

    __HAL_RCC_GPIOH_CLK_ENABLE();			//开启GPIOH时钟
    __HAL_RCC_GPIOI_CLK_ENABLE();			//开启GPIOI时钟

    // SCL
    GPIO_Initure.Pin=SIIC_SCL_PIN;
    GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //推挽输出
    GPIO_Initure.Pull=GPIO_PULLUP;          //上拉
    GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //高速
    HAL_GPIO_Init(SIIC_SCL_PORT,&GPIO_Initure);     //初始化

    //SDA
    GPIO_Initure.Pin=SIIC_SDA_PIN;
    HAL_GPIO_Init(SIIC_SDA_PORT,&GPIO_Initure);     //初始化

    SIIC_SDA_H;
    SIIC_SCL_H;
}

//产生SIIC起始信号
void SIIC_Start(void)
{
    SIIC_SDA_OUT();     //sda线输出
    SIIC_SDA_H;
    SIIC_SCL_H;
    SIIC_SPEED_START_DELAY();
    SIIC_SDA_L;//START:when CLK is high,DATA change form high to low
    SIIC_SPEED_DELAY();
    SIIC_SCL_L;//钳住I2C总线，准备发送或接收数据
}
//产生SIIC停止信号
void SIIC_Stop(void)
{
    SIIC_SDA_OUT();//sda线输出
    SIIC_SCL_L;
    SIIC_SDA_L;//STOP:when CLK is high DATA change form low to high
    SIIC_SPEED_START_DELAY();
    SIIC_SCL_H;
    SIIC_SPEED_DELAY();
    SIIC_SDA_H;//发送I2C总线结束信号

}
//等待应答信号到来
//返回值：1，接收应答失败
//        0，接收应答成功
uint8_t SIIC_Wait_Ack(void)
{
    uint8_t ucErrTime=0;
    SIIC_SIIC_SDA_IN();
    SIIC_SDA_H;
    SIIC_SCL_H;
    SIIC_SPEED_DELAY();
    while(SIIC_SIIC_SDA_Read())
    {
        ucErrTime++;
        if(ucErrTime>250)
        {
            SIIC_Stop();
            return 1;
        }
        SIIC_SPEED_DELAY();
    }
    SIIC_SCL_L;//时钟输出0
    return 0;
}
//产生ACK应答
void SIIC_Ack(void)
{
    SIIC_SCL_L;
    SIIC_SDA_OUT();
    SIIC_SDA_L;
    SIIC_SPEED_DELAY();
    SIIC_SCL_H;
    SIIC_SPEED_DELAY();
    SIIC_SCL_L;
}
//不产生ACK应答
void SIIC_NAck(void)
{
    SIIC_SCL_L;
    SIIC_SDA_OUT();
    SIIC_SDA_H;
    SIIC_SPEED_DELAY();
    SIIC_SCL_H;
    SIIC_SPEED_DELAY();
    SIIC_SCL_L;
}
//SIIC发送一个字节
//返回从机有无应答
//1，有应答
//0，无应答
void SIIC_Send_Byte(uint8_t txd)
{
    uint8_t t;
    SIIC_SDA_OUT();
    SIIC_SCL_L;//拉低时钟开始数据传输
    for(t=0; t<8; t++)
    {

        SIIC_SIIC_SDA_Write(((txd&0x80)>>7));
        txd<<=1;
        SIIC_SPEED_DELAY();   //对TEA5767这三个延时都是必须的
        SIIC_SCL_H;
        SIIC_SPEED_DELAY();
        SIIC_SCL_L;
        SIIC_SPEED_DELAY();
    }
}
//读1个字节，ack=1时，发送ACK，ack=0，发送nACK
uint8_t SIIC_Read_Byte(unsigned char ack)
{
    unsigned char i,receive=0;
    SIIC_SIIC_SDA_IN();//SDA设置为输入
    SIIC_SPEED_START_DELAY();
    for(i=0; i<8; i++ )
    {
        SIIC_SCL_L;
        SIIC_SPEED_DELAY();
        SIIC_SCL_H;
        receive<<=1;
        if(SIIC_SIIC_SDA_Read())
        {

            receive++;
        }
        SIIC_SPEED_DELAY();
    }
    if (!ack)
        SIIC_NAck();//发送nACK
    else
        SIIC_Ack(); //发送ACK
    return receive;
}
