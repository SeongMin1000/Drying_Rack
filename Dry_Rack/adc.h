/*
 * adc.h
 *
 * Created: 09-11 (목) 오후 3:25:23
 *  Author: csm
 */ 


#ifndef ADC_H_
#define ADC_H_

#include <avr/io.h>

#define MOISTURE_CHANNELS	2 //빨래 건조대 개수(젖은 빨래를 감지할 센서 개수)

//현재는 젖은 빨래를 2개만 감지하도록 했지만
//추후에 더 많은 ADC를 사용할 수 있으므로 유지보수가 쉽도록 이렇게 코드 작성했습니다
extern uint16_t channels[MOISTURE_CHANNELS]; //젖은 빨래 ADC 값을 읽을 센서 채널

void ADC_init(void);
uint16_t ADC_read(uint8_t channel);
void ADC_input(uint16_t* moists);

#endif /* ADC_H_ */