/*
 * File:   main.c
 * Author: Delucchi Manuel, Matteo Cappellini
 *
 * Created on 4 novembre 2023, 16.28
 */

// DSPIC30F4011 Configuration Bit Settings
// 'C' source line config statements

// FOSC Configuration
#pragma config FPR = XT                 // Primary Oscillator Mode (XT)
#pragma config FOS = PRI                // Oscillator Source (Primary Oscillator)
#pragma config FCKSMEN = CSW_FSCM_OFF   // Clock Switching and Monitor (Sw Disabled, Mon Disabled)

// FWDT Configuration
#pragma config FWPSB = WDTPSB_16        // WDT Prescaler B (1:16)
#pragma config FWPSA = WDTPSA_512       // WDT Prescaler A (1:512)
#pragma config WDT = WDT_OFF            // Watchdog Timer (Disabled)

// FBORPOR Configuration
#pragma config FPWRT = PWRT_64          // POR Timer Value (64ms)
#pragma config BODENV = BORV20          // Brown Out Voltage (Reserved)
#pragma config BOREN = PBOR_ON          // PBOR Enable (Enabled)
#pragma config LPOL = PWMxL_ACT_HI      // Low-side PWM Output Polarity (Active High)
#pragma config HPOL = PWMxH_ACT_HI      // High-side PWM Output Polarity (Active High)
#pragma config PWMPIN = RST_IOPIN       // PWM Output Pin Reset (Control with PORT/TRIS regs)
#pragma config MCLRE = MCLR_EN          // Master Clear Enable (Enabled)

// FGS Configuration
#pragma config GWRP = GWRP_OFF          // General Code Segment Write Protect (Disabled)
#pragma config GCP = CODE_PROT_OFF      // General Segment Code Protection (Disabled)

// FICD Configuration
#pragma config ICS = ICS_PGD            // Comm Channel Select (Use PGC/EMUC and PGD/EMUD)

// Include necessary libraries and headers
#include <xc.h>
#include <stdio.h>
#include <stdlib.h>
#include "headers.h"

// Create the CircularBuffer object
volatile CircularBuffer cb;

// Interrupt handler for UART receive
void __attribute__((__interrupt__, __auto_psv__)) _U2RXInterrupt() {
    IFS1bits.U2RXIF = 0;
    char receivedChar = U2RXREG; // Copy char from received REG
    cb_push(&cb, receivedChar);  // When a new char is received, push it to the circular buffer
    U2TXREG = receivedChar;
}

// Interrupt handler for external interrupt 0 (INT0)
void __attribute__ ((__interrupt__ , __auto_psv__ ) ) _INT0Interrupt() {
    IFS0bits.INT0IF = 0;
    IEC0bits.INT0IE = 0;                 // Disable interrupt for INT0
    uart_write(cb.count);               // Send count via UART
    tmr_setup_period(TIMER3, 20);       // Start timer 3  //////////VEDI//////////
    IEC0bits.T3IE = 1;                   // Enable interrupt for timer 3
}

// Interrupt handler for Timer 3
void __attribute__ (( __interrupt__ , __auto_psv__ ) ) _T3Interrupt() {
    IFS0bits.T3IF = 0;
    IFS0bits.INT0IF = 0;    // Reset INT0 IF
    T3CONbits.TON = 0;      // Stop timer 3
    TMR3 = 0;
    IEC0bits.T3IE = 0;
    IEC0bits.INT0IE = 1;    // Enable interrupt for INT0
}

// Main function
int main() {
    // Initialize the LCD, UART and timers
    spi_setup();
    uart_setup();
    tmr_wait_ms(TIMER1, 1000);    // Wait 1s to start the SPI correctly
    tmr_setup_period(TIMER1, 10); // Period definition
    
    // Initialize Circular Buffer Variables
    cb.head = 0;
    cb.tail = 0;    
    cb.count = 0;
    cb.to_read = 0;
    
    // Variables to keep track of the received characters and the current position
    char readChar;
    int writeIndex = 0;    // Points to the next position to write
    
    // Buffer to hold the "Char Recv: XXX" string
    char charCountStr[16]= "Char Recv: ";
    
    // Display the initial message on the LCD
    for (int i = 0; charCountStr[i] != '\0'; i++) {
        lcd_write(i + 16, charCountStr[i]);
    }
    
    lcd_move_cursor(0);
    
    IEC0bits.INT0IE = 1;    // Enable interrupt for INT0

    while (1) {
        // Delay for 7ms to simulate the algorithm execution time
        algorithm();
        
        IEC1bits.U2RXIE = 0;
        int read = cb_pop(&cb, &readChar); // Read data from buffer
        IEC1bits.U2RXIE = 1;
                
        if (read == 1) {
            lcd_move_cursor(writeIndex);
            lcd_write(writeIndex, readChar); 
            writeIndex = (writeIndex + 1) % 16; // When it reaches the end, go back to the start
            
            // Maximum of cb.count not entirely precise
            if (cb.count >= 1000) {
                lcd_clear(16, 16);
                cb.count = 0;
            }
            
            // Clear the first row of the LCD            
            if (readChar == '\r' || readChar == '\n' || writeIndex == 0) {
                lcd_clear(0, 16);
                writeIndex = 0;
            }
            
            // Convert the charCount to a string and display it on the second row
            sprintf(charCountStr, "Char Recv: %d", cb.count);
            for (int i=0; charCountStr[i] != '\0'; i++) 
                lcd_write(i+16, charCountStr[i]);
        }
        
        // Reset count and clear LCD on external interrupt 1 (INT1) trigger
        if(IFS1bits.INT1IF == 1){
            IFS1bits.INT1IF = 0;
            cb.count = 0;
            writeIndex = 0;
            lcd_clear(0, 32);
            
            // si può togliere, toglie tutto ma appena vengono inseriti char riappare tutto
            // preferirei lasciarlo tbh
            sprintf(charCountStr, "Char Recv: %d", cb.count);
            for (int i=0; charCountStr[i] != '\0'; i++) 
                lcd_write(i+16, charCountStr[i]);
        }
        
        lcd_move_cursor(writeIndex); // Set cursor at the desired position
        tmr_wait_period(1); // Wait before next loop
    }
    
    return 0;
}
