#include "config.h"
#include "lcd.h"
#include "i2c.h"

// PCF8574 I/O 확장 칩의 핀맵
#define LCD_RS (1 << 0)
#define LCD_RW (1 << 1)
#define LCD_E  (1 << 2)
#define LCD_BL (1 << 3) // 백라이트

static uint8_t backlight_state = LCD_BL; // 백라이트 상태 저장

void lcd_pulse_enable(uint8_t data)
{
	i2c_write(data | LCD_E);
	_delay_us(1);
	i2c_write(data & ~LCD_E);
	_delay_us(50);
}

void lcd_send_nibble(uint8_t nibble, uint8_t is_data)
{
	uint8_t data = (nibble << 4) | backlight_state;
	if (is_data) data |= LCD_RS;
	
	i2c_start();
	i2c_write(LCD_I2C_ADDRESS);
	i2c_write(data);
	lcd_pulse_enable(data);
	i2c_stop();
}

void lcd_send_byte(uint8_t byte, uint8_t is_data)
{
	lcd_send_nibble(byte >> 4, is_data);   // 상위 4비트
	lcd_send_nibble(byte & 0x0F, is_data); // 하위 4비트
}

void lcd_init(void)
{
	i2c_init();
	_delay_ms(50);
	
	lcd_send_nibble(0x03, 0);
	_delay_ms(5);
	lcd_send_nibble(0x03, 0);
	_delay_us(100);
	lcd_send_nibble(0x03, 0);
	
	lcd_send_nibble(0x02, 0); // 4비트 모드 설정
	
	lcd_send_byte(0x28, 0); // 4비트, 2라인, 5x8 폰트
	lcd_send_byte(0x0C, 0); // 디스플레이 ON, 커서 OFF
	lcd_send_byte(0x06, 0); // 엔트리 모드
	lcd_clear();
}

void lcd_clear(void)
{
	lcd_send_byte(0x01, 0);
	_delay_ms(2);
}

void lcd_i2c_goto_xy(uint8_t x, uint8_t y)
{
	uint8_t address = (y == 0) ? 0x80 : 0xC0;
	lcd_send_byte(address + x, 0);
}

void lcd_i2c_puts(const char *str)
{
	while(*str)
	{
		lcd_send_byte(*str++, 1);
	}
}