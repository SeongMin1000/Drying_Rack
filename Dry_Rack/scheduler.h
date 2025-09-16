/*
 * scheduler.h
 *
 * Created: 09-16 (화) 오후 3:22:09
 *  Author: csm
 */ 


#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <avr/io.h>

void reservation_start(uint8_t hours);
void reservation_stop(void);
uint8_t reservation_is_active(void);
uint8_t reservation_is_finished(void);

#endif /* SCHEDULER_H_ */