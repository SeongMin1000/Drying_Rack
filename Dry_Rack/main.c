/*
 * GccApplication2.c
 *
 * Created: 09-11 (목) 오후 3:12:11
 * Author : csm
 */ 
#include "adc.h"
#include "pwm.h"
#include "usart.h"
#include "timer.h" 

int main(void)
{
    /* Replace with your application code */
    ADC_init();
    pwm_init();
    USART_init();
	timer_init();
	
	 sei(); // 모든 초기화가 끝난 후 전역 인터럽트 활성화

    while (1) 
    {
		
    }
}

