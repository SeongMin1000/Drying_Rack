#ifndef CONFIG_H_
#define CONFIG_H_

// =================================================================
// --- 시스템 기본 설정 ---
// =================================================================

#define F_CPU 16000000UL // 시스템 클럭 주파수 (16MHz)


// =================================================================
// --- 핀 및 포트 설정 ---
// =================================================================

// --- LED 핀 (PORTC) ---
#define LED_DDR     DDRC
#define LED_PORT    PORTC
#define LED0_PIN    PC1 // 첫 번째 빨래 건조 완료 LED
#define LED1_PIN    PC2 // 두 번째 빨래 건조 완료 LED

// --- 부저 핀 (PORTB) ---
#define BUZZER_DDR  DDRB
#define BUZZER_PORT PORTB
#define BUZZER_PIN  PB0

// --- PWM 핀 (PORTB)
#define PWM_OUTPUT_DDR      DDRB
#define PWM_OUTPUT_PIN      DDB1  // 팬 PWM 출력 핀 (OC1A)

// --- 버튼 핀 (PORTD) ---
#define BUTTON_DDR  DDRD
#define BUTTON_PORT PORTD
#define BUTTON_PIN  PIND
#define RESERVE_BUTTON_PIN PD2 // 예약 버튼 (INT0)
#define START_BUTTON_PIN   PD3 // 시작 버튼 (INT1)
#define FAN_MODE_BUTTON_PIN PD7 // 팬 모드 버튼 (PCINT23)


// =================================================================
// --- ADC 센서 설정 ---
// =================================================================
#define MOISTURE_CHANNELS     2     // 젖은 빨래 감지 센서 개수
#define MOIST_DRY_THRESHOLD   10    // 빨래가 다 말랐다고 판단하는 ADC 값


// =================================================================
// --- PWM (팬 제어) 설정 ---
// =================================================================
#define PWM_TOP_VALUE 4999 // PWM 주기를 결정하는 ICR1 레지스터 값 (50Hz)

// 팬 모드별 속도 (Duty Cycle)
#define FAN_SPEED_STRONG      4000  // 80%
#define FAN_SPEED_MODERATE    3000  // 60%
#define FAN_SPEED_LIGHT       2000  // 40%
#define FAN_SPEED_LOW_NOISE   1000  // 20%
#define FAN_SPEED_OFF         0     // 정지


// =================================================================
// --- 동작 모드 설정 ---
// =================================================================

// --- 예약 모드 ---
#define ONE_HOUR_MS (3600000UL) // 1시간 (밀리초)

// =================================================================
// --- I2C 및 주변 장치 설정 ---
// =================================================================
#define LCD_I2C_ADDRESS	  0x27      // I2C LCD 모듈 주소
#define I2C_SCL_FREQUENCY 400000UL  // I2C 통신 속도 (400kHz)


// =================================================================
// --- LCD 설정 ---
// =================================================================
// PCF8574 I/O 확장 칩의 핀맵
#define LCD_RS (1 << 0)
#define LCD_RW (1 << 1)
#define LCD_E  (1 << 2)
#define LCD_BL (1 << 3) // 백라이트


// =================================================================
// --- USART (시리얼 통신) 설정 ---
// =================================================================
#define USART_BAUD_RATE 115200 // 시리얼 통신 속도


#endif /* CONFIG_H_ */
