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
// bouncing del bottone
// test cb, 17 char alla prima, 15+2, 1000 char alla prima iterazione,
// 34/51 alla prima, 18-60 alla prima, stessi dopo il clear da s6
// 1+2+3+4+..., 17+s6+17, 16/32/48 alla prima
// slide finali

//TODO
// limite al cb.count (999)
// test dei bottoni s5 s6
// bouncing s5
// clear non usa posizione -> remove start
// while(SPI1STATbits.SPIBUF == 1) in lcd_move_cursor, in caso metti solo una volta all'inizio
// controlla cosa fa appena arriva alla fine della prima riga dell'lcd
// ottimizzazione
// char readChar, senza mettere = cb.buffer[cb.tail]
// (writeIndex + 1) % 16 anziche writeIndex++ e writeIndex %= 16
// change array del charCount da [4] a [3]
// OVERFLOW
// fix timer functions (vedi prescaler)
// vedi se vedi enum per TIMERs

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
void uart_write(char str[]);

// Definition for Circular Buffer related functions
void cb_push(volatile CircularBuffer *cb, char data);
int cb_pop(volatile CircularBuffer *cb, char *data);

#endif
