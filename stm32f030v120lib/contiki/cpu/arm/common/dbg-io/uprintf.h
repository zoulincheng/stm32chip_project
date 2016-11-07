#ifndef _UPRINTF_H
#define _UPRINTF_H

#include <stdio.h>
//#include <debug_uart.h>
#include <string.h>
#include "strformat.h"


extern int u_printf(const char *fmt, ...);

//int printf(const char *fmt, ...);

int u_snprintf(char *str, size_t size, const char *format, ...);

int u_vsnprintf(char *str, size_t size, const char *format, va_list ap);

int u_sprintf(char *str, const char *format, ...);


#endif
