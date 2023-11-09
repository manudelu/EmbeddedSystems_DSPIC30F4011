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

void __attribute__((__interrupt__, __auto_psv__)) _U2RXInterrupt() {
    IFS1bits.U2RXIF = 0;
    char receivedChar = U2RXREG; // Copy char from received REG
    cb_push(&cb, receivedChar); // When a new char is received push it
    U2TXREG = receivedChar; //vedi i char che arrivano
}

int main() {
    // Initialize the LCD and UART
    spi_setup();
    uart_setup();
    tmr_wait_ms(TIMER1, 1000);  // Wait 1s to start the SPI correctly
    
    // Initialize Circular Buffer Variables
    cb.head = 0;
    cb.tail = 0;    
    
    // Variables to keep track of the received characters and the current position
    //char receivedChar;
    char readChar[16];
    //int readIndex = 0;     // Points to the next character to read
    int writeIndex = 0;    // Points to the next position to write
    int charCount = 0;
    
    // Buffer to hold the "Char Recv: XXX" string
    char charCountStr[16];
    
    //TRISBbits.TRISB0 = 0; // Set the LED as OUT

    while (1) {  
        // Delay for 7ms to simulate the algorithm execution time
        algorithm();
        
        // Check Overflow
        /*if (U2STAbits.OERR == 1) { // Was it pressed before?
            LATBbits.LATB0 = 1;
        }*/
                 
        // Check if a character is available from UART2
        
        int read = cb_pop(&cb, &readChar);
        
        if (read == 1) {
            lcd_write(writeIndex, readChar);
            writeIndex++;
            charCount++;
            // Check for CR or LF characters, vedi se va qui davvero
            writeIndex = writeIndex % 16;
            if (readChar == '\r' || readChar == '\n' || writeIndex == 0) { // sistema "\"?
                // Clear the first row of the LCD
                lcd_clear(0, 16); //vedi
                charCount = 0;
            }
            // Convert the charCount to a string and display it on the second row
            /*sprintf(charCountStr, "Char Recv: %d", charCount);
            lcd_write(16, charCountStr);*/
            
        }
        
    }
    
    return 0;
}