/*
 * File:   switch.c
 * Author: matte
 *
 * Created on 28 settembre 2023, 11.24
 */


// DSPIC30F4011 Configuration Bit Settings

// 'C' source line config statements

#include <xc.h>

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

int main(void) {
    int pinValue1;
    int toggle = 0;
    
    TRISEbits.TRISE8 = 1; // Set the button as IN
    TRISBbits.TRISB0 = 0; // Set the LED as OUT
    
    while(1){
        pinValue1 = PORTEbits.RE8; // Read from button

        if (pinValue1 == 0){ // Button pressed

            if (toggle == 0) { // Was it pressed before?
                LATBbits.LATB0 = 0;
                toggle = 1; // Change 0 to 1
               }            
            else { // Was it pressed before?
                LATBbits.LATB0 = 1;
                toggle = 0; // Change 1 to 0
            }
            //pinValue1 = PORTEbits.RE8; // Read from button
        }           
        //LATBbits.LATB0 = 0;
        }
    return 0;
}