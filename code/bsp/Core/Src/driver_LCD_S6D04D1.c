#include "driver_LCD_S6D04D1.h"

#include "delay.h"

/***  生长方向
//------>  Y   480
|
|
|
|
X

320

***/

#define LCD_PIX_WIDTH  400
#define LCD_PIX_HEIGHT 240


#define LCD_SET_X_CMD  0X2A
#define LCD_SET_Y_CMD  0X2B
#define LCD_WRITE_GRAM_CMD  0X2C


static LCDTypedef *g_mlcd;
static void lcd_config(LCDTypedef *lcd);
static void lcd_init(void);

//----------- start---------
// 初始化
void driver_LCD_S6D04D1_init(LCDTypedef *lcd)
{

    g_mlcd = lcd;
    // 配置初始化
    lcd_config(lcd);

    // 初始化液晶屏
    lcd_init();

    lcd->id = LCD_ID_S6D04D1;
}


// 写命令
static void lcd_write_reg(uint16_t value)
{
    FSMC_LCD->REG =  value;
}

// 写数据
static void lcd_write_data(uint16_t value)
{
    FSMC_LCD->RAM = value;
}

// 写颜色
static void lcd_write_color(uint16_t value)
{
#if USE_8080_16BIT
    FSMC_LCD->RAM = value;
#else

    FSMC_LCD->RAM = value>>8;
    FSMC_LCD->RAM = value&0xff;
#endif
}
// 读取数据
static uint16_t lcd_read_data(void)
{
    volatile uint16_t value = FSMC_LCD->RAM;
    return value;
}

static void lcd_write_gram_pre(void)
{
    FSMC_LCD->REG = LCD_WRITE_GRAM_CMD;
}

// 写窗口
static void lcd_set_windows(uint16_t start_x,uint16_t start_y,uint16_t end_x,uint16_t end_y)
{

    uint16_t msx = 0;
    uint16_t msy = 0;

    uint16_t mex = 0;
    uint16_t mey = 0;

#if LCD_USE_HARD_DIR
    msx =start_x;
    msy =start_y;
    mex =end_x;
    mey =end_y;
#else
    // 竖屏
    if(g_mlcd->dir== LCD_DIR_VERTICAL) {
        // 原生分辨率
        msx = start_y;
        msy = LCD_PIX_HEIGHT - end_x -1;
        mex = end_y,
        mey = LCD_PIX_HEIGHT - start_x -1;
    } else { // 横屏
        msx =start_x;
        msy =start_y;
        mex =end_x;
        mey =end_y;
    }
#endif

    lcd_write_reg(LCD_SET_X_CMD);
    lcd_write_data(msx>>8);
    lcd_write_data(msx&0xff);
    lcd_write_data(mex>>8);
    lcd_write_data(mex&0xff);

    lcd_write_reg(LCD_SET_Y_CMD);
    lcd_write_data(msy>>8);
    lcd_write_data(msy&0xff);
    lcd_write_data(mey>>8);
    lcd_write_data(mey&0xff);

    lcd_write_reg(LCD_WRITE_GRAM_CMD);

}

// 写窗口
static void lcd_set_cursor(uint16_t x,uint16_t y)
{

    uint16_t mx = 0;
    uint16_t my = 0;

#if LCD_USE_HARD_DIR
    mx = x;
    my = y;
#else
    // 竖屏
    if(g_mlcd->dir == LCD_DIR_VERTICAL) {
        mx = y;
        my = LCD_PIX_HEIGHT - x -1;
    } else { // 横屏
        mx = x;
        my = y;
    }
#endif



    lcd_write_reg(LCD_SET_X_CMD);
    lcd_write_data(mx>>8);
    lcd_write_data(mx&0xff);
    lcd_write_data(mx>>8);
    lcd_write_data(mx&0xff);

    lcd_write_reg(LCD_SET_Y_CMD);
    lcd_write_data(my>>8);
    lcd_write_data(my&0xff);
    lcd_write_data(my>>8);
    lcd_write_data(my&0xff);
    lcd_write_reg(LCD_WRITE_GRAM_CMD);
}




// ----------- end -----------

// 配置初始化
static void lcd_config(LCDTypedef *lcd)
{
    lcd->lcd_type = LCD_TYPE_8080;
    lcd->width = LCD_PIX_WIDTH;
    lcd->height = LCD_PIX_HEIGHT;

    lcd->lcd_8080.set_x_cmd = LCD_SET_X_CMD;
    lcd->lcd_8080.set_y_cmd = LCD_SET_Y_CMD;
    lcd->lcd_8080.w_ram_cmd = LCD_WRITE_GRAM_CMD;

    lcd->lcd_8080.set_set_windows = lcd_set_windows;
    lcd->lcd_8080.set_set_cursor = lcd_set_cursor;
    lcd->lcd_8080.write_reg = lcd_write_reg;
    lcd->lcd_8080.write_data = lcd_write_data;
    lcd->lcd_8080.read_data = lcd_read_data;
    lcd->lcd_8080.write_color = lcd_write_color;
    lcd->lcd_8080.write_gram_pre = lcd_write_gram_pre;
}


// 液晶初始化
static void lcd_init(void)
{
    // VCOM 3.3V
    lcd_write_reg(0xf4);
    lcd_write_data(0x59);
    lcd_write_data(0x59);
    lcd_write_data(0x52);
    lcd_write_data(0x52);
    lcd_write_data(0x11);

    // Source Output Control Register
    lcd_write_reg(0xf5);
    lcd_write_data(0x12);
    lcd_write_data(0x00);
    lcd_write_data(0x0b);
    lcd_write_data(0xf0);
    lcd_write_data(0x00);
    delay_ms(10);

    // Power Control Register
    lcd_write_reg(0xf3);
    lcd_write_data(0xff);
    lcd_write_data(0x2a);
    lcd_write_data(0x2a);
    lcd_write_data(0x0a);
    lcd_write_data(0x22);
    lcd_write_data(0x72);
    lcd_write_data(0x72);
    lcd_write_data(0x20);

    // Interface Pixel Format
    lcd_write_reg(0x3a);
    lcd_write_data(0x55);

    // Display Control Register
    lcd_write_reg(0xf2);
    lcd_write_data(0x10);
    lcd_write_data(0x10);
    lcd_write_data(0x01);
    lcd_write_data(0x08);
    lcd_write_data(0x08);
    lcd_write_data(0x08);
    lcd_write_data(0x08);
    lcd_write_data(0x00);
    lcd_write_data(0x00);//04
    lcd_write_data(0x1a);
    lcd_write_data(0x1a);

    // Interface Control Register
    lcd_write_reg(0xf6);
    lcd_write_data(0x48);
    lcd_write_data(0x88);//88
    lcd_write_data(0x10);

    // Positive Gamma Control Register for Red
    lcd_write_reg(0xf7);
    lcd_write_data(0x0d);
    lcd_write_data(0x00);
    lcd_write_data(0x03);
    lcd_write_data(0x0e);
    lcd_write_data(0x1c);
    lcd_write_data(0x29);
    lcd_write_data(0x2d);
    lcd_write_data(0x34);
    lcd_write_data(0x0e);
    lcd_write_data(0x12);
    lcd_write_data(0x24);
    lcd_write_data(0x1e);
    lcd_write_data(0x07);
    lcd_write_data(0x22);
    lcd_write_data(0x22);

    //  Negative Gamma Control Register for Red
    lcd_write_reg(0xf8);
    lcd_write_data(0x0d);
    lcd_write_data(0x00);
    lcd_write_data(0x03);
    lcd_write_data(0x0e);
    lcd_write_data(0x1c);
    lcd_write_data(0x29);
    lcd_write_data(0x2d);
    lcd_write_data(0x34);
    lcd_write_data(0x0e);
    lcd_write_data(0x12);
    lcd_write_data(0x24);
    lcd_write_data(0x1e);
    lcd_write_data(0x07);
    lcd_write_data(0x22);
    lcd_write_data(0x22);

    // Positive Gamma Control Register for Green
    lcd_write_reg(0xf9);
    lcd_write_data(0x1e);
    lcd_write_data(0x00);
    lcd_write_data(0x0a);
    lcd_write_data(0x19);
    lcd_write_data(0x23);
    lcd_write_data(0x31);
    lcd_write_data(0x37);
    lcd_write_data(0x3f);
    lcd_write_data(0x01);
    lcd_write_data(0x03);
    lcd_write_data(0x16);
    lcd_write_data(0x19);
    lcd_write_data(0x07);
    lcd_write_data(0x22);
    lcd_write_data(0x22);

    // Negative Gamma Control Register for Green
    lcd_write_reg(0xfA);
    lcd_write_data(0x0D);
    lcd_write_data(0x11);
    lcd_write_data(0x0A);
    lcd_write_data(0x19);
    lcd_write_data(0x23);
    lcd_write_data(0x31);
    lcd_write_data(0x37);
    lcd_write_data(0x3f);
    lcd_write_data(0x01);
    lcd_write_data(0x03);
    lcd_write_data(0x16);
    lcd_write_data(0x19);
    lcd_write_data(0x07);
    lcd_write_data(0x22);
    lcd_write_data(0x22);

    // Positive Gamma Control Register for Blue
    lcd_write_reg(0xfB);
    lcd_write_data(0x0D);
    lcd_write_data(0x00);
    lcd_write_data(0x03);
    lcd_write_data(0x0E);
    lcd_write_data(0x1C);
    lcd_write_data(0x29);
    lcd_write_data(0x2D);
    lcd_write_data(0x34);
    lcd_write_data(0x0E);
    lcd_write_data(0x12);
    lcd_write_data(0x24);
    lcd_write_data(0x1E);
    lcd_write_data(0x07);
    lcd_write_data(0x22);
    lcd_write_data(0x22);

    // Negative Gamma Control Register for Blue
    lcd_write_reg(0xfC);
    lcd_write_data(0x0D);
    lcd_write_data(0x00);
    lcd_write_data(0x03);
    lcd_write_data(0x0E);
    lcd_write_data(0x1C);
    lcd_write_data(0x29);
    lcd_write_data(0x2D);
    lcd_write_data(0x34);
    lcd_write_data(0x0E);
    lcd_write_data(0x12);
    lcd_write_data(0x24);
    lcd_write_data(0x1E);
    lcd_write_data(0x07);
    lcd_write_data(0x22);
    lcd_write_data(0x22);

    // Gate Control Register
    lcd_write_reg(0xFD);
    lcd_write_data(0x11);
    lcd_write_data(0x01);

    // Memory Data Access Control  --RGB 和BRG切换  和刷图方向控制  262页  bit3控制RGB BGR, 0为RGB
    lcd_write_reg(0x36);

#if LCD_USE_HARD_DIR
    // 竖屏
    if(g_mlcd->dir == LCD_DIR_VERTICAL) {
        lcd_write_data(0x00);
    } else { // 横屏
        lcd_write_data(0x60);
    }
#else

    lcd_write_data(0x60);
#endif

    //  RGB

    // BRG
    // lcd_write_data(0x08);
    //

    // Tearing Effect Line ON
    lcd_write_reg(0x35);
    lcd_write_data(0x00);

    // Column Address Set
    lcd_write_reg(0x2A);
    lcd_write_data(0x00);
    lcd_write_data(0x00);
    lcd_write_data(0x00);
    lcd_write_data(0xEF);

    // Page Address Set
    lcd_write_reg(0x2B);
    lcd_write_data(0x00);
    lcd_write_data(0x00);
    lcd_write_data(0x01);
    lcd_write_data(0x8F);

    // KEY Control Register
    lcd_write_reg(0xF1);
    lcd_write_data(0x5A);

    // Logic Test Register2
    lcd_write_reg(0xFF);
    lcd_write_data(0x00);
    lcd_write_data(0x00);
    lcd_write_data(0x00);
    lcd_write_data(0x40);

    // Sleep Out
    lcd_write_reg(0x11);
    delay_ms(120);

    // KEY Control Register
    lcd_write_reg(0xF1);
    lcd_write_data(0x00);

    // Display On
    lcd_write_reg(0x29);
    delay_ms(40);

}




