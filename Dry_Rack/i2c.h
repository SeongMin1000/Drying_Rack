#ifndef I2C_H_
#define I2C_H_

#include <avr/io.h>
#include <util/twi.h> // TWI 상태 코드를 위해 추가

// I2C 통신 결과 상태
#define I2C_SUCCESS 0
#define I2C_ERROR   1

void i2c_init(void);
uint8_t i2c_start(void); // 반환 타입 변경
void i2c_stop(void);
uint8_t i2c_write(uint8_t data); // 반환 타입 변경

#endif /* I2C_H_ */