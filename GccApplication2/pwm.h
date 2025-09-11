/*
 * pwm.h
 *
 * Created: 09-11 (목) 오후 3:15:15
 *  Author: csm
 */ 


#ifndef PWM_H_
#define PWM_H_

#include <avr/io.h>

void pwm_init();
void pwm_set_speed(uint16_t speed);

#endif /* PWM_H_ */