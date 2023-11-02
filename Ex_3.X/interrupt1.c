/*
 * File:   ExeInt2.c
 * Author: asus
 *
 * Created on 10 ottobre 2023, 14.18
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
#define TIMER1 1
#define TIMER2 2
void tmr_setup_period(int timer, int ms){
    int presc = 1;
    int tk = 0;
    long int Fcy = 1843200;
    float sec = (float)ms/1000;
    long int steps = Fcy*sec; 
    
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
    
    if (timer == 1){
        TMR1 = 0; 
        PR1 = steps/presc;
        T1CONbits.TCKPS = tk;
        T1CONbits.TON = 1;
    }
    else{
        TMR2 = 0;
        PR2 = steps/presc;
        T2CONbits.TCKPS = tk; 
        T2CONbits.TON = 1; 
    }
}

void tmr_wait_period(int timer){ //VOID VERSION OF TMR WAIT PERIOD
    //così tutto nel while è sincronizzato a quella frequenza, invece di essere 
    //ripetuto n volte comprese le funzioni successive con delays e false entrate
    if (timer == 1){
        int flag = IFS0bits.T1IF; //flag value
        while (flag == 0){
            flag = IFS0bits.T1IF;
        }
        IFS0bits.T1IF = 0; //reset the flag
        //return 1;
    }
    else{
        int flag = IFS0bits.T2IF; //flag value
        while (flag == 0){
            flag = IFS0bits.T2IF;
        }
        IFS0bits.T2IF = 0; //reset the flag
        //return 1;
    }
}

void __attribute__((__interrupt__, __auto_psv__)) _T1Interrupt() { //ISR
    //IFSx: Interrupt Flag Status registers - These flags are set to ?1? 
    //automatically, but must be set to ?0? by the user
    IFS0bits.T1IF = 0; // reset interrupt flag
    //Rule of thumb: Always read inputs from PORTx and write outputs to LATx.
    //If you need to read what you set an output to, read LATx
    //PORTBbits.RB1; //to read the value on the pin if set as input 
    //LATBbits.LATB1; //to read the value on the pin if set as output -> SEE IF IT WORKS
    if (LATBbits.LATB1 == 0){
        LATBbits.LATB1 = 1;
    }
    else{
        LATBbits.LATB1 = 0;
    }
    //LATBbits.LATB1 = STATELED;
}

// Blink D3 led at 1 Hz frequency (500ms time on, 500ms off)
// without using interrupts; then make D4 blink at 2 Hz frequency
// using interrupts (the timer interrupts).
int main(void) {
    IEC0bits.T1IE = 1; // enable T1 interrupt!
    
    int prev; int expired;
    TRISBbits.TRISB0 = 0; // set the led D3 as output
    TRISBbits.TRISB1 = 0; // set the led D4 as output
    prev = 0;
    tmr_setup_period(TIMER2, 500);
    tmr_setup_period(TIMER1, 1000);
    while (1) {
        tmr_wait_period(TIMER2);
        prev = 1-prev;
        LATBbits.LATB0 = prev;
    }      
    
    return 0;
}
