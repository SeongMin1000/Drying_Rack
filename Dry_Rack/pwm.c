#include "config.h"
#include "pwm.h"
#include "usart.h" // For debugging
#include <stdio.h> // For sprintf

// 디버깅 메시지 버퍼
static char debug_buf[64];


void pwm_init()
{
	// 1. 출력 핀 설정
	PWM_OUTPUT_DDR |= (1 << PWM_OUTPUT_PIN);// PB1 (OC1A) 핀을 출력으로

	// 2. 타이머 제어 레지스터 설정 (TCCR1A, TCCR1B)
	// non-inverting Fast PWM 모드(14), 64 프리스케일러
	TCCR1A |= (1 << COM1A1) | (1 << WGM11);
	TCCR1B |= (1 << WGM13) | (1 << WGM12) | (1 << CS11) | (1 << CS10);

	// 3. PWM 주기(주파수) 설정
	// 16MHz / (64 * 5000) = 50Hz
	ICR1 = PWM_TOP_VALUE;

	// 4. PWM 듀티 사이클 설정 (초기값 0%)
	OCR1A = 0;
}

// 팬 모드 설정 시 호출
void pwm_set_speed(uint16_t speed) {
	// OCR1A 레지스터에 값을 설정하여 듀티 사이클을 변경
	// speed 값의 범위는 0 ~ 4999
	if (speed > PWM_TOP_VALUE) {
		speed = PWM_TOP_VALUE; // 최대값을 넘지 않도록 제한
	}
	OCR1A = speed;

	// 디버깅 메시지 추가
	sprintf(debug_buf, "[PWM] Set speed (OCR1A) = %u\r\n", speed);
	USART_transmit_string(debug_buf);
}

// 자동 모드일 때 호출
void pwm_set_duty_from_adc(uint16_t adc_value)
{
	const unsigned long ADC_MAX = 1023UL;
	const unsigned long DUTY_MAX = PWM_TOP_VALUE;

	// ADC 반전 후 스케일
	unsigned long duty = ((ADC_MAX - (unsigned long)adc_value) * DUTY_MAX) / ADC_MAX;

	// 디버깅 메시지 추가
	sprintf(debug_buf, "[PWM] ADC value %u -> Duty cycle %lu\r\n", adc_value, duty);
	USART_transmit_string(debug_buf);
	
	pwm_set_speed((uint16_t)duty);
}
