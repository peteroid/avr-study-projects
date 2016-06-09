#include <stdlib.h>
#include <stdio.h>
#include "avr.h"
#include "lcd.h"

#define KEY_DDR   DDRC
#define KEY_PORT  PORTC
#define KEY_PIN   PINC

#define TIMER1_CUSTOM_PRESCALAR 0x03
#define TIMER1_CUSTOM_RESET     0x00
#define TIMER1_TOTAL_COUNT      65536
#define TIMER1_CUSTOM_MS        500
#define TIMER1_CUSTOM_TICKS     TIMER1_TOTAL_COUNT - (actualClk / 1000 * TIMER1_CUSTOM_MS)

const long actualClk = XTAL_FRQ / 64;

volatile unsigned char isTimerTicked = 0;
long minVolt = 1023, maxVolt, averageVolt, currentVolt;

typedef struct {
    long decimal;
    long floating;
} Float;

char getInputChar(unsigned char isBlocking) {
    char returnChar = 0;
    int i, j = 0;
    unsigned char input;
    // key down

    do {
        for (i = 0; i < 4; i++) {
            CLR_BIT(KEY_PORT, i);
            NOP();
            input = KEY_PIN;

            if (i == 3) {
                // special case
                if (((input & (1 << 4)) >> 4) == 0) {
                    // button 13
                    returnChar = '*';
                    j = 4;
                } else if (((input & (1 << 5)) >> 5) == 0) {
                    // button 14
                    returnChar = '0';
                    j = 5;
                } else if (((input & (1 << 6)) >> 6) == 0) {
                    // button 15
                    returnChar = '#';
                    j = 6;
                } else if (((input & (1 << 7)) >> 7) == 0) {
                    // button 16
                    returnChar = 'D';
                    j = 7;
                }
            } else {
                if (((input & (1 << 4)) >> 4) == 0) {
                    // button 1
                    returnChar = '1' + i * 3;
                    j = 4;
                } else if (((input & (1 << 5)) >> 5) == 0) {
                    // button 2
                    returnChar = '2' + i * 3;
                    j = 5;
                } else if (((input & (1 << 6)) >> 6) == 0) {
                    // button 3
                    returnChar = '3' + i * 3;
                    j = 6;
                } else if (((input & (1 << 7)) >> 7) == 0) {
                    // button 4
                    returnChar = 'A' + i;
                    j = 7;
                }
            }

            if (returnChar != 0) {
                // change to listen key up
                if (!isBlocking) {
                    SET_BIT(KEY_PORT, i);
                    return returnChar;
                }
                goto KeyUp;
            } else {
                SET_BIT(KEY_PORT, i);
            }
        }
    } while (isBlocking);

    KeyUp:
    // // key up
    while(1) {
        NOP();
        input = KEY_PIN;

        if (((input & (1 << j)) >> j) == 1) {
            SET_BIT(KEY_PORT, i);
            return returnChar;
        }
    }
}

void resetTimer() {
    TCNT1 = TIMER1_CUSTOM_TICKS;
    TCCR1B = TIMER1_CUSTOM_PRESCALAR;
}

void timerCallback() {
    isTimerTicked = 1;
}

// interrupt for timer 1
ISR(TIMER1_OVF_vect) {
    TCCR1B = TIMER1_CUSTOM_RESET;
    timerCallback();
    resetTimer();
}

long _min(long a, long b) {
    return (a < b)? a : b;
}

long _max(long a, long b) {
    return (a > b)? a : b;
}

Float rawADCToVolt (long adc) {
    double d = adc * 5.0 / 1024;
    long decimal = d;
    long floating = (d - decimal) * 1000;
    Float f;
    f.decimal = decimal;
    f.floating = floating;
    return f;
}

void printVolt() {
    char buf1[16], buf2[16];
    Float current = rawADCToVolt(currentVolt);
    Float average = rawADCToVolt(averageVolt);
    Float min = rawADCToVolt(minVolt);
    Float max = rawADCToVolt(maxVolt);

    sprintf(buf1, "C:%1ld.%3ld A:%1ld.%3ld",
        current.decimal, current.floating,
        average.decimal, average.floating);

    sprintf(buf2, "M:%1ld.%3ld X:%1ld.%3ld",
        min.decimal, min.floating,
        max.decimal, max.floating);

    clr_lcd();
    puts_lcd2(buf1);
    pos_lcd(1, 0);
    puts_lcd2(buf2);
}

void resetVolt () {
    minVolt = 1023;
    maxVolt = 0;
    averageVolt = 0;
    currentVolt = 0;
}

int main(void){
    /* insert your hardware initialization here */
    ini_lcd();

    // 0F => 00001111
    KEY_DDR = 0x0F;
    KEY_PORT = 0xFF;
    CLR_BIT(SFIOR, 2); // PUD

    // enable interrupt
    sei();
    SET_BIT(TIMSK, TOIE1);
    TCCR1B = 0x00;

    // configure the ADMUX: 01000000
    // AVCC, Left, ADC0
    ADMUX = 0x40;
    SET_BIT(ADCSRA, ADEN);

    clr_lcd();

    clr_lcd();
    puts_lcd2("C:----- A:-----");
    pos_lcd(1, 0);
    puts_lcd2("M:----- X:-----");

    char c = getInputChar(1);
    switch (c) {

    case '*':
        // start the timer
        resetTimer();
        break;

    }

    // main loop
    for(;;) {
        while (isTimerTicked) {
            SET_BIT(ADCSRA, ADSC);

            // wait for conversion
            while(GET_BIT(ADCSRA, ADSC));

            currentVolt = ADC;
            minVolt = _min(currentVolt, minVolt);
            maxVolt = _max(currentVolt, maxVolt);
            averageVolt = (averageVolt + currentVolt) / 2;

            printVolt();
            isTimerTicked = 0;
        }
    }

    /* never reached */
    return 0;
}
