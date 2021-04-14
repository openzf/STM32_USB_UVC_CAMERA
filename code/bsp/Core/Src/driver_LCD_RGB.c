#include "driver_LCD_RGB.h"
#include "ltdc.h"

//����������ֱ���ʱ,LCD�����֡���������С
uint16_t g_ltdc_framebuf[1280][800] __attribute__((at(RGB_LCD_BASE_ADDR)));

// 4.3��
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
// 4.3��
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
// ��ǰʹ�õ�����
static LCD_RGB_Config g_lcd_rgb_config;
static LCDTypedef *g_mlcd;

static void lcd_config_sequence(LCD_RGB_Config m_lcd_rgb_config);
static void lcd_config_layer(LCD_RGB_Config m_lcd_rgb_config,uint32_t layer_addr,uint8_t layer);


static void LTDC_Draw_Point(uint16_t x,uint16_t y,uint32_t color);
static uint32_t LTDC_Read_Point(uint16_t x,uint16_t y);
static void LTDC_Fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint32_t color);
static void LTDC_Color_Fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t *color);


// ��ʼ��
void driver_LCD_RGB_init(LCDTypedef *lcd)
{
    // ����
    g_lcd_rgb_config = lcd_rgb_4_3_inch_800x480;
    // ʱ��
    lcd_config_sequence( g_lcd_rgb_config);
    // ��
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

    //ʹ��DMA2Dʱ��
    __HAL_RCC_DMA2D_CLK_ENABLE();
    __HAL_LTDC_LAYER_ENABLE(&hltdc,0);
    __HAL_LTDC_LAYER_DISABLE(&hltdc,1);
    __HAL_LTDC_RELOAD_CONFIG(&hltdc);
}


// ʱ������
static void lcd_config_sequence(LCD_RGB_Config m_lcd_rgb_config)
{
    hltdc.Instance=LTDC;
    hltdc.Init.HSPolarity=LTDC_HSPOLARITY_AL;         //ˮƽͬ������
    hltdc.Init.VSPolarity=LTDC_VSPOLARITY_AL;         //��ֱͬ������
    hltdc.Init.DEPolarity=LTDC_DEPOLARITY_AL;         //����ʹ�ܼ���
    hltdc.Init.PCPolarity=LTDC_PCPOLARITY_IPC;        //����ʱ�Ӽ���
    hltdc.Init.HorizontalSync=m_lcd_rgb_config.hsw-1;          //ˮƽͬ������
    hltdc.Init.VerticalSync=m_lcd_rgb_config.vsw-1;            //��ֱͬ������
    hltdc.Init.AccumulatedHBP=m_lcd_rgb_config.hsw+m_lcd_rgb_config.hbp-1; //ˮƽͬ�����ؿ���
    hltdc.Init.AccumulatedVBP=m_lcd_rgb_config.vsw+m_lcd_rgb_config.vbp-1; //��ֱͬ�����ظ߶�
    hltdc.Init.AccumulatedActiveW=m_lcd_rgb_config.hsw+m_lcd_rgb_config.hbp+m_lcd_rgb_config.pix_width-1;//��Ч����
    hltdc.Init.AccumulatedActiveH=m_lcd_rgb_config.vsw+m_lcd_rgb_config.vbp+m_lcd_rgb_config.pix_height-1;//��Ч�߶�
    hltdc.Init.TotalWidth=m_lcd_rgb_config.hsw+m_lcd_rgb_config.hbp+m_lcd_rgb_config.pix_width+m_lcd_rgb_config.hfp-1;   //�ܿ���
    hltdc.Init.TotalHeigh=m_lcd_rgb_config.vsw+m_lcd_rgb_config.vbp+m_lcd_rgb_config.pix_height+m_lcd_rgb_config.vfp-1;  //�ܸ߶�
    hltdc.Init.Backcolor.Red=0;           //��Ļ�������ɫ����
    hltdc.Init.Backcolor.Green=0;         //��Ļ��������ɫ����
    hltdc.Init.Backcolor.Blue=0;          //��Ļ����ɫ��ɫ����
    if (HAL_LTDC_Init(&hltdc) != HAL_OK)
    {
        Error_Handler();
    }
}

// ���������
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


//���㺯��
static void LTDC_Draw_Point(uint16_t x,uint16_t y,uint32_t color)
{
    //����ϵת��
    if(g_mlcd->dir== LCD_DIR_HORIZONTAL)	//����
    {
        *(uint16_t*)((uint32_t)RGB_LCD_BASE_ADDR+RGB_LCD_PIXELS_BYTE*(g_lcd_rgb_config.pix_width*y+x))=color;
    } else			//����
    {
        *(uint16_t*)((uint32_t)RGB_LCD_BASE_ADDR+RGB_LCD_PIXELS_BYTE*(g_lcd_rgb_config.pix_width*(g_lcd_rgb_config.pix_height-x-1)+y))=color;
    }
}
//���㺯��
static uint32_t LTDC_Read_Point(uint16_t x,uint16_t y)
{
    //����ϵת��
    if(g_mlcd->dir== LCD_DIR_HORIZONTAL)	//����
    {
        return *(uint16_t*)((uint32_t)RGB_LCD_BASE_ADDR+RGB_LCD_PIXELS_BYTE*(g_lcd_rgb_config.pix_width*y+x));
    } else			//����
    {
        return *(uint16_t*)((uint32_t)RGB_LCD_BASE_ADDR+RGB_LCD_PIXELS_BYTE*(g_lcd_rgb_config.pix_width*(g_lcd_rgb_config.pix_height-x-1)+y));
    }
}





//LTDC������,DMA2D���
//(sx,sy),(ex,ey):�����ζԽ�����,�����СΪ:(ex-sx+1)*(ey-sy+1)
//color:Ҫ������ɫ
//��ʱ����ҪƵ���ĵ�����亯��������Ϊ���ٶȣ���亯�����üĴ����汾��
//���������ж�Ӧ�Ŀ⺯���汾�Ĵ��롣
static void LTDC_Fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint32_t color)
{
    uint32_t psx,psy,pex,pey;	//��LCD���Ϊ��׼������ϵ,����������仯���仯
    uint32_t timeout=0;
    uint16_t offline;
    uint32_t addr;

    //����ϵת��
    if(g_mlcd->dir== LCD_DIR_HORIZONTAL)	//����
    {
        psx=sx;
        psy=sy;
        pex=ex;
        pey=ey;
    } else			//����
    {
        psx=sy;
        psy=g_lcd_rgb_config.pix_height-ex-1;
        pex=ey;
        pey=g_lcd_rgb_config.pix_height-sx-1;
    }
    offline=g_lcd_rgb_config.pix_width-(pex-psx+1);
    addr=(RGB_LCD_BASE_ADDR+RGB_LCD_PIXELS_BYTE*(g_lcd_rgb_config.pix_width*psy+psx));

    __HAL_RCC_DMA2D_CLK_ENABLE();	//ʹ��DM2Dʱ��
    //DMA2D->CR&=~(DMA2D_CR_START);	//��ֹͣDMA2D


    DMA2D->CR=DMA2D_R2M;			//�Ĵ������洢��ģʽ
    // ���� DMA2D ���üĴ������洢����������ģʽ���� DMA2D �� OCOLR �Ĵ���������ɫֵ��䵽�洢�����档
    DMA2D->OPFCCR=LTDC_PIXEL_FORMAT_RGB565;	//������ɫ��ʽ

    DMA2D->OOR=offline;				//������ƫ��

    DMA2D->OMAR=addr;				//����洢����ַ
    DMA2D->NLR=(pey-psy+1)|((pex-psx+1)<<16);	//�趨�����Ĵ���
    DMA2D->OCOLR=color;						//�趨�����ɫ�Ĵ���

    DMA2D->CR|=DMA2D_CR_START;				//����DMA2D
    while((DMA2D->ISR&(DMA2D_FLAG_TC))==0)	//�ȴ��������
    {

    }
    DMA2D->IFCR|=DMA2D_FLAG_TC;		//���������ɱ�־
}






//���β�ɫ��亯��
static void LTDC_Color_Fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t *color)
{
    uint32_t psx,psy,pex,pey;	//��LCD���Ϊ��׼������ϵ,����������仯���仯
    uint32_t timeout=0;
    uint16_t offline;
    uint32_t addr;

    //����ϵת��
    if(g_mlcd->dir== LCD_DIR_HORIZONTAL)	//����
    {
        psx=sx;
        psy=sy;
        pex=ex;
        pey=ey;
    } else			//����
    {
        psx=sy;
        psy=g_lcd_rgb_config.pix_height-ex-1;
        pex=ey;
        pey=g_lcd_rgb_config.pix_height-sx-1;
    }

    offline=g_lcd_rgb_config.pix_width-(pex-psx+1);
    addr=(RGB_LCD_BASE_ADDR+RGB_LCD_PIXELS_BYTE*(g_lcd_rgb_config.pix_width*psy+psx));

    __HAL_RCC_DMA2D_CLK_ENABLE();	//ʹ��DM2Dʱ��
    //DMA2D->CR&=~(DMA2D_CR_START);	//��ֹͣDMA2D

    DMA2D->CR=DMA2D_M2M;			//�洢�����洢��ģʽ


    DMA2D->FGMAR=(uint32_t)color;		//Դ��ַ
    DMA2D->OMAR=addr;				//����洢����ַ

    DMA2D->FGOR=0;					//ǰ������ƫ��Ϊ0
    DMA2D->OOR=offline;				//������ƫ��

    DMA2D->FGPFCCR=0X02;	//������ɫ��ʽ
    DMA2D->OPFCCR  = LTDC_PIXEL_FORMAT_RGB565;

    DMA2D->NLR=(pey-psy+1)|((pex-psx+1)<<16);	//�趨�����Ĵ���

    DMA2D->CR|=DMA2D_CR_START;					//����DMA2D
    while((DMA2D->ISR&(DMA2D_FLAG_TC))==0)		//�ȴ��������
    {

    }
    DMA2D->IFCR|=DMA2D_FLAG_TC;				//���������ɱ�־
}
