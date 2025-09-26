#include "config.h"
#include "external_interrupt.h"
#include "timer.h"
#include "usart.h" // For debugging
#include <stdio.h> // For sprintf

// 디버깅 메시지 버퍼
static char debug_buf[64];

// 버튼 디바운싱을 위한 최소 시간 간격
#define DEBOUNCE_DELAY 200

volatile uint8_t fan_mode = 0;       // 1=강,2=중,3=약,4=저소음
volatile uint8_t reserve_hours = 0;  // 예약 시간 (0=없음, 1~8시간)
volatile uint8_t start_flag = 0;

// 각 인터럽트의 마지막 입력 시간을 저장하는 변수
static volatile unsigned long last_reserve_press_time = 0;
static volatile unsigned long last_fan_mode_press_time = 0;
static volatile unsigned long last_start_press_time = 0;


// ================= 예약 버튼 ISR (PD2 / INT0) =================
ISR(INT0_vect) {
	unsigned long current_time = millis();
	// 마지막으로 버튼이 눌린 시간으로부터 DEBOUNCE_DELAY 이상 지났을 때만 처리
	if (current_time - last_reserve_press_time > DEBOUNCE_DELAY) {
		last_reserve_press_time = current_time; // 현재 시간을 마지막 눌린 시간으로 기록

		reserve_hours++;
		if (reserve_hours > 8) reserve_hours = 0;

		sprintf(debug_buf, "[BUTTON] Reserve button pressed. Hours: %d\r\n", reserve_hours);
		USART_transmit_string(debug_buf);
	}
}


// ================= 바람세기 버튼 ISR (PD7 / PCINT23) =================
ISR(PCINT2_vect) {
	// PD7 핀이 Low가 되었을 때 (버튼이 눌렸을 때)
	if (!(PIND & (1 << FAN_MODE_BUTTON_PIN))) {
		unsigned long current_time = millis();
		// 마지막으로 버튼이 눌린 시간으로부터 DEBOUNCE_DELAY 이상 지났을 때만 처리
		if (current_time - last_fan_mode_press_time > DEBOUNCE_DELAY) {
			last_fan_mode_press_time = current_time;// 현재 시간을 마지막 눌린 시간으로 기록
			fan_mode++;
			if (fan_mode > 4) fan_mode = 0;

			sprintf(debug_buf, "[BUTTON] Fan Mode button pressed. Mode: %d\r\n", fan_mode);
			USART_transmit_string(debug_buf);
		}
	}

}


// ================= 시작 버튼 ISR (PD3 / INT1) =================
ISR(INT1_vect) {
	unsigned long current_time = millis();
	// 마지막으로 버튼이 눌린 시간으로부터 DEBOUNCE_DELAY 이상 지났을 때만 처리
	if (current_time - last_start_press_time > DEBOUNCE_DELAY) {
		last_start_press_time = current_time; // 현재 시간을 마지막 눌린 시간으로 기록

		start_flag = 1;

		USART_transmit_string("[BUTTON] Start button pressed.\r\n");
	}
}

// ================= 버튼 및 인터럽트 초기화 =================
void button_init(void) {
	// PD2(INT0), PD3(INT1), PD7(INT7) 입력 + 풀업
	DDRD &= ~((1 <<RESERVE_BUTTON_PIN) | (1 << START_BUTTON_PIN) | (1 << FAN_MODE_BUTTON_PIN));
	PORTD |= (1 <<RESERVE_BUTTON_PIN) | (1 << START_BUTTON_PIN) | (1 << FAN_MODE_BUTTON_PIN);

	// 외부 인터럽트 활성화
	EIMSK |= (1 << INT0) | (1 << INT1);

	// INT0, INT1: 하강 에지 트리거
	EICRA |= (1 << ISC01) | (1 << ISC11);
	
	PCICR |= (1 << PCIE2); //PORTD 전체 핀체인지 인터럽트 허용
	PCMSK2 |= (1 << PCINT23); //PD7만 허용
	

	//sei(); // 전역 인터럽트 허용
}
