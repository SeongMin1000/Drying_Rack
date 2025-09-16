/*
 * scheduler.c
 *
 * Created: 09-16 (화) 오후 3:22:51
 *  Author: csm
 */ 
#include "scheduler.h"
#include "timer.h"

static uint8_t is_reservation_active = 0;
static unsigned long reservation_start_time_ms = 0;
static unsigned long reservation_duration_ms = 0;

// 지정된 시간(시간 단위)으로 예약 타이머를 시작
void reservation_start(uint8_t hours)
{
	// 유효한 시간인지 확인 (1~8시간)
	if (hours > 0 && hours <= 8)
	{
		// 시간을 밀리초로 변환하여 저장
		reservation_duration_ms = (unsigned long)hours * 3600000UL; // 1시간 = 3,600,000ms
		
		// 현재 시간을 시작 시간으로 기록
		reservation_start_time_ms = millis();
		
		// 타이머를 활성 상태로 설정
		is_reservation_active = 1;
	}
}

// 예약 타이머를 강제로 중지
void reservation_stop(void)
{
	is_reservation_active = 0;
}

// 예약 타이머가 현재 활성 상태인지 확인 (true: 1, false: 0)
uint8_t reservation_is_active(void)
{
	return is_reservation_active;
}

// 예약 시간이 다 되었는지 확인, 시간이 다 되면 1을 반환하고 타이머를 비활성화함
uint8_t reservation_is_finished(void)
{
	// 타이머가 활성화되어 있지 않으면, 끝난 것 아님
	if (!is_reservation_active)
	{
		return 0; // 끝나지 않음
	}
	
	// 현재 시간과 시작 시간의 차이가 설정된 시간을 지났는지 확인
	if (millis() - reservation_start_time_ms >= reservation_duration_ms)
	{
		is_reservation_active = 0; // 타이머 비활성화
		return 1; // 끝남
	}
	
	return 0; // 아직 끝나지 않음
}