#include "avr.h"

/*

internal 1Mhz
fuse: lb=e1 hb=d1

external crystal
fuse: lb=de hb=d1

*/

// input high
unsigned char pushButtonState;

void wait(char mode, int millis) {
  if (0 == mode) {
    // instruction timing
    volatile int i, n;
    n = 21000;
    for (i = 0; i < n; i++);
  } else if (1 == mode) {
    wait_avr(millis);
  }
}

void blinkLED() {
  SET_BIT(PORTB, 0);
  wait(1, 500);
  CLR_BIT(PORTB, 0);
  wait(1, 500);
}

void loop() {
  while (1) {
    _NOP();
    pushButtonState = GET_BIT(PINB, 1) >> 1;
    if (pushButtonState == 0) {
      blinkLED();
    }
  }
}

int main() {

  // ini_avr();

  // init port B
  SET_BIT(DDRB, 0); // set output
  CLR_BIT(DDRB, 1); // set input

  loop();
}