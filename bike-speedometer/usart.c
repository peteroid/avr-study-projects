#include "usart.h"

void usart_init (void) {
  // enable tx/rx
  SET_BIT(UCSRB, TXEN);
  SET_BIT(UCSRB, RXEN);

  // sync mode
  SET_BIT(UCSRC, UMSEL);

  // no parity
  CLR_BIT(UCSRC, UPM1);
  CLR_BIT(UCSRC, UPM0);

  // 1-bit stop bit
  CLR_BIT(UCSRC, USBS);
  
  // 8-bit frame
  CLR_BIT(UCSRB, UCSZ2);
  SET_BIT(UCSRC, UCSZ1);
  SET_BIT(UCSRC, UCSZ0);

  SET_BIT(UCSRC, UCPOL);

  // BAUD rate
  CLR_BIT(UBRRH, UMSEL);
  UBRRH = USART_BAUD_RATE_BIT_H;
  UBRRL = USART_BAUD_RATE_BIT_L;
}

void usart_transmit( unsigned char data ) {
  /* Wait for empty transmit buffer */
  while ( !( UCSRA & (1<<UDRE)) );
  /* Put data into buffer, sends the data */
  UDR = data;
}

unsigned char usart_receive( void ) {
  /* Wait for data to be received */
  while ( !(UCSRA & (1<<RXC)) );
  /* Get and return received data from buffer */
  return UDR;
}

void usart_flush( void ) {
  unsigned char dummy;
  while ( UCSRA & (1<<RXC) ) dummy = UDR;
}