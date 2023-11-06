/*
 * File:   functions.c
 * Author: Delucchi Manuel, Matteo Cappellini
 *
 * Created on 4 novembre 2023, 16.29
 */


#include "xc.h"
#include "headers.h"

// Function that setups the timer to count for the specified amount of ms
void tmr_setup_period(int timer, int ms) {    
    // Fcy = Fosc / 4 = 7.3728 MHz / 4 = 1843200 Hz -> 1843.2 clock steps in 1 ms
    long steps = 1843.2*ms;
    int presc; 
    int t = 1;
    
    if (steps <= 65535) {          // Prescaler 1:1
        presc = 1;
        t = 0; // 00  
    }
    else if (steps/8 <= 65535) {   // Prescaler 1:8
        presc = 8;
        t = 1; // 01
    }
    else if (steps/64 <= 65535) {  // Prescaler 1:64
        presc = 64;
        t = 2; // 10
    }
    else if (steps/256 <= 65535) { // Prescaler 1:256
        presc = 256;
        t = 3; // 11
    }
    
    if (timer == 1) {
        T1CONbits.TON = 0;      // Stops the timer
        TMR1 = 0;               // Reset timer counter
        T1CONbits.TCKPS = t;    // Set the prescale value
        PR1 = steps/presc;      // Set the number of clock step of the counter
        T1CONbits.TON = 1;      // Starts the timer
    }
    else if (timer == 2) {
        T1CONbits.TON = 0;       // Stops the timer
        TMR2 = 0;                // Reset timer counter
        T2CONbits.TCKPS = t;     // Set the prescaler 
        PR2 = steps/presc;       // Set the number of clock step of the counter
        T2CONbits.TON = 1;       // Starts the timer
    }
}

// Function that uses the timer flag to wait until it has expired
// Exit the while loop only when the timerx has expired and it has set the TxIF flag to one
void tmr_wait_period(int timer) { 
    if (timer == 1) {
        while(IFS0bits.T1IF == 0){};
        IFS0bits.T1IF = 0; // Reset timer1 interrupt flag
    }
    else if (timer == 2) {
        while(IFS0bits.T2IF == 0){};
        IFS0bits.T2IF = 0; // Reset timer2 interrupt flag
    }
}

// Function that uses the timer to wait for a given number of ms given as IN
void tmr_wait_ms(int timer, int ms) {    
    tmr_setup_period(timer, ms); 
    tmr_wait_period(timer);     
}

// Setup for the Synchronous serial communication (SPI) peripheral
void spi_setup() {
    SPI1CONbits.MSTEN = 1;  // Master mode 
    SPI1CONbits.MODE16 = 0; // 8-bit mode (16-bit if = 1)
    SPI1CONbits.PPRE = 3;   // 1:1 Primary prescaler 
    SPI1CONbits.SPRE = 6;   // 2:1 Secondary prescaler 
    SPI1STATbits.SPIEN = 1; // Enable SPI
}

// Function to move the LCD cursor to the specified position
void lcd_move_cursor(short position)
{
    // The arg "position" goes from 0 to 31. Beyond 15 we need to write on the second line
    // The value 0x80 moves the cursor at the beginning of the first line
    // The value 0xC0 moves the cursor at the beginning of the second line
    
    // Wait until the SPI Transmit Buffer is not full
    while (SPI1STATbits.SPITBF == 1); 
    if (position < 16)
        SPI1BUF = 0x80 + position;
    else 
        SPI1BUF = 0xC0 + (position%16);
}

// Function to write a string on the LCD starting at a specified position
void lcd_write(short start, char str[]){
    // Move the cursor to the specified starting position
    lcd_move_cursor(start);
    
    // Iterate through the string and write each character to the LCD
    for(int i = 0; str[i] != '\0'; i++) {
        // Wait until the SPI Transmit Buffer is not full
        while (SPI1STATbits.SPITBF == 1) {}; 
        SPI1BUF = str[i];
    }
}

// Function to clear a portion of the LCD by writing spaces
void lcd_clear(short start, short n){
    // Create an array of spaces to clear the LCD
    char spaces[n];
    for(int i=0; i<n; ++i)
        spaces[i] = ' ';
    // Write the spaces to the LCD starting at the specified position
    lcd_write(start, spaces);
}

// Setup for the Universal Asynchronous Receiver-Transmitter (UART) peripheral
void uart_setup() {
    U2BRG = 11;               // (7372800 / 4) / (16 * 9600)
    U2MODEbits.UARTEN = 1;    // Enable UART 
    U2STAbits.UTXEN = 1;      // Enable Transmission (must be after UARTEN)
    // It will trigger the interrupt when 3/4 of the UART buffer is full
    U2STAbits.URXISEL = 0b10; // Set interrupt when buffer is 3/4 full
    IEC1bits.U2RXIE = 1;      // Enable UART receiver interrupt
}

// Function used for transmitting data over a UART
void uart_write(char str[]) {
    for(int i = 0; str[i] != '\0'; i++)
        U2TXREG = str[i];
}