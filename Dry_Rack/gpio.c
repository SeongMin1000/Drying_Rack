/*
 * gpio.c
 *
 * Created: 09-13 (토) 오후 9:12:56
 *  Author: csm
 */ 
// gpio.c

#include "gpio.h"

// --- 핀 정의 ---
// LED 핀 (PORTD)
#define LED_DDR     DDRC
#define LED_PORT    PORTC
#define LED0_PIN    DDC1
#define LED1_PIN    DDC2

// 부저 핀 (PORTB)
#define BUZZER_DDR  DDRB
#define BUZZER_PORT PORTB
#define BUZZER_PIN  DDB0


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