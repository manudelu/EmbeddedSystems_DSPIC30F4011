/*
 * File:   main.c
 * Author: Delucchi Manuel, Matteo Cappellini
 *
 * Created on 4 novembre 2023, 16.28
 */

// DSPIC30F4011 Configuration Bit Settings

// 'C' source line config statements

// FOSC
#pragma config FPR = XT                 // Primary Oscillator Mode (XT)
#pragma config FOS = PRI                // Oscillator Source (Primary Oscillator)
#pragma config FCKSMEN = CSW_FSCM_OFF   // Clock Switching and Monitor (Sw Disabled, Mon Disabled)

// FWDT
#pragma config FWPSB = WDTPSB_16        // WDT Prescaler B (1:16)
#pragma config FWPSA = WDTPSA_512       // WDT Prescaler A (1:512)
#pragma config WDT = WDT_OFF            // Watchdog Timer (Disabled)

// FBORPOR
#pragma config FPWRT = PWRT_64          // POR Timer Value (64ms)
#pragma config BODENV = BORV20          // Brown Out Voltage (Reserved)
#pragma config BOREN = PBOR_ON          // PBOR Enable (Enabled)
#pragma config LPOL = PWMxL_ACT_HI      // Low-side PWM Output Polarity (Active High)
#pragma config HPOL = PWMxH_ACT_HI      // High-side PWM Output Polarity (Active High)
#pragma config PWMPIN = RST_IOPIN       // PWM Output Pin Reset (Control with PORT/TRIS regs)
#pragma config MCLRE = MCLR_EN          // Master Clear Enable (Enabled)

// FGS
#pragma config GWRP = GWRP_OFF          // General Code Segment Write Protect (Disabled)
#pragma config GCP = CODE_PROT_OFF      // General Segment Code Protection (Disabled)

// FICD
#pragma config ICS = ICS_PGD            // Comm Channel Select (Use PGC/EMUC and PGD/EMUD)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include <stdio.h>
#include <stdlib.h>
#include "headers.h"

// Create the CircularBuffer object
volatile CircularBuffer cb;

// init buttons
// interrupt S5 & S6
// S5.onPressed -> send the number of charachters to the uart2 //U2TXREG = CharNumber
// S6.onPressed -> clear the first row and reset the characters received counter

int main() {
    // Initialize the LCD and UART
    spi_setup();
    uart_setup();
    tmr_wait_ms(TIMER1, 1000);  // Wait 1s to start the SPI correctly
    tmr_setup_period(TIMER1, 10);
    
    // Initialize Circular Buffer Variables
    cb.head = 0;
    cb.tail = 0;    
    
    // Variables to keep track of the received characters and the current position
    char receivedChar;
    char readChar;
    //int readIndex = 0;     // Points to the next character to read
    //int writeIndex = 0;    // Points to the next position to write
    int charCount = 0;
    
    // Buffer to hold the "Char Recv: XXX" string
    char charCountStr[16];

    lcd_write(16, "Char Recv: "); 

    while (1) {  
        // Delay for 7ms to simulate the algorithm execution time
        tmr_wait_ms(TIMER1, 7);
                 
        // Check if a character is available from UART2
        if (U2STAbits.URXDA) {
            // Read the character from UART2
            receivedChar = U2RXREG;

            // Store the character in the circular buffer if there's space
            //if (charCount < BUFFER_SIZE) {
                
            //}
            
            if (cb_push(&cb, receivedChar)) {
                if (cb_pop(&cb, &readChar)) {
                    lcd_write(cb.tail, &readChar);
                    charCount++;
                }
                /*else {
                    lcd_clear(0x80, 16);
                }*/
            }
            /*else { 
                lcd_clear(0x80, 16);
            }*/

            // Check for CR or LF characters
            /*if (receivedChar == '\r' || receivedChar == '\n') {
                // Clear the first row of the LCD
                lcd_clear(0, 16);
                readIndex = 0;
                writeIndex = 0;
                charCount = 0;
            }*/

            // Convert the charCount to a string and display it on the second row
            //-
            //sprintf(charCountStr, "Char Recv: %d", charCount);
            //lcd_write(16, charCountStr);
            //lcd_clear(0, 16);
            
            /*char ReceivedChar = U2RXREG; // Read char from uart2
            cb_push(&cb, ReceivedChar); // Push

            if (cb_pop(&cb, &ReceivedChar) == 1) { // If there is a new char that can be read
                // Write on terminal
                while(SPI1STATbits.SPITBF == 1); 
                SPI1BUF = ReceivedChar; // Write the char on the LCD screen
                cb.tail++; // Go to the next one
            }*/
        }
    }
    
    return 0;
}