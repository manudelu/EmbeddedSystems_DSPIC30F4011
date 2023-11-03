/*
 * File:   Read.c
 * Author: matte
 *
 * Created on 3 novembre 2023, 11.29
 */

// FOSC
#pragma config FPR = XT            // Primary Oscillator Mode (XT)
#pragma config FOS = PRI           // Oscillator Source (Primary Oscillator)
#pragma config FCKSMEN = CSW_FSCM_OFF// Clock Switching and Monitor (Sw Disabled, Mon Disabled)

// FWDT
#pragma config FWPSB = WDTPSB_16   // WDT Prescaler B (1:16)
#pragma config FWPSA = WDTPSA_512  // WDT Prescaler A (1:512)
#pragma config WDT = WDT_OFF       // Watchdog Timer (Disabled)

// FBORPOR
#pragma config FPWRT = PWRT_64  // POR Timer Value (64ms)
#pragma config BODENV = BORV20  // Brown Out Voltage (Reserved)
#pragma config BOREN = PBOR_ON  // PBOR Enable (Enabled)
#pragma config LPOL = PWMxL_ACT_HI// Low-side PWM Output Polarity (Active High)
#pragma config HPOL = PWMxH_ACT_HI// High-side PWM Output Polarity (Active High)
#pragma config PWMPIN = RST_IOPIN// PWM Output Pin Reset (Control with PORT/TRIS regs)
#pragma config MCLRE = MCLR_EN  // Master Clear Enable (Enabled)

// FGS
#pragma config GWRP = GWRP_OFF      // General Code Segment Write Protect (Disabled)
#pragma config GCP = CODE_PROT_OFF  // General Segment Code Protection (Disabled)

// FICD
#pragma config ICS = ICS_PGD       // Comm Channel Select (Use PGC/EMUC and PGD/EMUD)

#include "xc.h"
#include <stdio.h>
#include <math.h>

#define TIMER1 1

void tmr_setup_period(int timer, int ms){
    
    // Fcy = Fosc / 4 = 7.3728 MHz / 4 = 1843200 (number of clocks in one second)
    // in 0.5 second there would be 921600 clocks steps (moltpilica number of clocks per tempo in s)
    // this is too high to be put in a 16 bit register (max 65535)
    // If we set a prescaler of 1:64 we have 921600/64 = 14400 clock steps, OK!
    //PR1 = 14400; //prescaler setted with the correct clock steps
    
    long int Fosc = 7372800;
    long int Fcy = Fosc/4; //vedi se long
    float steps = Fcy * ((float)ms/1000); //vedi se long
    int presc, t = 1; //init variable for the prescaler setup based on different times
    
    //varios cases
    if (steps < 65535) {
        presc = 1;
        t = 0;
    }
    else if ((float)steps/8 < 65535) {
        presc = 8;
        t = 1;
    }
    else if ((float)steps/64 < 65535) {
        presc = 64;
        t = 2;
    }
    else if ((float)steps/256 < 65535) {
        presc = 256;
        t = 3;
    }
    
    if (timer == 1) {
        TMR1 = 0; // reset timer counter
        PR1 = steps/presc; //set prescaler
        T1CONbits.TCKPS = t; // prescaler 1:64 //VA IN DECIMALE NON IN BINARIO (non 10 ma 2)
        T1CONbits.TON = 1; // starts the timer!
    }
    else if (timer == 2) {
        TMR2 = 0; // reset timer counter
        PR2 = steps/presc; //set prescaler
        T2CONbits.TCKPS = presc; // prescaler 1:64 //VA IN DECIMALE NON IN BINARIO (non 10 ma 2)
        T2CONbits.TON = 1; // starts the timer!
    }
    //else {
        //err
    //}
}

//solution with while loop, not ideal, prova poi a fare con if
void tmr_wait_period(int timer) { 
    if (timer == 1) {
        int flag1 = IFS0bits.T1IF; //initialize the flag1 variable
        while(flag1 == 0) {
            flag1 = IFS0bits.T1IF; //busy waiting for the flag1 to change its value
        }
        IFS0bits.T1IF = 0; //reset timer1 flag
    }
    else if (timer == 2) {
        int flag2 = IFS0bits.T2IF; //initialize the flag2 variable
        while(flag2 == 0) {
            flag2 = IFS0bits.T2IF; //busy waiting for the flag2 to change its value
        };
        IFS0bits.T2IF = 0; //reset timer2 flag
    }
    //else {
        //err
    //} 
}

int main(void) {
    //config lcd
    SPI1CONbits.MSTEN = 1; // master mode
    SPI1CONbits.MODE16 = 0; // 8-bit mode
    SPI1CONbits.PPRE = 3; // 1:1 primary prescaler
    SPI1CONbits.SPRE = 3; // 5:1 secondary prescaler
    SPI1STATbits.SPIEN = 1; // enable SPI
    
    //config uart2    
    U2BRG = 11; // ((7372800 / 4) / (16 * 9600)) ? 1
    U2MODEbits.UARTEN = 1; // enable UART
    U2STAbits.UTXEN = 1; // enable U2TX (must be after UARTEN)
    while (U2STAbits.UTXBF == 1); // Wait for transmit buffer to be not full
    //U2TXREG = 'C'; // Send character via UART2 //non serve a un cazzo?
    
    tmr_setup_period(TIMER1, 1000);
    
    tmr_wait_period(TIMER1);

    //perch� funziona???????
    while (1) {        
        if (U2STAbits.URXDA == 1) {
            char c = U2RXREG; // Read character from UART2
            while(SPI1STATbits.SPITBF == 1);    
            tmr_wait_period(TIMER1);
            SPI1BUF = c;
        }
    }
    
    
    
    return 0;
}
