/*
 * mode.c
 *
 * Created: 2025-09-14 오후 8:15:18
 *  Author: KimHyeeun
 */ 

#include "external_interrupt.h"
#include "pwm.h"
#include "adc.h"


//ADC1 : PC0, ADC2 : PC3

//빨래 중 가장 덜마른 빨래 return
uint16_t most_moist(uint16_t *moist_values){
	uint16_t most_moist_value = moist_values[0];
	for(int i = 0; i < MOISTURE_CHANNELS; i++){
		if(most_moist_value < moist_values[i]) 
			most_moist_value = moist_values[i];
	}
	
	return most_moist_value;
}

//모드 설정
//0 : 바람 세기 설정 모드, 1 : 예약 모드, 2 : 그냥 모드
uint8_t set_mode(void){
	if(start_flag == 1) {
		if(reserve_hours > 0) return 0;
		else if(fan_mode > 0) return 1;
		else return 2;
	}
}

//빨래 젖을 수록 ADC 값 낮음
//빨래 마를 수록 ADC 값 높음

//바람 세기 조절 모드
//상시 바람 세기 조절 가능
// fan_mode: 0=자동, 1=강, 2=중, 3=약, 4=저소음
// temp_adc : 팬 온도센서 ADC 값
// moist_adc: 빨래 젖음 상태 ADC 값
void control_fan_mode(uint16_t* moist_adc_values, uint16_t temp_adc, uint8_t fan_mode)
{
	ADC_input(moist_adc_values); //배열에 ADC 값 모두 측정해서 넣기
	uint16_t moist_adc = most_moist(moist_adc_values); //ADC 값 중 가장 덜 마른 빨래의 ADC값 
	
	
	// 팬 온도 과열 체크 (예: 70도 이상이라고 가정 → ADC 800 이상)
	if (temp_adc > 800) {
		pwm_set_speed(0);   // 팬 정지
		return;
	}

	// 다 말랐다고 판단 (예: ADC > 950)
	if (moist_adc > 950) {
		pwm_set_speed(0);   // 팬 끄고 절전
		return;
	}

	// 모드별 동작
	switch (fan_mode) {
		case 1: // 강
		pwm_set_speed(4000); // 80% duty
		break;

		case 2: // 중
		pwm_set_speed(3000); // 60% duty
		break;

		case 3: // 약
		pwm_set_speed(2000); // 40% duty
		break;

		case 4: // 저소음
		pwm_set_speed(1000); // 20% duty
		break;

		default:
		pwm_set_speed(0); // 잘못된 값 → 안전하게 OFF
		break;
	}
}
