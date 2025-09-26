#include "config.h"
#include "gpio.h"

void gpio_init(void)
{
	// LED 핀들을 출력으로 설정
	LED_DDR |= (1 << LED0_PIN) | (1 << LED1_PIN);
	// 부저 핀을 출력으로 설정
	BUZZER_DDR |= (1 << BUZZER_PIN);
	
	// 모든 LED와 부저를 끈 상태로 시작
	LED_PORT &= ~((1 << LED0_PIN) | (1 << LED1_PIN));
	BUZZER_PORT &= ~(1 << BUZZER_PIN);
}

void set_led(uint8_t index, GpioState state)
{
	uint8_t pin;
	
	// 인덱스에 따라 핀 번호 선택
	if (index == 0) {
		pin = LED0_PIN;
		} else if (index == 1) {
		pin = LED1_PIN;
		} else {
		return; // 범위를 벗어나면 아무것도 하지 않음
	}

	if (state == GPIO_HIGH) {
		LED_PORT |= (1 << pin); // 해당 핀 켜기
		} else {
		LED_PORT &= ~(1 << pin); // 해당 핀 끄기
	}
}

void buzzer_on(void)
{
	BUZZER_PORT |= (1 << BUZZER_PIN);
}

void buzzer_off(void)
{
	BUZZER_PORT &= ~(1 << BUZZER_PIN);
}

void play_completion_beep(void)
{
	buzzer_on();
	_delay_ms(1000); // 1초간 울림
	buzzer_off();
}