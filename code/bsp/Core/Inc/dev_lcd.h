#ifndef __DEV_LCD_H
#define __DEV_LCD_H

#include "stdio.h"
#include "string.h"
#include "dev_lcd_conf.h"
#include "touch.h"

void dev_lcd_init(void);
void dev_lcd_power_on(void);
void dev_lcd_power_off(void);
void dev_lcd_set_windows(uint16_t start_x,uint16_t start_y,uint16_t end_x,uint16_t end_y);
void dev_lcd_write_color(uint16_t rgb_value);
void dev_lcd_clear(uint16_t rgb_value);

void dev_lcd_draw_point(uint16_t x,uint16_t y,uint16_t color);
void dev_lcd_show_char(uint16_t x,uint16_t y,uint16_t num,uint8_t size,uint8_t mode,uint16_t  font_color, uint16_t  back_color);
void dev_lcd_show_string(uint16_t x,uint16_t y,uint8_t size,uint8_t *p,uint16_t color);

void dev_lcd_draw_circle(uint16_t x0,uint16_t y0,uint8_t r,uint16_t color);
void dev_lcd_draw_rectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,uint16_t color);
void dev_lcd_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,uint16_t color);
void dev_lcd_fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t color);
void dev_lcd_fast_fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t color);
void dev_lcd_color_fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t *color);


#endif
