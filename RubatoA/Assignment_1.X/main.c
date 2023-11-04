/*
 * File:   main.c
 * Group: 
 * Member 1: Awais Tahir 
 * Member 2: Raja Farooq Dilshad 
 *
 * Created on November 17, 2022, 2:55 AM
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
#include "UART_header.h"
#include <stdlib.h>
#include <stdio.h>

#define BUFFER_SIZE 60
#define TIMER1 (1) 
#define TIMER2 (2)

typedef struct {
    char buffer[BUFFER_SIZE];
    int readIndex;
    int writeIndex;
} CircularBuffer;
volatile CircularBuffer cb;

int read_buffer( volatile CircularBuffer* cb, char* value)
{
    if (cb->readIndex == cb-> writeIndex)
        return 0;
    *value = cb->buffer[cb->readIndex];
    cb->readIndex++;
    if (cb->readIndex == BUFFER_SIZE)
        cb->readIndex = 0;
    return 1;
}

void write_buffer(volatile CircularBuffer* cb, char value)
{
    cb->buffer[cb->writeIndex] = value;
    cb->writeIndex++;
    if (cb->writeIndex == BUFFER_SIZE)
        cb->writeIndex = 0;
}

void __attribute__ (( __interrupt__ , __auto_psv__ ) ) _U2RXInterrupt() {
IFS1bits.U2RXIF = 0;
char val = U2RXREG;
write_buffer(&cb, val);
}

int main(void) {
    cb.writeIndex = 0;
    cb.readIndex = 0;

    TRISBbits.TRISB0 = 0; // output 
    TRISEbits.TRISE8 = 1;
    TRISDbits.TRISD0 = 1;
    int btnStatus, prevbtnStatus;
    int btn1Status, prevbtn1Status;
    prevbtnStatus = 1;
    prevbtn1Status = 1;

    SPI1CONbits.MSTEN = 1; // master mode
    SPI1CONbits.MODE16 = 0; // 8?bit mode
    SPI1CONbits.PPRE = 3; // 1:1 primary prescaler
    SPI1CONbits.SPRE = 6; // 2:1 secondary prescaler
    SPI1STATbits.SPIEN = 1; // enable SPI
    
    U2BRG = 11; // (7372800 / 4) / (16 ? 9600) ? 1
    U2MODEbits.UARTEN = 1; // enable UART
    U2STAbits.UTXEN = 1; // enable U1TX
    IEC1bits.U2RXIE = 1;
    char strbuf[16];
    //int rowNumber = 1;
    int written = 0;
    int written1 = 0;
    tmr_setup_period(TIMER1, 10);
    while (1) {
        //S5 button, to check the number of characters 
        //received to UART
        btnStatus = PORTEbits.RE8;
        if (btnStatus == 0 && prevbtnStatus == 1) { 
            while (SPI1STATbits.SPITBF == 1);
            SPI1BUF = 0xC0;
            write_string_LCD("Char Recv: ", 16);
            SPI1BUF = 0xCB;
            sprintf(strbuf ,"%d", written1);
            write_string_LCD(strbuf, 16);
            while (SPI1STATbits.SPITBF == 1);
            SPI1BUF = 0x80;
        } 
        prevbtnStatus = btnStatus;
        //S6 button, to reset the counter and first row
        btn1Status = PORTDbits.RD0;
        if (btn1Status == 0 && prevbtn1Status == 1) {
            while (SPI1STATbits.SPITBF == 1);
            SPI1BUF = 0x80;
            clear_LCD();
            written1 = 0;
            while (SPI1STATbits.SPITBF == 1);
            SPI1BUF = 0xCB;
            sprintf(strbuf ,"%d", written1);
            write_string_LCD(strbuf, 16);
            while (SPI1STATbits.SPITBF == 1);
            SPI1BUF = 0xCC;
            sprintf(strbuf ,"%d", written1);
            write_string_LCD(strbuf, 16);
            while (SPI1STATbits.SPITBF == 1);
            SPI1BUF = 0x80;
        } 
        prevbtn1Status = btn1Status;
        //Reading the character from buffer
        //and sending it to UART2
        char value;
        int read = read_buffer(&cb, &value);
        if (read == 1) {
                U2TXREG = value;
                while (SPI1STATbits.SPITBF == 1);
                SPI1BUF = value;
                written++;
                written1++;
                //Clearing the first row of LCD if 
                //received character >= 16
                if (written == 16 || written > 16) {
                    written = 0;
                    while (SPI1STATbits.SPITBF == 1);
                    SPI1BUF = 0x80;
                    clear_LCD();
                    while (SPI1STATbits.SPITBF == 1);
                    SPI1BUF = 0x80;
                }
                //Clearing the first row
                //if '\n' or '\r' is received 
                if(value == '\n' || value == '\r') {
                    while (SPI1STATbits.SPITBF == 1);
                    SPI1BUF = 0x80;
                    clear_LCD();
                    while (SPI1STATbits.SPITBF == 1);
                    SPI1BUF = 0x80;
                }
                //Clearing the first row
                //if '\n' && '\r' is received 
                else if (value == '\n' && value == '\r') {
                    while (SPI1STATbits.SPITBF == 1);
                    SPI1BUF = 0x80;
                    clear_LCD();
                    while (SPI1STATbits.SPITBF == 1);
                    SPI1BUF = 0x80;
                }   
        }
    //Algorithm needs 7ms for its execution
    tmr_wait_ms(TIMER2, 7);
    tmr_wait_period(TIMER1);
    };
}