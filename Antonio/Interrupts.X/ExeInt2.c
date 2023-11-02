/*
 * File:   ExampleInt2.c
 * Author: asus
 *
 * Created on 10 ottobre 2023, 12.05
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

int tmr_wait_period(int timer){ //INT VERSION OF TMR WAIT PERIOD
    //così tutto nel while è sincronizzato a quella frequenza, invece di essere ripetuto n volte comprese 
    //le funzioni successive con delays e false entrate
    //questo è un vero wait 
    
    //A quanto pare l'if flag ==1 else che sembra inutile rispetto al while serve solo nel caso vogliamo
    //ritornare int (info ulteriore), altrimenti basta anche solo il while, che comunque non viene iniziato se flag == 1
    if (timer == 1){
        int flag = IFS0bits.T1IF; //flag value
        if (flag == 1){ //if expired
            IFS0bits.T1IF = 0; //reset the flag
            return 1;
        }
        else{
            while (flag == 0){
                flag = IFS0bits.T1IF;
            }
            IFS0bits.T1IF = 0; //reset the flag
            return 1;
        }
    }
    else if (timer == 2){
        int flag = IFS0bits.T2IF; //flag value
        if (flag == 1){ //if expired
            IFS0bits.T2IF = 0; //reset the flag
            return 1;
        }
        else{
            while (flag == 0){
                flag = IFS0bits.T2IF;
            }
            IFS0bits.T2IF = 0; //reset the flag
            return 1;
        }
    }
    else{
        return 0;
    }
}

void __attribute__((__interrupt__, __auto_psv__)) _T2Interrupt() { //ISR
    //IFSx: Interrupt Flag Status registers - These flags are set to ?1? automatically, but must be set to ?0? by the user
    IFS0bits.T2IF = 0; // reset interrupt flag
    //Rule of thumb: Always read inputs from PORTx and write outputs to
    //LATx. If you need to read what you set an output to, read LATx
    //PORTBbits.RB1; //to read the value on the pin if set as input 
    //LATBbits.LATB1; //to read the value on the pin if set as output -> SEE IF IT WORKS
    if (IFS0bits.INT0IF == 1){
        //IFSx: Interrupt Flag Status registers - These flags are set to ?1? automatically, but must be set to ?0? by the user
        IFS0bits.INT0IF = 0; // reset interrupt flag
        //Rule of thumb: Always read inputs from PORTx and write outputs to
        //LATx. If you need to read what you set an output to, read LATx
        //PORTBbits.RB1; //to read the value on the pin if set as input 
        //LATBbits.LATB1; //to read the value on the pin if set as output -> SEE IF IT WORKS
        LATBbits.LATB1 = !LATBbits.LATB1;
    
    }
    
    //STOPPA IL TIMER
    IFS0bits.T2IF = 0; //reset the flag
    TMR2 = 0; // reset timer counter
    T2CONbits.TON = 0; // stop the timer
}

void __attribute__((__interrupt__, __auto_psv__)) _INT0Interrupt() { //ISR
    //attesa 20ms (timer) -> se trovo stato cambiato allora ... -> per evitare multipli input
    tmr_setup_period(TIMER2,20); //solo questo -> ricevi l'interrupt timer -> fai il resto e stoppa il timer!
    //mai attendere nell'interrupt -> no while, no busy waiting, no tmr_wait
}

// Blink D3 led at 1 Hz frequency (500ms time on, 500ms off)
// without using interrupts; every time the button S5 is pressed,
// toggle the led D4 using interrupts
int main(void) {
    //IECx: Interrupt Enable Control registers
    //INTxIE: bit enable
    IEC0bits.INT0IE = 1; // enable INT0 interrupt!
    
    // initialization code
    int pinValue1; int press;
    int prev; int prev2; int expired;
    TRISEbits.TRISE8 = 1; // set the button S5 as input
    TRISBbits.TRISB0 = 0; // set the led D3 as output
    TRISBbits.TRISB1 = 0; // set the led D4 as output
    prev = 0;
    prev2 = 0;
    press = 0;
    tmr_setup_period(TIMER1, 500);
    //tmr_setup_period(TIMER2, 20);
    while (1) {
        // code to blink LED
        expired = tmr_wait_period(TIMER1);
        if (expired == 1){
            prev = 1-prev;
            LATBbits.LATB0 = prev; //LATBbits.LATB0 = !LATBbits.LATB0;
            expired = 0;
        }
        
        //S5 button interrupt
        
        
        // code to toggle with the button S5
        //pinValue1 = PORTEbits.RE8; // read from button
        /*
        if (pinValue1 == 0){ //se il bottone è premuto
            if (press == 0){ //se è la sua prima pressione
                press = 1; //pressione effettuata almeno una volta
            }
        }
        else{ //se il bottone non è premuto
            press = 0; //nessuna pressione
        }
        //LED D4 -> RB1
        */
    }      
            
    return 0;
}