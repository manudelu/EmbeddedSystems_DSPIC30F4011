/*
 * File:   main.c
 * Authors: Awais Tahir (S5174335)
 *          Raja Farooq Dilshad (5245187)
 *
 * Created on December 20, 2022, 1:47 PM
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
#include <p30F4011.h>
#include <stdio.h>
#include <string.h>
#include "timer.h"
#include "spi.h"
#include "parser.h"

#define TIMER1 (1) 
#define TIMER2 (2)

#define MAX_TASKS 3
#define BUFFER_SIZE 60

#define CIRCULAR_BUFFER_SIZE 15

#define FIRST_ROW 0
#define SECOND_ROW 1

typedef struct {
    char buffer[BUFFER_SIZE];
    int readIndex;
    int writeIndex;
} CircularBuffer;
volatile CircularBuffer cb;

int read_buffer( volatile CircularBuffer* cb, char* value)
{
    if (cb->readIndex == cb-> writeIndex)
        return 0;
    *value = cb->buffer[cb->readIndex];
    cb->readIndex++;
    if (cb->readIndex == BUFFER_SIZE)
        cb->readIndex = 0;
    return 1;
}

void write_buffer(volatile CircularBuffer* cb, char value)
{
    cb->buffer[cb->writeIndex] = value;
    cb->writeIndex++;
    if (cb->writeIndex == BUFFER_SIZE)
        cb->writeIndex = 0;
}

typedef struct {
    int n;
    int N;
} heartbeat;

void blink_led(){
    LATBbits.LATB0 = !LATBbits.LATB0;
}

void adc_configuration() {
    ADCON3bits.ADCS = 16;
    //ADCON1bits.ASAM = 0; // manual sampling start
    ADCON1bits.ASAM = 1; // automatic sampling start
    //ADCON1bits.SSRC = 0; // manual conversion start
    ADCON1bits.SSRC = 7; // automatic conversion start
    ADCON3bits.SAMC = 31; // fixed conversion time (Only if SSRC = 7)
    ADCON2bits.CHPS = 0; // CH0 only
    //ADCON2bits.CHPS = 1; // CH0 & CH1
    ADCHSbits.CH0SA = 2; // AN2 connected to CH0
    //ADCHSbits.CH0SA = 3; // AN3 connected to CH0
    ADCHSbits.CH123SA = 1; // AN3 connected to CH1
    ADPCFG = 0xFFFF;
    ADPCFGbits.PCFG2 = 0; // AN2 as analog
    //ADPCFGbits.PCFG3 = 0; // AN3 as analog
    //ADCON2bits.SMPI = 1; // 2 sample/convert sequences
    //ADCON1bits.SIMSAM = 1;
    //ADCON2bits.CSCNA = 1; // scan mode;

    /*ADCSSL = 0;
    ADCSSLbits.CSSL2 = 1; // scan AN2
    ADCSSLbits.CSSL3 = 1; // scan AN3 */
    ADCON1bits.ADON = 1;
}

int avl_bytes_cb(volatile CircularBuffer* cb) {
    if (cb->readIndex <= cb->writeIndex) {
        return cb->writeIndex - cb->readIndex;
    } else {
        return BUFFER_SIZE - cb->readIndex + cb->writeIndex;
    }
}

void __attribute__((__interrupt__, __auto_psv__)) _U2RXInterrupt() {
    IFS1bits.U2RXIF = 0;

    while (U2STAbits.URXDA == 1) {
        write_buffer(&cb, U2RXREG);
    }
}

void __attribute__((__interrupt__, __auto_psv__)) _U2TXInterrupt() {
    IFS1bits.U2TXIF = 0;

    while (U2STAbits.UTXBF == 0){
        if(read_buffer(&cb, &str6) == 0){
            U2TXREG = str6;
        }else{
            break;
        }
    }
}

void send_string_uart(char* string) {
    int i;
    for (i = 0; string[i] != '\0'; i++) {
        while (U2STAbits.UTXBF == 1);
        U2TXREG = string[i];
    }
}

void send_bytes_uart(char* byte_array, int size) {
    int i;
    for (i = 0; i < size; i++) {
        while (U2STAbits.UTXBF == 1);
        U2TXREG = byte_array[i];
    }
}

int extract_integer(const char* str, int* n) {
    int i = 0, number = 0, sign = 1;
    if (str[i] == '-') {
        sign = -1;
        i++;
    } else if (str[i] == '+') {
        sign = 1;
        i++;
    }
    while (str[i] != ',' && str[i] != '\0') {
        if ((str[i] - '0') < 0 || (str[i] - '0') > 9)
            return -1;
        number *= 10; // multiply the current number by 10;
        number += str[i] - '0'; // converting character to decimal number
        i++;
    }
    *n = sign*number;
    return 0;
}

long map(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Global Variable
float value;

// Task 1
void task1() {
    TRISBbits.TRISB0 = 0;
    
    IEC1bits.U2RXIE = 0;
    int avl = avl_bytes_cb(&cb);
    IEC1bits.U2RXIE = 1;
    
    // Parser Configuration
    parser_state pstate;
    pstate.state = STATE_DOLLAR;
    pstate.index_type = 0;
    pstate.index_payload = 0;
            
    U2BRG = 11; // 9600
    U2MODEbits.UARTEN = 1;
    U2STAbits.UTXEN = 1;
    IEC1bits.U2RXIE = 1;
    
    char str[16];
    char str1[16];
    int count = 0;
        while (count < avl) {
            char byte;
            
            IEC1bits.U2RXIE = 0;
            read_buffer(&cb, &byte);
            IEC1bits.U2RXIE = 1;
            
            int ret = parse_byte(&pstate, byte);
            if (ret == NEW_MESSAGE) {
                
                if (strcmp(pstate.msg_type, "MCREF") == 0) {
                    ret = extract_integer(pstate.msg_payload, &value);
                    if (ret == 0) {
                            sprintf(str, "RPM=%2.1f", value);
                    } else {
                       send_string_uart("ERR4"); 
                    }
                } else {
                    send_string_uart("ERR2");
                }
            } else if (ret == PARSE_ERROR) {
                send_string_uart("ERR1");
            }

            count++;
            double duty_cycle = 0.25;
            char str2[16];
            float voltage = value/200;
            duty_cycle = (value/1000);
            
            PDC2 = duty_cycle * 2 * PTPER;
            sprintf(str2, "DC=%.1f", duty_cycle*100);
            sprintf(str, "PDC2 = %d", PDC2);
        }
}

// Task2
void task2(){
    blink_led();
}

// Task 3
void task3(){
    char str4[16];
    char str5[16];
    char str6[16];
    
    // Reading Potentiometer
    while(!ADCON1bits.DONE);
    int potBits = ADCBUF0;
    float voltages = potBits * 5 / 1024.0;
    float current = map(potBits, 0, 1024, -30, +20);
    sprintf(str4, "ADC=%2.1f,%2.1f", current, voltages);
    spi_move_cursor(FIRST_ROW, 0);
    spi_put_string(str4);
    LATBbits.LATB1 = 0;
    if(current > 15){
        LATBbits.LATB1 = 1;
    }
    
    // Temperature
    while(!ADCON1bits.DONE);
    int tempBits = ADCBUF1;
    float tempVolts = tempBits * 5 / 1024.0;
    float temperature = (tempVolts - 0.75) * 100.0 + 25;
    spi_move_cursor(SECOND_ROW, 0);
    spi_put_string(str5);
    sprintf(str5, "$MCFBK,%2.1f,%2.1f*", current, temperature);
    write_buffer(&cb, &str5);
    
    // Transmitting to UART using Circular Buffer
    IEC1bits.U2RXIE = 0;
    IEC1bits.U2TXIE = 0;
    while (U2STAbits.UTXBF == 0){
    if(read_buffer(&cb, &str6) == 1){
        U2TXREG = str6;
    } else{
        break;
        }
    }
    IEC1bits.U2TXIE = 1;
    IEC1bits.U2RXIE = 1;
    
    // We have tried this but the professor said that we need to send it by 
    // circular buffer.
    // So we tried to do it above
    // Otherwise I can send it by send_string_uart() function 
    // send_string_uart(str5);
}

// Scheduler
void scheduler(heartbeat schedInfo[]) {
    int i;
    for (i = 0; i < MAX_TASKS; i++) {
        schedInfo[i].n++;
        if (schedInfo[i].n >= schedInfo[i].N) {
            switch (i) {
                case 0:
                    task1();
                    break;
                case 1:
                    task2();
                    break;
                case 2:
                    task3();
                    break;
            }
            schedInfo[i].n = 0;
        }
    }
}

int main(void) {
    heartbeat schedInfo[MAX_TASKS];
    
    TRISBbits.TRISB0 = 0;
    TRISBbits.TRISB1 = 0;
    TRISEbits.TRISE8 = 1;
    
    tmr_setup_period(TIMER1, 5);
    
    // SPI Configuration
    SPI1CONbits.PPRE = 0b11; // 1:1
    SPI1CONbits.SPRE = 0b110; // 2:1
    SPI1CONbits.MSTEN = 1; //master
    SPI1CONbits.MODE16 = 0; // 8 bits
    SPI1STATbits.SPIEN = 1; // enable
    
    adc_configuration();
    
    //Scheduling Configuration
    schedInfo[0].n = 0;
    schedInfo[0].N = 1;
    schedInfo[1].n = 0;
    schedInfo[1].N = 100;
    schedInfo[2].n = 0;
    schedInfo[2].N = 200;
    
    // PWM Configuration
    PTCONbits.PTMOD = 0; // free running
    PTCONbits.PTCKPS = 0; // 1:1 prescaler
    PWMCON1bits.PEN2H = 1;
    PWMCON1bits.PEN2L = 1;
    PTPER = 1842; // 1 kHz
    PTCONbits.PTEN = 1; // enable pwm
    
    while(1){
        scheduler(schedInfo);
        tmr_wait_period(TIMER1); 
    }
    return 0;
}

