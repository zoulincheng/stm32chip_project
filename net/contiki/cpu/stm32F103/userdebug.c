/**
 * \file
 *
 *   Definition of some debugging functions.
 *
 *   putstring() and puthex() are from msp430/watchdog.c
 *
 * \author
 *         George Oikonomou - <oikonomou@users.sourceforge.net>
 */
#include "basictype.h"
#include "userdebug.h"
//#include "debug-uart.h"
#include "xprintf.h"

static const char hexconv[] = "0123456789abcdef";
static const char binconv[] = "01";
/*---------------------------------------------------------------------------*/
void
putstring(char *s)
{
  while(*s) {
    xputc(*s++);
  }
}
/*---------------------------------------------------------------------------*/
void
puthex(uint8_t c)
{
  xputc(hexconv[c >> 4]);
  xputc(hexconv[c & 0x0f]);
}
/*---------------------------------------------------------------------------*/
void
putbin(uint8_t c)
{
  unsigned char i = 0x80;
  while(i) {
    xputc(binconv[(c & i) != 0]);
    i >>= 1;
  }
}
/*---------------------------------------------------------------------------*/
void
putdec(uint8_t c)
{
  uint8_t div;
  uint8_t hassent = 0;
  for(div = 100; div > 0; div /= 10) {
    uint8_t disp = c / div;
    c %= div;
    if((disp != 0) || (hassent) || (div == 1)) {
      hassent = 1;
      xputc('0' + disp);
    }
  }
}
