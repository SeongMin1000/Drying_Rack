/*
 * mode.h
 *
 * Created: 09-16 (화) 오전 11:17:43
 *  Author: csm
 */ 


#ifndef MODE_H_
#define MODE_H_

#include <avr/io.h>

uint16_t most_moist(uint16_t *moist_values);
uint8_t set_mode(void);
void individual_complete();
void control_fan_mode(uint16_t* moist_adc_values, uint16_t temp_adc, uint8_t fan_mode); 
void auto_mode(uint16_t* moist_adc_values, uint16_t temp_adc);

#endif /* MODE_H_ */