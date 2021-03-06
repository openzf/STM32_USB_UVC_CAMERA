#include "driver_LCD_RGB.h"
#include "ltdc.h"

//定义最大屏分辨率时,LCD所需的帧缓存数组大小
uint16_t g_ltdc_framebuf[1280][800] __attribute__((at(RGB_LCD_BASE_ADDR)));

// 4.3寸
static LCD_RGB_Config lcd_rgb_4_3_inch_800x480 =
{
    0x4384,
    800,
    480,
    88,
    40,
    48,
    32,
    13,
    5
};
// 4.3寸
static LCD_RGB_Config lcd_rgb_4_3_inch_480x272 =
{
    0x4342,
    480,
    272,
    1,
    1,
    40,
    8,
    5,
    8
};
// 当前使用的配置
static LCD_RGB_Config g_lcd_rgb_config;
static LCDTypedef *g_mlcd;

static void lcd_config_sequence(LCD_RGB_Config m_lcd_rgb_config);
static void lcd_config_layer(LCD_RGB_Config m_lcd_rgb_config,uint32_t layer_addr,uint8_t layer);


static void LTDC_Draw_Point(uint16_t x,uint16_t y,uint32_t color);
static uint32_t LTDC_Read_Point(uint16_t x,uint16_t y);
static void LTDC_Fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint32_t color);
static void LTDC_Color_Fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t *color);


// 初始化
void driver_LCD_RGB_init(LCDTypedef *lcd)
{
    // 配置
    g_lcd_rgb_config = lcd_rgb_4_3_inch_800x480;
    // 时序
    lcd_config_sequence( g_lcd_rgb_config);
    // 层
    lcd_config_layer( g_lcd_rgb_config,RGB_LCD_BASE_ADDR,0);
    lcd_config_layer( g_lcd_rgb_config,RGB_LCD_BASE_ADDR,1);

    g_mlcd = lcd;
    lcd->lcd_type = LCD_TYPE_RGB;
    lcd->width = g_lcd_rgb_config.pix_width;
    lcd->height =g_lcd_rgb_config.pix_height;

    lcd->lcd_rgb.draw_point = LTDC_Draw_Point;
    lcd->lcd_rgb.read_point = LTDC_Read_Point;
    lcd->lcd_rgb.fill = LTDC_Fill;
    lcd->lcd_rgb.color_fill = LTDC_Color_Fill;

    //使能DMA2D时钟
    __HAL_RCC_DMA2D_CLK_ENABLE();
    __HAL_LTDC_LAYER_ENABLE(&hltdc,0);
    __HAL_LTDC_LAYER_DISABLE(&hltdc,1);
    __HAL_LTDC_RELOAD_CONFIG(&hltdc);
}


// 时序配置
static void lcd_config_sequence(LCD_RGB_Config m_lcd_rgb_config)
{
    hltdc.Instance=LTDC;
    hltdc.Init.HSPolarity=LTDC_HSPOLARITY_AL;         //水平同步极性
    hltdc.Init.VSPolarity=LTDC_VSPOLARITY_AL;         //垂直同步极性
    hltdc.Init.DEPolarity=LTDC_DEPOLARITY_AL;         //数据使能极性
    hltdc.Init.PCPolarity=LTDC_PCPOLARITY_IPC;        //像素时钟极性
    hltdc.Init.HorizontalSync=m_lcd_rgb_config.hsw-1;          //水平同步宽度
    hltdc.Init.VerticalSync=m_lcd_rgb_config.vsw-1;            //垂直同步宽度
    hltdc.Init.AccumulatedHBP=m_lcd_rgb_config.hsw+m_lcd_rgb_config.hbp-1; //水平同步后沿宽度
    hltdc.Init.AccumulatedVBP=m_lcd_rgb_config.vsw+m_lcd_rgb_config.vbp-1; //垂直同步后沿高度
    hltdc.Init.AccumulatedActiveW=m_lcd_rgb_config.hsw+m_lcd_rgb_config.hbp+m_lcd_rgb_config.pix_width-1;//有效宽度
    hltdc.Init.AccumulatedActiveH=m_lcd_rgb_config.vsw+m_lcd_rgb_config.vbp+m_lcd_rgb_config.pix_height-1;//有效高度
    hltdc.Init.TotalWidth=m_lcd_rgb_config.hsw+m_lcd_rgb_config.hbp+m_lcd_rgb_config.pix_width+m_lcd_rgb_config.hfp-1;   //总宽度
    hltdc.Init.TotalHeigh=m_lcd_rgb_config.vsw+m_lcd_rgb_config.vbp+m_lcd_rgb_config.pix_height+m_lcd_rgb_config.vfp-1;  //总高度
    hltdc.Init.Backcolor.Red=0;           //屏幕背景层红色部分
    hltdc.Init.Backcolor.Green=0;         //屏幕背景层绿色部分
    hltdc.Init.Backcolor.Blue=0;          //屏幕背景色蓝色部分
    if (HAL_LTDC_Init(&hltdc) != HAL_OK)
    {
        Error_Handler();
    }
}

// 缓冲层配置
static void lcd_config_layer(LCD_RGB_Config m_lcd_rgb_config,uint32_t layer_addr,uint8_t layer)
{
    LTDC_LayerCfgTypeDef pLayerCfg = {0};
    LTDC_LayerCfgTypeDef pLayerCfg1 = {0};


    pLayerCfg.WindowX0 = 0;
    pLayerCfg.WindowX1 = m_lcd_rgb_config.pix_width;
    pLayerCfg.WindowY0 = 0;
    pLayerCfg.WindowY1 = m_lcd_rgb_config.pix_height;
    pLayerCfg.PixelFormat = LTDC_PIXEL_FORMAT_RGB565;
    pLayerCfg.Alpha = 255;
    pLayerCfg.Alpha0 = 0;
    pLayerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA;
    pLayerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_PAxCA;
    pLayerCfg.FBStartAdress =layer_addr;
    pLayerCfg.ImageWidth = m_lcd_rgb_config.pix_width;
    pLayerCfg.ImageHeight = m_lcd_rgb_config.pix_height;
    pLayerCfg.Backcolor.Blue = 0;
    pLayerCfg.Backcolor.Green = 0;
    pLayerCfg.Backcolor.Red = 0;
    if (HAL_LTDC_ConfigLayer(&hltdc, &pLayerCfg, layer) != HAL_OK)
    {
        Error_Handler();
    }



}


//画点函数
static void LTDC_Draw_Point(uint16_t x,uint16_t y,uint32_t color)
{
    //坐标系转换
    if(g_mlcd->dir== LCD_DIR_HORIZONTAL)	//横屏
    {
        *(uint16_t*)((uint32_t)RGB_LCD_BASE_ADDR+RGB_LCD_PIXELS_BYTE*(g_lcd_rgb_config.pix_width*y+x))=color;
    } else			//竖屏
    {
        *(uint16_t*)((uint32_t)RGB_LCD_BASE_ADDR+RGB_LCD_PIXELS_BYTE*(g_lcd_rgb_config.pix_width*(g_lcd_rgb_config.pix_height-x-1)+y))=color;
    }
}
//读点函数
static uint32_t LTDC_Read_Point(uint16_t x,uint16_t y)
{
    //坐标系转换
    if(g_mlcd->dir== LCD_DIR_HORIZONTAL)	//横屏
    {
        return *(uint16_t*)((uint32_t)RGB_LCD_BASE_ADDR+RGB_LCD_PIXELS_BYTE*(g_lcd_rgb_config.pix_width*y+x));
    } else			//竖屏
    {
        return *(uint16_t*)((uint32_t)RGB_LCD_BASE_ADDR+RGB_LCD_PIXELS_BYTE*(g_lcd_rgb_config.pix_width*(g_lcd_rgb_config.pix_height-x-1)+y));
    }
}





//LTDC填充矩形,DMA2D填充
//(sx,sy),(ex,ey):填充矩形对角坐标,区域大小为:(ex-sx+1)*(ey-sy+1)
//color:要填充的颜色
//有时候需要频繁的调用填充函数，所以为了速度，填充函数采用寄存器版本，
//不过下面有对应的库函数版本的代码。
static void LTDC_Fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint32_t color)
{
    uint32_t psx,psy,pex,pey;	//以LCD面板为基准的坐标系,不随横竖屏变化而变化
    uint32_t timeout=0;
    uint16_t offline;
    uint32_t addr;

    //坐标系转换
    if(g_mlcd->dir== LCD_DIR_HORIZONTAL)	//横屏
    {
        psx=sx;
        psy=sy;
        pex=ex;
        pey=ey;
    } else			//竖屏
    {
        psx=sy;
        psy=g_lcd_rgb_config.pix_height-ex-1;
        pex=ey;
        pey=g_lcd_rgb_config.pix_height-sx-1;
    }
    offline=g_lcd_rgb_config.pix_width-(pex-psx+1);
    addr=(RGB_LCD_BASE_ADDR+RGB_LCD_PIXELS_BYTE*(g_lcd_rgb_config.pix_width*psy+psx));

    __HAL_RCC_DMA2D_CLK_ENABLE();	//使能DM2D时钟
    //DMA2D->CR&=~(DMA2D_CR_START);	//先停止DMA2D


    DMA2D->CR=DMA2D_R2M;			//寄存器到存储器模式
    // 设置 DMA2D 采用寄存器往存储器传输数据模式，即 DMA2D 将 OCOLR 寄存器设置颜色值填充到存储器里面。
    DMA2D->OPFCCR=LTDC_PIXEL_FORMAT_RGB565;	//设置颜色格式

    DMA2D->OOR=offline;				//设置行偏移

    DMA2D->OMAR=addr;				//输出存储器地址
    DMA2D->NLR=(pey-psy+1)|((pex-psx+1)<<16);	//设定行数寄存器
    DMA2D->OCOLR=color;						//设定输出颜色寄存器

    DMA2D->CR|=DMA2D_CR_START;				//启动DMA2D
    while((DMA2D->ISR&(DMA2D_FLAG_TC))==0)	//等待传输完成
    {

    }
    DMA2D->IFCR|=DMA2D_FLAG_TC;		//清除传输完成标志
}






//矩形彩色填充函数
static void LTDC_Color_Fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t *color)
{
    uint32_t psx,psy,pex,pey;	//以LCD面板为基准的坐标系,不随横竖屏变化而变化
    uint32_t timeout=0;
    uint16_t offline;
    uint32_t addr;

    //坐标系转换
    if(g_mlcd->dir== LCD_DIR_HORIZONTAL)	//横屏
    {
        psx=sx;
        psy=sy;
        pex=ex;
        pey=ey;
    } else			//竖屏
    {
        psx=sy;
        psy=g_lcd_rgb_config.pix_height-ex-1;
        pex=ey;
        pey=g_lcd_rgb_config.pix_height-sx-1;
    }

    offline=g_lcd_rgb_config.pix_width-(pex-psx+1);
    addr=(RGB_LCD_BASE_ADDR+RGB_LCD_PIXELS_BYTE*(g_lcd_rgb_config.pix_width*psy+psx));

    __HAL_RCC_DMA2D_CLK_ENABLE();	//使能DM2D时钟
    //DMA2D->CR&=~(DMA2D_CR_START);	//先停止DMA2D

    DMA2D->CR=DMA2D_M2M;			//存储器到存储器模式


    DMA2D->FGMAR=(uint32_t)color;		//源地址
    DMA2D->OMAR=addr;				//输出存储器地址

    DMA2D->FGOR=0;					//前景层行偏移为0
    DMA2D->OOR=offline;				//设置行偏移

    DMA2D->FGPFCCR=0X02;	//设置颜色格式
    DMA2D->OPFCCR  = LTDC_PIXEL_FORMAT_RGB565;

    DMA2D->NLR=(pey-psy+1)|((pex-psx+1)<<16);	//设定行数寄存器

    DMA2D->CR|=DMA2D_CR_START;					//启动DMA2D
    while((DMA2D->ISR&(DMA2D_FLAG_TC))==0)		//等待传输完成
    {

    }
    DMA2D->IFCR|=DMA2D_FLAG_TC;				//清除传输完成标志
}

