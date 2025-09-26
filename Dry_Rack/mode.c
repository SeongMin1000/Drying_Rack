#include "config.h"
#include "mode.h"
#include "external_interrupt.h"
#include "pwm.h"
#include "adc.h"
#include "gpio.h"
#include "timer.h"
#include "lcd.h"
#include "usart.h" // For debugging
#include <stdio.h>

// 인터럽트 핸들러에서 사용하는 외부 변수
extern volatile uint8_t fan_mode;
extern volatile uint8_t reserve_hours;
extern volatile uint8_t start_flag;

// 디버깅 메시지 버퍼
static char debug_buf[64];

// --- Static Function Prototypes ---
static void handle_idle(DryerContext* ctx);
static void handle_fan_control(DryerContext* ctx);
static void handle_reserve(DryerContext* ctx);
static void handle_auto_dry(DryerContext* ctx);
static void handle_completed(DryerContext* ctx);
static void handle_error_overheat(DryerContext* ctx);
static void transition_to_state(DryerContext* ctx, SystemState new_state);
static void update_sensors(DryerContext* ctx);
static void update_drying_status(DryerContext* ctx);
static void display_status(DryerContext* ctx);


// =================================================================
// --- 공개 함수 ---
// =================================================================

// 건조기 초기 상태 정의
void context_init(DryerContext* ctx) {
	USART_transmit_string("--- System Initializing ---\r\n");
	ctx->state = STATE_IDLE;                  // 시스템 초기 상태를 '대기(IDLE)'로 설정
	ctx->fan_speed_setting = FAN_OFF;         // 팬 속도를 '꺼짐(OFF)'으로 초기화
	ctx->reserve_hours_setting = 0;           // 예약 시간 설정을 0으로 초기화
	ctx->all_dry = 0;                         // 모든 빨래가 건조되지 않은 상태로 초기화
	ctx->state_entry_time = millis();         // 현재 상태 진입 시간을 타임스탬프로 기록
	for (int i = 0; i < MOISTURE_CHANNELS; i++) {
		ctx->dry_flags[i] = 0;                // 각 센서별 건조 여부를 '젖음(0)'으로 초기화
	}
	
	// 인터럽트 플래그 및 변수 초기화
	fan_mode = 0;
	reserve_hours = 0;
	start_flag = 0;
	
	set_fan_from_speed_level(FAN_OFF);
	// display_status(ctx);
	USART_transmit_string("[STATE] -> IDLE\r\n");
}

// 건조기 동작 상태 머신
void state_machine_run(DryerContext* ctx) {
	// 센서 입력값 갱신
	update_sensors(ctx);

	// 과열 상태 확인
	if (ctx->temp_value > TEMP_OVERHEAT_ADC && ctx->state != STATE_ERROR_OVERHEAT) {
		sprintf(debug_buf, "[EVENT] Overheat detected! Temp ADC: %u\r\n", ctx->temp_value);
		USART_transmit_string(debug_buf);
		transition_to_state(ctx, STATE_ERROR_OVERHEAT);
	}

	// 현재 상태에 해당하는 핸들러 실행
	switch (ctx->state) {
		case STATE_IDLE:            handle_idle(ctx);           break;
		case STATE_FAN_CONTROL:     handle_fan_control(ctx);    break;
		case STATE_RESERVE:         handle_reserve(ctx);   break;
		case STATE_AUTO_DRY:        handle_auto_dry(ctx);       break;
		case STATE_COMPLETED:       handle_completed(ctx);      break;
		case STATE_ERROR_OVERHEAT:  handle_error_overheat(ctx); break;
	}
	
	// 건조 상태 및 LED 업데이트
	update_drying_status(ctx);
	
	// LCD 상태 업데이트
	// display_status(ctx); // 매 루프마다 호출하면 속도가 느려질 수 있음
}


// =================================================================
// --- 상태 핸들러 ---
// =================================================================

// 초기 대기 상태
void handle_idle(DryerContext* ctx) {
	// START 버튼이 눌렸을 때
	if (start_flag) {
		start_flag = 0; // 플래그 소모
		
		// 어떤 모드로 진입할지 결정
		if (fan_mode > 0) {
			ctx->fan_speed_setting = (FanSpeed)fan_mode;
			transition_to_state(ctx, STATE_FAN_CONTROL);
			} else if (reserve_hours > 0) {
			ctx->reserve_hours_setting = reserve_hours;
			transition_to_state(ctx, STATE_RESERVE);
			} else {
			transition_to_state(ctx, STATE_AUTO_DRY);
		}
		return;
	}
	
	// FAN 또는 RESERVE 버튼으로 설정만 변경했을 때
	if (fan_mode > 0) {
		ctx->fan_speed_setting = (FanSpeed)fan_mode;
		// 상태 변화 없음, 설정만 갱신
	}
	if (reserve_hours > 0) {
		ctx->reserve_hours_setting = reserve_hours;
		// 상태 변화 없음, 설정만 갱신
	}
	
	// 현재 설정을 디스플레이에 표시
	//display_status(ctx);
}

//----------------------모드 설정----------------------------
// 팬 모드 설정 상태
static void handle_fan_control(DryerContext* ctx) {
	if (ctx->all_dry) {
		transition_to_state(ctx, STATE_COMPLETED);
		return;
	}
	
	// 동작 중에도 팬 속도 변경 허용
	if (fan_mode > 0) { // ISR에 의해 새로운 팬 모드 값이 들어왔다면
		ctx->fan_speed_setting = (FanSpeed)fan_mode; // 팬 속도 설정을 바꾸고
		fan_mode = 0; // 사용한 신호는 바로 0으로 초기화 (소모)
	}
	
	set_fan_from_speed_level(ctx->fan_speed_setting);
	//display_status(ctx);
}

// 예약 상태
static void handle_reserve(DryerContext* ctx) {
	// 프로그램 시작된 후부터 현재까지 흐른 시간
	unsigned long elapsed_time = millis() - ctx->state_entry_time;

	// 1. 타이머 만료 시 완료 상태로 전환
	if (elapsed_time >= (unsigned long)ctx->reserve_hours_setting * ONE_HOUR_MS) {
		transition_to_state(ctx, STATE_COMPLETED);
		return;
	}

	// 2. 센서가 건조 완료를 감지하면 완료 상태로 전환
	if (ctx->all_dry) {
		transition_to_state(ctx, STATE_COMPLETED);
		return;
	}
	
	// 3. 자동 건조 로직 실행
	uint16_t min_moist = 1023;
	for(int i=0; i<MOISTURE_CHANNELS; i++) {
		if (ctx->moist_values[i] < min_moist) {
			min_moist = ctx->moist_values[i];
		}
	}
	pwm_set_duty_from_adc(min_moist);
	//display_status(ctx);
}

// 자동 건조 상태
static void handle_auto_dry(DryerContext* ctx) {
	if (ctx->all_dry) {
		transition_to_state(ctx, STATE_COMPLETED);
		return;
	}
	
	// 습도값 기반 자동 팬 제어 (최소값 기준)
	uint16_t min_moist = 1023;
	for(int i=0; i<MOISTURE_CHANNELS; i++) {
		if (ctx->moist_values[i] < min_moist) {
			min_moist = ctx->moist_values[i];
		}
	}
	pwm_set_duty_from_adc(min_moist);
	//display_status(ctx);
}

//----------------------------------------------------------------

// 건조 완료 상태
static void handle_completed(DryerContext* ctx) {
	// 팬 정지 및 완료 비프음 재생
	set_fan_from_speed_level(FAN_OFF);
	play_completion_beep();
	
	// 모든 LED 끄기
	for (int i = 0; i < MOISTURE_CHANNELS; i++) {
		set_led(i, GPIO_LOW);
	}
	
	// START 버튼 누르면 다시 IDLE로 복귀
	if (start_flag) {
		start_flag = 0;
		context_init(ctx); // 새 시작을 위해 재초기화
		transition_to_state(ctx, STATE_IDLE);
	}
	// display_status(ctx);
}

// 팬 과열 상태
static void handle_error_overheat(DryerContext* ctx) {
	set_fan_from_speed_level(FAN_OFF);
	//display_status(ctx);
}


// =================================================================
// --- 보조 함수 ---
// =================================================================

// 상태 처리 함수
static void transition_to_state(DryerContext* ctx, SystemState new_state) {
	const char* state_names[] = {"IDLE", "FAN_CONTROL", "RESERVE", "AUTO_DRY", "COMPLETED", "ERROR_OVERHEAT"};
	sprintf(debug_buf, "[STATE] %s -> %s\r\n", state_names[ctx->state], state_names[new_state]);
	USART_transmit_string(debug_buf);

	ctx->state = new_state;
	ctx->state_entry_time = millis();
	
	// 새로운 상태 화면을 위해 LCD 초기화
	//lcd_clear();
}

// 센서값 업데이트 함수
static void update_sensors(DryerContext* ctx) {
	ADC_input(ctx->moist_values);
	ctx->temp_value = ADC_read(6); // 온도 센서 채널 6

	sprintf(debug_buf, "[SENSOR] Moist1: %4u, Moist2: %4u, Temp: %4u\r\n",
	ctx->moist_values[0], ctx->moist_values[1], ctx->temp_value);
	USART_transmit_string(debug_buf);
}

// 건조 상태 업데이트 함수
static void update_drying_status(DryerContext* ctx) {
	int dry_count = 0;
	for (int i = 0; i < MOISTURE_CHANNELS; i++) {
		if (ctx->moist_values[i] > MOIST_DRY_THRESHOLD) {
			if (ctx->dry_flags[i] == 0) {
				ctx->dry_flags[i] = 1;
				set_led(i, GPIO_HIGH); // 새로 마른 빨래 LED 켜기
				sprintf(debug_buf, "[EVENT] Channel %d is now dry.\r\n", i);
				USART_transmit_string(debug_buf);
			}
			dry_count++;
		}
	}
	
	if (dry_count == MOISTURE_CHANNELS && ctx->all_dry == 0) {
		ctx->all_dry = 1;
		USART_transmit_string("[EVENT] All channels are now dry.\r\n");
	}
}

// 팬 모드에 따른 pwm 업데이트 함수
void set_fan_from_speed_level(FanSpeed speed) {
	uint16_t pwm_value = 0;
	switch (speed) {
		case FAN_STRONG:        pwm_value = FAN_SPEED_STRONG;       break;
		case FAN_MODERATE:      pwm_value = FAN_SPEED_MODERATE;     break;
		case FAN_LIGHT:         pwm_value = FAN_SPEED_LIGHT;        break;
		case FAN_LOW_NOISE:     pwm_value = FAN_SPEED_LOW_NOISE;    break;
		case FAN_OFF:
		default:
		pwm_value = FAN_SPEED_OFF;
		break;
	}
	pwm_set_speed(pwm_value);
}

// lcd 표시 함수
static void display_status(DryerContext* ctx) {
	lcd_goto_xy(0, 0);
	switch (ctx->state) {
		case STATE_IDLE:
		lcd_puts("모드 선택...");
		// 2번째 줄에 설정 표시 가능
		break;
		case STATE_FAN_CONTROL:
		lcd_puts("수동 팬 제어");
		lcd_goto_xy(0, 1);
		switch(ctx->fan_speed_setting) {
			case FAN_STRONG:    lcd_puts("세기: 강");       break;
			case FAN_MODERATE:  lcd_puts("세기: 중간");     break;
			case FAN_LIGHT:     lcd_puts("세기: 약");       break;
			case FAN_LOW_NOISE: lcd_puts("세기: 저소음");   break;
			default:            lcd_puts("세기: 꺼짐");     break;
		}
		break;
		case STATE_RESERVE:
		lcd_puts("예약 건조");
		
		// 남은 시간 계산
		unsigned long remaining_ms = 0;
		unsigned long elapsed_time = millis() - ctx->state_entry_time;
		unsigned long total_duration_ms = (unsigned long)ctx->reserve_hours_setting * ONE_HOUR_MS;

		if (elapsed_time < total_duration_ms) {
			remaining_ms = total_duration_ms - elapsed_time;
		}

		unsigned int remaining_hours = remaining_ms / ONE_HOUR_MS;
		unsigned int remaining_mins = (remaining_ms % ONE_HOUR_MS) / 60000;

		char buf[16];
		sprintf(buf, "남은시간 %02u:%02u", remaining_hours, remaining_mins);
		lcd_goto_xy(0, 1);
		lcd_puts(buf);
		break;
		case STATE_AUTO_DRY:
		lcd_puts("자동 모드");
		// 센서값 표시 가능
		break;
		case STATE_COMPLETED:
		lcd_puts("건조 완료!");
		lcd_goto_xy(0, 1);
		lcd_puts("START 버튼");
		break;
		case STATE_ERROR_OVERHEAT:
		lcd_puts("! 과열 발생 !");
		lcd_goto_xy(0, 1);
		lcd_puts("시스템 정지");
		break;
		default:
		lcd_puts("알 수 없는 상태");
		break;
	}
}
