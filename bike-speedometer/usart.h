#include "avr.h"

#ifndef __usart__
#define __usart__

#define USART_BAUDRATE 38400
#define USART_BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)
#define USART_BAUD_RATE_BIT_H USART_BAUD_PRESCALE >> 8
#define USART_BAUD_RATE_BIT_L USART_BAUD_PRESCALE

void usart_init (void);
unsigned char usart_receive (void);
void usart_transmit (unsigned char data);
void usart_flush (void);

#endif