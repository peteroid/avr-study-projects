#include <stdlib.h>
#include <stdio.h>
#include "avr.h"
#include "lcd.h"
#include "note.h"

#define KEY_DDR   DDRC
#define KEY_PORT  PORTC
#define KEY_PIN   PINC

#define SPEAKER_DDR  DDRA
#define SPEAKER_PORT PORTA
#define SPEAKER_PIN  7

#define TIMER1_CUSTOM_PRESCALAR 0x03
#define TIMER1_CUSTOM_RESET     0x00

#define TEMPO_MAX  600
#define TEMPO_MIN  150
#define TEMPO_STEP 50

#define VOLUME_BASE 100
#define VOLUME_STEP 5
#define VOLUME_MAX  50
#define VOLUME_MIN  1

// clk = clk/64
const long actualClk = XTAL_FRQ / 64;
long noteTicks, risingTicks, fallingTicks;
unsigned char speakerOutput = 0;
int notes[] = {C4, D4, E4, F4, G4, A4, B4};

// define a new character for the lcd
int numOfBlocks = 7;
unsigned char blocks[7][7] = {
    {0,0xFF,0x18,0x10,0x18,0xFF,0},
    {0,0xFF,0xFF,0xFF,0xFF,0xFF,0},
    {0,0xFF,0,0,0,0xFF,0},
    {0,0xFF,0x03,0x01,0x03,0xFF,0},
    {0x0E,0x0E,0xFF,0xFF,0xFF,0x0E,0x0E},
    {0,0x00,0xFF,0xFF,0xFF,0x00,0},
    {0,0,0,0,0,0,0}};


int songNumber = 4;
int menuItemCount = 2;
Song* songs[5] = {
    &BIRTHDAY_SONG,
    &MARY_HAS_A_LITTLE_LIMB,
    &MARIO_THEME,
    &FUR_ELISE};
int currentIndex = 0;

int tempo = 350; // default
int volume = 50;

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
    TCNT1 = 65536 - noteTicks;
    TCCR1B = TIMER1_CUSTOM_PRESCALAR;
}

void timerCallback() {
    if (!speakerOutput) {
        SET_BIT(SPEAKER_PORT, SPEAKER_PIN);
        noteTicks = risingTicks;
    } else {
        CLR_BIT(SPEAKER_PORT, SPEAKER_PIN);
        noteTicks = fallingTicks;
    }

    speakerOutput = !speakerOutput;
}

// interrupt for timer 1
ISR(TIMER1_OVF_vect) {
    TCCR1B = TIMER1_CUSTOM_RESET;
    timerCallback();
    resetTimer();
}

void hertzToTicks(int hertz) {
    double total = (double) actualClk / hertz;
    risingTicks = (long) (total * volume / VOLUME_BASE);
    fallingTicks = (long) (total - risingTicks);

    noteTicks = risingTicks;
}

void playNote(int note, int duration) {
    if (note != o) {
        hertzToTicks(note);
        resetTimer();
    }
    wait_avr(duration);
    TCCR1B = TIMER1_CUSTOM_RESET;
}

void putMenuTitleAndName (char *title, char *name, char *value) {
    clr_lcd();
    puts_lcd2(title);
    pos_lcd(1,0);
    put_lcd('<');
    puts_lcd2(name);
    puts_lcd2(value);
    pos_lcd(1,15);
    put_lcd('>');
}

void putMenuName (char *name, char *value) {
    putMenuTitleAndName(name, "", value);
}

void putSongName (char *name) {
    putMenuTitleAndName("Select Music", name, "");
}

void putPlayingSongName (char *name) {
    putMenuTitleAndName("Playing...", name, "");
}

void setSliderChar(char buf[], int value, int min, int max, int step) {
    int i, maxLength = 10, offset = 2;

    // hot fix for tempo
    if (min > 1) {
        value -= min;
        max -= min;
        min -= min;
    }

    for (i = 0; i < maxLength; i++) {
        buf[i + offset] = 6;
        if (value / step == i) {
            buf[i + offset] = 5;
        }
    }

    if (value == max) {
        buf[maxLength - 1 + offset] = 5;
    } else if (value == min) {
        buf[offset] = 5;
    }

    for (i = 0; i < offset; i++) {
        buf[i] = 7;
        buf[i + maxLength + offset] = 7;
    }
}

void updateMenuItems () {
    char buffer[15];
    if (currentIndex == songNumber) {
        setSliderChar(buffer, tempo, TEMPO_MIN, TEMPO_MAX, TEMPO_STEP);
        putMenuName("Setting: Tempo", buffer);
    } else if (currentIndex == songNumber + 1) {
        setSliderChar(buffer, volume, VOLUME_MIN, VOLUME_MAX, VOLUME_STEP);
        putMenuName("Setting: Volume", buffer);
    } else {
        putSongName(songs[currentIndex]->name);
    }
}

void modifyTempo (unsigned char dir) {
    if (dir) {
        tempo += TEMPO_STEP;
    } else {
        tempo -= TEMPO_STEP;
    }

    if (tempo > TEMPO_MAX) {
        tempo = TEMPO_MAX;
    } else if (tempo < TEMPO_MIN) {
        tempo = TEMPO_MIN;
    }
}

void modifyVolume (unsigned char dir) {
    if (dir) {
        volume += VOLUME_STEP;
    } else {
        volume -= VOLUME_STEP;
    }

    if (volume > VOLUME_MAX) {
        volume = VOLUME_MAX;
    } else if (volume < VOLUME_MIN) {
        volume = VOLUME_MIN;
    }
}

void playSong(Song *song, int duration) {
    int i;
    putPlayingSongName(song->name);
    for (i = 0; i < song->size; i++) {
        playNote(
            song->noteSpeedPairs[i * 2],
            duration / song->noteSpeedPairs[i * 2 + 1]
        );

        char ch = getInputChar(0);
        if (ch == '2') {
            modifyVolume(1);
        } else if (ch == '8') {
            modifyVolume(0);
        }
    }
}

int main(void){
    /* insert your hardware initialization here */
    ini_lcd();

    int i;
    for (i = 0; i < numOfBlocks; i++) {
        set_custom_char(i, blocks[i]);
    }

    // 0F => 00001111
    KEY_DDR = 0x0F;
    KEY_PORT = 0xFF;
    CLR_BIT(SFIOR, 2); // PUD

    // set output pin for speaker
    SET_BIT(SPEAKER_DDR, SPEAKER_PIN);

    // enable interrupt
    sei();
    SET_BIT(TIMSK, TOIE1);
    TCCR1B = 0x00;

    putSongName(songs[currentIndex]->name);

    // main loop
    for(;;) {
        char ch = getInputChar(1);
        // put_lcd(ch);
        switch (ch) {
        case '6':
            // right
            if (++currentIndex > menuItemCount + songNumber - 1) {
                currentIndex = 0;
            }
            break;
        case '4':
            // left
            if (--currentIndex < 0) {
                currentIndex = songNumber + menuItemCount - 1;
            }
            break;
        case '2':
            // up

            if (currentIndex == songNumber) {
                // tempo
                modifyTempo(1);
            } else if (currentIndex == songNumber + 1) {
                // volume
                modifyVolume(1);
            }
            break;
        case '8':
            // down

            if (currentIndex == songNumber) {
                // tempo
                modifyTempo(0);
            } else if (currentIndex == songNumber + 1) {
                // volume
                modifyVolume(0);
            }
            break;
        case '5':

            // play
            if (currentIndex >= 0 && currentIndex < songNumber) {
                playSong(songs[currentIndex], tempo);
            }
            break;
        }
        updateMenuItems();
    }

    /* never reached */
    return 0;
}
