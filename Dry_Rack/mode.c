/*
1. 시작만 누를 경우 (기본: 완전 자동 건조)
	-동작: 수분 센서가 건조를 감지할 때까지 자동으로 풍량을 조절하며 건조합니다.

	-종료 조건: 모든 빨래가 마르면 자동 종료.

2. 시간만 설정 + 시작 (타이머 건조)
	-동작: 설정된 시간 동안 자동으로 풍량을 조절하며 건조합니다.

	-종료 조건: 설정된 시간이 지나면 빨래가 다 말랐는지와 상관없이 즉시 종료.

3. 바람 세기만 설정 + 시작 (수동 건조)	
	-동작: 사용자가 설정한 고정된 바람 세기로 계속 건조합니다.

	-종료 조건: 모든 빨래가 마르면 자동 종료.

4. 시간 + 바람 세기 설정 + 시작 (지정 건조)
	-동작: 사용자가 설정한 고정된 바람 세기로, 설정된 시간 동안만 건조합니다.

	-종료 조건: 설정된 시간이 지나면 즉시 종료
*/

#include "config.h"
#include "mode.h"
#include "external_interrupt.h"
#include "pwm.h"
#include "adc.h"
#include "gpio.h"
#include "timer.h"
#include "lcd.h"

// 디버깅 헤더
#include "usart.h"
#include <stdio.h>

// 인터럽트 핸들러에서 사용하는 외부 변수
extern volatile uint8_t g_reserve_button_flag;
extern volatile uint8_t g_fan_mode_button_flag;
extern volatile uint8_t g_start_button_flag;

// 디버깅 메시지 버퍼
static char debug_buf[64];

// --- Static Function Prototypes ---
static void handle_idle(DryerContext* ctx);
static void handle_drying(DryerContext* ctx);
static void handle_completed(DryerContext* ctx);
static void handle_error_overheat(DryerContext* ctx);

static const char* get_state_name(SystemState state);
static void change_state(DryerContext* ctx, SystemState new_state);
static void update_sensors(DryerContext* ctx);
static void update_drying_status(DryerContext* ctx);
static void set_fan_from_speed_level(FanSpeed speed);
//static void display_status(DryerContext* ctx);

// =================================================================
// --- 공개 함수 ---
// =================================================================

// 건조기 초기 상태 정의
void context_init(DryerContext* ctx) {
	ctx->state = STATE_IDLE;                  // 시스템 초기 상태를 '대기(IDLE)'로 설정
	ctx->fan_speed_setting = FAN_OFF;         // 팬 속도를 '꺼짐(OFF)'으로 초기화
	ctx->reserve_hours_setting = 0;           // 예약 시간 설정을 0으로 초기화
	ctx->all_dry = 0;                         // 모든 빨래가 건조되지 않은 상태로 초기화
	ctx->state_entry_time = millis();         // 현재 상태 진입 시간을 타임스탬프로 기록
	for (int i = 0; i < MOISTURE_CHANNELS; i++) {
		ctx->dry_flags[i] = 0;                // 각 센서별 건조 여부를 '젖음(0)'으로 초기화
	}
	
	// 인터럽트 플래그 및 변수 초기화
	g_reserve_button_flag = 0;
    g_fan_mode_button_flag = 0;
    g_start_button_flag = 0;
	
	//set_fan_from_speed_level(FAN_OFF);
	// 팬 회전 테스트 용
	// main에서 state_machine_run(&context); 부분 주석 처리하고 실행
	//set_fan_from_speed_level(FAN_STRONG);

	// display_status(ctx);

	// ================= 디버깅용 =================
	USART_transmit_string("--- System Initializing ---\r\n");
	USART_transmit_string("[STATE] -> IDLE\r\n");
	// ===========================================
}

// 건조기 동작 상태 머신
void state_machine_run(DryerContext* ctx) {
	// 센서 입력값 갱신
	update_sensors(ctx);

	// 과열 상태 확인
	if (ctx->temp_value > TEMP_OVERHEAT_ADC && ctx->state != STATE_ERROR_OVERHEAT) {
		change_state(ctx, STATE_ERROR_OVERHEAT);

		// ================= 디버깅용 =================
		sprintf(debug_buf, "Overheat detected! Temp ADC: %u\r\n", ctx->temp_value);
		USART_transmit_string(debug_buf);
		// ===========================================
	}

	// 현재 상태에 해당하는 핸들러 실행
	switch (ctx->state) {
		case STATE_IDLE:            handle_idle(ctx);           break;
		case STATE_DRYING:          handle_drying(ctx);         break;
		case STATE_COMPLETED:       handle_completed(ctx);      break;
		case STATE_ERROR_OVERHEAT:  handle_error_overheat(ctx); break;
	}
	// display_status(ctx);
}


// =================================================================
// --- 상태 핸들러 ---
// =================================================================

// 초기 대기 상태
static void handle_idle(DryerContext* ctx) {
    // 예약 버튼 플래그 확인
    if (g_reserve_button_flag) {
        g_reserve_button_flag = 0;

        ctx->reserve_hours_setting++;
        if (ctx->reserve_hours_setting > 8) ctx->reserve_hours_setting = 0;

        // ================= 디버깅용 =================
        sprintf(debug_buf, "Reserve button handled. Hours: %d\r\n", ctx->reserve_hours_setting);
        USART_transmit_string(debug_buf);
		// ===========================================
    }

    // 팬 모드 버튼 플래그 확인
    if (g_fan_mode_button_flag) {
        g_fan_mode_button_flag = 0;

        int current_speed = ctx->fan_speed_setting;
        current_speed++;
        if (current_speed > FAN_LOW_NOISE) current_speed = FAN_STRONG; // 0(OFF)은 건너뜀
        ctx->fan_speed_setting = (FanSpeed)current_speed;

		// ================= 디버깅용 =================
        sprintf(debug_buf, "Fan Mode handled. Mode: %d\r\n", ctx->fan_speed_setting);
        USART_transmit_string(debug_buf);
		// ===========================================
    }

    // 시작 버튼 플래그 확인
    if (g_start_button_flag) {
        g_start_button_flag = 0;

		// ================= 디버깅용 =================
        USART_transmit_string("Start button handled. Transitioning to DRYING state.\r\n");
		// ===========================================

		// 건조기 동작 상태로 전환
        change_state(ctx, STATE_DRYING);
    }
	
	//display_status(ctx);
}

// 건조기 동작 상태
static void handle_drying(DryerContext* ctx) {

    // --- 종료 조건 확인 ---

    // 1. 시간 설정(타이머)이 있는 경우: 시간 경과가 최우선 종료 조건
    if (ctx->reserve_hours_setting > 0) {
        unsigned long elapsed_time = millis() - ctx->state_entry_time;
        unsigned long duration_ms = (unsigned long)ctx->reserve_hours_setting * ONE_HOUR_MS; // 시간 -> ms

        if (elapsed_time >= duration_ms) {
			// ================= 디버깅용 =================
            USART_transmit_string("[EVENT] Timer finished. Transitioning to COMPLETED.\r\n");
			// ===========================================

			change_state(ctx, STATE_COMPLETED);
            return; // 상태가 변경되었으므로 즉시 함수 종료
        }
		
		
    }
    // 2. 시간 설정이 없는 경우: 센서 감지가 종료 조건
    else {
		update_drying_status(ctx);
        // 모든 빨래가 건조되었는지 확인 (이 값은 update_drying_status 함수에서 갱신됨)
        if (ctx->all_dry) {
			// ================= 디버깅용 =================
            USART_transmit_string("[EVENT] All items are dry. Transitioning to COMPLETED.\r\n");
			// ===========================================

			change_state(ctx, STATE_COMPLETED);
            return; // 상태가 변경되었으므로 즉시 함수 종료
        }
    }

    // --- 팬 속도 결정 및 제어 ---

    // 1. 수동으로 팬 속도를 설정한 경우
    if (ctx->fan_speed_setting != FAN_OFF) {
        // 사용자가 설정한 고정 속도로 팬 구동
        set_fan_from_speed_level(ctx->fan_speed_setting);
		//set_fan_from_speed_level(FAN_STRONG);
    }
    // 2. 수동 설정이 없는 경우 (자동 모드)
    else {
        // 수분 센서 값에 따라 자동으로 팬 속도 계산 및 구동
        
		 // 여러 센서 값의 평균을 계산
        uint16_t avg_moist_value = 0;
        for (int i = 0; i < MOISTURE_CHANNELS; i++) {
			if(avg_moist_value < ctx->moist_values[i])
				avg_moist_value = ctx->moist_values[i];
        }

		pwm_set_duty_from_adc(avg_moist_value);
    }
}

// 건조 완료 상태
static void handle_completed(DryerContext* ctx) {
	// 팬 정지 및 완료 비프음 재생
	set_fan_from_speed_level(FAN_OFF);
	play_completion_beep();
	
	// 모든 LED 끄기
	for (int i = 0; i < MOISTURE_CHANNELS; i++) {
		set_led(i, GPIO_LOW);
	}

	// 새 시작을 위해 재초기화
	ctx->reserve_hours_setting = 0;
    ctx->fan_speed_setting = FAN_OFF;
    ctx->all_dry = 0;
    for (int i = 0; i < MOISTURE_CHANNELS; i++) {
        ctx->dry_flags[i] = 0;
    }
	change_state(ctx, STATE_IDLE);
	
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

static const char* get_state_name(SystemState state) {
    switch (state) {
        case STATE_IDLE:           return "IDLE";
        case STATE_DRYING:         return "DRYING";
        case STATE_COMPLETED:      return "COMPLETED";
        case STATE_ERROR_OVERHEAT: return "ERROR_OVERHEAT";
        default:                   return "UNKNOWN";
    }
}

// 상태 처리 함수
static void change_state(DryerContext* ctx, SystemState new_state) {
	if (ctx->state == new_state) {
        return;
    }

	// ================= 디버깅용 =================
	sprintf(debug_buf, "[STATE] %s -> %s\r\n", get_state_name(ctx->state), get_state_name(new_state));
    USART_transmit_string(debug_buf);
	// ===========================================

	ctx->state = new_state;
	ctx->state_entry_time = millis();
	
	//lcd_clear();
}

// 센서값 업데이트 함수
static void update_sensors(DryerContext* ctx) {
	ADC_input(ctx->moist_values);
	ctx->temp_value = ADC_read(6); // 온도 센서 채널 6

	// ================= 디버깅용 =================
	// sprintf(debug_buf, "[SENSOR] Moist1: %4u, Moist2: %4u, Temp: %4u\r\n",
	// ctx->moist_values[0], ctx->moist_values[1], ctx->temp_value);
	// USART_transmit_string(debug_buf);
	// ===========================================
}

// 건조 상태 업데이트 함수
static void update_drying_status(DryerContext* ctx) {
	int dry_count = 0;
	for (int i = 0; i < MOISTURE_CHANNELS; i++) {
		if (ctx->moist_values[i] < MOIST_DRY_THRESHOLD) {
			if (ctx->dry_flags[i] == 0) {
				ctx->dry_flags[i] = 1;
				set_led(i, GPIO_HIGH); // 새로 마른 빨래 LED 켜기

				// ================= 디버깅용 =================
				sprintf(debug_buf, "[EVENT] Channel %d is now dry.\r\n", i);
				USART_transmit_string(debug_buf);
				// ===========================================
			}
			dry_count++;
		}
	}
	
	if (dry_count == MOISTURE_CHANNELS && ctx->all_dry == 0) {
		ctx->all_dry = 1;

		// ================= 디버깅용 =================
		USART_transmit_string("[EVENT] All channels are now dry.\r\n");
		// ===========================================
	}
}

// 팬 모드에 따른 pwm 업데이트 함수
static void set_fan_from_speed_level(FanSpeed speed) {
	uint16_t pwm_value = 0;
	switch (speed) {
		case FAN_STRONG:        pwm_value = FAN_SPEED_STRONG;       break;
		case FAN_MODERATE:      pwm_value = FAN_SPEED_MODERATE;     break;
		case FAN_LIGHT:         pwm_value = FAN_SPEED_LIGHT;        break;
		case FAN_LOW_NOISE:     pwm_value = FAN_SPEED_LOW_NOISE;    break;
		case FAN_OFF:
		default:
								pwm_value = FAN_SPEED_OFF;          break;
	}
	pwm_set_speed(pwm_value);
}

// lcd 표시 함수
// static void display_status(DryerContext* ctx) {
// 	lcd_goto_xy(0, 0);
// 	switch (ctx->state) {
// 		case STATE_IDLE:
// 		lcd_puts("모드 선택...");
// 		// 2번째 줄에 설정 표시 가능
// 		break;
// 		case STATE_FAN_CONTROL:
// 		lcd_puts("수동 팬 제어");
// 		lcd_goto_xy(0, 1);
// 		switch(ctx->fan_speed_setting) {
// 			case FAN_STRONG:    lcd_puts("세기: 강");       break;
// 			case FAN_MODERATE:  lcd_puts("세기: 중간");     break;
// 			case FAN_LIGHT:     lcd_puts("세기: 약");       break;
// 			case FAN_LOW_NOISE: lcd_puts("세기: 저소음");   break;
// 			default:            lcd_puts("세기: 꺼짐");     break;
// 		}
// 		break;
// 		case STATE_RESERVE:
// 		lcd_puts("예약 건조");
		
// 		// 남은 시간 계산
// 		unsigned long remaining_ms = 0;
// 		unsigned long elapsed_time = millis() - ctx->state_entry_time;
// 		unsigned long total_duration_ms = (unsigned long)ctx->reserve_hours_setting * ONE_HOUR_MS;

// 		if (elapsed_time < total_duration_ms) {
// 			remaining_ms = total_duration_ms - elapsed_time;
// 		}

// 		unsigned int remaining_hours = remaining_ms / ONE_HOUR_MS;
// 		unsigned int remaining_mins = (remaining_ms % ONE_HOUR_MS) / 60000;

// 		char buf[16];
// 		sprintf(buf, "남은시간 %02u:%02u", remaining_hours, remaining_mins);
// 		lcd_goto_xy(0, 1);
// 		lcd_puts(buf);
// 		break;
// 		case STATE_AUTO_DRY:
// 		lcd_puts("자동 모드");
// 		// 센서값 표시 가능
// 		break;
// 		case STATE_COMPLETED:
// 		lcd_puts("건조 완료!");
// 		lcd_goto_xy(0, 1);
// 		lcd_puts("START 버튼");
// 		break;
// 		case STATE_ERROR_OVERHEAT:
// 		lcd_puts("! 과열 발생 !");
// 		lcd_goto_xy(0, 1);
// 		lcd_puts("시스템 정지");
// 		break;
// 		default:
// 		lcd_puts("알 수 없는 상태");
// 		break;
// 	}
// }
