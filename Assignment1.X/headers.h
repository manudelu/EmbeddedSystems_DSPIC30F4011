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
#define TIMER3 3
#define BUFFER_SIZE 16 // Define the size of the circular buffer

// CHECKS
// prima riga piena fino a 16 16/32/48 alla prima

//TODO
// controlla cosa fa appena arriva alla fine della prima riga dell'lcd
// fai la roba che quando raggiunge la fine della prima cursor a 0xC0
// ottimizzazione

// fix timer functions (vedi prescaler)
// vedi se fare enum per TIMERs e switch nelle funzioni

// OVERFLOW, tecnicamente gia fatto, non ci va mai
// chiama int0 interrupt, enable T3 interrupt -> dentro (set + disable INT0) if (PORTEbits.RE8) { uart_write(cb.counter); }
//non dovrebbe servire

typedef struct {
    char buffer[BUFFER_SIZE];
    int head;
    int tail;
    int count;
    int to_read;
} CircularBuffer;

// Definition of Timer related functions
void algorithm();
void tmr_setup_period(int timer, int ms); 
void tmr_wait_period(int timer);
void tmr_wait_ms(int timer, int ms);

// Definition of SPI related functions
void spi_setup();
void lcd_move_cursor(short position);
void lcd_write(short start, char str);
void lcd_clear(short start, short n); 

// Definition of UARTS related functions
void uart_setup();
void uart_write(int count);

// Definition for Circular Buffer related functions
void cb_push(volatile CircularBuffer *cb, char data);
int cb_pop(volatile CircularBuffer *cb, char *data);

#endif
