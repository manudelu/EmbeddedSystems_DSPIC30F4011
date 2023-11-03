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

#include "xc.h"

#define FOSC 7372800.0 //Hz Clock breadboard
#define REG_SXT_BIT 65535.0 // MAX 16 bit register
#define TIMER1 1
#define TIMER2 2
#define LED1 1
void tmr_setup_period(int timer, int ms);
void tmr_wait_period(int timer);

void tmr_setup_period(int timer, int ms){ // Set the prescaler and the PR value
// Fosc = 737280 Hz -> Fcy = Fosc / 4 = 184320 number of clocks in one second so in 0.1 secon there would be 184320 clocks steps
// this is too high to be put in a 16 bit register (max 65535)
// If we set a prescaler of 1:8 we have 184320/8 = 23040 clock steps
    double Fcy = FOSC / 4.0;
    double clock_steps = Fcy * (ms/1000.0);
    int count = 0;
    if (clock_steps > REG_SXT_BIT){
        double prescaler_value[3] = {8.0,8.0,4.0};
        while(clock_steps > REG_SXT_BIT){
            clock_steps = clock_steps/prescaler_value[count];
            count += 1;           
        }
    }
    if (timer == TIMER1){
        TMR1 = 0.0; // Reset the timer
        PR1 = clock_steps; // Set the desired count value
        T1CONbits.TCKPS = count; // set PRESCALER
    }
    if (timer == TIMER2){
        TMR2 = 0.0;
        PR2 = clock_steps; 
        T2CONbits.TCKPS = count; // set PRESCALER
    }
}

void tmr_wait_period (int timer) {
    if(timer == TIMER1){
        while(IFS0bits.T1IF == 0){
            // Wait until the flag is high -> The timer finished to count
        }
    }
    
    if(timer == TIMER2){
       while(IFS0bits.T2IF == 0){
            // Wait until the flag is high -> The timer finished to count
        } 
    }
        
}

void tmr_wait_ms(int timer, int ms){
    tmr_setup_period(timer, ms);
    if (timer == TIMER1){
        IFS0bits.T1IF = 0; // Reset the flag
        T1CONbits.TON = 1; // Starts the timer
    }
    if (timer == TIMER2){ // CONTROLLARE T2
        IFS0bits.T2IF = 0; // Reset the flag
        T2CONbits.TON = 1; // Starts the timer
    }
    tmr_wait_period(timer);
}

char algorithm() {
    tmr_wait_ms(TIMER2, 7);
    U1BRG = 11; // (7372800 / 4) / (16 * 9600) ? 1
    U1MODEbits.UARTEN = 1; // enable UART
    U1STAbits.UTXEN = 1;   // enable U1TX (must be after UARTEN)
    //U1TXREG = c; // send ?C?
    
    //if 
            
}

//hyperslowmo 1.30 - 1.35 howtouart tutorial

//if (rowIndex == 0) {
//    lcd_clear(FIRST_ROW);
//}
//lcd_move_cursor(FIRST_ROW, rowIndex);
//spi_put_char(c);
//rowIndex =  (rowIndex + 1) % 16;

//sprintf(secondRow, "Char received: %d", charReceived);
//spi_put_string(secondRow);

//timer_wait_period(TIMER1);

//IEC1bits.U1RXIE = 1 //interrupt ricezione abilitato

void spi_put_string(char* str) {
    for (int i=0; str[i]!='\0'; i++) {
        while (SPI1STATbits.SPITBF == 1);
        SPI1BUF = str[i];
    }
}

void spi_put_string(char c) {
    while (SPI1STATbits.SPITBF == 1);
    SPI1BUF = c;
}

void clearLCD() {
    while(SPI1STATbits.SPITBF == 1); // wait until not full
    SPI1BUF = 0x80;
}


int main() {    
    //config SPI
    SPI1CONbits.MSTEN = 1; // master mode
    SPI1CONbits.MODE16 = 0; // 8?bit mode
    SPI1CONbits.PPRE = 3; // 1:1 primary prescaler
    SPI1CONbits.SPRE = 3; // 5:1 secondary prescaler
    SPI1STATbits.SPIEN = 1; // enable SPI
    
    //config UART
    U2BRG = 11; // (7372800 / 4) / (16 * 9600) ? 1
    U2MODEbits.UARTEN = 1; // enable UART
    U2STAbits.UTXEN = 1; // enable U1TX (must be after UARTEN)
    
    tmr_setup_period(TIMER1, 10);
    
    tmr_setup_period(TIMER1,1000);
    
    while(1) {
        algorithm();
        tmr_wait_period(TIMER1);
        //SPI1BUF = c;
        // code to handle the assignment   
    }
    
    if (U2STAbits.URXDA == 1) {
        char receivedChar = U2RXREG;
        
        // Check if it's a CR or LF character
        if (receivedChar == '\r' || receivedChar == '\n') {
            clearLCD(); //only apply to the FIRST_ROW
        } else {
            // Display received character on the first row of LCD
            displayLCD(&receivedChar, 1, charCount);
            charCount++; //per tenere traccia dei caratteri quando arriva al numero max per riga, fare % numero_max se da 0 -> clear
        }
        
    return 0;
    
    
/* case TIMER1
 * ?
 while (IFS0bits.T1F == 0) {
 * }
 * IFS0bits.T1F = 0;
 * break;
 * 
 * case TIMER2
 */
}

/*
 #include <xc.h>
#include <stdio.h>
#include <string.h>

#define _XTAL_FREQ 20000000  // Define your crystal frequency (20 MHz in this example)

void initUART2() {
    U2BRG = 64; // for 115200 baud rate at 20 MHz clock
    U2MODE = 0x8000; // enable UART, 8N1
    U2STA = 0x1400; // enable TX and RX, clear interrupt flags
}

void initLCD() {
    // Initialize LCD configuration here
}

void displayLCD(char* message, int row, int col) {
    // Function to display a message on LCD at specified row and column
}

void clearLCD() {
    // Function to clear the LCD display
}

int main() {
    int charCount = 0;
    char receivedChar;
    char buffer[10]; // Buffer to hold character count as string

    initUART2();
    initLCD();

    while (1) {
        // Simulate the algorithm execution time
        __delay_ms(7);

        // Check if a character has been received
        if (U2STAbits.URXDA == 1) {
            receivedChar = U2RXREG;

            // Check if it's a CR or LF character
            if (receivedChar == '\r' || receivedChar == '\n') {
                clearLCD();
            } else {
                // Display received character on the first row of LCD
                displayLCD(&receivedChar, 1, charCount);
                charCount++;
            }
        }

        // Check if button S5 is pressed
        if (S5_BUTTON_PRESSED) {
            sprintf(buffer, "%d", charCount);
            putsUART2(buffer);
        }

        // Check if button S6 is pressed
        if (S6_BUTTON_PRESSED) {
            clearLCD();
            charCount = 0;
        }

        // Display the character count on the second row of LCD
        sprintf(buffer, "Char Recv: %d", charCount);
        displayLCD(buffer, 2, 1);
    }

    return 0;
}

 */

