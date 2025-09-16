/*
 * GccApplication2.c
 *
 * Created: 09-11 (목) 오후 3:12:11
 * Author : csm
 */ 
#include "adc.h"
#include "pwm.h"
#include "usart.h"
#include "timer.h"
#include "lcd.h"
#include "gpio.h" 
#include "external_interrupt.h"
#include "mode.h" 

// --- 예약 모드 시간 설정 (밀리초 단위) ---
const unsigned long FAN_ON_DURATION_MS      = 3UL * 60 * 1000;  // 팬 작동 시간: 3분
const unsigned long FAN_OFF_INTERVAL_MS     = 27UL * 60 * 1000; // 팬 휴식 시간: 27분
const unsigned long TOTAL_SCHEDULE_TIME_MS  = 1UL * 60 * 60 * 1000; // 기본 시간: 1시간

uint16_t moist_values[MOISTURE_CHANNELS]; //젖은 빨래들의 ADC 값을 불러올 배열

volatile uint8_t fan_mode;                                                        
volatile uint8_t reserve_hours;                                                   
volatile uint8_t start_flag;


int main(void)
{
    /* Replace with your application code */
    ADC_init();
    pwm_init();
    USART_init();
	timer_init();
	lcd_init();
	gpio_init();
	button_init();
	
	sei(); // 모든 초기화가 끝난 후 전역 인터럽트 활성화
	
	lcd_clear();
	lcd_goto_xy(0,0);
	lcd_puts("Select Mode...");
	
	
	uint8_t current_mode = 0;
	uint16_t temp_value = 0;
    while (1) 
    {
		if (start_flag) {
			current_mode = set_mode(); // 현재 모드 가져오기

			switch (current_mode)
			{
				case 0: // 바람 세기 모드
				lcd_clear();
				lcd_goto_xy(0, 0);
				lcd_puts("Fan Control");
				control_fan_mode(moist_values, temp_value, fan_mode);
				break;

				case 1: // 예약 모드
				lcd_clear();
				lcd_goto_xy(0, 0);
				lcd_puts("Reserve Mode");
				break;

				case 2: // 자동 모드
				lcd_clear();
				lcd_goto_xy(0, 0);
				lcd_puts("Auto Mode");
				control_fan_mode(moist_values, temp_value, 0);
				break;

				default:
				lcd_clear();
				lcd_goto_xy(0, 0);
				lcd_puts("Idle");
				break;
			}
			} else {
			pwm_set_speed(0); // 시작 전에는 팬 정지
			lcd_clear();
			lcd_goto_xy(0, 0);
			lcd_puts("System Idle");
		}

		_delay_ms(500); // 0.5초 간격으로 루프 실행

    }
}

