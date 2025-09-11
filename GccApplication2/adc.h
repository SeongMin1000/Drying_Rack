/*
 * adc.h
 *
 * Created: 09-11 (목) 오후 3:25:23
 *  Author: csm
 */ 


#ifndef ADC_H_
#define ADC_H_

#include <avr/io.h>

void ADC_init(void);
uint16_t ADC_read(uint8_t channel);

#endif /* ADC_H_ */