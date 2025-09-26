#ifndef LCD_H_
#define LCD_H_

#include <avr/io.h>
#include <util/delay.h>

void lcd_init(void);
void lcd_clear(void);
void lcd_goto_xy(uint8_t x, uint8_t y);
void lcd_puts(const char *str);

#endif /* LCD_H_ */