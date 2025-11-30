#ifndef MODE_H_
#define MODE_H_

#include <avr/io.h>

// 시스템 상태 정의
typedef enum {
    STATE_IDLE,            // 대기 상태
    STATE_DRYING,          // 건조 중
    STATE_COMPLETED,       // 건조 완료
} SystemState;

// 팬 속도 정의
typedef enum {
    FAN_OFF = 0,       // 팬 정지
    FAN_STRONG,        // 강
    FAN_MODERATE,      // 중간
    FAN_LIGHT,         // 약
    FAN_LOW_NOISE      // 저소음
} FanSpeed;

// 시스템 컨텍스트
typedef struct {
    SystemState state;                         // 현재 시스템 상태
    FanSpeed fan_speed_setting;                // 사용자가 설정한 팬 속도
    uint8_t reserve_hours_setting;             // 사용자가 설정한 예약 시간
    
    uint16_t moist_values[MOISTURE_CHANNELS];  // 습도 센서 값 배열
    
    uint8_t dry_flags[MOISTURE_CHANNELS];      // 0: 젖음, 1: 건조
    uint8_t all_dry;                           // 모든 빨래가 건조되었는지 표시

    unsigned long state_entry_time;            // 현재 상태 진입 시간 타임스탬프

    uint8_t display_needs_update;              // LCD 업데이트 필요 여부 플래그

} DryerContext;

// 메인에선 아래 두 함수만 사용
void context_init(DryerContext* ctx);
void state_machine_run(DryerContext* ctx);

#endif /* MODE_H_ */
