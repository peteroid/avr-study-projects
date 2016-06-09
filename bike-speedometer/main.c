#include <stdlib.h>
#include <stdio.h>
#include "avr.h"
#include "lcd.h"
#include "usart.h"

#define KEY_DDR   DDRC
#define KEY_PORT  PORTC
#define KEY_PIN   PINC

#define TIMER1_RESET            0x00
#define TIMER1_TOTAL_COUNT      65536
#define TIMER1_CUSTOM_PRESCALAR 0x04
#define TIMER1_CUSTOM_CLICK     XTAL_FRQ / 256
#define TIMER1_CUSTOM_MS        1000
#define TIMER1_CUSTOM_TICKS     TIMER1_TOTAL_COUNT - (TIMER1_CUSTOM_CLICK / 1000 * TIMER1_CUSTOM_MS)

#define BIKE_SPEED_FRAME_SIZE 10
#define BIKE_SPEED_SHOW_MILE  1
#define BIKE_SPEED_SHOW_KM    2

uint32_t int0Ticks;
volatile unsigned char isTimerTicked = 0;

// bike config
float bikeDiameter = 0.6604; // in m (26 inches)
int bikeDuration = TIMER1_CUSTOM_MS; // in ms
float bikeTotalDistance = 0;
float bikeCurrentSpeed = 0;
uint32_t bikeSpeedFrame[BIKE_SPEED_FRAME_SIZE];
int bikeSpeedCount = 0;


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
    TCCR1B = TIMER1_RESET;
    timerCallback();
    resetTimer();
}

void int0Callback() {
    int0Ticks++;
}

// interrupt for ext int0
ISR(INT0_vect) {
  int0Callback();
}

float formatSpeed (float speed, unsigned char mode) {
  switch (mode) {
  case BIKE_SPEED_SHOW_MILE:
    speed = speed * 2.2369356;
    break;
  case BIKE_SPEED_SHOW_KM:
    speed = formatSpeed(speed, BIKE_SPEED_SHOW_MILE) * 1.60934;
    break;
  default:
    break;
  }
  return speed;
}

int main(void){
  ini_lcd();

  // 0F => 00001111
  KEY_DDR = 0x0F;
  KEY_PORT = 0xFF;
  CLR_BIT(SFIOR, 2); // PUD

  // enable interrupt
  sei();
  SET_BIT(TIMSK, TOIE1);
  TCCR1B = 0x00;
  resetTimer();

  // init USART
  usart_init();

  // init interrupt for IO
  CLR_BIT(DDRD, PB2); // set output

  // rising edge of INT0
  SET_BIT(MCUCR, ISC01);
  SET_BIT(MCUCR, ISC00);

  // enable INT0
  SET_BIT(GICR, INT0);

  clr_lcd();
  pos_lcd(0, 0);
  puts_lcd2("HW!");

  
  for (;;) {
    
    if (isTimerTicked) {
      isTimerTicked = 0;
      uint32_t localInt0Ticks = int0Ticks;

      // store the speed in the frame
      if (bikeSpeedCount >= BIKE_SPEED_FRAME_SIZE) {
        bikeSpeedCount = 0;
      }
      bikeSpeedFrame[bikeSpeedCount] = localInt0Ticks;
      bikeSpeedCount++;

      // get the average speed from the frame
      int i, totalTicks = 0;
      for (i = 0; i < BIKE_SPEED_FRAME_SIZE; i++) {
        totalTicks += bikeSpeedFrame[i];
      }

      float distance = totalTicks * bikeDiameter / BIKE_SPEED_FRAME_SIZE;
      bikeCurrentSpeed = formatSpeed(distance * 1000 / bikeDuration, BIKE_SPEED_SHOW_MILE);

      bikeTotalDistance += (distance * 0.000621371);

      char buf[16], buf2[16];
      clr_lcd();
      pos_lcd(0, 0);
      sprintf(buf, "s:%2.2fd:%1.5f",
        bikeCurrentSpeed, bikeTotalDistance);
      puts_lcd2(buf);
      pos_lcd(1, 0);
      sprintf(buf2, "c: %ld%ld%ld%ld%ld %ld%ld%ld%ld%ld",
        bikeSpeedFrame[0],
        bikeSpeedFrame[1],
        bikeSpeedFrame[2],
        bikeSpeedFrame[3],
        bikeSpeedFrame[4],
        bikeSpeedFrame[5],
        bikeSpeedFrame[6],
        bikeSpeedFrame[7],
        bikeSpeedFrame[8],
        bikeSpeedFrame[9]);
      puts_lcd2(buf2);

      char speedBuf[16];
      sprintf(speedBuf, "@@<%f>@@", bikeCurrentSpeed);
      for (i = 0; i < 16; i++) {
        usart_transmit(speedBuf[i]);
      }

      // reset the count for next iteration
      int0Ticks -= localInt0Ticks;
    }
  }

  return 0;
}