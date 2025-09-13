﻿/*
 * i2c.h
 *
 * Created: 09-13 (토) 오후 8:24:28
 *  Author: csm
 */ 


#ifndef I2C_H_
#define I2C_H_

#include <avr/io.h>

void i2c_init(void);
void i2c_start(void);
void i2c_stop(void);
void i2c_write(uint8_t data);

#endif /* I2C_H_ */