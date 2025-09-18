#ifndef TIMER_H_
#define TIMER_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

void timer_init();
unsigned long millis();

#endif /* TIMER_H_ */