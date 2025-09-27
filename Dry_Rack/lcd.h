#ifndef LCD_H_
#define LCD_H_

void lcd_write(unsigned char x);
void lcd_4bit_send(unsigned char x);
void lcd_cmd(unsigned char x);
void lcd_dwr(unsigned char x);
void lcd_msg(char *c);
void lcd_clear();
void lcd_init();

#endif /* LCD_H_ */