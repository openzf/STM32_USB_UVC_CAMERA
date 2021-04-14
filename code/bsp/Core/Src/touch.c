#include "touch.h"

#include "siic.h"

// ´¥ÃþÐ¾Æ¬
#include "ft5206.h"
#include "gt9147.h"

TouchTypedef mtouch;

// »ñÈ¡´¥Ãþ
uint8_t (*g_scan)(TouchTypedef *touch);


void touch_init(uint16_t pix_w, uint16_t pix_h, LCD_DIR_ENUM lcd_dir, uint16_t lcd_id)
{

    mtouch.pix_h = pix_h;
    mtouch.pix_w = pix_w;
    mtouch.dir = lcd_dir;
    mtouch.lcd_id = lcd_id;
	

    HAL_GPIO_WritePin(TOUCH_RESET_GPIO_Port,TOUCH_RESET_Pin,GPIO_PIN_SET);
    HAL_Delay(500);
    HAL_GPIO_WritePin(TOUCH_RESET_GPIO_Port,TOUCH_RESET_Pin,GPIO_PIN_RESET);
    HAL_Delay(500);
    HAL_GPIO_WritePin(TOUCH_RESET_GPIO_Port,TOUCH_RESET_Pin,GPIO_PIN_SET);
    HAL_Delay(500);

    SIIC_Init();

    // ³õÊ¼»¯Ð¾Æ¬, ×¢²á»Øµ÷º¯Êý
    //ft5206_init();
    gt9147_init(&g_scan);
		
}



uint8_t touch_get(uint16_t *x,uint16_t *y)
{
	  if(g_scan(&mtouch)) {
        *x = mtouch.x[0];
        *y = mtouch.y[0];
        printf("touch: %d %d\r\n",*x, *y);
        return 1;
    } else {
        return 0;
    }

}
