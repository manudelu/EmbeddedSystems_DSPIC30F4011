/*
 * File:   main.c
 * Author: Delucchi Manuel, Matteo Cappellini
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

// Include necessary libraries and header
#include <xc.h>
#include <stdio.h>
#include <stdlib.h>
#include "header.h"

// Create the CircularBuffer object
volatile CircularBuffer cb;

// Interrupt handler for the char recevied on UART2
void __attribute__((__interrupt__, __auto_psv__)) _U2RXInterrupt() {
    IFS1bits.U2RXIF = 0;         // Reset UART2 Receiver Interrupt Flag Status bit
    char receivedChar = U2RXREG; // Get char from UART2 received REG
    cb_push(&cb, receivedChar);  // When a new char is received, push it to the circular buffer
}

// Interrupt handler for external interrupt 0 (INT0)
void __attribute__ ((__interrupt__ , __auto_psv__ )) _INT0Interrupt() {
    IFS0bits.INT0IF = 0;            // Reset External Interrupt 0 Flag Status bit
    IEC0bits.INT0IE = 0;            // Disable Interrupt for INT0
    tmr_setup_period(TIMER3, 20);   // Setup timer 3
    IEC0bits.T3IE = 1;              // Enable interrupt for TIMER3
}

// Interrupt handler for Timer 3 used to avoid the bouncing of S5
void __attribute__ (( __interrupt__ , __auto_psv__ )) _T3Interrupt() {
    IFS0bits.T3IF = 0;          // Reset External Interrupt 3 Flag Status bit
    IFS0bits.INT0IF = 0;        // Reset INT0 Interrupt Flag
    T3CONbits.TON = 0;          // Stop timer 3
    TMR3 = 0;                   // Reset timer counter
    IEC0bits.T3IE = 0;          // Disable interrupt Timer 3
    IEC0bits.INT0IE = 1;        // Enable interrupt for INT0
    if (PORTEbits.RE8) {        
        uart_write(cb.count);   // Send the count via UART2 if RE8 is set
    }
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
    
    IEC0bits.INT0IE = 1;    // Enable interrupt for INT0
    
    // Initializze Variables
    char readChar;          // Keep track of the received characters
    int writeIndex = 0;     // Keep track of the position on the LCD
    
    // Buffer to hold the "Char Recv: XXX" string
    char charCountStr[16]= "Char Recv: ";
    
    // Display the initial message on the LCD
    for (int i = 0; charCountStr[i] != '\0'; i++) {
        lcd_write(i + 16, charCountStr[i]);
    }

    while (1) {
        // Delay for 7ms to simulate the algorithm execution time
        algorithm();
        
        IEC1bits.U2RXIE = 0;                // Disable UART2 Receiver Interrupt
        int read = cb_pop(&cb, &readChar);  // Pop data from buffer
        IEC1bits.U2RXIE = 1;                // Enable UART2 Receiver Interrupt
                
        if (read == 1) {       
            // Write up to 16 char on the first row of the LCD before clearing it
            if (writeIndex == 16) {
                writeIndex = 0;
                lcd_clear(0, 16);
            }
            
            lcd_move_cursor(writeIndex);        // Move to the desired position on the LCD
            lcd_write(writeIndex, readChar);    // Write on the LCD screen
            writeIndex++;                       // Go to the next position
            
            // Clear the first row of the LCD when CR and LF char are received           
            if (readChar == '\r' || readChar == '\n') { 
                lcd_clear(0, 16);
                writeIndex = 0;
            }
            
            // Convert the charCount to a string and display it on the second row
            sprintf(charCountStr, "Char Recv: %d", cb.count);
            for (int i=0; charCountStr[i] != '\0'; i++) 
                lcd_write(i + 16, charCountStr[i]);
        }
        
        // Reset count and clear LCD on external interrupt 1 (INT1) trigger
        if(IFS1bits.INT1IF == 1){
            IFS1bits.INT1IF = 0;
            cb.count = 0;
            writeIndex = 0;
            lcd_clear(0, 32);
            
            sprintf(charCountStr, "Char Recv: %d", cb.count);
            for (int i=0; charCountStr[i] != '\0'; i++) 
                lcd_write(i + 16, charCountStr[i]);
        }
        
        lcd_move_cursor(writeIndex); // Set cursor at the desired position
        tmr_wait_period(TIMER1); // Wait before next loop
    }
    
    return 0;
}
