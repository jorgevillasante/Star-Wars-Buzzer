#pragma config FOSC = INTOSCIO_EC
#pragma config PWRT = ON // Power-up Timer Enable bit (PWRT enabled) 
#pragma config BOR = OFF // Brown-out Reset Enable bits (Brown-out Reset disabled in hardware and software) 
#pragma config WDT = OFF // Watchdog Timer Enable bit (WDT disabled (control is placed on the SWDTEN bit)) 
#pragma config CCP2MX = ON // CCP2 MUX bit (CCP2 input/output is multiplexed with RC1) 
#pragma config PBADEN = OFF // PORTB A/D Enable bit (PORTB<4:0> pins are configured as digital I/O on Reset) 
#pragma config MCLRE = ON // MCLR Pin Enable bit (MCLR pin enabled; RE3 input pin disabled) 
#pragma config LVP = OFF // Single-Supply ICSP Enable bit (Single-Supply ICSP disabled) 
#include <xc.h>

#define _XTAL_FREQ      4000000
#define TMR2PRESCALE    16

// TONOS  ==========================================
#define  c     261
#define  d     294
#define  e     329
#define  f     349
#define  g     391
#define  gS    415
#define  a     440
#define  aS    455
#define  b     466
#define  C     523
#define  CS    554
#define  D     587
#define  DS    622
#define  E     659
#define  F     698
#define  FS    740
#define  G     784
#define  GS    830
#define  A     880

// Define a special note, 'R', to represent a rest
#define  R     0


int tempo = 2000;

int current_tone = 0;
int current_tone_duration = 0;

void delay_ms(unsigned int milliseconds) {
   while(milliseconds > 0) {
      milliseconds--;
       __delay_us(990);
   }
}

void SetupClock() {
    OSCCONbits.IRCF = 0b110;
    OSCCONbits.SCS = 0b00;
}

void PWM1_Init(long desired_frequency) {
    PR2 = (_XTAL_FREQ / (desired_frequency * 4 * TMR2PRESCALE)) - 1;
    current_tone = desired_frequency;
}

int PWM_Max_Duty() {
    return (_XTAL_FREQ / (current_tone * TMR2PRESCALE));
}

void PWM1_Duty(unsigned int duty) {
    if (duty < 1024) {
        duty = ((float) duty / 1023) * PWM_Max_Duty();
        DC1B1 = duty & 2;
        DC1B0 = duty & 1;
        CCPR1L = duty >> 2;
    }
}

void PWM1_Mode_On() {
    CCP1M3 = 1;
    CCP1M2 = 1;
}

void PWM1_Start() {
    PWM1_Mode_On();

#if TMR2PRESCALE == 1
    T2CKPS0 = 0;
    T2CKPS1 = 0;
#elif TMR2PRESCALE == 4
    T2CKPS0 = 1;
    T2CKPS1 = 0;
#elif TMR2PRESCALE == 16
    T2CKPS0 = 1;
    T2CKPS1 = 1;
#endif
    TMR2ON = 1;
}

void PWM1_Stop() {
    CCP1M3 = 0;
    CCP1M2 = 0;
}

void playTone() {
    if (current_tone > 0) {
        PWM1_Init(current_tone);
        PWM1_Duty(511);
        
        delay_ms(current_tone_duration);
        PWM1_Duty(0);
    }
}

int main(int argc, char** argv) {
    short melody[] = {a, a, a, f, C, a, f, C, a, R, E, E, E, F, C, gS, f,       //c, f, c, f, c, f, c, f, C, F, C, F, C, F, C, gS, f,
        C, a, R, A, a, a, A, GS, G, FS, F, FS, R, aS, DS, D, CS, C, b, C, R,
        f, gS, f, a, C, a, C, E, R, A, a, a, A, GS, G, FS, F, FS, R, aS, DS,
        D, CS, C, b, C, R, f, gS, f, C, a, f, C, a, R};
    char beats[] = {50, 50, 50, 35, 15, 50, 35, 15, 65, 50, 50, 50, 50, 35, 15, 50, 35,
        15, 65, 50, 50, 30, 15, 50, 33, 18, 13, 13, 25, 33, 25, 50, 33, 18, 13, 13, 25, 35,
        25, 50, 35, 13, 50, 38, 13, 65, 50, 50, 30, 15, 50, 33, 18, 13, 13, 25, 33, 25, 50,
        33, 18, 13, 13, 25, 35, 25, 50, 38, 13, 50, 38, 13, 65, 65};

    char MELODY_LENGTH = sizeof (melody) / sizeof (melody[0]);

    SetupClock();
    TRISCbits.TRISC2 = 0;
    PORTC = 0x00;

    PWM1_Init(500); //exact value doesn't matter at all, just not 0
    PWM1_Duty(0);
    PWM1_Start();

    while (1) {
        __delay_ms(7000);
        for (int i = 0; i < MELODY_LENGTH; i++) {
            current_tone = melody[i];
            current_tone_duration = 10 * beats[i];

            playTone();
            delay_ms(tempo/200);
        }
        //__delay_ms(2000);
        
        asm("sleep");
    }
    return 0;
}