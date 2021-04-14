#ifndef _touch_H
#define _touch_H

#include "stdio.h"
#include "stdint.h"

#include "touch_conf.h"
#include "dev_lcd_conf.h"

void touch_init(uint16_t pix_w, uint16_t pix_h, LCD_DIR_ENUM lcd_dir, uint16_t lcd_id);
uint8_t touch_get(uint16_t *x,uint16_t *y);

#endif
