/*
 * File:   ExampleInt.c
 * Author: asus
 *
 * Created on 10 ottobre 2023, 11.51
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

volatile double encoder; //does not optimize the access with volatile

void __attribute__((__interrupt__, __auto_psv__))_INT0Interrupt(){
    IFS2bits.QEIIF = 0;
    if(QEICONbits.UPDN == 1){
        encoder += MAXCNT;
    }
    else{
        encoder -= MAXCNT;
    }
    //register.bit1 = 0; ////
}

void task_update_feedback(void* param){
    control_data *cd = (control_data*) param;
    IEC2bits.QEIIE = 0;
    cd->positionFeedback = (encoder + POSCNT) / GEAR_RATIO / 12.0 * 360.0;
    cd->encoder = encoder + POSCNT;
    //register.bit3 = 1; YES!
    IEC2bits.QEIIE = 1;
    //register.bit3 = 1; NO! 
}

//void task_send_uart()

int main(void) {
    return 0;
}
