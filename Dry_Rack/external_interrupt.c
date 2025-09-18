#include "config.h"
#include "external_interrupt.h"

volatile uint8_t fan_mode = 0;       // 0=자동, 1=상,2=중,3=하,4=저소음
volatile uint8_t reserve_hours = 0;  // 예약 시간 (0=없음, 1~8시간)
volatile uint8_t start_flag = 0;


// ================= 예약 버튼 ISR (PD2 / INT0) =================
ISR(INT0_vect) {
	reserve_hours++;
	if (reserve_hours > 8) reserve_hours = 1;
}

// ================= 바람세기 버튼 ISR (PD7 / INT7) =================
ISR(PCINT2_vect) {
	if (!(PIND & (1 << FAN_MODE_BUTTON_PIN))) {
		fan_mode++;
		if (fan_mode > 4) fan_mode = 1;
	}
}

// ================= 시작 버튼 ISR (PD3 / INT1) =================
ISR(INT1_vect) {
	start_flag = 1;
}


// ================= 버튼 및 인터럽트 초기화 =================
void button_init(void) {
	// PD2(INT0), PD3(INT1), PD7(INT7) 입력 + 풀업
	DDRD &= ~((1 << RESERVE_BUTTON_PIN) | (1 << START_BUTTON_PIN) | (1 << FAN_MODE_BUTTON_PIN));
	PORTD |= (1 << RESERVE_BUTTON_PIN) | (1 << START_BUTTON_PIN) | (1 << FAN_MODE_BUTTON_PIN);

	// 외부 인터럽트 활성화
	EIMSK |= (1 << INT0) | (1 << INT1);

	// INT0, INT1: 하강 에지 트리거
	EICRA |= (1 << ISC01) | (1 << ISC11);
	
	PCICR |= (1 << PCIE2); //PORTD 전체 핀체인지 인터럽트 허용
	PCMSK2 |= (1 << PCINT23); //PD7만 허용
	

	//sei(); // 전역 인터럽트 허용
}

