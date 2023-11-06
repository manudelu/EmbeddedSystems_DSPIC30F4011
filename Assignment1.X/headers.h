/*
 * File:   headers.h
 * Author: Delucchi Manuel, Matteo Cappellini
 *
 * Created on 4 novembre 2023, 16.31
 */

#ifndef HEADERS_H
#define	HEADERS_H

#include "xc.h"

// Definition of timers.
#define TIMER1 1 
#define TIMER2 2

// Definition of Timer related functions
void tmr_setup_period(int timer, int ms); 
void tmr_wait_period(int timer);
void tmr_wait_ms(int timer, int ms);

// Definition of SPI related functions
void spi_setup();
void lcd_move_cursor(short position);
void lcd_write(short start, char str[]);
void lcd_clear(short start, short n);

// Definition of UARTS related functions
void uart_setup();
void uart_write(char str[]);

#endif
