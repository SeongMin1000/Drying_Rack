#include "config.h"
#include "usart.h"
#include <stdio.h>

// 2배속 모드(U2X0=1)를 사용하여 보드레이트 오차율을 줄임
#define UBRR_VALUE ((F_CPU / (8UL * USART_BAUD_RATE)) - 1)

void USART_init() {
	/* Baud rate 설정 */
	UBRR0H = (unsigned char)(UBRR_VALUE >> 8);
	UBRR0L = (unsigned char)UBRR_VALUE;

	// 2배속 모드 활성화
	UCSR0A |= (1 << U2X0);

	/* << 수정된 부분 >> 전송(TX)과 수신(RX) 모두 활성화 */
	UCSR0B = (1 << TXEN0) | (1 << RXEN0);

	/* 8-bit 데이터 형식, 1 스톱 비트 */
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void USART_transmit(unsigned char data) {
	/* 전송 버퍼가 비어질 때까지 대기 */
	while (!(UCSR0A & (1 << UDRE0)))
	;

	/* 데이터를 전송 버퍼에 기록 */
	UDR0 = data;
}

void USART_transmit_string(const char* str) {
	while (*str) {
		USART_transmit(*str++);
	}
}

/* << 추가된 함수 >> 데이터 수신 함수 */
unsigned char USART_receive(void) {
	/* 데이터 수신이 완료될 때까지 대기 */
	while (!(UCSR0A & (1 << RXC0)))
	;

	/* 수신된 데이터를 버퍼에서 읽어 반환 */
	return UDR0;
}