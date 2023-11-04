#include "UART_header.h"
#define TIMER1 (1) 
#define TIMER2 (2)
#define BUFFER_SIZE 60


void choose_prescaler(int ms, int* tckps, int* pr){
    long ticks = 1843.2*ms;
    if(ticks <= 65535){
        *tckps = 0; 
        *pr = ticks; 
        return;
    }
    ticks = ticks / 8;
    if(ticks <= 65535){
        *tckps = 1; 
        *pr = ticks; 
        return;
    }
    ticks = ticks / 8;
    if(ticks <= 65535){
        *tckps = 2; 
        *pr = ticks; 
        return;
    }
    ticks = ticks / 8;
    if(ticks <= 65535){
        *tckps = 3; 
        *pr = ticks; 
        return;
    }
}

void tmr_setup_period(int n,int ms){ 
    int tckps, pr; 
    choose_prescaler(ms,&tckps,&pr); 
    switch(n){ 
        case TIMER1:{  
            TMR1=0;//reset the current value; 
            T1CONbits.TCKPS=tckps; 
            PR1=pr; 
            T1CONbits.TON=1; 
            break; 
        } 
        case TIMER2:{ 
            TMR2=0;//reset the current value; 
            T2CONbits.TCKPS=tckps; 
            PR2=pr; 
            T2CONbits.TON=1; 
            break; 
        } 
    } 
}

void tmr_wait_period(int n){ 
    switch(n){ 
        case TIMER1:{ 
            while(IFS0bits.T1IF==0){} 
            IFS0bits.T1IF=0;//settozerotobeabletorecognizethenexttimethetimerhasexpired 
            break; 
        } 
        case TIMER2:{ 
            while(IFS0bits.T2IF==0){} 
            IFS0bits.T2IF=0;//settozerotobeabletorecognizethenexttimethetimerhasexpired 
            break; 
        } 
    } 
}



void tmr_wait_ms(int n,int ms){ 
    int tckps, pr; 
    choose_prescaler(ms,&tckps,&pr); 
    switch(n){ 
        case TIMER1:{  
            TMR1=0;//reset the current value; 
            T1CONbits.TCKPS=tckps; 
            PR1=pr; 
            T1CONbits.TON=1; 
            break; 
        } 
        case TIMER2:{ 
            TMR2=0;//reset the current value; 
            T2CONbits.TCKPS=tckps; 
            PR2=pr; 
            T2CONbits.TON=1; 
            break; 
        } 
    } 
}
 
void write_string_LCD(char* str,int max){ 
    int i=0; 
    for(i=0; str[i] !=  '\0' && i < max; i++){ 
        put_char_SPI(str[i]); 
    } 
} 

void put_char_SPI(char c) {
while (SPI1STATbits.SPITBF == 1); // wait for previous transmissions to finish
SPI1BUF = c;
}

void new(char e){
    while (SPI1STATbits.SPITBF == 1); // wait for previous transmissions to finish
    SPI1BUF = e;
}

void move_cursor_first_row(){ 
    put_char_SPI(0x80); 
}

void clear_LCD() {
move_cursor_first_row();
int j = 0;
for (j = 0; j <= 16; j++) {
new(' ') ; // write spaces to ?clear? the LCD from previous characters
    }
}



