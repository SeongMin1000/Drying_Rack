/*
 * gpio.h
 *
 * Created: 09-13 (토) 오후 9:13:11
 *  Author: csm
 */ 


#ifndef GPIO_H_
#define GPIO_H_

#include <avr/io.h>
#include <util/delay.h>

typedef enum {
	GPIO_LOW,
	GPIO_HIGH
} GpioState;

// 함수 원형 선언
void gpio_init(void);
void set_led(uint8_t index, GpioState state);
void buzzer_on(void);
void buzzer_off(void);
void play_completion_beep(void);

#endif /* GPIO_H_ */