#ifndef I2C_H_
#define I2C_H_

#include <avr/io.h>
#include <avr/delay.h>

void i2c_init(void);
void i2c_start(void); // 반환 타입 변경
void i2c_stop(void);
void i2c_write(unsigned char data); // 반환 타입 변경

#endif /* I2C_H_ */