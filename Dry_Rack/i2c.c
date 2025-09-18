#include "i2c.h"

// 100kHz SCL 주파수 설정 (F_CPU = 16MHz)
void i2c_init(void)
{
	TWSR = 0x00; // 프리스케일러 1
	TWBR = 72;   // Bit Rate 설정
	TWCR = (1 << TWEN);
}

void i2c_start(void)
{
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)));
}

void i2c_stop(void)
{
	TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
}

void i2c_write(uint8_t data)
{
	TWDR = data;
	TWCR = (1 << TWINT) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)));
}