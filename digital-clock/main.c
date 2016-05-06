#include <stdlib.h>
#include <stdio.h>
#include "avr.h"
#include "lcd.h"
#include "datetime/datetime.h"

#define KEY_DDR   DDRC
#define KEY_PORT  PORTC
#define KEY_PIN   PINC

int year, month, day, hour, minute, second;
DateTime dateTime;

char getInputChar() {
    char returnChar = 0;
    int i, j;
    unsigned char input;
    // key down

    while (1) {
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
                goto KeyUp;
            } else {
                SET_BIT(KEY_PORT, i);
            }
        }
    }

    KeyUp:
    // key up
    while(1) {
        NOP();
        input = KEY_PIN;

        if (((input & (1 << j)) >> j) == 1) {
            SET_BIT(KEY_PORT, i);
            return returnChar;
        }
    }
}

char* getInputString(int len) {
    char* str = (char*) malloc(sizeof(char) * (len + 1));
    int i;
    for (i = 0; i < len; i++) {
        char c = getInputChar();
        while (!(c >= '0' && c <= '9')) {
            c = getInputChar();
        }
        put_lcd(c);
        *(str + i) = c;
    }
    *(str + len) = 0;
    return str;
}

int getIntWithLabel(const char* label, int len) {
    char preLabel[] = "INPUT ";
    char colLabel[] = ": ";

    clr_lcd();
    pos_lcd(0, 0);
    puts_lcd2(preLabel);
    puts_lcd2(label);
    puts_lcd2(colLabel);
    pos_lcd(1, 0);
    char* input = getInputString(len);
    wait_avr(500);

    pos_lcd(1, 0);
    puts_lcd2(label);
    puts_lcd2(colLabel);
    puts_lcd2(input);

    return (int) strtol(input, NULL, 10);
}

// inclusive range
int getIntWithLabelAndRange(const char* label, int len, int min, int max) {
    int nextInt;
    do {
        nextInt = getIntWithLabel(label, len);
    } while (!(nextInt >= min && nextInt <= max));
    return nextInt;
}

void printDate(int year, int month, int day) {
    char date[16];
    sprintf(date, "%04d/%02d/%02d", year, month, day);
    puts_lcd2(date);
}

void printTime(int hour, int minute, int second) {
    char _time[16];
    sprintf(_time, "%02d:%02d:%02d", hour, minute, second);
    puts_lcd2(_time);
}

void printDateTime() {
    clr_lcd();
    pos_lcd(0, 0);
    printDate(dateTime.year, dateTime.month, dateTime.day);
    pos_lcd(1, 0);
    printTime(dateTime.hour, dateTime.minute, dateTime.second);
}

void resetTimer() {
    TCNT1 = 65536 - 31250;
    TCCR1B = 0x04;
}

void timerCallback() {
    tickSecond(&dateTime);
    printDateTime();
}

// interrupt for timer 1
ISR(TIMER1_OVF_vect) {
    TCCR1B = 0x00;
    timerCallback();
    resetTimer();
}

int main(void)
{
    /* insert your hardware initialization here */
    ini_lcd();

    // 0F => 00001111
    KEY_DDR = 0x0F;
    KEY_PORT = 0xFF;
    CLR_BIT(SFIOR, 2); // PUD

    /* Date */
    // input year
    year = getIntWithLabel("YEAR", 4);
    month = getIntWithLabelAndRange("MONTH", 2, 1, 12);
    day = getIntWithLabelAndRange("DAY", 2, 1, 31);

    /* Time */
    hour = getIntWithLabelAndRange("HOUR", 2, 0, 23);
    minute = getIntWithLabelAndRange("MINUTE", 2, 0, 59);
    second = getIntWithLabelAndRange("SECOND", 2, 0, 59);

    // compose the final result
    setDateTime(&dateTime, year, month, day, hour, minute, second);
    printDateTime();

    // enable interrupt
    sei();
    SET_BIT(TIMSK, TOIE1);
    TCCR1B = 0x00;
    resetTimer();

    // main loop
    for(;;);

    /* never reached */
    return 0;
}
