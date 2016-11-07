#ifndef _DEBUG_UART_H
#define _DEBUG_UART_H

#ifndef dbg_setup_uart
#define dbg_setup_uart dbg_setup_uart_default
#endif

//void dbg_setup_uart( );

//void dbg_set_input_handler(void (*handler)(const char *inp, unsigned int len));

unsigned int dbg_send_bytes(const unsigned char *seq, unsigned int len);

void dbg_putchar(const char ch);


//void dbg_blocking_putchar(const char ch);

//void dbg_drain();



void debug_uart_init(void);
void debug_send_one_byte(unsigned char ubch);





#endif



