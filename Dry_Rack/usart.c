#include "config.h"
#include "usart.h"
#include <stdio.h>

#define UBRR_VALUE ((F_CPU / (16UL * USART_BAUD_RATE)) - 1) 

void USART_init() {
	/* Baud rate 설정 */
	UBRR0H = (unsigned char)(UBRR_VALUE >> 8);
	UBRR0L = (unsigned char)UBRR_VALUE;

	/* 전송 활성화 (TX), 수신 비활성화 (RX는 활성화하지 않음) */
	UCSR0B = (1 << TXEN0);

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