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
	// ADMUX의 하위 3비트(채널 선택 비트)만 변경
	
	// 위의 코드는 아래와 같이 REFS 설정을 명시적으로 유지하는 것과 같습니다.
	ADMUX = (1 << REFS0) | (channel & 0x07);

	ADCSRA |= (1 << ADSC);      // 변환 시작
	while (ADCSRA & (1 << ADSC)); // 변환 완료 대기
	return ADC;
}

//빨래 건조대 개수에 맞춰서 moist_values배열에 adc 값을 하나씩 넣음
void ADC_input(uint16_t* moists){
	for(int i = 0; i < MOISTURE_CHANNELS; i++){
		moists[i] = ADC_read(channels[i]);
	}
}
