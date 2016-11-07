#include <stdio.h>

#include <string.h>


#include "debug_uart.h"



#undef putchar
#undef putc

int
putchar(int c)
{
  dbg_putchar(c);
  return c;
}

int
putc(int c, FILE *f)
{
  dbg_putchar(c);
  return c;
}

int
__sp(struct _reent *_ptr, int c, FILE *_p) {
  dbg_putchar(c);
  return c;
}
