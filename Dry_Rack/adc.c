/*
 * adc.c
 *
 * Created: 09-11 (목) 오후 3:24:40
 *  Author: csm
 */ 

#include "adc.h"

// ADC 초기화 함수
void ADC_init() {
	ADMUX = (1 << REFS0); // AVCC를 기준 전압으로 사용
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // 분주비 128 설정
}

// ADC 변환 시작 및 값 읽기
uint16_t ADC_read(uint8_t channel) {
	// ADMUX의 하위 3비트(채널 선택 비트)만 변경
	
	// 위의 코드는 아래와 같이 REFS 설정을 명시적으로 유지하는 것과 같습니다.
	ADMUX = (1 << REFS0) | (channel & 0x07);

	ADCSRA |= (1 << ADSC);      // 변환 시작
	while (ADCSRA & (1 << ADSC)); // 변환 완료 대기
	return ADC;
}
