#ifndef __TOUCH_CONF_H
#define __TOUCH_CONF_H

#include "stdio.h"
#include "stdint.h"
#include "gpio.h"


#define TOUCH_MAX_NUM 5
typedef struct TouchTypedef_TAG
{
	uint16_t lcd_id;
	uint8_t touch_type;
	uint8_t dir;
	uint16_t pix_w;
	uint16_t pix_h;
	uint8_t touch_num;
	uint16_t x[TOUCH_MAX_NUM];
	uint16_t y[TOUCH_MAX_NUM];
}TouchTypedef;






#endif

