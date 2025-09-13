/*
 * GccApplication2.c
 *
 * Created: 09-11 (목) 오후 3:12:11
 * Author : csm
 */ 

#include <avr/io.h>
#include "adc.h"
#include "pwm.h"
#include "usart.h"


int main(void)
{
    /* Replace with your application code */
    ADC_init();
    pwm_init();
    USART_init();

    while (1) 
    {
		
    }
}

