#include "config.h"
#include "adc.h"
#include "pwm.h"
#include "usart.h"
#include "timer.h"
#include "lcd.h"
#include "gpio.h" 
#include "external_interrupt.h"
#include "mode.h" 

extern volatile uint8_t start_flag;
uint16_t moist_values[MOISTURE_CHANNELS]; //젖은 빨래들의 ADC 값을 불러올 배열



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
	
	uint8_t last_mode = 255;
	uint8_t current_mode = 255;
	uint16_t temp_value = 0;

	
    while (1) 
    {
		//제일 처음 시작할 때만 모드 설정 가능
		if (start_flag) current_mode = set_mode(); // 현재 모드 가져오기
		
		temp_value = ADC_read(6);
		
		//LCD 깜박거림 최소화를 위해 모드 변경시에만 clear하도록 수정
		if (current_mode != last_mode) {
			lcd_clear();
			lcd_goto_xy(0,0);
			if (current_mode == 0) {
				if(fan_mode == 1) lcd_puts("Fan Control\nPower : Strong");
				else if(fan_mode == 2) lcd_puts("Fan Control\nPower : Moderate");
				else if(fan_mode == 3) lcd_puts("Fan Control\nPower : Light");
				else lcd_puts("Fan Control\nPower : Low Noise");
			}
			else if (current_mode == 1) lcd_puts("Reserve Mode");
			else if (current_mode == 2) lcd_puts("Auto Mode");
			else lcd_puts("System Idle");
			last_mode = current_mode;
		}
		
		switch (current_mode)
		{
			case 0: // 바람 세기 모드
			control_fan_mode(moist_values, temp_value);
			break;

			case 1: // 예약 모드
			reserved_mode(moist_values, temp_value);
			break;

			case 2: // 자동 모드
			auto_mode(moist_values, temp_value);
			break;

			default:
			pwm_set_speed(0); // 시작 전에는 팬 정지

			break;
		}
			

		_delay_ms(500); // 0.5초 간격으로 루프 실행

    }
}

