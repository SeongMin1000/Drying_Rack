#include "config.h"
#include "adc.h"
#include "pwm.h"
#include "usart.h"
#include "timer.h"
#include "lcd.h"
#include "gpio.h"
#include "external_interrupt.h"
#include "mode.h"
#include <stdio.h> // for sprintf

// 시스템 상태를 컨텍스트
DryerContext context;

// 디버그 정보 출력 함수
void print_debug_info(void) {
	char buffer[64]; // sprintf를 위한 버퍼

	USART_transmit_string("\n--- System Boot ---\n");
	USART_transmit_string("--- Settings ---\n");

	// 시스템 설정
	sprintf(buffer, "F_CPU: %lu Hz\n", F_CPU);
	USART_transmit_string(buffer);
	sprintf(buffer, "Baud Rate: %d\n", USART_BAUD_RATE);
	USART_transmit_string(buffer);

	// ADC 설정
	sprintf(buffer, "Moisture Channels: %d\n", MOISTURE_CHANNELS);
	USART_transmit_string(buffer);
	sprintf(buffer, "Dry Threshold: %d\n", MOIST_DRY_THRESHOLD);
	USART_transmit_string(buffer);
	sprintf(buffer, "Overheat Threshold: %d\n", TEMP_OVERHEAT_ADC);
	USART_transmit_string(buffer);

	// 팬 설정
	USART_transmit_string("Fan Speeds (PWM):\n");
	sprintf(buffer, "  Strong: %d\n", FAN_SPEED_STRONG);
	USART_transmit_string(buffer);
	sprintf(buffer, "  Moderate: %d\n", FAN_SPEED_MODERATE);
	USART_transmit_string(buffer);
	sprintf(buffer, "  Light: %d\n", FAN_SPEED_LIGHT);
	USART_transmit_string(buffer);

	USART_transmit_string("\n--- Initial Sensor Values ---\n");

	// ADC 센서 값 출력
	for (uint8_t i = 0; i < MOISTURE_CHANNELS; i++) {
		uint16_t adc_val = ADC_read(i);
		sprintf(buffer, "Moisture Sensor (ADC %d): %u\n", i, adc_val);
		USART_transmit_string(buffer);
	}

	USART_transmit_string("-----------------------\n\n");
}

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

	// --- 부팅 후 디버그 정보 출력 ---
	print_debug_info();

	// --- 전역 인터럽트 활성화 ---
	sei();

	while (1) {
		// 상태 머신 실행
		state_machine_run(&context);
		_delay_ms(100);
	}
}
