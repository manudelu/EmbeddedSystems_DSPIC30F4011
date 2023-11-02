/*
 * File:   PrintSecButtonReset.c
 * Author: asus
 *
 * Created on 19 ottobre 2023, 16.32
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
#include <stdio.h>

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
    //così tutto nel while è sincronizzato a quella frequenza, invece di essere ripetuto n volte comprese le funzioni successive con delays e false entrate
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

void tmr_wait_ms(int timer, int ms){
    tmr_setup_period(timer, ms);
    tmr_wait_period(timer);
    if (timer == 1){
        IFS0bits.T1IF = 0; //reset the flag
        TMR1 = 0; // reset timer counter
        T1CONbits.TON = 0; // stop the timer
    }
    else{
        IFS0bits.T2IF = 0; //reset the flag
        TMR2 = 0; // reset timer counter
        T2CONbits.TON = 0; // stop the timer
    }
}

// Improve the previous assignment by resetting the counter every
// time the button S5 is pressed.
int main(void) {
    int pinValue1;
    //int press;
    int i = 0;
    int n = 17; //16 bit + eot
    int cnt = 0;
    int old_i = 0;
    char str[n];
    //char v[] = {'h','e','l','l','o',' ','w','o','r','l','d'};
    tmr_setup_period(TIMER1, 1000);
    //Example: writing one character to the SPI (Fcy = 2 MHz, SPI clock 400 KHz)
    
    SPI1CONbits.MSTEN = 1; // master mode
    SPI1CONbits.MODE16 = 0; // 8?bit mode
    SPI1CONbits.PPRE = 3; // 1:1 primary prescaler
    SPI1CONbits.SPRE = 3; // 5:1 secondary prescaler
    
    SPI1STATbits.SPIEN = 1; // enable SPI
    
    //TRISBbits.TRISB0 = 0; // set the led as output
    TRISEbits.TRISE8 = 1; // set the button as input
    //press = 0;
    
    int pinValueOld;
    pinValueOld = 1;
    while(1){
        tmr_wait_period(TIMER1);
        cnt++; //NOTA: rendila variabile globale così l'interrupt può modificarla e fai l'handling del bottone come interrupt
        
        pinValue1 = PORTEbits.RE8; // read from button
        if (pinValue1 == 0){ //se il bottone è premuto
            if (pinValueOld == 1){ //se è la sua prima pressione
                cnt = 0;
            }
        }
        else{ //se il bottone non è premuto
            if (pinValueOld == 0){ //se è stato premuto in precedenza
                tmr_wait_ms(TIMER2,20);
                pinValue1 = PORTEbits.RE8; //se dopo 20 ms da 1 passa a 0 significa che era un falso rilascio
                //rileggendo il valore impedisco a pinValueOld di diventare 1, rischiando di azzerare di nuovo cnt come fosse un'altra pressione
            }
        }
        pinValueOld = pinValue1;
        /*
        pinValue1 = PORTEbits.RE8; // read from button
        if (pinValue1 == 0){ //se il bottone è premuto
            if (press == 0){ //se è la sua prima pressione
                cnt = 0;
                press = 1; //pressione effettuata almeno una volta
            }
        }
        else{ //se il bottone non è premuto
            press = 0; //nessuna pressione
        }
        */
        
        sprintf(str, "%d", cnt);
        
        for(i = 0; str[i] != '\0'; i++){
            while(SPI1STATbits.SPITBF == 1); // wait until not full
            SPI1BUF = str[i];
        }
        if (i < old_i){
            for(int j = 0; j < old_i-i; i++){
                while(SPI1STATbits.SPITBF == 1); // wait until not full
                SPI1BUF = ' ';
            }
        }
        
        //After each single character display, the cursor is automatically shifted to the next position on the LCD.
        //Cursor repositioning: write the address of the desired location on SPIBUF (first row addresses range from 0x80 to 0x8F, 
        //second row addresses range from 0xC0 through 0xCF).
        //After 16 char are written in the 1st row, it is necessary to explicitly write 0xC0 to place the cursor in the 2nd row.
        while(SPI1STATbits.SPITBF == 1); // wait until not full
        SPI1BUF = 0x80;
        
        old_i = i;
    }
    
    //IFS0bits.T1IF = 0; //reset the flag
    //TMR1 = 0; // reset timer counter
    //T1CONbits.TON = 0; // stop the timer
    //SPI1BUF = 'x'; // send the 'x' character
    //IFS0bits.T1IF = 0; //reset the flag
    //TMR1 = 0; // reset timer counter
    //T1CONbits.TON = 0; // stop the timer
    //while(SPI1STATbits.SPITBF == 1); // wait until not full
    //SPI1BUF = 22; // send the byte containing the value 22

    return 0;
}

