/*
 * File:   newmainXC16.c
 * Author: asus
 *
 * Created on 17 ottobre 2022, 21.32
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


#define TIMER1 1
#define TIMER2 2
void tmr_setup_period(int timer, int ms);
void tmr_wait_period(int timer);
void tmr_wait_ms(int timer, int ms);

int main(void) {
    //Example: read from pin D3
    int pinValue;
    TRISDbits.TRISD3 = 1; // set the pin as input
    pinValue = PORTDbits.RD3; // read from pin
    
    //Example: write to pin B2
    TRISBbits.TRISB2 = 0; // set the pin as output
    LATBbits.LATB2 = 1; // set the pin high
    pinValue = PORTDbits.RD2; // logic error! Cannot read from an output pin
    
    //1. Turn on the led D3
    TRISBbits.TRISB0 = 0; // output
    LATBbits.LATB0 = 1; // logic level high
    while (1) { // infinite loop to keep the led on
    }
    //2. Read the button S5; if the button is pressed turn on the led D3, 
    //otherwise turn it off.
    TRISBbits.TRISB0 = 0; // output (led)
    TRISEbits.TRISE8 = 1; // input (button)
    while (1) {
    if (PORTEbits.RE8 == 0) {
    // pressing the button makes the line go to ground
    // so we will have ?0? in RE8 when we press the button
    LATBbits.LATB0 = 1;
    } else {
    LATBbits.LATB0 = 0;
    }
    }
    //3. Whenever the button S5 is pressed, toggle the status of the led D3 
    //(once per button click).
    TRISBbits.TRISB0 = 0; // output (led)
    TRISEbits.TRISE8 = 1; // input (button)
    int btnStatus, prevbtnStatus;
    prevbtnStatus = 1; // not pressed
    while (1) {
    btnStatus = PORTEbits.RE8;
    if (btnStatus == 0 && prevbtnStatus == 1) {
    // button was just pressed
    LATBbits.LATB0 = !LATBbits.LATB0;
    }
    prevbtnStatus = btnStatus;
    }

    //dsPICDEM2 board oscillator (XT): 7.3728 MHz
    
    //Example: timer expiring every 100 ms, assuming FOSC = 8 MHz
    TMR1 = 0; // reset timer counter
    // Fcy = Fosc / 4 = 8000000 / 4 = 2000000 (number of clocks in one second)
    // in 0.1 second there would be 200000 clocks steps
    // this is too high to be put in a 16 bit register (max 65535)
    // If we set a prescaler of 1:8 we have 200000/8 = 25000 clock steps, OK!
    PR1 = 25000; // (8 MHz / 4) / 8 / 10
    T1CONbits.TCKPS = 1; // prescaler 1:8
    T1CONbits.TON = 1; // starts the timer!
    
    //1. Create two functions:
    //void tmr setup period(int timer, int ms);
    //void tmr wait period(int timer);
    //the first function setups the timer timer to count for the specified
    //amount of milliseconds. The function should support values up
    //to one second (at least). The second function should use the
    //timer flag to wait until it has expired.
    void choose_prescaler(int ms, int? tckps, int? pr) {
        // Fcy = 1843200 Hz ?> 1843,2 clock ticks in 1 ms
        long ticks = 1843.2?ms; // there can be an approximation
        if ( ticks <= 65535) { // if ticks is > 65535 it cannot be put in PR1 (only 16 bits )
            ?tckps = 0;
            ?pr = ticks ;
            return;
        }
        ticks = ticks / 8; // prescaler 1:8;
        if ( ticks <= 65535) {
            ?tckps = 1;
            ?pr = ticks;
            return;
        }
        ticks = ticks / 8; // prescaler 1:64;
        if ( ticks <= 65535) {
            ?tckps = 2;
            ?pr = ticks ;
            return;
        }
        ticks = ticks / 4; // prescaler 1:256;
        ?tckps = 3;
        ?pr = ticks ;
        return;
    }
    void tmr1_setup_period(int ms) {
        T1CONbits.TON = 0;
        TMR1 = 0; // reset the current value;
        int tckps, pr;
        choose prescaler(ms, &tckps, &pr);
        T1CONbits.TCKPS = tckps;
        PR1 = pr;
        T1CONbits.TON = 1;
        return;
    }
    void tmr1 wait period() {
        while ( IFS0bits .T1IF == 0) {
        }
        // I will exit the above loop only when the timer 1 peripheral has expired
        // and it has set the T1IF flag to one
        IFS0bits .T1IF = 0; // set to zero to be able to recognize the next time the timer has expired
    }
    int main() {
        TRISBbits.TRISB0 = 0; // output (led)
        tmr1 setup period(500); // setup and start timer 1
        while (1) {
            // this loop is executed once every 500 ms
            LATBbits.LATB0 = !LATBbits.LATB0 ; // toggle and write the value to the pin
            tmr1 wait period(); // wait what is needed for the next loop
        }
        return 0;
    }
    
    //2. Use the two above defined functions to blink D3 led at 1 Hz
    //frequency (500ms time on, 500ms off)
    void tmr2_wait_ms(int ms) {
        T2CONbits.TON = 0;
        IFS0bits .T2IF = 0;
        TMR2 = 0;
        int tckps, pr;
        choose_prescaler(ms, &tckps, &pr);
        T2CONbits.TCKPS = tckps;
        PR2 = pr;
        T2CONbits.TON = 1;
        while ( IFS0bits .T2IF == 0) {
        }
        IFS0bits .T2IF = 0;
        T2CONbits.TON = 0;
    }
    int main(void) {
        TRISBbits.TRISB0 = 0; // output (led)
        LATBbits.LATB0 = 1;
        tmr2_wait_ms(1000);
        LATBbits.LATB0 = 0;
        tmr2_wait_ms(5000);
        LATBbits.LATB0 = 1;
        tmr2_wait_ms(500);
        LATBbits.LATB0 = 0;
        while (1) {
            // led stays off
        }
        return 0;
    }
   
    //3. Create a function:
    //void tmr wait ms(int timer, int ms);
    //that uses the timer timer to wait for a given number of
    //milliseconds, given as input. Use the function to: turn on the led
    //for 1 second, turn off for 5 seconds, turn on for 500ms, turn off.
    // initialization code
    // [...]
    tmr_wait_ms(TIMER2, 1000);
    // [...]
    tmr_wait_ms(TIMER2, 5000);
    // [...]
    tmr_wait_ms(TIMER2, 500);
    // [...]
    
    //4. Test the following code. Observe the behaviour. Try changing the 
    //delay value from 250, 500, 2000.
    tmr_setup_period(TIMER1, 500);
    int delay = 250;
    while (1) {
        tmr_wait_ms(TIMER2, delay);
        // code to blink LED
        tmr_wait_period(TIMER1);
    }
    
    //5. Pulsing LED
    //A LED pulse is defined as 100 ms on, then off. Pulses are separated 
    //by 100 ms. The period is 1000 ms. With one pulse only, the LED should 
    //stay on 100 ms and off 900 ms; With two pulses, LED should stay on 100 
    //ms, off 100 ms, on 100 ms, and finally off 700 ms.
    //Exercise: Initially, D3 is off. Whenever S5 is pressed, D3 should 
    //increase its pulses, up to a maximum of 3. Once 3 is reached, a further 
    //press will reset the count to 1. If S5 is kept pressed for at least 3 
    //seconds, D3 should be turned off.
    
    //Example of Interrupts: enabling INT0 interrupt
    void __attribute__ ((__interrupt__, __auto_psv__)) _INT0Interrupt() {
        IFS0bits.INT0IF = 0; // reset interrupt flag
        [...] // do some action
    }
    int main() {
        IEC0bits.INT0IE = 1; // enable INT0 interrupt!
        [...] // rest of the code.
    }
    
    //1. Blink D3 led at 1 Hz frequency (500ms time on, 500ms off) without 
    //using interrupts; then make D4 blink at 2 Hz frequency using interrupts.
    #define TIMER1 (1)
    #define TIMER2 (2)
    void tmr_setup_period(int n, int ms) {
        int tckps, pr;
        choose_prescaler(ms, &tckps, &pr);
        switch (n) {
            case TIMER1: {
                TMR1 = 0; // reset the current value;
                T1CONbits.TCKPS = tckps;
                PR1 = pr;
                T1CONbits.TON = 1;
                break;
            }
            case TIMER2: {
                TMR2 = 0; // reset the current value;
                T2CONbits.TCKPS = tckps;
                PR2 = pr;
                T2CONbits.TON = 1;
                break;
            }
        }
    }
    void tmr_wait_period(int n) {
        switch (n) {
            case TIMER1: {
                while ( IFS0bits .T1IF == 0) {}
                IFS0bits .T1IF = 0; // set to zero to be able to recognize the next time the timer has expired
                break;
            }
            case TIMER2: {
                while ( IFS0bits .T2IF == 0) {}
                IFS0bits .T2IF = 0; // set to zero to be able to recognize the next time the timer has expired
                break;
            }
        }
    }
    void __attribute__ (( __interrupt__ , __auto_psv__ ) ) _T2Interrupt() {
        IFS0bits .T2IF = 0;
        LATBbits.LATB1 = !LATBbits.LATB1;
    }
    int main() {
        TRISBbits.TRISB0 = 0; // output (led)
        TRISBbits.TRISB1 = 0; // output (led)
        tmr setup period(TIMER1, 500); // setup and start timer 1
        tmr setup period(TIMER2, 250); // setup and start timer 2
        IEC0bits.T2IE = 1; // enable interrupt for Timer 2
        while (1) {
            // this loop is executed once every 500 ms
            LATBbits.LATB0 = !LATBbits.LATB0; // write the value to the pin
            tmr wait period(TIMER1); // wait what is needed for the next loop
        }
        return 0;
    }
    
    //2. Blink D3 led at 1 Hz frequency (500ms time on, 500ms off) without 
    //using interrupts; every time the button S5 is pressed, toggle the led D4 
    //using interrupts.
    void attribute (( interrupt , auto psv ) ) INT0Interrupt() {
        IFS0bits . INT0IF = 0;
        LATBbits.LATB1 = !LATBbits.LATB1;
        IEC0bits.INT0IE = 0; // disable interrupt for INT0
        tmr setup period(TIMER2, 20); // start timer 2
    }
    void attribute (( interrupt , auto psv ) ) T2Interrupt() {
        IFS0bits .T2IF = 0;
        T2CONbits.TON = 0; // stop timer 2
        IFS0bits . INT0IF = 0; // reset INT0 IF
        IEC0bits.INT0IE = 1; // enable interrupt for INT0
    }
    int main() {
        TRISBbits.TRISB0 = 0; // output (led)
        TRISBbits.TRISB1 = 0; // output (led)
        tmr setup period(TIMER1, 500); // setup and start timer 1
        IEC0bits.INT0IE = 1; // enable interrupt for INT0
        IEC0bits.T2IE = 1; // enable interrupt for T2
        while (1) {
            // this loop is executed once every 500 ms
            LATBbits.LATB0 = !LATBbits.LATB0; // write the value to the pin
            tmr wait period(TIMER1); // wait what is needed for the next loop
        }
        return 0;
    }
    
    return 0;
}
