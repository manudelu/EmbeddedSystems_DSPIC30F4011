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
//volatile short S5onPressed = 0;
//volatile short S6onPressed = 0;

volatile char charCount[4];

// init buttons
// interrupt S5 & S6
// S5.onPressed -> send the number of charachters to the uart2 //U2TXREG = CharNumber
// S6.onPressed -> clear the first row and reset the characters received counter

void __attribute__((__interrupt__, __auto_psv__)) _U2RXInterrupt() {
    IFS1bits.U2RXIF = 0;
    char receivedChar = U2RXREG; // Copy char from received REG
    cb_push(&cb, receivedChar); // When a new char is received push it
}

void __attribute__ ((__interrupt__ , __auto_psv__ ) ) _INT0Interrupt() {
    IFS0bits.INT0IF = 0;
    IEC0bits.INT0IE = 0; // disable interrupt for INT0
    sprintf(charCount, "%d", cb.count);
    uart_write(charCount);
    tmr_setup_period(TIMER3, 30); // start timer 3
    IEC0bits.T3IE = 1; // enable interrupt for T3
}

/*void __attribute__ ((__interrupt__ , __auto_psv__ ) ) _INT1Interrupt() {
    IFS1bits.INT1IF = 0;
    U2TXREG = cb.count; // Funge ma se passo il numero stampa '<3>' (quadratino)
    IEC0bits.INT0IE = 0; // disable interrupt for INT0
    tmr_setup_period(TIMER2, 20); // start timer 2
}*/

void __attribute__ (( __interrupt__ , __auto_psv__ ) ) _T3Interrupt() {
    IFS0bits.T3IF = 0;
    IFS0bits.INT0IF = 0; // reset INT0 IF
    T3CONbits.TON = 0; // stop timer 3
    TMR3 = 0;
    IEC0bits.T3IE = 0;
    IEC0bits.INT0IE = 1; // enable interrupt for INT0
}

/*void __attribute__((__interrupt__, __auto_psv__)) _INT1Interrupt() {
    IFS1bits.INT1IF = 0;
    //set the various timer to prevent button bouncing
}*/

int main() {
    // Initialize the LCD and UART
    spi_setup();
    uart_setup();
    tmr_wait_ms(TIMER1, 1000);  // Wait 1s to start the SPI correctly
    tmr_setup_period(TIMER2, 10); // Period definition
    
    // Initialize Circular Buffer Variables
    cb.head = 0;
    cb.tail = 0;    
    cb.count = 0;
    
    // Variables to keep track of the received characters and the current position
    char readChar = cb.buffer[cb.tail];
    int writeIndex = 0;    // Points to the next position to write
    
    // Buffer to hold the "Char Recv: XXX" string
    char charCountStr[16]= "Char Recv:";
    
    for (int i = 0; charCountStr[i] != '\0'; i++) {
        lcd_write(i + 16, charCountStr[i]);
    }
    
    lcd_move_cursor(0);
    
    IEC0bits.INT0IE = 1; // enable interrupt for INT0
    
    TRISBbits.TRISB0 = 0; // Set the LED D3 as OUT
    TRISBbits.TRISB1 = 0; // Set the LED D4 as OUT

    while (1) {  
        // Delay for 7ms to simulate the algorithm execution time
        algorithm();
        
        // Check Overflow
        /*if (U2STAbits.OERR == 1) { // Was it pressed before?
            LATBbits.LATB0 = 1;
        }*/
        
        //if (U2STAbits.URXDA == 1) { //???
        
        //non capisco perche ma a quanto pare serve, idk //TODO
        // BOH MAGARI è PER QUESTO CHE CI SERVE LA STRINGA AL POSTO DEL SINGOLO CHAR
        IEC1bits.U2RXIE = 0;
        int read = cb_pop(&cb, &readChar); //READ DATA FROM BUFFER
        IEC1bits.U2RXIE = 1;
        
        //int read = cb_pop(&cb, &readChar); //FORSE QUI MEGLIO USARE IL BUFFER STESSO NELLA POSE TAIL ANZICHE LA STRINGA
        
        if (read == 1) {
            lcd_move_cursor(writeIndex);
            lcd_write(writeIndex, readChar); //FORSE QUI MEGLIO USARE IL BUFFER STESSO NELLA POSE TAIL ANZICHE LA STRINGA
            writeIndex++;
            
            writeIndex = writeIndex % 16; // when it reaches the end go back to the start
            
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
        //}
        
        /*else {
            LATBbits.LATB0 = 1;
        }*/   
        
        //TODO
        
        //TODO
        /*if(IFS1bits.INT1IF == 1){
            lcd_clear(0, 16);
            cb.count = 0;
            writeIndex = 0;
            IFS1bits.INT1IF = 0;
        }*/
        
        lcd_move_cursor(writeIndex); // Set cursor at the desired position
        tmr_wait_period(2); // Wait before next loop
    }
    
    return 0;
}