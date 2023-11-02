/*
 * File:   HelloWorld.c
 * Author: matte
 *
 * Created on 17 ottobre 2023, 12.18
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

#define TIMER1 1

//uses the timer timer to wait for a given number of
//milliseconds, given as input. Use the function to: turn on the led
//for 1 second, turn off for 5 seconds, turn on for 500ms, turn off.
int tmr_wait_ms(int timer, int ms){
    int presc = 1;
    int tk = 0;
    //Fosc = 7372800; Fcy = Fosc/4 = 1843200;
    long int Fcy = 1843200;
    float sec = (float)ms/1000;
    long int steps = Fcy*sec; //921600 -> 1:64 -> 14400 < 65535
    
    if (steps <= 65535){
        presc = 1;
        tk = 0;
    }
    else if (steps/8 <= 65535){
        presc = 8;
        tk = 1;
    }
    else if (steps/64 <= 65535){
        presc = 64;
        tk = 2;
    }
    else{
        
        presc = 256;
        tk = 3;
    }
    
    PR1 = steps/presc; //14400; //steps/presc;
    T1CONbits.TCKPS = tk; //tk; // 2 = 10 = prescaler 1:64 // 1 = 01 = prescaler 1:8
    T1CONbits.TON = 1; // starts the timer!
    
    int flag = IFS0bits.T1IF; //flag value
    if (flag == 1){ //if expired
        IFS0bits.T1IF = 0; //reset the flag
        TMR1 = 0; // reset timer counter
        T1CONbits.TON = 0; 
        return 1;    
    }
    else{
        return 0;
    }    
}

int main() {
    SPI1CONbits.MSTEN = 1; // master mode
    SPI1CONbits.MODE16 = 0; // 8-bit mode
    SPI1CONbits.PPRE = 3; // 1:1 primary prescaler
    SPI1CONbits.SPRE = 3; // 5:1 secondary prescaler
    SPI1STATbits.SPIEN = 1; // enable SPI
    
    tmr_wait_ms(1, 1000);
    while(SPI1STATbits.SPITBF == 1); // wait until not full
    SPI1BUF = "HELLO WORLD"; // send the string
    
    return 0;
}
