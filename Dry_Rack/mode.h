#ifndef MODE_H_
#define MODE_H_

#include <avr/io.h>

uint16_t most_moist(uint16_t *moist_values);
void restart_reset(void);
uint8_t set_mode(void);
void individual_complete(uint16_t* moist_values);
void all_complete(uint16_t* moist_values);
void control_fan_mode(uint16_t* moist_adc_values, uint16_t temp_adc);
void reserved_mode(uint16_t* moist_adc_values, uint16_t temp_adc);
void auto_mode(uint16_t* moist_adc_values, uint16_t temp_adc);


#endif /* MODE_H_ */