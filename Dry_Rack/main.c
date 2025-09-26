#include "config.h"
#include "adc.h"
#include "pwm.h"
#include "usart.h"
#include "timer.h"
#include "lcd.h"
#include "gpio.h"
#include "external_interrupt.h"
#include "mode.h"

// 시스템 상태 컨텍스트
DryerContext context;

int main(void) {
	// --- 하드웨어 및 주변 장치 초기화 ---
	ADC_init();
	pwm_init();
	USART_init();
	timer_init();
	//lcd_init();
	gpio_init();
	button_init();

	// --- 시스템 컨텍스트 및 상태 머신 초기화 ---
	context_init(&context);

	// --- 전역 인터럽트 활성화 ---
	sei();

	while (1) {
		// 상태 머신 실행
		state_machine_run(&context);
		_delay_ms(100);
	}
}
