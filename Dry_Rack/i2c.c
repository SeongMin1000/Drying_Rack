#include "i2c.h"
#include "config.h"

void i2c_init(void)
{
    // TWBR 값 계산: ((F_CPU / SCL_FREQ) - 16) / 2
    // 프리스케일러(TWPS) = 1 (TWSR의 하위 2비트 = 0)
    TWSR = 0x00;
    TWBR = ((F_CPU / I2C_SCL_FREQUENCY) - 16) / 2;
    TWCR = 0X04;
}

// START 신호 전송
void i2c_start(void)
{
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
}

// STOP 신호 전송
void i2c_stop(void)
{
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
}

// 데이터 전송
void i2c_write(unsigned char data)
{
    TWDR = data;
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
}