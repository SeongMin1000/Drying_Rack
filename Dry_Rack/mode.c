/*
 * mode.c
 *
 * Created: 2025-09-14 오후 8:15:18
 *  Author: KimHyeeun
 */ 

#include "external_interrupt.h"
#include "pwm.h"
#include "adc.h"
#include "gpio.h"

#define MOIST_DRY_THRESHOLD		950  //빨래가 다 말랐다고 판단하는 ADC값
#define TEMP_OVERHEAT_ADC		800  //fan의 온도가 너무 높다고 판단하는 ADC값

extern volatile uint8_t start_flag;
extern volatile uint8_t fan_mode;
extern volatile uint8_t reserve_hours;
uint8_t prev_dry_state[MOISTURE_CHANNELS] = {0}; // 개별 빨래의 이전 건조 상태 0=안마름, 1=마름
static unsigned long fan_start_time = 0; //팬 돌기 시작한 시간
static unsigned long fan_off_time = 0; //팬 휴식기 들어간 시간
static uint8_t fan_off = 0; //1이면 fan이 꺼져있는 상태이다
static uint8_t all_off = 0;
static uint8_t last_mode = 255;

//ADC1 : PC0, ADC2 : PC3

//빨래 중 가장 덜마른 빨래 return
uint16_t most_moist(uint16_t *moist_values){
	uint16_t most_moist_value = moist_values[0];
	for(int i = 0; i < MOISTURE_CHANNELS; i++){
		if(most_moist_value > moist_values[i]) 
			most_moist_value = moist_values[i];
	}
	
	return most_moist_value;
}

//전원이 들어와있는 상태에서 한 모드가 다 끝난 후 다시 시작했을 때 상태값/flag 다 초기화
void restart_reset(void){
	fan_start_time = 0;
	fan_off_time = 0;
	fan_off = 0;
	all_off = 0;
}

//모드 설정
//0 : 바람 세기 설정 모드, 1 : 예약 모드, 2 : 그냥 모드
uint8_t set_mode(void){
	start_flag = 0;
	restart_reset();
	fan_start_time = millis(); //시작을 누른 시간 
	
	if(fan_mode > 0) return 0;
	else if(reserve_hours > 0) return 1;
	else return 2;
}


//개별 빨래 건조 완료시 LED 출력
//PC0(ADC1) 건조 완료시 - PC1 LED 켜짐
//PC3(ADC2) 건조 완료시 - PC2 LED 켜짐
void individual_complete(uint16_t* moist_values){
	for(int i = 0; i < MOISTURE_CHANNELS; i++){
		uint8_t dry = (moist_values[i] > MOIST_DRY_THRESHOLD);
		if(dry == 1 && dry != prev_dry_state[i]){ // 해당 빨래가 다 말랐고 말랐다는 판정이 이번이 처음일 때만 LED를 키고, 빨래 건조 상태값에 건조라고 표기한다.
			set_led(i, GPIO_HIGH);
			prev_dry_state[i] = dry;
		}
	}
}


//모든 빨래 건조 완료시
void all_complete(uint16_t* moist_values){
	pwm_set_speed(0); //fan 종료
	
	for(int i = 0; i < MOISTURE_CHANNELS; i++){ //모든 LED off
		if(moist_values[i] > 950){ 
			set_led(i, GPIO_LOW);
		}
	}	
}

//빨래 젖을 수록 ADC 값 낮음
//빨래 마를 수록 ADC 값 높음

//바람 세기 조절 모드
//상시 바람 세기 조절 가능
// fan_mode: 0=자동, 1=강, 2=중, 3=약, 4=저소음
// temp_adc : 팬 온도센서 ADC 값
// moist_adc: 빨래 젖음 상태 ADC 값
void control_fan_mode(uint16_t* moist_adc_values, uint16_t temp_adc)
{
	ADC_input(moist_adc_values); //배열에 ADC 값 모두 측정해서 넣기
	individual_complete(moist_adc_values);
	uint16_t moist_adc = most_moist(moist_adc_values); //ADC 값 중 가장 덜 마른 빨래의 ADC값 
	
	
	// 팬 온도 과열 체크 (예: 70도 이상이라고 가정 → ADC 800 이상)
	if (temp_adc > TEMP_OVERHEAT_ADC) {
		pwm_set_speed(0);   // 팬 정지
		return;
	}

	// 다 말랐다고 판단 (예: ADC > 950)
	if (moist_adc > MOIST_DRY_THRESHOLD) {
		pwm_set_speed(0);   // 팬 끄고 절전
		fan_mode = 0;
		return;
	}

	// 모드별 동작
	switch (fan_mode) {
		case 0: //다 말랐을 때
		play_completion_beep(); //부저 울림
		all_complete(moist_adc_values); //절전 모드
		break;
		
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

//예약 모드
void reserved_mode(uint16_t* moist_adc_values, uint16_t temp_adc)
{
	ADC_input(moist_adc_values);
	individual_complete(moist_adc_values);
	uint16_t moist_adc = most_moist(moist_adc_values);
	unsigned long current_time = millis();

	// 팬 온도 과열 체크 (예: 70도 이상이라고 가정 → ADC 800 이상)
	if (temp_adc > TEMP_OVERHEAT_ADC) {
		pwm_set_speed(0);   // 팬 정지
		return;
	}
	
	
	if(all_off == 0 && current_time - fan_start_time >= reserve_hours * 3600000UL){ //사용자가 누른 시간(reserve_hours)만큼의 시간이 흐르면 all_off이 된다
		all_off = 1;
		return;
	}
	else if(all_off == 0){
		pwm_set_duty_from_adc(moist_adc); //all_off 0이라면(아직 예약 시간 이내라면) adc 변화에 따라 pwm duty cycle을 조정하여 팬의 power를 바꾼다 
		return;
	}
	else if(all_off == 1){ //all_off 1이라면(예약 시간이 지났다면) 팬의 작동을 멈춘다
		play_completion_beep();
		all_complete(moist_adc_values);
		return;
	}
	
	
	return;
	
}

//기본 자동 시작 모드
void auto_mode(uint16_t* moist_adc_values, uint16_t temp_adc)
{
	ADC_input(moist_adc_values);
	individual_complete(moist_adc_values);
	uint16_t moist_adc = most_moist(moist_adc_values);
	unsigned long current_time = millis();

	// 팬 온도 과열 체크 (예: 70도 이상이라고 가정 → ADC 800 이상)
	if (temp_adc > TEMP_OVERHEAT_ADC) {
		pwm_set_speed(0);   // 팬 정지
		return;
	}

	// 다 말랐다고 판단 (예: ADC > 950)
	if (moist_adc > MOIST_DRY_THRESHOLD) {
		play_completion_beep();
		all_complete(moist_adc_values);
		all_off = 1;
		return;
	}
	
	if(all_off == 0){ 
		//25분 동안 팬 작동
		//5분 휴식
		if(current_time - fan_start_time >= 1500000UL){ //25분 작동 했다면 팬 멈춘다
			pwm_set_speed(0);
			fan_off = 1;
			fan_start_time = millis();
			
		}
		else if(fan_off == 1 && current_time - fan_start_time >= 300000UL) { //5분 동안 휴식했으면 다시 팬 작동
			fan_off = 0;
			fan_start_time = millis();
			pwm_set_duty_from_adc(moist_adc);
		}
		else if(fan_off == 0){
			pwm_set_duty_from_adc(moist_adc);
		}		
	}
	
	return;
}

