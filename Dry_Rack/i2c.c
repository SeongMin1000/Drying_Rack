#include "i2c.h"
#include "config.h" // F_CPU 및 I2C_SCL_FREQUENCY를 위해 추가

// I2C 통신 속도 설정
void i2c_init(void)
{
    // TWBR 값 계산: ((F_CPU / SCL_FREQ) - 16) / 2
    // 프리스케일러(TWPS) = 1 (TWSR의 하위 2비트 = 0)
    TWSR = 0x00;
    TWBR = ((F_CPU / I2C_SCL_FREQUENCY) - 16) / 2;
    TWCR = (1 << TWEN);
}

// START 신호 전송
uint8_t i2c_start(void)
{
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
    
    // START 신호가 정상적으로 전송되었는지 확인 (0x08)
    if ((TW_STATUS & 0xF8) != TW_START)
    {
        return I2C_ERROR;
    }
    return I2C_SUCCESS;
}

// STOP 신호 전송
void i2c_stop(void)
{
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
    // STOP 신호 전송 후에는 TWINT 플래그가 설정되지 않으므로 대기할 필요 없음
}

// 데이터 전송 및 ACK 수신 확인
uint8_t i2c_write(uint8_t data)
{
    TWDR = data;
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
    
    // SLA+W 전송 후 ACK(0x18) 또는 데이터 전송 후 ACK(0x28) 확인
    if ((TW_STATUS & 0xF8) != TW_MT_SLA_ACK && (TW_STATUS & 0xF8) != TW_MT_DATA_ACK)
    {
        return I2C_ERROR;
    }
    return I2C_SUCCESS;
}