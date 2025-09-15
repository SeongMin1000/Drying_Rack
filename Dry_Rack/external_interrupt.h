/*
 * external_interrupt.h
 *
 * Created: 2025-09-14 오후 7:59:59
 *  Author: KimHyeeun
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

#ifndef EXTERNAL_INTERRUPT_H_
#define EXTERNAL_INTERRUPT_H_

// 버튼 초기화 함수
void button_init(void);

// 전역 변수 extern 선언 (다른 파일에서도 접근 가능)
extern volatile uint8_t fan_mode;
extern volatile uint8_t reserve_hours;
extern volatile uint8_t start_flag;

#endif /* EXTERNAL_INTERRUPT_H_ */