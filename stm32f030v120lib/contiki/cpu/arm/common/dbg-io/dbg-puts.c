#include <stdio.h>

#include <string.h>


#include "debug_uart.h"

int
puts(const char *str)
{
  dbg_send_bytes((unsigned char*)str, strlen(str));
  dbg_putchar('\n');
  return 0;
}
