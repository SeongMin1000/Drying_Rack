#include "config.h"
#include "lcd.h"
#include "i2c.h"

static uint8_t backlight_state = LCD_BL; // 백라이트 상태 저장

// 내부 함수: 4비트 데이터를 I2C 버스에 직접 전송 (Enable 펄스 포함)
// 이 함수는 i2c_start()와 주소 전송이 완료된 후에 호출되어야 합니다.
static void lcd_write_4bits(uint8_t data)
{
    // 데이터와 백라이트 상태를 합친 후 Enable 신호 ON
    i2c_write(data | backlight_state | LCD_E);
    _delay_us(1); // Enable 펄스 길이
    
    // Enable 신호 OFF
    i2c_write(data | backlight_state);
    _delay_us(50); // 명령 실행 대기 시간
}

// 1바이트 데이터를 4비트 모드로 전송
void lcd_send_byte(uint8_t byte, uint8_t is_data)
{
    uint8_t rs_mask = is_data ? LCD_RS : 0;
    uint8_t high_nibble = byte & 0xF0;
    uint8_t low_nibble = (byte << 4) & 0xF0;

    if (i2c_start() != I2C_SUCCESS) return;
    if (i2c_write(LCD_I2C_ADDRESS) != I2C_SUCCESS) {
        i2c_stop();
        return;
    }
    
    // 상위 4비트 전송
    lcd_write_4bits(high_nibble | rs_mask);
    
    // 하위 4비트 전송
    lcd_write_4bits(low_nibble | rs_mask);
    
    i2c_stop();
}

// LCD 초기화
void lcd_init(void)
{
    i2c_init();
    _delay_ms(50); // 전원 인가 후 대기

    // 8비트 모드로 3번 시도 (초기화 시퀀스)
    if (i2c_start() != I2C_SUCCESS) return;
    if (i2c_write(LCD_I2C_ADDRESS) != I2C_SUCCESS) {
        i2c_stop();
        return;
    }

    lcd_write_4bits(0x30);
    _delay_ms(5);
    lcd_write_4bits(0x30);
    _delay_us(100);
    lcd_write_4bits(0x30);
    _delay_us(100);

    // 4비트 모드로 설정
    lcd_write_4bits(0x20);
    _delay_us(100);
    
    i2c_stop();

    // 4비트 모드 설정 완료 후 데이터 전송
    lcd_send_byte(0x28, 0); // Function Set: 4-bit, 2-line, 5x8 dots
    lcd_send_byte(0x0C, 0); // Display ON/OFF Control: Display ON, Cursor OFF, Blink OFF
    lcd_send_byte(0x06, 0); // Entry Mode Set: Increment cursor, no display shift
    lcd_clear();            // Clear Display
}

void lcd_clear(void)
{
    lcd_send_byte(0x01, 0); // Clear display command
    _delay_ms(2);           // 이 명령어는 실행 시간이 길다
}

void lcd_goto_xy(uint8_t x, uint8_t y)
{
    uint8_t address;
    switch(y)
    {
        case 0: address = 0x80; break;
        case 1: address = 0xC0; break;
        default: return;
    }
    lcd_send_byte(address + x, 0);
}

void lcd_puts(const char *str)
{
    while(*str)
    {
        lcd_send_byte(*str++, 1);
    }
}