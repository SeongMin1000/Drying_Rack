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

// --- 예약 모드 시간 설정 (밀리초 단위) ---
const unsigned long FAN_ON_DURATION_MS      = 3UL * 60 * 1000;  // 팬 작동 시간: 3분
const unsigned long FAN_OFF_INTERVAL_MS     = 27UL * 60 * 1000; // 팬 휴식 시간: 27분
const unsigned long TOTAL_SCHEDULE_TIME_MS  = 1UL * 60 * 60 * 1000; // 기본 시간: 1시간

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

