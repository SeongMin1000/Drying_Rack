/*
 * lcdoutput.c
 *
 * Created: 2025-09-20 오후 5:28:56
 *  Author: KimHyeeun
 */ 

#include "lcdoutput.h"
#include "lcd.h"
#include "external_interrupt.h"


void lcd_print_mode(uint8_t mode){
	lcd_clear();
	lcd_goto_xy(0,0);
		
	switch(mode){
		case 0 : lcd_puts("Fan Control Mode");
		case 1 : lcd_puts("Reserve Mode");
		case 2 : lcd_puts("Auto Mode");
		default: lcd_puts("System Idle");
	}
}

void lcd_print_fan_mode(uint8_t fan_mode){
	lcd_clear();
	lcd_goto_xy(0,0);
	
	switch(fan_mode){
		case 1 : lcd_puts("Fan Control\nPower : Strong");
		case 2 : lcd_puts("Fan Control\nPower : Moderate");
		case 3 : lcd_puts("Fan Control\nPower : Light");
		case 4 : lcd_puts("Fan Control\nPower : Low Noise");
		default: lcd_puts("System Idle");
	}	
}