/*
 * File:   mainfe2.c
 * Author: asus
 *
 * Created on 28 settembre 2023, 10.47
 */

// DSPIC30F4011 Configuration Bit Settings

// 'C' source line config statements

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

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
//#include "xc.h"

int main(void) {
    //GP=general purpose, Motor=motor control -> set jumpers to motor in H12 -> M 34, 78
    //Leds are linked to RB0 and RB1 -> Motor family -> U2A1 RB0_2
    //button as input, led as output -> INT0 - RE8 
    
    int pinValue1; int pinValue2;
    TRISEbits.TRISE8 = 1; // set the button as input
    TRISBbits.TRISB0 = 0; // set the led as output
    
    //TRISEbits.TRISE8 = 1; // set the button as input
    while(1){
        pinValue1 = PORTEbits.RE8; // read from button
        
        if(pinValue1 == 0){ //while instead of if??
            //TRISBbits.TRISB0 = 0; // set the led as output
            LATBbits.LATB0 = 1; //Turn LED D3 On by setting the pin RB0_2 high
            //pinValue2 = PORTBbits.RB0; // write to led -> ERROR! Can't read from output
        }

    }
    return 0;
}