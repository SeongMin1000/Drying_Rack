#include "config.h"
#include "adc.h"

//현재는 젖은 빨래를 2개만 감지하도록 했지만
//추후에 더 많은 ADC를 사용할 수 있으므로 유지보수가 쉽도록 이렇게 코드 작성했습니다
uint8_t channels[MOISTURE_CHANNELS] = {0, 3}; //젖은 빨래 ADC 값을 읽을 센서 채널

// ADC 초기화 함수
void ADC_init() {
	ADMUX = (1 << REFS0); // AVCC를 기준 전압으로 사용
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // 분주비 128 설정
}

// ADC 변환 시작 및 값 읽기
uint16_t ADC_read(uint8_t channel) {
	// ADMUX의 하위 4비트(채널 선택 비트)를 설정합니다.
	// 기존 REFS0 설정은 유지하면서 채널만 변경합니다.
	ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);

	ADCSRA |= (1 << ADSC); // ADC 변환 시작

	// ADC 변환이 완료될 때까지 대기
	// ADSC 비트는 변환이 끝나면 하드웨어에 의해 0으로 클리어됩니다.
	while (ADCSRA & (1 << ADSC));

	return ADC; // 변환된 값 반환
}

//빨래 건조대 개수에 맞춰서 moist_values배열에 adc 값을 하나씩 넣음
void ADC_input(uint16_t* moists){
	for(int i = 0; i < MOISTURE_CHANNELS; i++){
		moists[i] = ADC_read(channels[i]);
	}
}
