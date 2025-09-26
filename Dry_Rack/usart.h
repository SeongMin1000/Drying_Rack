#ifndef USART_H_
#define USART_H_

#include <avr/io.h>

void USART_init();
void USART_transmit(unsigned char data);
void USART_transmit_string(const char* str);

#endif /* USART_H_ */