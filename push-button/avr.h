#ifndef __avr__
#define __avr__

#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/io.h>

#define XTAL_FRQ 8000000lu

#define SET_BIT(p,i) ((p) |= (1 << (i)))
#define CLR_BIT(p,i) ((p) &= ~(1 << (i)))
#define GET_BIT(p,i) ((p) & (1 << (i)))

// used for a very short delay
#define _NOP() do { __asm__ __volatile__ ("nop"); } while (0)


void ini_avr(void);
void wait_avr(unsigned short msec);

#endif