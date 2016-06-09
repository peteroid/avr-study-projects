#include "avr.h"
#include "lcd.h"

#define DDR     DDRB
#define PORT    PORTB
#define RS_PIN  5
#define RW_PIN  6
#define EN_PIN  7

static inline void
set_data(unsigned char x)
{
  PORTA = x;
  DDRA = 0xff;
}

static inline unsigned char
get_data(void)
{
  DDRA = 0x00;
  return PINA;
}

static inline void
sleep_700ns(void)
{
  NOP();
  NOP();
  NOP();
}

static unsigned char
input(unsigned char rs)
{
  unsigned char d;
  if (rs) SET_BIT(PORT, RS_PIN); else CLR_BIT(PORT, RS_PIN);
  SET_BIT(PORT, RW_PIN);
  get_data();
  SET_BIT(PORT, EN_PIN);
  sleep_700ns();
  d = get_data();
  CLR_BIT(PORT, EN_PIN);
  return d;
}

static void
output(unsigned char d, unsigned char rs)
{
  if (rs) SET_BIT(PORT, RS_PIN); else CLR_BIT(PORT, RS_PIN);
  CLR_BIT(PORT, RW_PIN);
  set_data(d);
  SET_BIT(PORT, EN_PIN);
  sleep_700ns();
  CLR_BIT(PORT, EN_PIN);
}

static void
write(unsigned char c, unsigned char rs)
{
  while (input(0) & 0x80);
  output(c, rs);
}

void
ini_lcd(void)
{
  SET_BIT(DDR, RS_PIN);
  SET_BIT(DDR, RW_PIN);
  SET_BIT(DDR, EN_PIN);
  wait_avr(16);
  output(0x30, 0);
  wait_avr(5);
  output(0x30, 0);
  wait_avr(1);
  write(0x3c, 0);
  write(0x0c, 0);
  write(0x06, 0);
  write(0x01, 0);
}

void
clr_lcd(void)
{
  write(0x01, 0);
}

void
pos_lcd(unsigned char r, unsigned char c)
{
  unsigned char n = r * 40 + c;
  write(0x02, 0);
  while (n--) {
    write(0x14, 0);
  }
}

void
put_lcd(char c)
{
  write(c, 1);
}

void
puts_lcd1(const char *s)
{
  char c;
  while ((c = pgm_read_byte(s++)) != 0) {
    write(c, 1);
  }
}

void
puts_lcd2(const char *s)
{
  char c;
  while ((c = *(s++)) != 0) {
    write(c, 1);
  }
}

void set_custom_char(unsigned char n, unsigned char* chars) {
  n++;
  unsigned char cursor = input(0);
  int i;
  for (i = 0; i < 7; i++) {
    write(0x40 | (n << 3) | (i), 0);
    write(chars[i], 1);
  }
  write(0x40 | (n << 3) | 7, 0);
  write(0x00, 1);
  pos_lcd(cursor, 0);
}