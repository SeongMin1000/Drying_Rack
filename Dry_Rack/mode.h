/*
 * mode.h
 *
 * Created: 09-16 (화) 오전 11:17:43
 *  Author: csm
 */ 


#ifndef MODE_H_
#define MODE_H_

#include <avr/io.h>

uint8_t set_mode(void);
void control_fan_mode(uint16_t* moist_adc_values, uint16_t temp_adc, uint8_t fan_mode); 

#endif /* MODE_H_ */