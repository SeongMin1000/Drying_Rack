/*
 * pwm.c
 *
 * Created: 09-11 (목) 오후 3:14:39
 *  Author: csm
 */ 
#include "pwm.h"

void pwm_init()
{
	// 1. 출력 핀 설정
	DDRB |= (1 << DDB1); // PB1 (OC1A) 핀을 출력으로

	// 2. 타이머 제어 레지스터 설정 (TCCR1A, TCCR1B)
	// non-inverting Fast PWM 모드(14), 64 프리스케일러
	TCCR1A |= (1 << COM1A1) | (1 << WGM11);
	TCCR1B |= (1 << WGM13) | (1 << WGM12) | (1 << CS11) | (1 << CS10);

	// 3. PWM 주기(주파수) 설정
	// 16MHz / (64 * 5000) = 50Hz
	ICR1 = 4999;

	// 4. PWM 듀티 사이클 설정 (초기값 0%)
	OCR1A = 0;
}

void pwm_set_speed(uint16_t speed) {
	// OCR1A 레지스터에 값을 설정하여 듀티 사이클을 변경
	// speed 값의 범위는 0 ~ 4999
	if (speed > 4999) {
		speed = 4999; // 최대값을 넘지 않도록 제한
	}
	OCR1A = speed;
}

void pwm_set_duty_from_adc(uint16_t adc_value)
{
    // ADC 값(0~1023)을 PWM 듀티 사이클 값(0~4999)으로 변환.
    // 계산 중 오버플로우를 방지하기 위해 32비트(unsigned long)로 형변환
    uint16_t speed = (uint16_t)(((unsigned long)adc_value * 4999L) / 1023L);
    
    //pwm_set_speed 함수를 호출하여 실제 듀티 사이클을 설정
    pwm_set_speed(speed);
}
