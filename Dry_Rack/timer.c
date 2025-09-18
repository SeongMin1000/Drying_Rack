 #include "timer.h"

static volatile unsigned long g_millis = 0;

// Timer2 비교 매치 인터럽트 서비스 루틴 (ISR)
// 1ms마다 g_millis를 1씩 증가시킴
ISR(TIMER2_COMPA_vect)
{
	g_millis++;
}

// 1ms 타이머 인터럽트를 설정하는 함수
void timer_init()
{
	// Timer2 CTC 모드, 64 프리스케일러 설정
	TCCR2A |= (1 << WGM21);
	TCCR2B |= (1 << CS22);
	
	// 비교 매치 값 설정 (16MHz / 64 / 1000Hz = 250) -> 249
	OCR2A = 249;
	
	// Timer2 비교 매치 인터럽트 활성화
	TIMSK2 |= (1 << OCIE2A);
	
	// 전역 인터럽트 활성화 (필요 시 main에서 호출)
	// sei();
	// sei()는 보통 모든 초기화가 끝난 후 main에서 한번만 호출하는 것이 안전합니다.
}

// 현재 g_millis 값을 안전하게 읽어 반환하는 함수
unsigned long millis()
{
	unsigned long m;

    // 이 블록 안의 코드가 실행되는 동안에는 모든 인터럽트가 비활성화됩니다.
    // 블록이 끝나면 이전 인터럽트 상태로 자동 복원됩니다.
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
	    m = g_millis;
    }

	return m;
}